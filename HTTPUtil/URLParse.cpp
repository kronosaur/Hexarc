//	URLParse.cpp
//
//	CAEONURL class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(PROTOCOL_HTTP,					"http");
DECLARE_CONST_STRING(PROTOCOL_HTTPS,				"https");
DECLARE_CONST_STRING(PROTOCOL_MAILTO,				"mailto");

CString CAEONURL::ComposeURL (const CString& sProtocol, const CString& sHostname, DWORD dwPort, const CString& sPath, CDatum dQuery, const CString& sFragment)

//	ComposeURL
//
//	Composes an URL from the components, encoding as appropriate.

	{
	CStringBuffer Output;

	//	We only write known protocols (for now).

	if (sProtocol.IsEmpty() || strEqualsNoCase(sProtocol, PROTOCOL_HTTPS))
		Output.Write("https://", 8);
	else if (strEqualsNoCase(sProtocol, PROTOCOL_HTTP))
		Output.Write("http://", 7);
	else if (strEqualsNoCase(sProtocol, PROTOCOL_MAILTO))
		Output.Write("mailto:", 7);
	else
		return NULL_STR;

	//	Validate the hostname. It must be a valid IPv6 address or a valid hostname.

	if (!ValidateHostname(sHostname))
		return NULL_STR;

	//	Write the hostname

	Output.Write(sHostname);

	//	If we have a port, write it out.

	if (dwPort != 0)
		{
		Output.WriteChar(':');
		Output.Write(strFromInt(dwPort));
		}

	//	Write the path.

	if (!sPath.IsEmpty())
		{
		const char* pPos = sPath.GetParsePointer();

		//	Path must be absolute.

		if (*pPos != '/')
			Output.WriteChar('/');

		//	Encode the path.

		if (!EncodePath(Output, sPath))
			return NULL_STR;
		}

	//	Write the query string.

	if (!dQuery.IsNil())
		{
		Output.WriteChar('?');
		
		bool bNeedAmp = false;
		for (int i = 0; i < dQuery.GetCount(); i++)
			{
			CString sKey = dQuery.GetKey(i);
			CString sValue = dQuery.GetElement(i).AsString();
					
			if (bNeedAmp)
				Output.WriteChar('&');
					
			Output.Write(sKey);
			Output.WriteChar('=');
			Output.Write(urlEncodeParam(sValue));
					
			bNeedAmp = true;
			}
		}

	//	Write the fragment

	if (!sFragment.IsEmpty())
		{
		Output.WriteChar('#');

		if (!EncodeFragment(Output, sFragment))
			return NULL_STR;
		}

	//	Done

	return CString(std::move(Output));
	}

bool CAEONURL::ParseURL (const CString& sURL, CString* retsProtocol, CString* retsHostname, DWORD* retdwPort, CString *retsPath, CDatum* retdQuery, CString *retsFragment)

//	ParseURL
//
//	Parse an URL and return components. If the URL is not valid, we return FALSE
//	and all return values are undefined.
//
//	NOTE: We return port 0 for the default port for the protocol.

	{
	const char *pPos = sURL.GetParsePointer();

	//	Protocol

	if (!ParseProtocol(pPos, retsProtocol))
		return false;

	//	Now parse the hostname

	if (!ParseHostname(pPos, retsHostname))
		return false;

	//	If we have a port, parse it

	if (*pPos == ':')
		{
		pPos++;
		
		DWORD dwPort = strParseInt(pPos, 0, &pPos);
		if (retdwPort)
			*retdwPort = dwPort;
		}
	else
		{
		if (retdwPort)
			*retdwPort = 0;
		}

	//	If we have a path, parse it

	if (!ParsePath(pPos, retsPath))
		return false;

	//	If we have a query, parse it

	if (!ParseQuery(pPos, retdQuery))
		return false;

	//	Lastly, parse the fragment.

	if (!ParseFragment(pPos, retsFragment))
		return false;

	//	Done

	return true;
	}

