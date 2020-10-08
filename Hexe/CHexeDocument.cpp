//	CHexeDocument.cpp
//
//	CHexeDocument class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_CODE,						"code")

DECLARE_CONST_STRING(KEYWORD_DEFINE,					"define")
DECLARE_CONST_STRING(KEYWORD_FUNCTION,					"function")
DECLARE_CONST_STRING(KEYWORD_INCLUDE,					"include")
DECLARE_CONST_STRING(KEYWORD_MESSAGES,					"messages")
DECLARE_CONST_STRING(KEYWORD_VAR,						"var")

DECLARE_CONST_STRING(TYPE_DATA,							"$data")
DECLARE_CONST_STRING(TYPE_HEXE_LISP,					"$hexeLisp")
DECLARE_CONST_STRING(TYPE_INCLUDE,						"$include")
DECLARE_CONST_STRING(TYPE_INCLUDE_MESSAGES,				"$includeMessages")
DECLARE_CONST_STRING(TYPE_VAR,							"$var")
DECLARE_CONST_STRING(TYPE_FUNCTION,						"function")

DECLARE_CONST_STRING(ERR_COLON_EXPECTED,				"':' expected: %s.")
DECLARE_CONST_STRING(ERR_DUPLICATE_NAME,				"Duplicate name: %s.")
DECLARE_CONST_STRING(ERR_INCLUDE_NAME_EXPECTED,			"Invalid include reference: %s.")
DECLARE_CONST_STRING(ERR_NAME_EXPECTED,					"Name expected: %s.")
DECLARE_CONST_STRING(ERR_TYPE_EXPECTED,					"Type expected: %s.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_TOKEN,			"Unable to parse token.")
DECLARE_CONST_STRING(ERR_BOM_NOT_SUPPORTED,				"UTF8 Byte Order Mark not supported.")
DECLARE_CONST_STRING(ERR_CODE_EXPECTED,					"Code expected.")
DECLARE_CONST_STRING(ERR_CRASH_PARSING,					"Crash parsing Lisp expression.")

DECLARE_CONST_STRING(STR_LAMBDA_PREFIX,					"(lambda")

CHexeDocument::~CHexeDocument (void)

//	CHexeDocument destructor

	{
	}

void CHexeDocument::AddEntry (const CString &sName, const CString &sType, CDatum dData)

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
			sNewName = strPattern("$%s_%04x", sType, mathRandom(0, 65535));
			}
		while (FindEntry(sNewName));

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

bool CHexeDocument::IsAnonymous (const CString &sName, const CString &sType)

//	IsAnonymous
//
//	Returns TRUE if this is an anonymous entry.

	{
	CString sPrefix = strPattern("$%s_", sType);
	return strStartsWith(sName, sPrefix);
	}

CDatum CHexeDocument::AsDatum (void) const

//	AsDatum
//
//	Represent document as a datum. The datum will be a structure of types-lists
//	Each type-list will be a structure of name/value pairs.
//
//	DOCUMENT
//
//	Type1 Name1 Value
//	Type1 Name2 Value
//	Type2 Name3 Value
//
//	DATUM
//
//	{
//	Type1: { Name1:Value Name2:Value }
//	Type2: { Name3:Value }
//	}

	{
	int i;

	//	Place every entry into a type list

	TSortMap<CString, CComplexStruct *> AllLists;
	for (i = 0; i < m_Doc.GetCount(); i++)
		{
		const SEntry &Entry = m_Doc[i];

		CComplexStruct *pTypeList;
		if (!AllLists.Find(Entry.sType, &pTypeList))
			{
			pTypeList = new CComplexStruct;
			AllLists.Insert(Entry.sType, pTypeList);
			}

		pTypeList->SetElement(Entry.sName, Entry.dData);
		}

	//	Now create the overall structure

	CComplexStruct *pData = new CComplexStruct;
	for (i = 0; i < AllLists.GetCount(); i++)
		pData->SetElement(AllLists.GetKey(i), CDatum(AllLists[i]));

	//	Done

	return CDatum(pData);
	}

