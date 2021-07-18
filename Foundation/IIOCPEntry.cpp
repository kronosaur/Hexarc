//	IIOCPEntry.cpp
//
//	IIOCPEntry class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_INVALID_STATE,				"Invalid IO operation state.")
DECLARE_CONST_STRING(ERR_FILE_ERROR,				"IO operation error: %d.")
DECLARE_CONST_STRING(ERR_NOT_SUPPORTED,				"IO operation not supported.")
DECLARE_CONST_STRING(ERR_NO_BYTES,					"IO operation returned 0 bytes.")
DECLARE_CONST_STRING(ERR_INVALID_ADDRESS,			"Invalid address: %s %d.")
DECLARE_CONST_STRING(ERR_CANNOT_BIND,				"Unable to bind socket.")

bool IIOCPEntry::BeginConnection (const CString &sAddress, DWORD dwPort, CString *retsError)

//	BeginConnection
//
//	Overlapped connection to the given address.

	{
	//	Must be in the proper state

	HANDLE hHandle = GetCompletionHandle();
	if (hHandle == INVALID_HANDLE_VALUE
			|| m_iCurrentOp != opNone)
		{
		if (retsError) *retsError = ERR_INVALID_STATE;
		return false;
		}

	m_iCurrentOp = opConnect;
	m_dwOpStartTime = sysGetTickCount64();

	//	Bind the socket before passing to ConnectEx. For some reason the API
	//	does not do its own bind (probably so you can reuse sockets).

	SOCKADDR_IN LocalAddress;
	utlMemSet(&LocalAddress, sizeof(LocalAddress), 0);
	LocalAddress.sin_family = AF_INET;
	LocalAddress.sin_addr.s_addr = INADDR_ANY;
	LocalAddress.sin_port = 0;
	if (::bind((SOCKET)hHandle, (SOCKADDR *)&LocalAddress, sizeof(LocalAddress)) != 0)
		{
		m_iCurrentOp = opNone;
		if (retsError) *retsError = ERR_CANNOT_BIND;
		return false;
		}

	//	Get the ConnectEx pointer

	GUID guidConnectEx = WSAID_CONNECTEX;
	LPFN_CONNECTEX pfnConnectEx = NULL;
	DWORD dwBytes;
	if (::WSAIoctl((SOCKET)hHandle,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&guidConnectEx,
			sizeof(guidConnectEx),
			&pfnConnectEx,
			sizeof(pfnConnectEx),
			&dwBytes,
			NULL,
			NULL) == SOCKET_ERROR)
		{
		m_iCurrentOp = opNone;
		if (retsError) *retsError = ERR_NOT_SUPPORTED;
		return false;
		}

	//	Prepare structure to connect

	CWSAddrInfo AddrInfo = CWSAddrInfo::Get(sAddress, dwPort);
	if (!AddrInfo)
		{
		m_iCurrentOp = opNone;
		if (retsError) *retsError = strPattern(ERR_INVALID_ADDRESS, sAddress, dwPort);
		return false;
		}

	const ADDRINFOW *pAI = AddrInfo.GetFirstIPInfo();
	if (!pAI)
		{
		m_iCurrentOp = opNone;
		if (retsError) *retsError = strPattern(ERR_INVALID_ADDRESS, sAddress, dwPort);
		return false;
		}

	//	Initialize operation

	utlMemSet(&m_Overlapped, sizeof(m_Overlapped));

	//	Connect

	DWORD lasterror = 0;
	if (!pfnConnectEx((SOCKET)hHandle,
			pAI->ai_addr,
			(int)pAI->ai_addrlen,
			NULL,
			0,
			NULL,
			&m_Overlapped))
		lasterror = ::WSAGetLastError();

	//	If IO is pending or we succeeded, then nothing to do--we will get an
	//	event on the completion port.

	if (lasterror == ERROR_IO_PENDING 
			|| lasterror == 0)
		return true;

	//	If another error or 0 bytes read, then we fail

	else
		{
		m_iCurrentOp = opNone;

		if (retsError)
			*retsError = strPattern(ERR_FILE_ERROR, lasterror);

		return false;
		}
	}

bool IIOCPEntry::BeginRead (CString *retsError)

