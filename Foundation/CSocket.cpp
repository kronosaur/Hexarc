//	CSocket.cpp
//
//	CSocket class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

//	LATER: Fix
#define _WINSOCK_DEPRECATED_NO_WARNINGS

DECLARE_CONST_STRING(STR_UNKNOWN_HOST,					"0.0.0.0")

int CSocket::m_iGlobalSocketCount = 0;
const CSocket CSocket::m_Null;

CSocket::CSocket (const ADDRINFOW &AI)

//	CSocket constructor

	{
	m_hSocket = socket(AI.ai_family, AI.ai_socktype, AI.ai_protocol);
	if (m_hSocket != INVALID_SOCKET)
		m_iGlobalSocketCount++;
	}

CSocket::~CSocket (void)

//	CSocket destructor

	{
	Close();
	}

bool CSocket::AcceptConnection (CSocket &retSocket) const

//	AcceptConnection
//
//	Waits until a client connects to the socket. When one does, we return
//	the new socket for the connection.
//
//	This socket must have previously been created with CreateForListen.

	{
	retSocket.Close();

	//	Accept. This will block until a client connects or until a different
	//	thread closes this socket.

	CWSAddrInfoStorage AI;
	retSocket.m_hSocket = accept(m_hSocket,
			&AI.GetAddrRef(), 
			&AI.GetAddrLenRef());
	if (retSocket.m_hSocket == INVALID_SOCKET)
		{
		int iError = WSAGetLastError();
		return false;
		}

	//	Done

	retSocket.m_bConnected = true;
	m_iGlobalSocketCount++;

	return true;
	}

void CSocket::Close (void)

//	Close
//
//	Close the socket

	{
	if (m_hSocket != INVALID_SOCKET)
		{
		Disconnect();

		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
		m_iGlobalSocketCount--;
		}
	}

void CSocket::CloseHandoffSocket (SOCKET hSocket)

//	CloseHandoffSocket
//
//	Close a socket that has been previously handed off

	{
	if (hSocket != INVALID_SOCKET)
		{
		closesocket(hSocket);
		m_iGlobalSocketCount--;
		}
	}

bool CSocket::Connect (const CString &sHost, DWORD dwPort, EType iType)

//	Connect
//
//	Connect to the host on the given port

	{
	if (m_hSocket != INVALID_SOCKET)
		throw CException(errFail);

	//	Prepare structure to connect

	CWSAddrInfo AddrInfo = CWSAddrInfo::Get(sHost, dwPort);
	if (!AddrInfo)
		return false;

	//	Loop over all returned addresses and try to connect

	for (const ADDRINFOW *pAI = &AddrInfo.CastADDRINFO(); pAI != NULL; pAI = pAI->ai_next)
		{
		//	Skip non-internet

		if (pAI->ai_family != AF_INET && pAI->ai_family != AF_INET6)
			continue;

		//	Create the socket

		m_hSocket = socket(pAI->ai_family, pAI->ai_socktype, pAI->ai_protocol);
		if (m_hSocket == INVALID_SOCKET)
			{
			//	Skip
			continue;
			}

		if (!m_bBlocking)
			{
			DWORD dwMode = 1;
			::ioctlsocket(m_hSocket, FIONBIO, &dwMode);
			}

		//	Try to connect

		if (!Connect(pAI->ai_addr, pAI->ai_addrlen))
			{
			closesocket(m_hSocket);
			m_hSocket = INVALID_SOCKET;
			continue;
			}

		//	If we get here then success.

		m_iGlobalSocketCount++;
		m_bConnected = true;
		return true;
		}

	//	Otherwise, we failed

	return false;
	}

bool CSocket::Connect (SOCKADDR *pSockAddr, size_t iSockAddrLen)
	{
	if (m_bBlocking)
		{
		if (connect(m_hSocket, pSockAddr, (int)iSockAddrLen) == SOCKET_ERROR)
			return false;
		}
	else
		{
		while (true)
			{
			if (connect(m_hSocket, pSockAddr, (int)iSockAddrLen) == SOCKET_ERROR)
				{
				int iError = ::WSAGetLastError();
				if (iError != WSAEWOULDBLOCK)
					return false;

				//	Wait until we connect or timeout

				if (!SelectWaitWrite())
					return false;

				//	The socket it ready; try again.
				}

			//	Success

			break;
			}
		}

	return true;
	}

