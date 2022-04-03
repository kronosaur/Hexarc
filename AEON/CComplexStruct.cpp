//	CComplexStruct.cpp
//
//	CComplexStruct class
//	Copyright (c) 2010 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_STRUCT,					"struct");

CComplexStruct::CComplexStruct (CDatum dSrc)

//	CComplexStruct constructor

	{
	//	Clone from another complex structure

	for (int i = 0; i < dSrc.GetCount(); i++)
		{
		CString sKey = dSrc.GetKey(i);
		if (!sKey.IsEmpty())
			SetElement(sKey, dSrc.GetElement(i));
		}
	}

CComplexStruct::CComplexStruct (const TSortMap<CString, CString> &Src)

//	CComplexStruct construtor

	{
	int i;

	for (i = 0; i < Src.GetCount(); i++)
		{
		const CString &sKey = Src.GetKey(i);
		if (!sKey.IsEmpty())
			SetElement(sKey, Src.GetValue(i));
		}
	}

CComplexStruct::CComplexStruct (const TSortMap<CString, CDatum> &Src)

//	CComplexStruct constructor

	{
	m_Map = Src;
	}

const CString &CComplexStruct::GetTypename (void) const { return TYPENAME_STRUCT; }

void CComplexStruct::AppendStruct (CDatum dDatum)

//	AppendStruct
//
//	Appends the element of the given structure

	{
	int i;

	if (dDatum.GetBasicType() == CDatum::typeStruct)
		{
		for (i = 0; i < dDatum.GetCount(); i++)
			SetElement(dDatum.GetKey(i), dDatum.GetElement(i));
		}
	}

CString CComplexStruct::AsString (void) const

//	AsString
//
//	Represent as a string

	{
	CStringBuffer Output;

	Output.Write("{", 1);

	for (int i = 0; i < m_Map.GetCount(); i++)
		{
		if (i != 0)
			Output.Write(" ", 1);

		Output.Write(m_Map.GetKey(i));
		Output.Write(":", 1);

		Output.Write(m_Map[i].AsString());
		}

	Output.Write("}", 1);

	CString sOutput;
	sOutput.TakeHandoff(Output);
	return sOutput;
	}

size_t CComplexStruct::CalcMemorySize (void) const

//	CalcMemorySize
//
//	Computes the amount of memory being used.

	{
	size_t dwSize = 0;

	for (int i = 0; i < m_Map.GetCount(); i++)
		{
		dwSize += m_Map.GetKey(i).GetLength() + sizeof(DWORD) + 1;
		dwSize += m_Map[i].CalcMemorySize();
		}

	return dwSize;
	}

bool CComplexStruct::Contains (CDatum dValue, TArray<IComplexDatum *> &retChecked) const

//	Contains
//
//	Returns TRUE if we contain dValue.

	{
	for (int i = 0; i < m_Map.GetCount(); i++)
		if (m_Map[i].Contains(dValue, retChecked))
			return true;

	return false;
	}

bool CComplexStruct::FindElement (const CString &sKey, CDatum *retpValue)

//	FindElement
//
//	Find element by key.

	{
	CDatum *pValue = m_Map.GetAt(sKey);
	if (pValue == NULL)
		return false;

	if (retpValue)
		*retpValue = *pValue;

	return true;
	}

CDatum CComplexStruct::GetElementAt (CDatum dIndex) const

//	GetElement
//
//	Returns the element.

	{
	if (dIndex.IsNil())
		return CDatum();
	else if (dIndex.IsNumberInt32())
		{
		int iIndex = dIndex;
		if (iIndex >= 0 && iIndex < m_Map.GetCount())
			return m_Map[iIndex];
		else
			return CDatum();
		}
	else
		{
		CDatum* pValue = m_Map.GetAt(dIndex.AsString());
		if (pValue)
			return *pValue;
		else
			return CDatum();
		}
	}

void CComplexStruct::OnMarked (void)

//	OnMarked
//
//	Mark

	{
	for (int i = 0; i < m_Map.GetCount(); i++)
		m_Map[i].Mark();
	}

void CComplexStruct::SetElementAt (CDatum dIndex, CDatum dDatum)

//	SetElement
//
//	Sets the element.

	{
	if (dIndex.IsNil())
		{ }
	else if (dIndex.IsNumberInt32())
		{
		int iIndex = dIndex;
		if (iIndex >= 0 && iIndex < m_Map.GetCount())
			m_Map[iIndex] = dDatum;
		}
	else
		{
		m_Map.SetAt(dIndex.AsString(), dDatum);
		}
	}
