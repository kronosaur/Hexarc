//	CDatum.cpp
//
//	CDatum class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_DOUBLE,					"double");
DECLARE_CONST_STRING(TYPENAME_ENUM,						"enum");
DECLARE_CONST_STRING(TYPENAME_INT32,					"integer32");
DECLARE_CONST_STRING(TYPENAME_NIL,						"nil");
DECLARE_CONST_STRING(TYPENAME_STRING,					"string");
DECLARE_CONST_STRING(TYPENAME_TRUE,						"true");

DECLARE_CONST_STRING(STR_NAN,							"nan");
DECLARE_CONST_STRING(STR_NIL,							"nil");
DECLARE_CONST_STRING(STR_TRUE,							"true");

DECLARE_CONST_STRING(ERR_UNKNOWN_FORMAT,				"Unable to determine file format: %s.");
DECLARE_CONST_STRING(ERR_CANT_OPEN_FILE,				"Unable to open file: %s.");
DECLARE_CONST_STRING(ERR_DESERIALIZE_ERROR,				"Unable to parse file: %s.");
DECLARE_CONST_STRING(ERR_NO_METHODS,					"Methods not supported.");
DECLARE_CONST_STRING(ERR_INVALID_TABLE_DESC,			"Invalid table descriptor.");

static CAEONFactoryList *g_pFactories = NULL;
static int g_iUnalignedLiteralCount = 0;

CDatum::SAnnotation CDatum::m_NullAnnotation;

CDatum::CDatum (Types iType)

//	CDatum constructor

	{
	switch (iType)
		{
		case typeNil:
			m_dwData = 0;
			break;

		case typeTrue:
			m_dwData = CONST_TRUE;
			break;

		case typeInteger32:
			*this = CDatum((int)0);
			break;

		case typeString:
			*this = CDatum(NULL_STR);
			break;

		case typeArray:
			*this = CDatum(new CComplexArray);
			break;

		case typeDouble:
			*this = CDatum((double)0.0);
			break;

		case typeStruct:
			*this = CDatum(new CComplexStruct);
			break;

		case typeDateTime:
			*this = CDatum(CDateTime(CDateTime::Now));
			break;

		case typeIntegerIP:
			*this = CDatum(CIPInteger(0));
			break;

		case typeInteger64:
			*this = CDatum((DWORDLONG)0);
			break;

		case typeObject:
			*this = CDatum(new CAEONObject(CAEONTypeSystem::GetCoreType(IDatatype::ANY)));
			break;

		case typeNaN:
			m_dwData = CONST_NAN;
			break;

		default:
			ASSERT(false);
			m_dwData = 0;
			break;
		}
	}

CDatum::CDatum (Types iType, const CString& sValue)

//	CDatum constructor

	{
	switch (iType)
		{
		case typeDateTime:
			{
			CDateTime DateTime;
			if (CComplexDateTime::CreateFromString(sValue, &DateTime)
					&& DateTime.IsValid())
				{
				CComplexDateTime *pDateTime = new CComplexDateTime(DateTime);

				//	Take ownership of the complex type

				CAEONStore::Alloc(pDateTime);

				//	Store the pointer and assign type

				m_dwData = ((DWORD_PTR)pDateTime | AEON_TYPE_COMPLEX);
				}
			else
				m_dwData = 0;

			break;
			}

		case typeString:
			*this = CDatum(sValue);
			break;

		default:
			throw CException(errFail);
		}
	}

CDatum::CDatum (int iValue)

//	CDatum constructor

	{
	DWORD dwValue = (DWORD)iValue;
	m_dwData = MAKEDWORDLONG((DWORD)AEON_NUMBER_INTEGER, dwValue);
	}

CDatum::CDatum (DWORD dwValue)

//	CDatum constructor

	{
	m_dwData = MAKEDWORDLONG((DWORD)AEON_NUMBER_INTEGER, dwValue);
	}

CDatum::CDatum (DWORDLONG ilValue)

//	CDatum constructor

	{
	//	Store as IP integer

	CComplexInteger *pIPInt = new CComplexInteger(ilValue);

	//	Take ownership of the complex type

	CAEONStore::Alloc(pIPInt);

	//	Store the pointer and assign type

	m_dwData = ((DWORD_PTR)pIPInt | AEON_TYPE_COMPLEX);
	}

CDatum::CDatum (double rValue)

//	CDatum constructor

	{
	DWORD_PTR dwID = CAEONStore::Alloc(rValue);
	m_dwData = (dwID << 4) | AEON_NUMBER_DOUBLE;
	}

CDatum::CDatum (const CString &sString)

//	CDatum constructor

	{
	//	Empty strings are always nil

	if (sString.IsEmpty())
		{
		m_dwData = 0;
		return;
		}

	//	If this is a literal string, then we can just
	//	take the value, because it doesn't need to be freed.
	//	NOTE: We can only do this if the literal pointer is
	//	DWORD aligned.

	if (sString.IsLiteral())
		{
		m_dwData = (DWORD_PTR)(LPSTR)sString;
		if ((m_dwData & AEON_TYPE_MASK) == AEON_TYPE_STRING)
			return;

#ifdef DEBUG
		else
			g_iUnalignedLiteralCount++;
#endif
		}

	//	Get a copy of the naked LPSTR out of the string

	CString sNewString(sString);
	LPSTR pString = sNewString.Handoff();

	//	Track it with our allocator (but only if not NULL).
	//	(If pString is NULL then this is represented as Nil).

	if (pString)
		CAEONStore::Alloc(pString);

	//	Store the pointer

	m_dwData = (DWORD_PTR)pString;

	//	No need to mark the type (because TYPE_STRING is 0)

	ASSERT(AEON_TYPE_STRING == 0x00);
	}

CDatum::CDatum (CString &&sString)

//	CDatum constructor

	{
	CreateStringFromHandoff(sString, this);
	}

CDatum::CDatum (CStringBuffer &&String)
	{
	CreateStringFromHandoff(String, this);
	}

CDatum::CDatum (IComplexDatum *pValue)

//	CDatum constructor

	{
	ASSERT(pValue);

	//	Take ownership of the complex type

	CAEONStore::Alloc(pValue);

	//	Store the pointer and assign type

	m_dwData = ((DWORD_PTR)pValue | AEON_TYPE_COMPLEX);
	}

CDatum::CDatum (const CDateTime &DateTime)

//	CDatum constructor

	{
	if (DateTime.IsValid())
		{
		CComplexDateTime *pDateTime = new CComplexDateTime(DateTime);

		//	Take ownership of the complex type

		CAEONStore::Alloc(pDateTime);

		//	Store the pointer and assign type

		m_dwData = ((DWORD_PTR)pDateTime | AEON_TYPE_COMPLEX);
		}
	else
		m_dwData = 0;
	}

CDatum::CDatum (const CIPInteger &Value)

//	CDatum constructor

	{
	CComplexInteger *pValue = new CComplexInteger(Value);

	//	Take ownership of the complex type

	CAEONStore::Alloc(pValue);

	//	Store the pointer and assign type

	m_dwData = ((DWORD_PTR)pValue | AEON_TYPE_COMPLEX);

	if (m_dwData == 0x46d)
		throw CException(errFail);
	}

CDatum::CDatum (const CRGBA32Image &Value)

//	CDatum constructor

	{
	CComplexImage32 *pValue = new CComplexImage32(Value);

	//	Take ownership of the complex type

	CAEONStore::Alloc(pValue);

	//	Store the pointer and assign type

	m_dwData = ((DWORD_PTR)pValue | AEON_TYPE_COMPLEX);
	}

CDatum::CDatum (CRGBA32Image &&Value)

//	CDatum constructor

	{
	CComplexImage32 *pValue = new CComplexImage32(std::move(Value));

	//	Take ownership of the complex type

	CAEONStore::Alloc(pValue);

	//	Store the pointer and assign type

	m_dwData = ((DWORD_PTR)pValue | AEON_TYPE_COMPLEX);
	}

CDatum::CDatum (const CTimeSpan &TimeSpan)

//	CDatum constructor

	{
	CAEONTimeSpan *pTimeSpan = new CAEONTimeSpan(TimeSpan);

	//	Take ownership of the complex type

	CAEONStore::Alloc(pTimeSpan);

	//	Store the pointer and assign type

	m_dwData = ((DWORD_PTR)pTimeSpan | AEON_TYPE_COMPLEX);
	}

CDatum::CDatum (const CVector2D& Value)

//	CDatum constructor

	{
	auto pVector = new CAEONVector2D(Value);
	CAEONStore::Alloc(pVector);
	m_dwData = ((DWORD_PTR)pVector | AEON_TYPE_COMPLEX);
	}

CDatum::operator int () const

