//	CHTTPMessage.cpp
//
//	CHTTPMessage class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_HEADER_CONS,					"%s: %s\r\n")
DECLARE_CONST_STRING(STR_REQUEST_LINE,					"%s %s HTTP/1.1\r\n")
DECLARE_CONST_STRING(STR_EMPTY_PATH,					"/")
DECLARE_CONST_STRING(STR_HTTP,							"HTTP")
DECLARE_CONST_STRING(STR_HTTP_VERSION,					"HTTP/1.1")
DECLARE_CONST_STRING(STR_RESPONSE_LINE,					"HTTP/1.1 %d %s\r\n")
DECLARE_CONST_STRING(STR_CHUNK_CONS,					"%X\r\n")

DECLARE_CONST_STRING(HEADER_ACCEPT_ENCODING,			"accept-encoding")
DECLARE_CONST_STRING(HEADER_CONTENT_ENCODING,			"content-encoding")
DECLARE_CONST_STRING(HEADER_CONTENT_LENGTH,				"content-length")
DECLARE_CONST_STRING(HEADER_CONTENT_TYPE,				"content-type")
DECLARE_CONST_STRING(HEADER_COOKIE,						"cookie")
DECLARE_CONST_STRING(HEADER_HOST,						"host")
DECLARE_CONST_STRING(HEADER_TRANSFER_ENCODING,			"transfer-encoding")

DECLARE_CONST_STRING(ENCODING_CHUNKED,					"chunked")
DECLARE_CONST_STRING(ENCODING_GZIP,						"gzip")
DECLARE_CONST_STRING(ENCODING_IDENTITY,					"identity")

DECLARE_CONST_STRING(MEDIA_WWW_FORM_URL_ENCODED,		"application/x-www-form-urlencoded")

CString GetToken (char *pPos, char *pEndPos, char chDelimiter, char **retpPos);

CHTTPMessage::CHTTPMessage (void) : 
		m_iType(typeUnknown),
		m_bHTTP11(true),
		m_pBody(NULL),
		m_pBodyBuilder(NULL)

//	CHTTPMessage constructor

	{
	InitFromPartialBufferReset();
	}

CHTTPMessage::~CHTTPMessage (void)

//	CHTTPMessage destructor

	{
	if (m_pBodyBuilder)
		m_pBodyBuilder->Delete();

	SetBody(NULL);
	}

void CHTTPMessage::AddHeader (const CString &sField, const CString &sValue)

//	AddHeader
//
//	Adds a header to the message

	{
	SHeader *pHeader = m_Headers.Insert();

	//	Fields are case-insensitive, so we always store in lowercase to speed up
	//	compares.
	pHeader->sField = strToLower(sField);
	pHeader->sValue = sValue;
	}

void CHTTPMessage::AddHeader (const CString &sField, const CDateTime &Value)

//	AddHeader
//
//	Adds a date/time header

	{
	AddHeader(sField, Value.FormatIMF());
	}

CString CHTTPMessage::DebugGetInitState (void) const

//	DebugGetInitState
//
//	Returns a debug string indicating the current parse state.

	{
	switch (m_iState)
		{
		case stateStart:
			return CString("stateStart");

		case stateHeaders:
			return CString("stateHeaders");

		case stateBody:
			return strPattern("stateBody: %d bytes", m_pBodyBuilder->GetLength());

		case stateChunk:
			return strPattern("stateChunk: body = %d bytes; %d bytes left in chunk.", m_pBodyBuilder->GetLength(), m_iChunkLeft);

		case stateDone:
			return CString("stateDone");

		default:
			return strPattern("Unknown state: %d.", (int)m_iState);
		}
	}

bool CHTTPMessage::Encode (EContentEncodingTypes iEncoding)

//	Encode
//
//	Encodes

	{
	if (m_pBody)
		m_pBody->EncodeContent(iEncoding);

	return true;
	}

bool CHTTPMessage::FindHeader (const CString &sField, CString *retsValue) const

//	FindHeader
//
//	Looks for the given header and returns its value

	{
	int i;

	CString sFieldLower = strToLower(sField);

	for (i = 0; i < m_Headers.GetCount(); i++)
		{
		if (strEquals(m_Headers[i].sField, sFieldLower))
			{
			if (retsValue)
				*retsValue = m_Headers[i].sValue;

			return true;
			}
		}

	return false;
	}

