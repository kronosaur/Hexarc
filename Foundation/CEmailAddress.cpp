//	CEmailAddress.cpp
//
//	CEmailAddress class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CString CEmailAddress::AsString () const

//	AsString
//
//	Returns the email address as a string.

	{
	if (!IsValid())
		return NULL_STR;
	else if (m_sDisplayName.IsEmpty())
		return strPattern("%s@%s", m_sLocalPart, m_sDomain);
	else
		return strPattern("%s <%s@%s>", m_sDisplayName, m_sLocalPart, m_sDomain);
	}

CString CEmailAddress::GetAddressOnly () const

//	GetAddressOnly
//
//	Returns the address only (no display name).

	{
	if (!IsValid())
		return NULL_STR;
	else
		return strPattern("%s@%s", m_sLocalPart, m_sDomain);
	}

bool CEmailAddress::Parse (const CString& sAddr, CEmailAddress* retAddr)

//	Parse
//
//	Parses as string as an email address. Returns TRUE if it is a valid address.

	{
	CString sDisplayName;
	CString sLocalPart;
	CString sDomain;

	const char *pPos = sAddr.GetParsePointer();

	//	Skip leading whitespace.

	while (strIsWhitespace(pPos))
		pPos++;

	//	Look for an open angle-bracket.

	const char *pStart = pPos;
	while (*pPos != '<' && *pPos != '\0')
		pPos++;

	//	If we have an angle-bracket, then we need to parse a display name.

	if (*pPos == '<' && *pStart != '"')
		{
		//	Back up if we've got whitespace.

		const char *pEnd = pPos;
		while (pEnd > pStart && strIsWhitespace(pEnd-1))
			pEnd--;

		sDisplayName = CString(pStart, pEnd - pStart);

		//	Start parsing after the angle brackets.

		pPos++;
		while (strIsWhitespace(pPos))
			pPos++;
		}

	//	Otherwise, reset to the beginning.

	else
		pPos = pStart;

	//	Now parse the local part of the address.

	pStart = pPos;

	//	If we have a quoted local part, then we need to parse

	if (*pPos == '"')
		{
		pPos++;

		bool bBackslash = false;

		while (true)
			{
			if (*pPos == '\0')
				break;
			else if (bBackslash)
				{
				pPos++;
				bBackslash = false;
				}
			else if (*pPos == '\\')
				{
				pPos++;
				bBackslash = true;
				}
			else if (*pPos == '"')
				{
				pPos++;
				break;
				}
			else
				pPos++;
			}
		}

	//	Otherwise, this is plain.

	else
		{
		//	If we start with a paren, then this is a comment.

		if (*pPos == '(')
			{
			while (*pPos != ')' && *pPos != '\0')
				pPos++;

			if (*pPos == ')')
				pPos++;
			else
				return false;
			}

		//	Parse

		bool bFirst = true;
		bool bDot = false;
		while (true)
			{
			if (bDot)
				{
				if (*pPos == '.')
					return false;
				else
					bDot = false;
				}

			if (*pPos == '\0')
				return false;
			else if (*pPos >= 'A' && *pPos <= 'Z')
				pPos++;
			else if (*pPos >= 'a' && *pPos <= 'z')
				pPos++;
			else if (*pPos >= '0' && *pPos <= '9')
				pPos++;
			else if (*pPos == '!' || *pPos == '#' || *pPos == '$' || *pPos == '%' || *pPos == '&' || *pPos == '\'' || *pPos == '*'
					|| *pPos == '+' || *pPos == '-' || *pPos == '/' || *pPos == '=' || *pPos == '?' || *pPos == '^'
					|| *pPos == '_' || *pPos == '`' || *pPos == '{' || *pPos == '|' || *pPos == '}' || *pPos == '~')
				pPos++;
			else if (*pPos == '.')
				{
				bDot = true;
				pPos++;
				}
			else if (*pPos == '(')
				break;
			else if (*pPos == '@')
				break;
			else
				return false;
			}

		//	May end in a comment.

		if (*pPos == '(')
			{
			while (*pPos != ')' && *pPos != '\0')
				pPos++;

			if (*pPos == ')')
				pPos++;
			else
				return false;
			}
		}

	//	Done parsing the local part

	sLocalPart = CString(pStart, pPos - pStart);
	if (sLocalPart.IsEmpty())
		return false;

	if (sLocalPart.GetLength() > MAX_LOCAL_PART_CHARS)
		return false;

	//	Must has an @ sign.

	if (*pPos == '@')
		pPos++;
	else
		return false;

	//	Now parse the domain

	pStart = pPos;
	while (*pPos != '>' && *pPos != '\0' && !strIsWhitespace(pPos))
		pPos++;

	sDomain = CString(pStart, pPos - pStart);
	if (sDomain.IsEmpty())
		return false;

	//	Validate domain

	if (!ValidateDomain(sDomain))
		return false;

	//	Success!

	if (retAddr)
		{
		retAddr->m_sDisplayName = sDisplayName;
		retAddr->m_sLocalPart = sLocalPart;
		retAddr->m_sDomain = sDomain;
		}

	return true;
	}

bool CEmailAddress::ValidateDomain (const CString& sDomain)
	{
	const char *pPos = sDomain.GetParsePointer();
	if (*pPos == '[')
		{
		while (*pPos != ']' && *pPos != '\0')
			pPos++;

		if (*pPos == ']')
			pPos++;
		else
			return false;

		if (*pPos != '\0')
			return false;

		return true;
		}
	else
		{
		while (true)
			{
			const char *pStart = pPos;
			while (true)
				{
				if (*pPos == '\0')
					break;
				else if (*pPos == '.')
					break;
				else if (*pPos >= 'A' && *pPos <= 'Z')
					pPos++;
				else if (*pPos >= 'a' && *pPos <= 'z')
					pPos++;
				else if (*pPos >= '0' && *pPos <= '9')
					pPos++;
				else if (*pPos == '-')
					pPos++;
				else
					return false;
				}

			if (pStart == pPos)
				return false;
			else if (*pStart == '-' || pPos[-1] == '-')
				return false;
			else if (*pPos == '\0')
				break;
			else if (*pPos == '.')
				pPos++;
			else
				return false;
			}

		return true;
		}
	}