void CHexeDocument::CreateFunctionCall (const CString &sFunction, const TArray<CDatum> &Args, CDatum *retdEntryPoint)

//	CreateFunctionCall
//
//	Creates a function call

	{
	CHexeCode::CreateFunctionCall(sFunction, Args, retdEntryPoint);
	}

bool CHexeDocument::FindEntry (const CString &sName, CString *retsType, CDatum *retdData) const

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

void CHexeDocument::GetEntryPoints (TArray<SEntryPoint> *retList) const

//	GetEntryPoints
//
//	Returns all function entry points defined in this document.

	{
	int i;

	retList->DeleteAll();

	//	Add all functions to the process global environment

	int iCodeIndex = GetTypeIndex(TYPE_FUNCTION);
	for (i = 0; i < GetTypeIndexCount(iCodeIndex); i++)
		{
		SEntryPoint *pEntry = retList->Insert();

		pEntry->sFunction = GetTypeIndexName(iCodeIndex, i);
		pEntry->dDesc = GetTypeIndexData(iCodeIndex, i);
		pEntry->dCode = pEntry->dDesc.GetElement(FIELD_CODE);
		}
	}

void CHexeDocument::GetHexeDefinitions (TArray<CDatum> *retList) const

//	GetHexeDefinitions
//
//	Returns all HexeLisp definitions.

	{
	int i;

	retList->DeleteAll();

	//	Add all definitions

	int iCodeIndex = GetTypeIndex(TYPE_HEXE_LISP);
	for (i = 0; i < GetTypeIndexCount(iCodeIndex); i++)
		retList->Insert(GetTypeIndexData(iCodeIndex, i));
	}

int CHexeDocument::GetTypeIndex (const CString &sType) const

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

bool CHexeDocument::InitFromData (CDatum dData, CHexeProcess &Process, CString *retsError)

//	InitFromData
//
//	Initializes from a datum

	{
	CBuffer Buffer((const CString &)dData);
	return InitFromStream(Buffer, Process, retsError);
	}

bool CHexeDocument::InitFromStream (IByteStream &Stream, CHexeProcess &Process, CString *retsError)

//	CHexeDocument
//
//	Initialize the document from a stream

	{
	CCharStream CharStream;
	CharStream.Init(Stream);
	CString sError;

	//	Initialize the document

	m_Doc.DeleteAll();
	InvalidateTypeIndex();

	//	Check for UTF8 BOM (which we do not support)

	char chChar = CharStream.GetChar();
	if ((BYTE)chChar == 0xef)
		{
		chChar = CharStream.ReadChar();
		if ((BYTE)chChar == 0xbb)
			{
			chChar = CharStream.ReadChar();
			if ((BYTE)chChar == 0xbf)
				{
				*retsError = ERR_BOM_NOT_SUPPORTED;
				return false;
				}
			}
		}

	//	Keep parsing until we reach the end of the stream

	while (chChar != '\0')
		{
		//	Skip comments and whitespace

		if (strIsWhitespace(&chChar) || chChar == '/' || chChar == ';')
			{
			if (!ParseComments(&CharStream, retsError))
				return false;
			}

		//	If we have an open paren then we need to parse a HexeLisp expression

		else if (chChar == '(')
			{
			CString sName;
			CString sType;
			CDatum dDatum;

			//	This compiles the term into an expression (generally a define)
			//	that is ready for evaluation. Note that there is no security
			//	context associated with this expression: when it is evaluated,
			//	the result will inherit the security context of process in which
			//	it was evaluated.

			if (!ParseHexeLispDef(&CharStream, &sName, &sType, &dDatum, retsError))
				return false;

			AddEntry(sName, sType, dDatum);
			}

		//	Otherwise, parse an AEON identifier

		else
			{
			CString sName;
			CString sType;
			CDatum dDatum;

			if (!ParseAEONDef(&CharStream, Process, &sName, &sType, &dDatum, &sError))
				{
				if (retsError)
					*retsError = strPattern("ERROR(%d): %s", CharStream.GetLine(), sError);
				return false;
				}

			AddEntry(sName, sType, dDatum);
			}

		chChar = CharStream.GetChar();
		}

	//	Done

	return true;
	}

