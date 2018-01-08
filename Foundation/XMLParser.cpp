//	XMLParser.cpp
//
//	XML functions and classes
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

enum TokenTypes
	{
	tkEOF,						//	end of file
	tkPIOpen,					//	<?
	tkPIClose,					//	?>
	tkTagOpen,					//	<
	tkTagClose,					//	>
	tkEndTagOpen,				//	</
	tkSimpleTagClose,			//	/>
	tkEquals,					//	=
	tkQuote,					//	"
	tkSingleQuote,				//	'
	tkText,						//	plain text
	tkDeclOpen,					//	<!
	tkBracketOpen,				//	[
	tkBracketClose,				//	]
	tkError,					//	error
	};

enum StateTypes
	{
	StartState,
	StartDeclState,
	OpenTagState,
	ContentState,
	SlashState,
	QuestionState,
	IdentifierState,
	TextState,
	AttributeTextState,
	CommentState,
	CDATAState,
	EntityState,
	AttributeState,
	ParseEntityState,
	EntityDeclarationState,
	EntityDeclarationFindValueState,
	EntityDeclarationValueState,
	EntityDeclarationEndState,
	};

struct ParserCtx
	{
	public:
		ParserCtx (IMemoryBlock &Stream, IXMLParserController *pController);
		ParserCtx (ParserCtx *pParentCtx, const CString &sString);

		void DefineEntity (const CString &sName, const CString &sValue);
		CString LookupEntity (const CString &sName, bool *retbFound = NULL);

	public:
		IXMLParserController *m_pController;
		ParserCtx *m_pParentCtx;

		char *pPos;
		char *pEndPos;

		TSortMap<CString, CString> EntityTable;

		CXMLElement *pElement;

		TokenTypes iToken;
		CString sToken;
		int iLine;

		bool m_bParseRootElement;
		bool m_bParseRootTag;
		CString m_sRootTag;

		TokenTypes iAttribQuote;

		CString sError;
	};

DECLARE_CONST_STRING(STR_10,						"1.0")
DECLARE_CONST_STRING(STR_DOCTYPE,					"DOCTYPE")
DECLARE_CONST_STRING(STR_XML,						"xml")

DECLARE_CONST_STRING(ENTITY_AMP,					"amp")
DECLARE_CONST_STRING(ENTITY_AMP_SUB,				"&")
DECLARE_CONST_STRING(ENTITY_APOS,					"apos")
DECLARE_CONST_STRING(ENTITY_APOS_SUB,				"'")
DECLARE_CONST_STRING(ENTITY_GT,						"gt")
DECLARE_CONST_STRING(ENTITY_GT_SUB,					">")
DECLARE_CONST_STRING(ENTITY_LT,						"lt")
DECLARE_CONST_STRING(ENTITY_LT_SUB,					"<")
DECLARE_CONST_STRING(ENTITY_QUOT,					"quot")
DECLARE_CONST_STRING(ENTITY_QUOT_SUB,				"\"")

DECLARE_CONST_STRING(FIELD_ENCODING,				"encoding")
DECLARE_CONST_STRING(FIELD_VERSION,					"version")

DECLARE_CONST_STRING(ERR_EQUAL_EXPECTED,			"= expected.")
DECLARE_CONST_STRING(ERR_XML_PROLOGUE_EXPECTED,		"<?XML prologue expected.")
DECLARE_CONST_STRING(ERR_ATTRIB_EXPECTED,			"Attribute expected.")
DECLARE_CONST_STRING(ERR_ATTRIB_NEEDS_QUOTES,		"Attribute value must be quoted.")
DECLARE_CONST_STRING(ERR_UNMATCHED_CLOSE_TAG,		"Close tag does not match open.")
DECLARE_CONST_STRING(ERR_CLOSE_TAG_EXPECTED,		"Close tag expected.")
DECLARE_CONST_STRING(ERR_CONTENT_EXPECTED,			"Content expected.")
DECLARE_CONST_STRING(ERR_DOCTYPE_BRACKET_EXPECTED,	"DOCTYPE: ] expected.")
DECLARE_CONST_STRING(ERR_DOCTYPE_EXPECTED,			"DOCTYPE declaration expected.")
DECLARE_CONST_STRING(ERR_DOCTYPE_GREATER_EXPECTED,	"DOCTYPE: > expected.")
DECLARE_CONST_STRING(ERR_DOCTYPE_ID_EXPECTED,		"DOCTYPE: external ID expected.")
DECLARE_CONST_STRING(ERR_DOCTYPE_NAME_EXPECTED,		"DOCTYPE: name expected.")
DECLARE_CONST_STRING(ERR_DOCTYPE_QUOTE_EXPECTED,	"DOCTYPE: \" expected.")
DECLARE_CONST_STRING(ERR_ELEMENT_TAG_EXPECTED,		"Element tag expected.")
DECLARE_CONST_STRING(ERR_ENCODING_EXPECTED,			"Encoding attribute expected.")
DECLARE_CONST_STRING(ERR_INVALID_ENCODING_ATTRIB,	"Invalid encoding attribute.")
DECLARE_CONST_STRING(ERR_INVALID_PROLOG_ATTRIB,		"Invalid prologue attribute.")
DECLARE_CONST_STRING(ERR_INVALID_XML_PROLOG,		"Invalid XML prologue.")
DECLARE_CONST_STRING(ERR_INVALID_VERSION_ATTRIB,	"Invalid version attribute.")
DECLARE_CONST_STRING(ERR_MISMATCHED_ATTRIB_QUOTE,	"Mismatched attribute quote.")
DECLARE_CONST_STRING(ERR_VERSION_EXPECTED,			"Version attribute expected.")
DECLARE_CONST_STRING(ERR_VERSION_10_EXPECTED,		"Version 1.0 attribute expected.")

