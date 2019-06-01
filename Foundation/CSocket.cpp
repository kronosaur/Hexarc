//	CSocket.cpp
//
//	CSocket class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

//	LATER: Fix
#define _WINSOCK_DEPRECATED_NO_WARNINGS

DECLARE_CONST_STRING(STR_UNKNOWN_HOST,					"0.0.0.0")

int CSocket::m_iGlobalSocketCount = 0;

CSocket::~CSocket (void)

//	CSocket destructor

	{
	Close();
	}

bool CSocket::AcceptConnection (CSocket *retSocket) const

//	AcceptConnection
//
//	Waits until a client connects to the socket. When one does, we return
//	the new socket for the connection.
//
//	This socket must have previously been created with CreateForListen.

	{
	retSocket->Close();

	//	Accept. This will block until a client connects or until a different
	//	thread closes this socket.

	int iAddressSize = sizeof(retSocket->m_Address);
	retSocket->m_hSocket = accept(m_hSocket,
			(SOCKADDR *)&retSocket->m_Address, 
			&iAddressSize);
	if (retSocket->m_hSocket == INVALID_SOCKET)
		return false;

	//	Done

	retSocket->m_bConnected = true;
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

bool CSocket::ComposeAddress (const CString &sHost, DWORD dwPort, SOCKADDR_IN *retAddress)

//	ComposeAddress
//
//	Compose an address from hostname and port
//
//	LATER:
//
//	::inet_addr needs to be replaced with ::inet_pton
//	::gethostbyname needs to be replaced with ::getaddrinfo

	{
	//	Prepare structure to connect

	utlMemSet(retAddress, sizeof(SOCKADDR_IN));
	retAddress->sin_family = AF_INET;
	retAddress->sin_port = ::htons((WORD)dwPort);

	//	Now check to see if the host name is a valid IP address.
	//	If it is not then we do a DNS lookup on the host name.

	retAddress->sin_addr.s_addr = ::inet_addr(sHost);
	if (retAddress->sin_addr.s_addr == INADDR_NONE)
		{
		//	DNS lookup

		HOSTENT *phe = ::gethostbyname(sHost);
		if (phe == NULL)
			return false;

		utlMemCopy((char *)phe->h_addr, (char *)&retAddress->sin_addr, phe->h_length);
		}

	//	Done

	return true;
	}

bool CSocket::Connect (const CString &sHost, DWORD dwPort, Types iType)

//	Connect
//
//	Connect to the host on the given port

	{
	ASSERT(m_hSocket == INVALID_SOCKET);

	//	Prepare structure to connect

	if (!ComposeAddress(sHost, dwPort, &m_Address))
		return false;

	//	Create the socket

	if (!Create(iType))
		return false;

	//	Connect to the server

	if (m_bBlocking)
		{
		if (connect(m_hSocket, (SOCKADDR *)&m_Address, sizeof(m_Address)))
			{
			Close();
			return false;
			}
		}
	else
		{
		if (!ConnectWithTimeout())
			{
			Close();
			return false;
			}
		}

	//	Done

	m_bConnected = true;
	return true;
	}

bool CSocket::ConnectWithTimeout (void)

//	ConnectWithTimeout
//
//	Non-block connect with 60 second timeout

	{
	while (true)
		{
		if (connect(m_hSocket, (SOCKADDR *)&m_Address, sizeof(m_Address)))
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

	return true;
	}

bool CSocket::Create (Types iType)

//	Create
//
//	Creates and binds the socket

	{
	ASSERT(m_hSocket == INVALID_SOCKET);

	//	Compose the type

	int iSocketType;
	switch (iType)
		{
		case typeTCP:
			iSocketType = SOCK_STREAM;
			break;

		case typeUDP:
			iSocketType = SOCK_DGRAM;
			break;

		default:
			ASSERT(false);
			return false;
		}

	//	Create the socket
	//	NOTE: For IPv6 we need to use AF_INET6

	m_hSocket = socket(AF_INET, iSocketType, IPPROTO_TCP);
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

bool CSocket::CreateForListen (DWORD dwPort, Types iType)

//	CreateForListen
//
//	Creates a new socket to listen for connections
//	on the given port.

	{
	ASSERT(m_hSocket == INVALID_SOCKET);

	if (!Create(iType))
		return false;

	//	Bind to a port
	//	NOTE: To use IPv6, change AF_INET to AF_INET6.

	utlMemSet(&m_Address, sizeof(m_Address));
	m_Address.sin_family = AF_INET;
	m_Address.sin_port = htons((WORD)dwPort);
	m_Address.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(m_hSocket, (SOCKADDR *)&m_Address, sizeof(m_Address)) == SOCKET_ERROR)
		{
		Close();
		return false;
		}

	//	Start listening

	if (listen(m_hSocket, SOMAXCONN) == SOCKET_ERROR)
		{
		Close();
		return false;
		}

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

	//	Get the info

	CString sHostName(NI_MAXHOST);
	CString sPortName(NI_MAXSERV);
	if (getnameinfo((SOCKADDR *)&m_Address, 
			sizeof(m_Address), 
			sHostName, 
			sHostName.GetLength(),
			sPortName,
			sPortName.GetLength(),
			NI_NUMERICHOST | NI_NUMERICSERV) != 0)
		{
		return strPattern("%s:%d", STR_UNKNOWN_HOST, 0);
		}

	//	Done

	return strPattern("%s:%s", sHostName, sPortName);
	}

CString CSocket::GetHostAddress (void) const

//	GetHostAddress
//
//	Returns the address that we're connected to

	{
	//	Only if we're connected

	if (m_hSocket == INVALID_SOCKET)
		return STR_UNKNOWN_HOST;

	//	Get the info

	CString sHostName(NI_MAXHOST);
	if (getnameinfo((SOCKADDR *)&m_Address, 
			sizeof(m_Address), 
			sHostName, 
			sHostName.GetLength(),
			NULL,
			0,
			NI_NUMERICHOST
			) != 0)
		{
		sHostName = STR_UNKNOWN_HOST;
		}

	//	Done

	return sHostName;
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