//	int cast operator

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			return (m_dwData == 0 ? 0 : strToInt(raw_GetString()));

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData)
						{
						case CONST_NAN:
							return 0;

						case CONST_TRUE:
							return 1;

						default:
							ASSERT(false);
							return 0;
						}
					}

				case AEON_NUMBER_INTEGER:
				case AEON_NUMBER_ENUM:
					return (int)HIDWORD(m_dwData);

				case AEON_NUMBER_DOUBLE:
					return (int)CAEONStore::GetDouble(GetNumberIndex());

				default:
					ASSERT(false);
					return 0;
				}

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->CastInteger32();

		default:
			ASSERT(false);
			return 0;
		}
	}

CDatum::operator DWORD () const

//	DWORD cast operator

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			return (m_dwData == 0 ? 0 : (DWORD)strToInt(raw_GetString()));

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData)
						{
						case CONST_NAN:
							return 0;

						case CONST_TRUE:
							return 1;

						default:
							ASSERT(false);
							return 0;
						}
					}

				case AEON_NUMBER_INTEGER:
				case AEON_NUMBER_ENUM:
					return HIDWORD(m_dwData);

				case AEON_NUMBER_DOUBLE:
					return (DWORD)CAEONStore::GetDouble(GetNumberIndex());

				default:
					ASSERT(false);
					return 0;
				}

		case AEON_TYPE_COMPLEX:
			return (DWORD)raw_GetComplex()->CastInteger32();

		default:
			ASSERT(false);
			return 0;
		}
	}

CDatum::operator DWORDLONG () const

//	DWORDLONG operator

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			return 0;	//	LATER: See if we can convert

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData)
						{
						case CONST_NAN:
							return 0;

						case CONST_TRUE:
							return 1;

						default:
							ASSERT(false);
							return 0;
						}
					}

				case AEON_NUMBER_INTEGER:
				case AEON_NUMBER_ENUM:
					return (DWORDLONG)HIDWORD(m_dwData);

				case AEON_NUMBER_DOUBLE:
					return (DWORDLONG)CAEONStore::GetDouble(GetNumberIndex());

				default:
					ASSERT(false);
					return 0;
				}

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->CastDWORDLONG();

		default:
			ASSERT(false);
			return 0;
		}
	}

CDatum::operator double () const

//	double cast operator

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			return (m_dwData == 0 ? 0.0 : strToDouble(raw_GetString()));

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData)
						{
						case CONST_NAN:
							return std::nan("");

						case CONST_TRUE:
							return 1.0;

						default:
							return CreateNaN();
						}
					}

				case AEON_NUMBER_INTEGER:
				case AEON_NUMBER_ENUM:
					return (double)(int)HIDWORD(m_dwData);

				case AEON_NUMBER_DOUBLE:
					return CAEONStore::GetDouble(GetNumberIndex());

				default:
					return CreateNaN();
				}

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->CastDouble();

		default:
			return CreateNaN();
		}
	}

CDatum::operator const IDatatype & () const

//	IDatatype cast operator

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->CastIDatatype();

		default:
			return (const IDatatype &)CAEONTypeSystem::GetCoreType(IDatatype::ANY);
		}
	}

CDatum::operator const CDateTime & () const

//	CDateTime cast operator

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->CastCDateTime();

		default:
			return NULL_DATETIME;
		}
	}

CDatum::operator const CTimeSpan & () const

//	CTimeSpan cast operator

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->CastCTimeSpan();

		default:
			return CTimeSpan::Null();
		}
	}

CDatum::operator const CRGBA32Image & () const

//	CRGBA32Image cast operator

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->CastCRGBA32Image();

		default:
			return CRGBA32Image::Null();
		}
	}

CDatum::operator const CIPInteger & () const

//	CIPInteger cast operator
//
//	NOTE: The difference between this and AsIPInteger() is that
//	AsIPInteger returns an object by value, which allow for
//	more conversion types. This returns a CIPInteger by reference.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->CastCIPInteger();

		default:
			return NULL_IPINTEGER;
		}
	}

CDatum::operator const CString & () const

//	CString cast operator
//
//	NOTE: The difference between this and AsString() is that
//	AsString returns a string by value, which allow for
//	more conversion types. This returns a string by reference.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			return (m_dwData == 0 ? NULL_STR : raw_GetString());

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData)
						{
						case CONST_NAN:
							return STR_NAN;

						case CONST_TRUE:
							return STR_TRUE;

						default:
							ASSERT(false);
							return NULL_STR;
						}
					}

				case AEON_NUMBER_INTEGER:
				case AEON_NUMBER_DOUBLE:
				case AEON_NUMBER_ENUM:
					return NULL_STR;

				default:
					ASSERT(false);
					return NULL_STR;
				}

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->CastCString();

		default:
			ASSERT(false);
			return NULL_STR;
		}
	}

CDatum::operator const CVector2D& () const

//	CVector2D cast operator

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->CastCVector2D();

		default:
			return CVector2D::Null;
		}
	}

void CDatum::Append (CDatum dValue)

//	Append
//
//	Appends the value to the datum. Note that this modifies the datum in place.
//	Do not use this unless you are sure about who else might have a pointer
//	to the datum.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			raw_GetComplex()->Append(dValue);
			break;

		default:
			//	Nothing happens
			break;
		}
	}

int CDatum::AsArrayIndex () const

//	AsArrayIndex
//
//	Converts to a 32-bit signed index. If we return -1, then conversion failed.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			{
			if (m_dwData == 0)
				return -1;
			else
				{
				CDatum dNumberValue;
				if (!CDatum::CreateFromStringValue(*this, &dNumberValue)
						|| !dNumberValue.IsNumber())
					return -1;

				return dNumberValue.AsArrayIndex();
				}
			}

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData)
						{
						case CONST_NAN:
						case CONST_TRUE:
							return -1;

						default:
							return -1;
						}
					}

				case AEON_NUMBER_INTEGER:
				case AEON_NUMBER_ENUM:
					return (int)HIDWORD(m_dwData);

				case AEON_NUMBER_DOUBLE:
					return (int)CAEONStore::GetDouble(GetNumberIndex());

				default:
					return -1;
				}

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->AsArrayIndex();

		default:
			return -1;
		}
	}

void CDatum::AsAttributeList (CAttributeList *retAttribs) const

//	AsAttributeList
//
//	Generates an attribute list

	{
	int i;

	retAttribs->DeleteAll();
	for (i = 0; i < GetCount(); i++)
		retAttribs->Insert(GetElement(i));
	}

CDateTime CDatum::AsDateTime (void) const

//	AsDateTime
//
//	Coerces to a CDateTime

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			{
			if (m_dwData == 0)
				return NULL_DATETIME;

			CDateTime DateTime;
			if (!CComplexDateTime::CreateFromString(raw_GetString(), &DateTime))
				return NULL_DATETIME;

			return DateTime;
			}

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->CastCDateTime();

		default:
			return NULL_DATETIME;
		}
	}

CIPInteger CDatum::AsIPInteger () const

//	AsIPInteger
//
//	Returns an IPInteger

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			{
			if (m_dwData == 0)
				return CIPInteger(0);
			else
				{
				CIPInteger Result;
				Result.InitFromString(raw_GetString());
				return Result;
				}
			}

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData)
						{
						case CONST_NAN:
							return CIPInteger(0);

						case CONST_TRUE:
							return CIPInteger(1);

						default:
							ASSERT(false);
							return CIPInteger(0);
						}
					}

				case AEON_NUMBER_INTEGER:
				case AEON_NUMBER_ENUM:
					return CIPInteger((int)HIDWORD(m_dwData));

				case AEON_NUMBER_DOUBLE:
					return CIPInteger(CAEONStore::GetDouble(GetNumberIndex()));

				default:
					ASSERT(false);
					return CIPInteger(0);
				}

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->CastCIPInteger();

		default:
			ASSERT(false);
			return CIPInteger(0);
		}
	}

CDatum CDatum::AsOptions (bool *retbConverted) const

//	AsOptions
//
//	Always returns as a structure, converting as appropriate. retbConverted is
//	set to TRUE if we convert.

	{
	int i;

	Types iType = GetBasicType();
	bool bConverted = false;
	CDatum dResult;

	if (IsNil() || iType == typeStruct)
		{ }
	else if (iType == typeString)
		{
		const CString &sValue = *this;
		if (!sValue.IsEmpty())
			{
			dResult = CDatum(new CComplexStruct);
			dResult.SetElement(sValue, CDatum(true));
			}

		bConverted = true;
		}
	else if (iType == typeArray)
		{
		dResult = CDatum(new CComplexStruct);
		for (i = 0; i < GetCount(); i++)
			{
			CString sValue = GetElement(i).AsString();
			if (!sValue.IsEmpty())
				dResult.SetElement(sValue, CDatum(true));
			}

		if (dResult.GetCount() == 0)
			dResult = CDatum();

		bConverted = true;
		}
	else
		{
		bConverted = true;
		}

	//	Done

	if (retbConverted) *retbConverted = bConverted;
	return dResult;
	}

