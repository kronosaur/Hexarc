//	CAttributeList.cpp
//
//	CAttributeList class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

const char SEPARATOR_CHAR =	',';

void CAttributeList::Delete (const CString &sAttrib)

//	Delete
//
//	Delete the attribute from the list

	{
	//	LATER
	ASSERT(false);
	}

bool CAttributeList::FindAttribute (const CString &sAttrib, int *retiPos) const

//	FindAttribute
//
//	Finds the attribute in the list.
//	NOTE: We assume that sAttrib is lowercase (for case-insensitivity)

	{
	//	Handle some special cases

	if (sAttrib.IsEmpty() || m_sAttributes.IsEmpty())
		return false;

	//	Get some pointers

	char *pAttribStart = sAttrib.GetParsePointer();
	char *pAttribPosEnd = pAttribStart + sAttrib.GetLength();

	//	Loop over all characters in our list until we find the first character.

	char *pPos = m_sAttributes.GetParsePointer();
	char *pPosEnd = pPos + m_sAttributes.GetLength();
	while (pPos < pPosEnd)
		{
		if (*pPos == *pAttribStart)
			{
			//	Look for the remaining characters

			char *pDest = pPos + 1;
			char *pSrc = pAttribStart + 1;
			while (pDest < pPosEnd && pSrc < pAttribPosEnd)
				{
				if (*pSrc != *pDest)
					break;

				pSrc++;
				pDest++;
				}

			//	If we reached the end of the attrib we're searching for, then
			//	see if we matched a whole attribute. If so, then we found it.

			if (pSrc == pAttribPosEnd && (pDest == pPosEnd || *pDest == SEPARATOR_CHAR))
				{
				if (retiPos)
					*retiPos = (int)(pPos - m_sAttributes.GetParsePointer());
				return true;
				}

			//	Otherwise we keep looking.
			}

		pPos++;
		}

	//	Not found

	return false;
	}

void CAttributeList::GetAll (TArray<CString> *retAttribs) const

//	GetAll
//
//	Returns all attributes

	{
	retAttribs->DeleteAll();
	if (m_sAttributes.IsEmpty())
		return;

	char *pPos = m_sAttributes.GetParsePointer();
	char *pEndPos = pPos + m_sAttributes.GetLength();
	char *pStart = pPos;

	while (pPos <= pEndPos)
		{
		if (pPos == pEndPos || *pPos == SEPARATOR_CHAR)
			{
			retAttribs->Insert(CString(pStart, pPos - pStart));
			pStart = pPos + 1;
			}

		pPos++;
		}
	}

void CAttributeList::Insert (CStringView sAttrib)

//	Insert
//
//	Insert the attribute

	{
	//	Convert to lowercase for case-insensitivity

	CString sAttribLower = strToLower(sAttrib);
	if (ValidateAttribute(sAttribLower) && !FindAttribute(sAttribLower))
		{
		if (m_sAttributes.IsEmpty())
			m_sAttributes = sAttribLower;
		else
			m_sAttributes = strPattern("%s%c%s", m_sAttributes, SEPARATOR_CHAR, sAttribLower);
		}
	}

bool CAttributeList::ValidateAttribute (const CString &sAttrib)

//	ValidateAttribute
//
//	Returns TRUE if this is a valid attribute

	{
	char *pPos = sAttrib.GetParsePointer();
	char *pEndPos = pPos + sAttrib.GetLength();

	//	Can't be empty

	if (pPos == pEndPos)
		return false;

	//	Can't have whitespace, commas, semi-colons, or control characters

	while (pPos < pEndPos)
		{
		if (*pPos == ' ' || *pPos == ',' || *pPos == ';' || strIsASCIIControl(pPos))
			return false;

		pPos++;
		}

	//	Otherwise, we're OK

	return true;
	}