DWORD CHTTPMessage::GetBodySize (void) const

//	GetBodySize
//
//	Returns the current size of the body in bytes. This is valid during parsing.

	{
	if (m_pBody) 
		return GetBodyBuffer().GetLength(); 

	else if (m_iState != stateDone && m_pBodyBuilder)
		return m_pBodyBuilder->GetLength();

	else 
		return 0;
	}

CString CHTTPMessage::GetCookie (const CString &sKey) const

//	GetCookie
//
//	Returns the given cookie (or NULL_STR)

	{
	int i;

	//	Look over all cookie headers

	for (i = 0; i < m_Headers.GetCount(); i++)
		if (strEquals(m_Headers[i].sField, HEADER_COOKIE))
			{
			TSortMap<CString, CString> Cookies;

			ParseCookies(m_Headers[i].sValue, &Cookies);

			CString sValue;
			if (Cookies.Find(sKey, &sValue))
				return sValue;
			}

	return NULL_STR;
	}

EContentEncodingTypes CHTTPMessage::GetDefaultEncoding (void) const

//	GetDefaultEncoding
//
//	Returns the default encoding based on the media type

	{
	if (m_pBody == NULL
			|| m_pBody->GetMediaBuffer().IsEmpty())
		return http_encodingIdentity;

	return IMediaType::GetDefaultEncodingType(m_pBody->GetMediaType());
	}

CString CHTTPMessage::GetRequestedHost (void) const

//	GetRequestedHost
//
//	Returns the host that the message is requesting

	{
	CString sHost;

	//	Get the host from the URL, if it's there.

	if (!urlParse(m_sURL, NULL, &sHost, NULL))
		return NULL_STR;

	if (!sHost.IsEmpty())
		return sHost;

	//	Otherwise, get it from the headers

	if (!FindHeader(HEADER_HOST, &sHost))
		return NULL_STR;

	return sHost;
	}

CString CHTTPMessage::GetRequestedURL (CString *retsFullURL) const

//	GetRequestedURL
//
//	Returns the URL that the message is requesting.
//	(URL is always absolute, starting with a leading '/', but has no host)

	{
	CString sPath;

	//	Parse the URL

	if (!urlParse(m_sURL, NULL, NULL, &sPath))
		return NULL_STR;

	if (retsFullURL)
		*retsFullURL = m_sURL;

	return sPath;
	}

bool CHTTPMessage::InitFromBuffer (const IMemoryBlock &Buffer, bool bNoBody)

//	InitFromBuffer
//
//	This function parses the given buffer into a message. We expect the
//	full message to be in the buffer.

	{
	InitFromPartialBufferReset();
	if (!InitFromPartialBuffer(Buffer, bNoBody))
		return false;

	//	Success only if we're done parsing

	return (m_iState == stateDone);
	}

bool CHTTPMessage::InitFromPartialBuffer (const IMemoryBlock &Buffer, bool bNoBody)