void CHexeDocument::InitTypeIndex (void) const

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

bool CHexeDocument::IsValidIdentifier (const CString &sIdentifier)

//	IsValidIdentifier
//
//	Returns TRUE if sIdentifier is a valid Hexe identifier

	{
	char *pPos = sIdentifier.GetParsePointer();
	char *pPosEnd = pPos + sIdentifier.GetLength();

	while (pPos < pPosEnd)
		{
		switch (*pPos)
			{
			case ' ':
			case '(':
			case ')':
			case '{':
			case '}':
			case '[':
			case ']':
			case '\'':
			case '\"':
			case '`':
			case ',':
			case ';':
			case ':':
				return false;

			default:
				if (strIsWhitespace(pPos) || strIsASCIIControl(pPos))
					return false;
				break;
			}

		pPos++;
		}

	return true;
	}

void CHexeDocument::Mark (void)

//	Mark
//
//	Mark data in use

	{
	int i;

	for (i = 0; i < m_Doc.GetCount(); i++)
		m_Doc[i].dData.Mark();
	}

void CHexeDocument::Merge (CHexeDocument *pDoc)

//	Merge
//
//	Merges the definition in the given document into ours.

	{
	int i;

	for (i = 0; i < pDoc->m_Doc.GetCount(); i++)
		{
		const SEntry &Entry = pDoc->m_Doc[i];

		if (IsAnonymous(Entry.sName, Entry.sType))
			AddEntry(NULL_STR, Entry.sType, Entry.dData);
		else
			AddEntry(Entry.sName, Entry.sType, Entry.dData);
		}
	}

bool CHexeDocument::ParseAEONDef (CCharStream *pStream, CHexeProcess &Process, CString *retsName, CString *retsType, CDatum *retdDatum, CString *retsError)

