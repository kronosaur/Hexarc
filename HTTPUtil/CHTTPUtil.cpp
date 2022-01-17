//	CHTTPUtil.cpp
//
//	CHTTPUtil Class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(HEADER_CONTENT_TYPE,				"content-type");
DECLARE_CONST_STRING(HEADER_HOST,						"host");

DECLARE_CONST_STRING(MEDIA_TYPE_FORM_URL_ENCODED,		"application/x-www-form-urlencoded");
DECLARE_CONST_STRING(MEDIA_TYPE_JSON,					"application/json");
DECLARE_CONST_STRING(MEDIA_TYPE_JSON_REQUEST,			"application/jsonrequest");
DECLARE_CONST_STRING(MEDIA_TYPE_MULTIPART_FORM,			"multipart/form-data");
DECLARE_CONST_STRING(MEDIA_TYPE_HTML,					"text/html");
DECLARE_CONST_STRING(MEDIA_TYPE_TEXT,					"text/plain");
DECLARE_CONST_STRING(MEDIA_TYPE_TEXT_PREFIX,			"text/");

DECLARE_CONST_STRING(METHOD_GET,						"GET");

DECLARE_CONST_STRING(PROTOCOL_HTTPS,					"https");

DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_MULTIPART,		"Error parsing MIME multipart/form-data.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_JSON,			"Error parsing JSON.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_FORM_URL_ENCODED,	"Error parsing form URL encoded.");
DECLARE_CONST_STRING(ERR_UNSUPPORTED_MEDIA_TYPE,		"Unsupported media type: %s.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_RECEIVE,				"Unable to receive data from %s; connection lost.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_SEND,				"Unable to send data to %s; connection lost.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_CONNECT,				"Unable to connect to server at %s.");
DECLARE_CONST_STRING(ERR_INVALID_PORT,					"Unable to determine port from URL: %s.");
DECLARE_CONST_STRING(ERR_INVALID_URL,					"Invalid URL: %s.");

bool CHTTPUtil::ConvertBodyToDatum (const CHTTPMessage &Message, CDatum &retdBody)

//	ConvertBodyToDatum
//
//	Converts the body of a message to a datum, based on the content-type.

	{
	//	Edge condition. This is valid (e.g.) if we have a pure GET request.

	IMediaTypePtr pBody = Message.GetBody();
	if (!pBody)
		{
		retdBody = CDatum();
		return true;
		}

	//	Get the media and media type

	CString sMediaType;
	if (!CHTTPMessage::ParseHeaderValue(pBody->GetMediaType(), &sMediaType, NULL))
		{
		retdBody = strPattern(ERR_UNSUPPORTED_MEDIA_TYPE, pBody->GetMediaType());
		return false;
		}

	const CString &sBuffer = pBody->GetMediaBuffer();

	//	Parse based on media type

	if (strEquals(sMediaType, MEDIA_TYPE_FORM_URL_ENCODED))
		{
		if (!ConvertFormURLEncodedToDatum(sBuffer, retdBody))
			{
			retdBody = ERR_UNABLE_TO_PARSE_FORM_URL_ENCODED;
			return false;
			}
		}
	else if (strEquals(sMediaType, MEDIA_TYPE_JSON))
		{
		CStringBuffer Buffer(sBuffer);
		if (!CDatum::Deserialize(CDatum::EFormat::JSON, Buffer, &retdBody))
			{
			retdBody = ERR_UNABLE_TO_PARSE_JSON;
			return false;
			}
		}
	else if (strEquals(sMediaType, MEDIA_TYPE_MULTIPART_FORM))
		{
		CBuffer Buffer(pBody->GetMediaBuffer());
		CHTTPMultipartParser Parser(pBody->GetMediaType(), Buffer);
		if (!Parser.ParseAsDatum(retdBody))
			{
			retdBody = ERR_UNABLE_TO_PARSE_MULTIPART;
			return false;
			}
		}
	else if (strStartsWith(sMediaType, MEDIA_TYPE_TEXT_PREFIX))
		{
		retdBody = sBuffer;
		}
	else
		{
		retdBody = strPattern(ERR_UNSUPPORTED_MEDIA_TYPE, sMediaType);
		return false;
		}

	//	Done

	return true;
	}

bool CHTTPUtil::ConvertFormURLEncodedToDatum (const CString &sText, CDatum &retdValue)

//	ConvertFormURLEncodedToDatum
//
//	Converts from x-www-form-urlencoded to a struct

	{
	enum class EState
		{
		Start,
		Skip,
		FieldName,
		FieldValue,
		Escape,
		Done,
		};

	CComplexStruct *pResult = new CComplexStruct;

	char *pPos = sText.GetParsePointer();
	char *pPosEnd = pPos + sText.GetLength();
	EState iState = EState::Start;
	EState iOldState;

	CMemoryBuffer Token;
	CString sFieldName;

	while (iState != EState::Done)
		{
		switch (iState)
			{
			case EState::Start:
				if (pPos == pPosEnd)
					iState = EState::Done;
				else if (*pPos == '&')
					break;
				else if (*pPos == '=')
					iState = EState::Skip;
				else
					{
					Token.SetLength(0);
					Token.Write(pPos, 1);
					iState = EState::FieldName;
					}

				break;

			case EState::Skip:
				if (pPos == pPosEnd)
					iState = EState::Done;
				else if (*pPos == '&')
					iState = EState::Start;
				break;

			case EState::FieldName:
				if (pPos == pPosEnd)
					iState = EState::Done;
				else if (*pPos == '=')
					{
					sFieldName = CString(Token.GetPointer(), Token.GetLength());
					Token.SetLength(0);
					iState = EState::FieldValue;
					}
				else if (*pPos == '+')
					Token.Write(" ", 1);
				else if (*pPos == '%')
					{
					iOldState = iState;
					iState = EState::Escape;
					}
				else
					Token.Write(pPos, 1);
				break;

			case EState::FieldValue:
				if (pPos == pPosEnd
						|| *pPos == '&')
					{
					pResult->SetElement(sFieldName, CString(Token.GetPointer(), Token.GetLength()));
					Token.SetLength(0);
					iState = ((pPos == pPosEnd) ? EState::Done : EState::Start);
					}
				else if (*pPos == '+')
					Token.Write(" ", 1);
				else if (*pPos == '%')
					{
					iOldState = iState;
					iState = EState::Escape;
					}
				else
					Token.Write(pPos, 1);
				break;

			case EState::Escape:
				if (pPos+1 >= pPosEnd)
					iState = EState::Done;
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

		if (iState != EState::Done)
			pPos++;
		else
			break;
		}

	//	Return result

	if (pResult->GetCount() == 0)
		{
		delete pResult;
		retdValue = CDatum();
		return true;
		}

	retdValue = CDatum(pResult);
	return true;
	}

CDatum CHTTPUtil::ConvertHeadersToDatum (const CHTTPMessage &Message)

//	ConvertHeadersToDatum
//
//	Converts the headers in an HTTP message to a struct.

	{
	CDatum dResult(CDatum::typeStruct);
	for (int i = 0; i < Message.GetHeaderCount(); i++)
		{
		CString sHeader;
		CString sValue;

		Message.GetHeader(i, &sHeader, &sValue);

		//	If this header is already in the structure, then we need to append it.
		//
		//	NOTE: CHTTPMessage always converts headers to lowercase for ease of
		//	compare, since the HTTP spec says header names are case-insensitive.

		const CString &sOriginalData = dResult.GetElement(sHeader);
		if (!sOriginalData.IsEmpty())
			dResult.SetElement(sHeader, strPattern("%s, %s", sOriginalData, sValue));
		
		//	Otherwise we just set it

		else
			dResult.SetElement(sHeader, sValue);
		}

	//	Done

	if (dResult.GetCount() > 0)
		return dResult;
	else
		return CDatum();
	}

CDatum CHTTPUtil::DecodeResponse (const CHTTPMessage &Response)
	{
	CDatum dResult(CDatum::typeArray);
	dResult.Append((int)Response.GetStatusCode());
	dResult.Append(Response.GetStatusMsg());
	dResult.Append(ConvertHeadersToDatum(Response));

	CDatum dBody;
	if (false)
		dBody = CDatum(Response.GetBodyBuffer());
	else
		{
		ConvertBodyToDatum(Response, dBody);
		}

	//	OK if we get an error; dBody will be the error.

	dResult.Append(dBody);

	//	Done

	return dResult;
	}

CHTTPMessage CHTTPUtil::EncodeRequest (const CString &sMethod, const CString &sHost, const CString &sPath, CDatum dHeaders, CDatum dBody)
	{
	CHTTPMessage Request;
	Request.InitRequest(sMethod, sPath);

	//	Add headers

	bool bFoundHost = false;
	for (int i = 0; i < dHeaders.GetCount(); i++)
		{
		Request.AddHeader(dHeaders.GetKey(i), (const CString &)dHeaders.GetElement(i));
		if (strEquals(strToLower(dHeaders.GetKey(i)), HEADER_HOST))
			bFoundHost = true;
		}

	//	Add body

	if (!dBody.IsNil())
		{
		CString sMediaType;
		if (!Request.FindHeader(HEADER_CONTENT_TYPE, &sMediaType))
			sMediaType = MEDIA_TYPE_TEXT;

		IMediaTypePtr pBody = IMediaTypePtr(new CRawMediaType);
		pBody->DecodeFromBuffer(sMediaType, CStringBuffer(dBody.AsString()));

		Request.SetBody(pBody);
		}

	//	If there is no Host header and we have a host, then add it

	if (!bFoundHost && !sHost.IsEmpty())
		Request.AddHeader(HEADER_HOST, sHost);

	//	Done

	return Request;
	}

bool CHTTPUtil::RPC (const CString &sProtocol, const CString &sHostname, DWORD dwPort, const CHTTPMessage &Request, CHTTPMessage &retResponse, CDatum &retdResult)
	{
	bool bUseSSL = strEquals(sProtocol, PROTOCOL_HTTPS);

	//	Serialize request

	CStringBuffer RequestBuffer;
	Request.WriteToBuffer(RequestBuffer);

	//	Connect to the host

	CSSLSocketStream SocketStream;
	if (!SocketStream.Connect(sHostname, dwPort, bUseSSL, NULL))
		{
		retdResult = strPattern(ERR_UNABLE_TO_CONNECT, Request.GetRequestedURL());
		return false;
		}

	//	Write out (synchronously, for now)

	int iSent = SocketStream.Write(RequestBuffer.GetPointer(), RequestBuffer.GetLength());
	if (iSent != RequestBuffer.GetLength())
		{
		retdResult = strPattern(ERR_UNABLE_TO_SEND, Request.GetRequestedURL());
		return false;
		}

	//	Keep reading until we have a full HTTP message (or until we get an error)

	if (!retResponse.InitFromStream(SocketStream))
		{
		retdResult = strPattern(ERR_UNABLE_TO_RECEIVE, Request.GetRequestedURL());
		return false;
		}

	return true;
	}
