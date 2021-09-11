//	CEsperMultipartParser.cpp
//
//	CEsperMultipartParser class
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_BOUNDARY,					"boundary")
DECLARE_CONST_STRING(FIELD_CONTENT_DISPOSITION,			"content-disposition")
DECLARE_CONST_STRING(FIELD_CONTENT_TYPE,				"content-type")
DECLARE_CONST_STRING(FIELD_FORM_DATA,					"form-data")
DECLARE_CONST_STRING(FIELD_NAME,						"name")

DECLARE_CONST_STRING(MEDIA_MULTIPART_FORM,				"multipart/form-data")
DECLARE_CONST_STRING(MEDIA_TYPE_TEXT,					"text/plain")

const int MAX_IN_MEMORY_SIZE =							4 * 1024 * 1024;

CEsperMultipartParser::~CEsperMultipartParser (void)

//	CEsperMultipartParser destructor

	{
	if (m_pResult)
		delete m_pResult;
	}

bool CEsperMultipartParser::ParseAsDatum (CDatum *retdBody)

//	ParseAsDatum
//
//	Parse into a datum structure

	{
	//	Extract the boundary from the media type

	CString sType;
	TSortMap<CString, CString> Fields;
	if (!CHTTPMessage::ParseHeaderValue(m_sMediaType, &sType, &Fields))
		return false;

	CString sBoundary;
	if (!Fields.Find(FIELD_BOUNDARY, &sBoundary))
		return false;

	//	Prepare the result

	if (m_pResult)
		delete m_pResult;
	m_pResult = new CComplexStruct;

	//	Parse the buffer

	char *pPos = m_Block.GetPointer();
	char *pPosEnd = pPos + m_Block.GetLength();

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

		//	Parse headers to figure out the name and type

		CString sPartName;
		CString sPartType;
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

				if (!Fields.Find(FIELD_NAME, &sPartName))
					return false;
				}
			else if (strEquals(sField, FIELD_CONTENT_TYPE))
				{
				sPartType = sValue;
				}

			if (bDone)
				break;
			}

		//	Default type

		if (sPartType.IsEmpty())
			sPartType = MEDIA_TYPE_TEXT;

		//	Now we're at the content for this part. Loop until we hit the boundary

		CDatum dData;
		if (!ParseToBoundary(pPos, pPosEnd, sBoundary, sPartType, &dData, &pPos))
			return false;

		//	Add to the structure

		m_pResult->SetElement(sPartName, dData);
		}

	//	If we get this far, then we're done and we hand off the result to our
	//	caller.

	*retdBody = CDatum(m_pResult);
	m_pResult = NULL;
	
	return true;
	}

bool CEsperMultipartParser::ParseToBoundary (char *pPos, char *pPosEnd, const CString &sBoundary, const CString &sPartType, CDatum *retdData, char **retpPos) const

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

	//	Remember our current position

	*retpPos = pPos;

	//	Done. Convert to a datum.

	int iDataLen = (int)(pEnd - pStart);
	if (strEquals(sPartType, MEDIA_TYPE_TEXT))
		{
		//	If the data starts with { then assume that it is a JSON
		//	encoded string. We need to do this because the current
		//	implementation of FormData (on JavaScript) does not support
		//	JSON natively (i.e., it does not set the media type).

		if (*pStart == '{')
			{
			CBuffer Buffer(pStart, iDataLen, false);
			CDatum dData;
			if (!CDatum::Deserialize(CDatum::EFormat::JSON, Buffer, retdData))
				return false;
			}

		//	Otherwise this is just a string.

		else
			{
			CString Data(pStart, iDataLen);
			if (!CDatum::CreateStringFromHandoff(Data, retdData))
				return false;
			}
		}
	else if (iDataLen > MAX_IN_MEMORY_SIZE)
		{
		CBuffer Buffer(pStart, iDataLen, false);
		CComplexBinaryFile *pBinaryBlob = new CComplexBinaryFile;
		pBinaryBlob->Append(Buffer);
		*retdData = CDatum(pBinaryBlob);
		}
	else
		{
		CBuffer Buffer(pStart, iDataLen, false);
		if (!CDatum::CreateBinary(Buffer, iDataLen, retdData))
			return false;
		}

	return true;
	}