CString CDatum::AsString (void) const

//	AsString
//
//	Coerces to a CString

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			return (m_dwData == 0 ? NULL_STR : raw_GetString());

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData)
						{
						case CONST_NAN:
							return STR_NAN;

						case CONST_TRUE:
							return STR_TRUE;

						default:
							ASSERT(false);
							return NULL_STR;
						}
					}

				case AEON_NUMBER_INTEGER:
					return strFromInt((int)HIDWORD(m_dwData));

				case AEON_NUMBER_ENUM:
					{
					int iValue = (int)HIDWORD(m_dwData);
					DWORD dwDatatypeID = GetNumberIndex();
					CDatum dType = CAEONTypes::Get(dwDatatypeID);
					const IDatatype& Datatype = dType;
					int iIndex = Datatype.FindMemberByOrdinal(iValue);

					if (iIndex == -1)
						return NULL_STR;
					else
						return Datatype.GetMember(iIndex).sName;
					}

				case AEON_NUMBER_DOUBLE:
					return strFromDouble(CAEONStore::GetDouble(GetNumberIndex()));

				default:
					ASSERT(false);
					return NULL_STR;
				}

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->AsString();

		default:
			ASSERT(false);
			return NULL_STR;
		}
	}

TArray<CString> CDatum::AsStringArray (void) const

//	AsStringArray
//
//	Coerces to an array of strings.

	{
	int i;

	TArray<CString> Result;
	Result.InsertEmpty(GetCount());
	for (i = 0; i < GetCount(); i++)
		Result[i] = GetElement(i).AsString();

	return Result;
	}

size_t CDatum::CalcMemorySize (void) const

//	CalcMemorySize
//
//	Computes the amount of memory used up by this datum.

	{
	size_t dwSize = sizeof(CDatum);

	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			if (m_dwData != 0)
				dwSize += raw_GetString().GetLength() + sizeof(DWORD) + 1;
			break;

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					break;

				case AEON_NUMBER_INTEGER:
				case AEON_NUMBER_ENUM:
					break;

				case AEON_NUMBER_DOUBLE:
					dwSize += sizeof(double);
					break;

				default:
					ASSERT(false);
					break;
				}
			break;

		case AEON_TYPE_COMPLEX:
			dwSize += raw_GetComplex()->CalcMemorySize();
			break;

		default:
			ASSERT(false);
			break;
		}

	return dwSize;
	}

size_t CDatum::CalcSerializeSize (EFormat iFormat) const

//	CalcSerializeSize
//
//	Computes the size needed to serialize this datum.

	{
	switch (iFormat)
		{
		case EFormat::AEONScript:
		case EFormat::AEONLocal:
			return CalcSerializeSizeAEONScript(iFormat);

		case EFormat::JSON:
			ASSERT(false);	//	Not Yet Implemented
			return 0;

		default:
			ASSERT(false);
			return 0;
		}
	}

bool CDatum::CanInvoke (void) const

//	CanInvoke
//
//	Returns TRUE if we can invoke it.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->CanInvoke();

		default:
			return false;
		}
	}

CDatum CDatum::Clone (EClone iMode) const

//	Clone
//
//	Returns a (shallow) clone of our datum.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		//	Complex types are the only ones with pointer to other datums

		case AEON_TYPE_COMPLEX:
			{
			auto pClone = raw_GetComplex()->Clone(iMode);
			if (pClone)
				return CDatum(pClone);

			//	Else, it's immutable.

			else
				return *this;
			}

		default:
			return *this;
		}
	}

bool CDatum::Contains (CDatum dValue, TArray<IComplexDatum *> &retChecked) const

//	Contains
//
//	Returns TRUE this datum contains dValue.

	{
	if (dValue.m_dwData == m_dwData)
		return true;

	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			{
			auto pComplex = raw_GetComplex();

			if (retChecked.Find(pComplex))
				return false;

			retChecked.Insert(pComplex);

			return pComplex->Contains(dValue, retChecked);
			}

		default:
			return false;
		}
	}

CDatum CDatum::CreateAnnotated (CDatum dValue, const SAnnotation& Annotation)

//	CreateAnnotated
//
//	Create an annotated value.

	{
	return CAEONAnnotated::Create(dValue, Annotation);
	}

CDatum CDatum::CreateArrayAsType (CDatum dType, CDatum dValue)

//	CreateArrayAsType
//
//	Creates an array of the given type.

	{
	const IDatatype &Type = dType;
	if (Type.GetClass() != IDatatype::ECategory::Array
			|| Type.GetMemberCount() < 1
			|| Type.GetMember(0).iType != IDatatype::EMemberType::ArrayElement)
		return CDatum();

	CDatum dArray;
	switch (Type.GetCoreType())
		{
		case IDatatype::INT_32:
			dArray = CDatum::VectorOf(CDatum::typeInteger32);
			break;

		case IDatatype::INT_IP:
			dArray = CDatum::VectorOf(CDatum::typeIntegerIP);
			break;

		case IDatatype::FLOAT_64:
			dArray = CDatum::VectorOf(CDatum::typeDouble);
			break;

		case IDatatype::STRING:
			dArray = CDatum::VectorOf(CDatum::typeString);
			break;

		default:
			//	For anything else, we create a generic array.

			dArray = CDatum::VectorOf(CDatum::typeUnknown);
			break;
		}

	//	Copy data

	if (!dValue.IsNil())
		{
		dArray.GrowToFit(dValue.GetCount());

		for (int i = 0; i < dValue.GetCount(); i++)
			dArray.Append(dValue.GetElement(i));
		}

	return dArray;
	}

CDatum CDatum::CreateAsType (CDatum dType, CDatum dValue)

//	CreateAsType
//
//	Create a new datum of the given type.

	{
	const IDatatype &Type = dType;

	switch (Type.GetClass())
		{
		case IDatatype::ECategory::Array:
			return CreateArrayAsType(dType, dValue);

		case IDatatype::ECategory::ClassDef:
			return CreateObject(dType, dValue);

		case IDatatype::ECategory::Schema:
			return CreateTable(dType, dValue);

		default:
			return dValue;
		}
	}

bool CDatum::CreateBinary (IByteStream &Stream, int iSize, CDatum *retDatum)

//	CreateBinary
//
//	Creates a string datum containing binary data from the stream.
//	If iSize is -1 then we read as much as the stream has.
//	Otherwise  we read up to iSize.

	{
	//	LATER: Handle streams more than 2GB. Instead of asking how much space
	//	is left, maybe we should just ask to truncate the size that we're
	//	requesting.
	int iDataRemaining = Stream.GetStreamLength() - Stream.GetPos();
	int iBinarySize = (iSize < 0 ? iDataRemaining : Min(iDataRemaining, iSize));

	//	0-size

	if (iBinarySize == 0)
		{
		*retDatum = CDatum();
		return true;
		}

	//	Read the stream

	CComplexBinary *pBinary;
	try
		{
		pBinary = new CComplexBinary(Stream, iBinarySize);
		}
	catch (...)
		{
		return false;
		}

	//	Done

	*retDatum = CDatum(pBinary);

	//	Done

	return true;
	}

bool CDatum::CreateBinaryFromHandoff (CStringBuffer &Buffer, CDatum *retDatum)

//	CreateBinaryFromHandoff
//
//	Creates a binary datum

	{
	CComplexBinary *pBinary = new CComplexBinary;
	pBinary->TakeHandoff(Buffer);
	*retDatum = CDatum(pBinary);
	return true;
	}

CDatum CDatum::CreateEnum (int iValue, DWORD dwTypeID)

//	CreateEnum
//
//	Creates an enum value datum.

	{
	CDatum dResult;
	dResult.m_dwData = MAKEDWORDLONG((dwTypeID << 4) | (DWORD)AEON_NUMBER_ENUM, (DWORD)iValue);
	return dResult;
	}

CDatum CDatum::CreateEnum (int iValue, CDatum dType)

//	CreateEnum
//
//	Creates an enum value datum.

	{
	DWORD dwTypeID = CAEONTypes::Add(dType);
	return CreateEnum(iValue, dwTypeID);
	}

CDatum CDatum::CreateError (const CString& sErrorDesc, const CString& sErrorCode)

//	CreateError
//
//	Creates an error value.

	{
	return CAEONError::Create(sErrorCode, sErrorDesc);
	}

bool CDatum::CreateFromAttributeList (const CAttributeList &Attribs, CDatum *retdDatum)

//	CreateFromAttributeList
//
//	Creates a datum from an attribute list

	{
	int i;

	TArray<CString> AllAttribs;
	Attribs.GetAll(&AllAttribs);

	if (AllAttribs.GetCount() == 0)
		{
		*retdDatum = CDatum();
		return true;
		}

	CComplexArray *pArray = new CComplexArray;
	for (i = 0; i < AllAttribs.GetCount(); i++)
		pArray->Insert(AllAttribs[i]);

	*retdDatum = CDatum(pArray);
	return true;
	}

