//	CAEONLines.cpp
//
//	CAEONLines class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_TEXT_LINES,				"textLines");

TDatumPropertyHandler<CAEONLines> CAEONLines::m_Properties = {
	{
		"length",
		"Returns the number of lines.",
		[](const CAEONLines &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	};

CAEONLines::CAEONLines (CDatum dValue)

//	CAEONLines constructor

	{
	if (!dValue.IsNil())
		Append(dValue);
	}

void CAEONLines::Append (CDatum dDatum)

//	Append
//
//	Append to the datum.

	{
	//	LATER: Handle some special types like another lines type or a vector
	//	of strings.

	if (dDatum.IsNil())
		m_Lines.Insert(NULL_STR);

	else if (dDatum.IsArray())
		{
		TArray<CString> Lines;
		for (int i = 0; i < dDatum.GetCount(); i++)
			{
			CDatum dElement = dDatum.GetElement(i);

			if (dElement.GetBasicType() == CDatum::typeString)
				{
				const CString& sValue = dElement;
				Lines.Insert(SplitBuffer(CBuffer(sValue)));
				}
			else
				{
				CString sValue = dElement.AsString();
				Lines.Insert(SplitBuffer(CBuffer(sValue)));
				}
			}

		Insert(std::move(Lines));
		}
	else if (dDatum.GetBasicType() == CDatum::typeString)
		{
		const CString& sValue = dDatum;
		Insert(SplitBuffer(CBuffer(sValue)));
		}
	else
		{
		CString sValue = dDatum.AsString();
		Insert(SplitBuffer(CBuffer(sValue)));
		}

	OnModify();
	}

void CAEONLines::ApplyDiff (const CAEONTextLinesDiff& Diff)

//	ApplyDiff
//
//	Applies difference.

	{
#ifdef DEBUG_DIFF
	printf("Cur lines: %d\n", m_Lines.GetCount());
#endif

	int iCurLine = 0;
	for (int i = 0; i < Diff.GetCount(); i++)
		{
		auto& Op = Diff.GetOp(i);

		switch (Op.iType)
			{
			case CAEONTextLinesDiff::EType::Delete:
				{
				int iCount = Op.dParam;
#ifdef DEBUG_DIFF
				printf("Delete %d at %d.\n", iCount, iCurLine);
#endif
				m_Lines.Delete(iCurLine, iCount);
				break;
				}

			case CAEONTextLinesDiff::EType::Insert:
				{
#ifdef DEBUG_DIFF
				printf("Insert at %d\n", iCurLine);
#endif
				m_Lines.Insert(Op.dParam, iCurLine);
				iCurLine++;
				break;
				}

			case CAEONTextLinesDiff::EType::Same:
				iCurLine += (int)Op.dParam;
				break;

			default:
				throw CException(errFail);
			}
		}

	OnModify();
	}

CString CAEONLines::AsString (void) const

//	AsString
//
//	Represent as a string.

	{
	CStringBuffer Buffer;

	for (int i = 0; i < m_Lines.GetCount(); i++)
		{
		if (i != 0)
			Buffer.Write("\n", 1);

		Buffer.Write(m_Lines[i]);
		}

	return CDatum(std::move(Buffer));
	}

size_t CAEONLines::CalcMemorySize (void) const

//	CalcMemorySize
//
//	Compute the amount of total memory used by this datum.

	{
	size_t iTotal = 0;
	for (int i = 0; i < m_Lines.GetCount(); i++)
		iTotal += m_Lines[i].GetLength() + sizeof(DWORD) + 1;

	return iTotal;
	}

IComplexDatum *CAEONLines::Clone (CDatum::EClone iMode) const

//	Clone
//
//	Create a clone.

	{
	switch (iMode)
		{
		case CDatum::EClone::ShallowCopy:
		case CDatum::EClone::CopyOnWrite:
		case CDatum::EClone::DeepCopy:
			return new CAEONLines(*this);

		default:
			throw CException(errFail);
		}
	}

void CAEONLines::DeleteElement (int iIndex)

//	DeleteElement
//
//	Delete the nth line.

	{
	if (iIndex >= 0 && iIndex < m_Lines.GetCount())
		{
		m_Lines.Delete(iIndex);
		OnModify();
		}
	}

CDatum CAEONLines::GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const

//	GetElementAt
//
//	Returns an element.

	{
	int iIndex;

	if (dIndex.IsNil())
		return CDatum();
	else if (dIndex.IsNumberInt32(&iIndex))
		return GetElement(iIndex);
	else
		return CDatum();
	}

const CString &CAEONLines::GetTypename (void) const	{ return TYPENAME_TEXT_LINES; }

void CAEONLines::GrowToFit (int iCount)

//	GrowToFit
//
//	Allocates space for the given number of additional lines.

	{
	m_Lines.GrowToFit(iCount);
	}

void CAEONLines::Insert (TArray<CString>&& Lines, int iIndex)

//	Insert
//
//	Insert lines.

	{
	int iStart = m_Lines.InsertEmpty(Lines.GetCount(), iIndex);

	for (int i = 0; i < Lines.GetCount(); i++)
		{
		m_Lines[iStart + i] = std::move(Lines[i]);
		}
	}

size_t CAEONLines::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	CalcSerializeSizeAEONScript
//
//	Computes the amount of space required to serialize.

	{
	return CalcMemorySize();
	}

bool CAEONLines::OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Deserializes

	{
	int iCount;
	Stream.Read(&iCount, sizeof(iCount));
	Stream.Read(&m_Seq, sizeof(m_Seq));

	m_Lines.DeleteAll();
	m_Lines.InsertEmpty(iCount);
	for (int i = 0; i < iCount; i++)
		m_Lines[i] = CString::Deserialize(Stream);

	return true;
	}

void CAEONLines::OnModify ()

//	OnModify
//
//	Data structure has been modified.

	{
	//	Increment sequence so we know it got modified.

	m_Seq++;
	}

void CAEONLines::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serializes.

	{
	int iCount = m_Lines.GetCount();
	Stream.Write(&iCount, sizeof(iCount));
	Stream.Write(&m_Seq, sizeof(m_Seq));

	for (int i = 0; i < m_Lines.GetCount(); i++)
		m_Lines[i].Serialize(Stream);
	}

void CAEONLines::SetElement (int iIndex, CDatum dDatum)

//	SetElement
//
//	Sets the given line.

	{
	if (iIndex < 0 || iIndex >= m_Lines.GetCount())
		return;

	//	Parse the value into 0 or more lines.

	TArray<CString> Lines;
	if (dDatum.IsArray())
		{
		for (int i = 0; i < dDatum.GetCount(); i++)
			{
			CDatum dElement = dDatum.GetElement(i);

			if (dElement.GetBasicType() == CDatum::typeString)
				{
				const CString& sValue = dElement;
				Lines.Insert(SplitBuffer(CBuffer(sValue)));
				}
			else
				{
				CString sValue = dElement.AsString();
				Lines.Insert(SplitBuffer(CBuffer(sValue)));
				}
			}

		}
	else if (dDatum.GetBasicType() == CDatum::typeString)
		{
		const CString& sValue = dDatum;
		Lines = SplitBuffer(CBuffer(sValue));
		}
	else
		{
		CString sValue = dDatum.AsString();
		Lines = SplitBuffer(CBuffer(sValue));
		}

	//	We only support 0 or 1 lines.

	if (Lines.GetCount() == 0)
		m_Lines[iIndex] = NULL_STR;
	else
		m_Lines[iIndex] = Lines[0];

	OnModify();
	}

void CAEONLines::SetElementAt (CDatum dIndex, CDatum dDatum)

//	SetElementAt
//
//	Sets the element.

	{
	int iIndex;

	if (dIndex.IsNil())
		{ }
	else if (dIndex.IsNumberInt32(&iIndex))
		SetElement(iIndex, dDatum);
	else
		{ }
	}

TArray<CString> CAEONLines::SplitBuffer (const IMemoryBlock& Buffer)

//	SplitBuffer
//
//	Takes a buffer of text and splits it into zero or more lines.

	{
	TArray<CString> Result;
	CString *pDest = NULL;

	//	Keep looping until we've added all lines to the result.

	const char* pPos = Buffer.GetPointer();
	const char* pEnd = pPos + Buffer.GetLength();
	while (true)
		{
		//	Increment pPos until we hit the end of the line or until we hit an
		//	invalid character (which we need to fix).

		const char* pLineStart = pPos;
		while (pPos < pEnd
				&& (*pPos == '\t' || (BYTE)*pPos >= (BYTE)' '))
			pPos++;

		//	Add to the current line.

		if (pPos > pLineStart)
			{
			if (!pDest)
				pDest = Result.Insert();

			(*pDest) += CBuffer(pLineStart, pPos - pLineStart, false);
			}

		//	If we're done, then done.

		if (pPos == pEnd)
			break;

		//	If this is a line end, then go to the next line.

		else if (*pPos == '\n' || *pPos == '\r')
			{
			if (!pDest)
				Result.Insert();
			else
				pDest = NULL;

			char chEnd = *pPos++;
			if (chEnd == '\n' && *pPos == '\r')
				pPos++;
			else if (chEnd == '\r' && *pPos == '\n')
				pPos++;
			}

		//	Otherwise, this is an invalid character.

		else
			{
			if (!pDest)
				pDest = Result.Insert();

			(*pDest) += CBuffer("?", 1, false);
			pPos++;
			}
		}

	return Result;
	}
