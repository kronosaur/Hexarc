//	CEsperBodyBuilder.cpp
//
//	CEsperBodyBuilder class
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#ifdef DEBUG
//#define DEBUG_BINARY_OBJECT
#endif

DECLARE_CONST_STRING(FIELD_BOUNDARY,					"boundary")
DECLARE_CONST_STRING(FIELD_CONTENT_DISPOSITION,			"content-disposition")
DECLARE_CONST_STRING(FIELD_CONTENT_TYPE,				"content-type")
DECLARE_CONST_STRING(FIELD_FORM_DATA,					"form-data")
DECLARE_CONST_STRING(FIELD_NAME,						"name")

DECLARE_CONST_STRING(MEDIA_TYPE_FORM_URL_ENCODED,		"application/x-www-form-urlencoded")
DECLARE_CONST_STRING(MEDIA_TYPE_JSON,					"application/json")
DECLARE_CONST_STRING(MEDIA_TYPE_JSON_REQUEST,			"application/jsonrequest")
DECLARE_CONST_STRING(MEDIA_TYPE_MULTIPART_FORM,			"multipart/form-data")
DECLARE_CONST_STRING(MEDIA_TYPE_HTML,					"text/html")
DECLARE_CONST_STRING(MEDIA_TYPE_TEXT,					"text/plain")
DECLARE_CONST_STRING(MEDIA_TYPE_TEXT_PREFIX,			"text/")

DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_FORM_URL_ENCODED,	"Error parsing form URL encoded.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_JSON,			"Error parsing JSON.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_MULTIPART,		"Error parsing MIME multipart/form-data.")
DECLARE_CONST_STRING(ERR_UNSUPPORTED_MEDIA_TYPE,		"Unsupported media type: %s.")
DECLARE_CONST_STRING(ERR_UNKNOWN_STATE,					"Unknown parse state.")
DECLARE_CONST_STRING(ERR_INVALID_MULTIPART_DATA,		"Error parsing multipart data: %s")

const int DEFAULT_SIZE =								64 * 1024;
const int MAX_IN_MEMORY_SIZE =							4 * 1024 * 1024;

CEsperBodyBuilder::CEsperBodyBuilder (void) : m_Body(DEFAULT_SIZE),
		m_iBodyRead(0),
		m_pStruct(NULL),
		m_pPartContent(NULL)

//	CEsperBodyBuilder constructor

	{
	}

void CEsperBodyBuilder::Append (void *pPos, int iLength)

//	Append
//
//	Append the data

	{
	//	We always need to keep track of how much of the body we've been given.

	m_iBodyRead += iLength;

	//	Append the data to the buffer, if necessary

	switch (m_iState)
		{
		case stateFormBuild:
		case stateJSONBuild:
		case stateTextBuild:

		case stateMultipartFirstBoundary:
		case stateMultipartHeader:
		case stateMultipartStartContent:
			m_Body.Write(pPos, iLength);
			break;
		}

	//	Parse based on state

	while (true)
		{
		switch (m_iState)
			{
			case stateMultipartFirstBoundary:
				if (ProcessMultipartFirstBoundary())
					continue;
				break;

			case stateMultipartHeader:
				if (ProcessMultipartHeader())
					continue;
				break;

			case stateMultipartStartContent:
				if (ProcessMultipartStartContent())
					continue;
				break;

			case stateMultipartMoreContent:
				if (ProcessMultipartMoreContent(pPos, iLength))
					continue;
				break;
			}

		break;
		}
	}

bool CEsperBodyBuilder::CreateMedia (IMediaType **retpBody)

//	CreateMedia
//
//	Creates the media buffer

	{
	//	We actually don't need the body, because this builder stored a datum for
	//	the result.

	*retpBody = new CRawMediaType;

	//	Generate the body based on our state

	switch (m_iState)
		{
		case stateFormBuild:
			{
			CString sBuffer(m_Body.GetPointer(), m_Body.GetLength());
			if (!CEsperInterface::ConvertFormURLEncodedToDatum(sBuffer, &m_dBody))
				{
				m_dBody = ERR_UNABLE_TO_PARSE_FORM_URL_ENCODED;
				m_bParseSuccess = false;
				return false;
				}
			break;
			}

		case stateJSONBuild:
			{
			m_Body.Seek(0);
			if (!CDatum::Deserialize(CDatum::formatJSON, m_Body, &m_dBody))
				{
				m_dBody = ERR_UNABLE_TO_PARSE_JSON;
				m_bParseSuccess = false;
				return false;
				}
			break;
			}

		case stateMultipartDone:
		case stateTextDone:
			//	m_dBody is already initialized
			break;

		case stateTextBuild:
			{
			CString sBuffer(m_Body.GetPointer(), m_Body.GetLength());
			m_dBody = sBuffer;
			break;
			}

		case stateError:
			{
			m_bParseSuccess = false;
			return false;
			}

		default:
			m_dBody = ERR_UNKNOWN_STATE;
			m_bParseSuccess = false;
			return false;
		}

	m_bParseSuccess = true;
	return true;
	}