ParserCtx::ParserCtx (IMemoryBlock &Stream, IXMLParserController *pController) : 
		m_pController(pController)
	{
	pPos = Stream.GetPointer();
	pEndPos = pPos + Stream.GetLength();
	pElement = NULL;
	iToken = tkEOF;
	iLine = 1;
	m_bParseRootElement = false;
	m_bParseRootTag = false;

	m_pParentCtx = NULL;
	}

ParserCtx::ParserCtx (ParserCtx *pParentCtx, const CString &sString) : 
		m_pController(pParentCtx->m_pController)
	{
	pPos = sString.GetPointer();
	pEndPos = pPos + sString.GetLength();
	pElement = NULL;
	iToken = tkEOF;
	iLine = 1;
	m_bParseRootElement = false;
	m_bParseRootTag = false;

	m_pParentCtx = pParentCtx;
	}

void ParserCtx::DefineEntity (const CString &sName, const CString &sValue)
	{
	EntityTable.Insert(sName, sValue);
	}

CString ParserCtx::LookupEntity (const CString &sName, bool *retbFound)
	{
	CString *pValue = EntityTable.GetAt(sName);
	if (pValue == NULL)
		{
		if (m_pParentCtx)
			return m_pParentCtx->LookupEntity(sName, retbFound);
		else if (m_pController)
			return m_pController->ResolveExternalEntity(sName, retbFound);
		else
			{
			if (retbFound) *retbFound = false;
			return sName;
			}
		}

	if (retbFound) *retbFound = true;
	return *pValue;
	}

//	Forwards

bool ParseElement (ParserCtx *pCtx, CXMLElement **retpElement);
bool ParsePrologue (ParserCtx *pCtx);
TokenTypes ParseToken (ParserCtx *pCtx, StateTypes iInitialState = StartState);
CString ResolveEntity (ParserCtx *pCtx, const CString &sName, bool *retbFound);

bool CXMLElement::ParseXML (IMemoryBlock &Stream, 
							   CXMLElement **retpElement, 
							   CString *retsError,
							   CExternalEntityTable *retEntityTable)
	{
	return ParseXML(Stream, NULL, retpElement, retsError, retEntityTable);
	}

bool CXMLElement::ParseXML (IMemoryBlock &Stream, 
							   IXMLParserController *pController,
							   CXMLElement **retpElement, 
							   CString *retsError,
							   CExternalEntityTable *retEntityTable)

//	ParseXML
//
//	Parses the block and returns an XML element

	{
	//	Initialize context

	ParserCtx Ctx(Stream, pController);

	//	Parse the prologue

	if (!ParsePrologue(&Ctx))
		{
		*retsError = strPattern("Line(%d): %s", Ctx.iLine, Ctx.sError);
		return false;
		}

	//	Next token must be an element open tag

	if (Ctx.iToken != tkTagOpen)
		{
		*retsError = strPattern("Line(%d): root element expected.", Ctx.iLine);
		return false;
		}

	//	Parse the root element

	if (!ParseElement(&Ctx, retpElement))
		{
		*retsError = strPattern("Line(%d): %s", Ctx.iLine, Ctx.sError);
		return false;
		}

	//	Done

	if (retEntityTable)
		retEntityTable->AddTable(Ctx.EntityTable);

	return true;
	}

