//	CMultipartData.cpp
//
//	CMultipartData class
//	Copyright (c) 2012 by Kronosaur Productions. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_BOUNDARY,					"boundary")
DECLARE_CONST_STRING(FIELD_CONTENT_DISPOSITION,			"content-disposition")
DECLARE_CONST_STRING(FIELD_CONTENT_TYPE,				"content-type")
DECLARE_CONST_STRING(FIELD_FORM_DATA,					"form-data")
DECLARE_CONST_STRING(FIELD_NAME,						"name")

DECLARE_CONST_STRING(MEDIA_MULTIPART_FORM,				"multipart/form-data")
DECLARE_CONST_STRING(MEDIA_TYPE_TEXT,					"text/plain")

bool CMultipartData::DecodeFromBuffer (const CString &sMediaType, const IMemoryBlock &Buffer)

//	DecodeFromBuffer
//
//	Decode from a buffer.

	{
	//	Extract the boundary from the media type

	CString sType;
	TSortMap<CString, CString> Fields;
	if (!CHTTPMessage::ParseHeaderValue(sMediaType, &sType, &Fields))
		return false;

	CString sBoundary;
	if (!Fields.Find(FIELD_BOUNDARY, &sBoundary))
		return false;

	//	Parse the buffer

	char *pPos = Buffer.GetPointer();
	char *pPosEnd = pPos + Buffer.GetLength();

	m_Parts.DeleteAll();

	//	Read the boundary

	if (pPos == pPosEnd || *pPos++ != '-')
		return false;

	if (pPos == pPosEnd || *pPos++ != '-')
		return false;

	char *pBoundary = sBoundary.GetParsePointer();
	char *pBoundaryEnd = pBoundary + sBoundary.GetLength();
	while (pPos < pPosEnd && pBoundary < pBoundaryEnd && *pPos == *pBoundary)
		{
		pPos++;
		pBoundary++;
		}

	if (pBoundary != pBoundaryEnd)
		return false;

	//	Loop over all parts

	while (pPos < pPosEnd)
		{
		//	Expect either CRLF or --

		if (*pPos == '\r')
			{
			pPos++;
			if (pPos == pPosEnd || *pPos++ != '\n')
				return false;

			//	Continue parsing
			}
		else if (*pPos == '-')
			{
			pPos++;
			if (pPos == pPosEnd || *pPos++ != '-')
				return false;

			//	Reached the end

			break;
			}
		else
			return false;

		//	Add a new part

		SPart *pNewPart = m_Parts.Insert();

		//	Parse headers

		while (true)
			{
			bool bDone;
			CString sField;
			CString sValue;

			if (!CHTTPMessage::ParseHeader(pPos, pPosEnd, &sField, &sValue, &pPos, &bDone))
				{
				if (bDone)
					break;
				return false;
				}

			if (strEquals(sField, FIELD_CONTENT_DISPOSITION))
				{
				CString sDisposition;
				TSortMap<CString, CString> Fields;

				if (!CHTTPMessage::ParseHeaderValue(sValue, &sDisposition, &Fields))
					return false;

				if (!strEquals(strToLower(sDisposition), FIELD_FORM_DATA))
					return false;

				if (!Fields.Find(FIELD_NAME, &pNewPart->sName))
					return false;
				}
			else if (strEquals(sField, FIELD_CONTENT_TYPE))
				{
				pNewPart->sContentType = sValue;
				}

			if (bDone)
				break;
			}

		//	Default type

		if (pNewPart->sContentType.IsEmpty())
			pNewPart->sContentType = MEDIA_TYPE_TEXT;

		//	Now we're at the content for this part. Loop until we hit the boundary

		if (!ParseToBoundary(pPos, pPosEnd, sBoundary, &pNewPart->sData, &pPos))
			return false;
		}
	
	return true;
	}

bool CMultipartData::EncodeToBuffer (IByteStream &Stream, DWORD dwOffset, DWORD dwSize) const

//	EncodeToBuffer
//
//	Encode to a buffer.

	{
	return false;
	}

const CString &CMultipartData::GetMediaType (void) const

//	GetMediaType
//
//	Return media type.

	{
	return MEDIA_MULTIPART_FORM;
	}

bool CMultipartData::ParseToBoundary (char *pPos, char *pPosEnd, const CString &sBoundary, CString *retsData, char **retpPos) const

//	ParseToBoundary
//
//	Assumes that pPos starts at the data and parses until we reach the given
//	boundary.
//
//	retsData is the full data.
//	retpPos points to the first character after the boundary.

	{
	enum EStates
		{
		stateParsing,
		stateExpectLF,
		stateExpectDash1,
		stateExpectDash2,
		stateExpectBoundary,
		stateDone,
		};

	char *pBoundary = sBoundary.GetParsePointer();
	char *pBoundaryEnd = pBoundary + sBoundary.GetLength();

	EStates iState = stateParsing;
	char *pStart = pPos;
	char *pEnd;

	while (iState != stateDone)
		{
		if (pPos == pPosEnd)
			return false;

		if (*pPos == '\r')
			{
			iState = stateExpectLF;
			pEnd = pPos;
			}
		else if (*pPos == '\n')
			iState = (iState == stateExpectLF ? stateExpectDash1 : stateParsing);
		else if (iState == stateParsing)
			;
		else
			{
			switch (iState)
				{
				case stateExpectLF:
					//	Since we check for LF above we only get here when the
					//	character is NOT LF.
					iState = stateParsing;
					break;

				case stateExpectDash1:
					if (*pPos == '-')
						iState = stateExpectDash2;
					else
						iState = stateParsing;
					break;

				case stateExpectDash2:
					if (*pPos == '-')
						{
						iState = stateExpectBoundary;
						pBoundary = sBoundary.GetParsePointer();
						}
					else
						iState = stateParsing;
					break;

				case stateExpectBoundary:
					if (*pPos == *pBoundary)
						{
						pBoundary++;
						if (pBoundary == pBoundaryEnd)
							iState = stateDone;
						}
					else
						iState = stateParsing;
					break;

				default:
					ASSERT(false);
					return false;
				}
			}

		pPos++;
		}

	//	Done

	*retsData = CString(pStart, (int)(pEnd - pStart));
	*retpPos = pPos;

	return true;
	}
