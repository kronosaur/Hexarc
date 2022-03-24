//	IComplexDatum.cpp
//
//	IComplexDatum class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_ARRAY,					"array");
DECLARE_CONST_STRING(TYPENAME_DATETIME,					"dateTime");
DECLARE_CONST_STRING(TYPENAME_STRUCT,					"struct");

DECLARE_CONST_STRING(DATETIME_YEAR,						"year");
DECLARE_CONST_STRING(DATETIME_MONTH,					"month");
DECLARE_CONST_STRING(DATETIME_DAY,						"day");
DECLARE_CONST_STRING(DATETIME_HOUR,						"hour");
DECLARE_CONST_STRING(DATETIME_MINUTE,					"minute");
DECLARE_CONST_STRING(DATETIME_SECOND,					"second");
DECLARE_CONST_STRING(DATETIME_MILLISECOND,				"millisecond");

//	IComplexDatum --------------------------------------------------------------

size_t IComplexDatum::CalcSerializeAsStructSize (CDatum::EFormat iFormat) const

//	CalcSerializeAsStructSize
//
//	Returns the approximate serialization size if we serialize as a struct (in
//	bytes).

	{
	size_t TotalSize = 2;

	for (int i = 0; i < GetCount(); i++)
		TotalSize += (size_t)GetKey(i).GetLength() + 2 + GetElement(i).CalcSerializeSize(iFormat);

	return TotalSize;
	}

size_t IComplexDatum::CalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	CalcSerializeSizeAEONScript
//
//	Computes an approximate size.

	{
	size_t TotalSize = 0;
	DWORD dwFlags = OnGetSerializeFlags();

	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			TotalSize = GetTypename().GetLength() + 3;

			//	If this is object is serializable as a struct, then we do that.

			if (dwFlags & FLAG_SERIALIZE_AS_STRUCT)
				TotalSize += OnCalcSerializeSizeAEONScript(iFormat);

			//	Otherwise, serialize as base64 encoding, so we increase the total size by
			//	20%.

			else
				{
				size_t RawSize = OnCalcSerializeSizeAEONScript(iFormat);
				TotalSize += RawSize + (RawSize / 5);
				}
			break;
			}

		default:
			ASSERT(false);
		}

	return TotalSize;
	}

const IDatatype &IComplexDatum::CastIDatatype (void) const

//	CastIDatatype
//
//	Cast to IDatatype (default implementation)

	{
	return (const IDatatype &)CAEONTypeSystem::GetCoreType(IDatatype::ANY);
	}

bool IComplexDatum::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)

//	OnDeserialize
//
//	Descendants can override this to deserialize from a struct.

	{
	for (int i = 0; i < dStruct.GetCount(); i++)
		SetElement(dStruct.GetKey(i), dStruct.GetElement(i));

	return true;
	}

bool IComplexDatum::DeserializeAEONScript (CDatum::EFormat iFormat, const CString &sTypename, CCharStream *pStream)

//	DeserializeAEONScript
//
//	Deserialize AEONScript

	{
	DWORD dwFlags = OnGetSerializeFlags();

	//	If we have an open brace then we've stored everything as a structure.

	if (pStream->GetChar() == '{')
		{
		//	Object must support this

		if (!(dwFlags & FLAG_SERIALIZE_AS_STRUCT))
			return false;

		//	Parse the structure

		CAEONScriptParser Parser(pStream);
		CDatum dData;
		CAEONScriptParser::ETokens iToken = Parser.ParseToken(&dData);
		if (iToken != CAEONScriptParser::tkDatum)
			return false;

		//	Take all the fields in the structure and apply them to our object
		//	(our descendants will do the right thing).

		if (!OnDeserialize(iFormat, dData))
			return false;
		}

	//	Otherwise we expect base64 encoded data

	else
		{
		//	Backup one character because we want the OnDeserialize call to read it.

		pStream->UnreadChar();

		//	Deserialize

		CBase64Decoder Decoder(pStream->GetByteStream());
		if (!OnDeserialize(iFormat, sTypename, Decoder))
			return false;

		//	Read the next character into the stream 

		pStream->RefreshStream();
		pStream->ReadChar();
		}

	return true;
	}

bool IComplexDatum::DeserializeJSON (const CString &sTypename, const TArray<CDatum> &Data)

//	DeserializeJSON
//
//	Deserialize from JSON

	{
	if (Data.GetCount() == 0 || Data[0].GetBasicType() != CDatum::typeString)
		return false;

	//	LATER: Handle serialization/deserialization of struct-based objects

	//	Default deserialization

	CStringBuffer Buffer(Data[0]);
	CBase64Decoder Decoder(&Buffer);
	if (!OnDeserialize(CDatum::EFormat::JSON, sTypename, Decoder))
		return false;

	return true;
	}