bool ParseDTD (ParserCtx *pCtx)

//	ParseDTD
//
//	Parse DTD declarations

	{
	ParseToken(pCtx, StartDeclState);
	while (pCtx->iToken != tkBracketClose
			&& pCtx->iToken != tkEOF)
		{
		ParseToken(pCtx, StartDeclState);
		}

	return true;
	}

bool ParseElement (ParserCtx *pCtx, CXMLElement **retpElement)

//	ParseElement
//
//	Parses an element and returns it. We assume that we've already
//	parsed an open tag

	{
	CXMLElement *pElement;

	ASSERT(pCtx->iToken == tkTagOpen);

	//	Parse the tag name

	if (ParseToken(pCtx) != tkText)
		{
		pCtx->sError = ERR_ELEMENT_TAG_EXPECTED;
		return false;
		}

	//	Create a new element with the tag

	pElement = new CXMLElement(pCtx->sToken, pCtx->pElement);
	if (pElement == NULL)
		throw CException(errOutOfMemory);

	//	Keep parsing until the tag is done

	ParseToken(pCtx);
	while (pCtx->iToken != tkTagClose && pCtx->iToken != tkSimpleTagClose)
		{
		//	If we've got an identifier then this must be an attribute

		if (pCtx->iToken == tkText)
			{
			CString sAttribute = pCtx->sToken;
			CString sValue;

			//	Expect an equals sign

			if (ParseToken(pCtx) != tkEquals)
				{
				pCtx->sError = ERR_EQUAL_EXPECTED;
				delete pElement;
				return false;
				}

			//	Expect a quote

			ParseToken(pCtx);
			if (pCtx->iToken != tkQuote && pCtx->iToken != tkSingleQuote)
				{
				pCtx->sError = ERR_ATTRIB_NEEDS_QUOTES;
				delete pElement;
				return false;
				}

			//	Remember what kind of qoute we used so that we can match it
			//	(and so we ignore the other kind inside it).

			pCtx->iAttribQuote = pCtx->iToken;

			//	Expect the value 

			ParseToken(pCtx, AttributeState);
			if (pCtx->iToken == tkText)
				{
				sValue = pCtx->sToken;
				ParseToken(pCtx);
				}
			else
				sValue = NULL_STR;

			//	Now expect an end-quote

			if (pCtx->iToken != pCtx->iAttribQuote)
				{
				if (pCtx->iToken != tkError || pCtx->sError.IsEmpty())
					pCtx->sError = ERR_MISMATCHED_ATTRIB_QUOTE;
				delete pElement;
				return false;
				}

			//	Add the attribute to the element

			pElement->AddAttribute(sAttribute, sValue);

			//	Parse the next token

			ParseToken(pCtx);
			}

		//	Otherwise this is an error

		else
			{
			if (pCtx->iToken != tkError || pCtx->sError.IsEmpty())
				pCtx->sError = ERR_ATTRIB_EXPECTED;
			delete pElement;
			return false;
			}
		}

	//	Give our controller a chance to deal with an element
	//	(We use this in Transcendence to parse the <Library> element, which
	//	contains external entities).
	//
	//	NOTE: We only worry about top-level elements (i.e., elements immediately
	//	under the root).

	if (pCtx->m_pController && pCtx->pElement && pCtx->pElement->GetParentElement() == NULL)
		{
		if (!pCtx->m_pController->OnOpenTag(pElement, &pCtx->sError))
			{
			delete pElement;
			return false;
			}
		}

	//	If we don't have an empty element then keep parsing until
	//	we find a close tag

	if (!pCtx->m_bParseRootElement && pCtx->iToken == tkTagClose)
		{
		CXMLElement *pParentElement;

		//	We are recursing

		pParentElement = pCtx->pElement;
		pCtx->pElement = pElement;

		//	Parse until we've got the begin close tag

		while (ParseToken(pCtx, ContentState) != tkEndTagOpen)
			{
			//	If this is text then append it as content

			if (pCtx->iToken == tkText)
				pElement->AppendContent(pCtx->sToken);

			//	Otherwise, append an element

			else if (pCtx->iToken == tkTagOpen)
				{
				CXMLElement *pSubElement;

				if (!ParseElement(pCtx, &pSubElement))
					{
					pCtx->pElement = pParentElement;
					delete pElement;
					return false;
					}

				pElement->AppendSubElement(pSubElement);
				}

			//	Otherwise we're in trouble

			else
				{
				pCtx->pElement = pParentElement;
				if (pCtx->iToken != tkError || pCtx->sError.IsEmpty())
					pCtx->sError = ERR_CONTENT_EXPECTED;
				delete pElement;
				return false;
				}
			}

		//	Done

		pCtx->pElement = pParentElement;

		//	The element tag should match ours

		if (ParseToken(pCtx) != tkText
				|| strEqualsNoCase(pCtx->sToken, pElement->GetTag()))
			{
			pCtx->sError = ERR_UNMATCHED_CLOSE_TAG;
			delete pElement;
			return false;
			}

		//	Parse the end tag

		if (ParseToken(pCtx) != tkTagClose)
			{
			pCtx->sError = ERR_CLOSE_TAG_EXPECTED;
			delete pElement;
			return false;
			}
		}

	//	Done

	*retpElement = pElement;

	return true;
	}

