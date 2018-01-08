//	CAEONDocument.cpp
//
//	CAEONDocument class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(KEYWORD_DEFINE,					"define")
DECLARE_CONST_STRING(KEYWORD_FUNCTION,					"function")
DECLARE_CONST_STRING(KEYWORD_INCLUDE,					"include")
DECLARE_CONST_STRING(KEYWORD_VAR,						"var")

DECLARE_CONST_STRING(TYPE_DATA,							"$data")
DECLARE_CONST_STRING(TYPE_FUNCTION,						"$function")
DECLARE_CONST_STRING(TYPE_INCLUDE,						"$include")
DECLARE_CONST_STRING(TYPE_VAR,							"$var")

DECLARE_CONST_STRING(ERR_COLON_EXPECTED,				"':' expected: %s.")
DECLARE_CONST_STRING(ERR_DUPLICATE_NAME,				"Duplicate name: %s.")
DECLARE_CONST_STRING(ERR_INCLUDE_NAME_EXPECTED,			"Invalid include reference: %s.")
DECLARE_CONST_STRING(ERR_NAME_EXPECTED,					"Name expected: %s.")
DECLARE_CONST_STRING(ERR_TYPE_EXPECTED,					"Type expected: %s.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_TOKEN,			"Unable to parse token.")

void CAEONDocument::AddEntry (const CString &sName, const CString &sType, CDatum dData)

//	AddEntry
//
//	Adds an entry

	{
	//	If no name, add an anonymous entry

	if (sName.IsEmpty())
		{
		CString sNewName;

		do
			{
			sNewName = strPattern("%s_%04x", mathRandom(0, 65535));
			}
		while (!FindEntry(sNewName));

		//	Add it

		SEntry *pEntry = m_Doc.Insert(sNewName);
		pEntry->sName = sNewName;
		pEntry->sType = sType;
		pEntry->dData = dData;
		}

	//	Otherwise add by name
	//	We assume that the name does not already exist.

	else
		{
		SEntry *pEntry = m_Doc.Insert(sName);
		pEntry->sName = sName;
		pEntry->sType = sType;
		pEntry->dData = dData;
		}

	InvalidateTypeIndex();
	}

bool CAEONDocument::FindEntry (const CString &sName, CString *retsType, CDatum *retdData)

//	FindEntry
//
//	Finds the given entry

	{
	int iIndex;
	if (!m_Doc.FindPos(sName, &iIndex))
		return false;

	if (retsType)
		*retsType = m_Doc[iIndex].sType;

	if (retdData)
		*retdData = m_Doc[iIndex].dData;

	return true;
	}

int CAEONDocument::GetTypeIndex (const CString &sType)

//	GetTypeIndex
//
//	Returns the type index for the type. This is only valid
//	until new entries are added.
//
//	If -1 is returned, the type cannot be found.

	{
	InitTypeIndex();

	int iPos;
	if (m_TypeIndex.FindPos(sType, &iPos))
		return iPos;
	else
		return -1;
	}

bool CAEONDocument::InitFromData (CDatum dData, CString *retsError)

//	InitFromData
//
//	Initializes from a datum

	{
	CBuffer Buffer((const CString &)dData);
	return InitFromStream(Buffer, retsError);
	}

bool CAEONDocument::InitFromStream (IByteStream &Stream, CString *retsError)

//	CAEONDocument
//
//	Initialize the document from a stream

	{
	CAEONScriptParser Parser(Stream);

	//	Initialize the document

	m_Doc.DeleteAll();
	InvalidateTypeIndex();

	//	Keep parsing until we reach the end of the stream

	CDatum dToken;
	CAEONScriptParser::ETokens iToken = Parser.ParseToken(&dToken);
	while (iToken != CAEONScriptParser::tkEOF)
		{
		//	Check for error

		if (iToken == CAEONScriptParser::tkError)
			{
			if (retsError)
				*retsError = ERR_UNABLE_TO_PARSE_TOKEN;
			return false;
			}

		//	After this point we expect a token

		else if (iToken != CAEONScriptParser::tkDatum)
			{
			if (retsError)
				*retsError = ERR_UNABLE_TO_PARSE_TOKEN;
			return false;
			}

		//	If this is an array then we expect a HexLisp form.

		else if (dToken.GetBasicType() == CDatum::typeArray)
			{
			//	The first element of the list is the type; the second
			//	is the name. This matches the syntax of (define {symbol} ...)

			CDatum dType = dToken.GetElement(0);
			if (dType.GetBasicType() != CDatum::typeString || !ValidateIdentifier(dType))
				{
				if (retsError)
					*retsError = strPattern(ERR_TYPE_EXPECTED, (const CString &)dType);
				return false;
				}

			CDatum dName = dToken.GetElement(1);
			if (dName.GetBasicType() != CDatum::typeString || !ValidateIdentifier(dName))
				{
				if (retsError)
					*retsError = strPattern(ERR_NAME_EXPECTED, (const CString &)dName);
				return false;
				}

			//	Make sure there is no duplicate

			if (FindEntry(dName))
				{
				if (retsError)
					*retsError = strPattern(ERR_DUPLICATE_NAME, (const CString &)dName);
				return false;
				}

			//	Add to the document

			AddEntry(dName, dType, dToken);
			}

		//	Check for reserved keywords
		//
		//	define TYPE NAME DATUM

		else if (strEquals(dToken, KEYWORD_DEFINE))
			{
			//	Get the type

			iToken = Parser.ParseToken(&dToken);
			if (iToken != CAEONScriptParser::tkDatum || dToken.GetBasicType() != CDatum::typeString || !ValidateIdentifier(dToken))
				{
				if (retsError)
					*retsError = strPattern(ERR_TYPE_EXPECTED, (const CString &)dToken);
				return false;
				}

			CString sType = dToken;

			//	Get the name

			iToken = Parser.ParseToken(&dToken);
			if (iToken != CAEONScriptParser::tkDatum || dToken.GetBasicType() != CDatum::typeString || !ValidateIdentifier(dToken))
				{
				if (retsError)
					*retsError = strPattern(ERR_NAME_EXPECTED, (const CString &)dToken);
				return false;
				}

			CString sName = dToken;

			//	Get the data

			iToken = Parser.ParseToken(&dToken);
			if (iToken != CAEONScriptParser::tkDatum)
				{
				if (retsError)
					*retsError = ERR_UNABLE_TO_PARSE_TOKEN;
				return false;
				}

			//	Make sure there is no duplicate

			if (FindEntry(sName))
				{
				if (retsError)
					*retsError = strPattern(ERR_DUPLICATE_NAME, sName);
				return false;
				}

			//	Add to the document

			AddEntry(sName, sType, dToken);
			}

		//	function NAME FUNCTION-DEFINITION

		else if (strEquals(dToken, KEYWORD_FUNCTION))
			{
			//	LATER
			}

		//	include DATUM

		else if (strEquals(dToken, KEYWORD_INCLUDE))
			{
			//	Get the include data

			iToken = Parser.ParseToken(&dToken);
			if (iToken != CAEONScriptParser::tkDatum || dToken.GetBasicType() != CDatum::typeString)
				{
				if (retsError)
					*retsError = strPattern(ERR_INCLUDE_NAME_EXPECTED, (const CString &)dToken);
				return false;
				}

			//	Add it

			AddEntry(NULL_STR, TYPE_INCLUDE, dToken);
			}

		//	var NAME = EXPRESSION

		else if (strEquals(dToken, KEYWORD_VAR))
			{
			//	LATER
			}

		//	NAME : DATUM

		else
			{
			//	Make sure the name is valid

			if (iToken != CAEONScriptParser::tkDatum || dToken.GetBasicType() != CDatum::typeString || !ValidateIdentifier(dToken))
				{
				if (retsError)
					*retsError = strPattern(ERR_NAME_EXPECTED, (const CString &)dToken);
				return false;
				}

			CString sName = dToken;
			
			//	Expect a colon

			iToken = Parser.ParseToken(&dToken);
			if (iToken != CAEONScriptParser::tkColon)
				{
				if (retsError)
					*retsError = strPattern(ERR_COLON_EXPECTED, (const CString &)dToken);
				return false;
				}

			//	Get the data

			iToken = Parser.ParseToken(&dToken);
			if (iToken != CAEONScriptParser::tkDatum)
				{
				if (retsError)
					*retsError = ERR_UNABLE_TO_PARSE_TOKEN;
				return false;
				}

			//	Make sure there is no duplicate

			if (FindEntry(sName))
				{
				if (retsError)
					*retsError = strPattern(ERR_DUPLICATE_NAME, sName);
				return false;
				}

			//	Add to the document

			AddEntry(sName, TYPE_DATA, dToken);
			}

		//	Next token

		iToken = Parser.ParseToken(&dToken);
		}

	//	Done

	return true;
	}

void CAEONDocument::InitTypeIndex (void)

//	InitTypeIndex
//
//	Initializes an index from type to entry indices

	{
	int i;

	if (m_TypeIndex.GetCount() == 0)
		{
		for (i = 0; i < m_Doc.GetCount(); i++)
			{
			//	Get a pointer to the index entry

			int iListPos;
			TArray<int> *pList;
			if (m_TypeIndex.FindPos(m_Doc[i].sType, &iListPos))
				pList = &m_TypeIndex[iListPos];
			else
				pList = m_TypeIndex.Insert(m_Doc[i].sType);

			//	Add to the list

			pList->Insert(i);
			}
		}
	}

void CAEONDocument::Mark (void)

//	Mark
//
//	Mark data in use

	{
	int i;

	for (i = 0; i < m_Doc.GetCount(); i++)
		m_Doc[i].dData.Mark();
	}

bool CAEONDocument::ValidateIdentifier (const CString &sName)

//	ValidateIdentifier
//
//	Returns TRUE if the identifier is valid

	{
	return !CAEONScriptParser::HasSpecialChars(sName);
	}