bool CAEONURL::EncodeFragment (CStringBuffer& Output, const CString& sFragment)

//	EncodeFragment
//
//	Encodes a fragment.

	{
	const char* pPos = sFragment.GetParsePointer();

	//	A fragment can have A-Z, a-z, 0-9, -, ., _, ~. In addition, fragments
	//	can use general delimiters and sub-delimiters. All other characters
	//	must be encoded.

	while (*pPos != '\0')
		{
		if (strIsASCIIAlphaNumeric(pPos) || *pPos == '-' || *pPos == '.' || *pPos == '_' || *pPos == '~' 
				|| *pPos == ':' || *pPos == '/' || *pPos == '?' || *pPos == '#' || *pPos == '[' || *pPos == ']' || *pPos == '@'
				|| *pPos == '!' || *pPos == '$' || *pPos == '&' || *pPos == '\'' || *pPos == '(' || *pPos == ')' || *pPos == '*' || *pPos == '+' || *pPos == ',' || *pPos == ';' || *pPos == '=')
			Output.WriteChar(*pPos);
		else
			{
			Output.WriteChar('%');
			Output.WriteChar(strEncodeHexDigit((BYTE)*pPos / 16));
			Output.WriteChar(strEncodeHexDigit((BYTE)*pPos % 16));
			}
				
		pPos++;
		}

	return true;
	}

bool CAEONURL::EncodePath (CStringBuffer& Output, const CString& sPath)

//	EncodePath
//
//	Encodes a path.

	{
	const char* pPos = sPath.GetParsePointer();

	//	A path can have A-Z, a-z, 0-9, -, ., _, ~, and /. All other characters
	//	must be encoded.

	while (*pPos != '\0')
		{
		if (strIsASCIIAlphaNumeric(pPos) || *pPos == '-' || *pPos == '.' || *pPos == '_' || *pPos == '~' || *pPos == '/')
			Output.WriteChar(*pPos);
		else
			{
			Output.WriteChar('%');
			Output.WriteChar(strEncodeHexDigit((BYTE)*pPos / 16));
			Output.WriteChar(strEncodeHexDigit((BYTE)*pPos % 16));
			}
		
		pPos++;
		}

	return true;
	}

bool CAEONURL::ParseFragment (const char*& pPos, CString* retsFragment)

//	ParseFragment
//
//	Parse the fragment.

	{
	//	If no # then no fragment

	if (*pPos != '#')
		{
		if (retsFragment)
			*retsFragment = NULL_STR;
		return true;
		}

	//	Otherwise, we have a fragment

	pPos++;
	const char* pStart = pPos;

	bool bNeedDecoding = false;
	while (*pPos != '\0')
		{
		if (*pPos == '%')
			bNeedDecoding = true;

		pPos++;
		}

	if (retsFragment)
		{
		if (bNeedDecoding)
			//	NOTE: We don't decode + to space because they are literals in fragments.
			*retsFragment = ::urlDecode(CString(pStart, pPos - pStart), true);
		else
			*retsFragment = CString(pStart, pPos - pStart);
		}

	return true;
	}

bool CAEONURL::ParseHostname (const char*& pPos, CString* retsHostname)

//	ParseHostname
//
//	Parse a hostname.

	{
	const char* pStart = pPos;

	//	If the first character is a bracket, then this is an IPv6 address

	if (*pPos == '[')
		{
		pPos++;
		
		//	Find the end bracket

		while (*pPos != '\0' && *pPos != ']')
			pPos++;

		//	If we don't have a closing bracket, then this is not a valid IPv6
		//	address.

		if (*pPos != ']')
			return false;

		//	Done

		pPos++;
		if (retsHostname)
			*retsHostname = CString(pStart, pPos - pStart);

		return true;
		}

	//	Otherwise, this is a hostname

	else
		{
		//	First character must be a letter or a digit

		if (!strIsASCIIAlphaNumeric(pPos))
			return false;

		//	Subsequent characters can be letters, digits, -, or .

		pPos++;
		while (*pPos != '\0' && *pPos != ':' && *pPos != '/' && *pPos != '?' && *pPos != '#')
			{
			if (!strIsASCIIAlphaNumeric(pPos) && *pPos != '-' && *pPos != '.')
				return false;
		
			pPos++;
			}

		//	Done

		if (retsHostname)
			*retsHostname = CString(pStart, pPos - pStart);

		return true;
		}
	}

