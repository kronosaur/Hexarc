//	CComplexArray.cpp
//
//	CComplexArray class
//	Copyright (c) 2010 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_ARRAY,					"array");

const CString &CComplexArray::GetTypename (void) const { return TYPENAME_ARRAY; }

CComplexArray::CComplexArray (CDatum dSrc)

//	ComplexArray constructor

	{
	int i;

	if (dSrc.GetBasicType() == CDatum::typeStruct)
		{
		InsertEmpty(1);
		SetElement(0, dSrc);
		}
	else
		{
		int iCount = dSrc.GetCount();

		//	Clone from another complex array

		if (iCount > 0)
			{
			InsertEmpty(iCount);

			for (i = 0; i < iCount; i++)
				SetElement(i, dSrc.GetElement(i));
			}
		}
	}

CComplexArray::CComplexArray (const TArray<CString> &Src)

//	CComplexArray constructor

	{
	int i;
	int iCount = Src.GetCount();

	if (iCount > 0)
		{
		InsertEmpty(iCount);

		for (i = 0; i < iCount; i++)
			SetElement(i, Src[i]);
		}
	}

CComplexArray::CComplexArray (const TArray<CDatum> &Src)

//	CComplexArray constructor

	{
	int i;
	int iCount = Src.GetCount();

	if (iCount > 0)
		{
		InsertEmpty(iCount);

		for (i = 0; i < iCount; i++)
			SetElement(i, Src[i]);
		}
	}

CString CComplexArray::AsString (void) const

//	AsString
//
//	Represent as a string

	{
	CStringBuffer Output;

	Output.Write("(", 1);

	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		if (i != 0)
			Output.Write(" ", 1);

		CString sResult = m_Array[i].AsString();
		Output.Write(sResult);
		}

	Output.Write(")", 1);

	CString sOutput;
	sOutput.TakeHandoff(Output);
	return sOutput;
	}

size_t CComplexArray::CalcMemorySize (void) const

//	CalcMemorySize
//
//	Computes the amount of memory being used.

	{
	size_t dwSize = 0;

	for (int i = 0; i < m_Array.GetCount(); i++)
		dwSize += m_Array[i].CalcMemorySize();

	return dwSize;
	}

bool CComplexArray::Contains (CDatum dValue, TArray<IComplexDatum *> &retChecked) const

//	Contains
//
//	Returns TRUE if we contain dValue.

	{
	for (int i = 0; i < m_Array.GetCount(); i++)
		if (m_Array[i].Contains(dValue, retChecked))
			return true;

	return false;
	}

bool CComplexArray::FindElement (CDatum dValue, int *retiIndex) const

//	FindElement
//
//	Finds the given element

	{
	int i;

	for (i = 0; i < m_Array.GetCount(); i++)
		if (dValue.IsEqual(m_Array[i]))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}

	return false;
	}

CDatum CComplexArray::GetElementAt (CDatum dIndex) const

//	GetElementAt
//
//	Returns the element at the index.
	
	{
	int iIndex = dIndex.AsArrayIndex();
	if (iIndex >= 0 && iIndex < m_Array.GetCount())
		return m_Array[iIndex];
	else
		return CDatum();
	}

size_t CComplexArray::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns an approximation of serialization size.

	{
	int i;
	size_t TotalSize = 2 + m_Array.GetCount();

	for (i = 0; i < m_Array.GetCount(); i++)
		TotalSize += m_Array[i].CalcSerializeSize(iFormat);

	return TotalSize;
	}

void CComplexArray::OnMarked (void)

//	OnMarked
//
//	Mark any elements that we own

	{
	for (int i = 0; i < m_Array.GetCount(); i++)
		m_Array[i].Mark();
	}

void CComplexArray::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serialize to the given format.

	{
	int i;

	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			Stream.Write("(", 1);

			for (i = 0; i < m_Array.GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(" ", 1);

				m_Array[i].Serialize(iFormat, Stream);
				}

			Stream.Write(")", 1);
			break;
			}

		case CDatum::EFormat::JSON:
			{
			Stream.Write("[", 1);

			for (int i = 0; i < m_Array.GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(", ", 2);

				m_Array[i].Serialize(iFormat, Stream);
				}

			Stream.Write("]", 1);
			break;
			}

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

void CComplexArray::SetElementAt (CDatum dIndex, CDatum dDatum)

//	SetElementAt
//
//	Sets element at the index.

	{
	int iIndex = dIndex.AsArrayIndex();
	if (iIndex >= 0 && iIndex < m_Array.GetCount())
		m_Array[iIndex] = dDatum;
	}

//	CComplexDateTime -----------------------------------------------------------

CString CComplexDateTime::AsString (void) const

//	AsString
//
//	NOTE: We rely on the fact that the returned string is sortable (i.e.,
//	comparable to other strings).

	{
	if (m_DateTime.HasDate() && m_DateTime.HasTime())
		{
		if (m_DateTime.Millisecond() == 0)
			return strPattern("%04d-%02d-%02dT%02d:%02d:%02d",
					m_DateTime.Year(),
					m_DateTime.Month(),
					m_DateTime.Day(),
					m_DateTime.Hour(),
					m_DateTime.Minute(),
					m_DateTime.Second());
		else
			return strPattern("%04d-%02d-%02dT%02d:%02d:%02d.%03d",
					m_DateTime.Year(),
					m_DateTime.Month(),
					m_DateTime.Day(),
					m_DateTime.Hour(),
					m_DateTime.Minute(),
					m_DateTime.Second(),
					m_DateTime.Millisecond());
		}
	else if (m_DateTime.HasDate())
		{
		return strPattern("%04d-%02d-%02d",
				m_DateTime.Year(),
				m_DateTime.Month(),
				m_DateTime.Day());
		}
	else if (m_DateTime.HasTime())
		{
		if (m_DateTime.Millisecond() == 0)
			return strPattern("%02d:%02d:%02d",
					m_DateTime.Hour(),
					m_DateTime.Minute(),
					m_DateTime.Second());
		else
			return strPattern("%02d:%02d:%02d.%03d",
					m_DateTime.Hour(),
					m_DateTime.Minute(),
					m_DateTime.Second(),
					m_DateTime.Millisecond());
		}
	else
		return NULL_STR;
	}

