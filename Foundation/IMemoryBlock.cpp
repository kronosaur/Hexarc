//	IMemoryBlock.cpp
//
//	IMemoryBlock class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

bool IMemoryBlock::ReadCSVRow (DWORD_PTR &iCurPos, TArray<CString> &Row)

//	ReadCSVRow
//
//	Reads a CSV row. Returns FALSE if we've reached the end of file.

	{
	char *pPos = GetPointer() + iCurPos;
	char *pPosEnd = GetPointer() + GetLength();
	if (pPos >= pPosEnd)
		return false;

	Row.DeleteAll();

	char *pRowStart = pPos;

	//	Skip any CRLF from the previous row

	while (pPos < pPosEnd && (*pPos == '\r' || *pPos == '\n'))
		pPos++;

	if (pPos == pPosEnd)
		return false;

	//	Keep looping until we hit the end of the row

	while (pPos < pPosEnd && *pPos != '\r' && *pPos != '\n')
		{
		//	Parse a field

		char *pStart = pPos;
		while (pPos < pPosEnd && *pPos != ',' && *pPos != '\r' && *pPos != '\n')
			pPos++;

		Row.Insert(CString(pStart, (int)(pPos - pStart)));

		//	Skip the comma.

		if (pPos < pPosEnd && *pPos == ',')
			pPos++;
		}

	//	Consume ending CRLF

	while (pPos < pPosEnd && (*pPos == '\r' || *pPos == '\n'))
		pPos++;

	//	Update the current position

	iCurPos += (DWORD_PTR)(pPos - pRowStart);

	//	Done

	return true;
	}
