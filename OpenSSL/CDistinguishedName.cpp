//	CDistinguishedName.cpp
//
//	CDistinguishedName class
//	Copyright (c) 2017 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(PART_C,							"C")
DECLARE_CONST_STRING(PART_CN,							"CN")
DECLARE_CONST_STRING(PART_L,							"L")
DECLARE_CONST_STRING(PART_O,							"O")
DECLARE_CONST_STRING(PART_OU,							"OU")
DECLARE_CONST_STRING(PART_ST,							"ST")
DECLARE_CONST_STRING(PART_EMAIL_ADDRESS,				"emailAddress")

DECLARE_CONST_STRING(STR_COMMA,							",")
DECLARE_CONST_STRING(STR_EQUALS,						"=")

const int OBJ_NAME_LEN =				128;

CDistinguishedName CDistinguishedName::m_Null;

CDistinguishedName::CDistinguishedName (const CString &sName)

//	CDistinguishedName constructor

	{
	Parse(sName, m_DN);
	}

CDistinguishedName::CDistinguishedName (OpenSSL_X509NamePtr pName)

//	CDistinguishedName constructor

	{
	int i;

	if (pName == NULL)
		return;

	//	Initialize from a name

	X509_NAME *pDN = COpenSSL::AsX509Name(pName);
	for (i = 0; i < X509_NAME_entry_count(pDN); i++)
		{
		X509_NAME_ENTRY *pEntry = X509_NAME_get_entry(pDN, i);

		//	Add a new entry to our list

		SPart *pPart = m_DN.Insert();

		//	Set the field name

		ASN1_OBJECT *pField = X509_NAME_ENTRY_get_object(pEntry);
		int nid = OBJ_obj2nid(pField);

		//	If this is an undefined NID, then get the text

		if (nid == NID_undef)
			{
			pPart->sField.SetLength(OBJ_NAME_LEN);
			int iLen = OBJ_obj2txt(pPart->sField.GetPointer(), OBJ_NAME_LEN, pField, 0);
			pPart->sField.SetLength(iLen);
			pPart->sFieldLong = pPart->sField;
			pPart->iPart = ParsePart(pPart->sField);
			}

		//	Otherwise, get the NID name

		else
			{
			const char *pNID = OBJ_nid2sn(nid);
			if (pNID)
				pPart->sField = CString(pNID);
			else
				pPart->sField = strFromInt(nid);

			pNID = OBJ_nid2ln(nid);
			if (pNID)
				pPart->sFieldLong = CString(pNID);
			else
				pPart->sFieldLong = pPart->sField;

			pPart->iPart = ParsePart(pPart->sField);
			}

		//	Set the value

		ASN1_STRING *pValue = X509_NAME_ENTRY_get_data(pEntry);
		pPart->sValue = COpenSSL::AsString(pValue);
		}
	}

bool CDistinguishedName::operator== (const CDistinguishedName &Src) const

//	operator ==

	{
	int i;

	if (m_DN.GetCount() != Src.m_DN.GetCount())
		return false;

	for (i = 0; i < m_DN.GetCount(); i++)
		if (!strEqualsNoCase(m_DN[i].sField, Src.m_DN[i].sField)
				|| !strEqualsNoCase(m_DN[i].sValue, Src.m_DN[i].sValue))
			return false;

	return true;
	}

bool CDistinguishedName::operator!= (const CDistinguishedName &Src) const

//	operator !=

	{
	return (*this == Src ? false : true);
	}

bool CDistinguishedName::AddPart (const CString &sPart, const CString &sValue, TArray<SPart> &DN)

//	AddPart
//
//	Adds a part to the array

	{
	SPart *pPart = DN.Insert();
	pPart->iPart = ParsePart(sPart);
	pPart->sField = sPart;
	pPart->sFieldLong = sPart;
	pPart->sValue = sValue;

	return true;
	}

void CDistinguishedName::CopyTo (OpenSSL_X509NamePtr pName) const

//	CopyTo
//
//	Copies to the name object

	{
	int i;

	//	First delete all

	DeleteAll(pName);

	//	Now add all our parts

	for (i = 0; i < m_DN.GetCount(); i++)
		{
		X509_NAME_add_entry_by_txt(COpenSSL::AsX509Name(pName), (LPSTR)m_DN[i].sField,  MBSTRING_ASC, (unsigned char *)(LPSTR)m_DN[i].sValue, -1, -1, 0);
		}
	}

void CDistinguishedName::DeleteAll (OpenSSL_X509NamePtr pName)

//	DeleteAll
//
//	Deletes all parts

	{
	int i;
	X509_NAME *pDN = COpenSSL::AsX509Name(pName);
	for (i = 0; i < X509_NAME_entry_count(pDN); i++)
		{
		X509_NAME_ENTRY *pEntry = X509_NAME_delete_entry(pDN, 0);
		if (pEntry)
			X509_NAME_ENTRY_free(pEntry);
		}
	}

CString CDistinguishedName::EncodeValue (const CString &sValue)