bool ParsePrologue (ParserCtx *pCtx)

//	ParsePrologue
//
//	Parses <?XML prologue

	{
	//	We don't allow any whitespace at the beginning

	if (*pCtx->pPos != '<')
		{
		pCtx->sError = ERR_XML_PROLOGUE_EXPECTED;
		return false;
		}

	//	Expect open processor instruction. If we don't find it,
	//	then we assume that there is no prologue and proceed
	//	to parse the root element.

	if (ParseToken(pCtx) != tkPIOpen)
		return true;

	//	Expect XML tag

	if (ParseToken(pCtx) != tkText || !strEquals(strToLower(pCtx->sToken), STR_XML))
		{
		pCtx->sError = ERR_XML_PROLOGUE_EXPECTED;
		return false;
		}

	//	Parse contents

	while (ParseToken(pCtx) == tkText)
		{
		CString sTokenLC = strToLower(pCtx->sToken);

		if (strEquals(sTokenLC, FIELD_VERSION))
			{
			if (ParseToken(pCtx) != tkEquals)
				{
				pCtx->sError = ERR_VERSION_EXPECTED;
				return false;
				}

			if (ParseToken(pCtx) != tkQuote)
				{
				pCtx->sError = ERR_VERSION_EXPECTED;
				return false;
				}

			if (ParseToken(pCtx) != tkText || !strEquals(pCtx->sToken, STR_10))
				{
				pCtx->sError = ERR_VERSION_10_EXPECTED;
				return false;
				}

			if (ParseToken(pCtx) != tkQuote)
				{
				pCtx->sError = ERR_INVALID_VERSION_ATTRIB;
				return false;
				}
			}
		else if (strEquals(sTokenLC, FIELD_ENCODING))
			{
			if (ParseToken(pCtx) != tkEquals)
				{
				pCtx->sError = ERR_ENCODING_EXPECTED;
				return false;
				}

			if (ParseToken(pCtx) != tkQuote)
				{
				pCtx->sError = ERR_ENCODING_EXPECTED;
				return false;
				}

			if (ParseToken(pCtx) != tkText)
				{
				pCtx->sError = ERR_INVALID_ENCODING_ATTRIB;
				return false;
				}

			if (ParseToken(pCtx) != tkQuote)
				{
				pCtx->sError = ERR_INVALID_ENCODING_ATTRIB;
				return false;
				}
			}
		else
			{
			//	Assume it is an unknown attribute

			if (ParseToken(pCtx) != tkEquals)
				{
				pCtx->sError = ERR_INVALID_PROLOG_ATTRIB;
				return false;
				}

			if (ParseToken(pCtx) != tkQuote)
				{
				pCtx->sError = ERR_INVALID_PROLOG_ATTRIB;
				return false;
				}

			if (ParseToken(pCtx) != tkText)
				{
				pCtx->sError = ERR_INVALID_PROLOG_ATTRIB;
				return false;
				}

			if (ParseToken(pCtx) != tkQuote)
				{
				pCtx->sError = ERR_INVALID_PROLOG_ATTRIB;
				return false;
				}
			}

#ifdef LATER
	//	Handle EncodingDecl
	//	Handle RMDecl
#endif
		}

	//	Expect close

	if (pCtx->iToken != tkPIClose)
		{
		pCtx->sError = ERR_INVALID_XML_PROLOG;
		return false;
		}

	//	See if we've got a DOCTYPE declaration

	if (ParseToken(pCtx) == tkDeclOpen)
		{
		if (ParseToken(pCtx) != tkText
				|| !strEqualsNoCase(pCtx->sToken, STR_DOCTYPE))
			{
			pCtx->sError = ERR_DOCTYPE_EXPECTED;
			return false;
			}

		//	Get the name

		if (ParseToken(pCtx) != tkText)
			{
			pCtx->sError = ERR_DOCTYPE_NAME_EXPECTED;
			return false;
			}

		//	If we're just looking for the root tag, then we can quit after this

		if (pCtx->m_bParseRootTag)
			{
			pCtx->m_sRootTag = pCtx->sToken;
			return true;
			}

		//	External ID?

		if (ParseToken(pCtx, StartDeclState) == tkText)
			{
			//	Either SYSTEM or PUBLIC

			//	Expect a quote

			if (ParseToken(pCtx) != tkQuote)
				{
				pCtx->sError = ERR_DOCTYPE_ID_EXPECTED;
				return false;
				}

			pCtx->iAttribQuote = tkQuote;

			//	Get the path or URL

			if (ParseToken(pCtx, AttributeState) != tkText)
				{
				pCtx->sError = ERR_DOCTYPE_ID_EXPECTED;
				return false;
				}

			CString sDTDPath = pCtx->sToken;

			//	End quote

			if (ParseToken(pCtx) != pCtx->iAttribQuote)
				{
				pCtx->sError = ERR_DOCTYPE_QUOTE_EXPECTED;
				return false;
				}

			ParseToken(pCtx);
			}

		//	Internal DTD?

		if (pCtx->iToken == tkBracketOpen)
			{
			if (!ParseDTD(pCtx))
				return false;

			if (pCtx->iToken != tkBracketClose)
				{
				pCtx->sError = ERR_DOCTYPE_BRACKET_EXPECTED;
				return false;
				}

			ParseToken(pCtx);
			}

		//	Close

		if (pCtx->iToken != tkTagClose)
			{
			pCtx->sError = ERR_DOCTYPE_GREATER_EXPECTED;
			return false;
			}

		ParseToken(pCtx);
		}

	//	Consume tokens until we get a tag open

	while (pCtx->iToken == tkText)
		ParseToken(pCtx);

	return true;
	}

