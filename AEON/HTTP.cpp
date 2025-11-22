//	HTTP.cpp
//
//	Methods for handling HTTP messages, etc.
//	Copyright (c) 2012 GridWhale Corporation . All Rights Reserved.

#include "stdafx.h"

bool urlParseQuery (const CString &sURL, CString *retsPath, CDatum *retdQuery)

//	urlparseQuery
//
//	Parses a query of the form:
//
//	http://example.com/path?field1=value1&field2=value2
//
//	Returns TRUE on success. FALSE if there was an error.
//
//	LATER: Need to handle escaped ? and &

	{
	char *pPos = sURL.GetParsePointer();
	char *pPosEnd = pPos + sURL.GetLength();

	//	Look for the start of the query

	char *pStart = pPos;
	while (pPos < pPosEnd && *pPos != '?')
		pPos++;

	//	We have the path (everything before the query)

	if (retsPath)
		*retsPath = CString(pStart, pPos - pStart);

	//	Continue parsing the query (skipping the ?)

	if (retdQuery == NULL)
		NULL;
	else if (++pPos < pPosEnd)
		{
		//	Store the data in a structure

		CComplexStruct *pData = new CComplexStruct;

		//	Parse

		while (pPos < pPosEnd)
			{
			//	Look for the field

			pStart = pPos;
			while (pPos < pPosEnd && *pPos != '=')
				pPos++;

			CString sField(pStart, pPos - pStart);
			if (++pPos >= pPosEnd)
				break;

			//	Look for the value

			pStart = pPos;
			while (pPos < pPosEnd && *pPos != '&')
				pPos++;

			CString sValue(pStart, pPos - pStart);

			//	Convert to a datum

			CDatum dValue;
			if (!CDatum::CreateFromStringValue(urlDecode(sValue), &dValue))
				{
				delete pData;
				return false;
				}

			//	Add to structure

			pData->SetElement(urlDecode(sField), dValue);

			//	Skip separator

			if (pPos < pPosEnd && *pPos == '&')
				pPos++;
			}

		//	Done

		*retdQuery = CDatum(pData);
		}

	//	Else, no query

	else
		*retdQuery = CDatum();

	return true;
	}
