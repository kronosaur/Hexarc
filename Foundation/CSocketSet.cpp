//	CSocketSet.cpp
//
//	CSocketSet class
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

bool CSocketSet::AcceptConnection (CSocket &retSocket) const

//	AcceptConnection
//
//	Waits until a client connects to the socket. When one does, we return
//	the new socket for the connection.
//
//	This socket must have previously been created with CreateForListen.

	{
	if (m_Sockets.GetCount() == 0)
		return false;

	//	See if a socket is ready from a previous call.

	const CSocket &SocketReady = Select();
	if (!SocketReady.IsValid())
		return false;

	//	Accept

	return SocketReady.AcceptConnection(retSocket);
	}

bool CSocketSet::CreateListeners (DWORD dwPort, CSocket::EType iType)

//	CreateListeners
//
//	Creates listening sockets for all valid addresses.

	{
	Close();

	CWSAddrInfo AddrInfo = CWSAddrInfo::Get(dwPort);
	if (!AddrInfo)
		return false;

	for (const ADDRINFOW *pAI = &AddrInfo.CastADDRINFO(); pAI != NULL; pAI = pAI->ai_next)
		{
		//	Skip non-internet

		if (pAI->ai_family != AF_INET && pAI->ai_family != AF_INET6)
			continue;

		//	Create a socket

		CSocket Socket(*pAI);
		if (!Socket.IsValid())
			continue;

		//	Bind

		if (bind(Socket, pAI->ai_addr, (int)pAI->ai_addrlen) == SOCKET_ERROR)
			{
			continue;
			}

		//	Listen

		if (listen(Socket, SOMAXCONN) == SOCKET_ERROR)
			{
			continue;
			}

		//	If we get this far, then add the socket to our list.

		int iIndex = m_Sockets.GetCount();
		if (iIndex >= FD_SETSIZE)
			continue;

		m_Sockets.InsertEmpty();
		m_Sockets[iIndex] = std::move(Socket);
		}

	//	Done

	return m_Sockets.GetCount() > 0;
	}

const CSocket &CSocketSet::Select () const

//	Select
//
//	Calls select.

	{
	//	See if we have a previously selected socket.

	for (int i = 0; i < m_Sockets.GetCount(); i++)
		if (FD_ISSET(m_Sockets[i], &m_SockSet))
			{
			FD_CLR(m_Sockets[i], &m_SockSet);
			return m_Sockets[i];
			}

	//	Set all

	for (int i = 0; i < m_Sockets.GetCount(); i++)
		FD_SET(m_Sockets[i], &m_SockSet);

	//	Select

	if (select(m_Sockets.GetCount(), &m_SockSet, NULL, NULL, NULL) == SOCKET_ERROR)
		return CSocket::Null();

	//	Return it

	for (int i = 0; i < m_Sockets.GetCount(); i++)
		if (FD_ISSET(m_Sockets[i], &m_SockSet))
			{
			FD_CLR(m_Sockets[i], &m_SockSet);
			return m_Sockets[i];
			}

	//	This should never happen.

	return CSocket::Null();
	}
