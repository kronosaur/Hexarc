//	CHTTPMessage.cpp
//
//	CHTTPMessage class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_HEADER_CONS,					"%s: %s\r\n")
DECLARE_CONST_STRING(STR_REQUEST_LINE,					"%s %s HTTP/1.1\r\n")
DECLARE_CONST_STRING(STR_EMPTY_PATH,					"/")
DECLARE_CONST_STRING(STR_HTTP,							"HTTP")
DECLARE_CONST_STRING(STR_HTTP_VERSION,					"HTTP/1.1")
DECLARE_CONST_STRING(STR_RESPONSE_LINE,					"HTTP/1.1 %d %s\r\n")
DECLARE_CONST_STRING(STR_CHUNK_CONS,					"%X\r\n")

DECLARE_CONST_STRING(HEADER_AUTHORIZATION,				"authorization")
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

DECLARE_CONST_STRING(ERR_DUMP_FAILED,					"Unable to write message to buffer.")

CHTTPMessage::SStatusMessageEntry CHTTPMessage::m_StatusMessageTable[] = 
	{
		{	http_CONTINUE,							"Continue"						},
		{	http_SWITCHING_PROTOCOLS,				"Switching Protocols"			},

		{	http_OK,								"OK"							},
		{	http_CREATED,							"Created"						},
		{	http_ACCEPTED,							"Accepted"						},
		{	http_NON_AUTHORITATIVE,					"Non-Authoritative Information"	},
		{	http_NO_CONTENT,						"No Content"					},
		{	http_RESET_CONTENT,						"Reset Content"					},
		{	http_PARTIAL_CONTENT,					"Partial Content"				},

		{	http_MULTIPLE_CHOICES,					"Multiple Choices"				},
		{	http_MOVED_PERMANENTLY,					"Moved Permanently"				},
		{	http_FOUND,								"Found"							},
		{	http_SEE_OTHER,							"See Other"						},
		{	http_NOT_MODIFIED,						"Not Modified"					},
		{	http_USE_PROXY,							"Use Proxy"						},
		{	http_TEMPORARY_REDIRECT,				"Temporary Redirect"			},

		{	http_BAD_REQUEST,						"Bad Request"					},
		{	http_UNAUTHORIZED,						"Unauthorized"					},
		{	http_FORBIDDEN,							"Forbidden"						},
		{	http_NOT_FOUND,							"Not Found"						},
		{	http_NOT_ALLOWED,						"Method Not Allowed"			},
		{	http_NOT_ACCEPTABLE,					"Not Acceptable"				},
		{	http_PROXY_AUTH_REQUIRED,				"Proxy Authentication Required"	},
		{	http_REQUEST_TIMEOUT,					"Request Timeout"				},
		{	http_CONFLICT,							"Conflict"						},
		{	http_GONE,								"Gone"							},
		{	http_LENGTH_REQUIRED,					"Length Required"				},
		{	http_PRECONDITION_FAILED,				"Precondition Failed"			},
		{	http_ENTITY_TOO_LARGE,					"Payload Too Large"				},
		{	http_URI_TOO_LONG,						"URI Too Long"					},
		{	http_UNSUPPORTED_MEDIA_TYPE,			"Unsupported Media Type"		},
		{	http_BAD_REQUEST_RANGE,					"Range Not Satisfiable"			},
		{	http_EXPECTATION_FAILED,				"Expectation Failed"			},

		{	http_INTERNAL_SERVER_ERROR,				"Internal Server Error"			},
		{	http_NOT_IMPLEMENTED,					"Not Implemented"				},
		{	http_BAD_GATEWAY,						"Bad Gateway"					},
		{	http_SERVICE_UNAVAILABLE,				"Service Unavailable"			},
		{	http_GATEWAY_TIMEOUT,					"Gateway Timeout"				},
		{	http_VERSION_NOT_SUPPORTED,				"HTTP Version Not Supported"	},
	};

CString GetToken (char *pPos, char *pEndPos, char chDelimiter, char **retpPos);