bool CAEONURL::ParsePath (const char*& pPos, CString* retsPath)

//	ParsePath
//
//	Parse the path.

	{
	//	If no slash, then we have a blank path.

	if (*pPos != '/')
		{
		if (retsPath)
			*retsPath = NULL_STR;
		return true;
		}

	//	Otherwise, we have a path

	const char* pStart = pPos;
	pPos++;

	bool bNeedDecoding = false;
	while (*pPos != '\0' && *pPos != '?' && *pPos != '#')
		{
		if (*pPos == '%')
			bNeedDecoding = true;
		else if (!strIsASCIIAlphaNumeric(pPos) && *pPos != '-' && *pPos != '.' && *pPos != '_' && *pPos != '~' && *pPos != '/')
			return false;

		pPos++;
		}

	if (retsPath)
		{
		if (bNeedDecoding)
			*retsPath = ::urlDecode(CString(pStart, pPos - pStart));
		else
			*retsPath = CString(pStart, pPos - pStart);
		}

	return true;
	}

bool CAEONURL::ParseProtocol (const char*& pPos, CString* retsProtocol)

//	ParseProtocol
//
//	Parses the protocol. If we have a valid protocol, we return TRUE and 
//	retsProtocol is initialized. Otherwise, we return FALSE.

	{
	//	Protocol

	const char* pStart = pPos;
	while (*pPos != ':' && *pPos != '/' && *pPos != '\0')
		pPos++;

	//	If we don't have a colon then we don't have an explicit protocol, so we
	//	default to HTTPS

	if (*pPos != ':')
		{
		pPos = pStart;
		if (retsProtocol)
			*retsProtocol = PROTOCOL_HTTPS;
		return true;
		}

	const char* pEnd = pPos;
	pPos++;

	//	Parse double-slash

	bool bHasDoubleSlash = false;
	if (*pPos == '/')
		{
		pPos++;
		if (*pPos == '/')
			{
			pPos++;
			bHasDoubleSlash = true;
			}
		}

	//	If we don't have two slashes, then this is either mailto: or it is not
	//	a protocol.

	if (!bHasDoubleSlash)
		{
		CString sProtocol(pStart, pEnd - pStart);
		if (strEqualsNoCase(sProtocol, PROTOCOL_MAILTO))
			{
			if (retsProtocol)
				*retsProtocol = PROTOCOL_MAILTO;
			return true;
			}

		//	If this is HTTP or HTTPS, then this is a bad URL (no //).
		
		else if (strEqualsNoCase(sProtocol, PROTOCOL_HTTP) || strEqualsNoCase(sProtocol, PROTOCOL_HTTPS))
			return false;

		//	Otherwise, this is not a protocol we recognize, so we assume it is a
		//	hostname with a port (and default to HTTPS).

		else
			{
			pPos = pStart;
			if (retsProtocol)
				*retsProtocol = PROTOCOL_HTTPS;
			return true;
			}
		}

	//	If we get this far, then we've got a protocol. Validate it to make sure
	//	it doesn't have any illegal characters.

	CString sProtocol(pStart, pEnd - pStart);
	if (!ValidateProtocol(sProtocol))
		return false;

	//	Done

	if (retsProtocol)
		*retsProtocol = std::move(sProtocol);

	return true;
	}