bool CDatum::CreateFromFile (const CString &sFilespec, EFormat iFormat, CDatum *retdDatum, CString *retsError)

//	CreateFromFile
//
//	Loads a datum from a file.

	{
	//	Open the file

	CFileBuffer theFile;
	if (!theFile.OpenReadOnly(sFilespec))
		{
		*retsError = strPattern(ERR_CANT_OPEN_FILE, sFilespec);
		return false;
		}

	//	If unknown format, see if we can detect the format

	if (iFormat == EFormat::Unknown)
		{
		if (!DetectFileFormat(sFilespec, theFile, &iFormat, retsError))
			return false;

		//	If format unknown, then error.

		if (iFormat == EFormat::Unknown)
			{
			*retsError = strPattern(ERR_UNKNOWN_FORMAT, sFilespec);
			return false;
			}
		}

	//	Parse it

	if (!Deserialize(iFormat, theFile, retdDatum))
		{
		*retsError = strPattern(ERR_DESERIALIZE_ERROR, sFilespec);
		return false;
		}

	//	Done

	return true;
	}

bool CDatum::CreateFromStringValue (const CString &sValue, CDatum *retdDatum)

//	CreateFromStringValue
//
//	Creates either a string or a number depending on the value.

	{
	CDatum dDatum;

	switch (GetStringValueType(sValue))
		{
		case typeNil:
			break;

		case typeInteger32:
			dDatum = CDatum(strToInt(sValue, 0));
			break;

		case typeIntegerIP:
			{
			CIPInteger Value;
			Value.InitFromString(sValue);
			CDatum::CreateIPIntegerFromHandoff(Value, &dDatum);
			break;
			}

		case typeDouble:
			dDatum = CDatum(strToDouble(sValue));
			break;

		case typeString:
			dDatum = CDatum(sValue);
			break;

		default:
			return false;
		}

	if (retdDatum)
		*retdDatum = dDatum;

	return true;
	}

bool CDatum::CreateIPInteger (const CIPInteger &Value, CDatum *retdDatum)

//	CreateIPInteger
//
//	Creates an IPInteger datum

	{
	CComplexInteger *pIPInt = new CComplexInteger(Value);
	*retdDatum = CDatum(pIPInt);
	return true;
	}

bool CDatum::CreateIPIntegerFromHandoff (CIPInteger &Value, CDatum *retdDatum)

//	CreateIPInteger
//
//	Creates an IPInteger by taking a handoff

	{
	CComplexInteger *pIPInt = new CComplexInteger;
	pIPInt->TakeHandoff(Value);

	*retdDatum = CDatum(pIPInt);
	return true;
	}

CDatum CDatum::CreateLibraryFunction (const SAEONLibraryFunctionCreate& Create)

//	CreateLibraryFunction
//
//	Creates a library function.

	{
	return CAEONLibraryFunction::Create(Create);
	}

CDatum CDatum::CreateNaN ()

//	CreateNaN
//
//	Creates the special value NaN

	{
	CDatum dResult;
	dResult.m_dwData = CONST_NAN;
	return dResult;
	}

CDatum CDatum::CreateObject (CDatum dType, CDatum dValue)

//	CreateObject
//
//	Creates an object of the given type.

	{
	CDatum dObj = CDatum(new CAEONObject(dType));

	if (dValue.GetBasicType() == CDatum::typeStruct)
		{
		for (int i = 0; i < dValue.GetCount(); i++)
			{
			dObj.SetElement(dValue.GetKey(i), dValue.GetElement(i));
			}
		}

	return dObj;
	}

bool CDatum::CreateStringFromHandoff (CString &sString, CDatum *retDatum)

//	CreateStringFromHandoff
//
//	Creates a string by taking a handoff from a string

	{
	if (sString.IsLiteral())
		{
		if (sString.IsEmpty())
			retDatum->m_dwData = 0;
		else
			retDatum->m_dwData = (DWORD_PTR)(LPSTR)sString;
		}
	else
		{
		//	Take ownership of the data

		LPSTR pString = sString.Handoff();

		//	Track it with our allocator (but only if not NULL).
		//	(If pString is NULL then this is represented as Nil).

		if (pString)
			CAEONStore::Alloc(pString);

		//	Store the pointer

		retDatum->m_dwData = (DWORD_PTR)pString;
		}

	//	Done

	return true;
	}

bool CDatum::CreateStringFromHandoff (CStringBuffer &String, CDatum *retDatum)

//	CreateStringFromHandoff
//
//	Creates a string by taking a handoff from a string buffer

	{
	//	Take ownership of the data

	LPSTR pString = String.Handoff();

	//	Track it with our allocator (but only if not NULL).
	//	(If pString is NULL then this is represented as Nil).

	if (pString)
		CAEONStore::Alloc(pString);

	//	Store the pointer

	retDatum->m_dwData = (DWORD_PTR)pString;

	//	Done

	return true;
	}

CDatum CDatum::CreateTable (CDatum dType, CDatum dValue)

//	CreateTable
//
//	Creates a table

	{
	const IDatatype &Schema = dType;
	if (Schema.GetClass() != IDatatype::ECategory::Schema)
		return CDatum();

	CDatum dTable = CDatum(new CAEONTable(dType));
	IAEONTable *pTable = dTable.GetTableInterface();
	if (!pTable)
		throw CException(errFail);

	pTable->AppendTable(dValue);

	return dTable;
	}

bool CDatum::CreateTableFromDesc (CAEONTypeSystem &TypeSystem, CDatum dDesc, CDatum &retdDatum)

//	CreateTableFromDesc
//
//	Creates a table from various possible descriptors. We support the following:
//
//	schema: If dDesc is a schema, we create an empty table with the given
//		schema.
//
//	In case of error, we return nil

	{
	switch (dDesc.GetBasicType())
		{
		case CDatum::typeNil:
			return CAEONTable::CreateTableFromNil(TypeSystem, retdDatum);

		case CDatum::typeArray:
			return CAEONTable::CreateTableFromArray(TypeSystem, dDesc, retdDatum);

		case CDatum::typeDatatype:
			return CAEONTable::CreateTableFromDatatype(TypeSystem, dDesc, retdDatum);

		case CDatum::typeStruct:
			return CAEONTable::CreateTableFromStruct(TypeSystem, dDesc, retdDatum);

		case CDatum::typeTable:
			retdDatum = dDesc.Clone();
			return true;

		default:
			retdDatum = ERR_INVALID_TABLE_DESC;
			return false;
		}
	}

CDatum CDatum::CreateTextLines (CDatum dValue)

//	CreateTextLines
//
//	Creates a textLines datum.

	{
	return CAEONLines::Create(dValue);
	}

int CDatum::DefaultCompare (void *pCtx, const CDatum &dKey1, const CDatum &dKey2)

