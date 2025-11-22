//	CAMP1CommunicatorServer.cpp
//
//	CAMP1CommunicatorServer class
//	Copyright (c) 2025 by GridWhale Corporation. All Rights Reserved.
//
//	CLIENT								SERVER
//	------								------
//	Connect --------------------------> Accept connection
//
//	Send AUTH0 -----------------------> Handle AUTH0
//	<---------------------------------- Reply AUTH_OK (or Disconnect)
//
//	Send PING ------------------------> Handle PING
//	<---------------------------------- Reply OK
//
//	<---------------------------------- Send Messages
//	Reply OK -------------------------> Handle OK

#include "stdafx.h"

DECLARE_CONST_STRING(AMP1_AUTH0,						"AUTH0");
DECLARE_CONST_STRING(AMP1_AUTH_OK,						"AUTH_OK");
DECLARE_CONST_STRING(AMP1_OK,							"OK");
DECLARE_CONST_STRING(AMP1_PING,							"PING");

CAMP1CommunicatorServer::CAMP1CommunicatorServer () : 
		m_ListenerThread(*this),
		m_ProcessorThread(*this),
		m_ConnectingThread(*this)
	{
	}

void CAMP1CommunicatorServer::AcceptConnection (CSocket&& Socket)

//	AcceptConnection
//
//	Adds a new connection from the listener.

	{
#ifdef DEBUG_AMP1_SERVER
	printf("CAMP1CommunicatorServer::AcceptConnection\n");
#endif

	//	Create a new connection object

	TSharedPtr<SConnectionInfo> pNewConn(new SConnectionInfo);
	pNewConn->iClientID = -1;	//	Not authenticated yet
	pNewConn->Socket = std::move(Socket);
	pNewConn->bConnected = true;

	//	Add the connection

	AddConnection(pNewConn);

	//	Now start reading

	RequestRead(pNewConn);
	}

void CAMP1CommunicatorServer::AddClient (CString sNodeID, const CIPInteger& SecretKey)

//	AddClient
//
//	Add a client that is allowed to connect

	{
	CSmartLock Lock(m_cs);

	SClientInfo Client;
	Client.iClientID = m_Clients.GetCount();
	Client.sNodeID = sNodeID;
	Client.SecretKey = SecretKey;

	m_Clients.Insert(std::move(Client));
	}

void CAMP1CommunicatorServer::AddConnection (TSharedPtr<SConnectionInfo> pConnection)
	{
	CSmartLock Lock(m_cs);

	//	Allocate the read buffer

	pConnection->ReadBuffer.SetLength(MIN_BUFFER_SIZE);

	//	Add to the list and set the ID

	pConnection->dwID = m_Connections.Insert(pConnection);

	//	Add to the IOCP

	if (pConnection->Socket.IsValid())
		m_IOCP.AddObject(pConnection->Socket, (DWORD_PTR)pConnection->dwID);
	}

bool CAMP1CommunicatorServer::ConnectToServer (CStringView sServerAddr, DWORD dwPort, const CIPInteger& SecretKey, CString* retsError)

