//	CAEONRange.cpp
//
//	CAEONRange classes
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_END,							"end");
DECLARE_CONST_STRING(FIELD_START,						"start");
DECLARE_CONST_STRING(FIELD_STEP,						"step");

DECLARE_CONST_STRING(TYPENAME_RANGE,					"range");

TDatumPropertyHandler<CAEONRange> CAEONRange::m_Properties = {
	{
		"datatype",
		"%",
		"Returns the type of the object.",
		[](const CAEONRange &Obj, const CString &sProperty)
			{
			return CAEONTypeSystem::GetCoreType(IDatatype::RANGE);
			},
		NULL,
		},
	{
		"end",
		"I",
		"The last value in the range.",
		[](const CAEONRange &Obj, const CString &sProperty)
			{
			return (Obj.IsNil() ? CDatum() : CDatum(Obj.m_iEnd));
			},
		NULL,
		},
	{
		"length",
		"I",
		"The number of elements in the range.",
		[](const CAEONRange &Obj, const CString &sProperty)
			{
			return Obj.GetLength();
			},
		NULL,
		},
	{
		"start",
		"I",
		"The first value in the range.",
		[](const CAEONRange &Obj, const CString &sProperty)
			{
			return (Obj.IsNil() ? CDatum() : CDatum(Obj.m_iStart));
			},
		NULL,
		},
	{
		"step",
		"I",
		"The increment/decrement by which we step.",
		[](const CAEONRange &Obj, const CString &sProperty)
			{
			return (Obj.IsNil() ? CDatum() : CDatum(Obj.m_iStep));
			},
		NULL,
		},
	};

TDatumMethodHandler<CAEONRange> CAEONRange::m_Methods = {
	{
		"contains",
		"b:n=?",
		".contains(n) -> true/false",
		0,
		[](CAEONRange& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = Obj.Contains(LocalEnv.GetArgument(1));
			return true;
			},
		},
	{
		"max",
		"n:",
		".max() -> value",
		0,
		[](CAEONRange& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = Max(Obj.m_iStart, Obj.m_iEnd);
			return true;
			},
		},
	{
		"min",
		"n:",
		".min() -> value",
		0,
		[](CAEONRange& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = Min(Obj.m_iStart, Obj.m_iEnd);
			return true;
			},
		},
	{
		"reversed",
		"$RangeType:",
		".revered() -> range",
		0,
		[](CAEONRange& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CAEONRange::Create(Obj.m_iEnd, Obj.m_iStart, -Obj.m_iStep);
			return true;
			},
		},
	};

CString CAEONRange::AsString (void) const
	{
	if (m_iStep == 1 || m_iStep == -1)
		return strPattern("%d...%d", m_iStart, m_iEnd);
	else
		return strPattern("%d...%d by %d", m_iStart, m_iEnd, m_iStep);
	}

bool CAEONRange::Contains (CDatum dValue) const
	{
	switch (dValue.GetBasicType())
		{
		case CDatum::typeRange:
			{
			const IAEONRange* pRange = dValue.GetRangeInterface();
			if (pRange == NULL)
				return false;

			int iOtherStart = (int)pRange->GetStart();
			int iOtherEnd = (int)pRange->GetEnd();
			int iOtherStep = (int)pRange->GetStep();

			//	Empty range is always contains

			if (iOtherStep == 0)
				return true;

			//	We contain the other range if the other range is a subset of our range.

			else if (m_iStep > 0)
				{
				if (iOtherStep > 0)
					return (iOtherStart >= m_iStart && iOtherEnd <= m_iEnd && (iOtherStart - m_iStart) % m_iStep == 0);
				else if (iOtherStep < 0)
					return (iOtherStart <= m_iEnd && iOtherEnd >= m_iStart && (m_iStart - iOtherEnd) % -iOtherStep == 0);
				else
					return false;
				}
			else if (m_iStep < 0)
				{
				if (iOtherStep > 0)
					return (iOtherStart >= m_iEnd && iOtherEnd <= m_iStart && (iOtherStart - m_iEnd) % m_iStep == 0);
				else if (iOtherStep < 0)
					return (iOtherStart <= m_iStart && iOtherEnd >= m_iEnd && (m_iStart - iOtherStart) % -m_iStep == 0);
				else
					return false;
				}
			else
				return false;
			}

		default:
			{
			int iValue;
			if (!dValue.IsNumberInt32(&iValue))
				return false;

			if (m_iStep > 0)
				return (iValue >= m_iStart && iValue <= m_iEnd && (iValue - m_iStart) % m_iStep == 0);
			else if (m_iStep < 0)
				return (iValue <= m_iStart && iValue >= m_iEnd && (m_iStart - iValue) % -m_iStep == 0);
			else
				return false;
			}
		}
	}