//	DefaultCompare
//
//	Default comparison routine used for sorting. Returns:
//
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2
//
//	NOTES:
//
//	Nil == ""
//	Nil == {}
//	Nil == ()
//	"abc" != "ABC"

	{
	int i;

	//	If both are the same datatype, then compare

	CDatum::Types iType1 = dKey1.GetBasicType();
	CDatum::Types iType2 = dKey2.GetBasicType();

	//	If both types are equal, then compare

	if (iType1 == iType2)
		{
		switch (iType1)
			{
			case CDatum::typeNil:
			case CDatum::typeTrue:
			case CDatum::typeNaN:
				return 0;

			case CDatum::typeInteger32:
				if ((int)dKey1 > (int)dKey2)
					return 1;
				else if ((int)dKey1 < (int)dKey2)
					return -1;
				else
					return 0;

			case CDatum::typeInteger64:
				if ((DWORDLONG)dKey1 > (DWORDLONG)dKey2)
					return 1;
				else if ((DWORDLONG)dKey1 < (DWORDLONG)dKey2)
					return -1;
				else
					return 0;

			case CDatum::typeDouble:
				if ((double)dKey1 > (double)dKey2)
					return 1;
				else if ((double)dKey1 < (double)dKey2)
					return -1;
				else
					return 0;

			case CDatum::typeIntegerIP:
				return KeyCompare((const CIPInteger &)dKey1, (const CIPInteger &)dKey2);

			case CDatum::typeString:
				return KeyCompare((const CString &)dKey1, (const CString &)dKey2);

			case CDatum::typeDateTime:
				return ((const CDateTime &)dKey1).Compare((const CDateTime &)dKey2);

			case CDatum::typeTimeSpan:
				return ((const CTimeSpan &)dKey1).Compare((const CTimeSpan &)dKey2);

			case CDatum::typeEnum:
				{
				DWORD dwType1 = dKey1.GetNumberIndex();
				DWORD dwType2 = dKey2.GetNumberIndex();
				if (dwType1 > dwType2)
					return 1;
				else if (dwType1 < dwType2)
					return -1;
				else
					{
					int iValue1 = (int)dKey1;
					int iValue2 = (int)dKey2;
					if (iValue1 > iValue2)
						return 1;
					else if (iValue1 < iValue2)
						return -1;
					else
						return 0;
					}
				}

			case CDatum::typeArray:
				if (dKey1.GetCount() > dKey2.GetCount())
					return 1;
				else if (dKey1.GetCount() < dKey2.GetCount())
					return -1;
				else
					{
					for (i = 0; i < dKey1.GetCount(); i++)
						{
						CDatum dItem1 = dKey1.GetElement(i);
						CDatum dItem2 = dKey2.GetElement(i);
						int iItemCompare = CDatum::DefaultCompare(pCtx, dItem1, dItem2);
						if (iItemCompare != 0)
							return iItemCompare;
						}

					return 0;
					}

			case CDatum::typeStruct:
				if (dKey1.GetCount() > dKey2.GetCount())
					return 1;
				else if (dKey1.GetCount() < dKey2.GetCount())
					return -1;
				else
					{
					for (i = 0; i < dKey1.GetCount(); i++)
						{
						CString sItemKey1 = dKey1.GetKey(i);
						CString sItemKey2 = dKey2.GetKey(i);
						int iKeyCompare = KeyCompare(sItemKey1, sItemKey2);
						if (iKeyCompare != 0)
							return iKeyCompare;

						CDatum dItem1 = dKey1.GetElement(i);
						CDatum dItem2 = dKey2.GetElement(i);
						int iItemCompare = CDatum::DefaultCompare(pCtx, dItem1, dItem2);
						if (iItemCompare != 0)
							return iItemCompare;
						}

					return 0;
					}

			case CDatum::typeDatatype:
				return KeyCompare(((const IDatatype &)dKey1).GetFullyQualifiedName(), ((const IDatatype &)dKey2).GetFullyQualifiedName());

			//	LATER: Not yet supported

			default:
				return 0;
			}
		}

	//	If one of the types is nil, then compare

	else if (iType1 == CDatum::typeNil || iType2 == CDatum::typeNil)
		{
		CDatum dNonNil;
		int iResult;
		if (iType2 == CDatum::typeNil)
			{
			dNonNil = dKey1;
			Swap(iType1, iType2);
			iResult = 1;
			}
		else
			{
			dNonNil = dKey2;
			iResult = -1;
			}

		switch (iType2)
			{
			case CDatum::typeString:
				if (((const CString &)dNonNil).IsEmpty())
					return 0;
				else
					return iResult;

			case CDatum::typeArray:
			case CDatum::typeStruct:
				if (dNonNil.GetCount() == 0)
					return 0;
				else
					return iResult;

			default:
				//	nil is always less
				return iResult;
			}
		}

	//	If one of the types is a number, then compare as numbers

	else if (dKey1.IsNumber() || dKey2.IsNumber())
		{
		CNumberValue Number1(dKey1);
		CNumberValue Number2(dKey2);

		if (Number1.IsValidNumber() && Number2.IsValidNumber())
			return Number1.Compare(Number2);
		else if (Number1.IsValidNumber())
			return 1;
		else if (Number2.IsValidNumber())
			return -1;
		else
			return 0;
		}

	//	Otherwise, cannot compare

	else
		return 0;
	}

void CDatum::DeleteElement (int iIndex)

//	DeleteElement
//
//	Deletes the given element.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			raw_GetComplex()->DeleteElement(iIndex);
			break;

		default:
			break;
		}
	}

bool CDatum::Deserialize (EFormat iFormat, IByteStream &Stream, IAEONParseExtension *pExtension, CDatum *retDatum)

//	Deserialize
//
//	Deserialize from the given format

	{
	try
		{
		switch (iFormat)
			{
			case EFormat::AEONScript:
			case EFormat::AEONLocal:
				return DeserializeAEONScript(Stream, pExtension, retDatum);

			case EFormat::Binary:
				return CDatum::CreateBinary(Stream, Stream.GetStreamLength() - Stream.GetPos(), retDatum);

			case EFormat::JSON:
				return DeserializeJSON(Stream, retDatum);

			case EFormat::TextUTF8:
				return DeserializeTextUTF8(Stream, retDatum);

			default:
				ASSERT(false);
				return false;
			}
		}
	catch (...)
		{
		return false;
		}
	}

bool CDatum::DeserializeTextUTF8 (IByteStream &Stream, CDatum *retDatum)

//	DeserializeTextUTF8
//
//	Loads straight UTF-8 into a single string value.

	{
	CStringBuffer Buffer;

	//	See if we have an encoding mark

	BYTE BOM[3];
	Stream.Read(BOM, sizeof(BOM));
	if (BOM[0] == 0xef && BOM[1] == 0xbb && BOM[2] == 0xbf)
		;	//	UTF-8

	//	Otherwise, not an encoding mark, so write it to the buffer

	else
		Buffer.Write(BOM, sizeof(BOM));

	//	Write the rest

	Buffer.Write(Stream, Stream.GetStreamLength());
	return CreateStringFromHandoff(Buffer, retDatum);
	}

bool CDatum::DetectFileFormat (const CString &sFilespec, IMemoryBlock &Data, EFormat *retiFormat, CString *retsError)

//	DetectFileFormat
//
//	Try to figure out the file format. If we can't figure it out, we return TRUE
//	but retiFormat is formatUnknown. We only return FALSE if we have a read
//	error of some sort.

	{
	//	LATER.
	//	See: http://stackoverflow.com/questions/1031645/how-to-detect-utf-8-in-plain-c

	if (retiFormat)
		*retiFormat = EFormat::Unknown;

	return true;
	}

bool CDatum::EnumElements (std::function<bool(CDatum)> fn)

//	EnumElements
//
//	Enumerates over all elements, recursively. Returns FALSE if one of the calls
//	returned FALSE.

	{
	if (IsNil())
		return true;
	else if (IsContainer())
		{
		for (int i = 0; i < GetCount(); i++)
			{
			if (!GetElement(i).EnumElements(fn))
				return false;
			}

		return true;
		}
	else
		return fn(*this);
	}

bool CDatum::Find (CDatum dValue, int *retiIndex) const

//	Find
//
//	Looks for an element in an array and returns the index.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->Find(dValue, retiIndex);

		default:
			return false;
		}
	}

bool CDatum::FindElement (const CString &sKey, CDatum *retpValue)

//	FindElement
//
//	Looks for the element by key.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->FindElement(sKey, retpValue);

		default:
			return false;
		}
	}

bool CDatum::FindExternalType (const CString &sTypename, IComplexFactory **retpFactory)

//	FindExternalType
//
//	Finds a factory for the type

	{
	return (g_pFactories == NULL ? false : g_pFactories->FindFactory(sTypename, retpFactory));
	}

const CDatum::SAnnotation& CDatum::GetAnnotation () const

//	GetAnnotation
//
//	Get annotation

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetAnnotation();

		default:
			return m_NullAnnotation;
		}
	}

int CDatum::GetArrayCount (void) const

//	GetArrayCount
//
//	Returns the number of items in the datum.
//	
//	NOTE: Unlike GetCount, this only returns 1 for everything except arrays.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			return (m_dwData == 0 ? 0 : 1);

		case AEON_TYPE_NUMBER:
			return 1;

		case AEON_TYPE_COMPLEX:
			if (GetBasicType() == typeArray)
				return raw_GetComplex()->GetCount();
			else
				return 1;

		default:
			ASSERT(false);
			return 0;
		}
	}

CDatum CDatum::GetArrayElement (int iIndex) const

//	GetArrayElement
//
//	Gets the appropriate element

	{
	ASSERT(iIndex >= 0);

	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			{
			if (GetBasicType() == typeArray)
				return raw_GetComplex()->GetElement(iIndex);
			else
				return (iIndex == 0 ? *this : CDatum());
			}

		default:
			return (iIndex == 0 ? *this : CDatum());
		}
	}

CDatum::Types CDatum::GetBasicType (void) const

//	GetBasicType
//
//	Returns the type

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			return (m_dwData == 0 ? typeNil : typeString);

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData)
						{
						case CONST_NAN:
							return typeNaN;

						case CONST_TRUE:
							return typeTrue;

						default:
							ASSERT(false);
							return typeUnknown;
						}
					}

				case AEON_NUMBER_INTEGER:
					return typeInteger32;

				case AEON_NUMBER_DOUBLE:
					return typeDouble;

				case AEON_NUMBER_ENUM:
					return typeEnum;

				default:
					ASSERT(false);
					return typeUnknown;
				}

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetBasicType();

		default:
			ASSERT(false);
			return typeUnknown;
		}
	}

int CDatum::GetBinarySize (void) const