TokenTypes ParseToken (ParserCtx *pCtx, StateTypes iInitialState)

//	ParseToken
//
//	Parses the next token and updatex pCtx->iToken and pCtx->sToken.
//	If bContent is TRUE, then we treat whitespace as text.

	{
	bool bDone = false;
	bool bNoEOF = false;
	StateTypes iState;
	char *pStartRun;
	CString sName;

	//	If we're parsing an entity then the rules change slightly

	if (iInitialState == ParseEntityState)
		{
		iInitialState = TextState;
		pStartRun = pCtx->pPos;
		pCtx->sToken = CString("");
		bNoEOF = true;
		}

	//	If we're parsing content then start in the content state

	iState = iInitialState;

	//	Keep parsing until we're done

	StateTypes iSavedState = StartState;

	while (pCtx->pPos < pCtx->pEndPos && !bDone)
		{
		char chChar = *pCtx->pPos;

		switch (iState)
			{
			case StartState:
				{
				switch (chChar)
					{
					//	Swallow whitespace
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						break;

					case '<':
						iState = OpenTagState;
						break;

					case '>':
						pCtx->iToken = tkTagClose;
						bDone = true;
						break;

					case '/':
						iState = SlashState;
						break;

					case '?':
						iState = QuestionState;
						break;

					case '=':
						pCtx->iToken = tkEquals;
						bDone = true;
						break;

					case '"':
						pCtx->iToken = tkQuote;
						bDone = true;
						break;

					case '\'':
						pCtx->iToken = tkSingleQuote;
						bDone = true;
						break;

					default:
						iState = IdentifierState;
						pStartRun = pCtx->pPos;
						break;
					}

				break;
				}

			case StartDeclState:
				{
				switch (chChar)
					{
					//	Swallow whitespace
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						break;

					case '<':
						iState = OpenTagState;
						break;

					case '>':
						pCtx->iToken = tkTagClose;
						bDone = true;
						break;

					case '/':
						iState = SlashState;
						break;

					case '?':
						iState = QuestionState;
						break;

					case '=':
						pCtx->iToken = tkEquals;
						bDone = true;
						break;

					case '"':
						pCtx->iToken = tkQuote;
						bDone = true;
						break;

					case '[':
						pCtx->iToken = tkBracketOpen;
						bDone = true;
						break;

					case ']':
						pCtx->iToken = tkBracketClose;
						bDone = true;
						break;

					default:
						iState = IdentifierState;
						pStartRun = pCtx->pPos;
						break;
					}

				break;
				}

			case ContentState:
				{
				switch (chChar)
					{
					case '<':
						iState = OpenTagState;
						break;

					case '>':
						pCtx->iToken = tkTagClose;
						bDone = true;
						break;

					case '&':
						iState = EntityState;
						iSavedState = TextState;
						pStartRun = pCtx->pPos + 1;
						pCtx->sToken = NULL_STR;
						break;

					default:
						iState = TextState;
						pStartRun = pCtx->pPos;
						pCtx->sToken = NULL_STR;
						break;
					}

				break;
				}

			case AttributeState:
				{
				switch (chChar)
					{
					case '&':
						iState = EntityState;
						iSavedState = AttributeTextState;
						pStartRun = pCtx->pPos + 1;
						pCtx->sToken = NULL_STR;
						break;

					default:
						if ((chChar == '"' && pCtx->iAttribQuote == tkQuote)
								|| (chChar == '\'' && pCtx->iAttribQuote == tkSingleQuote))
							{
							pCtx->iToken = pCtx->iAttribQuote;
							bDone = true;
							break;
							}
						else
							{
							iState = AttributeTextState;
							pStartRun = pCtx->pPos;
							pCtx->sToken = NULL_STR;
							break;
							}
					}

				break;
				}

			case OpenTagState:
				{
				switch (chChar)
					{
					case '?':
						pCtx->iToken = tkPIOpen;
						bDone = true;
						break;

					case '/':
						pCtx->iToken = tkEndTagOpen;
						bDone = true;
						break;

					case '!':
						{
						//	Is this a comment?

						if ((pCtx->pPos + 2 < pCtx->pEndPos)
								&& pCtx->pPos[1] == '-' 
								&& pCtx->pPos[2] == '-')
							{
							iState = CommentState;
							pCtx->pPos += 2;
							}

						//	Is this CDATA?

						else if ((pCtx->pPos + 7 < pCtx->pEndPos)
								&& pCtx->pPos[1] == '['
								&& pCtx->pPos[2] == 'C'
								&& pCtx->pPos[3] == 'D'
								&& pCtx->pPos[4] == 'A'
								&& pCtx->pPos[5] == 'T'
								&& pCtx->pPos[6] == 'A'
								&& pCtx->pPos[7] == '[')
							{
							iState = CDATAState;
							pCtx->sToken = NULL_STR;
							pCtx->pPos += 7;
							pStartRun = pCtx->pPos + 1;
							}

						//	Is this and entity declaration?

						else if ((pCtx->pPos + 7 < pCtx->pEndPos)
								&& pCtx->pPos[1] == 'E'
								&& pCtx->pPos[2] == 'N'
								&& pCtx->pPos[3] == 'T'
								&& pCtx->pPos[4] == 'I'
								&& pCtx->pPos[5] == 'T'
								&& pCtx->pPos[6] == 'Y')
							{
							pCtx->pPos += 7;

							//	Skip any whitespace

							while (*pCtx->pPos == ' '
									|| *pCtx->pPos == '\t'
									|| *pCtx->pPos == '\r'
									|| *pCtx->pPos == '\n')
								pCtx->pPos++;

							iState = EntityDeclarationState;
							pStartRun = pCtx->pPos;
							}

						//	Else it is a declaration of some sort

						else
							{
							pCtx->iToken = tkDeclOpen;
							bDone = true;
							}

						break;
						}

					default:
						pCtx->iToken = tkTagOpen;
						pCtx->pPos--;
						bDone = true;
						break;
					}

				break;
				}

			case EntityDeclarationState:
				{
				switch (chChar)
					{
					//	Whitespace means the end of the name
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						sName = CString(pStartRun, pCtx->pPos - pStartRun);
						iState = EntityDeclarationFindValueState;
						break;

					case '\"':
					case '>':
					case '<':
						pCtx->iToken = tkError;
						bDone = true;
						break;
					}
				break;
				}

			case EntityDeclarationFindValueState:
				{
				switch (chChar)
					{
					case '\"':
						pStartRun = pCtx->pPos + 1;
						iState = EntityDeclarationValueState;
						break;

					case '>':
					case '<':
						pCtx->iToken = tkError;
						bDone = true;
						break;
					}
				break;
				}

			case EntityDeclarationValueState:
				{
				switch (chChar)
					{
					case '\"':
						{
						CString sValue = CString(pStartRun, pCtx->pPos - pStartRun);
						pCtx->DefineEntity(sName, sValue);
						iState = EntityDeclarationEndState;
						break;
						}

					case '>':
					case '<':
						pCtx->iToken = tkError;
						bDone = true;
						break;
					}
				break;
				}

			case EntityDeclarationEndState:
				{
				if (chChar == '>')
					iState = StartDeclState;
				break;
				}

			case SlashState:
				{
				if (chChar == '>')
					pCtx->iToken = tkSimpleTagClose;
				else
					pCtx->iToken = tkError;

				bDone = true;
				break;
				}

			case QuestionState:
				{
				if (chChar == '>')
					pCtx->iToken = tkPIClose;
				else
					pCtx->iToken = tkError;

				bDone = true;
				break;
				}

			case IdentifierState:
				{
				switch (chChar)
					{
					case ' ':
					case '\t':
					case '\n':
					case '\r':
					case '=':
					case '>':
					case '?':
					case '/':
					case '"':
					case '<':
						pCtx->iToken = tkText;
						pCtx->sToken = CString(pStartRun, pCtx->pPos - pStartRun);
						pCtx->pPos--;
						bDone = true;
						break;
					}

				break;
				}

			case TextState:
				{
				switch (chChar)
					{
					case '<':
					case '>':
						pCtx->iToken = tkText;
						pCtx->sToken += CString(pStartRun, pCtx->pPos - pStartRun);
						pCtx->pPos--;
						bDone = true;
						break;

					//	Handle embeded entities

					case '&':
						pCtx->sToken += CString(pStartRun, pCtx->pPos - pStartRun);
						pStartRun = pCtx->pPos + 1;
						iSavedState = TextState;
						iState = EntityState;
						break;
					}

				break;
				}

			case AttributeTextState:
				{
				switch (chChar)
					{
					//	Handle embeded entities

					case '&':
						pCtx->sToken += CString(pStartRun, pCtx->pPos - pStartRun);
						pStartRun = pCtx->pPos + 1;
						iSavedState = AttributeTextState;
						iState = EntityState;
						break;

					default:
						{
						if ((chChar == '"' && pCtx->iAttribQuote == tkQuote)
								|| (chChar == '\'' && pCtx->iAttribQuote == tkSingleQuote))
							{
							pCtx->iToken = tkText;
							pCtx->sToken += CString(pStartRun, pCtx->pPos - pStartRun);
							pCtx->pPos--;
							bDone = true;
							break;
							}
						}
					}

				break;
				}

			case EntityState:
				{
				switch (chChar)
					{
					case ';':
						{
						CString sEntity(pStartRun, pCtx->pPos - pStartRun);

						bool bFound;
						pCtx->sToken += ResolveEntity(pCtx, sEntity, &bFound);
						if (!bFound)
							{
							pCtx->iToken = tkError;
							pCtx->sError = strPattern("Invalid entity: %s.", sEntity);
							bDone = true;
							break;
							}

						pStartRun = pCtx->pPos + 1;
						ASSERT(iSavedState != StartState);
						iState = iSavedState;
						break;
						}

					case ' ':
					case '>':
					case '<':
					case '\"':
					case '\'':
					case '\\':
					case '&':
						{
						CString sEntity(pStartRun, (pCtx->pPos + 1) - pStartRun);

						pCtx->iToken = tkError;
						pCtx->sError = strPattern("Illegal character in entity: '%s' (or missing semi-colon).", sEntity);
						bDone = true;
						break;
						}
					}

				break;
				}

			case CommentState:
				{
				if (chChar == '-' 
						&& (pCtx->pPos + 2 < pCtx->pEndPos)
						&& pCtx->pPos[1] == '-'
						&& pCtx->pPos[2] == '>')
					{
					if (iInitialState == StartDeclState)
						iState = iInitialState;
					else
						iState = ContentState;
					pCtx->pPos += 2;
					}

				break;
				}

			case CDATAState:
				{
				if (chChar == ']'
						&& (pCtx->pPos + 2 < pCtx->pEndPos)
						&& pCtx->pPos[1] == ']'
						&& pCtx->pPos[2] == '>')
					{
					pCtx->iToken = tkText;
					pCtx->sToken += CString(pStartRun, pCtx->pPos - pStartRun);
					pCtx->pPos += 2;
					bDone = true;
					}

				break;
				}

			default:
				ASSERT(FALSE);
			}

		//	Count lines

		if (chChar == '\n')
			pCtx->iLine++;

		//	Next character

		pCtx->pPos++;
		}

	//	If we're not done, then we hit the end of the file

	if (!bDone)
		{
		if (bNoEOF)
			{
			pCtx->iToken = tkText;
			pCtx->sToken += CString(pStartRun, pCtx->pPos - pStartRun);
			}
		else
			pCtx->iToken = tkEOF;
		}

	return pCtx->iToken;
	}