//	InitFromPartialBuffer
//
//	This function parses the given buffer into a message. We can call this 
//	multiple times with pieces of the full message as long as the pieces are
//	contiguous and in proper order.
//
//	We assume that InitFromPartialBufferReset has been called.

	{
	char *pPos;
	char *pEndPos;

	if (m_pBodyBuilder == NULL)
		InitFromPartialBufferReset();

	//	If we have left overs, prepend it to the new buffer.

	CBuffer Temp;
	if (!m_sLeftOver.IsEmpty())
		{
		Temp.Write(m_sLeftOver);
		Temp.Write(Buffer.GetPointer(), Buffer.GetLength());
		pPos = Temp.GetPointer();
		pEndPos = Temp.GetPointer() + Temp.GetLength();

		m_sLeftOver = NULL_STR;
		}

	//	Otherwise, just parse the buffer

	else
		{
		pPos = Buffer.GetPointer();
		pEndPos = Buffer.GetPointer() + Buffer.GetLength();
		}

	while (pPos < pEndPos && m_iState != stateDone)
		{
		switch (m_iState)
			{
			case stateStart:
				{
				//	Remember the original position in case we need to back-track.

				char *pOriginalPos = pPos;

				//	Get the first token. If we don't find the delimiter, then we 
				//	need more data.

				CString sToken;
				if (!ParseToken(pPos, pEndPos, ' ', &pPos, &sToken))
					{
					m_sLeftOver = CString(pOriginalPos, pEndPos - pOriginalPos);
					return true;
					}

				//	If this starts with HTTP then this is a version (which means that
				//	this is a response).

				if (strStartsWith(sToken, STR_HTTP))
					{
					m_iType = typeResponse;
					m_sVersion = sToken;
					m_bHTTP11 = strEquals(m_sVersion, STR_HTTP_VERSION);

					//	Parse the status code

					CString sStatus;
					if (!ParseToken(pPos, pEndPos, ' ', &pPos, &sStatus))
						{
						m_sLeftOver = CString(pOriginalPos, pEndPos - pOriginalPos);
						return true;
						}

					m_dwStatusCode = strToInt(sStatus, 0);

					//	Parse the message

					if (!ParseToken(pPos, pEndPos, '\r', &pPos, &m_sStatusMsg))
						{
						m_sLeftOver = CString(pOriginalPos, pEndPos - pOriginalPos);
						return true;
						}

					m_sMethod = NULL_STR;
					m_sURL = NULL_STR;
					}

				//	Otherwise, this is a request

				else
					{
					m_iType = typeRequest;
					m_sMethod = sToken;

					//	Parse the URL

					if (!ParseToken(pPos, pEndPos, ' ', &pPos, &m_sURL))
						{
						m_sLeftOver = CString(pOriginalPos, pEndPos - pOriginalPos);
						return true;
						}

					//	Parse the version

					if (!ParseToken(pPos, pEndPos, '\r', &pPos, &m_sVersion))
						{
						m_sLeftOver = CString(pOriginalPos, pEndPos - pOriginalPos);
						return true;
						}

					m_bHTTP11 = strEquals(m_sVersion, STR_HTTP_VERSION);
					m_sStatusMsg = NULL_STR;
					m_dwStatusCode = 0;
					}

				//	Reset everything

				m_Headers.DeleteAll();
				SetBody(NULL);

				m_iState = stateHeaders;
				break;
				}

			case stateHeaders:
				{
				//	ParseHeader returns FALSE if we could not parse a header. If bHeadersDone
				//	is TRUE, the it means we're at the end. Otherwise, it means we need more
				//	data.

				CString sField;
				CString sValue;
				bool bHeadersDone;
				while (ParseHeader(pPos, pEndPos, &sField, &sValue, &pPos, &bHeadersDone))
					//	LATER: ParseHeader returns lowercased fields, but AddHeader
					//	wastefully lowercases again.
					AddHeader(sField, sValue);

				//	If we have no more headers, then see if we have a body
				//	(otherwise we stay in this state and parse more headers).

				if (bHeadersDone)
					{
					//	Some messages never have a body

					CString sEncoding;
					CString sLength;

					if (bNoBody 
							|| (m_dwStatusCode >= 100 && m_dwStatusCode < 200)
							|| (m_dwStatusCode == 204) || (m_dwStatusCode == 304))
						{
						m_pBodyBuilder->Init(NULL_STR);
						m_iState = stateDone;
						}

					//	If we have a non-identity transfer encoding, then we have 
					//	a body.

					else if (FindHeader(HEADER_TRANSFER_ENCODING, &sEncoding)
							&& !strEquals(sEncoding, ENCODING_IDENTITY))
						{
						CString sMediaType;
						if (!FindHeader(HEADER_CONTENT_TYPE, &sMediaType))
							sMediaType = NULL_STR;

						m_pBodyBuilder->Init(sMediaType);
						m_iState = stateBody;
						}

					//	If we have a non-zero content length, then we have a body.

					else if (FindHeader(HEADER_CONTENT_LENGTH, &sLength) 
							&& strToInt(sLength, 0) > 0)
						{
						CString sMediaType;
						if (!FindHeader(HEADER_CONTENT_TYPE, &sMediaType))
							sMediaType = NULL_STR;

						m_pBodyBuilder->Init(sMediaType);
						m_iState = stateBody;
						}

					//	Otherwise, no body

					else
						{
						m_pBodyBuilder->Init(NULL_STR);
						m_iState = stateDone;
						}
					}

				//	Otherwise, if headers are not yet done and we still have data
				//	in the buffer, then it means that we're waiting for more

				else if (pPos < pEndPos)
					{
					m_sLeftOver = CString(pPos, pEndPos - pPos);
					return true;
					}

				break;
				}

			case stateBody:
				{
				CString sEncoding;
				CString sLength;

				//	Chunked transfer encoding

				if (FindHeader(HEADER_TRANSFER_ENCODING, &sEncoding)
						&& strEquals(sEncoding, ENCODING_CHUNKED))
					{
					char *pOriginalPos = pPos;

					//	Read the chunk size line

					CString sLine;
					if (!ParseToken(pPos, pEndPos, '\r', &pPos, &sLine))
						{
						m_sLeftOver = CString(pOriginalPos, pEndPos - pOriginalPos);
						return true;
						}

					int iTotalLength = strParseIntOfBase(sLine, 16, 0);

					//	0-length means we're done

					if (iTotalLength == 0)
						{
						m_iState = stateDone;
						break;
						}

					//	Otherwise, read as much as we can

					int iLength = Min(iTotalLength, (int)(pEndPos - pPos));
					if (iLength > 0)
						{
						m_pBodyBuilder->Append(pPos, iLength);
						pPos += iLength;
						}

					//	If we have a partial chunk, then we need to remember state
					//	and wait for more

					if (iLength < iTotalLength)
						{
						m_iChunkLeft = iTotalLength - iLength;
						m_iState = stateChunk;
						}

					//	Otherwise, we're done with this chunk

					else
						{
						//	Read the terminating line end

						char *pOriginalPos = pPos;

						if (!ParseToken(pPos, pEndPos, '\r', &pPos, NULL))
							{
							m_iState = stateChunkEnd;
							m_sLeftOver = CString(pOriginalPos, pEndPos - pOriginalPos);
							return true;
							}
						}
					}

				//	Otherwise, look for content length

				else if (FindHeader(HEADER_CONTENT_LENGTH, &sLength))
					{
					int iTotalLength = strToInt(sLength, 0);
					int iRemaining = iTotalLength - m_pBodyBuilder->GetLength();
					int iLength = Min(iRemaining, (int)(pEndPos - pPos));

					//	Append the remainder of the buffer to the body

					if (iLength > 0)
						{
						m_pBodyBuilder->Append(pPos, iLength);
						pPos += iLength;
						}

					//	If we hit the end, then we're done

					if (iLength == iRemaining)
						m_iState = stateDone;
					}

				//	Otherwise, we are done

				else
					m_iState = stateDone;

				break;
				}

			case stateChunkEnd:
				{
				char *pOriginalPos = pPos;

				if (!ParseToken(pPos, pEndPos, '\r', &pPos, NULL))
					{
					m_iState = stateChunkEnd;
					m_sLeftOver = CString(pOriginalPos, pEndPos - pOriginalPos);
					return true;
					}

				m_iState = stateBody;
				break;
				}

			case stateChunk:
				{
				//	Keep reading

				int iLength = Min(m_iChunkLeft, (int)(pEndPos - pPos));
				if (iLength > 0)
					{
					m_pBodyBuilder->Append(pPos, iLength);
					pPos += iLength;
					m_iChunkLeft -= iLength;
					}

				//	Done?

				if (m_iChunkLeft == 0)
					{
					//	Read the terminating line end

					char *pOriginalPos = pPos;

					if (!ParseToken(pPos, pEndPos, '\r', &pPos, NULL))
						{
						m_iState = stateChunkEnd;
						m_sLeftOver = CString(pOriginalPos, pEndPos - pOriginalPos);
						return true;
						}

					//	Parse the next chunk

					m_iState = stateBody;
					}

				break;
				}
			}
		}

	//	If we're done, decode the body into an IMediaType object

	if (m_iState == stateDone && !m_pBodyBuilder->IsEmpty())
		{
		IMediaType *pBody;

		m_pBodyBuilder->CreateMedia(&pBody);
		SetBody(pBody);

		m_pBodyBuilder->Delete();
		m_pBodyBuilder = NULL;
		}

	return true;
	}