bool CEsperBodyBuilder::CreateMultipartDatum (const CString &sPartType, char *pPos, char *pPosEnd, CDatum *retdData)

//	CreateMultipartDatum
//
//	Creates a datum from a multipart type.

	{
	int iDataLen = (int)(pPosEnd - pPos);
	if (strEquals(sPartType, MEDIA_TYPE_TEXT))
		{
		//	If the data starts with { then assume that it is a JSON
		//	encoded string. We need to do this because the current
		//	implementation of FormData (on JavaScript) does not support
		//	JSON natively (i.e., it does not set the media type).

		if (*pPos == '{')
			{
			CDatum dData;
			if (!CDatum::Deserialize(CDatum::formatJSON, CBuffer(pPos, iDataLen, false), retdData))
				return false;
			}

		//	Otherwise this is just a string.

		else
			{
			CString Data(pPos, iDataLen);
			if (!CDatum::CreateStringFromHandoff(Data, retdData))
				return false;
			}
		}
	else if (iDataLen > MAX_IN_MEMORY_SIZE)
		{
		CComplexBinaryFile *pBinaryBlob = new CComplexBinaryFile;
		pBinaryBlob->Append(CBuffer(pPos, iDataLen, false));
		*retdData = CDatum(pBinaryBlob);
		}
	else
		{
		if (!CDatum::CreateBinary(CBuffer(pPos, iDataLen, false), iDataLen, retdData))
			return false;
		}

	return true;
	}

CEsperBodyBuilder::EParseResults CEsperBodyBuilder::FindMultipartBoundary (char *pPos, char *pPosEnd, char **retpBoundaryStart, CString *retsPartialBoundary) const

//	FindMultipartBoundary
//
//	Looks for a boundary marker in the given region. Returns one of the 
//	following:
//
//	resultNotFound: We did not find the boundary
//	resultFoundBoundary: We found a boundary
//	resultFoundLastBoundary: We found the last boundary
//	resultError: Fatal error parsing
//
//	NOTE: The start of the boundary includes the leading CRLF

	{
	enum EParseStates 
		{
		stateStart,
		stateFoundCR,
		stateFoundLF,
		stateFoundDash1,
		stateFoundDash2,
		stateFoundBoundaryText,
		stateFoundEndCR,
		stateFoundEndDash1,
		stateError,
		};

	char *pBoundary = NULL;
	EParseStates iState = stateStart;
	while (pPos < pPosEnd)
		{
		switch (iState)
			{
			case stateStart:
				if (*pPos == '\r')
					{
					iState = stateFoundCR;
					pBoundary = pPos;
					}
				break;

			case stateFoundCR:
				if (*pPos == '\n')
					iState = stateFoundLF;
				else if (*pPos == '\r')
					{
					iState = stateFoundCR;
					pBoundary = pPos;
					}
				else
					{
					iState = stateStart;
					pBoundary = NULL;
					}
				break;

			case stateFoundLF:
				if (*pPos == '-')
					iState = stateFoundDash1;
				else if (*pPos == '\r')
					{
					iState = stateFoundCR;
					pBoundary = pPos;
					}
				else
					{
					iState = stateStart;
					pBoundary = NULL;
					}
				break;

			case stateFoundDash1:
				if (*pPos == '-')
					iState = stateFoundDash2;
				else if (*pPos == '\r')
					{
					iState = stateFoundCR;
					pBoundary = pPos;
					}
				else
					{
					iState = stateStart;
					pBoundary = NULL;
					}
				break;

			case stateFoundDash2:
				{
				char *pTest = pPos;
				char *pSrc = m_sBoundary.GetParsePointer();
				while (pTest < pPosEnd && *pSrc != '\0')
					{
					if (*pTest != *pSrc)
						break;

					pTest++;
					pSrc++;
					}

				if (*pSrc == '\0')
					{
					iState = stateFoundBoundaryText;
					pPos = pTest - 1;
					}
				else if (pTest == pPosEnd)
					{
					pPos = pPosEnd - 1;
					}
				else if (*pPos == '\r')
					{
					pBoundary = pPos;
					iState = stateFoundCR;
					}
				else
					{
					iState = stateStart;
					pBoundary = NULL;
					}

				break;
				}

			case stateFoundBoundaryText:
				{
				if (*pPos == '\r')
					iState = stateFoundEndCR;
				else if (*pPos == '-')
					iState = stateFoundEndDash1;
				else
					return resultError;
				break;
				}

			case stateFoundEndCR:
				{
				if (*pPos == '\n')
					{
					if (retpBoundaryStart)
						*retpBoundaryStart = pBoundary;
					return resultFoundBoundary;
					}
				else
					return resultError;
				break;
				}

			case stateFoundEndDash1:
				{
				if (*pPos == '-')
					{
					if (retpBoundaryStart)
						*retpBoundaryStart = pBoundary;
					return resultFoundLastBoundary;
					}
				else
					return resultError;
				}
			}

		pPos++;
		}

	if (retsPartialBoundary)
		{
		if (pBoundary)
			*retsPartialBoundary = CString(pBoundary, (int)(pPosEnd - pBoundary));
		else
			*retsPartialBoundary = NULL_STR;
		}

	return resultNotFound;
	}

