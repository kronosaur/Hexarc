//	CIOCPSocketOp.cpp
//
//	CIOCPSocketOp class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"
#include "IOCompletionPortImpl.h"

DECLARE_CONST_STRING(IOCP_SOCKET_OP,				"IOCP.socketOp");

DECLARE_CONST_STRING(ERR_CANNOT_BIND,				"Unable to bind socket.");
DECLARE_CONST_STRING(ERR_NOT_SUPPORTED,				"IO operation not supported.");
DECLARE_CONST_STRING(ERR_FILE_ERROR,				"IO operation error: %d.");

const int DEFAULT_BUFFER_SIZE =							16 * 1024;

CIOCPSocket::CIOCPSocket (SOCKET hSocket) : IIOCPEntry(IOCP_SOCKET_OP),
		m_hSocket(hSocket)

//	CIOCPSocket constructor

	{
	if (hSocket == INVALID_SOCKET)
		throw CException(errFail);
	}

CIOCPSocket::CIOCPSocket (const CString &sAddress, DWORD dwPort) : IIOCPEntry(IOCP_SOCKET_OP),

		//	Resolve the address and create a socket for the appropriate family
		//	(either IPv4 or IPv6).

		m_AI(CWSAddrInfo::Get(sAddress, dwPort))

//	CIOCPSocket constructor

	{
	//	If we have a valid address, then create a socket. If not, then we will
	//	report the error at connect time.

	const ADDRINFOW *pAI = m_AI.GetFirstIPInfo();
	if (pAI)
		{
		CSocket NewSocket;
		if (NewSocket.Create(pAI->ai_family, CSocket::EType::TCP))
			m_hSocket = NewSocket.Handoff();
		}
	else
		m_hSocket = INVALID_SOCKET;
	}

CIOCPSocket::~CIOCPSocket (void)

//	CIOCPSocket destructor

	{
	if (m_hSocket != INVALID_SOCKET)
		{
		shutdown(m_hSocket, SD_BOTH);
		CSocket::CloseHandoffSocket(m_hSocket);

#ifdef DEBUG
		printf("[%x:%x] Closed socked\n", ((GetID() & 0x00ffffff) + 1), (DWORD)m_hSocket);
#endif
		}
	}

bool CIOCPSocket::CreateConnection (const CString &sAddress, DWORD dwPort, OVERLAPPED &Overlapped, CString *retsError)

//	CreateConnection
//
//	Creates a connection.

	{
	const ADDRINFOW *pAI = m_AI.GetFirstIPInfo();
	if (m_hSocket == INVALID_SOCKET || !pAI)
		throw CException(errFail);

	//	Bind the socket before passing to ConnectEx. For some reason the API
	//	does not do its own bind (probably so you can reuse sockets).

	CWSAddrInfo LocalAI = CWSAddrInfo::GetLocal(pAI->ai_family);
	const ADDRINFOW *pLocalAI = LocalAI.GetFirstIPInfo();
	if (!pLocalAI)
		{
		if (retsError) *retsError = ERR_CANNOT_BIND;
		return false;
		}

	if (::bind(m_hSocket, pLocalAI->ai_addr, (int)pLocalAI->ai_addrlen) != 0)
		{
		if (retsError) *retsError = ERR_CANNOT_BIND;
		return false;
		}

	//	Get the ConnectEx pointer

	GUID guidConnectEx = WSAID_CONNECTEX;
	LPFN_CONNECTEX pfnConnectEx = NULL;
	DWORD dwBytes;
	if (::WSAIoctl(m_hSocket,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&guidConnectEx,
			sizeof(guidConnectEx),
			&pfnConnectEx,
			sizeof(pfnConnectEx),
			&dwBytes,
			NULL,
			NULL) == SOCKET_ERROR)
		{
		if (retsError) *retsError = ERR_NOT_SUPPORTED;
		return false;
		}

	//	Connect

	DWORD lasterror = 0;
	if (!pfnConnectEx(m_hSocket,
			pAI->ai_addr,
			(int)pAI->ai_addrlen,
			NULL,
			0,
			NULL,
			&Overlapped))
		lasterror = ::WSAGetLastError();

	//	If IO is pending or we succeeded, then nothing to do--we will get an
	//	event on the completion port.

	if (lasterror == ERROR_IO_PENDING 
			|| lasterror == 0)
		return true;

	//	If another error or 0 bytes read, then we fail

	else
		{
		if (retsError)
			*retsError = strPattern(ERR_FILE_ERROR, lasterror);

		return false;
		}
	}

bool CIOCPSocket::ResetSocket (void)

//	ResetSocket
//
//	Closes the socket and creates a new one, ready for connecting.

	{
	if (m_hSocket != INVALID_SOCKET)
		{
		shutdown(m_hSocket, SD_BOTH);
		CSocket::CloseHandoffSocket(m_hSocket);

		m_hSocket = INVALID_SOCKET;
		}

	const ADDRINFOW *pAI = m_AI.GetFirstIPInfo();
	if (!pAI)
		//	Only valid for outbound sockets. Otherwise, we don't know how to
		//	recreate the socket.
		throw CException(errFail);

	CSocket NewSocket;
	if (!NewSocket.Create(pAI->ai_family, CSocket::EType::TCP))
		return false;

	m_hSocket = NewSocket.Handoff();

	return true;
	}

void CIOCPSocket::SetReadBufferLen (void)

//	SetReadBufferLen
//
//	Sets the size of the buffer to the default read size.

	{
	if (m_Buffer.GetLength() < DEFAULT_BUFFER_SIZE)
		m_Buffer.SetLength(DEFAULT_BUFFER_SIZE);
	}

void CIOCPSocket::SetWriteBuffer (const CString &sData)

//	SetWriteBuffer
//
//	Sets up the buffer

	{
	m_Buffer.Seek(0);
	m_Buffer.SetLength(0);
	m_Buffer.Write(sData);
	}