//	GetBinarySize
//
//	For binary blob objects, this returns the total size in bytes.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetBinarySize();

		default:
			{
			const CString &sData = *this;
			return sData.GetLength();
			}
		}
	}

CDatum CDatum::GetDatatype () const

//	GetDatatype
//
//	Returns a datatype.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			return (m_dwData == 0 ? CAEONTypeSystem::GetCoreType(IDatatype::NULL_T) : CAEONTypeSystem::GetCoreType(IDatatype::STRING));

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData)
						{
						case CONST_NAN:
							return CAEONTypeSystem::GetCoreType(IDatatype::FLOAT_64);
							
						case CONST_TRUE:
							return CAEONTypeSystem::GetCoreType(IDatatype::BOOL);

						default:
							throw CException(errFail);
						}
					}

				case AEON_NUMBER_INTEGER:
					return CAEONTypeSystem::GetCoreType(IDatatype::INT_32);

				case AEON_NUMBER_DOUBLE:
					return CAEONTypeSystem::GetCoreType(IDatatype::FLOAT_64);

				case AEON_NUMBER_ENUM:
					return CAEONTypes::Get(GetNumberIndex());

				default:
					throw CException(errFail);
				}

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetDatatype();

		default:
			throw CException(errFail);
		}
	}

CRGBA32Image *CDatum::GetImageInterface ()

//	GetImageInterface
//
//	Returns an image interface (or NULL).

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetImageInterface();

		default:
			return NULL;
		}
	}

IAEONCanvas *CDatum::GetCanvasInterface ()

//	GetCanvasInterface
//
//	Returns a canvas interface (or NULL).

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetCanvasInterface();

		default:
			return NULL;
		}
	}

const CAEONQuery* CDatum::GetQueryInterface () const

//	GetQueryInterface
//
//	Returns a query interface (or NULL).

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetQueryInterface();

		default:
			return NULL;
		}
	}

IAEONTable *CDatum::GetTableInterface ()

//	GetTableInterface
//
//	Returns a table interface (or NULL).

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetTableInterface();

		default:
			return NULL;
		}
	}

IAEONTextLines* CDatum::GetTextLinesInterface ()

//	GetTextLinesInterface
//
//	Returns a text lines interface (or NULL).

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetTextLinesInterface();

		default:
			return NULL;
		}
	}

void CDatum::GrowToFit (int iCount)

//	GrowToFit
//
//	Grow array to be able to fit the given number of additional elements

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			raw_GetComplex()->GrowToFit(iCount);
			break;
		}
	}

bool CDatum::IsMemoryBlock (void) const

//	IsMemoryBlock
//
//	Returns TRUE if we can represent this as a contiguous block of memory.
//	If we cannot, then we need to use functions like WriteBinaryToStream.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->IsMemoryBlock();

		default:
			{
			const CString &sData = *this;
			return (sData.GetLength() > 0);
			}
		}
	}

void CDatum::WriteBinaryToStream (IByteStream &Stream, int iPos, int iLength, IProgressEvents *pProgress) const

//	WriteBinaryToStream
//
//	Writes a binary blob object to a stream.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			raw_GetComplex()->WriteBinaryToStream(Stream, iPos, iLength, pProgress);
			break;

		default:
			{
			const CString &sData = *this;

			if (iPos >= sData.GetLength())
				return;

			if (iLength == -1)
				iLength = Max(0, sData.GetLength() - iPos);
			else
				iLength = Min(iLength, sData.GetLength() - iPos);

			if (pProgress)
				pProgress->OnProgressStart();

			Stream.Write(sData.GetPointer() + iPos, iLength);

			if (pProgress)
				pProgress->OnProgressDone();
			break;
			}
		}
	}

CDatum::ECallType CDatum::GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const

//	GetCallInfo
//
//	Returns info about invocation

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetCallInfo(retdCodeBank, retpIP);

		default:
			return ECallType::None;
		}
	}

IComplexDatum *CDatum::GetComplex (void) const

//	GetComplex
//
//	Returns a pointer to the complex type

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex();

		default:
			return NULL;
		}
	}

int CDatum::GetCount (void) const

//	GetCount
//
//	Returns the number of items in the datum

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			return (m_dwData == 0 ? 0 : 1);

		case AEON_TYPE_NUMBER:
			return 1;

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetCount();

		default:
			ASSERT(false);
			return 0;
		}
	}

CDatum CDatum::GetElement (IInvokeCtx *pCtx, int iIndex) const

//	GetElement
//
//	Gets the appropriate element

	{
	ASSERT(iIndex >= 0);

	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			{
			if (iIndex == 0)
				{
				IComplexDatum *pComplex = raw_GetComplex();
				if (pComplex->IsArray())
					return raw_GetComplex()->GetElement(pCtx, 0);
				else
					return *this;
				}
			else
				return raw_GetComplex()->GetElement(pCtx, iIndex);
			}

		default:
			return (iIndex == 0 ? *this : CDatum());
		}
	}

CDatum CDatum::GetElement (int iIndex) const

//	GetElement
//
//	Gets the appropriate element

	{
	ASSERT(iIndex >= 0);

	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			{
			if (iIndex == 0)
				{
				IComplexDatum *pComplex = raw_GetComplex();
				if (pComplex->IsArray())
					return raw_GetComplex()->GetElement(0);
				else
					return *this;
				}
			else
				return raw_GetComplex()->GetElement(iIndex);
			}

		default:
			return (iIndex == 0 ? *this : CDatum());
		}
	}

CDatum CDatum::GetElement (IInvokeCtx *pCtx, const CString &sKey) const

//	GetElement
//
//	Gets the appropriate element

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetElement(pCtx, sKey);

		default:
			return CDatum();
		}
	}

CDatum CDatum::GetElement (const CString &sKey) const

//	GetElement
//
//	Gets the appropriate element

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			if (m_dwData == 0)
				return CDatum();
			else
				return CAEONStringImpl::GetElement(raw_GetString(), sKey);

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetElement(sKey);

		default:
			return CDatum();
		}
	}

CDatum CDatum::GetElementAt (int iIndex) const

//	GetElementAt
//
//	Gets the appropriate element

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			if (m_dwData == 0)
				return CDatum();
			else
				{
				auto& sValue = raw_GetString();
				if (iIndex >= 0 && iIndex < sValue.GetLength())
					return CString(sValue.GetParsePointer() + iIndex, 1);
				else
					return CDatum();
				}

		case AEON_TYPE_COMPLEX:
			{
			if (iIndex == 0)
				{
				IComplexDatum *pComplex = raw_GetComplex();
				if (pComplex->IsArray())
					return raw_GetComplex()->GetElement(0);
				else
					return *this;
				}
			else
				return raw_GetComplex()->GetElement(iIndex);
			}

		default:
			return (iIndex == 0 ? *this : CDatum());
		}
	}

CDatum CDatum::GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const

//	GetElement
//
//	Gets the appropriate element

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			if (m_dwData == 0)
				return CDatum();
			else
				return CAEONStringImpl::GetElementAt(raw_GetString(), TypeSystem, dIndex);

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetElementAt(TypeSystem, dIndex);

		default:
			return CDatum();
		}
	}

int CDatum::GetElementAtCount () const

//	GetElementAtCount
//
//	Returns the number of elements as determined by GetElementAt. This is 
//	slightly different from GetCount and GetArrayCount.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			if (m_dwData == 0)
				return 0;
			else
				return raw_GetString().GetLength();

		case AEON_TYPE_NUMBER:
			return 1;

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetCount();

		default:
			ASSERT(false);
			return 0;
		}
	}

CString CDatum::GetKey (int iIndex) const

//	GetKey
//
//	Returns the key at the given index

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetKey(iIndex);

		default:
			return NULL_STR;
		}
	}

CDatum CDatum::GetMethod (const CString &sMethod) const

//	GetMethod
//
//	Returns a function definition (or nil).

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			if (m_dwData == 0)
				return CAEONNilImpl::GetMethod(sMethod);
			else
				return CAEONStringImpl::GetMethod(sMethod);

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetMethod(sMethod);

		default:
			return CDatum();
		}
	}

void* CDatum::GetMethodThis ()

//	GetMethodThis
//
//	Returns the this pointer for the object.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			{
			if (m_dwData == 0)
				return NULL;
			else
				return (CString *)&m_dwData;
			}

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex();

		default:
			return NULL;
		}
	}

CDatum::Types CDatum::GetNumberType (int *retiValue, CDatum *retdConverted) const