CDatum CEsperBodyBuilder::GetRawBody (void)

//	GetRawBody
//
//	Returns the raw body as a string

	{
	//	Generate the body based on our state

	switch (m_iState)
		{
		case stateFormBuild:
		case stateJSONBuild:
		case stateTextBuild:
			return CDatum(CString(m_Body.GetPointer(), m_Body.GetLength()));

		case stateMultipartDone:
		case stateTextDone:
			return m_dBody;

		case stateError:
			return CDatum();

		default:
			return CDatum();
		}
	}

void CEsperBodyBuilder::Init (const CString &sMediaType)

//	Init
//
//	Initializes

	{
	m_sMediaType = sMediaType;
	m_iBodyRead = 0;
	m_Body.SetLength(0);
	m_iParsePos = 0;
	m_dBody = CDatum();
	m_bParseSuccess = false;

	//	Figure out how to parse based on our media type

	CString sParsedMediaType;
	TSortMap<CString, CString> Fields;

	if (sMediaType.IsEmpty())
		{
		m_iState = stateTextDone;
		m_bParseSuccess = true;
		}

	else if (!CHTTPMessage::ParseHeaderValue(m_sMediaType, &sParsedMediaType, &Fields))
		{
		m_dBody = strPattern(ERR_UNSUPPORTED_MEDIA_TYPE, m_sMediaType);
		m_iState = stateError;
		}

	else if (strEquals(sParsedMediaType, MEDIA_TYPE_FORM_URL_ENCODED))
		m_iState = stateFormBuild;

	else if (strEquals(sParsedMediaType, MEDIA_TYPE_JSON))
		m_iState = stateJSONBuild;

	else if (strEquals(sParsedMediaType, MEDIA_TYPE_MULTIPART_FORM))
		{
		if (!Fields.Find(FIELD_BOUNDARY, &m_sBoundary))
			{
			m_dBody = ERR_UNABLE_TO_PARSE_MULTIPART;
			m_iState = stateError;
			return;
			}

		m_pStruct = new CComplexStruct;
		m_dBody = CDatum(m_pStruct);
		m_iState = stateMultipartFirstBoundary;
		}

	else if (strStartsWith(sParsedMediaType, MEDIA_TYPE_TEXT_PREFIX))
		m_iState = stateTextBuild;

	else
		{
		m_dBody = strPattern(ERR_UNSUPPORTED_MEDIA_TYPE, sParsedMediaType);
		m_iState = stateError;
		}
	}

void CEsperBodyBuilder::Mark (void)

//	Mark
//
//	Mark data in use

	{
	m_dBody.Mark();
	m_dPartContent.Mark();
	}

