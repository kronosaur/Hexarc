//	CEsperInterface.cpp
//
//	CEsperInterface class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_PROXY,						"proxy")
DECLARE_CONST_STRING(FIELD_RAW,							"raw")

DECLARE_CONST_STRING(HEADER_CONTENT_TYPE,				"content-type")
DECLARE_CONST_STRING(HEADER_HOST,						"host")

DECLARE_CONST_STRING(MEDIA_TYPE_FORM_URL_ENCODED,		"application/x-www-form-urlencoded")
DECLARE_CONST_STRING(MEDIA_TYPE_JSON,					"application/json")
DECLARE_CONST_STRING(MEDIA_TYPE_JSON_REQUEST,			"application/jsonrequest")
DECLARE_CONST_STRING(MEDIA_TYPE_MULTIPART_FORM,			"multipart/form-data")
DECLARE_CONST_STRING(MEDIA_TYPE_HTML,					"text/html")
DECLARE_CONST_STRING(MEDIA_TYPE_TEXT,					"text/plain")
DECLARE_CONST_STRING(MEDIA_TYPE_TEXT_PREFIX,			"text/")

DECLARE_CONST_STRING(PROTOCOL_HTTPS,					"https")

DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_FORM_URL_ENCODED,	"Error parsing form URL encoded.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_JSON,			"Error parsing JSON.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_MULTIPART,		"Error parsing MIME multipart/form-data.")
DECLARE_CONST_STRING(ERR_INVALID_URL,					"Invalid URL: %s.")
DECLARE_CONST_STRING(ERR_INVALID_PORT,					"Unable to determine port from URL: %s.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_CONNECT,				"Unable to connect to server at %s.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_RECEIVE,				"Unable to receive data from %s; connection lost.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_SEND,				"Unable to send data to %s; connection lost.")
DECLARE_CONST_STRING(ERR_UNSUPPORTED_MEDIA_TYPE,		"Unsupported media type: %s.")

void CEsperInterface::AddHeadersToMessage (CDatum dHeaders, CHTTPMessage *retMessage)

//	AddHeadersToMessage
//
//	Add headers to the given message

	{
	int i;

	for (i = 0; i < dHeaders.GetCount(); i++)
		retMessage->AddHeader(dHeaders.GetKey(i), dHeaders.GetElement(i).AsString());
	}

DWORD CEsperInterface::ConnectionToFriendlyID (CDatum dConnection)

//	ConnectionToFriendlyID
//
//	Converts a connection to a human-readable value

	{
	DWORD dwID = dConnection;
	if (dwID == 0)
		return 0;
	else
		return (dwID & 0x00ffffff) + 1;
	}

bool CEsperInterface::ConvertBodyToDatum (const CHTTPMessage &Message, CDatum *retdBody)

//	ConvertBodyToDatum
//
//	Converts to a datum.
//
//	NOTE: We need to implement this in CEsperInterface (as opposed to 
//	CHTTPMessageBodyBuilder) because of layering (we're above AEON, but 
//	Foundation is not).

	{
	//	Edge condition. This is valid (e.g.) if we have a pure GET request.

	IMediaTypePtr pBody = Message.GetBody();
	if (!pBody)
		{
		*retdBody = CDatum();
		return true;
		}

	//	Get the media and media type

	CString sMediaType;
	if (!CHTTPMessage::ParseHeaderValue(pBody->GetMediaType(), &sMediaType, NULL))
		{
		*retdBody = strPattern(ERR_UNSUPPORTED_MEDIA_TYPE, pBody->GetMediaType());
		return false;
		}

	const CString &sBuffer = pBody->GetMediaBuffer();

	//	Parse based on media type

	if (strEquals(sMediaType, MEDIA_TYPE_FORM_URL_ENCODED))
		{
		if (!ConvertFormURLEncodedToDatum(sBuffer, retdBody))
			{
			*retdBody = ERR_UNABLE_TO_PARSE_FORM_URL_ENCODED;
			return false;
			}
		}
	else if (strEquals(sMediaType, MEDIA_TYPE_JSON))
		{
		CStringBuffer Buffer(sBuffer);
		if (!CDatum::Deserialize(CDatum::EFormat::JSON, Buffer, retdBody))
			{
			*retdBody = ERR_UNABLE_TO_PARSE_JSON;
			return false;
			}
		}
	else if (strEquals(sMediaType, MEDIA_TYPE_MULTIPART_FORM))
		{
		CBuffer Buffer(pBody->GetMediaBuffer());
		CEsperMultipartParser Parser(pBody->GetMediaType(), Buffer);
		if (!Parser.ParseAsDatum(retdBody))
			{
			*retdBody = ERR_UNABLE_TO_PARSE_MULTIPART;
			return false;
			}
		}
	else if (strStartsWith(sMediaType, MEDIA_TYPE_TEXT_PREFIX))
		{
		*retdBody = sBuffer;
		}
	else
		{
		*retdBody = strPattern(ERR_UNSUPPORTED_MEDIA_TYPE, sMediaType);
		return false;
		}

	//	Done

	return true;
	}

bool CEsperInterface::ConvertFormURLEncodedToDatum (const CString &sText, CDatum *retdValue)

//	ConvertFormURLEncodedToDatum
//
//	Converts from x-www-form-urlencoded to a struct

	{
	enum EStates
		{
		stateStart,
		stateSkip,
		stateFieldName,
		stateFieldValue,
		stateEscape,
		stateDone,
		};

	CComplexStruct *pResult = new CComplexStruct;

	char *pPos = sText.GetParsePointer();
	char *pPosEnd = pPos + sText.GetLength();
	int iState = stateStart;
	int iOldState;

	CMemoryBuffer Token;
	CString sFieldName;

	while (iState != stateDone)
		{
		switch (iState)
			{
			case stateStart:
				if (pPos == pPosEnd)
					iState = stateDone;
				else if (*pPos == '&')
					break;
				else if (*pPos == '=')
					iState = stateSkip;
				else
					{
					Token.SetLength(0);
					Token.Write(pPos, 1);
					iState = stateFieldName;
					}

				break;

			case stateSkip:
				if (pPos == pPosEnd)
					iState = stateDone;
				else if (*pPos == '&')
					iState = stateStart;
				break;

			case stateFieldName:
				if (pPos == pPosEnd)
					iState = stateDone;
				else if (*pPos == '=')
					{
					sFieldName = CString(Token.GetPointer(), Token.GetLength());
					Token.SetLength(0);
					iState = stateFieldValue;
					}
				else if (*pPos == '+')
					Token.Write(" ", 1);
				else if (*pPos == '%')
					{
					iOldState = iState;
					iState = stateEscape;
					}
				else
					Token.Write(pPos, 1);
				break;

			case stateFieldValue:
				if (pPos == pPosEnd
						|| *pPos == '&')
					{
					pResult->SetElement(sFieldName, CString(Token.GetPointer(), Token.GetLength()));
					Token.SetLength(0);
					iState = ((pPos == pPosEnd) ? stateDone : stateStart);
					}
				else if (*pPos == '+')
					Token.Write(" ", 1);
				else if (*pPos == '%')
					{
					iOldState = iState;
					iState = stateEscape;
					}
				else
					Token.Write(pPos, 1);
				break;

			case stateEscape:
				if (pPos+1 >= pPosEnd)
					iState = stateDone;
				else
					{
					DWORD dwValue = (strParseHexChar(*pPos++) << 4);
					dwValue += strParseHexChar(*pPos);

					char chChar = (char)(BYTE)dwValue;
					Token.Write(&chChar, 1);
					iState = iOldState;
					}
				break;
			}

		if (iState != stateDone)
			pPos++;
		else
			break;
		}

	//	Return result

	if (pResult->GetCount() == 0)
		{
		delete pResult;
		*retdValue = CDatum();
		return true;
		}

	*retdValue = CDatum(pResult);
	return true;
	}

CDatum CEsperInterface::ConvertHeadersToDatum (const CHTTPMessage &Message)

//	ConvertHeadersToDatum
//
//	Converts to datum

	{
	int i;

	CComplexStruct *pResult = new CComplexStruct;
	for (i = 0; i < Message.GetHeaderCount(); i++)
		{
		CString sHeader;
		CString sValue;

		Message.GetHeader(i, &sHeader, &sValue);

		//	If this header is already in the structure, then we need to append it.
		//
		//	NOTE: CHTTPMessage always converts headers to lowercase for ease of
		//	compare, since the HTTP spec says header names are case-insensitive.

		const CString &sOriginalData = pResult->GetElement(sHeader);
		if (!sOriginalData.IsEmpty())
			pResult->SetElement(sHeader, strPattern("%s, %s", sOriginalData, sValue));
		
		//	Otherwise we just set it

		else
			pResult->SetElement(sHeader, sValue);
		}

	//	Done

	if (pResult->GetCount() > 0)
		return CDatum(pResult);
	else
		{
		delete pResult;
		return CDatum();
		}
	}

CDatum CEsperInterface::DecodeHTTPResponse (const CHTTPMessage &Message, bool bRawBody)

//	DecodeHTTPResponse
//
//	Converts an HTTP response message into a datum for return to clients.

	{
	CComplexArray *pResult = new CComplexArray;
	pResult->Append((int)Message.GetStatusCode());
	pResult->Append(Message.GetStatusMsg());
	pResult->Append(CEsperInterface::ConvertHeadersToDatum(Message));

	CDatum dBody;
	if (bRawBody)
		dBody = CDatum(Message.GetBodyBuffer());
	else
		CEsperInterface::ConvertBodyToDatum(Message, &dBody);

	//	OK if we get an error; dBody will be the error.

	pResult->Append(dBody);

	//	Done

	return CDatum(pResult);
	}

void CEsperInterface::EncodeHTTPRequest (const CString &sMethod, const CString &sHost, const CString &sPath, CDatum dHeaders, CDatum dBody, CHTTPMessage *retMessage)

//	EncodeHTTPRequest
//
//	Encodes a message

	{
	int i;

	retMessage->InitRequest(sMethod, sPath);

	//	Add headers

	bool bFoundHost = false;
	for (i = 0; i < dHeaders.GetCount(); i++)
		{
		retMessage->AddHeader(dHeaders.GetKey(i), (const CString &)dHeaders.GetElement(i));
		if (strEquals(strToLower(dHeaders.GetKey(i)), HEADER_HOST))
			bFoundHost = true;
		}

	//	Add body

	if (!dBody.IsNil())
		{
		CString sMediaType;
		if (!retMessage->FindHeader(HEADER_CONTENT_TYPE, &sMediaType))
			sMediaType = MEDIA_TYPE_TEXT;

		IMediaTypePtr pBody = IMediaTypePtr(new CRawMediaType);
		pBody->DecodeFromBuffer(sMediaType, CStringBuffer(dBody.AsString()));

		retMessage->SetBody(pBody);
		}


	//	If there is no Host header and we have a host, then add it

	if (!bFoundHost && !sHost.IsEmpty())
		retMessage->AddHeader(HEADER_HOST, sHost);
	}

bool CEsperInterface::HTTP (const CString &sMethod, const CString &sURL, CDatum dHeaders, CDatum dBody, CDatum dOptions, CDatum *retdResult)

//	HTTP
//
//	Makes an HTTP call

	{
	//	Get some options

	bool bProxy = !dOptions.GetElement(FIELD_PROXY).IsNil();
	bool bRaw = !dOptions.GetElement(FIELD_RAW).IsNil();

	//	Parse the URL

	CString sHost;
	CString sProtocol;
	CString sPath;
	if (!urlParse(sURL.GetParsePointer(), &sProtocol, &sHost, &sPath))
		{
		*retdResult = strPattern(ERR_INVALID_URL, sURL);
		return false;
		}

	CString sHostname;
	DWORD dwPort;
	if (!urlParseHostPort(sProtocol, sHost, &sHostname, &dwPort))
		{
		*retdResult = strPattern(ERR_INVALID_PORT, sURL);
		return false;
		}

	bool bUseSSL = strEquals(sProtocol, PROTOCOL_HTTPS);

	//	Connect to the host

	CSSLSocketStream SocketStream;
	if (!SocketStream.Connect(sHostname, dwPort, bUseSSL, NULL))
		{
		*retdResult = strPattern(ERR_UNABLE_TO_CONNECT, sURL);
		return false;
		}

	//	Serialize to an HTTP message buffer

	CHTTPMessage Request;
	CEsperInterface::EncodeHTTPRequest(sMethod, (bProxy ? NULL_STR : sHost), sPath, dHeaders, dBody, &Request);

	CStringBuffer RequestBuffer;
	Request.WriteToBuffer(RequestBuffer);

	//	Write out (synchronously, for now)

	int iSent = SocketStream.Write(RequestBuffer.GetPointer(), RequestBuffer.GetLength());
	if (iSent != RequestBuffer.GetLength())
		{
		*retdResult = strPattern(ERR_UNABLE_TO_SEND, sURL);
		return false;
		}

	//	Keep reading until we have a full HTTP message (or until we get an error)

	CHTTPMessage Response;
	if (!Response.InitFromStream(SocketStream))
		{
		*retdResult = strPattern(ERR_UNABLE_TO_RECEIVE, sURL);
		return false;
		}

	//	Compose a response

	*retdResult = CEsperInterface::DecodeHTTPResponse(Response, bRaw);

	//	Done

	SocketStream.Disconnect();
	return true;
	}