//	ConnectToServer
//
//	Makes sure we have a connection to the given server and returns TRUE if 
//	successful.

	{
	CSmartLock Lock(m_cs);

	if (m_iState != EState::Client)
		throw CException(errFail);

	//	If we already have a connection a valid connection, then we're done.

	if (FindConnectionByAddress(sServerAddr))
		return true;

#ifdef DEBUG_AMP1_SERVER
	printf("CAMP1CommunicatorServer::ConnectToServer: %s:%d\n", sServerAddr.GetParsePointer(), dwPort);
#endif

	//	Otherwise, try to connect.

	CWSAddrInfo AI(CWSAddrInfo::Get(sServerAddr, dwPort));

	//	If we have a valid address, then create a socket. If not, then we will
	//	report the error at connect time.

	const ADDRINFOW *pAI = AI.GetFirstIPInfo();
	if (!pAI)
		{
		if (retsError) *retsError = strPattern("Unable to resolve server address: %s", sServerAddr);
		return false;
		}

	CSocket NewSocket;
	if (!NewSocket.Create(pAI->ai_family, CSocket::EType::TCP))
		{
		if (retsError) *retsError = strPattern("Unable to create socket: %s", sServerAddr);
		return false;
		}

	//	Create a connection object

	TSharedPtr<SConnectionInfo> pNewConn(new SConnectionInfo);
	pNewConn->sServerAddr = sServerAddr;
	pNewConn->Socket = std::move(NewSocket);

	//	Bind the socket before passing to ConnectEx. For some reason the API
	//	does not do its own bind (probably so you can reuse sockets).

	CWSAddrInfo LocalAI = CWSAddrInfo::GetLocal(pAI->ai_family);
	const ADDRINFOW *pLocalAI = LocalAI.GetFirstIPInfo();
	if (!pLocalAI)
		{
		if (retsError) *retsError = strPattern("Unable to bind socket: %s", sServerAddr);
		return false;
		}

	if (::bind(pNewConn->Socket, pLocalAI->ai_addr, (int)pLocalAI->ai_addrlen) != 0)
		{
		if (retsError) *retsError = strPattern("Unable to bind socket: %s", sServerAddr);
		return false;
		}

	GUID guidConnectEx = WSAID_CONNECTEX;
	LPFN_CONNECTEX pfnConnectEx = NULL;
	DWORD dwBytes;
	if (::WSAIoctl(pNewConn->Socket,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&guidConnectEx,
			sizeof(guidConnectEx),
			&pfnConnectEx,
			sizeof(pfnConnectEx),
			&dwBytes,
			NULL,
			NULL) == SOCKET_ERROR)
		{
		if (retsError) *retsError = strPattern("Unable to connect to server: %s", sServerAddr);
		return false;
		}

	//	Add the connection. After this, any errors need to remove the connection.

	AddConnection(pNewConn);

	//	Get the ConnectEx pointer

	pNewConn->bConnectRequested = true;

	//	Connect

	DWORD dwIgnored;
	DWORD lasterror = 0;
	if (!pfnConnectEx(pNewConn->Socket,
			pAI->ai_addr,
			(int)pAI->ai_addrlen,
			NULL,
			0,
			&dwIgnored,
			&pNewConn->OverlappedWrite))
		lasterror = ::WSAGetLastError();

	//	If IO is pending or we succeeded, then nothing to do--we will get an
	//	event on the completion port.

	if (lasterror == ERROR_IO_PENDING 
			|| lasterror == 0)
		{ }

	//	If another error or 0 bytes read, then we fail

	else
		{
		pNewConn->bConnectRequested = false;
		Disconnect(pNewConn);

		if (retsError) *retsError = strPattern("Unable to connect: %x", lasterror);
		return false;
		}

	//	Wait for connection.

	return false;
	}

void CAMP1CommunicatorServer::Disconnect (TSharedPtr<SConnectionInfo> pConnection)

//	Disconnect
//
//	Disconnect.
//	NOTE: This must be called inside a lock.

	{
	CSmartLock Lock(pConnection->cs);
	if (!pConnection->bConnected)
		return;

#ifdef DEBUG_AMP1_SERVER
	printf("[%x] Disconnect\n", pConnection->dwID);
#endif

	int iClientID = pConnection->iClientID;

	//	Cancel requests, if we have any outstanding.

	if (pConnection->bConnectRequested)
		{
		::CancelIoEx(pConnection->Socket, &pConnection->OverlappedWrite);
		pConnection->bConnectRequested = false;
		}

	if (pConnection->bReadRequested)
		{
		::CancelIoEx(pConnection->Socket, &pConnection->OverlappedRead);
		pConnection->bReadRequested = false;
		}

	if (pConnection->bWriteRequested)
		{
		::CancelIoEx(pConnection->Socket, &pConnection->OverlappedWrite);
		pConnection->bWriteRequested = false;
		}

	pConnection->Socket.Disconnect();
	pConnection->bConnected = false;
	pConnection->bAuthenticated = false;

	Lock.Unlock();

	//	Now that we've marked as disconnected, remove from the list. We can't
	//	remove before we mark as disconnected because another thread might be
	//	working on the connection. We also cannot keep the connection lock while
	//	removing because that would be a lock order violation.
	//
	//	There is a small window after the connection is marked disconnected but
	//	before we remove where another thread might try to use the connection,
	//	but in that case the other thread will see that we're disconnected and
	//	abort.

	CSmartLock Lock2(m_cs);
	m_Connections.Delete(pConnection->dwID);

	//	In client mode we remember that we need to reconnect.

	CString sNodeID;
	if (m_iState == EState::Client)
		m_bConnected = false;
	else
		{
		if (iClientID != -1)
			{
			sNodeID = m_Clients[iClientID].sNodeID;
			m_Clients[iClientID].bConnected = false;
			}
		}

	Lock2.Unlock();

	//	Notify events

	if (m_pEvents && !sNodeID.IsEmpty())
		m_pEvents->OnAMP1ClientDisconnected(sNodeID);
	}