//	ParseAEONDef
//
//	Parses an AEON definition of the form:
//
//	[define] abc xyz datum
//		defines a typed piece of static data, which can be used
//		as a read-only variable. abc is the type. xyz is the name.
//
//	[define] function xyz [datum] (lambda (...) ...)
//		defines a function with options attributes.
//
//	include "filename"
//		includes file module "filename".

	{
	CLambdaParseExtension LambdaExt(Process);
	CAEONScriptParser Parser(pStream, &LambdaExt);

	CDatum dToken;
	CAEONScriptParser::ETokens iToken = Parser.ParseToken(&dToken);
	CString sTokenLC = strToLower(dToken);

	//	Check for error

	if (iToken == CAEONScriptParser::tkError)
		{
		*retsError = ERR_UNABLE_TO_PARSE_TOKEN;
		return false;
		}

	//	After this point we expect a token

	else if (iToken != CAEONScriptParser::tkDatum)
		{
		*retsError = ERR_UNABLE_TO_PARSE_TOKEN;
		return false;
		}

	//	Check for reserved keywords
	//
	//	define TYPE NAME DATUM

	else if (strEquals(sTokenLC, KEYWORD_DEFINE))
		{
		//	Get the type

		iToken = Parser.ParseToken(&dToken);
		if (iToken != CAEONScriptParser::tkDatum || dToken.GetBasicType() != CDatum::typeString || !IsValidIdentifier(dToken))
			{
			*retsError = strPattern(ERR_TYPE_EXPECTED, (const CString &)dToken);
			return false;
			}

		*retsType = dToken;

		//	Parse the rest

		return ParseDefine(Parser, Process, *retsType, retsName, retdDatum, retsError);
		}

	//	include DATUM

	else if (strEquals(sTokenLC, KEYWORD_INCLUDE))
		{
		//	Get the include data

		iToken = Parser.ParseToken(&dToken);
		if (iToken != CAEONScriptParser::tkDatum || dToken.GetBasicType() != CDatum::typeString)
			{
			*retsError = strPattern(ERR_INCLUDE_NAME_EXPECTED, (const CString &)dToken);
			return false;
			}

		//	If we're including messages, get the name separately

		CString sName;
		CString sType;
		if (strEquals(dToken, KEYWORD_MESSAGES))
			{
			iToken = Parser.ParseToken(&dToken);
			if (iToken != CAEONScriptParser::tkDatum || dToken.GetBasicType() != CDatum::typeString)
				{
				*retsError = strPattern(ERR_INCLUDE_NAME_EXPECTED, (const CString &)dToken);
				return false;
				}

			sName = dToken;
			sType = TYPE_INCLUDE_MESSAGES;
			}
		else
			{
			sName = dToken;
			sType = TYPE_INCLUDE;
			}

		//	Done

		*retsName = sName;
		*retsType = sType;
		*retdDatum = dToken;

		return true;
		}

	//	var NAME = EXPRESSION

	else if (strEquals(dToken, KEYWORD_VAR))
		{
		//	LATER
		}

	//	TYPE NAME DATUM

	else
		{
		//	Get the type

		if (dToken.GetBasicType() != CDatum::typeString || !IsValidIdentifier(dToken))
			{
			*retsError = strPattern(ERR_TYPE_EXPECTED, (const CString &)dToken);
			return false;
			}

		*retsType = dToken;

		//	Parse the rest

		return ParseDefine(Parser, Process, *retsType, retsName, retdDatum, retsError);
		}

	//	Done

	return false;
	}

bool CHexeDocument::ParseComments (CCharStream *pStream, CString *retsError)

//	ParseComments
//
//	Parses comments and whitespace:
//
//	// to end of line
//	; to end of line
//	/* ... */

	{
	char chChar = pStream->GetChar();

	//	Skip comments and whitespace

	while (strIsWhitespace(&chChar) || chChar == '/' || chChar == ';')
		{
		if (chChar == '/')
			{
			chChar = pStream->ReadChar();
			if (chChar == '*')
				{
				chChar = pStream->ReadChar();
				while (true)
					{
					if (chChar == '*')
						{
						chChar = pStream->ReadChar();
						if (chChar == '/')
							{
							chChar = pStream->ReadChar();
							break;
							}
						else if (chChar != '*')
							chChar = pStream->ReadChar();
						}
					else
						chChar = pStream->ReadChar();
					}
				}
			else if (chChar == '/')
				{
				pStream->ParseToEndOfLine();
				chChar = pStream->GetChar();
				}
			else
				{
				*retsError = ERR_UNABLE_TO_PARSE_TOKEN;
				return false;
				}
			}
		else if (chChar == ';')
			{
			pStream->ParseToEndOfLine();
			chChar = pStream->GetChar();
			}
		else
			chChar = pStream->ReadChar();
		}

	return true;
	}

bool CHexeDocument::ParseData (IByteStream &Stream, CDatum *retdData)

//	ParseData
//
//	Parses a single datum. This is equivalent to CDatum::Deserialize except that
//	we also parse lambda expressions into proper code blocks.

	{
	CHexeProcess Process;
	CLambdaParseExtension LambdaExt(Process);

	CCharStream CharStream;
	CharStream.Init(Stream);
	CAEONScriptParser Parser(&CharStream, &LambdaExt);

	CDatum dToken;
	CAEONScriptParser::ETokens iToken = Parser.ParseToken(&dToken);
	if (iToken != CAEONScriptParser::tkDatum)
		return false;

	//	Done

	*retdData = dToken;
	return true;
	}