CHTTPMessage::CHTTPMessage (void) : 
		m_iType(typeUnknown),
		m_bHTTP11(true)

//	CHTTPMessage constructor

	{
	InitFromPartialBufferReset();
	}

void CHTTPMessage::AddAuthBasic (const CString &sUsername, const CString &sPassword)

//	AddAuthBasic
//
//	Adds a basic authentication header

	{
	CString sAuth = strPattern("%s:%s", sUsername, sPassword);
	CString sEncoded = sAuth.AsBase64();
	
	AddHeader(HEADER_AUTHORIZATION, strPattern("Basic %s", sEncoded));
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

CString CHTTPMessage::DebugDump (void) const

//	DebugDump
//
//	Dumps the message to a string.

	{
	CStringBuffer Output;

	if (!WriteToBuffer(Output))
		return ERR_DUMP_FAILED;

	return CString(std::move(Output));
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
	if (!m_pBody
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

CString CHTTPMessage::GetRequestedPath (void) const

//	GetRequestedPath
//
//	Returns the URL that the message is requesting.
//	(URL is always absolute, starting with a leading '/', but has no host)

	{
	//	Parse the URL

	CString sPath;
	if (!urlParse(m_sURL, NULL, NULL, &sPath))
		return NULL_STR;

	return sPath;
	}

CString CHTTPMessage::GetRequestedURL (void) const

//	GetRequestedURL
//
//	Returns the full URL, including the host, but not always the protocol.

	{
	//	See if our URL has the host

	CString sHost;
	CString sPath;
	if (!urlParse(m_sURL, NULL, &sHost, &sPath))
		return NULL_STR;

	//	If the URL already has the host, then just return it.

	if (!sHost.IsEmpty())
		return m_sURL;

	//	Otherwise, get it from the headers

	if (!FindHeader(HEADER_HOST, &sHost) || sHost.IsEmpty())
		return m_sURL;

	return strPattern("%s%s", sHost, sPath);
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

bool CHTTPMessage::InitFromPartialBuffer (const IMemoryBlock &Buffer, bool bNoBody, TArray<CString> *pDebugOutput)

//	InitFromPartialBuffer
//
//	This function parses the given buffer into a message. We can call this 
//	multiple times with pieces of the full message as long as the pieces are
//	contiguous and in proper order.
//
//	We assume that InitFromPartialBufferReset has been called.

	{
	const char *pPos;
	const char *pEndPos;

	if (!m_pBodyBuilder)
		{
		InitFromPartialBufferReset();
		if (pDebugOutput)
			pDebugOutput->Insert(CString("Initializing buffer."));
		}

	//	If we have left overs, prepend it to the new buffer.

	CBuffer Temp;
	if (!m_sLeftOver.IsEmpty())
		{
		if (pDebugOutput)
			pDebugOutput->Insert(strPattern("m_sLeftOver has %d bytes.", m_sLeftOver.GetLength()));

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

	if (pDebugOutput)
		pDebugOutput->Insert(strPattern("Buffer has %d bytes.", Buffer.GetLength()));

	while (pPos < pEndPos && m_iState != stateDone)
		{
		switch (m_iState)
			{
			case stateStart:
				{
				//	Remember the original position in case we need to back-track.

				const char *pOriginalPos = pPos;

				//	Get the first token. If we don't find the delimiter, then we 
				//	need more data.

				CString sToken;
				if (!ParseToken(pPos, pEndPos, ' ', &pPos, &sToken))
					{
					m_sLeftOver = CString(pOriginalPos, pEndPos - pOriginalPos);

					if (pDebugOutput)
						pDebugOutput->Insert(strPattern("stateStart: Need more data."));

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

						if (pDebugOutput)
							pDebugOutput->Insert(strPattern("stateStart: Need more data for status code."));

						return true;
						}

					m_dwStatusCode = strToInt(sStatus, 0);

					//	Parse the message

					if (!ParseToken(pPos, pEndPos, '\r', &pPos, &m_sStatusMsg))
						{
						m_sLeftOver = CString(pOriginalPos, pEndPos - pOriginalPos);

						if (pDebugOutput)
							pDebugOutput->Insert(strPattern("stateStart: Need more data for message."));

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
				SetBody(IMediaTypePtr());

				m_iState = stateHeaders;

				if (pDebugOutput)
					pDebugOutput->Insert(strPattern("stateStart: %s %s", m_sMethod, m_sURL));

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

						if (pDebugOutput)
							pDebugOutput->Insert(strPattern("stateHeaders: No body."));
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

						if (pDebugOutput)
							pDebugOutput->Insert(strPattern("stateHeaders: transfer encoding: %s.", sEncoding));
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

						if (pDebugOutput)
							pDebugOutput->Insert(strPattern("stateHeaders: content length: %s.", sLength));
						}

					//	Otherwise, no body

					else
						{
						m_pBodyBuilder->Init(NULL_STR);
						m_iState = stateDone;

						if (pDebugOutput)
							pDebugOutput->Insert(strPattern("stateHeaders: No body."));
						}
					}

				//	Otherwise, if headers are not yet done and we still have data
				//	in the buffer, then it means that we're waiting for more

				else if (pPos < pEndPos)
					{
					if (pDebugOutput)
						pDebugOutput->Insert(strPattern("stateHeaders: Need more data."));

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
					const char *pOriginalPos = pPos;

					//	Read the chunk size line

					CString sLine;
					if (!ParseToken(pPos, pEndPos, '\r', &pPos, &sLine))
						{
						m_sLeftOver = CString(pOriginalPos, pEndPos - pOriginalPos);

						if (pDebugOutput)
							pDebugOutput->Insert(strPattern("stateBody: Chunked transfer, need more data."));

						return true;
						}

					int iTotalLength = strParseIntOfBase(sLine, 16, 0);

					if (pDebugOutput)
						pDebugOutput->Insert(strPattern("stateBody: Total length %d.", iTotalLength));

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

						if (pDebugOutput)
							pDebugOutput->Insert(strPattern("stateBody: Chunked transfer read %d need %d more.", iLength, m_iChunkLeft));
						}

					//	Otherwise, we're done with this chunk

					else
						{
						//	Read the terminating line end

						const char *pOriginalPos = pPos;

						if (!ParseToken(pPos, pEndPos, '\r', &pPos, NULL))
							{
							m_iState = stateChunkEnd;
							m_sLeftOver = CString(pOriginalPos, pEndPos - pOriginalPos);

							if (pDebugOutput)
								pDebugOutput->Insert(strPattern("stateBody: Chunked need more data."));

							return true;
							}

						if (pDebugOutput)
							pDebugOutput->Insert(strPattern("stateBody: Chunk done."));
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

					if (pDebugOutput)
						pDebugOutput->Insert(strPattern("stateBody: Total length %d remaining %d length %d.", iTotalLength, iRemaining, iLength));

					//	If we hit the end, then we're done

					if (iLength == iRemaining)
						{
						m_iState = stateDone;

						if (pDebugOutput)
							pDebugOutput->Insert(strPattern("stateBody: Done."));
						}
					}

				//	Otherwise, we are done

				else
					{
					m_iState = stateDone;

					if (pDebugOutput)
						pDebugOutput->Insert(strPattern("stateBody: Done done."));
					}

				break;
				}

			case stateChunkEnd:
				{
				const char *pOriginalPos = pPos;

				if (!ParseToken(pPos, pEndPos, '\r', &pPos, NULL))
					{
					m_iState = stateChunkEnd;
					m_sLeftOver = CString(pOriginalPos, pEndPos - pOriginalPos);

					if (pDebugOutput)
						pDebugOutput->Insert(strPattern("stateChunkEnd: Chunked need more data."));

					return true;
					}

				m_iState = stateBody;

				if (pDebugOutput)
					pDebugOutput->Insert(strPattern("stateChunkEnd: Chunk done."));

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

				if (pDebugOutput)
					pDebugOutput->Insert(strPattern("stateChunk: Read %d need %d more.", iLength, m_iChunkLeft));

				//	Done?

				if (m_iChunkLeft == 0)
					{
					//	Read the terminating line end

					const char *pOriginalPos = pPos;

					if (!ParseToken(pPos, pEndPos, '\r', &pPos, NULL))
						{
						m_iState = stateChunkEnd;
						m_sLeftOver = CString(pOriginalPos, pEndPos - pOriginalPos);

						if (pDebugOutput)
							pDebugOutput->Insert(strPattern("stateChunk: Chunked need more data."));

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
		IMediaTypePtr pBody;

		m_pBodyBuilder->CreateMedia(&pBody);
		SetBody(pBody);

		m_pBodyBuilder = IMediaTypeBuilderPtr();
		}

	return true;
	}

void CHTTPMessage::InitFromPartialBufferReset (IMediaTypeBuilderPtr pBodyBuilder)

//	InitFromPartialBufferReset
//
//	Must be called to reset before calling InitFromPartialBuffer a second time.
//	pBodyBuilder optionally constructs the body. We take ownership of this 
//	pointer.

	{
	if (pBodyBuilder)
		m_pBodyBuilder = pBodyBuilder;
	else
		m_pBodyBuilder = IMediaTypeBuilderPtr(new CHTTPMessageBodyBuilder);

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

	m_pBodyBuilder = NULL;

	SetBody(IMediaTypePtr());

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

	m_pBodyBuilder = NULL;

	SetBody(IMediaTypePtr());

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

bool CHTTPMessage::ParseHeader (const char *pPos, const char *pEndPos, CString *retpField, CString *retpValue, const char **retpPos, bool *retbHeadersDone)

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

	const char *pStart = pPos;
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

			if (*pPos == '"')
				{
				pPos++;

				pStart = pPos;
				while (pPos < pPosEnd && *pPos != '"')
					pPos++;

				if (retFields)
					retFields->Insert(sField, CString(pStart, pPos - pStart));

				if (pPos < pPosEnd && *pPos == '"')
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

bool CHTTPMessage::ParseRequestedURL (CString *retsProtocol, CString *retsHost, CString *retsPath) const

//	ParseRequestedURL
//
//	Returns the appropriate URL components. Returns FALSE if we get an error.

	{
	CString sHost;
	if (!urlParse(m_sURL, retsProtocol, &sHost, retsPath))
		return false;

	//	If no host in the URL, get it from the header

	if (sHost.IsEmpty())
		{
		if (!FindHeader(HEADER_HOST, &sHost))
			return false;
		}

	//	Return

	if (retsHost)
		*retsHost = sHost;

	return true;
	}

bool CHTTPMessage::ParseToken (const char *pPos, const char *pEndPos, char chDelimiter, const char **retpPos, CString *retsToken) const

//	ParseToken
//
//	Parses until the delimiter. If we hit the end before we hit the delimiter, 
//	then we return FALSE.

	{
	const char *pStart = pPos;
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

void CHTTPMessage::SetBody (IMediaTypePtr pBody)

//	SetBody
//
//	Sets the body

	{
	m_pBody = pBody;
	}

CString CHTTPMessage::StatusMessageFromStatusCode (DWORD dwStatusCode)

//	StatusMessageFromStatusCode
//
//	Returns the standard status message from a code. If the code is invalid, we 
//	return NULL_STR.

	{
	constexpr int TABLE_SIZE = SIZEOF_STATIC_ARRAY(m_StatusMessageTable);

	for (int i = 0; i < TABLE_SIZE; i++)
		if (m_StatusMessageTable[i].dwCode == dwStatusCode)
			return CString(m_StatusMessageTable[i].pszMessage);

	return NULL_STR;
	}

bool CHTTPMessage::WriteChunkToBuffer (IByteStream &Stream, DWORD dwOffset, DWORD dwSize) const

//	WriteChunkToBuffer
//
//	Writes a chunk to the buffer. If dwSize is 0 then we output a terminating 
//	chunk of zero length.

	{
	//	Must have a body.

	ASSERT(m_pBody);
	if (!m_pBody)
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