bool CSocket::Create (int iFamily, EType iType)

//	Create
//
//	Creates and binds the socket

	{
	if (m_hSocket != INVALID_SOCKET)
		throw CException(errFail);

	if (iFamily != AF_INET && iFamily != AF_INET6)
		throw CException(errFail);

	//	Compose the type

	int iSocketType;
	switch (iType)
		{
		case EType::TCP:
			iSocketType = SOCK_STREAM;
			break;

		case EType::UDP:
			iSocketType = SOCK_DGRAM;
			break;

		default:
			ASSERT(false);
			return false;
		}

	//	Create the socket
	//	NOTE: For IPv6 we need to use AF_INET6

	m_hSocket = socket(iFamily, iSocketType, IPPROTO_TCP);
	if (m_hSocket == INVALID_SOCKET)
		return false;

	if (!m_bBlocking)
		{
		DWORD dwMode = 1;
		::ioctlsocket(m_hSocket, FIONBIO, &dwMode);
		}

	m_iGlobalSocketCount++;

	//	Done

	return true;
	}

void CSocket::Disconnect (void)

//	Disconnect
//
//	Disconnect

	{
	if (m_bConnected)
		{
		shutdown(m_hSocket, SD_BOTH);
		m_bConnected = false;
		}
	}

CString CSocket::GetConnectionAddress (void) const

//	GetConnectionAddress
//
//	Returns a string that combines host address and port.
//	Callers should not try to parse this string, but should only
//	use it as a connection identifier.
//
//	See: http://tools.ietf.org/html/rfc5952 for IPv6 recommendations

	{
	//	Only if we're connected

	if (m_hSocket == INVALID_SOCKET)
		return STR_UNKNOWN_HOST;

	//	Get the address

	CWSAddrInfoStorage AI;
	if (!WSGetAddressInfo(AI))
		return STR_UNKNOWN_HOST;

	CString sIPAddr;
	CString sPort;
	if (!WSGetNameInfo(AI, AI.GetLength(), true, &sIPAddr, &sPort))
		return STR_UNKNOWN_HOST;

	//	See if this is an IPv6 address

	if (IsIPv6Addr(sIPAddr))
		return strPattern("[%s]:%s", sIPAddr, sPort);
	else
		return strPattern("%s:%s", sIPAddr, sPort);
	}

CString CSocket::GetHostAddress (void) const

//	GetHostAddress
//
//	Returns the address that we're connected to

	{
	//	Only if we're connected

	if (m_hSocket == INVALID_SOCKET)
		return STR_UNKNOWN_HOST;

	//	Get the address

	CWSAddrInfoStorage AI;
	if (!WSGetAddressInfo(AI))
		return STR_UNKNOWN_HOST;

	CString sIPAddr;
	if (!WSGetNameInfo(AI, AI.GetLength(), true, &sIPAddr))
		return STR_UNKNOWN_HOST;

	//	See if this is an IPv6 address

	if (IsIPv6Addr(sIPAddr))
		return strPattern("[%s]", sIPAddr);
	else
		return sIPAddr;
	}

bool CSocket::IsIPv6Addr (const CString &sHostName)

//	IsIPv6Addr
//
//	Parses a numeric hostname and sees if it is an IPv6 address.

	{
	const char *pPos = sHostName.GetParsePointer();
	while (*pPos != '\0')
		{
		if (*pPos == ':')
			return true;

		pPos++;
		}

	return false;
	}

void CSocket::Move (CSocket &Src) noexcept

//	Move
//
//	Takes ownership.

	{
	m_hSocket = Src.m_hSocket;
	Src.m_hSocket = INVALID_SOCKET;

	m_bConnected = Src.m_bConnected;
	m_bBlocking = Src.m_bBlocking;
	}

int CSocket::Read (void *pData, int iLength)