bool CAEONURL::ParseQuery (const char*& pPos, CDatum* retdQuery)

//	ParseQuery
//
//	Parses a query.

	{
	//	If no ? then we have no query

	if (*pPos != '?')
		{
		if (retdQuery)
			*retdQuery = CDatum();
		return true;
		}

	//	Otherwise, we have a query

	if (retdQuery)
		*retdQuery = CDatum(CDatum::typeStruct);

	pPos++;
	while (*pPos != '#' && *pPos != '\0')
		{
		//	Parse the key

		bool bNeedDecoding = false;
		const char* pKeyStart = pPos;
		while (*pPos != '=' && *pPos != '\0')
			{
			if (*pPos == '%' || *pPos == '+')
				bNeedDecoding = true;
			else if (!strIsASCIIAlphaNumeric(pPos) && *pPos != '-' && *pPos != '.' && *pPos != '_' && *pPos != '~')
				return false;

			pPos++;
			}

		CString sKey;
		if (bNeedDecoding)
			sKey = ::urlDecode(CString(pKeyStart, pPos - pKeyStart));
		else
			sKey = CString(pKeyStart, pPos - pKeyStart);

		if (sKey.IsEmpty())
			return false;

		//	Parse the value

		if (*pPos != '=')
			return false;

		pPos++;
		bNeedDecoding = false;
		const char* pValueStart = pPos;
		while (*pPos != '&' && *pPos != '#' && *pPos != '\0')
			{
			if (*pPos == '%' || *pPos == '+')
				bNeedDecoding = true;
			else if (!strIsASCIIAlphaNumeric(pPos) && *pPos != '-' && *pPos != '.' && *pPos != '_' && *pPos != '~')
				return false;
			
			pPos++;
			}

		if (retdQuery)
			{
			CString sValue;
			if (bNeedDecoding)
				sValue = ::urlDecode(CString(pValueStart, pPos - pValueStart));
			else
				sValue = CString(pValueStart, pPos - pValueStart);

			//	Add to query

			CDatum dValue;
			if (CDatum::CreateFromStringValue(sValue, &dValue))
				retdQuery->SetElement(sKey, dValue);
			else
				retdQuery->SetElement(sKey, sValue);
			}

		//	Next

		if (*pPos == '&')
			pPos++;
		}

	return true;
	}

bool CAEONURL::ValidateHostname (const CString& sHostname)

//	ValidateHostname
//
//	Returns TRUE if this is a valid hostname.

	{
	const char* pPos = sHostname.GetParsePointer();

	//	If we start with a bracket, then this is an IPv6 address

	if (*pPos == '[')
		{
		pPos++;

		//	We can have hex digits and colons.

		while (*pPos != ']' && *pPos != '\0')
			{
			if (!strIsHexDigit(pPos) && *pPos != ':')
				return false;

			pPos++;
			}

		if (*pPos != ']')
			return false;
		}

	//	Otherwise, expect a valid hostname.

	else
		{
		if (!strIsASCIIAlphaNumeric(pPos))
			return false;

		pPos++;
		while (*pPos != '\0')
			{
			if (!strIsASCIIAlphaNumeric(pPos) && *pPos != '-' && *pPos != '.')
				return false;

			pPos++;
			}
		}

	return true;
	}

bool CAEONURL::ValidateProtocol (const CString& sProtocol)

//	ValidateProtocol
//
//	Returns TRUE if this is a valid protocol.

	{
	const char* pPos = sProtocol.GetParsePointer();

	//	First character must be a letter

	if (!strIsASCIIAlpha(pPos))
		return false;

	//	Subsequent characters can be letters, digits, +, -, or .

	pPos++;
	while (*pPos != '\0')
		{
		if (!strIsASCIIAlphaNumeric(pPos) && *pPos != '+' && *pPos != '-' && *pPos != '.')
			return false;

		pPos++;
		}

	//	OK

	return true;
	}
