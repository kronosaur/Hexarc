//	CIOCPSocketOp.cpp
//
//	CIOCPSocketOp class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"
#include "IOCompletionPortImpl.h"

DECLARE_CONST_STRING(IOCP_SOCKET_OP,				"IOCP.socketOp")

const int DEFAULT_BUFFER_SIZE =							16 * 1024;

CIOCPSocket::CIOCPSocket (SOCKET hSocket) : IIOCPEntry(IOCP_SOCKET_OP),
		m_hSocket(hSocket)

//	CIOCPSocket constructor

	{
	if (hSocket == INVALID_SOCKET)
		{
		CSocket NewSocket;
		if (!NewSocket.Create(CSocket::typeTCP))
			return;

		m_hSocket = NewSocket.Handoff();
		}
	}

CIOCPSocket::~CIOCPSocket (void)

//	CIOCPSocket destructor

	{
	if (m_hSocket != INVALID_SOCKET)
		{
		shutdown(m_hSocket, SD_BOTH);
		CSocket::CloseHandoffSocket(m_hSocket);
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

	CSocket NewSocket;
	if (!NewSocket.Create(CSocket::typeTCP))
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