CEsperBodyBuilder::EParseResults CEsperBodyBuilder::ParseMultipartBoundary (void)

//	ParseMultipartBoundary
//
//	Parses for a boundary and returns TRUE if we find one. The parse position
//	is left on the first character after the boundary. If we DON'T find the
//	boundary, then the parse position is unchanged (because we might have
//	only gotten a partial boundary).

	{
	char *pPos = m_Body.GetPointer() + m_iParsePos;
	char *pPosEnd = m_Body.GetPointer() + m_Body.GetLength();

	//	First see if we have a boundary right at the beginning.

	if (*pPos == '-')
		{
		CString sStartBoundary = strPattern("--%s\r\n", m_sBoundary);
		int iLength = (int)(pPosEnd - pPos);
		if (iLength < sStartBoundary.GetLength())
			return resultNeedMoreData;

		if (strEquals(CString(pPos, sStartBoundary.GetLength()), sStartBoundary))
			{
			m_iParsePos += sStartBoundary.GetLength();
			return resultFoundBoundary;
			}
		}

	//	Otherwise, we expect some text (which we can ignore) ending in a CRLF
	//	and then the boundary.

	char *pBoundary;
	EParseResults iResult = FindMultipartBoundary(pPos, pPosEnd, &pBoundary);

	switch (iResult)
		{
		case resultFoundBoundary:
		case resultFoundLastBoundary:
			{
			char *pStart = pPos;
			pPos = pBoundary + m_sBoundary.GetLength() + 4;
			m_iParsePos += (int)(pPos - pStart);
			return iResult;
			}

		case resultNotFound:
			return resultNeedMoreData;

		default:
			return iResult;
		}
	}

CEsperBodyBuilder::EParseResults CEsperBodyBuilder::ParseMultipartHeader (CString *retsField, CString *retsValue)

//	ParseMultipartHeader
//
//	Parses a header line. If we find one, we leave the parse pointer at the 
//	beginning of the next line. Otherwise, we leave the parse pointer alone.
//
//	If we find an empty line, then it means we're done with headers and return
//	resultNotFound.

	{
	char *pPos = m_Body.GetPointer() + m_iParsePos;
	char *pPosEnd = m_Body.GetPointer() + m_Body.GetLength();
	char *pStart = pPos;

	//	Init. This needs to be valid even if we return FALSE.

	bool bHeadersDone = false;

	//	If we hit the end of the stream, then we need more data

	if (pPos >= pPosEnd)
		return resultNeedMoreData;

	//	If we hit CRLF, then no more headers

	else if (pPos < pPosEnd && *pPos == '\r')
		{
		pPos++;
		if (pPos == pPosEnd)
			return resultNeedMoreData;

		if (*pPos == '\n')
			pPos++;

		m_iParsePos += (int)(pPos - pStart);
		return resultNotFound;
		}

	//	Skip leading whitespace

	while (pPos < pPosEnd && (*pPos == ' ' || *pPos == '\t'))
		pPos++;

	//	Parse the field name

	char *pFieldStart = pPos;
	while (pPos < pPosEnd && *pPos != ':' && *pPos != ' ' && *pPos != '\t')
		pPos++;

	CString sField(pFieldStart, (int)(pPos - pFieldStart));

	//	Find the colon

	while (pPos < pPosEnd && *pPos != ':')
		pPos++;

	//	If we don't find the colon, then we need more data

	if (pPos == pPosEnd)
		return resultNeedMoreData;

	pPos++;

	//	Skip any whitespace

	while (pPos < pPosEnd && (*pPos == ' ' || *pPos == '\t'))
		pPos++;

	//	Accumulate the value

	bool bFoundLineEnd = false;
	char *pValueStart = pPos;
	while (pPos < pPosEnd)
		{
		//	If our next two characters are CRLF...

		if (pPos[0] == '\r' && pPos + 1 < pPosEnd && pPos[1] == '\n')
			{
			//	If the character after that is whitepace, then just keep
			//	going...

			if (pPos + 2 < pPosEnd && (pPos[2] == ' ' || pPos[2] == '\t'))
				pPos += 3;

			//	Otherwise, we've reached the end, so we break out

			else
				{
				bFoundLineEnd = true;
				break;
				}
			}
		else
			pPos++;
		}

	//	If we did not find the end of the header line, then we need to
	//	get more data.

	if (!bFoundLineEnd)
		return resultNeedMoreData;

	//	Value

	CString sValue(pValueStart, (int)(pPos - pValueStart));

	//	Swallow the CRLF that terminates the header

	if (pPos + 1 < pPosEnd)
		pPos += 2;
	else
		return resultNeedMoreData;

	//	Done

	m_iParsePos += (int)(pPos - pStart);
	*retsField = strToLower(sField);
	*retsValue = sValue;
	return resultFound;
	}