//	GetNumberType
//
//	Returns the most appropriate number type for the datum and optimistically
//	returns the value if the type is integer.
//
//	The number type is the best number type that the datum can be cast
//	to without loss of data, following this order (from most preferable to
//	least):
//
//	typeInteger32
//	typeDouble
//	typeIntegerIP
//
//	If the original datum is a string then we try to parse it and return the
//	best number type. retdConverted will be a datum representing the parsed
//	number.
//
//	If the value cannot be converted to a number we return typeUnknown.

	{
	//	Pre-init

	if (retdConverted)
		*retdConverted = *this;

	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			{
			if (m_dwData == 0)
				{
				if (retiValue)
					*retiValue = 0;
				return typeInteger32;
				}
			else
				{
				CDatum dNumberValue;
				if (!CDatum::CreateFromStringValue(*this, &dNumberValue)
						|| !dNumberValue.IsNumber())
					return typeNaN;

				if (retdConverted)
					*retdConverted = dNumberValue;

				return dNumberValue.GetNumberType(retiValue);
				}
			}

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData)
						{
						case CONST_NAN:
							return typeNaN;

						case CONST_TRUE:
							if (retiValue)
								*retiValue = 1;
							return typeInteger32;

						default:
							ASSERT(false);
							return typeNaN;
						}
					}

				case AEON_NUMBER_INTEGER:
				case AEON_NUMBER_ENUM:
					if (retiValue)
						*retiValue = (int)HIDWORD(m_dwData);
					return typeInteger32;

				case AEON_NUMBER_DOUBLE:
					if (retiValue)
						*retiValue = (int)CAEONStore::GetDouble(GetNumberIndex());
					return typeDouble;

				default:
					ASSERT(false);
					return typeNaN;
				}

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetNumberType(retiValue);

		default:
			ASSERT(false);
			return typeNaN;
		}
	}

CDatum::Types CDatum::GetStringValueType (const CString &sValue)

//	GetStringValueType
//
//	Returns one of the following:
//
//	typeNil if sValue is empty
//	typeInteger32 if sValue is a 32-bit integer
//	typeIntegerIP if sValue is an integer (which may or may not be > 64-bits)
//	typeDouble if sValue is a double
//	typeString otherwise.

	{
	enum EStates
		{
		stateStart,
		stateHex0,
		stateHex,
		stateInteger,
		stateDoubleFrac,
		stateDoubleExp,
		stateDoubleExpSign,
		};

	char *pPos = sValue.GetParsePointer();
	char *pPosEnd = pPos + sValue.GetLength();
	int iState = stateStart;

	while (pPos < pPosEnd)
		{
		switch (iState)
			{
			case stateStart:
				{
				//	If 0 then we might be a hex number

				if (*pPos == '0')
					iState = stateHex0;

				//	If -, +, or a digit, we might be an integer

				else if (*pPos == '-' || *pPos == '+' || strIsDigit(pPos))
					iState = stateInteger;

				//	If . then we might be a double

				else if (*pPos == '.')
					iState = stateDoubleFrac;

				//	Otherwise, we are a string

				else
					return typeString;

				break;
				}

			case stateHex0:
				{
				if (*pPos == 'x' || *pPos == 'X')
					iState = stateHex;
				else if (strIsDigit(pPos))
					iState = stateInteger;
				else if (*pPos == '.')
					iState = stateDoubleFrac;
				else if (*pPos == 'e' || *pPos == 'E')
					iState = stateDoubleExp;
				else
					return typeString;

				break;
				}

			case stateHex:
				{
				if (strIsDigit(pPos)
						|| (*pPos >= 'A' && *pPos <= 'F')
						|| (*pPos >= 'a' && *pPos <= 'f'))
					NULL;
				else
					return typeString;

				break;
				}

			case stateInteger:
				{
				if (strIsDigit(pPos))
					NULL;
				else if (*pPos == '.')
					iState = stateDoubleFrac;
				else if (*pPos == 'e' || *pPos == 'E')
					iState = stateDoubleExp;
				else
					return typeString;

				break;
				}

			case stateDoubleFrac:
				{
				if (strIsDigit(pPos))
					NULL;
				else if (*pPos == 'e' || *pPos == 'E')
					iState = stateDoubleExp;
				else
					return typeString;

				break;
				}

			case stateDoubleExp:
				{
				if (*pPos == '+' || *pPos == '-' || strIsDigit(pPos))
					iState = stateDoubleExpSign;
				else
					return typeString;

				break;
				}

			case stateDoubleExpSign:
				{
				if (strIsDigit(pPos))
					NULL;
				else
					return typeString;

				break;
				}
			}

		pPos++;
		}

	switch (iState)
		{
		case stateStart:
			return typeNil;

		case stateHex:
			//	LATER:
			return typeString;

		case stateHex0:
			return typeInteger32;

		case stateInteger:
			if (strOverflowsInteger32(sValue))
				return typeIntegerIP;
			else
				return typeInteger32;

		case stateDoubleFrac:
		case stateDoubleExpSign:
			return typeDouble;

		default:
			return typeString;
		}
	}

const CString &CDatum::GetTypename (void) const

//	GetTypename
//
//	Gets the typename of the object

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			if (m_dwData == 0)
				return TYPENAME_NIL;
			else
				return TYPENAME_STRING;

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData)
						{
						case CONST_NAN:
							return TYPENAME_DOUBLE;
							
						case CONST_TRUE:
							return TYPENAME_TRUE;

						default:
							ASSERT(false);
							return NULL_STR;
						}
					}

				case AEON_NUMBER_INTEGER:
					return TYPENAME_INT32;

				case AEON_NUMBER_DOUBLE:
					return TYPENAME_DOUBLE;

				case AEON_NUMBER_ENUM:
					return TYPENAME_ENUM;

				default:
					ASSERT(false);
					return NULL_STR;
				}

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->GetTypename();

		default:
			ASSERT(false);
			return NULL_STR;
		}
	}

CDatum::InvokeResult CDatum::Invoke (IInvokeCtx *pCtx, CDatum dLocalEnv, DWORD dwExecutionRights, CDatum *retdResult)

//	Invoke
//
//	Invokes the function

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->Invoke(pCtx, dLocalEnv, dwExecutionRights, retdResult);

		default:
			*retdResult = CDatum();
			return InvokeResult::error;
		}
	}

CDatum::InvokeResult CDatum::InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, CDatum *retdResult)

//	InvokeContinues
//
//	Invokes the function

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->InvokeContinues(pCtx, dContext, dResult, retdResult);

		default:
			*retdResult = CDatum();
			return InvokeResult::error;
		}
	}

bool CDatum::InvokeMethodImpl (const CString &sMethod, IInvokeCtx &Ctx, CDatum dLocalEnv, CDatum &retdResult)

//	InvokeMethodImpl
//
//	Invokes a method.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->InvokeMethodImpl(*this, sMethod, Ctx, dLocalEnv, retdResult);

		default:
			retdResult = ERR_NO_METHODS;
			return false;
		}
	}

bool CDatum::IsArray () const

//	IsArray
//
//	Returns TRUE if we are an array type

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->IsArray();

		default:
			return false;
		}
	}

bool CDatum::IsAtom () const

//	IsAtom
//
//	Returns TRUE if this is an atom (not a collection and not a function).

	{
	switch (GetBasicType())
		{
		case typeNil:
		case typeTrue:
		case typeNaN:
		case typeEnum:
		case typeError:
		case typeInteger32:
		case typeInteger64:
		case typeIntegerIP:
		case typeDouble:
		case typeString:
		case typeDateTime:
		case typeTimeSpan:
			return true;

		default:
			return false;
		}
	}

bool CDatum::IsContainer () const

//	IsContainer
//
//	Is a container of sub-elements.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->IsContainer();

		default:
			return false;
		}
	}

bool CDatum::IsEqual (CDatum dValue) const

//	IsEqual
//
//	Returns TRUE if the values are equal

	{
	switch (GetBasicType())
		{
		case typeNil:
			return dValue.IsNil();

		case typeTrue:
			return !dValue.IsNil();

		case typeInteger32:
		case typeInteger64:
		case typeIntegerIP:
		case typeDouble:
			return (dValue.IsNumber() && CNumberValue(*this).Compare(dValue) == 0);

		case typeNaN:
			return (dValue.GetBasicType() == typeNaN);

		case typeString:
			return (dValue.GetBasicType() == typeString && strEquals(*this, dValue));

		case typeDateTime:
			return (dValue.GetBasicType() == typeDateTime && ((const CDateTime &)*this == (const CDateTime &)dValue));

		case typeTimeSpan:
			return (dValue.GetBasicType() == typeTimeSpan && ((const CTimeSpan &)*this == (const CTimeSpan &)dValue));

		case typeEnum:
			{
			if (dValue.GetBasicType() != typeEnum)
				return false;

			DWORD dwType1 = GetNumberIndex();
			DWORD dwType2 = dValue.GetNumberIndex();
			if (dwType1 != dwType2)
				return false;

			return ((int)*this == (int)dValue);
			}

		case typeDatatype:
			return (dValue.GetBasicType() == typeDatatype && strEquals(((const IDatatype &)*this).GetFullyQualifiedName(), ((const IDatatype &)dValue).GetFullyQualifiedName()));

		case CDatum::typeVector2D:
			return (dValue.GetBasicType() == typeVector2D && (const CVector2D&)*this == (const CVector2D&)dValue);

		//	LATER
		case typeArray:
		case typeBinary:
		case typeImage32:
		case typeStruct:
		case typeSymbol:
			return false;

		default:
			ASSERT(false);
			return false;
		}
	}