void CHTTPMessage::InitFromPartialBufferReset (IMediaTypeBuilder *pBodyBuilder)

//	InitFromPartialBufferReset
//
//	Must be called to reset before calling InitFromPartialBuffer a second time.
//	pBodyBuilder optionally constructs the body. We take ownership of this 
//	pointer.

	{
	if (m_pBodyBuilder)
		m_pBodyBuilder->Delete();

	if (pBodyBuilder)
		m_pBodyBuilder = pBodyBuilder;
	else
		m_pBodyBuilder = new CHTTPMessageBodyBuilder;

	m_iState = stateStart;
	m_sLeftOver = NULL_STR;
	m_Headers.DeleteAll();
	}

bool CHTTPMessage::InitFromSocket (SOCKET hSocket)

//	InitFromSocket
//
//	Reads from the socket

	{
	//	Keep reading until we've got enough (or until the connection drops)

	InitFromPartialBufferReset();
	while (!IsMessageComplete())
		{
		CBuffer TempBuffer(8192);

		//	Read

		int iBytesRead = recv(hSocket, (char *)TempBuffer.GetPointer(), 8192, 0);
		TempBuffer.SetLength(iBytesRead);

		//	If we're no making progress, then we're done

		if (iBytesRead == 0)
			return false;

		//	Parse to see if we're done

		if (!InitFromPartialBuffer(TempBuffer))
			return false;
		}

	return true;
	}