TSharedPtr<CAMP1CommunicatorServer::SConnectionInfo> CAMP1CommunicatorServer::FindConnectionByAddress (CStringView sServerAddr)
	{
	CSmartLock Lock(m_cs);
	for (int i = 0; i < m_Connections.GetCount(); i++)
		{
		TSharedPtr<SConnectionInfo> pConnection = m_Connections[i];
		if (strEqualsNoCase(pConnection->sServerAddr, sServerAddr))
			{
			CSmartLock Lock(pConnection->cs);
			if (!pConnection->bConnected)
				continue;

			return pConnection;
			}
		}

	return NULL;
	}

TSharedPtr<CAMP1CommunicatorServer::SConnectionInfo> CAMP1CommunicatorServer::FindConnectionByID (DWORD dwID)
	{
	CSmartLock Lock(m_cs);
	if (m_Connections.IsValid(dwID))
		return m_Connections.GetAt(dwID);
	else
		return NULL;
	}

bool CAMP1CommunicatorServer::HandleAUTH0 (TSharedPtr<SConnectionInfo> pConnection, DWORD dwDataLen, const char* pData, int* retiClientID)

//	HandleAUTH0
//
//	If this is an AUTH0 command then we try to authenticate the connection.
//	Since we do this without AEON, we store this as two strings separated by
//	a space: {machine-name} {secret-key}.

	{
	const char* pPos = pData;
	const char* pPosEnd = pData + dwDataLen;
	while (pPos < pPosEnd && *pPos != ' ')
		pPos++;

	CString sMachineName = CString(pData, pPos - pData);
	CString sSecretKey = (pPos < pPosEnd ? CString(pPos + 1, pPosEnd - (pPos + 1)) : CString());
	CIPInteger SecretKey;
	SecretKey.InitFromString(sSecretKey);

	//	Look for the client by key.

	int iClientID = -1;
	CString sNodeID;

	CSmartLock Lock(m_cs);
	for (int i = 0; i < m_Clients.GetCount(); i++)
		{
		if (m_Clients[i].SecretKey == SecretKey)
			{
			iClientID = i;
			sNodeID = m_Clients[i].sNodeID;

			m_Clients[i].sMachineName = sMachineName;
			m_Clients[i].dwLastPing = ::sysGetTickCount64();
			m_Clients[i].bConnected = true;

			//	Done

			break;
			}
		}
	Lock.Unlock();

	//	If found, then we can authenticate

	if (iClientID != -1)
		{
		CSmartLock Lock(pConnection->cs);
		if (!pConnection->bConnected)
			return false;

		pConnection->iClientID = iClientID;
		pConnection->bAuthenticated = true;

		//	Send OK

		RequestWrite(pConnection, CAMP1Protocol::MakeMessage(AMP1_AUTH_OK, CBuffer(m_sMachineName)));

		if (retiClientID)
			*retiClientID = iClientID;

		Lock.Unlock();

		//	Notify events

		if (m_pEvents)
			m_pEvents->OnAMP1ClientConnected(sNodeID);

		return true;
		}
	else
		{
		Disconnect(pConnection);
		return false;
		}
	}

