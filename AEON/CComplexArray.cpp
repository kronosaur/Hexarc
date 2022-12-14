//	CComplexArray.cpp
//
//	CComplexArray class
//	Copyright (c) 2010 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_ARRAY,					"array");

TDatumPropertyHandler<CComplexArray> CComplexArray::m_Properties = {
	{
		"datatype",
		"Returns the type of the array.",
		[](const CComplexArray &Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"elementtype",
		"Returns the element type of the array.",
		[](const CComplexArray &Obj, const CString &sProperty)
			{
			return CAEONTypeSystem::GetCoreType(IDatatype::ANY);
			},
		NULL,
		},
	{
		"keys",
		"Returns an array of valid indices.",
		[](const CComplexArray &Obj, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			dResult.GrowToFit(Obj.GetCount());
			for (int i = 0; i < Obj.GetCount(); i++)
				dResult.Append(i);

			return dResult;
			},
		NULL,
		},
	{
		"length",
		"Returns the number of element in the array.",
		[](const CComplexArray &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	};

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

IComplexDatum *CComplexArray::Clone (CDatum::EClone iMode) const

//	Clone
//
//	Clones a copy

	{
	switch (iMode)
		{
		case CDatum::EClone::ShallowCopy:
			return new CComplexArray(m_Array);

		case CDatum::EClone::CopyOnWrite:
			{
			auto pClone = new CComplexArray(m_Array);
			pClone->m_bCopyOnWrite = true;
			return pClone;
			}

		case CDatum::EClone::DeepCopy:
			{
			auto pClone = new CComplexArray(m_Array);
			pClone->CloneContents();
			return pClone;
			}

		default:
			throw CException(errFail);
		}
	}

void CComplexArray::CloneContents ()

//	CloneContents
//
//	Clones all content so that it is a copy.

	{
	for (int i = 0; i < m_Array.GetCount(); i++)
		m_Array[i] = m_Array[i].Clone(CDatum::EClone::DeepCopy);
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

void CComplexArray::DeleteElement (int iIndex)

//	DeleteElement
//
//	Deletes the given element.

	{
	if (iIndex < 0 || iIndex >= GetCount())
		return;

	m_Array.Delete(iIndex);
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

CDatum CComplexArray::GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const

//	GetElementAt
//
//	Returns the element at the index.
	
	{
	int iIndex = dIndex.AsArrayIndex();
	if (iIndex >= 0 && iIndex < m_Array.GetCount())
		return m_Array[iIndex];
	else if (dIndex.IsContainer())
		{
		CDatum dResult(CDatum::typeArray);
		dResult.GrowToFit(dIndex.GetCount());

		for (int i = 0; i < dIndex.GetCount(); i++)
			{
			int iIndex = dIndex.GetElement(i).AsArrayIndex();
			if (iIndex >= 0 && iIndex < m_Array.GetCount())
				dResult.Append(m_Array[iIndex]);
			else
				dResult.Append(CDatum());
			}

		return dResult;
		}
	else
		return CDatum();
	}

CDatum CComplexArray::MathAbs () const

//	MathAbs
//
//	Returns a new array of absolute values.

	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		dResult.Append(m_Array[i].MathAbs());
		}
	return dResult;
	}

CDatum CComplexArray::MathMax () const

//	MathMax
//
//	Compute the maximum value.

	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	CNumberValue Result(m_Array[0].MathMax());
	for (int i = 1; i < m_Array.GetCount(); i++)
		{
		Result.Max(m_Array[i].MathMax());
		}

	if (Result.IsValidNumber())
		return Result.GetDatum();
	else
		return CDatum::CreateNaN();
	}

CDatum CComplexArray::MathMin () const

//	MathMin
//
//	Compute the minimum value.

	{
	if (m_Array.GetCount() == 0)
		return CDatum();

	CNumberValue Result(m_Array[0].MathMin());
	for (int i = 1; i < m_Array.GetCount(); i++)
		{
		Result.Min(m_Array[i].MathMin());
		}

	if (Result.IsValidNumber())
		return Result.GetDatum();
	else
		return CDatum::CreateNaN();
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

void CComplexArray::OnCopyOnWrite ()

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
	OnCopyOnWrite();

	int iIndex = dIndex.AsArrayIndex();
	if (iIndex >= 0 && iIndex < m_Array.GetCount())
		m_Array[iIndex] = dDatum;
	}