CString ResolveEntity (ParserCtx *pCtx, const CString &sName, bool *retbFound)

//	ResolveEntity
//
//	Resolves the entity from the parser table

	{
	*retbFound = true;
	CString sResult;

	//	Check to see if the name is one of the standard entities

	CString sNameLC = strToLower(sName);
	if (strEquals(sNameLC, ENTITY_AMP))
		return ENTITY_AMP_SUB;
	else if (strEquals(sNameLC, ENTITY_LT))
		return ENTITY_LT_SUB;
	else if (strEquals(sNameLC, ENTITY_GT))
		return ENTITY_GT_SUB;
	else if (strEquals(sNameLC, ENTITY_QUOT))
		return ENTITY_QUOT_SUB;
	else if (strEquals(sNameLC, ENTITY_APOS))
		return ENTITY_APOS_SUB;

	//	If the entity is a hex number, then this is a character

	char *pPos = sName.GetParsePointer();
	if (*pPos == '#')
		{
		pPos++;
		if (*pPos == 'x' || *pPos == 'X')
			{
			*pPos++;
			char chChar = (char)strParseIntOfBase(pPos, 16, 0x20, NULL, NULL);
			return CString(&chChar, 1);
			}
		else
			{
			char chChar = (char)strParseInt(pPos, 0x20);
			return CString(&chChar, 1);
			}
		}

	//	Otherwise, it is a general attribute

	bool bFound;
	CString sValue = pCtx->LookupEntity(sName, &bFound);
	if (bFound)
		{
		//	Parse the value to resolve embedded entities

		ParserCtx SubCtx(pCtx, sValue);

		ParseToken(&SubCtx, ParseEntityState);
		if (SubCtx.iToken == tkText)
			sResult = SubCtx.sToken;
		else
			{
			bFound = false;
			sResult = sName;
			}
		}

	if (retbFound)
		*retbFound = bFound;

	return sResult;
	}