bool CHTTPMessage::InitFromStream (IByteStream &Stream)

//	InitFromStream
//
//	Reads from a stream

	{
	//	Keep reading until we've got enough (or until the connection drops)

	InitFromPartialBufferReset();
	while (!IsMessageComplete())
		{
		CBuffer TempBuffer(8192);

		//	Read

		int iBytesRead = Stream.Read(TempBuffer.GetPointer(), 8192);
		TempBuffer.SetLength(iBytesRead);

		//	If we're no making progress, then we're done

		if (iBytesRead == 0)
			return false;

		//	Parse to see if we're done

		if (!InitFromPartialBuffer(TempBuffer))
			return false;
		}

	return true;
	}

bool CHTTPMessage::InitRequest (const CString &sMethod, const CString &sURL)

//	InitRequest
//
//	Initializes an HTTP request message

	{
	m_iType = typeRequest;
	m_sMethod = sMethod;
	m_sURL = (sURL.IsEmpty() ? STR_EMPTY_PATH : sURL);
	m_sVersion = STR_HTTP_VERSION;
	m_bHTTP11 = true;

	//	Clear out the rest

	m_sStatusMsg = NULL_STR;
	m_dwStatusCode = 0;
	m_Headers.DeleteAll();

	if (m_pBodyBuilder)
		{
		m_pBodyBuilder->Delete();
		m_pBodyBuilder = NULL;
		}
	SetBody(NULL);

	return true;
	}

bool CHTTPMessage::InitResponse (DWORD dwStatusCode, const CString &sStatusMsg)

//	InitResponse
//
//	Initializes an HTTP response message

	{
	m_iType = typeResponse;
	m_sVersion = STR_HTTP_VERSION;
	m_dwStatusCode = dwStatusCode;
	m_sStatusMsg = sStatusMsg;
	m_bHTTP11 = true;

	//	Clear out the rest

	m_sMethod = NULL_STR;
	m_sURL = NULL_STR;
	m_Headers.DeleteAll();

	if (m_pBodyBuilder)
		{
		m_pBodyBuilder->Delete();
		m_pBodyBuilder = NULL;
		}
	SetBody(NULL);

	return true;
	}

bool CHTTPMessage::IsEncodingAccepted (EContentEncodingTypes iEncoding)