bool CHexeDocument::ParseDefine (CAEONScriptParser &Parser, CHexeProcess &Process, const CString &sType, CString *retsName, CDatum *retdDatum, CString *retsError)

//	ParseDefine
//
//	Parse a definition.

	{
	//	Get the name

	CDatum dToken;
	CAEONScriptParser::ETokens iToken = Parser.ParseToken(&dToken);
	if (iToken != CAEONScriptParser::tkDatum || dToken.GetBasicType() != CDatum::typeString || !IsValidIdentifier(dToken))
		{
		*retsError = strPattern(ERR_NAME_EXPECTED, (const CString &)dToken);
		return false;
		}

	*retsName = dToken;

	//	Get the data

	iToken = Parser.ParseToken(&dToken);
	if (iToken != CAEONScriptParser::tkDatum)
		{
		if (iToken == CAEONScriptParser::tkError)
			*retsError = dToken.AsString();
		else
			*retsError = ERR_UNABLE_TO_PARSE_TOKEN;
		return false;
		}

	//	If this is a function then we do some special handling

	if (strEquals(sType, KEYWORD_FUNCTION))
		{
		//	If the data is a structure, then we continue parsing to find the
		//	actual function.

		if (dToken.GetBasicType() == CDatum::typeStruct)
			{
			CDatum dCode;
			iToken = Parser.ParseToken(&dCode);
			if (iToken != CAEONScriptParser::tkDatum || CHexeFunction::Upconvert(dCode) == NULL)
				{
				if (dCode.IsError())
					*retsError = dCode;
				else
					*retsError = ERR_CODE_EXPECTED;
				return false;
				}

			//	Add the code to the data structure

			dToken.SetElement(FIELD_CODE, dCode);
			}

		//	If this is a function, then we need to create a data structure
		//	to hold it.

		else if (CHexeFunction::Upconvert(dToken))
			{
			CComplexStruct *pStruct = new CComplexStruct;
			pStruct->SetElement(FIELD_CODE, dToken);

			dToken = CDatum(pStruct);
			}

		//	Otherwise this is an error

		else
			{
			*retsError = ERR_CODE_EXPECTED;
			return false;
			}
		}

	//	Make sure there is no duplicate

	if (FindEntry(*retsName))
		{
		*retsError = strPattern(ERR_DUPLICATE_NAME, *retsName);
		return false;
		}

	//	Done

	*retdDatum = dToken;
	return true;
	}

bool CHexeDocument::ParseLispExpression (const CString &sExpression, CDatum *retdExpression, CString *retsError)

//	ParseLispExpression
//
//	Parses a single HexeLisp expression and returns a code block.
//
//	NOTE: The resulting code block does not have any security context applied.
//	The caller must evaluate the code in a process (which already has a
//	security context).

	{
	try
		{
		CStringBuffer Stream(sExpression);
		CCharStream CharStream;
		CharStream.Init(Stream);

		return ParseHexeLispDef(&CharStream, NULL, NULL, retdExpression, retsError);
		}
	catch (...)
		{
		if (retsError) *retsError = ERR_CRASH_PARSING;
		return false;
		}
	}

bool CHexeDocument::ParseHexeLispDef (CCharStream *pStream, CString *retsName, CString *retsType, CDatum *retdDatum, CString *retsError)

//	ParseHexeLisp
//
//	Parses a HexeLisp expression into a code block. Note that the resulting 
//	datum is NOT a function--it is a code snippet that defines something. It 
//	must be executed to get a function (or other value).

	{
	CHexeCodeIntermediate CodeBlocks;
	int iBlock;

	CLispCompiler Compiler;
	if (!Compiler.CompileTerm(pStream, CodeBlocks, &iBlock, retsError))
		return false;

	//	Create a function call entry point

	if (retsName)
		*retsName = NULL_STR;

	if (retsType)
		*retsType = TYPE_HEXE_LISP;

	CHexeCode::Create(CodeBlocks, iBlock, retdDatum);

	//	Done

	return true;
	}

