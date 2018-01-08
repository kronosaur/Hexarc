//	URLs.cpp
//
//	URL utilities
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(PROTOCOL_HTTP,						"http")
DECLARE_CONST_STRING(PROTOCOL_HTTPS,					"https")
DECLARE_CONST_STRING(PROTOCOL_MAIL_TO,					"mailto")

DECLARE_CONST_STRING(EMPTY_PATH,						"/")

CString urlAppend (const CString &sPath, const CString &sComponent)

//	urlAppend
//
//	Appends a component to an URL.

	{
	if (sPath.IsEmpty())
		return sComponent;

	if (sComponent.IsEmpty())
		return sPath;

	bool bPathEndsWithSlash = (*(sPath.GetParsePointer() + sPath.GetLength() - 1) == '/');
	bool bComponentStartsWithSlash = (*sComponent.GetParsePointer() == '/');

	if (bPathEndsWithSlash && bComponentStartsWithSlash)
		return sPath + strSubString(sComponent, 1);
	else if (!bPathEndsWithSlash && !bComponentStartsWithSlash)
		return strPattern("%s/%s", sPath, sComponent);
	else
		return sPath + sComponent;
	}

CString urlDecode (const CString &sValue)

//	urlDecode
//
//	Decodes an URL.
//	See: http://en.wikipedia.org/wiki/Query_string

	{
	CStringBuffer Output;

	char *pPos = sValue.GetParsePointer();
	char *pPosEnd = pPos + sValue.GetLength();
	char *pStart = pPos;
	while (pPos < pPosEnd)
		{
		if (*pPos == '+')
			{
			Output.Write(pStart, pPos - pStart);
			Output.Write(" ", 1);

			pPos++;
			pStart = pPos;
			}
		else if (*pPos == '%')
			{
			Output.Write(pStart, pPos - pStart);

			//	Move the expected number into a temp value because we can't
			//	guarantee that there will be a terminator for the number.
			//	(E.g., "%201" is equivalent to " 1").

			char szTemp[3];
			pPos++;
			szTemp[0] = (pPos < pPosEnd ? *pPos++ : 'X');
			szTemp[1] = (pPos < pPosEnd ? *pPos++ : 'X');
			szTemp[2] = '\0';

			//	Parse

			char chChar = strParseIntOfBase(szTemp, 16, '*');
			Output.Write(&chChar, 1);

			pStart = pPos;
			}
		else
			pPos++;
		}

	Output.Write(pStart, pPos - pStart);

	return CString::CreateFromHandoff(Output);
	}

CString urlEncodeParam (const CString &sValue)

//	urlEncodeParam
//
//	Encodes an URL query parameter.
//	See: http://en.wikipedia.org/wiki/Query_string

	{
	CStringBuffer Output;

	char *pPos = sValue.GetParsePointer();
	char *pPosEnd = pPos + sValue.GetLength();
	char *pStart = pPos;
	while (pPos < pPosEnd)
		{
		if (strIsASCIIAlphaNumeric(pPos) || *pPos == '.' || *pPos == '-' || *pPos == '~' || *pPos == '_')
			pPos++;
		else if (*pPos == ' ')
			{
			Output.Write(pStart, pPos - pStart);
			Output.Write("+", 1);

			pPos++;
			pStart = pPos;
			}
		else
			{
			Output.Write(pStart, pPos - pStart);

			CString sChar = strPattern("%%%02x", (DWORD)(BYTE)*pPos);
			Output.Write(sChar);

			pPos++;
			pStart = pPos;
			}
		}

	Output.Write(pStart, pPos - pStart);

	return CString::CreateFromHandoff(Output);
	}

DWORD urlGetDefaultPort (const CString &sProtocol)

//	urlGetDefaultPort
//
//	Returns the default port for the given protocol

	{
	CString sProtocolLC = strToLower(sProtocol);
	if (strEquals(sProtocolLC, PROTOCOL_HTTP))
		return 80;
	else if (strEquals(sProtocolLC, PROTOCOL_HTTPS))
		return 443;
	else
		return 0;
	}

bool urlMatchPattern (const CString &sPattern, const CString &sURL)

//	urlMatchPattern
//
//	Matches urls using a wildcard pattern. For now we only support the following patterns:
//
//	/test				Matches exact URL
//	/test/*				Matches URLS that start with /test/
//
//	SAMPLES
//
//	Pattern				URL				Result
//	----------------------------------------------
//	""					""				true
//	""					"a"				false
//	""					"ab"			false
//	"a"					""				false
//	"a"					"a"				true
//	"a"					"ab"			false
//	"*"					""				true
//	"*"					"a"				true
//	"*"					"ab"			true
//	"a*"				"a"				true
//	"a*"				"ab"			true

	{
	char *pPatternPos = sPattern.GetParsePointer();
	char *pURLPos = sURL.GetParsePointer();

	//	Match

	while (*pPatternPos != '\0' && *pURLPos != '\0')
		{
		//	If pattern is * then we match everything

		if (*pPatternPos == '*')
			return true;

		//	If we don't match then we're done

		if (*pPatternPos != *pURLPos)
			return false;

		//	Advance

		pPatternPos++;
		pURLPos++;
		}

	//	If pattern and url ended at the same time, then
	//	we have a match

	if ((*pPatternPos == '\0' && *pURLPos == '\0') 
			|| *pPatternPos == '*')
		return true;

	//	Otherwise we did not

	return false;
	}