//	IsEncodingAccepted
//
//	Returns TRUE if the request header specifies that the given encoding is
//	allowed.

	{
	//	Identity is always allowed
	
	if (iEncoding == http_encodingIdentity)
		return true;

	//	Look for Accept-Encoding

	CString sValue;
	if (!FindHeader(HEADER_ACCEPT_ENCODING, &sValue))
		return false;

	TArray<CString> Encodings;
	strSplit(sValue, CString(","), &Encodings, -1, SSP_FLAG_WHITESPACE_SEPARATOR | SSP_FLAG_FORCE_LOWERCASE);

	//	Look for the specific encoding

	switch (iEncoding)
		{
		case http_encodingGzip:
			return Encodings.Find(ENCODING_GZIP);

		default:
			return false;
		}
	}

void CHTTPMessage::ParseCookies (const CString &sValue, TSortMap<CString, CString> *retCookies)

//	ParseCookies
//
//	Parses the cookies values of a header:
//
//	key1=value1; key2=value2; ...
//
//	We assume that keys and values are both url encoded (and we decode them
//	as part of the parsing process).

	{
	char *pPos = sValue.GetParsePointer();
	char *pPosEnd = pPos + sValue.GetLength();

	while (pPos < pPosEnd)
		{
		//	Skip whitespace

		while (pPos < pPosEnd && strIsWhitespace(pPos))
			pPos++;

		//	Key

		char *pStart = pPos;
		while (pPos < pPosEnd && *pPos != '=')
			pPos++;

		CString sKey(pStart, pPos - pStart);

		//	=

		if (pPos < pPosEnd && *pPos == '=')
			pPos++;

		//	Value

		pStart = pPos;
		while (pPos < pPosEnd && *pPos != ';')
			pPos++;

		CString sCookie(pStart, pPos - pStart);

		//	;

		if (pPos < pPosEnd && *pPos == ';')
			pPos++;

		if (!sKey.IsEmpty())
			retCookies->Insert(urlDecode(sKey), urlDecode(sCookie));
		}
	}

bool CHTTPMessage::ParseHeader (char *pPos, char *pEndPos, CString *retpField, CString *retpValue, char **retpPos, bool *retbHeadersDone)