bool CAMP1CommunicatorServer::HandleAUTHOK (TSharedPtr<SConnectionInfo> pConnection, CStringView sMachineName)

//	HandleAUTHOK
//
//	The client receives this from the server when the client's authentication
//	has been accepted.

	{
	CSmartLock Lock(pConnection->cs);
	if (!pConnection->bConnected)
		return false;

	pConnection->bAuthenticated = true;
	Lock.Unlock();

	//	Remember that we're connected

	CSmartLock Lock2(m_cs);
	m_Clients[0].sMachineName = sMachineName;
	m_Clients[0].bConnected = true;

	//	Notify events

	if (m_pEvents)
		m_pEvents->OnAMP1ConnectedToServer();

	return true;
	}

void CAMP1CommunicatorServer::HandleCommand (int iClientID, CStringView sCommand, CBuffer&& Data)

//	HandleCommand
//
//	Handle an AMP1 command.

	{
	//	If this is a PING, we reply.

	if (strEquals(sCommand, AMP1_PING))
		{
		CSmartLock Lock(m_cs);
		m_Clients[iClientID].dwLastPing = ::sysGetTickCount64();
		}

	//	Otherwise, send the command.

	else
		{
		CSmartLock Lock(m_cs);
		m_Clients[iClientID].dwLastPing = ::sysGetTickCount64();
		}
	}

void CAMP1CommunicatorServer::HandleRead (TSharedPtr<SConnectionInfo> pConnection, DWORD dwBytesRead)