bool urlParse (char *pStart, CString *retsProtocol, CString *retsHost, CString *retsPath, char **retpEnd)

//	urlParse
//
//	Parses an url of the following forms:
//
//	http://www.example.com/dir/file.ext
//	//www.example.com/dir/file.ext
//	www.example.com/dir/file.ext
//	/dir/file.exe

	{
	enum States
		{
		stateStart,
		stateStartPath,
		stateStartText,
		stateProtocolSlash1,
		stateProtocolSlash2,
		stateProtocol,
		stateMailTo,
		stateHost,
		statePath,
		};

	//	Edge-conditions

	if (pStart == NULL)
		{
		if (retpEnd)
			*retpEnd = NULL;
		return false;
		}

	//	Prepare

	CString sProtocol;
	CString sHost;
	CString sPath;

	//	Parse

	States iState = stateStart;
	char *pToken = pStart;
	char *pPos = pStart;
	while (true)
		{
		switch (iState)
			{
			case stateStart:
				if (*pPos == '/')
					{
					pToken = pPos;
					iState = stateStartPath;
					}
				else if (*pPos == ':' || *pPos == '\0' || strIsWhitespace(pPos))
					return false;
				else
					{
					pToken = pPos;
					iState = stateStartText;
					}

				break;

			case stateStartPath:
				if (*pPos == '\0' || strIsWhitespace(pPos))
					sPath = CString(pToken, (int)(pPos - pToken));
				else if (*pPos == '/')
					iState = stateProtocol;
				else
					iState = statePath;
				break;

			case stateStartText:
				if (*pPos == '/' || *pPos == '\0' || strIsWhitespace(pPos))
					{
					sHost = CString(pToken, (int)(pPos - pToken));
					pToken = pPos;
					iState = statePath;
					}
				else if (*pPos == ':')
					{
					CString sToken = CString(pToken, (int)(pPos - pToken));

					//	Handle mailto

					if (strEquals(sToken, PROTOCOL_MAIL_TO))
						{
						sProtocol = sToken;
						iState = stateMailTo;
						}

					//	If a number follows the colon, then this is just the port
					//	on the host

					else if (strIsDigit(pPos + 1))
						break;

					//	Otherwise this is the protocol

					else
						{
						sProtocol = sToken;
						pToken = pPos;
						iState = stateProtocolSlash1;
						}
					}

				break;

			case stateMailTo:
				//	We just skipped the colon; the rest is the "path"
				pToken = pPos;
				iState = statePath;
				break;

			case stateProtocolSlash1:
				//	We expect a first slash
				if (*pPos != '/')
					return false;

				iState = stateProtocolSlash2;
				break;

			case stateProtocolSlash2:
				//	We expect a second slash
				if (*pPos != '/')
					return false;

				iState = stateProtocol;
				break;

			case stateProtocol:
				pToken = pPos;

				//	Another slash means a null host

				if (*pPos == '/')
					iState = statePath;
				else
					iState = stateHost;
				break;

			case stateHost:
				if (*pPos == '/' || *pPos == '\0' || strIsWhitespace(pPos))
					{
					sHost = CString(pToken, (int)(pPos - pToken));
					pToken = pPos;
					iState = statePath;
					}
				break;

			case statePath:
				if (*pPos == '\0' || strIsWhitespace(pPos))
					sPath = CString(pToken, (int)(pPos - pToken));
				break;
			}

		if (*pPos == '\0')
			break;
		else
			pPos++;
		}

	//	Done

	if (retsProtocol)
		{
		if (sProtocol.IsEmpty())
			*retsProtocol = PROTOCOL_HTTP;
		else
			*retsProtocol = sProtocol;
		}

	if (retsHost)
		*retsHost = sHost;

	if (retsPath)
		{
		if (sPath.IsEmpty())
			*retsPath = EMPTY_PATH;
		else
			*retsPath = sPath;
		}

	if (retpEnd)
		*retpEnd = pPos;

	return true;
	}

bool urlParseHostPort (const CString &sProtocol, const CString &sHost, CString *retsHostname, DWORD *retdwPort)

//	urlParseHostPort
//
//	Attempts to figure out the hostname and port using the following methods:
//
//	1.	If the port is encoded in sHost, then we return the hostname along and 
//		the port.
//
//	2.	If the protocol is well-known, then we return the port.
//
//	Otherwise, we return FALSE.

	{
	char *pPos = sHost.GetParsePointer();
	char *pStart = pPos;
	while (*pPos != ':' && *pPos != '\0')
		pPos++;

	if (retsHostname)
		*retsHostname = CString(pStart, pPos - pStart);

	//	If the port is encoded in the host, then we return that.

	DWORD dwPort;
	if (*pPos == ':')
		{
		pPos++;
		dwPort = strParseInt(pPos, 0);
		if (dwPort == 0)
			return false;
		}
	
	//	Otherwise we try to figure it out based on the protocol

	else
		{
		dwPort = urlGetDefaultPort(sProtocol);
		if (dwPort == 0)
			return false;
		}

	//	Done

	if (retdwPort)
		*retdwPort = dwPort;

	return true;
	}
