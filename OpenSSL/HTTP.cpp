//	HTTP.cpp
//
//	HTTP utilities
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(HEADER_CONTENT_TYPE,				"content-type")
DECLARE_CONST_STRING(HEADER_HOST,						"host")

DECLARE_CONST_STRING(MEDIA_TYPE_TEXT,					"text/plain")

DECLARE_CONST_STRING(PROTOCOL_HTTPS,					"https")

DECLARE_CONST_STRING(ERR_INVALID_URL,					"Invalid URL: %s.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_CONNECT,				"Unable to connect to server at %s.")
DECLARE_CONST_STRING(ERR_INVALID_PORT,					"Unable to determine port from URL: %s.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_RECEIVE,				"Unable to receive data from %s; connection lost.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_SEND,				"Unable to send data to %s; connection lost.")

CHTTPMessage httpRequest (const CString &sURL, const CString &sMethod, const TSortMap<CString, CString> *pHeaders, const IMemoryBlock *pBuffer)

//	httpRequest
//
//	Makes a synchronous HTTP request and returns a result.

	{
	int i;
	CHTTPMessage Response;

	//	Parse the URL

	CString sHost;
	CString sProtocol;
	CString sPath;
	if (!urlParse(sURL.GetParsePointer(), &sProtocol, &sHost, &sPath))
		{
		Response.InitResponse(http_BAD_REQUEST, strPattern(ERR_INVALID_URL, sURL));
		return Response;
		}

	CString sHostname;
	DWORD dwPort;
	if (!urlParseHostPort(sProtocol, sHost, &sHostname, &dwPort))
		{
		Response.InitResponse(http_BAD_REQUEST, strPattern(ERR_INVALID_PORT, sURL));
		return Response;
		}

	bool bUseSSL = strEquals(sProtocol, PROTOCOL_HTTPS);

	//	Connect to the host

	CSSLSocketStream SocketStream;
	if (!SocketStream.Connect(sHostname, dwPort, bUseSSL, NULL))
		{
		Response.InitResponse(http_NOT_FOUND, strPattern(ERR_UNABLE_TO_CONNECT, sURL));
		return Response;
		}

	//	Serialize to an HTTP message buffer

	CHTTPMessage Request;
	Request.InitRequest(sMethod, sPath);

	bool bFoundHost = false;
	if (pHeaders)
		{
		for (i = 0; i < pHeaders->GetCount(); i++)
			{
			Request.AddHeader(pHeaders->GetKey(i), pHeaders->GetValue(i));
			if (strEquals(strToLower(pHeaders->GetKey(i)), HEADER_HOST))
				bFoundHost = true;
			}
		}

	//	Add body

	if (pBuffer)
		{
		CString sMediaType;
		if (!Request.FindHeader(HEADER_CONTENT_TYPE, &sMediaType))
			sMediaType = MEDIA_TYPE_TEXT;

		IMediaTypePtr pBody = IMediaTypePtr(new CRawMediaType);
		pBody->DecodeFromBuffer(sMediaType, *pBuffer);

		Request.SetBody(pBody);
		}

	//	If there is no Host header and we have a host, then add it

	if (!bFoundHost && !sHost.IsEmpty())
		Request.AddHeader(HEADER_HOST, sHost);

	//	Serialize the request into a buffer

	CStringBuffer RequestBuffer;
	Request.WriteToBuffer(RequestBuffer);

	//	Write out (synchronously, for now)

	int iSent = SocketStream.Write(RequestBuffer.GetPointer(), RequestBuffer.GetLength());
	if (iSent != RequestBuffer.GetLength())
		{
		Response.InitResponse(http_NOT_FOUND, strPattern(ERR_UNABLE_TO_SEND, sURL));
		return Response;
		}

	//	Keep reading until we have a full HTTP message (or until we get an error)

	if (!Response.InitFromStream(SocketStream))
		{
		Response.InitResponse(http_NOT_FOUND, strPattern(ERR_UNABLE_TO_RECEIVE, sURL));
		return Response;
		}

	SocketStream.Disconnect();

	//	Done

	return Response;
	}