bool CXMLElement::ParseEntityTable (IMemoryBlock &Stream, CExternalEntityTable *retEntityTable, CString *retsError)

//	ParseEntityTable
//
//	This function parses only the entity table

	{
	//	Initialize context

	ParserCtx Ctx(Stream, NULL);

	//	Parse the prologue

	if (!ParsePrologue(&Ctx))
		{
		*retsError = strPattern("Line(%d): %s", Ctx.iLine, Ctx.sError);
		return false;
		}

	//	Done

	if (retEntityTable)
		retEntityTable->AddTable(Ctx.EntityTable);

	return true;
	}

bool CXMLElement::ParseRootElement (IMemoryBlock &Stream, CXMLElement **retpRoot, CExternalEntityTable *retEntityTable, CString *retsError)

//	ParseRootElement
//
//	Parses the entity definitions and the root element (but not the contents
//	of the root element).

	{
	//	Initialize context

	ParserCtx Ctx(Stream, NULL);

	//	Parse the prologue

	if (!ParsePrologue(&Ctx))
		{
		*retsError = strPattern("Line(%d): %s", Ctx.iLine, Ctx.sError);
		return false;
		}

	//	Next token must be an element open tag

	if (Ctx.iToken != tkTagOpen)
		{
		*retsError = strPattern("Line(%d): Root element expected.", Ctx.iLine);
		return false;
		}

	//	Parse the root element

	Ctx.m_bParseRootElement = true;
	if (!ParseElement(&Ctx, retpRoot))
		{
		*retsError = strPattern("Line(%d): %s", Ctx.iLine, Ctx.sError);
		return false;
		}

	//	Done

	if (retEntityTable)
		retEntityTable->AddTable(Ctx.EntityTable);

	return true;
	}

bool CXMLElement::ParseRootTag (IMemoryBlock &Stream, CString *retsTag)

//	ParseRootTag
//
//	This function parses only enough to determine the root tag, either by reading
//	as far as the first open tag or by getting the DOCTYPE.
//
//	This function is a hack to allow Transcendence to read the root tag for an
//	extension without loading the whole file.

	{
	//	Initialize context

	ParserCtx Ctx(Stream, NULL);
	Ctx.m_bParseRootTag = true;

	//	Parse the prologue

	if (!ParsePrologue(&Ctx))
		return false;

	if (!Ctx.m_sRootTag.IsEmpty())
		{
		*retsTag = Ctx.m_sRootTag;
		return true;
		}

	//	Next token must be an element open tag

	if (Ctx.iToken != tkTagOpen)
		return false;

	//	Parse the root element name

	if (ParseToken(&Ctx) != tkText)
		return false;

	*retsTag = Ctx.sToken;

	//	Done

	return true;
	}