//	EncodeValue
//
//	Certain characters must be escaped when output a raw string.

	{
	int iExtra = 0;

	//	Optimistically assume that we don't need to encode.

	char *pPos = sValue.GetParsePointer();
	char *pPosEnd = pPos + sValue.GetLength();
	while (pPos < pPosEnd)
		{
		switch (*pPos)
			{
			case ',':
			case '+':
			case '\"':
			case '\\':
			case '<':
			case '>':
			case ';':
			case '=':
			case '/':
				//	Need a slash
				iExtra += 1;
				break;

			case '\r':
			case '\n':
				iExtra += 2;
				break;
			}

		pPos++;
		}

	if (iExtra == 0)
		return sValue;

	//	Create an encoded string

	CString sNewValue(sValue.GetLength() + iExtra);
	char *pDest = sNewValue.GetParsePointer();

	pPos = sValue.GetParsePointer();
	while (pPos < pPosEnd)
		{
		switch (*pPos)
			{
			case ',':
			case '+':
			case '\"':
			case '\\':
			case '<':
			case '>':
			case ';':
			case '=':
			case '/':
				*pDest++ = '\\';
				*pDest++ = *pPos++;
				break;

			case '\n':
				*pDest++ = '\\';
				*pDest++ = '0';
				*pDest++ = 'A';
				pPos++;
				break;

			case '\r':
				*pDest++ = '\\';
				*pDest++ = '0';
				*pDest++ = 'D';
				pPos++;
				break;

			default:
				*pDest++ = *pPos++;
				break;
			}
		}

	//	Done

	return sNewValue;
	}

CString CDistinguishedName::GetDN (void) const

//	GetDN
//
//	Writes out the name

	{
	int i;
	CStringBuffer Output;

	for (i = 0; i < m_DN.GetCount(); i++)
		{
		if (i != 0)
			Output.Write(STR_COMMA);

		Output.Write(m_DN[i].sField);
		Output.Write(STR_EQUALS);
		Output.Write(EncodeValue(m_DN[i].sValue));
		}

	CString sResult;
	sResult.TakeHandoff(Output);
	return sResult;
	}

CString CDistinguishedName::GetPart (EPart iPart) const

//	GetPart
//
//	Returns the given well-known part of the distinguished name.

	{
	int i;

	if (iPart == unknownPart)
		return NULL_STR;

	for (i = 0; i < m_DN.GetCount(); i++)
		if (m_DN[i].iPart == iPart)
			return m_DN[i].sValue;

	return NULL_STR;
	}

const CString &CDistinguishedName::GetPartName (EPart iPart)

//	GetPartName
//
//	Returns the short form of the field.

	{
	switch (iPart)
		{
		case C:
			return PART_C;

		case CN:
			return PART_CN;

		case L:
			return PART_L;

		case O:
			return PART_O;

		case OU:
			return PART_OU;

		case ST:
			return PART_ST;

		case emailAddress:
			return PART_EMAIL_ADDRESS;

		default:
			return NULL_STR;
		}
	}

bool CDistinguishedName::Parse (const CString &sName, TArray<SPart> &DN)

//	Parse
//
//	Parse a little-endian DN

	{
	enum EStates
		{
		stateStart,
		statePart,
		stateEquals,
		stateValue,
		};

	char *pPos = sName.GetParsePointer();
	char *pPosEnd = pPos + sName.GetLength();

	EStates iState = stateStart;
	char *pStart;
	CString sPart;
	CString sValue;
	while (pPos < pPosEnd)
		{
		switch (iState)
			{
			case stateStart:
				if (strIsWhitespace(pPos))
					;
				else
					{
					pStart = pPos;
					iState = statePart;
					}
				break;

			case statePart:
				if (*pPos == '=')
					{
					sPart = CString(pStart, pPos - pStart);
					iState = stateEquals;
					}
				break;

			case stateEquals:
				if (strIsWhitespace(pPos))
					;
				else
					{
					pStart = pPos;
					iState = stateValue;
					}
				break;

			case stateValue:
				if (*pPos == ',')
					{
					sValue = CString(pStart, pPos - pStart);
					AddPart(sPart, ParseValue(sValue), DN);
					iState = stateStart;
					}
				break;
			}

		pPos++;
		}

	if (iState == stateValue)
		{
		sValue = CString(pStart, pPos - pStart);
		AddPart(sPart, ParseValue(sValue), DN);
		}

	return true;
	}

CDistinguishedName::EPart CDistinguishedName::ParsePart (const CString &sPart)

//	ParsePart
//
//	Returns a known part/field

	{
	if (strEqualsNoCase(sPart, PART_C))
		return C;
	else if (strEqualsNoCase(sPart, PART_CN))
		return CN;
	else if (strEqualsNoCase(sPart, PART_L))
		return L;
	else if (strEqualsNoCase(sPart, PART_O))
		return O;
	else if (strEqualsNoCase(sPart, PART_OU))
		return OU;
	else if (strEqualsNoCase(sPart, PART_ST))
		return ST;
	else if (strEqualsNoCase(sPart, PART_EMAIL_ADDRESS))
		return emailAddress;
	else
		return unknownPart;
	}

CString CDistinguishedName::ParseValue (const CString &sValue)

//	ParseValue
//
//	Parses a value

	{
	char *pPos = sValue.GetParsePointer();
	char *pPosEnd = pPos + sValue.GetLength();
	CString sResult(sValue.GetLength());

	char *pDest = sResult.GetPointer();
	while (pPos < pPosEnd)
		{
		if (*pPos == '\\')
			{
			pPos++;
			if (pPos >= pPosEnd)
				return NULL_STR;

			if (*pPos == '0')
				{
				pPos++;
				if (pPos >= pPosEnd)
					return NULL_STR;

				if (*pPos == 'A' || *pPos == 'a')
					*pDest++ = '\n';
				else if (*pPos == 'D' || *pPos == 'd')
					*pDest++ = '\r';
				else
					return NULL_STR;
				}
			else
				*pDest++ = *pPos;
			}
		else
			*pDest++ = *pPos;

		pPos++;
		}

	sResult.SetLength((int)(pDest - sResult.GetPointer()));
	return sResult;
	}
