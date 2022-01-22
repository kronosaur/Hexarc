//	HTTPUtil.h
//
//	HTTP Utilities
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "Foundation.h"
#include "AEON.h"
#include "OpenSSLUtil.h"

class CHTTPUtil
	{
	public:
		static bool ConvertBodyToDatum (const CHTTPMessage &Message, CDatum &retdBody);
		static bool ConvertFormURLEncodedToDatum (const CString &sText, CDatum &retdValue);
		static CDatum ConvertHeadersToDatum (const CHTTPMessage &Message);
		static CDatum DecodeResponse (const CHTTPMessage &Response);
		static CDatum DecodeResponse (CDatum dResponse);
		static CHTTPMessage EncodeRequest (const CString &sMethod, const CString &sHost, const CString &sPath, CDatum dHeaders, CDatum dBody);
		static bool RPC (const CString &sProtocol, const CString &sHostname, DWORD dwPort, const CHTTPMessage &Request, CHTTPMessage &retResponse, CDatum &retdResult);

	private:

	};