CDatum IComplexDatum::GetDatatype () const

//	GetDatatype
//
//	Default implementation. This should only be used for internal datum types.

	{
	return CAEONTypeSystem::GetCoreType(IDatatype::ANY);
	}

void IComplexDatum::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize all elements for obects that are serialized as structures.

	{
	for (int i = 0; i < GetCount(); i++)
		pStruct->SetElement(GetKey(i), GetElement(i));
	}

void IComplexDatum::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serialize the datum

	{
	DWORD dwFlags = OnGetSerializeFlags();

	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			if (!(dwFlags & FLAG_SERIALIZE_NO_TYPENAME))
				{
				Stream.Write("[", 1);
				Stream.Write(GetTypename());
				Stream.Write(":", 1);
				}

			//	If this is object is serializable as a struct, then we do that.

			if (dwFlags & FLAG_SERIALIZE_AS_STRUCT)
				{
				CComplexStruct *pStruct = new CComplexStruct;

				OnSerialize(iFormat, pStruct);

				CDatum dDatum(pStruct);
				dDatum.Serialize(iFormat, Stream);
				}

			//	Otherwise, serialize as base64 encoding

			else
				{
				CBase64Encoder Encoder(&Stream);
				OnSerialize(iFormat, Encoder);
				Encoder.Close();
				}

			if (!(dwFlags & FLAG_SERIALIZE_NO_TYPENAME))
				Stream.Write("]", 1);
			break;
			}

		case CDatum::EFormat::JSON:
			{
			if (!(dwFlags & FLAG_SERIALIZE_NO_TYPENAME))
				{
				Stream.Write("[\"AEON2011:", 11);
				Stream.Write(GetTypename());
				Stream.Write(":v1\", ", 6);
				}

			if (dwFlags & FLAG_SERIALIZE_AS_STRUCT)
				{
				CComplexStruct *pStruct = new CComplexStruct;

				OnSerialize(iFormat, pStruct);

				CDatum dDatum(pStruct);
				dDatum.Serialize(iFormat, Stream);
				}
			else
				{
				Stream.Write("\"", 1);

				CBase64Encoder Encoder(&Stream);
				OnSerialize(iFormat, Encoder);
				Encoder.Close();

				Stream.Write("\"", 1);
				}

			if (!(dwFlags & FLAG_SERIALIZE_NO_TYPENAME))
				Stream.Write("]", 1);
			break;
			}

		default:
			ASSERT(false);
		}
	}

void IComplexDatum::SerializeAsStruct (CDatum::EFormat iFormat, IByteStream &Stream) const

//	SerializeAsStruct
//
//	Helper function to serialize the object as if it were a struct.

	{
	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			Stream.Write("{", 1);

			for (int i = 0; i < GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(" ", 1);

				//	Write the key

				CDatum Key(GetKey(i));
				Key.Serialize(iFormat, Stream);

				//	Separator

				Stream.Write(":", 1);

				//	Write the value

				GetElement(i).Serialize(iFormat, Stream);
				}

			Stream.Write("}", 1);
			break;
			}

		case CDatum::EFormat::JSON:
			{
			Stream.Write("{", 1);

			for (int i = 0; i < GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(", ", 2);

				//	Write the key

				CDatum Key(GetKey(i));
				Key.Serialize(iFormat, Stream);

				//	Separator

				Stream.Write(": ", 2);

				//	Write the value

				GetElement(i).Serialize(iFormat, Stream);
				}

			Stream.Write("}", 1);
			break;
			}

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

CString IComplexDatum::StructAsString () const

//	StructAsString
//
//	Returns a struct-type object as a string.

	{
	CStringBuffer Output;

	Output.Write("{", 1);

	for (int i = 0; i < GetCount(); i++)
		{
		if (i != 0)
			Output.Write(" ", 1);

		Output.Write(GetKey(i));
		Output.Write(":", 1);

		Output.Write(GetElement(i).AsString());
		}

	Output.Write("}", 1);

	return CString(Output);
	}

void IComplexDatum::WriteBinaryToStream (IByteStream &Stream, int iPos, int iLength, IProgressEvents *pProgress) const

//	WriteBinaryToStream
	
	{
	const CString &sData = CastCString();
	if (iPos >= sData.GetLength())
		return;

	if (pProgress)
		pProgress->OnProgressStart();

	if (iLength == -1)
		iLength = Max(0, sData.GetLength() - iPos);
	else
		iLength = Min(iLength, sData.GetLength() - iPos);

	Stream.Write(sData.GetPointer() + iPos, iLength); 

	if (pProgress)
		pProgress->OnProgressDone();
	}

//	CComplexArray --------------------------------------------------------------

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

bool CComplexDateTime::CreateFromString (const CString &sString, CDateTime *retDateTime)