bool CDatum::IsError (void) const

//	IsError
//
//	Returns TRUE if this is an error value

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->IsError();

		default:
			return false;
		}
	}

bool CDatum::IsNil (void) const

//	IsNil
//
//	Returns TRUE if this is a nil value

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			return (m_dwData == 0 ? true : false);

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->IsNil();

		default:
			return false;
		}
	}

bool CDatum::IsNumber (void) const

//	IsNumber
//
//	Returns TRUE if this is a number

	{
	switch (GetBasicType())
		{
		case typeInteger32:
		case typeInteger64:
		case typeIntegerIP:
		case typeDouble:
			return true;

		default:
			return false;
		}
	}

bool CDatum::IsNumberInt32 (int *retiValue) const

//	IsNumberInt32
//
//	Returns TRUE if this value can be safely (losslessly) cast to a 32-bit 
//	signed integer.
//
//	NOTE: Unlike some other number functions, we don't convert strings to 
//	numbers because callers rely on this to tell the difference between (e.g.)
//	numeric indices vs. string indices.

	{
	switch (GetBasicType())
		{
		case typeInteger32:
			if (retiValue) *retiValue = (int)*this;
			return true;

		case typeInteger64:
			{
			DWORDLONG dwValue = (DWORDLONG)*this;
			if (dwValue <= (DWORDLONG)INT_MAX)
				{
				if (retiValue) *retiValue = (int)dwValue;
				return true;
				}
			else
				return false;
			}

		case typeIntegerIP:
			{
			const auto &Value = (const CIPInteger &)*this;
			if (Value.FitsAsInteger32Signed())
				{
				if (retiValue) *retiValue = Value.AsInteger32Signed();
				return true;
				}
			else
				return false;
			}

		case typeDouble:
			{
			double rValue = (double)*this;
			if ((double)(int)rValue == rValue
					&& rValue >= (double)INT_MIN
					&& rValue <= (double)INT_MAX)
				{
				if (retiValue) *retiValue = (int)rValue;
				return true;
				}
			else
				return false;
			}

		default:
			return false;
		}
	}

void CDatum::Mark (void)

//	Mark
//
//	Marks the datum so that we know that we cannot free it

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			if (m_dwData != 0)
				CAEONStore::MarkString((LPSTR)m_dwData);
			break;

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_DOUBLE:
					CAEONStore::MarkDouble(GetNumberIndex());
					break;
				}
			break;

		case AEON_TYPE_COMPLEX:
			CAEONStore::MarkComplex(raw_GetComplex());
			break;
		}
	}

void CDatum::MarkAndSweep (void)

//	MarkAndSweep
//
//	After we've marked all the extended objects that we are using,
//	we sweep away everything not being used.
//	(Sweeping also clears the marks)

	{
	CAEONTypeSystem::MarkCoreTypes();
	CAEONTypes::MarkAndSweep();
	CAEONStore::Sweep();
	}

CDatum CDatum::MergeKeysNoCase () const

//	MergeKeysNoCase
//
//	Creates a new struct with keys merged so that they are case-insensitive.
//	For now, we throw away duplicate keys, but in the future we can add flags
//	to combine keys.

	{
	CDatum dResult(CDatum::typeStruct);

	for (int i = 0; i < GetCount(); i++)
		{
		dResult.SetElement(strToLower(GetKey(i)), GetElement(i));
		}

	return dResult;
	}

double CDatum::raw_GetDouble (void) const

//	raw_GetDouble
//
//	Returns a double

	{
	return CAEONStore::GetDouble(GetNumberIndex());
	}

bool CDatum::RegisterExternalType (const CString &sTypename, IComplexFactory *pFactory)

//	RegisterExternalType
//
//	Registers an external type

	{
	if (g_pFactories == NULL)
		g_pFactories = new CAEONFactoryList;

	g_pFactories->RegisterFactory(sTypename, pFactory);

	return true;
	}

void CDatum::RegisterMarkProc (MARKPROC fnProc)

//	RegisterMarkProc
//
//	Register a procedure that will mark data in use

	{
	CAEONStore::RegisterMarkProc(fnProc);
	}

void CDatum::ResolveDatatypes (const CAEONTypeSystem &TypeSystem)

//	ResolveDatatypes
//
//	If we have references to datatypes, we resolve them against the type system 
//	so that we match. This is helpful when we serialize/deserialize values.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			raw_GetComplex()->ResolveDatatypes(TypeSystem);
			break;

		default:
			break;
		}
	}

void CDatum::Serialize (EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serializes the datum

	{
	switch (iFormat)
		{
		case EFormat::AEONScript:
		case EFormat::AEONLocal:
			SerializeAEONScript(iFormat, Stream);
			break;

		case EFormat::JSON:
			SerializeJSON(Stream);
			break;

		default:
			ASSERT(false);
		}
	}

CString CDatum::SerializeToString (EFormat iFormat) const

//	SerializeToString
//
//	Returns the serialization of the datum as a string

	{
	CStringBuffer Output;
	Serialize(iFormat, Output);
	return CString::CreateFromHandoff(Output);
	}

void CDatum::SetElement (IInvokeCtx *pCtx, const CString &sKey, CDatum dValue)

//	SetElement
//
//	Sets the value to the datum. Note that this modifies the datum in place.
//	Do not use this unless you are sure about who else might have a pointer
//	to the datum.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			raw_GetComplex()->SetElement(pCtx, sKey, dValue);
			break;

		default:
			//	Nothing happens
			break;
		}
	}

void CDatum::SetElement (const CString &sKey, CDatum dValue)

//	SetElement
//
//	Sets the value to the datum. Note that this modifies the datum in place.
//	Do not use this unless you are sure about who else might have a pointer
//	to the datum.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			raw_GetComplex()->SetElement(sKey, dValue);
			break;

		default:
			//	Nothing happens
			break;
		}
	}

void CDatum::SetElement (int iIndex, CDatum dValue)

//	SetElement
//
//	Sets the value to the datum. Note that this modifies the datum in place.
//	Do not use this unless you are sure about who else might have a pointer
//	to the datum.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			raw_GetComplex()->SetElement(iIndex, dValue);
			break;

		default:
			//	Nothing happens
			break;
		}
	}

void CDatum::SetElementAt (CDatum dIndex, CDatum dValue)

//	SetElement
//
//	Sets the value to the datum. Note that this modifies the datum in place.
//	Do not use this unless you are sure about who else might have a pointer
//	to the datum.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			raw_GetComplex()->SetElementAt(dIndex, dValue);
			break;

		default:
			//	Nothing happens
			break;
		}
	}

void CDatum::SetMethodsExt (CDatum::Types iType, TDatumMethodHandler<IComplexDatum> &MethodsExt)

//	SetMethodsExt
//
//	Sets an extension block for methods.

	{
	switch (iType)
		{
		case Types::typeArray:
			CComplexArray::SetMethodsExt(MethodsExt);
			break;

		case Types::typeTable:
			CAEONTable::SetMethodsExt(MethodsExt);
			CAEONTableRef::SetMethodsExt(MethodsExt);
			break;

		//	Not Implemented.

		default:
			throw CException(errFail);
		}
	}

void CDatum::Sort (ESortOptions Order, TArray<CDatum>::COMPAREPROC pfCompare, void *pCtx)

//	Sort
//
//	Sorts the datum in place

	{
	if (pfCompare == NULL)
		pfCompare = CDatum::DefaultCompare;

	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_COMPLEX:
			raw_GetComplex()->Sort(Order, pfCompare, pCtx);
			break;

		default:
			//	Nothing happens
			break;
		}
	}

CDatum CDatum::VectorOf (Types iType, CDatum dValues)

//	VectorOf
//
//	Creates a vector of the given type.

	{
	CDatum dVector;

	switch (iType)
		{
		case typeUnknown:
			dVector = CDatum(CDatum::typeArray);
			break;

		case typeDouble:
			dVector = CDatum(new CAEONVectorFloat64());
			break;

		case typeInteger32:
			dVector = CDatum(new CAEONVectorInt32());
			break;

		case typeIntegerIP:
			dVector = CDatum(new CAEONVectorIntIP());
			break;

		case typeString:
			dVector = CDatum(new CAEONVectorString());
			break;

		default:
			//	Not supported
			throw CException(errFail);
		}
	
	for (int i = 0; i < dValues.GetCount(); i++)
		dVector.Append(dValues.GetElement(i));

	return dVector;
	}
