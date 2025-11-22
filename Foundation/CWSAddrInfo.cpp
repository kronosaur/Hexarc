//	CWSAddrInfo.cpp
//
//	CWSAddrInfo class
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CWSAddrInfo CWSAddrInfo::Get (DWORD dwPort, CString *retsError)

//	Get
//
//	Gets address info for binding to a listener.

	{
	//	Convert to Unicode

	CString16 sPort16(strFromInt(dwPort));

	//	By setting the AI_PASSIVE flag in the hints to getaddrinfo, we're
	//	indicating that we intend to use the resulting address(es) to bind
	//	to a socket(s) for accepting incoming connections.  This means that
	//	when the Address parameter is NULL, getaddrinfo will return one
	//	entry per allowed protocol family containing the unspecified address
	//	for that family.

	ADDRINFOW Hints;
	utlMemSet(&Hints, sizeof(Hints));
	Hints.ai_family = AF_UNSPEC;        // Accept either IPv4 or IPv6
	Hints.ai_socktype = SOCK_STREAM;    //  TCP
	Hints.ai_protocol = IPPROTO_TCP;	//	TCP
	Hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;

	CWSAddrInfo Result;
	int iResult = ::GetAddrInfoW(NULL, sPort16, &Hints, &Result.m_pAddrInfo);
	if (iResult != 0)
		{
		int iLastError = WSAGetLastError();
		if (retsError) *retsError = strPattern("Unable to get address info: Error = %d", iLastError);
		return CWSAddrInfo();
		}

	return Result;
	}

CWSAddrInfo CWSAddrInfo::Get (const CString &sHost, DWORD dwPort, CString *retsError)

//	Get
//
//	Gets the address info. If there's an error, then we return a NULL object.

	{
	//	Convert to Unicode

	CString16 sHost16(sHost);
	CString16 sPort16(strFromInt(dwPort));

	//  By not setting the AI_PASSIVE flag in the hints to getaddrinfo, we're
	//  indicating that we intend to use the resulting address(es) to connect
	//  to a service.  This means that when the Server parameter is NULL,
	//  getaddrinfo will return one entry per allowed protocol family
	//  containing the loopback address for that family.

	ADDRINFOW Hints;
	utlMemSet(&Hints, sizeof(Hints));
	Hints.ai_family = AF_UNSPEC;        // Accept either IPv4 or IPv6
	Hints.ai_socktype = SOCK_STREAM;    //  TCP
	Hints.ai_protocol = IPPROTO_TCP;	//	TCP

	CWSAddrInfo Result;
	int iResult = ::GetAddrInfoW(sHost16, sPort16, &Hints, &Result.m_pAddrInfo);
	if (iResult != 0)
		{
		int iLastError = WSAGetLastError();
		if (retsError) *retsError = strPattern("Unable to resolve %s: Error = %d", sHost, iLastError);
		return CWSAddrInfo();
		}

	return Result;
	}

CWSAddrInfo CWSAddrInfo::GetLocal (int iFamily, CString *retsError)

//	GetLocal
//
//	Returns a local address.

	{
	ADDRINFOW Hints;
	utlMemSet(&Hints, sizeof(Hints));
	Hints.ai_family = iFamily;
	Hints.ai_socktype = SOCK_STREAM;    //  TCP
	Hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV | AI_PASSIVE;

	CWSAddrInfo Result;
	int iResult = ::GetAddrInfoW(NULL, L"0", &Hints, &Result.m_pAddrInfo);
	if (iResult != 0)
		{
		int iLastError = WSAGetLastError();
		if (retsError) *retsError = CString("Unable to resolve local address.");
		return CWSAddrInfo();
		}

	return Result;
	}

const ADDRINFOW *CWSAddrInfo::GetFirstIPInfo () const

//	GetFirstIPInfo
//
//	Returns the first entry that support IP (or NULL)

	{
	for (const ADDRINFOW *pAI = m_pAddrInfo; pAI != NULL; pAI = pAI->ai_next)
		{
		//	Skip non-internet

		if (pAI->ai_family != AF_INET && pAI->ai_family != AF_INET6)
			continue;

		//	Found

		return pAI;
		}

	//	Not found

	return NULL;
	}

bool CWSAddrInfo::GetNext (SAddrInfo &retInfo) const

//  GetNext
//
//  Fills in retInfo with the next address information block. If there are no 
//	more then we return FALSE.

	{
	ADDRINFOW *pNext;
	if (retInfo.pIterator == NULL)
		pNext = m_pAddrInfo;
	else
		pNext = ((ADDRINFOW *)retInfo.pIterator)->ai_next;

	if (!pNext)
		return false;

	retInfo.pIterator = pNext;
	retInfo.iFlags = pNext->ai_flags;
	retInfo.iFamily = pNext->ai_family;
	retInfo.iSockType = pNext->ai_socktype;
	retInfo.iProtocol = pNext->ai_protocol;
	retInfo.sName = CString16(pNext->ai_canonname);

	return true;
	}