bool CEsperBodyBuilder::ProcessMultipartFirstBoundary (void)

//	ProcessMultipartFirstBoundary
//
//	Handles m_iState == stateMultipartFirstBoundary

	{
	switch (ParseMultipartBoundary())
		{
		case resultNeedMoreData:
			return false;

		case resultFoundBoundary:
			ResetMultipartTemps();
			m_iState = stateMultipartHeader;
			return true;

		case resultFoundLastBoundary:
			m_dBody = CDatum();
			m_iState = stateMultipartDone;
			return false;

		case resultError:
			m_dBody = ERR_UNABLE_TO_PARSE_MULTIPART;
			m_iState = stateError;
			return false;

		default:
			ASSERT(false);
			return false;
		}
	}

bool CEsperBodyBuilder::ProcessMultipartHeader (void)

//	ProcessMultipartHeader
//
//	Handles m_iState == stateMultipartHeader

	{
	CString sField;
	CString sValue;

	switch (ParseMultipartHeader(&sField, &sValue))
		{
		case resultNeedMoreData:
			return false;

		case resultFound:
			{
			if (strEquals(sField, FIELD_CONTENT_DISPOSITION))
				{
				CString sDisposition;
				TSortMap<CString, CString> Fields;

				if (!CHTTPMessage::ParseHeaderValue(sValue, &sDisposition, &Fields))
					{
					m_dBody = ERR_UNABLE_TO_PARSE_MULTIPART;
					m_iState = stateError;
					return false;
					}

				if (!strEquals(strToLower(sDisposition), FIELD_FORM_DATA))
					{
					m_dBody = ERR_UNABLE_TO_PARSE_MULTIPART;
					m_iState = stateError;
					return false;
					}

				if (!Fields.Find(FIELD_NAME, &m_sPartName))
					{
					m_dBody = ERR_UNABLE_TO_PARSE_MULTIPART;
					m_iState = stateError;
					return false;
					}
				}
			else if (strEquals(sField, FIELD_CONTENT_TYPE))
				{
				m_sPartType = sValue;
				}

			return true;
			}

		case resultNotFound:
			m_iState = stateMultipartStartContent;
			return true;

		case resultError:
			m_dBody = ERR_UNABLE_TO_PARSE_MULTIPART;
			m_iState = stateError;
			return false;

		default:
			ASSERT(false);
			return false;
		}
	}

bool CEsperBodyBuilder::ProcessMultipartMoreContent (void *pPos, int iLength)