//	ParseHeader
//
//	Parses a header line. We return FALSE if there is not enough data in the buffer
//	to parse a header (or even the end of the header blocks).
//
//	Fields are always returned in lowercase (to speed up case-insensitive compares).

	{
	//	Init. This needs to be valid even if we return FALSE.

	*retbHeadersDone = false;

	//	If we hit the end of the stream, then we need more data

	if (pPos >= pEndPos)
		return false;

	//	If we hit CRLF, then no more headers

	else if (pPos < pEndPos && *pPos == '\r')
		{
		pPos++;
		if (pPos == pEndPos)
			return false;

		if (*pPos == '\n')
			pPos++;

		if (retpPos)
			*retpPos = pPos;

		*retbHeadersDone = true;
		return false;
		}

	//	Skip leading whitespace

	while (pPos < pEndPos && (*pPos == ' ' || *pPos == '\t'))
		pPos++;

	//	Parse the field name

	char *pStart = pPos;
	while (pPos < pEndPos && *pPos != ':' && *pPos != ' ' && *pPos != '\t')
		pPos++;

	if (pPos == pEndPos)
		return false;

	CString sField(pStart, pPos - pStart);

	//	Find the colon

	while (pPos < pEndPos && *pPos != ':')
		pPos++;

	//	If we don't find the colon, then we need more data

	if (pPos == pEndPos)
		return false;

	pPos++;

	//	Skip any whitespace

	while (pPos < pEndPos && (*pPos == ' ' || *pPos == '\t'))
		pPos++;

	//	Accumulate the value

	bool bFoundLineEnd = false;
	pStart = pPos;
	while (pPos < pEndPos)
		{
		//	If our next two characters are CRLF...

		if (pPos[0] == '\r' && pPos + 1 < pEndPos && pPos[1] == '\n')
			{
			//	If the character after that is whitepace, then just keep
			//	going...

			if (pPos + 2 < pEndPos && (pPos[2] == ' ' || pPos[2] == '\t'))
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
		return false;

	//	Value

	CString sValue(pStart, pPos - pStart);

	//	Swallow the CRLF that terminates the header. We can guarantee that we
	//	have this because the loop above found a CRLF.

	if (pPos + 1 < pEndPos)
		pPos += 2;

	if (retpPos)
		*retpPos = pPos;

	//	Done

	*retpField = strToLower(sField);
	*retpValue = sValue;
	*retbHeadersDone = false;
	return !sField.IsEmpty();
	}

bool CHTTPMessage::ParseHeaderValue (const CString &sValue, CString *retsValue, TSortMap<CString, CString> *retFields)

//	ParseHeaderValue
//
//	Parses a complex header value of the form:
//
//	value; field1=value1; field2=value2; ...

	{
	char *pPos = sValue.GetParsePointer();
	char *pPosEnd =  pPos + sValue.GetLength();

	while (pPos < pPosEnd)
		{
		char *pStart = pPos;
		while (pPos < pPosEnd && *pPos != ';' && *pPos != '=')
			pPos++;

		CString sField(pStart, pPos - pStart);
		if (pPos == pPosEnd || *pPos == ';')
			{
			if (retsValue)
				*retsValue = sField;
			}
		else
			{
			pPos++;

			if (*pPos == '\"')
				{
				pPos++;

				pStart = pPos;
				while (pPos < pPosEnd && *pPos != '\"')
					pPos++;

				if (retFields)
					retFields->Insert(sField, CString(pStart, pPos - pStart));

				if (pPos < pPosEnd && *pPos == '\"')
					pPos++;
				}
			else
				{
				pStart = pPos;
				while (pPos < pPosEnd && *pPos != ';')
					pPos++;

				if (retFields)
					retFields->Insert(sField, CString(pStart, pPos - pStart));
				}
			}

		if (pPos < pPosEnd && *pPos == ';')
			pPos++;

		while (pPos < pPosEnd && strIsWhitespace(pPos))
			pPos++;
		}

	return true;
	}

bool CHTTPMessage::ParseToken (char *pPos, char *pEndPos, char chDelimiter, char **retpPos, CString *retsToken) const

//	ParseToken
//
//	Parses until the delimiter. If we hit the end before we hit the delimiter, 
//	then we return FALSE.

	{
	char *pStart = pPos;
	while (pPos < pEndPos && *pPos != chDelimiter)
		pPos++;

	std::ptrdiff_t iLen = (pPos - pStart);

	//	If we hit the end, then we're done

	if (pPos == pEndPos)
		{
		if (retpPos)
			*retpPos = pPos;

		if (retsToken)
			*retsToken = CString(pStart, iLen);

		return false;
		}

	//	Otherwise, skip the delimiter

	if (chDelimiter == ' ')
		{
		while (*pPos == ' ' && pPos < pEndPos)
			pPos++;
		}
	else if (chDelimiter == '\r')
		{
		pPos++;
		if (pPos == pEndPos)
			{
			if (retpPos)
				*retpPos = pPos;

			if (retsToken)
				*retsToken = CString(pStart, iLen);

			return false;
			}

		if (*pPos == '\n')
			pPos++;
		}
	else
		pPos++;

	//	Return the new position

	if (retpPos)
		*retpPos = pPos;

	if (retsToken)
		*retsToken = CString(pStart, iLen);

	return true;
	}

void CHTTPMessage::SetBody (IMediaType *pBody)

//	SetBody
//
//	Sets the body

	{
	if (m_pBody)
		delete m_pBody;

	m_pBody = pBody;
	}

bool CHTTPMessage::WriteChunkToBuffer (IByteStream &Stream, DWORD dwOffset, DWORD dwSize) const

//	WriteChunkToBuffer
//
//	Writes a chunk to the buffer. If dwSize is 0 then we output a terminating 
//	chunk of zero length.

	{
	//	Must have a body.

	ASSERT(m_pBody);
	if (m_pBody == NULL)
		return false;

	//	Write out the chunk size.

	CString sSize = strPattern(STR_CHUNK_CONS, dwSize);
	Stream.Write(sSize);

	//	Write the chunk data

	if (dwSize)
		{
		if (!m_pBody->EncodeToBuffer(Stream, dwOffset, dwSize))
			return false;
		}

	//	CRLF terminates the chunk

	Stream.Write("\r\n", 2);
	return true;
	}

bool CHTTPMessage::WriteHeadersToBuffer (IByteStream &Stream, DWORD dwFlags) const

//	WriteHeadersToBuffer
//
//	Writes the header of the message (not the body).

	{
	int i;

	ASSERT(m_iType != typeUnknown);

	//	Write the status line

	CString sLine;
	if (m_iType == typeRequest)
		{
		ASSERT(!m_sMethod.IsEmpty());
		ASSERT(!m_sURL.IsEmpty());
		sLine = strPattern(STR_REQUEST_LINE, m_sMethod, m_sURL);
		}
	else
		{
		ASSERT(m_dwStatusCode > 0);
		ASSERT(!m_sStatusMsg.IsEmpty());
		sLine = strPattern(STR_RESPONSE_LINE, m_dwStatusCode, m_sStatusMsg);
		}

	Stream.Write(sLine);

	//	Write the headers

	for (i = 0; i < m_Headers.GetCount(); i++)
		{
		if (!m_Headers[i].sField.IsEmpty())
			{
			sLine = strPattern(STR_HEADER_CONS, m_Headers[i].sField, m_Headers[i].sValue);
			Stream.Write(sLine);
			}
		}

	//	If we don't have a Content-Type header then output our own
	//	NOTE: If we don't have a media type then we let the client figure it out (we omit
	//	the content-type header).

	if (!FindHeader(HEADER_CONTENT_TYPE) && m_pBody && !m_pBody->GetMediaType().IsEmpty())
		{
		sLine = strPattern(STR_HEADER_CONS, HEADER_CONTENT_TYPE, m_pBody->GetMediaType());
		Stream.Write(sLine);
		}

	//	Output an encoding header

	EContentEncodingTypes iContentEncoding;
	if (!FindHeader(HEADER_CONTENT_ENCODING) && m_pBody && (iContentEncoding = m_pBody->GetMediaEncoding()) != http_encodingIdentity)
		{
		sLine = strPattern(STR_HEADER_CONS, HEADER_CONTENT_ENCODING, m_pBody->GetMediaEncodingHeader());
		Stream.Write(sLine);
		}

	//	If we have chunked encoding, then output the appropriate header

	if (dwFlags & FLAG_CHUNKED_ENCODING)
		{
		if (!FindHeader(HEADER_TRANSFER_ENCODING))
			{
			sLine = strPattern(STR_HEADER_CONS, HEADER_TRANSFER_ENCODING, ENCODING_CHUNKED);
			Stream.Write(sLine);
			}
		}

	//	Otherwise, if we don't have a Content-Length header then we output our own

	else if (!FindHeader(HEADER_CONTENT_LENGTH))
		{
		int iLength = (m_pBody ? m_pBody->GetMediaLength() : 0);
		sLine = strPattern(STR_HEADER_CONS, HEADER_CONTENT_LENGTH, strFromInt(iLength));
		Stream.Write(sLine);
		}

	//	Done with headers

	Stream.Write("\r\n", 2);

	return true;
	}

bool CHTTPMessage::WriteToBuffer (IByteStream &Stream) const

//	WriteToBuffer
//
//	Writes the message to a buffer suitable for transmission.
//	We assume that the stream has already been created.

	{
	WriteHeadersToBuffer(Stream);

	//	Output the body

	if (m_pBody)
		{
		if (!m_pBody->EncodeToBuffer(Stream))
			return false;
		}

	//	Done

	return true;
	}

//	Utilities ------------------------------------------------------------------

CString GetToken (char *pPos, char *pEndPos, char chDelimiter, char **retpPos)
	{
	char *pStart = pPos;
	while (pPos < pEndPos && *pPos != chDelimiter)
		pPos++;

	int iLen = (int)(pPos - pStart);

	//	If we hit the end, then we're done

	if (pPos == pEndPos)
		{
		if (retpPos)
			*retpPos = pPos;
		return CString(pStart, iLen);
		}

	//	Otherwise, skip the delimiter

	if (chDelimiter == ' ')
		{
		while (*pPos == ' ' && pPos < pEndPos)
			pPos++;
		}
	else if (chDelimiter == '\r')
		{
		pPos++;
		if (pPos < pEndPos && *pPos == '\n')
			pPos++;
		}
	else
		pPos++;

	//	Return the new position

	if (retpPos)
		*retpPos = pPos;

	return CString(pStart, iLen);
	}

