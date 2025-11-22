//	HTTPUtil.h
//
//	HTTP Utilities
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "Foundation.h"
#include "AEON.h"
#include "OpenSSLUtil.h"

class CHTTPUtil
	{
	public:

		static bool Boot ();
		static bool ConvertBodyToDatum (const CHTTPMessage &Message, CDatum &retdBody);
		static bool ConvertFormURLEncodedToDatum (const CString &sText, CDatum &retdValue);
		static CDatum ConvertHeadersToDatum (const CHTTPMessage &Message);
		static CDatum CreateURL (const CString& sURL, CDatum dComponents);
		static CDatum CreateXMLElement (const IMemoryBlock& FileData);
		static CDatum DecodeResponse (const CHTTPMessage &Response);
		static CDatum DecodeResponse (CDatum dResponse);
		static CHTTPMessage EncodeRequest (const CString &sMethod, const CString &sHost, const CString &sPath, CDatum dHeaders, CDatum dBody);
		static bool RPC (const CString &sProtocol, const CString &sHostname, DWORD dwPort, const CHTTPMessage &Request, CHTTPMessage &retResponse, CDatum &retdResult);

		static DWORD URL_TYPE;
		static DWORD XML_ELEMENT_TYPE;

	private:

		static bool m_bAEONRegistered;
	};

class CHTTPMultipartParser
	{
	public:
		CHTTPMultipartParser (const CString &sMediaType, IMemoryBlock &Block) : 
				m_sMediaType(sMediaType),
				m_Block(Block)
			{ }

		CHTTPMultipartParser (const CHTTPMultipartParser &Src) = delete;
		CHTTPMultipartParser (CHTTPMultipartParser &&Src) = delete;

		~CHTTPMultipartParser ();

		CHTTPMultipartParser &operator= (const CHTTPMultipartParser &Src) = delete;
		CHTTPMultipartParser &operator= (CHTTPMultipartParser &&Src) = delete;

		bool ParseAsDatum (CDatum &retdBody);

	private:
		bool ParseToBoundary (const char *pPos, const char *pPosEnd, const CString &sBoundary, const CString &sPartType, CDatum &retdData, const char **retpPos = NULL) const;

		CString m_sMediaType;
		IMemoryBlock &m_Block;
		CComplexStruct *m_pResult = NULL;
	};