//	BeginRead
//
//	Overlapped read on the given handle. Returns FALSE if we get an error.

	{
	//	Must be in the proper state

	HANDLE hHandle = GetCompletionHandle();
	if (hHandle == INVALID_HANDLE_VALUE
			|| m_iCurrentOp != opNone)
		{
		if (retsError) *retsError = ERR_INVALID_STATE;
		return false;
		}

	m_iCurrentOp = opRead;
	m_dwOpStartTime = sysGetTickCount64();

	//	If we don't have a buffer, then we're done.

	IMemoryBlock *pBuffer = GetBuffer();
	if (pBuffer == NULL)
		{
		m_iCurrentOp = opNone;
		if (retsError) *retsError = ERR_INVALID_STATE;
		return false;
		}

	//	Initialize operation

	utlMemSet(&m_Overlapped, sizeof(m_Overlapped));

	//	Let our subclasses know

	OnBeginRead();

	//	Read into the buffer

	DWORD lasterror = 0;
	if (!::ReadFile(hHandle,
			pBuffer->GetPointer(),
			pBuffer->GetLength(),
			NULL,
			&m_Overlapped))
		lasterror = GetLastError();

	//	If IO is pending or we succeeded, then nothing to do--we will get an
	//	event on the completion port.

	if (lasterror == ERROR_IO_PENDING 
			|| lasterror == 0)
		return true;

	//	If another error or 0 bytes read, then we fail

	else
		{
		m_iCurrentOp = opNone;

		if (retsError)
			*retsError = strPattern(ERR_FILE_ERROR, lasterror);

		return false;
		}
	}

bool IIOCPEntry::BeginWrite (const CString &sData, CString *retsError)

//	BeginWrite
//
//	Overlapped write on the given handle. Returns FALSE if we get an error.

	{
	//	Must be in the proper state

	HANDLE hHandle = GetCompletionHandle();
	if (hHandle == INVALID_HANDLE_VALUE
			|| m_iCurrentOp != opNone)
		{
		if (retsError) *retsError = ERR_INVALID_STATE;
		return false;
		}

	m_iCurrentOp = opWrite;
	m_dwOpStartTime = sysGetTickCount64();

	//	If we don't have a buffer, then we're done.

	IMemoryBlock *pBuffer = GetBuffer();
	if (pBuffer == NULL)
		{
		m_iCurrentOp = opNone;
		if (retsError) *retsError = ERR_INVALID_STATE;
		return false;
		}

	//	Initialize operation

	utlMemSet(&m_Overlapped, sizeof(m_Overlapped));

	//	Let our subclasses know (this also initializes the buffer)

	OnBeginWrite(sData);

	//	Write into the buffer

	DWORD lasterror = 0;
	if (!::WriteFile(hHandle,
			pBuffer->GetPointer(),
			pBuffer->GetLength(),
			NULL,
			&m_Overlapped))
		lasterror = GetLastError();

	//	If IO is pending or we succeeded, then nothing to do--we will get an
	//	event on the completion port.

	if (lasterror == ERROR_IO_PENDING
			|| lasterror == 0)
		return true;

	//	If another error or 0 bytes read, then we fail

	else
		{
		m_iCurrentOp = opNone;

		if (retsError)
			*retsError = strPattern(ERR_FILE_ERROR, lasterror);

		return false;
		}
	}

void IIOCPEntry::OperationComplete (DWORD dwBytesTransferred)

//	OperationComplete
//
//	Operation has completed.
	
	{
	if (m_bDeleteOnCompletion)
		{
		delete this;
		return;
		}

	//	We need to reset the status because we might get called back
	//	(on another thread) before we return from OnOperationComplete.

	EOperations iOp = m_iCurrentOp;
	m_iCurrentOp = opNone;

	//	Let our subclass handle this.

	OnOperationComplete(iOp, dwBytesTransferred);
	}

void IIOCPEntry::OperationFailed (void)

//	OperationFailed
//
//	Operation has failed.
	
	{
	if (m_bDeleteOnCompletion)
		{
		delete this;
		return;
		}

	//	We need to reset the status because we might get called back
	//	(on another thread) before we return from OnOperationComplete.

	EOperations iOp = m_iCurrentOp;
	m_iCurrentOp = opNone;

	//	Let our subclass handle this.

	OnOperationFailed(iOp);
	}

void IIOCPEntry::Process (void)
	
//	Process
//
//	Process a simple event
	
	{
	if (m_bDeleteOnCompletion)
		{
		delete this;
		return;
		}
	
	OnProcess(); 
	}

bool IIOCPEntry::TimeoutCheck (DWORDLONG dwNow, DWORDLONG dwTimeout)

//	TimeoutCheck
//
//	Check to see if we need to cancel an operation in progress.
//	NOTE: We rely on our caller to lock appropriately.
//
//	If we return TRUE, it means the caller should delete us.

	{
	if (m_dwOpStartTime == 0)
		return false;

	HANDLE hHandle = GetCompletionHandle();
	if (hHandle == INVALID_HANDLE_VALUE)
		return false;

	DWORDLONG dwElapsed = dwNow - m_dwOpStartTime;
	if (dwElapsed <= dwTimeout)
		return false;

	if (m_iCurrentOp != opNone)
		{
		::CancelIoEx(hHandle, &m_Overlapped);
		return false;
		}
	else
		return true;
	}