//	HandleRead
//
//	We've read more into the connection read buffer. Returns TRUE if we need to 
//	continue processing.

	{
	CSmartLock Lock(pConnection->cs);
	if (!pConnection->bConnected)
		return;

	if (!pConnection->bReadRequested)
		throw CException(errFail);

	pConnection->bReadRequested = false;

	//	If no bytes read, then the connection has been closed.

	if (dwBytesRead == 0)
		{
		Disconnect(pConnection);
		return;
		}

	TArray<SCommandDesc> Commands;
	int iClientID = pConnection->iClientID;
	bool bAuthenticated = pConnection->bAuthenticated;
	bool bReadRequested = false;

	//	Keep processing until we've handled everything in the read buffer.

	const char* pPos = pConnection->ReadBuffer.GetPointer();
	const char* pEnd = pPos + dwBytesRead;
	while (pPos < pEnd)
		{
		//	If the header is ready, then we know how much data to expect. We just
		//	copy the data to the data buffer.

		if (pConnection->bHeaderReady)
			{
			DWORD dwBytesNeeded = pConnection->dwExpectedReadSize - pConnection->DataBuffer.GetLength();
			DWORD dwBytesToCopy = Min(dwBytesNeeded, (DWORD)(pEnd - pPos));

			pConnection->DataBuffer.Write(pPos, dwBytesToCopy);
			pPos += dwBytesToCopy;

			//	If we're done, then we have a complete command, so we add it
			//	to the list.

			if (pConnection->DataBuffer.GetLength() == pConnection->dwExpectedReadSize)
				{
				SCommandDesc* pNewCommand = Commands.Insert();
				pNewCommand->sCommand = pConnection->sCommand;
				pNewCommand->Data = std::move(pConnection->DataBuffer);

				pConnection->bHeaderReady = false;
				}

			//	Otherwise, we continue wait for more data.
			}

		//	If we don't have a complete header, then try to parse it.

		else
			{
			//	Look for the CRLF that ends the header.

			const char* pStartHeader = pPos;
			while (pPos < pEnd - 1 && !(pPos[0] == '\r' && pPos[1] == '\n'))
				pPos++;

			//	If found, then parse the header.

			if (pPos[0] == '\r' && pPos[1] == '\n')
				{
				pPos += 2;

				//	If we have previous data in the buffer, we need to combine it.
				//	Otherwise we just parse directly.

				if (pConnection->DataBuffer.GetLength() > 0)
					{
					pConnection->DataBuffer.Write(pStartHeader, (DWORD)(pPos - pStartHeader));
					if (!CAMP1Protocol::GetHeader(pConnection->DataBuffer, &pConnection->sCommand, &pConnection->dwExpectedReadSize))
						{
						//	Bad header
						Disconnect(pConnection);
						return;
						}

					pConnection->DataBuffer.SetLength(0);
					pConnection->bHeaderReady = true;
					}

				//	Parse directly.

				else
					{
					if (!CAMP1Protocol::GetHeader(CBuffer(pStartHeader, (DWORD)(pPos - pStartHeader)), &pConnection->sCommand, &pConnection->dwExpectedReadSize))
						{
						//	Bad header
						Disconnect(pConnection);
						return;
						}

					pConnection->bHeaderReady = true;
					}
				}

			//	If we didn't find the end of the header, then we need to store
			//	the partial data.

			else
				{
				pConnection->DataBuffer.Write(pStartHeader, (DWORD)(pEnd - pStartHeader));
				pPos = pEnd;

				//	If the data is more than the max header size, then this is a bad
				//	request.

				if (pConnection->DataBuffer.GetLength() > CAMP1Protocol::MAX_HEADER_LENGTH)
					{
					Disconnect(pConnection);
					return;
					}
				}
			}
		}

	//	If we're authenticated, then keep reading. Otherwise, we need to wait
	//	until we process the AUTH0 command before requesting more.

	if (pConnection->bAuthenticated || Commands.GetCount() == 0)
		{
		RequestRead(pConnection);
		bReadRequested = true;
		}

	Lock.Unlock();

	//	Now handle all the commands we received.

	for (int i = 0; i < Commands.GetCount(); i++)
		{
		if (strEquals(Commands[i].sCommand, AMP1_AUTH0))
			{
			if (HandleAUTH0(pConnection, Commands[i].Data.GetLength(), Commands[i].Data.GetPointer(), &iClientID))
				bAuthenticated = true;
			}
		else if (strEquals(Commands[i].sCommand, AMP1_AUTH_OK))
			{
			CString sMachineName = CString(Commands[i].Data.GetPointer(), Commands[i].Data.GetLength());

			if (HandleAUTHOK(pConnection, sMachineName))
				bAuthenticated = true;
			}
		else if (bAuthenticated && iClientID != -1)
			{
			HandleCommand(iClientID, Commands[i].sCommand, std::move(Commands[i].Data));
			}
		}

	//	If we didn't request a read above, then we need to do it now.

	if (!bReadRequested)
		RequestRead(pConnection);
	}

void CAMP1CommunicatorServer::HandleWrite (TSharedPtr<SConnectionInfo> pConnection)

//	HandleWrite
//
//	We've completed a write.

	{
	CSmartLock Lock(pConnection->cs);

	if (pConnection->bConnectRequested)
		{
#ifdef DEBUG_AMP1_SERVER
		printf("[%x] Connection completed.\n", pConnection->dwID);
#endif

		pConnection->bConnectRequested = false;
		pConnection->bConnected = true;

		//	Finalize context so the socket becomes a normal connected socket

		int r = ::setsockopt(pConnection->Socket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, nullptr, 0);
		if (r == SOCKET_ERROR)
			{
			Disconnect(pConnection);
			return;
			}

		//	Send an AUTH0 message

		RequestWrite(pConnection, CAMP1Protocol::MakeAUTH0Message(m_sMachineName, m_ServerKey));
		}
	else if (pConnection->bWriteRequested)
		{
#ifdef DEBUG_AMP1_SERVER
		printf("[%x] Write completed.\n", pConnection->dwID);
#endif

		pConnection->bWriteRequested = false;
		}
	}

void CAMP1CommunicatorServer::RequestRead (TSharedPtr<SConnectionInfo> pConnection, int iReadSize)

