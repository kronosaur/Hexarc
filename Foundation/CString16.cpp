//	CString16.cpp
//
//	CString16 class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

CString16::CString16 (LPSTR pStr)

//	CString16 constructor

	{
	m_pString = CreateUTF16BufferFromUTF8(pStr, strLength(pStr));
	}

CString16::CString16 (const CString &sStr)

//	CString16 constructor

	{
	m_pString = CreateUTF16BufferFromUTF8(sStr, sStr.GetLength());
	}

CString16::CString16 (LPCTSTR pStr, int iLen)

//	CString16 constructor

	{
	//	Validate length

	if (iLen == -1)
		{
		if (pStr)
			iLen = CalcLength(pStr);
		else
			iLen = 0;
		}
	else if (iLen < 0)
		iLen = 0;

	//	Create

	m_pString = new TCHAR [iLen + 1];
	if (pStr)
		utlMemCopy(pStr, m_pString, iLen * sizeof(TCHAR));
	m_pString[iLen] = _T('\0');
	}

CString16::CString16 (LPCTSTR pStr, size_t iLen)

//	CString16 constructor

	{
	//	Validate length

	if (iLen == -1)
		{
		if (pStr)
			iLen = CalcLength(pStr);
		else
			iLen = 0;
		}
	else if (iLen < 0)
		iLen = 0;

	//	Create

	m_pString = new TCHAR [iLen + 1];
	if (pStr)
		utlMemCopy(pStr, m_pString, iLen * sizeof(TCHAR));
	m_pString[iLen] = _T('\0');
	}

CString16::CString16 (size_t iLen)

//	CString16 constructor

	{
	if (iLen <= 0)
		return;

	m_pString = new TCHAR [iLen + 1];
	m_pString[iLen] = _T('\0');
	}

CString16::~CString16 (void)

//	CString16 destructor

	{
	if (m_pString)
		delete [] m_pString;
	}

CString16 &CString16::operator= (const CString16 &sStr)

//	CString16 operator =

	{
	if (m_pString)
		delete [] m_pString;

	int iLen = sStr.GetLength();
	if (iLen > 0)
		{
		m_pString = new TCHAR [iLen + 1];
		utlMemCopy(sStr.m_pString, m_pString, iLen * sizeof(TCHAR));
		m_pString[iLen] = _T('\0');
		}
	else
		{
		m_pString = new TCHAR [1];
		*m_pString = _T('\0');
		}

	return *this;
	}

CString16 &CString16::operator= (LPSTR pStr)

//	CString16 operator =

	{
	if (m_pString)
		delete [] m_pString;

	m_pString = CreateUTF16BufferFromUTF8(pStr, -1);

	return *this;
	}

CString16 &CString16::operator= (const CString &sStr)

//	CString16 operator =

	{
	if (m_pString)
		delete [] m_pString;

	m_pString = CreateUTF16BufferFromUTF8(sStr, sStr.GetLength());

	return *this;
	}

int CString16::CalcLength (LPCTSTR pStr)

//	CalcLength
//
//	Returns the length of the given wide string

	{
	if (pStr == NULL)
		return 0;

	LPCTSTR pStart = pStr;
	while (*pStr != _T('\0'))
		pStr++;

	return (int)(pStr - pStart);
	}

LPTSTR CString16::CreateUTF16BufferFromUTF8 (LPSTR pStr, int iLen)

//	CreateUTF16BufferFromUTF8
//
//	Allocates a new UTF-16 buffer from a UTF-8 string
//	Both are null terminated.

	{
	if (iLen == -1)
		iLen = strLength(pStr);

	//	Handle NULL input

	if (iLen == 0)
		{
		LPTSTR pNewBuffer = new TCHAR [1];
		*pNewBuffer = _T('\0');
		return pNewBuffer;
		}

	//	We optimistically assume that the UTF-16 buffer
	//	will be twice the size of the UTF-8 buffer.

	int iNewBufferLen = iLen;
	LPTSTR pNewBuffer = new TCHAR [iNewBufferLen + 1];
	int iResult = ::MultiByteToWideChar(CP_UTF8, 0, pStr, iLen, pNewBuffer, iNewBufferLen);

	//	Deal with failure

	if (iResult == 0)
		{
		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
			delete [] pNewBuffer;

			//	Figure out how big the buffer should be and allocate appropriately
			//	And redo the conversion.

			iNewBufferLen = ::MultiByteToWideChar(CP_UTF8, 0, pStr, iLen, NULL, 0);
			if (iNewBufferLen == 0)
				{
				pNewBuffer = new TCHAR [1];
				*pNewBuffer = _T('\0');
				return pNewBuffer;
				}

			pNewBuffer = new TCHAR [iNewBufferLen + 1];
			iResult = ::MultiByteToWideChar(CP_UTF8, 0, pStr, iLen, pNewBuffer, iNewBufferLen);
			}
		else
			{
			*pNewBuffer = _T('\0');
			return pNewBuffer;
			}
		}

	//	Null-terminate the end

	ASSERT(iResult <= iNewBufferLen);
	pNewBuffer[iResult] = _T('\0');

	//	Done

	return pNewBuffer;
	}
