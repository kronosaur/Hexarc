//	CComplexBinary.cpp
//
//	CComplexBinary class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_BINARY,				"binary")
const CString &CComplexBinary::GetTypename (void) const { return TYPENAME_BINARY; }

CComplexBinary::CComplexBinary (IByteStream &Stream, int iLength)

//	CComplexBinary constructor

	{
	ASSERT(iLength >= 0);

	if (iLength == 0)
		m_pData = NULL;
	else
		{
		char *pPos = new char [sizeof(DWORD) + iLength + 1];

		*(DWORD *)pPos = iLength;
		pPos += sizeof(DWORD);

		m_pData = pPos;

		Stream.Read(pPos, iLength);
		pPos += iLength;

		*pPos = '\0';
		}
	}

CComplexBinary::~CComplexBinary (void)

//	CComplexBinary destructor

	{
	if (m_pData)
		delete [] GetBuffer();
	}

void CComplexBinary::Append (CDatum dDatum)

//	Append
//
//	Appends data

	{
	const CString &sNewData = dDatum;
	if (sNewData.GetLength() == 0)
		return;

	//	Compute the new length

	int iOldLen = GetLength();
	int iNewLen = iOldLen + sNewData.GetLength();

	//	Allocate a new buffer

	char *pNewBuffer = new char [sizeof(DWORD) + iNewLen + 1];
	char *pPos = pNewBuffer;
	*(DWORD *)pPos = iNewLen;
	pPos += sizeof(DWORD);

	//	Copy the original data

	if (iOldLen)
		{
		utlMemCopy(m_pData, pPos, iOldLen);
		pPos += iOldLen;
		}

	//	Copy the new data

	utlMemCopy(sNewData.GetParsePointer(), pPos, sNewData.GetLength());
	pPos += sNewData.GetLength();

	//	NULL-terminator

	*pPos++ = '\0';

	//	Free our original buffer and swap

	if (m_pData)
		delete [] GetBuffer();

	m_pData = pNewBuffer + sizeof(DWORD);
	}

CString CComplexBinary::AsString (void) const

//	AsString
//
//	Returns the datum as a string

	{
	return CastCString();
	}

const CString &CComplexBinary::CastCString (void) const

//	CastCString
//
//	Casts to a string

	{
	if (m_pData == NULL)
		return NULL_STR;
	else
		return *(CString *)&m_pData;
	}

IComplexDatum *CComplexBinary::Clone (void) const

//	Clone
//
//	Creates a clone

	{
	CComplexBinary *pDest = new CComplexBinary;
	if (m_pData)
		{
		int iAllocLen = sizeof(DWORD) + GetLength() + 1;
		pDest->m_pData = new char [iAllocLen];
		utlMemCopy(m_pData, pDest->m_pData, iAllocLen);
		}

	return pDest;
	}

bool CComplexBinary::OnDeserialize (CDatum::ESerializationFormats iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Deserialize

	{
	if (m_pData)
		{
		delete [] GetBuffer();
		m_pData = NULL;
		}

	DWORD dwLength;
	Stream.Read(&dwLength, sizeof(DWORD));

	if (dwLength > 0)
		{
		CComplexBinary Temp(Stream, dwLength);
		m_pData = Temp.m_pData;
		Temp.m_pData = NULL;
		}

	return true;
	}

void CComplexBinary::OnSerialize (CDatum::ESerializationFormats iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize

	{
	DWORD dwLength = GetLength();

	Stream.Write(&dwLength, sizeof(DWORD));
	if (dwLength)
		Stream.Write(m_pData, dwLength);
	}

void CComplexBinary::TakeHandoff (CStringBuffer &Buffer)

//	TakeHandoff
//
//	Takes ownership

	{
	if (m_pData)
		delete [] GetBuffer();

	m_pData = Buffer.Handoff();
	}