//	ProcessMultipartMoreContent
//
//	Handles m_iState == stateMultipartMoreContent

	{
	//	Write to the part buffer, but remember where we left off because we
	//	only start parsing at the end.

	int iParsePos = m_PartContent.GetLength();
	m_PartContent.Write(pPos, iLength);

	//	If we have a partial boundary then back up a little so we can include it

	if (m_sPartialBoundary)
		iParsePos -= m_sPartialBoundary.GetLength();

	char *pParsePos = m_PartContent.GetPointer() + iParsePos;
	char *pPosEnd = m_PartContent.GetPointer() + m_PartContent.GetLength();

	//	Look for the start of the boundary in the data

	char *pBoundary;
	EParseResults iResult = FindMultipartBoundary(pParsePos, pPosEnd, &pBoundary, &m_sPartialBoundary);

	switch (iResult)
		{
		case resultFoundBoundary:
		case resultFoundLastBoundary:
			{
			//	Create a datum for the whole thing.

			CDatum dData;
			if (m_pPartContent)
				{
#ifdef DEBUG_BINARY_OBJECT
				printf("Appending %d bytes to binary object.\n", (int)(pBoundary - m_PartContent.GetPointer()));
				printf("Done with binary object.\n");
#endif
				m_pPartContent->Append(CBuffer(m_PartContent.GetPointer(), (int)(pBoundary - m_PartContent.GetPointer()), false));
				dData = m_dPartContent;
				}
			else
				{
				if (!CreateMultipartDatum(m_sPartType, m_PartContent.GetPointer(), pBoundary, &dData))
					{
					m_dBody = strPattern(ERR_INVALID_MULTIPART_DATA, dData.AsString());
					m_iState = stateError;
					return false;
					}
				}

			m_dBody.SetElement(m_sPartName, dData);
			char *pNewPart = pBoundary + m_sBoundary.GetLength() + 6;

			if (iResult == resultFoundLastBoundary)
				{
				ResetMultipartTemps();
				m_iState = stateMultipartDone;
				return false;
				}
			else
				{
				m_Body.SetLength(0);
				m_Body.Write(pNewPart, (int)(pPosEnd - pNewPart));
				m_iParsePos = 0;

				ResetMultipartTemps();
				m_iState = stateMultipartHeader;
				return true;
				}
			}

		case resultNotFound:
			{
			if (m_pPartContent)
				{
				if (m_sPartialBoundary.IsEmpty())
					{
#ifdef DEBUG_BINARY_OBJECT
					printf("Appending %d bytes to binary object.\n", m_PartContent.GetLength());
#endif
					m_pPartContent->Append(m_PartContent);
					m_PartContent.SetLength(0);
					}
				}
			else if (m_PartContent.GetLength() > MAX_IN_MEMORY_SIZE 
						&& !strEquals(m_sPartType, MEDIA_TYPE_TEXT)
						&& m_sPartialBoundary.IsEmpty())
				{
#ifdef DEBUG_BINARY_OBJECT
				printf("Creating binary object with %d bytes.\n", m_PartContent.GetLength());
#endif
				m_pPartContent = new CComplexBinaryFile;
				m_pPartContent->Append(m_PartContent);
				m_dPartContent = CDatum(m_pPartContent);
				m_PartContent.SetLength(0);
				}

			return false;
			}

		case resultError:
			{
			m_dBody = ERR_UNABLE_TO_PARSE_MULTIPART;
			m_iState = stateError;
			return false;
			}

		default:
			ASSERT(false);
			return false;
		}
	}

bool CEsperBodyBuilder::ProcessMultipartStartContent (void)

//	ProcessMultipartStartContent
//
//	Handles m_iState == stateMultipartHeader

	{
	char *pPos = m_Body.GetPointer() + m_iParsePos;
	char *pPosEnd = m_Body.GetPointer() + m_Body.GetLength();
	char *pStart = pPos;

	//	Look for the start of the boundary in the data

	char *pBoundary;
	EParseResults iResult = FindMultipartBoundary(pPos, pPosEnd, &pBoundary, &m_sPartialBoundary);
	switch (iResult)
		{
		case resultFoundBoundary:
		case resultFoundLastBoundary:
			{
			CDatum dData;
			if (!CreateMultipartDatum(m_sPartType, pPos, pBoundary, &dData))
				{
				m_dBody = strPattern(ERR_INVALID_MULTIPART_DATA, dData.AsString());
				m_iState = stateError;
				return false;
				}

			m_dBody.SetElement(m_sPartName, dData);
			pPos = pBoundary + m_sBoundary.GetLength() + 6;
			m_iParsePos += (int)(pPos - pStart);
			if (iResult == resultFoundLastBoundary)
				{
				m_iState = stateMultipartDone;
				return false;
				}
			else
				{
				ResetMultipartTemps();
				m_iState = stateMultipartHeader;
				return true;
				}
			}

		case resultNotFound:
			{
			m_PartContent.Write(pPos, (int)(pPosEnd - pPos));
			m_Body.SetLength(0);
			m_iParsePos = 0;
			m_iState = stateMultipartMoreContent;
			return false;
			}

		case resultError:
			{
			m_dBody = ERR_UNABLE_TO_PARSE_MULTIPART;
			m_iState = stateError;
			return false;
			}

		default:
			ASSERT(false);
			return false;
		}
	}

void CEsperBodyBuilder::ResetMultipartTemps (void)

//	ResetMultipartTemps
//
//	Resets the temporaries used to build a part in a multipart object

	{
	m_sPartName = NULL_STR;
	m_sPartType = MEDIA_TYPE_TEXT;
	m_PartContent.SetLength(0);
	m_pPartContent = NULL;
	m_dPartContent = CDatum();
	}