//	RequestRead
//
//	Requests read on the given connection.
//	NOTE: This must be called inside a lock.

	{
	CSmartLock Lock(pConnection->cs);
	if (!pConnection->bConnected)
		return;

	if (pConnection->bReadRequested)
		throw CException(errFail);

#ifdef DEBUG_AMP1_SERVER
	printf("[%x] RequestRead\n", pConnection->dwID);
#endif

	//	Read

	pConnection->bReadRequested = true;
	pConnection->dwReadStartTime = ::sysGetTickCount64();

	utlMemSet(&pConnection->OverlappedRead, sizeof(pConnection->OverlappedRead));

	//	Read into the buffer

	DWORD lasterror = 0;
	if (!::ReadFile(pConnection->Socket,
			pConnection->ReadBuffer.GetPointer(),
			pConnection->ReadBuffer.GetLength(),
			NULL,
			&pConnection->OverlappedRead))
		lasterror = GetLastError();

	//	If IO is pending or we succeeded, then nothing to do--we will get an
	//	event on the completion port.

	if (lasterror == ERROR_IO_PENDING 
			|| lasterror == 0)
		{ }

	//	If another error or 0 bytes read, then we fail and close the connection.

	else
		{
		Disconnect(pConnection);
		}
	}

void CAMP1CommunicatorServer::RequestWrite (TSharedPtr<SConnectionInfo> pConnection, CStringView sData)

//	RequestWrite
//
//	Request a write on the given connection.

	{
	CSmartLock Lock(pConnection->cs);
	if (!pConnection->bConnected)
		return;

	if (pConnection->bWriteRequested)
		throw CException(errFail);

	//	Make sure we have data to write
	if (sData.IsEmpty())
		throw CException(errFail);

#ifdef DEBUG_AMP1_SERVER
	printf("[%x] RequestWrite: %s\n", pConnection->dwID, (LPCSTR)sData);
#endif

	//	Write

	pConnection->bWriteRequested = true;
	utlMemSet(&pConnection->OverlappedWrite, sizeof(pConnection->OverlappedWrite));

	DWORD lasterror = 0;
	if (!::WriteFile(pConnection->Socket,
			sData.GetPointer(),
			sData.GetLength(),
			NULL,
			&pConnection->OverlappedWrite))
		lasterror = GetLastError();

	//	If IO is pending or we succeeded, then nothing to do--we will get an
	//	event on the completion port.

	if (lasterror == ERROR_IO_PENDING 
			|| lasterror == 0)
		{ }

	//	If another error or 0 bytes read, then we fail and close the connection.

	else
		{
		Disconnect(pConnection);
		}
	}

void CAMP1CommunicatorServer::RunConnection ()

//	RunConnection
//
//	Handle a persistent connection to the server.

	{
	while (true)
		{
		if (m_bTerminated)
			break;

		//	Connect to the server, if we're not already connected.

		CString sError;
		if (!ConnectToServer(m_sServerAddr, m_dwPort, m_ServerKey, &sError))
			{
			if (m_pEvents)
				m_pEvents->OnAMP1FatalError(sError);

			//	Wait a bit before trying again

			::Sleep(5000);
			continue;
			}

		//	Ping the server every second

		auto pConnection = FindConnectionByAddress(m_sServerAddr);
		if (pConnection)
			{
			RequestWrite(pConnection, CAMP1Protocol::MakeMessage(AMP1_PING));
			}

		//	Wait a bit

		::Sleep(1000);
		}
	}

void CAMP1CommunicatorServer::RunListener ()

//	RunListener
//
//	Listen for incoming connections

	{
	//	Keep accepting clients until we're asked to quit

	while (!m_bTerminated)
		{
		//	Accept a new connection

		CSocket NewSocket;
		bool bSuccess = m_Listener.AcceptConnection(NewSocket);

		//	If we've been asked to terminate, then exit

		if (m_bTerminated)
			break;

		//	If accept failed, then report the error and exit

		if (!bSuccess)
			{
			if (m_pEvents)
				m_pEvents->OnAMP1FatalError(CString("CAMP1CommunicatorServer: Unable to accept connection."));

			break;
			}

		//	Accept the new connection

		AcceptConnection(std::move(NewSocket));
		}
	}

