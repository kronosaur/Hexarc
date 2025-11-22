//	CSymbolTable.cpp
//
//	CSymbolTable class
//	Copyright (c) 2023 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DWORD CSymbolTable::Atomize (CStringSlice sSymbol)

//	Atomize
//
//	Converts the symbol to an atom.

	{
	int iPos;
	if (FindPos(sSymbol.GetPointer(), sSymbol.GetLength(), &iPos))
		return Index2Atom(m_Index[iPos]);

	int iIndex = m_Array.GetCount();
	m_Array.Insert(CString(sSymbol.GetPointer(), sSymbol.GetLength()));
	m_Index.Insert(iIndex, iPos);

	return Index2Atom(iIndex);
	}

DWORD CSymbolTable::Atomize (CStringView sSymbol)

//	Atomize
//
//	Converts the symbol to an atom.

	{
	int iPos;
	if (FindPos(sSymbol, &iPos))
		return Index2Atom(m_Index[iPos]);

	int iIndex = m_Array.GetCount();
	m_Array.Insert(sSymbol);
	m_Index.Insert(iIndex, iPos);

	return Index2Atom(iIndex);
	}

bool CSymbolTable::FindPos (LPCSTR pSymbol, int iSymbolLen, int *retiPos) const

//	FindPos
//
//	Finds the position of the symbol in the index. If found, we return TRUE and 
//	optionally the position. If not found, we return FALSE and optionally
//	return the position where we should insert the symbol.

	{
	int iCount = m_Index.GetCount();
	int iMin = 0;
	int iMax = iCount;

	while (iMin < iMax)
		{
		int iTry = iMin + (iMax - iMin)	/ 2;

		const CString& sKey = GetKey(iTry);
		int iCompare = ::KeyCompare(pSymbol, iSymbolLen, sKey.GetPointer(), sKey.GetLength());

		if (iCompare == 0)
			{
			if (retiPos)
				*retiPos = iTry;
			return true;
			}
		else if (iCompare == -1)
			{
			iMin = iTry + 1;
			}
		else
			{
			iMax = iTry;
			}
		}

	if (retiPos)
		*retiPos = iMin;

	return false;
	}

DWORD CSymbolTable::FindSymbol (const CString& sSymbol) const

//	FindSymbol
//
//	Converts the symbol to an atom, but returns 0 if the symbol is not found.

	{
	int iPos;
	if (FindPos(sSymbol, &iPos))
		return Index2Atom(m_Index[iPos]);
	else
		return 0;
	}

