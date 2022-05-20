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
	OnCopyOnWrite();

	if (dDatum.GetBasicType() == CDatum::typeStruct)
		{
		for (int i = 0; i < dDatum.GetCount(); i++)
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

IComplexDatum *CComplexStruct::Clone (CDatum::EClone iMode) const

//	Clone
//
//	Clones a copy

	{
	switch (iMode)
		{
		case CDatum::EClone::ShallowCopy:
			return new CComplexStruct(m_Map);

		case CDatum::EClone::CopyOnWrite:
			{
			auto pClone = new CComplexStruct(m_Map);
			pClone->m_bCopyOnWrite = true;
			return pClone;
			}

		case CDatum::EClone::DeepCopy:
			{
			auto pClone = new CComplexStruct(m_Map);
			pClone->CloneContents();
			return pClone;
			}

		default:
			throw CException(errFail);
		}
	}

void CComplexStruct::CloneContents ()

//	CloneContents
//
//	Clones all content so that it is a copy.

	{
	for (int i = 0; i < m_Map.GetCount(); i++)
		m_Map[i] = m_Map[i].Clone(CDatum::EClone::DeepCopy);
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

CDatum CComplexStruct::GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const

//	GetElement
//
//	Returns the element.

	{
	int iIndex;

	if (dIndex.IsNil())
		return CDatum();
	else if (dIndex.IsNumberInt32(&iIndex))
		{
		if (iIndex >= 0 && iIndex < m_Map.GetCount())
			return m_Map[iIndex];
		else
			return CDatum();
		}
	else if (dIndex.IsContainer())
		{
		CDatum dResult(CDatum::typeStruct);

		for (int i = 0; i < dIndex.GetCount(); i++)
			{
			CDatum dEntry = dIndex.GetElement(i);
			if (dEntry.IsNil())
				{ }
			else if (dEntry.IsNumberInt32(&iIndex))
				{
				if (iIndex >= 0 && iIndex < m_Map.GetCount())
					dResult.SetElement(m_Map.GetKey(iIndex), m_Map[iIndex]);
				}
			else
				{
				CString sKey = dEntry.AsString();
				CDatum* pValue = m_Map.GetAt(sKey);
				if (pValue)
					dResult.SetElement(sKey, *pValue);
				}
			}

		return dResult;
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

void CComplexStruct::OnCopyOnWrite ()

//	OnCopyOnWrite
//
//	We're about to be modified, so see if we need to make a copy.

	{
	if (m_bCopyOnWrite)
		{
		CloneContents();
		m_bCopyOnWrite = false;
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
	OnCopyOnWrite();

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