//	CreateFromString
//
//	Reverse of AsString

	{
	const char *pPos = sString.GetParsePointer();

	int iYear = strParseInt(pPos, 0, &pPos);
	if (iYear < 1 || iYear > 30827)
		return false;

	if (*pPos++ != '-')
		return false;

	int iMonth = strParseInt(pPos, 0, &pPos);
	if (iMonth < 1 || iMonth > 12)
		return false;

	if (*pPos++ != '-')
		return false;

	int iDay = strParseInt(pPos, 0, &pPos);
	if (iDay < 1 || iDay > CDateTime::GetDaysInMonth(iMonth, iYear))
		return false;

	if (*pPos++ != 'T')
		return false;

	int iHour = strParseInt(pPos, -1, &pPos);
	if (iHour < 0 || iHour > 23)
		return false;

	if (*pPos++ != ':')
		return false;

	int iMinute = strParseInt(pPos, -1, &pPos);
	if (iMinute < 0 || iMinute > 59)
		return false;

	if (*pPos++ != ':')
		return false;

	int iSecond = strParseInt(pPos, -1, &pPos);
	if (iSecond < 0 || iSecond > 59)
		return false;

	if (*pPos++ != '.')
		return false;

	int iMillisecond = strParseInt(pPos, -1, &pPos);
	if (iMillisecond < 0 || iMillisecond > 999)
		return false;

	//	Done

	*retDateTime = CDateTime(iDay, iMonth, iYear, iHour, iMinute, iSecond, iMillisecond);
	return true;
	}

bool CComplexDateTime::CreateFromString (const CString &sString, CDatum *retdDatum)

//	CreateFromString
//
//	Creates a datum from a string

	{
	CDateTime DateTime;
	if (!CreateFromString(sString, &DateTime))
		return false;

	*retdDatum = CDatum(DateTime);
	return true;
	}

CDatum CComplexDateTime::GetElement (int iIndex) const

//	GetElement
//
//	Returns a dateTime component

	{
	switch (iIndex)
		{
		case partYear:
			return CDatum(m_DateTime.Year());

		case partMonth:
			return CDatum(m_DateTime.Month());

		case partDay:
			return CDatum(m_DateTime.Day());

		case partHour:
			return CDatum(m_DateTime.Hour());

		case partMinute:
			return CDatum(m_DateTime.Minute());

		case partSecond:
			return CDatum(m_DateTime.Second());

		case partMillisecond:
			return CDatum(m_DateTime.Millisecond());

		default:
			return CDatum();
		}
	}

CDatum CComplexDateTime::GetElement (const CString &sKey) const

//	GetElement
//
//	Returns a dateTime component

	{
	if (strEquals(sKey, DATETIME_YEAR))
		return CDatum(m_DateTime.Year());
	else if (strEquals(sKey, DATETIME_MONTH))
		return CDatum(m_DateTime.Month());
	else if (strEquals(sKey, DATETIME_DAY))
		return CDatum(m_DateTime.Day());
	else if (strEquals(sKey, DATETIME_HOUR))
		return CDatum(m_DateTime.Hour());
	else if (strEquals(sKey, DATETIME_MINUTE))
		return CDatum(m_DateTime.Minute());
	else if (strEquals(sKey, DATETIME_SECOND))
		return CDatum(m_DateTime.Second());
	else if (strEquals(sKey, DATETIME_MILLISECOND))
		return CDatum(m_DateTime.Millisecond());
	else
		return CDatum();
	}

const CString &CComplexDateTime::GetTypename (void) const { return TYPENAME_DATETIME; }

size_t CComplexDateTime::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns an approximation of serialization size.

	{
	return 25;
	}

void CComplexDateTime::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serialize to the given format.

	{
	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			CString sDate = strPattern("#%d-%02d-%02dT%02d:%02d:%02d.%03d",
					m_DateTime.Year(),
					m_DateTime.Month(),
					m_DateTime.Day(),
					m_DateTime.Hour(),
					m_DateTime.Minute(),
					m_DateTime.Second(),
					m_DateTime.Millisecond());

			Stream.Write(sDate);
			break;
			}

		case CDatum::EFormat::JSON:
			{
			CString sDate = strPattern("\"%d-%02d-%02dT%02d:%02d:%02d.%03d\"",
					m_DateTime.Year(),
					m_DateTime.Month(),
					m_DateTime.Day(),
					m_DateTime.Hour(),
					m_DateTime.Minute(),
					m_DateTime.Second(),
					m_DateTime.Millisecond());

			Stream.Write(sDate);
			break;
			}

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

//	CComplexStruct -------------------------------------------------------------

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

void CComplexStruct::OnMarked (void)

//	OnMarked
//
//	Mark

	{
	for (int i = 0; i < m_Map.GetCount(); i++)
		m_Map[i].Mark();
	}

