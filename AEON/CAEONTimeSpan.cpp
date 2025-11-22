//	CAEONTimeSpan.cpp
//
//	CAEONTimeSpan class
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_MINUS,							"-");

DECLARE_CONST_STRING(TYPENAME_TABLE,					"table");
DECLARE_CONST_STRING(TYPENAME_TIME_SPAN,				"timeSpan");

TDatumPropertyHandler<CAEONTimeSpan> CAEONTimeSpan::m_Properties = {
	{
		"datatype",
		"%",
		"Returns the type of the object.",
		[](const CAEONTimeSpan &Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"days",
		"i",
		"Returns the total number of days (or fractions of a day).",
		[](const CAEONTimeSpan &Obj, const CString &sProperty)
			{
			double rValue = (double)Obj.m_TimeSpan.Milliseconds64() / ((double)SECONDS_PER_DAY * 1000.0);
			return CDatum(Obj.m_TimeSpan.IsNegative() ? -rValue : rValue);
			},
		NULL,
		},
	{
		"milliseconds",
		"i",
		"Returns the total number of milliseconds.",
		[](const CAEONTimeSpan &Obj, const CString &sProperty)
			{
			return CDatum(CIPInteger(Obj.m_TimeSpan.Milliseconds64(), Obj.m_TimeSpan.IsNegative()));
			},
		NULL,
		},
	{
		"seconds",
		"i",
		"Returns the total number of seconds (or fractions of a second).",
		[](const CAEONTimeSpan &Obj, const CString &sProperty)
			{
			double rValue = (double)Obj.m_TimeSpan.Milliseconds64() / (double)1000.0;
			return CDatum(Obj.m_TimeSpan.IsNegative() ? -rValue : rValue);
			},
		NULL,
		},
	};

CString CAEONTimeSpan::AsString (void) const

//	AsString
//
//	Returns the timespan as a string.

	{
	int iTimeMs = m_TimeSpan.MillisecondsSinceMidnight();
	int iMilliseconds = iTimeMs % 1000;
	int iTimeSecs = iTimeMs / 1000;
	int iSeconds = iTimeSecs % 60;
	int iTimeMinutes = iTimeSecs / 60;
	int iMinutes = iTimeMinutes % 60;
	int iHours = iTimeMinutes / 60;

	CString sSign;
	if (m_TimeSpan.IsNegative())
		sSign = STR_MINUS;

	if (m_TimeSpan.Days() == 0)
		{
		if (iMilliseconds == 0)
			return strPattern("%s%02d:%02d:%02d",
					sSign,
					iHours,
					iMinutes,
					iSeconds
					);
		else
			return strPattern("%s%02d:%02d:%02d.%03d",
				sSign,
				iHours,
				iMinutes,
				iSeconds,
				iMilliseconds
			);
		}
	else
		{
		if (iMilliseconds == 0)
			return strPattern("%s%d:%02d:%02d:%02d",
					sSign,
					m_TimeSpan.Days(),
					iHours,
					iMinutes,
					iSeconds
					);
		else
			return strPattern("%s%d:%02d:%02d:%02d.%03d",
				sSign,
				m_TimeSpan.Days(),
				iHours,
				iMinutes,
				iSeconds,
				iMilliseconds
			);
		}
	}

CDatum CAEONTimeSpan::GetElement (int iIndex) const

//	GetElement
//
//	Returns the element.

	{
	switch (iIndex)
		{
		case PART_DAYS:
			return CDatum(m_TimeSpan.Days());

		case PART_MILLISECONDS:
			return CDatum(m_TimeSpan.MillisecondsSinceMidnight());

		case PART_NEGATIVE:
			return CDatum(m_TimeSpan.IsNegative() ? -1 : 1);

		default:
			return CDatum();
		}
	}

CDatum CAEONTimeSpan::GetElement (const CString &sKey) const

//	GetElement
//
//	Returns the element.

	{
	return CDatum();
	}

const CString &CAEONTimeSpan::GetTypename (void) const
	{
	return TYPENAME_TIME_SPAN;
	}

CDatum CAEONTimeSpan::MathAbs () const

//	MathAbs
//
//	Computes absolute value.

	{
	if (m_TimeSpan.IsNegative())
		return CDatum(CTimeSpan(m_TimeSpan, false));
	else
		return CDatum::raw_AsComplex(this);
	}

size_t CAEONTimeSpan::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeAEONScript
//
//	Returns the approximate size of the serialization.

	{
	return 25;
	}

bool CAEONTimeSpan::OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Load

	{
	int iLoad;
	Stream.Read(&iLoad, sizeof(iLoad));
	bool bNegative = (iLoad < 0);
	int iDays = Abs(iLoad) - 1;

	int iMilliseconds;
	Stream.Read(&iMilliseconds, sizeof(iMilliseconds));

	m_TimeSpan = CTimeSpan(iDays, iMilliseconds, bNegative);
	return true;
	}

void CAEONTimeSpan::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize

	{
	//	We encode the sign in the days values, so we need to bias by 1.

	int iSave = (m_TimeSpan.Days() + 1);
	if (m_TimeSpan.IsNegative())
		iSave = -iSave;

	Stream.Write(&iSave, sizeof(iSave));

	//	Write milliseconds

	iSave = m_TimeSpan.MillisecondsSinceMidnight();
	Stream.Write(&iSave, sizeof(iSave));
	}

CDatum CAEONTimeSpan::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	CAEONTimeSpan* pValue = new CAEONTimeSpan;
	CDatum dValue(pValue);

	int iLoad = (int)Stream.ReadDWORD();
	bool bNegative = (iLoad < 0);
	int iDays = Abs(iLoad) - 1;

	int iMilliseconds = (int)Stream.ReadDWORD();

	pValue->m_TimeSpan = CTimeSpan(iDays, iMilliseconds, bNegative);
	return dValue;
	}

void CAEONTimeSpan::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serialize to the given format.

	{
	switch (iFormat)
		{
		case CDatum::EFormat::GridLang:
			Stream.Write(strPattern("TimeSpan(%s)", CIPInteger(m_TimeSpan.Milliseconds64()).AsString()));
			break;

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

void CAEONTimeSpan::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	Stream.Write(CDatum::SERIALIZE_TYPE_TIME_SPAN);

	//	We encode the sign in the days values, so we need to bias by 1.

	int iSave = (m_TimeSpan.Days() + 1);
	if (m_TimeSpan.IsNegative())
		iSave = -iSave;

	Stream.Write(iSave);

	//	Write milliseconds

	iSave = m_TimeSpan.MillisecondsSinceMidnight();
	Stream.Write(iSave);
	}