//	Read
//
//	Read from the socket

	{
	ASSERT(m_hSocket != INVALID_SOCKET);

	int iBytesReceived;
	if (m_bBlocking)
		{
		iBytesReceived = recv(m_hSocket, (char *)pData, iLength, 0);
		if (iBytesReceived == SOCKET_ERROR)
			//	LATER: On error we get a -1
			return 0;
		}
	else
		{
		while (true)
			{
			if ((iBytesReceived = recv(m_hSocket, (char *)pData, iLength, 0)) == SOCKET_ERROR)
				{
				int iError = ::WSAGetLastError();
				if (iError != WSAEWOULDBLOCK)
					return 0;

				//	Wait until we connect or timeout

				if (!SelectWaitRead())
					return false;

				//	The socket it ready; try again.

				continue;
				}

			//	Success

			break;
			}
		}

	return iBytesReceived;
	}

bool CSocket::SelectWaitRead (void)

//	SelectWaitRead
//
//	Waits on the socket for read to be available. Or for a timeout

	{
	if (m_bBlocking)
		return true;

	fd_set SocketList;
	FD_ZERO(&SocketList);
	FD_SET(m_hSocket, &SocketList);

	timeval Timeout;
	Timeout.tv_sec = 60;	//	60 second timeout
	Timeout.tv_usec = 0;

	int iResult = select(1, &SocketList, NULL, NULL, &Timeout);
	return (iResult != 0);
	}

bool CSocket::SelectWaitWrite (void)

//	SelectWaitWrite
//
//	Waits on the socket for write to be available. Or for a timeout

	{
	if (m_bBlocking)
		return true;

	fd_set SocketList;
	FD_ZERO(&SocketList);
	FD_SET(m_hSocket, &SocketList);

	timeval Timeout;
	Timeout.tv_sec = 60;	//	60 second timeout
	Timeout.tv_usec = 0;

	int iResult = select(1, NULL, &SocketList, NULL, &Timeout);
	return (iResult != 0);
	}

void CSocket::SetBlockingMode (bool bBlocking)

//	SetBlockingMode
//
//	Sets to blocking/non-blocking mode

	{
	m_bBlocking = bBlocking;
	if (m_hSocket != INVALID_SOCKET)
		{
		DWORD dwMode = (m_bBlocking ? 0 : 1);
		::ioctlsocket(m_hSocket, FIONBIO, &dwMode);
		}
	}

bool CSocket::WSGetAddressInfo (CWSAddrInfoStorage &retAI) const

//	WSGetAddressInfo
//
//	Returns address info for the current socket.

	{
	if (m_hSocket == INVALID_SOCKET)
		return false;

	if (int iError = getsockname(m_hSocket, &retAI.GetAddrRef(), &retAI.GetAddrLenRef()))
		return false;

	return true;
	}

bool CSocket::WSGetNameInfo (const SOCKADDR &SockAddr, size_t iSockAddrLen, bool bNumericHost, CString *retsHostname, CString *retsPort)

//	WSGetNameInfo
//
//	This is a wrapper over GetNameInfoW.

	{
	CString16 sHostName(NI_MAXHOST);
	CString16 sPortName(NI_MAXSERV);

	DWORD dwFlags = NI_NUMERICSERV;
	if (bNumericHost)
		dwFlags |= NI_NUMERICHOST;

	if (int iError = GetNameInfoW(&SockAddr, (socklen_t)iSockAddrLen, sHostName, sHostName.GetLength(), sPortName, sPortName.GetLength(), dwFlags))
		return false;

	if (retsHostname)
		*retsHostname = sHostName;

	if (retsPort)
		*retsPort = sPortName;

	return true;
	}

int CSocket::Write (const void *pData, int iLength)

//	Write
//
//	Write to the socket

	{
	ASSERT(m_hSocket != INVALID_SOCKET);

	int iBytesSent;
	if (m_bBlocking)
		{
		iBytesSent = send(m_hSocket, (char *)pData, iLength, 0);
		if (iBytesSent == SOCKET_ERROR)
			return 0;
		}
	else
		{
		while (true)
			{
			if ((iBytesSent = send(m_hSocket, (char *)pData, iLength, 0)) == SOCKET_ERROR)
				{
				int iError = ::WSAGetLastError();
				if (iError != WSAEWOULDBLOCK)
					return 0;

				//	Wait until we connect or timeout

				if (!SelectWaitWrite())
					return false;

				//	The socket it ready; try again.

				continue;
				}

			//	Success

			break;
			}
		}

	return iBytesSent;
	}