void CAMP1CommunicatorServer::RunProcessor ()

//	RunProcessor
//
//	Process incoming messages

	{
	while (true)
		{
		//	Wait until an object completes.

		CIOCompletionPort::SResult Result;
		bool bSuccess = m_IOCP.ProcessCompletion(Result);

		//	If the Ctx is 0, then it means we need to quit.

		if (Result.Ctx == 0)
			{
#ifdef DEBUG_AMP1_SERVER
			printf("RunProcessor: Terminating.\n");
#endif
			return;
			}

		//	Otherwise, it is a connection ID

		TSharedPtr<SConnectionInfo> pConnection = FindConnectionByID((DWORD)Result.Ctx);

		//	If we can't find the connection, it means it was closed while we were
		//	waiting.

		if (!pConnection)
			{
#ifdef DEBUG_AMP1_SERVER
			printf("[%x] RunProcessor: Connection not found.\n", (DWORD)Result.Ctx);
#endif
			continue;
			}

		//	If we failed, then disconnect.

		else if (!bSuccess)
			{
#ifdef DEBUG_AMP1_SERVER
			printf("[%x] RunProcessor: IO failed.\n", pConnection->dwID);
#endif
			Disconnect(pConnection);
			}

		//	If this is a read result, handle it.

		else if (Result.pOverlapped == &pConnection->OverlappedRead)
			{
#ifdef DEBUG_AMP1_SERVER
			printf("[%x] RunProcessor: Read %d bytes.\n", pConnection->dwID, Result.dwBytesTransferred);
#endif
			HandleRead(pConnection, Result.dwBytesTransferred);
			}

		//	If this is a write result, handle it.

		else if (Result.pOverlapped == &pConnection->OverlappedWrite)
			{
			HandleWrite(pConnection);
			}

		//	Otherwise, this can never happen

		else
			{
			throw CException(errFail);
			}
		}
	}

bool CAMP1CommunicatorServer::StartAsClient (CStringView sMachineName, CStringView sServerAddr, DWORD dwPort, const CIPInteger& SecretKey, IAMP1CommunicatorEvents& Events, CString* retsError)

//	StartAsClient
//
//	Start the threads.

	{
	if (m_pEvents || m_iState != EState::Unknown)
		throw CException(errFail);

	m_pEvents = &Events;
	m_sMachineName = sMachineName;
	m_dwPort = dwPort;
	m_sServerAddr = sServerAddr;
	m_ServerKey = SecretKey;

	//	Add a client to represent the server. Arcology Prime is always ID = 0.

	AddClient(CMecharcologyDb::ArcologyPrimeNodeID(), SecretKey);

	//	Start threads

	m_iState = EState::Client;
	m_ConnectingThread.Start();
	m_ProcessorThread.Start();

	return true;
	}

bool CAMP1CommunicatorServer::StartAsServer (DWORD dwPort, IAMP1CommunicatorEvents& Events, CString* retsError)

//	StartAsServer
//
//	Start the threads.

	{
	if (m_pEvents || m_iState != EState::Unknown)
		throw CException(errFail);

	m_pEvents = &Events;
	m_dwPort = dwPort;
	m_sServerAddr = NULL_STR;	//	We're the server
	m_ServerKey = CIPInteger();

	//	Create the listener

	if (!m_Listener.CreateListeners(dwPort))
		{
		if (retsError) *retsError = strPattern("Unable to create listener on port %d.", dwPort);
		return false;
		}

	//	Start threads

	m_iState = EState::Server;
	m_ListenerThread.Start();
	m_ProcessorThread.Start();

	return true;
	}

void CAMP1CommunicatorServer::Shutdown ()

//	Shutdown
//
//	Stop the threads

	{
	CSmartLock Lock(m_cs);

	m_bTerminated = true;

	//	We close the socket to force an error

	if (m_iState != EState::Unknown)
		{
		m_Listener.Close();

		//	Signal a null event to quit.

		m_IOCP.SignalEvent(0);

		//	Done

		m_iState = EState::Unknown;
		}
	}