CDatum CAEONRange::Create (int iStart, int iEnd, int iStep)
	{
	CAEONRange* pValue = new CAEONRange;

	//	Validate the inputs

	if (iStep > 0)
		{
		if (iStart > iEnd)
			{
			//	null range
			return CDatum(pValue);
			}

		//	Adjust the end value to be the last value in the range

		iEnd = iStart + (iEnd - iStart) / iStep * iStep;
		}
	else if (iStep < 0)
		{
		if (iStart < iEnd)
			{
			//	null range
			return CDatum(pValue);
			}

		//	Adjust the end value to be the last value in the range

		iEnd = iStart + (iEnd - iStart) / iStep * iStep;
		}
	else
		{
		//	null range
		return CDatum(pValue);
		}

	pValue->m_iStart = iStart;
	pValue->m_iEnd = iEnd;
	pValue->m_iStep = iStep;

	return CDatum(pValue);
	}

CDatum CAEONRange::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	CAEONRange *pValue = new CAEONRange;
	CDatum dValue(pValue);

	pValue->m_iStart = Stream.ReadInt();
	pValue->m_iEnd = Stream.ReadInt();
	pValue->m_iStep = Stream.ReadInt();

	return dValue;
	}

int CAEONRange::GetLength () const
	{
	if (m_iStep == 0)
		return 0;

	int iLength = (m_iEnd - m_iStart) / m_iStep + 1;
	if (iLength < 0)
		return 0;

	return iLength;
	}

TArray<IDatatype::SMemberDesc> CAEONRange::GetMembers (void)
	{
	TArray<IDatatype::SMemberDesc> Members;

	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);

	return Members;
	}

const CString& CAEONRange::GetTypename (void) const
	{
	return TYPENAME_RANGE;
	}

size_t CAEONRange::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const
	{
	return sizeof(DWORD) * 3;
	}

int CAEONRange::OpCompare (CDatum::Types iValueType, CDatum dValue) const

//	OpCompare
//
//  -2:		If dKey < dKey2 (but not comparable, e.g., string and int)
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2
//	2:		If dKey > dKey2 (but not comparable, e.g., string and int)

	{
	const IAEONRange* pOther = dValue.GetRangeInterface();
	if (pOther)
		{
		if (m_iStart < (int)pOther->GetStart())
			return -1;
		else if (m_iStart > (int)pOther->GetStart())
			return 1;
		else if (m_iEnd < (int)pOther->GetEnd())
			return -1;
		else if (m_iEnd > (int)pOther->GetEnd())
			return 1;
		else if (m_iStep < (int)pOther->GetStep())
			return -1;
		else if (m_iStep > (int)pOther->GetStep())
			return 1;
		else
			return 0;
		}
	else
		{
		int iCompare = KeyCompareNoCase(AsString(), dValue.AsString());
		if (iCompare < 0)
			return -2;
		else if (iCompare > 0)
			return 2;
		else
			return 0;
		}
	}

bool CAEONRange::OpContains (CDatum dValue) const

//	OpContains
//
//	Returns TRUE if dValue is an integer in the range.

	{
	int iValue;
	if (!dValue.IsNumberInt32(&iValue))
		return false;

	if (m_iStep > 0)
		return (iValue >= m_iStart && iValue <= m_iEnd && (iValue - m_iStart) % m_iStep == 0);
	else if (m_iStep < 0)
		return (iValue <= m_iStart && iValue >= m_iEnd && (m_iStart - iValue) % -m_iStep == 0);
	else
		return false;
	}

bool CAEONRange::OpIsEqual (CDatum::Types iValueType, CDatum dValue) const
	{
	if (iValueType == CDatum::typeRange)
		{
		const IAEONRange* pOther = dValue.GetRangeInterface();
		if (pOther == NULL)
			return false;

		return (m_iStart == (int)pOther->GetStart() && m_iEnd == (int)pOther->GetEnd() && m_iStep == (int)pOther->GetStep());
		}
	else
		return false;
	}

void CAEONRange::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const
	{
	switch (iFormat)
		{
		case CDatum::EFormat::GridLang:
			{
			if (m_iStep == 1 || m_iStep == -1)
				Stream.Write(strPattern("%d...%d", m_iStart, m_iEnd));
			else
				Stream.Write(strPattern("%d...%d by %d", m_iStart, m_iEnd, m_iStep));
			break;
			}

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

void CAEONRange::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	Stream.Write(CDatum::SERIALIZE_TYPE_RANGE);

	Stream.Write(m_iStart);
	Stream.Write(m_iEnd);
	Stream.Write(m_iStep);
	}
