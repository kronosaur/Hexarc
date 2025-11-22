//	CDatum.cpp
//
//	CDatum class
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"
#include <functional>
#include <string_view>

DECLARE_CONST_STRING(DATE_ORDER_DMY,					"DMY");
DECLARE_CONST_STRING(DATE_ORDER_MDY,					"MDY");
DECLARE_CONST_STRING(DATE_ORDER_YMD,					"YMD");

DECLARE_CONST_STRING(FIELD_CENTURY,						"century");
DECLARE_CONST_STRING(FIELD_DATE_ORDER,					"dateOrder");
DECLARE_CONST_STRING(FIELD_DAY_MISSING,					"dayMissing");
DECLARE_CONST_STRING(FIELD_DECODE_AS,					"decodeAs");
DECLARE_CONST_STRING(FIELD_DEFAULT,						"default");
DECLARE_CONST_STRING(FIELD_IMPUTE,						"impute");
DECLARE_CONST_STRING(FIELD_MONTH_MISSING,				"monthMissing");
DECLARE_CONST_STRING(FIELD_TYPE,						"type");
DECLARE_CONST_STRING(FIELD_YEAR_MISSING,				"yearMissing");

DECLARE_CONST_STRING(FORMAT_EXCEL,						"excel");
DECLARE_CONST_STRING(FORMAT_EXCEL1900,					"excel1900");
DECLARE_CONST_STRING(FORMAT_UNIX,						"unix");
DECLARE_CONST_STRING(FORMAT_UNIX_MS,					"unixMS");
DECLARE_CONST_STRING(FORMAT_UNIX_SECONDS,				"unixSeconds");

DECLARE_CONST_STRING(IMPUTE_DEFAULT,					"default");
DECLARE_CONST_STRING(IMPUTE_FIRST,						"first");
DECLARE_CONST_STRING(IMPUTE_LAST,						"last");
DECLARE_CONST_STRING(IMPUTE_NONE,						"none");
DECLARE_CONST_STRING(IMPUTE_NULL,						"null");

DECLARE_CONST_STRING(TYPENAME_DOUBLE,					"double");
DECLARE_CONST_STRING(TYPENAME_ENUM,						"enum");
DECLARE_CONST_STRING(TYPENAME_FALSE,					"false");
DECLARE_CONST_STRING(TYPENAME_INT32,					"integer32");
DECLARE_CONST_STRING(TYPENAME_NIL,						"nil");
DECLARE_CONST_STRING(TYPENAME_STRING,					"string");
DECLARE_CONST_STRING(TYPENAME_TRUE,						"true");

DECLARE_CONST_STRING(STR_B001,							"B001");
DECLARE_CONST_STRING(STR_FALSE,							"false");
DECLARE_CONST_STRING(STR_INFINITY_N,					"-infinity");
DECLARE_CONST_STRING(STR_INFINITY_P,					"+infinity");
DECLARE_CONST_STRING(STR_NAN,							"nan");
DECLARE_CONST_STRING(STR_NIL,							"nil");
DECLARE_CONST_STRING(STR_TRUE,							"true");

DECLARE_CONST_STRING(ERR_UNKNOWN_FORMAT,				"Unable to determine file format: %s.");
DECLARE_CONST_STRING(ERR_CANT_OPEN_FILE,				"Unable to open file: %s.");
DECLARE_CONST_STRING(ERR_DESERIALIZE_ERROR,				"Unable to parse file: %s.");
DECLARE_CONST_STRING(ERR_NO_METHODS,					"Methods not supported.");
DECLARE_CONST_STRING(ERR_INVALID_TABLE_DESC,			"Invalid table descriptor.");
DECLARE_CONST_STRING(ERR_EXPECTED_INT32,				"Expected Int32 value: %s.");
DECLARE_CONST_STRING(ERR_NOT_SUPPORTED,					"Operation not supported.");
DECLARE_CONST_STRING(ERR_INVALID_PARAMS,				"Invalid parameters.");

static CAEONFactoryList *g_pFactories = NULL;
static int g_iUnalignedLiteralCount = 0;

// One list to rule them all:
#define DATUM_COMPARE_ORDER(X)        \
	X(typeNil,           1)           \
	X(typeNaN,           2)           \
	X(typeFalse,         3)           \
	X(typeTrue,          4)           \
	X(typeInteger32,     5)           \
	X(typeInteger64,     6)           \
	X(typeIntegerIP,     7)           \
	X(typeDouble,        8)           \
	X(typeRange,         9)           \
	X(typeTimeSpan,     10)           \
	X(typeDateTime,     11)           \
	X(typeEnum,         12)           \
	X(typeSymbol,       13)           \
	X(typeString,       14)           \
	X(typeArray,        15)           \
	X(typeTensor,       16)           \
	X(typeTable,        17)           \
	X(typeStruct,       18)           \
	X(typeRowRef,       19)           \
	X(typeObject,       20)           \
	X(typeClassInstance,21)           \
	X(typeAEONObject,   22)           \
	X(typeVoid,         23)           \
	X(typeBinary,       24)           \
	X(typeImage32,      25)           \
	X(typeVector2D,     26)           \
	X(typeVector3D,     27)           \
	X(typeTextLines,    28)           \
	X(typeExpression,   29)           \
	X(typeLibraryFunc,  30)           \
	X(typeAnnotated,    31)           \
	X(typeForeign,      32)           \
	X(typeDatatype,     33)           \
	X(typeError,        34)

const CDatum::SCompareOrderEntry CDatum::m_COMPARE_ORDER[typeCount] = {
#define X(t,r) { t, r },
	DATUM_COMPARE_ORDER(X)
#undef X
};

constinit std::array<int, CDatum::typeCount> CDatum::m_COMPARE_RANK = []() consteval {
	std::array<int, typeCount> a{};      // zero-initialized
#define X(t,r) a[t] = r;
	DATUM_COMPARE_ORDER(X)
#undef X
	return a;
}();

CDatum::SAnnotation CDatum::m_NullAnnotation;

CDatum::CDatum (Types iType)

//	CDatum constructor

	{
	switch (iType)
		{
		case typeNil:
			m_dwData = VALUE_NULL;
			break;

		case typeFalse:
			m_dwData = VALUE_FALSE;
			break;

		case typeTrue:
			m_dwData = VALUE_TRUE;
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

		case typeClassInstance:
			*this = CDatum(new CAEONObject(CAEONTypeSystem::GetCoreType(IDatatype::ANY)));
			break;

		case typeNaN:
			m_dwData = VALUE_NAN;
			break;

		default:
			ASSERT(false);
			m_dwData = VALUE_NULL;
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

				m_dwData = EncodePointer(pDateTime);
				}
			else
				m_dwData = VALUE_NULL;

			break;
			}

		case typeString:
			*this = CDatum(sValue);
			break;

		default:
			throw CException(errFail);
		}
	}

CDatum::CDatum (DWORD dwValue)

//	CDatum constructor

	{
	if (dwValue > INT_MAX)
		*this = CDatum(CIPInteger((DWORDLONG)dwValue));
	else
		m_dwData = EncodeInt32((int)dwValue);
	}

CDatum::CDatum (DWORDLONG ilValue)

//	CDatum constructor

	{
	//	Store as IP integer

	CComplexInteger *pIPInt = new CComplexInteger(ilValue);

	//	Take ownership of the complex type

	CAEONStore::Alloc(pIPInt);

	//	Store the pointer and assign type

	m_dwData = EncodePointer(pIPInt);
	}

CDatum::CDatum (double rValue)

//	CDatum constructor

	{
	DWORDLONG bits = *(DWORDLONG*)&rValue;	// Interpret the double as a 64-bit integer
	DWORDLONG exponent = (bits >> 52) & 0x7FF;
	DWORDLONG fraction = bits & 0xFFFFFFFFFFFFF;

	if (exponent == 0x7FF)
		{
		if (fraction != 0)
			{
			// It's a NaN
			m_dwData = VALUE_NAN;
			}
		else
			{
			if (bits & (1ULL << 63))
				{
				// It's -Infinity
				m_dwData = VALUE_INFINITY_N;
				}
			else
				{
				// It's +Infinity
				m_dwData = VALUE_INFINITY_P;
				}
			}
		}
	else
		{
		// Regular double value, handle accordingly
		m_dwData = bits;
		}
	}

CDatum::CDatum (const CString &sString)

//	CDatum constructor

	{
	//	Empty strings are always nil

	if (sString.IsEmpty())
		{
		m_dwData = VALUE_NULL;
		}

	//	If this is a literal string, then we can just
	//	take the value, because it doesn't need to be freed.

	else if (sString.IsLiteral())
		{
		m_dwData = EncodeString((LPCSTR)sString);
		}
	else
		{
		//	Get a copy of the naked LPSTR out of the string

		CString sNewString(sString);
		LPSTR pString = sNewString.Handoff();

		//	Track it with our allocator (but only if not NULL).
		//	(If pString is NULL then this is represented as Nil).

		if (pString)
			{
			CAEONStore::Alloc(pString);
			m_dwData = EncodeString(pString);
			}
		else
			m_dwData = VALUE_NULL;
		}
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

	m_dwData = EncodePointer(pValue);
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

		m_dwData = EncodePointer(pDateTime);
		}
	else
		m_dwData = VALUE_NULL;
	}

CDatum::CDatum (const CIPInteger &Value)

//	CDatum constructor

	{
	CComplexInteger *pValue = new CComplexInteger(Value);

	//	Take ownership of the complex type

	CAEONStore::Alloc(pValue);

	//	Store the pointer and assign type

	m_dwData = EncodePointer(pValue);
	}

CDatum::CDatum (const CRGBA32Image &Value)

//	CDatum constructor

	{
	CComplexImage32 *pValue = new CComplexImage32(Value);

	//	Take ownership of the complex type

	CAEONStore::Alloc(pValue);

	//	Store the pointer and assign type

	m_dwData = EncodePointer(pValue);
	}

CDatum::CDatum (CRGBA32Image &&Value)

//	CDatum constructor

	{
	CComplexImage32 *pValue = new CComplexImage32(std::move(Value));

	//	Take ownership of the complex type

	CAEONStore::Alloc(pValue);

	//	Store the pointer and assign type

	m_dwData = EncodePointer(pValue);
	}

CDatum::CDatum (const CStringFormat& Value)

//	CDatum constructor

	{
	m_dwData = EncodePointer(new CAEONStringFormat(Value));
	}

CDatum::CDatum (CStringFormat&& Value)

//	CDatum constructor

	{
	m_dwData = EncodePointer(new CAEONStringFormat(std::move(Value)));
	}

CDatum::CDatum (const CTimeSpan &TimeSpan)

//	CDatum constructor

	{
	CAEONTimeSpan *pTimeSpan = new CAEONTimeSpan(TimeSpan);

	//	Take ownership of the complex type

	CAEONStore::Alloc(pTimeSpan);

	//	Store the pointer and assign type

	m_dwData = EncodePointer(pTimeSpan);
	}

CDatum::CDatum (const CVector2D& Value)

//	CDatum constructor

	{
	auto pVector = new CAEONVector2D(Value);
	CAEONStore::Alloc(pVector);
	m_dwData = EncodePointer(pVector);
	}

CDatum::CDatum (const CVector3D& Value)

//	CDatum constructor

	{
	auto pVector = new CAEONVector3D(Value);
	CAEONStore::Alloc(pVector);
	m_dwData = EncodePointer(pVector);
	}

CDatum::operator int () const

//	int cast operator

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return 0;

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
					return 0;

				case VALUE_TRUE:
					return 1;

				default:
					ASSERT(false);
					return 0;
				}
			}

		case TYPE_INT32:
			return DecodeInt32(m_dwData);

		case TYPE_ENUM:
			return DecodeEnumValue(m_dwData);

		case TYPE_STRING:
			return strToInt(DecodeString(m_dwData));

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).CastInteger32();

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
		case TYPE_ROW_REF:
			return 0;

		default:
			return (int)DecodeDouble(m_dwData);
		}
	}

CDatum::operator DWORD () const

//	DWORD cast operator

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return 0;

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
					return 0;

				case VALUE_TRUE:
					return 1;

				default:
					ASSERT(false);
					return 0;
				}
			}

		case TYPE_INT32:
			return (DWORD)DecodeInt32(m_dwData);

		case TYPE_ENUM:
			return (DWORD)DecodeEnumValue(m_dwData);

		case TYPE_STRING:
			return (DWORD)strToInt(DecodeString(m_dwData));

		case TYPE_COMPLEX:
			return (DWORD)DecodeComplex(m_dwData).CastInteger32();

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
		case TYPE_ROW_REF:
			return 0;

		default:
			return (DWORD)DecodeDouble(m_dwData);
		}
	}

CDatum::operator DWORDLONG () const

//	DWORDLONG operator

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return 0;

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
					return 0;

				case VALUE_TRUE:
					return 1;

				default:
					ASSERT(false);
					return 0;
				}
			}

		case TYPE_INT32:
			return (DWORDLONG)DecodeInt32(m_dwData);

		case TYPE_ENUM:
			return (DWORDLONG)DecodeEnumValue(m_dwData);

		case TYPE_STRING:
			return (DWORDLONG)strToInt(DecodeString(m_dwData));

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).CastDWORDLONG();

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
		case TYPE_ROW_REF:
			return 0;

		default:
			return (DWORDLONG)DecodeDouble(m_dwData);
		}
	}

CDatum::operator double () const

//	double cast operator

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return 0.0;

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
					return 0.0;

				case VALUE_TRUE:
					return 1.0;

				default:
					ASSERT(false);
					return 0.0;
				}
			}

		case TYPE_INT32:
			return (double)DecodeInt32(m_dwData);

		case TYPE_ENUM:
			return (double)DecodeEnumValue(m_dwData);

		case TYPE_STRING:
			return strToDouble(DecodeString(m_dwData));

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).CastDouble();

		case TYPE_ROW_REF:
			return nan("");

		case TYPE_NAN:
			return nan("");

		case TYPE_INFINITY_N:
			return -std::numeric_limits<double>::infinity();

		case TYPE_INFINITY_P:
			return std::numeric_limits<double>::infinity();

		default:
			return DecodeDouble(m_dwData);
		}
	}

CDatum::operator const IDatatype & () const

//	IDatatype cast operator

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).CastIDatatype();

		default:
			return (const IDatatype &)CAEONTypeSystem::GetCoreType(IDatatype::ANY);
		}
	}

CDatum::operator const CDateTime & () const

//	CDateTime cast operator

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).CastCDateTime();

		default:
			return NULL_DATETIME;
		}
	}

CDatum::operator const CTimeSpan & () const

//	CTimeSpan cast operator

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).CastCTimeSpan();

		default:
			return CTimeSpan::Null();
		}
	}

CDatum::operator const CRGBA32Image & () const

//	CRGBA32Image cast operator

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).CastCRGBA32Image();

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
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).CastCIPInteger();

		default:
			return NULL_IPINTEGER;
		}
	}

CDatum::operator CStringView () const

//	CString cast operator
//
//	NOTE: The difference between this and AsString() is that
//	AsString returns a string by value, which allow for
//	more conversion types. This returns a string by reference.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return CStringView();

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
					return STR_FALSE;

				case VALUE_TRUE:
					return STR_TRUE;

				default:
					ASSERT(false);
					return CStringView();
				}
			}

		case TYPE_INT32:
		case TYPE_ENUM:
			return CStringView();

		case TYPE_STRING:
			return DecodeString(m_dwData);

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).CastCString();

		case TYPE_NAN:
			return STR_NAN;

		case TYPE_INFINITY_N:
			return STR_INFINITY_N;

		case TYPE_INFINITY_P:
			return STR_INFINITY_P;

		default:
			return CStringView();
		}
	}

CDatum::operator const CStringFormat& () const

//	CStringFormat cast operator

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).CastCStringFormat();

		default:
			return CStringFormat::Null;
		}
	}

CDatum::operator const CVector2D& () const

//	CVector2D cast operator

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).CastCVector2D();

		default:
			return CVector2D::Null;
		}
	}

CDatum::operator const CVector3D& () const

//	CVector3D cast operator

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).CastCVector3D();

		default:
			return CVector3D::Null;
		}
	}

void CDatum::Append (CDatum dValue)

//	Append
//
//	Appends the value to the datum. Note that this modifies the datum in place.
//	Do not use this unless you are sure about who else might have a pointer
//	to the datum.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).Append(dValue);
			break;

		default:
			//	Nothing happens
			break;
		}
	}

void CDatum::AppendArray (CDatum dArray)

//	AppendArray
//
//	Appends the elements of the given array to this datum. If this datum is not
//	an array, then nothing happens.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).AppendArray(dArray);
			break;

		default:
			//	Nothing happens
			break;
		}
	}


int CDatum::AsArrayIndex (int iArrayLen, bool* retbFromEnd) const

//	AsArrayIndex
//
//	Converts to a 32-bit signed index. If we return -1, then conversion failed.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return -1;

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
				case VALUE_TRUE:
					return -1;

				default:
					ASSERT(false);
					return -1;
				}
			}

		case TYPE_INT32:
			return CalcArrayIndex(DecodeInt32(m_dwData), iArrayLen, retbFromEnd);

		case TYPE_ENUM:
			return CalcArrayIndex(DecodeEnumValue(m_dwData), iArrayLen, retbFromEnd);

		case TYPE_STRING:
			{
			CDatum dNumberValue;
			if (!CDatum::CreateFromStringValue(DecodeString(m_dwData), &dNumberValue)
					|| !dNumberValue.IsNumber())
				return -1;

			return dNumberValue.AsArrayIndex(iArrayLen, retbFromEnd);
			}

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).AsArrayIndex(iArrayLen, retbFromEnd);

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
		case TYPE_ROW_REF:
			return -1;

		default:
			return CalcArrayIndex((int)DecodeDouble(m_dwData), iArrayLen, retbFromEnd);
		}
	}

void CDatum::AsAttributeList (CAttributeList* retAttribs) const

//	AsAttributeList
//
//	Generates an attribute list

	{
	retAttribs->DeleteAll();
	for (int i = 0; i < GetCount(); i++)
		retAttribs->Insert(GetElement(i));
	}

CDateTime CDatum::AsDateTime (void) const

//	AsDateTime
//
//	Coerces to a CDateTime

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_STRING:
			{
			CDateTime DateTime;
			CDateTimeParser::SOptions Options;
			if (!CDateTimeParser::Parse(DecodeString(m_dwData), Options, &DateTime))
				return NULL_DATETIME;

			return DateTime;
			}

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).CastCDateTime();

		default:
			return NULL_DATETIME;
		}
	}

const CAEONExpression& CDatum::AsExpression () const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			{
			const CAEONExpression* pExpr = DecodeComplex(m_dwData).GetQueryInterface();
			if (pExpr)
				return *pExpr;
			else
				return CAEONExpression::Null;
			}

		default:
			return CAEONExpression::Null;
		}
	}

CDatum CDatum::AsFloat (bool bNullIfInvalid) const

//	AsFloat
//
//	Coerces to float of the appropriate size. If the value cannot be 
//	converted, we return nan. NOTE: We lose precision if the value is an 
//	IPInteger that does not fit in a double.

	{
	switch (GetBasicType())
		{
		case typeNil:
			if (bNullIfInvalid)
				return CDatum();
			else if (IsIdenticalToNil())
				return CDatum(0.0);
			else
				return CDatum::CreateNaN();

		case typeFalse:
			return CDatum(0.0);

		case typeTrue:
			return CDatum(1.0);

		case typeInteger32:
		case typeInteger64:
		case typeIntegerIP:
			return CDatum((double)*this);

		case typeDouble:
		case typeNaN:
			return *this;

		case typeEnum:
			return CDatum((double)(int)*this);

		case typeString:
			{
			CDatum dNumberValue;
			if (!CDatum::CreateFromStringValue(*this, &dNumberValue)
					|| !dNumberValue.IsNumber())
				return (bNullIfInvalid ? CDatum() : CDatum::CreateNaN());

			return dNumberValue.AsFloat();
			}

		case typeTimeSpan:
			{
			const CTimeSpan &Value = (const CTimeSpan &)*this;
			if (Value.IsNegative())
				return CDatum(-(double)Value.Milliseconds64());
			else
				return CDatum((double)Value.Milliseconds64());
			}

		case typeExpression:
			return CAEONExpression::CreateUnaryOp(CAEONExpression::EOp::Real, AsExpression());

		default:
			return (bNullIfInvalid ? CDatum() : CDatum::CreateNaN());
		}
	}

int CDatum::AsInt32 () const

//	AsInt32
//
//	Coerces into an int32. If the value exceeds the range of an integer, we 
//	return either INT_MAX or INT_MIN.

	{
	switch (GetBasicType())
		{
		case typeNil:
		case typeFalse:
			return 0;

		case typeTrue:
			return 1;

		case typeInteger32:
		case typeEnum:
			return (int)*this;

		case typeInteger64:
		case typeIntegerIP:
			{
			const auto &Value = (const CIPInteger &)*this;
			if (Value.FitsAsInteger32Signed())
				return Value.AsInteger32Signed();
			else if (Value.IsNegative())
				return INT_MIN;
			else
				return INT_MAX;
			}

		case typeDouble:
			{
			double rValue = (double)*this;
			if (rValue < (double)INT_MIN)
				return INT_MIN;
			else if (rValue > (double)INT_MAX)
				return INT_MAX;
			else
				return (int)rValue;
			}

		case typeString:
			{
			CDatum dNumberValue;
			if (!CDatum::CreateFromStringValue(CStringView(*this), &dNumberValue)
					|| !dNumberValue.IsNumber())
				return 0;

			return dNumberValue.AsInt32();
			}

		default:
			return 0;
		}
	}

CDatum CDatum::AsInteger () const

//	AsInteger
//
//	Coerces to an integer of the appropriate size. If the value cannot be 
//	converted, we return Nil. NOTE: We lose precision if the value is a double.

	{
	switch (GetBasicType())
		{
		case typeNil:
			return CDatum();

		case typeFalse:
			return CDatum(0);

		case typeTrue:
			return CDatum(1);

		case typeInteger32:
		case typeEnum:
			return CDatum((int)*this);

		case typeInteger64:
		case typeIntegerIP:
			return CDatum((const CIPInteger &)*this);

		case typeDouble:
			{
			double rValue = (double)*this;
			if (rValue >= INT_MIN && rValue <= INT_MAX)
				return CDatum((int)rValue);
			else
				return CDatum(CIPInteger(rValue));
			}

		case typeString:
			{
			CDatum dNumberValue;
			if (!CDatum::CreateFromStringValue(CStringView(*this), &dNumberValue)
					|| !dNumberValue.IsNumber())
				return CDatum();

			return dNumberValue.AsInteger();
			}

		case typeTimeSpan:
			{
			const CTimeSpan &Value = (const CTimeSpan &)*this;
			DWORDLONG dwValue = Value.Milliseconds64();
			if (dwValue <= INT_MAX)
				return CDatum(Value.IsNegative() ? -(int)dwValue : (int)dwValue);
			else
				return CDatum(CIPInteger(dwValue, Value.IsNegative()));
			}

		case typeExpression:
			return CAEONExpression::CreateUnaryOp(CAEONExpression::EOp::Integer, AsExpression());

		default:
			return CDatum();
		}
	}

CIPInteger CDatum::AsIPInteger () const

//	AsIPInteger
//
//	Returns an IPInteger

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return CIPInteger(0);

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
					return CIPInteger(0);

				case VALUE_TRUE:
					return CIPInteger(1);

				default:
					ASSERT(false);
					return CIPInteger(0);
				}
			}

		case TYPE_INT32:
			return CIPInteger(DecodeInt32(m_dwData));

		case TYPE_ENUM:
			return CIPInteger(DecodeEnumValue(m_dwData));

		case TYPE_STRING:
			{
			CIPInteger Result;
			Result.InitFromString(DecodeString(m_dwData));
			return Result;
			}

		case TYPE_COMPLEX:
			{
			const CIPInteger& Result = DecodeComplex(m_dwData).CastCIPInteger();
			if (!Result.IsEmpty())
				return Result;
			else
				return CIPInteger(0);
			}

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
		case TYPE_ROW_REF:
			return CIPInteger(0);

		default:
			return CIPInteger(DecodeDouble(m_dwData));
		}
	}

CDatum CDatum::AsMapColumnExpression () const

//	AsMapColumnExpression
//
//	Converts to a map column expression

	{
	if (GetBasicType() == CDatum::typeStruct)
		{
		//	If any of the elements are ExpressionType, then this is a map 
		//	column expression.

		for (int i = 0; i < GetCount(); i++)
			{
			if (GetElement(i).GetBasicType() == CDatum::typeExpression)
				return CAEONMapColumnExpression::CreateFromStruct(*this);
			}

		//	Not a map column expression

		return CDatum();
		}
	else if (CAEONMapColumnExpression::Upconvert(*this))
		return *this;
	else
		return CDatum();
	}

CDatum CDatum::AsNumber (bool bNullIfInvalid) const

//	AsNumber
//
//	Coerces to a number of the appropriate type. If the value cannot be 
//	converted, we return nan.

	{
	switch (GetBasicType())
		{
		case typeNil:
			return CDatum();

		case typeFalse:
			return CDatum(0);

		case typeTrue:
			return CDatum(1);

		case typeInteger32:
		case typeInteger64:
		case typeIntegerIP:
		case typeDouble:
		case typeNaN:
			return *this;

		case typeEnum:
			return CDatum((int)*this);

		case typeString:
			{
			CDatum dNumberValue;
			if (!CDatum::CreateFromStringValue(*this, &dNumberValue)
					|| !dNumberValue.IsNumber())
				return (bNullIfInvalid ? CDatum() : CDatum::CreateNaN());

			return dNumberValue;
			}

		case typeTimeSpan:
			{
			const CTimeSpan &Value = (const CTimeSpan &)*this;
			DWORDLONG dwValue = Value.Milliseconds64();
			if (dwValue <= INT_MAX)
				return CDatum(Value.IsNegative() ? -(int)dwValue : (int)dwValue);
			else
				return CDatum(CIPInteger(dwValue, Value.IsNegative()));
			}

		case typeExpression:
			return CAEONExpression::CreateUnaryOp(CAEONExpression::EOp::Number, AsExpression());

		default:
			return (bNullIfInvalid ? CDatum() : CDatum::CreateNaN());
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
		CStringView sValue = *this;
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

IByteStream& CDatum::AsStream () const

//	AsStream
//
//	Returns a stream to the datum.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).AsStream();

		default:
			throw CException(errFail);
		}
	}

CDatum CDatum::AsStruct () const

//	AsStruct
//
//	Converts to a structure.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).AsStruct();

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::AsStruct(m_dwData);

		default:
			return CDatum(CDatum::typeStruct);
		}
	}

CString CDatum::AsString (void) const

//	AsString
//
//	Coerces to a CString

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return NULL_STR;

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
					return STR_FALSE;

				case VALUE_TRUE:
					return STR_TRUE;

				default:
					ASSERT(false);
					return NULL_STR;
				}
			}

		case TYPE_INT32:
			return strFromInt(DecodeInt32(m_dwData));

		case TYPE_ENUM:
			{
			int iValue = DecodeEnumValue(m_dwData);
			DWORD dwDatatypeID = DecodeEnumType(m_dwData);
			CDatum dType = CAEONTypes::Get(dwDatatypeID);
			const IDatatype& Datatype = dType;
			int iIndex = Datatype.FindMemberByOrdinal(iValue);

			if (iIndex == -1)
				return NULL_STR;
			else
				return Datatype.GetMember(iIndex).sLabel;
			}

		case TYPE_STRING:
			return CString(DecodeString(m_dwData));

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).AsString();

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::AsString(m_dwData);

		case TYPE_NAN:
			return STR_NAN;

		case TYPE_INFINITY_N:
			return STR_INFINITY_N;

		case TYPE_INFINITY_P:
			return STR_INFINITY_P;

		default:
			return strFromDouble(DecodeDouble(m_dwData));
		}
	}

TArray<CString> CDatum::AsStringArray (void) const

//	AsStringArray
//
//	Coerces to an array of strings.

	{
	TArray<CString> Result;
	Result.InsertEmpty(GetCount());
	for (int i = 0; i < GetCount(); i++)
		Result[i] = GetElement(i).AsString();

	return Result;
	}

CTimeSpan CDatum::AsTimeSpan () const

//	AsTimeSpan
//
//	Coerces to a time span.

	{
	switch (GetBasicType())
		{
		case typeNil:
			return CTimeSpan();

		case typeInteger32:
			{
			int iValue = (int)*this;
			return CTimeSpan((DWORDLONG)Abs(iValue), iValue < 0);
			}

		case typeInteger64:
			return CTimeSpan((DWORDLONG)*this);

		case typeIntegerIP:
			return CTimeSpan((const CIPInteger&)*this);

		case typeDouble:
			{
			double rValue = (double)*this;
			return CTimeSpan((DWORDLONG)Abs(rValue), rValue < 0.0);
			}

		case typeString:
			{
			if (m_dwData == 0)
				return CTimeSpan();

			//	LATER: Parse a time span like "00:10:13"

			CIPInteger Value;
			Value.InitFromString(DecodeString(m_dwData));
			return CTimeSpan(Value);
			}

		case typeTimeSpan:
			return (const CTimeSpan &)*this;

		default:
			return CTimeSpan();
		}
	}

DWORD CDatum::AsUInt32 () const

//	AsUInt32
//
//	Coerces into an unsigned int32. If the value exceeds the range of an integer,
//	we return either UINT_MAX or 0.

	{
	switch (GetBasicType())
		{
		case typeNil:
			return 0;

		case typeFalse:
			return 0;

		case typeTrue:
			return 1;

		case typeInteger32:
		case typeEnum:
			{
			int iValue = (int)*this;
			if (iValue < 0)
				return 0;
			else
				return (DWORD)iValue;
			}

		case typeInteger64:
		case typeIntegerIP:
			{
			const auto &Value = (const CIPInteger &)*this;
			if (Value.IsNegative())
				return 0;
			else
				{
				DWORDLONG dwValue = Value.AsInteger64Unsigned();
				if (dwValue > MAXUINT32)
					return MAXUINT32;
				else
					return (DWORD)dwValue;
				}
			}

		case typeDouble:
			{
			double rValue = (double)*this;
			if (rValue < 0.0)
				return 0;
			else if (rValue > (double)MAXUINT32)
				return MAXUINT32;
			else
				return (DWORD)rValue;
			}

		case typeString:
			{
			CDatum dNumberValue;
			if (!CDatum::CreateFromStringValue(CStringView(*this), &dNumberValue)
					|| !dNumberValue.IsNumber())
				return 0;

			return dNumberValue.AsUInt32();
			}

		default:
			return 0;
		}
	}

size_t CDatum::CalcMemorySize (void) const

//	CalcMemorySize
//
//	Computes the amount of memory used up by this datum.

	{
	size_t dwSize = sizeof(CDatum);

	switch (DecodeType(m_dwData))
		{
		case TYPE_STRING:
			dwSize += DecodeString(m_dwData).GetLength() + sizeof(DWORD) + 1;
			break;

		case TYPE_COMPLEX:
			dwSize += DecodeComplex(m_dwData).CalcMemorySize();
			break;

		case TYPE_ROW_REF:
			dwSize += CAEONRowRefImpl::CalcMemorySize(m_dwData);
			break;

		default:
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
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).CanInvoke();

		default:
			return false;
		}
	}

bool CDatum::CanSum () const

//	CanSum
//
//	Returns TRUE if this value can be included in a sum.

	{
	switch (GetBasicType())
		{
		case typeNil:
		case typeNaN:
			return false;

		case typeFalse:
		case typeTrue:
			return true;
		
		case typeInteger32:
		case typeInteger64:
		case typeIntegerIP:
			return true;

		case typeDouble:
			return !IsNaN();
		
		case typeEnum:
			return true;
		
		case typeString:
			{
			CDatum dNumberValue;
			if (!CDatum::CreateFromStringValue(*this, &dNumberValue)
					|| !dNumberValue.IsNumber())
				return false;
					
			return dNumberValue.CanSum();
			}
		
		case typeTimeSpan:
			return true;
		
		default:
			return false;
		}
	}

CDatum CDatum::Clone (EClone iMode) const

//	Clone
//
//	Returns a (shallow) clone of our datum.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			{
			if (DecodeComplex(m_dwData).IsImmutable())
				return *this;

			else
				{
				//	Let the subclass handle it.

				auto pClone = DecodeComplex(m_dwData).Clone(iMode);
				if (pClone)
					return CDatum(pClone);

				//	Try to handle it ourselves.

				else
					{
					if (iMode == EClone::CopyOnWrite)
						return CDatum(new CAEONCopyOnWrite(*this));

					//	Otherwise, we just return a reference to ourselves.

					else
						return *this;
					}
				}
			}

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::Clone(m_dwData, iMode);

		default:
			return *this;
		}
	}

int CDatum::CompareByType (Types iType1, Types iType2)
	{
	int iRank1 = m_COMPARE_RANK[iType1];
	int iRank2 = m_COMPARE_RANK[iType2];
	if (iRank1 < iRank2)
		return -1;
	else if (iRank1 > iRank2)
		return 1;
	else
		//	Do not call if same type.
		throw CException(errFail);
	}

bool CDatum::Contains (CDatum dValue) const

//	Contains
//
//	Returns TRUE this datum contains dValue.

	{
	if (dValue.m_dwData == m_dwData)
		return true;

	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			{
			auto pComplex = raw_GetComplex();

			IComplexDatum::CRecursionGuard RecursionGuard(*pComplex);
			if (RecursionGuard.InRecursion())
				return false;

			return pComplex->Contains(dValue);
			}

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::Contains(m_dwData, dValue);

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

CDatum CDatum::CreateArray (const TArray<CString>& Value)

//	CreateArray
//
//	Creates an array datum.

	{
	return CDatum(new CComplexArray(Value));
	}

CDatum CDatum::CreateArrayAsType (CDatum dType, CDatum dValue)

//	CreateArrayAsType
//
//	Creates an array of the given type.

	{
	const IDatatype& Type = dType;
	if (Type.IsA(IDatatype::DICTIONARY))
		return CDatum::CreateDictionary(dType, dValue);
	else if (Type.GetClass() == IDatatype::ECategory::Tensor)
		return CreateTensorAsType(dType, dValue);
	else
		return CreateArrayAsTypeOfElement(Type.GetElementType(), dValue, dType);
	}

CDatum CDatum::CreateArrayAsTypeOfElement (CDatum dElementType, CDatum dValue, CDatum dArrayType)

//	CreateArrayAsTypeOfElement
//
//	Creates an array of dElementType.

	{
	const IDatatype& ElementType = dElementType;

	CDatum dArray;
	switch (ElementType.GetCoreType())
		{
		case IDatatype::ANY:
		case IDatatype::NULL_T:
			dArray = CDatum::VectorOf(CDatum::typeUnknown);
			break;

		case IDatatype::INT_32:
			dArray = CDatum::VectorOf(CDatum::typeInteger32);
			break;

		case IDatatype::INT_64:
		case IDatatype::INT_IP:
			dArray = CDatum::VectorOf(CDatum::typeIntegerIP);
			break;

		case IDatatype::FLOAT:
		case IDatatype::FLOAT_64:
			dArray = CDatum::VectorOf(CDatum::typeDouble);
			break;

		case IDatatype::NUMBER:
			dArray = CDatum(new CAEONVectorNumber);
			break;

		case IDatatype::STRING:
			dArray = CDatum::VectorOf(CDatum::typeString);
			break;

		//	For anything else, we create a generic typed array

		default:
			if (dArrayType.IsNil())
				dArrayType = CAEONTypeSystem::CreateAnonymousArray(NULL_STR, dElementType);

			dArray = CDatum(new CAEONVectorTyped(dArrayType));
			break;
		}

	//	Copy data

	if (dValue.IsNil())
		{ }
	else if (dValue.GetBasicType() == CDatum::typeTensor)
		{
		CBuffer i = dValue.raw_IteratorStart();
		while (dValue.raw_IteratorHasMore(i))
			{
			dArray.Append(dValue.raw_IteratorGetElement(i));
			dValue.raw_IteratorNext(i);
			}
		}
	else if (dValue.GetBasicType() == CDatum::typeRange)
		{
		const IAEONRange* pRange = dValue.GetRangeInterface();
		if (pRange)
			{
			int iStart = pRange->GetStart();
			int iEnd = pRange->GetEnd();
			int iInc = pRange->GetStep();

			dArray.GrowToFit(pRange->GetLength());
			if (iInc > 0)
				{
				for (int i = iStart; i <= iEnd; i += iInc)
					dArray.Append(CDatum(i));
				}
			else if (iInc < 0)
				{
				for (int i = iStart; i >= iEnd; i += iInc)
					dArray.Append(CDatum(i));
				}
			}
		}
	else if (dValue.GetBasicType() == CDatum::typeStruct)
		{
		//	If an array can be an element, then create an array of key/value
		//	pairs.

		if (((const IDatatype&)CAEONTypes::Get(IDatatype::ARRAY)).IsA(ElementType))
			{
			dArray.GrowToFit(dValue.GetCount());
			for (int i = 0; i < dValue.GetCount(); i++)
				{
				CDatum dElement(CDatum::typeArray);
				dElement.GrowToFit(2);
				dElement.Append(dValue.GetKey(i));
				dElement.Append(dValue.GetElement(i));

				dArray.Append(dElement);
				}
			}
		else
			{
			dArray.GrowToFit(dValue.GetCount());

			for (int i = 0; i < dValue.GetCount(); i++)
				dArray.Append(dValue.GetElement(i));
			}
		}
	else
		{
		dArray.GrowToFit(dValue.GetCount());

		for (int i = 0; i < dValue.GetCount(); i++)
			dArray.Append(dValue.GetElement(i));
		}

	return dArray;
	}

CDatum CDatum::CreateDateTime (CDatum dValue, CDatum dOptions)
	{
	CDateTimeParser::SOptions Options;
	if (!dOptions.IsNil())
		{
		if (!ParseDateParseOptions(dOptions, Options))
			return CDatum::CreateError(ERR_INVALID_PARAMS);
		}

	switch (dValue.GetBasicType())
		{
		case CDatum::typeDateTime:
			return dValue;

		case CDatum::typeString:
			{
			CDateTime Result;
			if (!CDateTimeParser::Parse(dValue.AsStringView(), Options, &Result))
				return CDateTime();

			return CDatum(Result);
			}

		case CDatum::typeInteger32:
		case CDatum::typeInteger64:
		case CDatum::typeIntegerIP:
		case CDatum::typeDouble:
			{
			CDateTime Result;
			if (!CDateTimeParser::Parse((double)dValue, Options, &Result))
				return CDateTime();

			return CDatum(Result);
			}

		case CDatum::typeExpression:
			return CAEONExpression::CreateUnaryOp(CAEONExpression::EOp::DateTime, dValue.AsExpression());

		default:
			{
			CDateTime Result;
			if (!CDateTimeParser::Parse(dValue.AsString(), Options, &Result))
				return CDateTime();

			return CDatum(Result);
			}
		}
	}

bool CDatum::ParseDateParseOptions (CDatum dOptions, CDateTimeParser::SOptions& retOptions)
	{
	CStringView sDateOrder = dOptions.GetElement(FIELD_DATE_ORDER).AsStringView();
	if (sDateOrder.IsEmpty())
		{ }
	else if (strEqualsNoCase(sDateOrder, DATE_ORDER_DMY))
		retOptions.iDateOrder = CDateTimeParser::EDateOrder::DMY;
	else if (strEqualsNoCase(sDateOrder, DATE_ORDER_MDY))
		retOptions.iDateOrder = CDateTimeParser::EDateOrder::MDY;
	else if (strEqualsNoCase(sDateOrder, DATE_ORDER_YMD))
		retOptions.iDateOrder = CDateTimeParser::EDateOrder::YMD;
	else
		return false;
	
	retOptions.iCentury = (int)dOptions.GetElement(FIELD_CENTURY);

	//	If we have a single impute field then it applies to all.

	CDatum dImpute = dOptions.GetElement(FIELD_IMPUTE);
	if (!dImpute.IsNil())
		{
		if (!ParseDateParseImpute(dImpute, retOptions.DayMissing))
			return false;

		retOptions.MonthMissing = retOptions.DayMissing;
		retOptions.YearMissing = retOptions.DayMissing;
		}

	//	Otherwise, we expect separate fields.

	else
		{
		if (!ParseDateParseImpute(dOptions.GetElement(FIELD_DAY_MISSING), retOptions.DayMissing))
			return false;

		if (!ParseDateParseImpute(dOptions.GetElement(FIELD_MONTH_MISSING), retOptions.MonthMissing))
			return false;

		if (!ParseDateParseImpute(dOptions.GetElement(FIELD_YEAR_MISSING), retOptions.YearMissing))
			return false;
		}

	//	Option to convert from a numeric format.

	CStringView sDecodeAs = dOptions.GetElement(FIELD_DECODE_AS).AsStringView();
	if (sDecodeAs.IsEmpty())
		retOptions.iNumericSystem = CDateTimeParser::ENumericSystem::None;
	else if (strEqualsNoCase(sDecodeAs, FORMAT_EXCEL1900) || strEqualsNoCase(sDecodeAs, FORMAT_EXCEL))
		retOptions.iNumericSystem = CDateTimeParser::ENumericSystem::Excel1900;
	else if (strEqualsNoCase(sDecodeAs, FORMAT_UNIX))
		retOptions.iNumericSystem = CDateTimeParser::ENumericSystem::Unix;
	else if (strEqualsNoCase(sDecodeAs, FORMAT_UNIX_MS))
		retOptions.iNumericSystem = CDateTimeParser::ENumericSystem::UnixMS;
	else if (strEqualsNoCase(sDecodeAs, FORMAT_UNIX_SECONDS))
		retOptions.iNumericSystem = CDateTimeParser::ENumericSystem::UnixSeconds;
	else
		return false;

	return true;
	}

bool CDatum::ParseDateParseImpute (CDatum dImpute, CDateTimeParser::SImputeDesc& retImpute)
	{
	if (dImpute.IsNil())
		retImpute.iType = CDateTimeParser::EImpute::None;
	else if (dImpute.GetBasicType() == CDatum::typeString)
		{
		retImpute.iType = ParseImputeType(dImpute.AsStringView());
		}
	else
		{
		retImpute.iType = ParseImputeType(dImpute.GetElement(FIELD_TYPE).AsStringView());
		retImpute.Default = dImpute.GetElement(FIELD_DEFAULT);
		}

	return true;
	}

CDateTimeParser::EImpute CDatum::ParseImputeType (CStringView sValue)
	{
	if (sValue.IsEmpty() || strEqualsNoCase(sValue, IMPUTE_NONE))
		return CDateTimeParser::EImpute::None;
	else if (strEqualsNoCase(sValue, IMPUTE_DEFAULT))
		return CDateTimeParser::EImpute::Default;
	else if (strEqualsNoCase(sValue, IMPUTE_FIRST))
		return CDateTimeParser::EImpute::First;
	else if (strEqualsNoCase(sValue, IMPUTE_LAST))
		return CDateTimeParser::EImpute::Last;
	else if (strEqualsNoCase(sValue, IMPUTE_NULL))
		return CDateTimeParser::EImpute::Null;
	else
		return CDateTimeParser::EImpute::None;
	}

CDatum CDatum::CreateString (CDatum dValue, CDatum dFormat)
	{
	if (dFormat.GetBasicType() == CDatum::typeString)
		{
		CStringFormat Format(dFormat.AsStringView());
		return CDatum(dValue.Format(Format));
		}
	else
		{
		return CDatum(dValue.Format((const CStringFormat&)dFormat));
		}
	}

CDatum CDatum::CreateTensorAsType (CDatum dType, CDatum dValue, bool bConstruct)

//	CreateTensorAsType
//
//	Creates a new tensor object.

	{
	const IDatatype& Type = dType;

	//	If we're already this type, then we're done.

	const IDatatype& ValueType = dValue.GetDatatype();
	if (!bConstruct && ValueType.IsA(Type) && !ValueType.IsNullType())
		return dValue;

	//	Make sure this is a tensor type.

	TArray<CDatum> Dims = Type.GetDimensionTypes();
	if (Dims.GetCount() == 0)
		return CDatum();

	//	Dimensions must be numeric ranges.

	for (int i = 0; i < Dims.GetCount(); i++)
		{
		const IDatatype& DimType = Dims[i];
		IDatatype::SNumberDesc NumberDesc = DimType.GetNumberDesc();
		if (NumberDesc.bNumber && NumberDesc.bSubRange)
			{ }
		else if (DimType.GetClass() == IDatatype::ECategory::Enum)
			{ }
		else
			return CDatum();
		}

	//	Now create the tensor.

	return CDatum(new CAEONTensor(dType, dValue));
	}

CDatum CDatum::CreateAsType (CDatum dType, CDatum dValue, bool bConstruct)

//	CreateAsType
//
//	Create a new datum of the given type.
//
//	If bConstruct is TRUE, then we create a new object (a clone) even if the
//	value is already of the appropriate type. Note that this only applies to
//	mutable types.

	{
	const IDatatype &Type = dType;

	//	Handle nullable types

	if (Type.IsNullable())
		{
		if (dValue.IsNil())
			return CDatum();
		else
			return CreateAsType(Type.GetVariantType(), dValue, bConstruct);
		}

	//	Handle some core types.

	switch (Type.GetCoreType())
		{
		case IDatatype::ANY:
			return dValue;

		case IDatatype::ARRAY:
			if (dValue.GetBasicType() == CDatum::typeArray || dValue.GetBasicType() == CDatum::typeTensor)
				{
				if (bConstruct)
					return dValue.Clone();
				else
					return dValue;
				}
			else
				return CreateArrayAsType(dType, dValue);

		case IDatatype::BOOL:
			return CDatum((bool)!dValue.IsNil());

		case IDatatype::DATATYPE:
			if (dValue.GetBasicType() == CDatum::typeDatatype)
				return dValue;
			else
				return CDatum();

		case IDatatype::DATE_TIME:
			if (dValue.GetBasicType() == CDatum::typeDateTime)
				return dValue;
			else
				return CDatum(dValue.AsDateTime());

		case IDatatype::DICTIONARY:
			if (dValue.GetBasicType() == CDatum::typeStruct)
				{
				if (bConstruct)
					return dValue.Clone();
				else
					return dValue;
				}
			else
				return CDatum::CreateDictionary(CAEONTypes::Get(IDatatype::DICTIONARY), dValue);

		case IDatatype::ERROR_T:
			return CDatum::CreateError(dValue);

		case IDatatype::EXPRESSION:
			if (dValue.GetBasicType() == CDatum::typeExpression)
				return dValue;
			else if (dValue.GetBasicType() == CDatum::typeString)
				return CAEONExpression::Parse(dValue.AsStringView());
			else
				return CDatum();

		case IDatatype::FLOAT:
		case IDatatype::FLOAT_64:
			return dValue.AsFloat();

		case IDatatype::REAL:
			return dValue.AsFloat(true);

		case IDatatype::INT_32:
			return dValue.AsInt32();

		case IDatatype::UINT_32:
			return dValue.AsUInt32();

		case IDatatype::INT_64:
		case IDatatype::UINT_64:
		case IDatatype::INT_IP:
			//	LATER: Need to clamp if 64-bit
			if (dValue.GetBasicType() == CDatum::typeIntegerIP)
				return dValue;
			else
				return CDatum(dValue.AsIPInteger());

		case IDatatype::INTEGER:
		case IDatatype::SIGNED:
		case IDatatype::UNSIGNED:
			return dValue.AsInteger();

		case IDatatype::NULL_T:
			return CDatum();

		case IDatatype::NUMBER:
			//	TRUE means we return null for invalid values.
			return dValue.AsNumber(true);

		case IDatatype::STRING:
			if (dValue.GetBasicType() == CDatum::typeString)
				return dValue;
			else
				return CDatum(dValue.AsString());

		case IDatatype::STRUCT:
			if (dValue.GetBasicType() == CDatum::typeStruct)
				{
				if (bConstruct)
					return dValue.Clone();
				else
					return dValue;
				}
			else
				{
				CDatum dStruct(CDatum::typeStruct);
				dStruct.Append(dValue);
				return dStruct;
				}

		case IDatatype::TIME_SPAN:
			if (dValue.GetBasicType() == CDatum::typeTimeSpan)
				return dValue;
			else
				return dValue.AsTimeSpan();

		default:
			//	Not handled.
			break;
		}

	//	Handle custom types.

	switch (Type.GetClass())
		{
		case IDatatype::ECategory::Array:
			return CreateArrayAsType(dType, dValue);

		case IDatatype::ECategory::Dictionary:
			return CDatum::CreateDictionary(dType, dValue);

		case IDatatype::ECategory::ClassDef:
		case IDatatype::ECategory::Schema:
			{
			//	If the given value is already of the appropriate type, then 
			//	we're done.

			if (((const IDatatype&)dValue.GetDatatype()).IsA(dType))
				{
				if (bConstruct)
					return dValue.Clone();
				else
					return dValue;
				}

			//	Otherwise, we create a new object.

			else
				return CreateObject(dType, dValue);
			}

		case IDatatype::ECategory::Enum:
			{
			switch (dValue.GetBasicType())
				{
				case typeEnum:
					return dValue;

				case typeInteger32:
				case typeInteger64:
				case typeIntegerIP:
				case typeDouble:
					{
					int iPos = Type.FindMemberByOrdinal((int)dValue);
					if (iPos == -1)
						return CDatum();

					return CreateEnum((int)dValue, dType);
					}

				case typeString:
					{
					int iPos = Type.FindMember(dValue);
					if (iPos == -1)
						return CDatum();

					IDatatype::SMemberDesc Member = Type.GetMember(iPos);
					if (Member.iType != IDatatype::EMemberType::EnumValue)
						return CDatum();

					return CreateEnum(Member.iOrdinal, dType);
					}

				case typeArray:
					{
					int iOrdinal = (int)dValue.GetElement(0);
					int iPos = Type.FindMemberByOrdinal(iOrdinal);
					if (iPos == -1)
						return CDatum();

					return CreateEnum(iOrdinal, dType);
					}

				//	Otherwise, invalid.

				default:
					return CDatum();
				}
			}

		case IDatatype::ECategory::Tensor:
			return CreateTensorAsType(dType, dValue, bConstruct);

		case IDatatype::ECategory::Number:
			{
			IDatatype::SNumberDesc NumberDesc = Type.GetNumberDesc();
			if (NumberDesc.bNumber && NumberDesc.bSubRange)
				{
				int iValue = dValue.AsInt32();
				if (iValue < NumberDesc.iSubRangeMin)
					return CDatum(NumberDesc.iSubRangeMin);
				else if (iValue > NumberDesc.iSubRangeMax)
					return CDatum(NumberDesc.iSubRangeMax);
				else
					return CDatum(iValue);
				}
			else
				return dValue;
			}

		case IDatatype::ECategory::Table:
			return CreateTable(dType, dValue);

		default:
			return Type.CreateAsType(dValue);
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

CDatum CDatum::CreateBinary (CString&& sData)

//	CreateBinary
//
//	Creates a binary datum.

	{
	return CDatum(new CComplexBinary(std::move(sData)));
	}

CDatum CDatum::CreateBinary (CStringBuffer&& Buffer)

//	CreateBinary
//
//	Creates a binary datum.

	{
	CComplexBinary *pBinary = new CComplexBinary;
	pBinary->TakeHandoff(Buffer);
	return CDatum(pBinary);
	}

CDatum CDatum::CreateBinary (CBuffer64&& Buffer)

//	CreateBinary
//
//	Creates a large binary datum.

	{
	if (Buffer.GetLength() < MAXINT32)
		{
		//	If the buffer is small enough, then we can just use a CComplexBinary.
		CComplexBinary *pBinary = new CComplexBinary(Buffer);
		return CDatum(pBinary);
		}
	else
		{
		CComplexBinary64* pBinary = new CComplexBinary64;
		pBinary->TakeHandoff(Buffer);
		return CDatum(pBinary);
		}
	}

CDatum CDatum::CreateBinary (int iSize)

//	CreateBinary
//
//	Creates a binary datum of the given size.

	{
	return CDatum(new CComplexBinary(iSize));
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

CDatum CDatum::CreateDictionary (CDatum dType, CDatum dValue)

//	CreateDictionary
//
//	Creates a dictionary of the given type.

	{
	const IDatatype& Type = dType;
	if (!Type.IsA(IDatatype::DICTIONARY))
		return CDatum();

	return CDatum(new CAEONDictionary(dType, dValue));
	}

CDatum CDatum::CreateEnum (int iValue, DWORD dwTypeID)

//	CreateEnum
//
//	Creates an enum value datum.

	{
	if (iValue < INT16_MIN || iValue > INT16_MAX)
		throw CException(errFail);

	CDatum dResult;
	dResult.m_dwData = EncodeEnum(dwTypeID, iValue);
	return dResult;
	}

CDatum CDatum::CreateEnum (int iValue, CDatum dType)

//	CreateEnum
//
//	Creates an enum value datum.

	{
	DWORD dwTypeID = ((const IDatatype&)dType).GetCoreType();
	if (dwTypeID == 0)
		{
		CDatum dNewType = CAEONTypes::FindEnumOrAdd(dType);
		dwTypeID = ((const IDatatype&)dNewType).GetCoreType();
		if (dwTypeID == 0)
			//	If this happens it means we somehow added a type without
			//	setting its ID.
			throw CException(errFail);
		}

	return CreateEnum(iValue, dwTypeID);
	}

CDatum CDatum::CreateError (CStringView sErrorDesc, CStringView sErrorCode)

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

bool CDatum::CreateFromStringValue (CStringView sValue, CDatum *retdDatum)

//	CreateFromStringValue
//
//	Creates either a string or a number depending on the value.

	{
	CDatum dDatum;

	CString sClean;
	Types iType = GetStringValueType(sValue, sClean);
	CStringView sCleanView = sClean;
	CStringView *pValue = (sClean.IsEmpty() ? &sValue : &sCleanView);

	switch (iType)
		{
		case typeNil:
			break;

		case typeInteger32:
			dDatum = CDatum(strToInt(*pValue, 0));
			break;

		case typeIntegerIP:
			{
			CIPInteger Value;
			Value.InitFromString(*pValue);
			CDatum::CreateIPIntegerFromHandoff(Value, &dDatum);
			break;
			}

		case typeDouble:
			dDatum = CDatum(strToDouble(*pValue));
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
	dResult.m_dwData = VALUE_NAN;
	return dResult;
	}

CDatum CDatum::CreateObject (CDatum dType, CDatum dValue)

//	CreateObject
//
//	Creates an object of the given type.

	{
	const IDatatype& Type = dType;
	CDatum dObj = CDatum(new CAEONObject(dType));

	//	If the type definition doesn't have members, then could be a built-in
	//	object, so just set the element from the data.

	if (Type.GetMemberCount() == 0)
		{
		if (dValue.GetBasicType() == CDatum::typeStruct)
			{
			for (int i = 0; i < dValue.GetCount(); i++)
				{
				dObj.SetElement(dValue.GetKey(i), dValue.GetElement(i));
				}
			}
		}

	//	Otherwise, use the object definition to set members.

	else
		{
		for (int i = 0; i < Type.GetMemberCount(); i++)
			{
			IDatatype::SMemberDesc MemberDesc = Type.GetMember(i);
			if (MemberDesc.iType == IDatatype::EMemberType::InstanceKeyVar
					|| MemberDesc.iType == IDatatype::EMemberType::InstanceVar)
				{
				dObj.SetElement(MemberDesc.sID, dValue.GetElement(MemberDesc.sID));
				}
			}
		}

	return dObj;
	}

CDatum CDatum::CreateObjectEmpty (CDatum dType)

//	CreateObjectEmpty
//
//	We create an object of the given type, but we do not initialize and of its
//	member variables. Callers are responsible for setting them appropriately.

	{
	return CDatum(new CAEONObject(dType));
	}

CDatum CDatum::CreateRange (CDatum dStart, CDatum dEnd, CDatum dStep)

//	CreateRange
//
//	Creates a range.

	{
	int iEnd;
	if (!dEnd.IsNumberInt32(&iEnd))
		return CreateError(strPattern(ERR_EXPECTED_INT32, dEnd.AsString()));

	int iStart;
	if (!dStart.IsNumberInt32(&iStart))
		return CreateError(strPattern(ERR_EXPECTED_INT32, dStart.AsString()));

	int iStep;
	if (dStep.IsNil())
		iStep = (iStart <= iEnd ? 1 : -1);
	else if (!dStep.IsNumberInt32(&iStep))
		return CreateError(strPattern(ERR_EXPECTED_INT32, dStep.AsString()));

	//	Create the range

	return CAEONRange::Create(iStart, iEnd, iStep);
	}

CDatum CDatum::CreateRange (int iStart, int iEnd, int iStep)

//	CreateRange
//
//	Creates a range.

	{
	return CAEONRange::Create(iStart, iEnd, iStep);
	}

CDatum CDatum::CreateRowRef (DWORD dwTableID, int iRowIndex)

//	CreateRowRef
//
//	Creates a row reference.

	{
	CDatum dResult;
	dResult.m_dwData = ENCODED_ROW_REF | CAEONRowRefImpl::Encode(dwTableID, iRowIndex);
	return dResult;
	}

CDatum CDatum::CreateString (CStringBuffer&& Buffer)
	{
	//	Take ownership of the data

	LPSTR pString = Buffer.Handoff();

	//	Track it with our allocator (but only if not NULL).
	//	(If pString is NULL then this is represented as Nil).

	CDatum dResult;
	if (pString)
		{
		CAEONStore::Alloc(pString);
		dResult.m_dwData = EncodeString(pString);
		}
	else
		dResult.m_dwData = VALUE_NULL;

	//	Done

	return dResult;
	}

bool CDatum::CreateStringFromHandoff (CString &sString, CDatum *retDatum)

//	CreateStringFromHandoff
//
//	Creates a string by taking a handoff from a string

	{
	if (sString.IsLiteral())
		{
		if (sString.IsEmpty())
			retDatum->m_dwData = VALUE_NULL;
		else
			retDatum->m_dwData = EncodeString((LPSTR)sString);
		}
	else
		{
		//	Take ownership of the data

		LPSTR pString = sString.Handoff();

		//	Track it with our allocator (but only if not NULL).
		//	(If pString is NULL then this is represented as Nil).

		if (pString)
			{
			CAEONStore::Alloc(pString);
			retDatum->m_dwData = EncodeString(pString);
			}
		else
			retDatum->m_dwData = VALUE_NULL;
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
		{
		CAEONStore::Alloc(pString);
		retDatum->m_dwData = EncodeString(pString);
		}
	else
		retDatum->m_dwData = VALUE_NULL;

	//	Done

	return true;
	}

CDatum CDatum::CreateTable (CDatum dType, CDatum dValue)

//	CreateTable
//
//	Creates a table

	{
	const IDatatype &Schema = dType;
	if (Schema.GetClass() == IDatatype::ECategory::Schema)
		{
		dType = CAEONTypeSystem::CreateAnonymousTable(NULL_STR, dType);
		}
	else if (Schema.GetClass() != IDatatype::ECategory::Table)
		return CDatum();

	CDatum dTable = CDatum(new CAEONTable(dType));
	IAEONTable *pTable = dTable.GetTableInterface();
	if (!pTable)
		throw CException(errFail);

	pTable->AppendTable(dValue);

	//	If dValue is a table, take the nextID.

	if (dValue.GetBasicType() == CDatum::typeTable)
		{
		IAEONTable *pValueTable = dValue.GetTableInterface();
		if (!pValueTable)
			throw CException(errFail);

		pTable->SetNextID(pValueTable->GetNextID());
		}

	return dTable;
	}

CDatum CDatum::CreateTable (CDatum dType, TArray<CDatum>&& Cols)
	{
	return CAEONTable::Create(dType, std::move(Cols));
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
		case CDatum::typeTensor:
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

void CDatum::DeleteElement (int iIndex)

//	DeleteElement
//
//	Deletes the given element.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).DeleteElement(iIndex);
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
			case EFormat::AEONBinary:
			case EFormat::AEONBinaryLocal:
				{
				//	If the stream starts with 'B001' then this is a binary 
				//	format.

				CString sB001(4);
				int iRead = Stream.Read(sB001.GetPointer(), 4);
				if (strEquals(sB001, STR_B001))
					{
					CAEONSerializedMap Serialized;
					if (iFormat == EFormat::AEONBinaryLocal)
						Serialized.SetLocal(true);

					*retDatum = DeserializeAEON(Stream, Serialized);
					return true;
					}

				//	Otherwise, this is the original format.

				else
					{
					Stream.Seek(Stream.GetPos() - iRead);
					return DeserializeAEONScript(Stream, pExtension, retDatum);
					}
				}

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

bool CDatum::DeserializeEnumFromAEONScript (CDatum dType, CDatum dValue, CDatum &retdDatum)

//	DeserializeEnumFromAEONScript
//
///	Deserialize an enum.

	{
	//	Make sure the type is valid.

	const IDatatype& EnumType = dType;
	if (EnumType.GetClass() != IDatatype::ECategory::Enum)
		return false;

	//	The value is either an ordinal (for backwards compatibility) or a 
	//	string ID.

	if (dValue.GetBasicType() == CDatum::typeString)
		{
		CStringView sID = dValue;
		int iIndex = EnumType.FindMember(sID);
		if (iIndex == -1)
			return false;

		IDatatype::SMemberDesc MemberDesc = EnumType.GetMember(iIndex);
		retdDatum = CDatum::CreateEnum(MemberDesc.iOrdinal, dType);
		return true;
		}
	else
		{
		int iOrdinal = dValue;
		if (EnumType.FindMemberByOrdinal(iOrdinal) == -1)
			return false;

		retdDatum = CDatum::CreateEnum(iOrdinal, dType);
		return true;
		}
	}

bool CDatum::DeserializeEnumFromJSON (const CString& sLabel, const CString& sValue, CDatum &retdDatum)

//	DeserializeEnumFromJSON
//
//	Deserialize an enum.

	{
	//	For efficiency we don't store an entire datatype in an enum value when
	//	saving to JSON, so all we have is the label and ID. We deserialize the
	//	ID.
	//
	//	If you need to deserialize into an enum, use CDatum::CreateAsType with
	//	the enum type.

	retdDatum = sValue;
	return true;
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

bool CDatum::EnumElements (DWORD dwFlags, std::function<bool(CDatum)> fn) const

//	EnumElements
//
//	Enumerates over all elements, recursively. Returns FALSE if one of the calls
//	returned FALSE.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			if (dwFlags & FLAG_ALLOW_NULLS)
				return fn(*this);
			else
				return true;

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).EnumElements(dwFlags, fn);

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::EnumElements(m_dwData, dwFlags, fn);

		default:
			return fn(*this);
		}
	}

bool CDatum::Find (CDatum dValue, int *retiIndex) const

//	Find
//
//	Looks for an element in an array and returns the index.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).Find(dValue, retiIndex);

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::Find(m_dwData, dValue, retiIndex);

		default:
			return false;
		}
	}

CDatum CDatum::FindAll (CDatum dValue) const

//	FindAll
//
//	Looks for an element in an array and returns the index.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).FindAll(dValue);

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::FindAll(m_dwData, dValue);

		default:
			return CDatum();
		}
	}

CDatum CDatum::FindAllExact (CDatum dValue) const

//	FindAllExact
//
//	Looks for an element in an array and returns the index.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).FindAllExact(dValue);

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::FindAllExact(m_dwData, dValue);

		default:
			return CDatum();
		}
	}

bool CDatum::FindElement (const CString &sKey, CDatum *retpValue)

//	FindElement
//
//	Looks for the element by key.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).FindElement(sKey, retpValue);

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::FindElement(m_dwData, sKey, retpValue);

		default:
			return false;
		}
	}

bool CDatum::FindExact (CDatum dValue, int *retiIndex) const

//	FindExact
//
//	Looks for an element in an array and returns the index.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).FindExact(dValue, retiIndex);

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::FindExact(m_dwData, dValue, retiIndex);

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

int CDatum::FindMaxElement () const

//	FindMaxElement
//
//	Returns the index of the max element.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).FindMaxElement();

		case TYPE_ROW_REF:
			//	LATER: Implement
			throw CException(errFail);

		default:
			return -1;
		}
	}

int CDatum::FindMinElement () const

//	FindMinElement
//
//	Returns the index of the min element.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).FindMinElement();

		case TYPE_ROW_REF:
			//	LATER: Implement
			throw CException(errFail);

		default:
			return -1;
		}
	}

CString CDatum::Format (const CStringFormat& Format) const

//	Format
//
//	Formats a datum based on the format syntax.

	{
	if (Format.IsEmpty())
		return AsString();

	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return Format.FormatNull();

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
					return STR_FALSE;

				case VALUE_TRUE:
					return STR_TRUE;

				default:
					ASSERT(false);
					return NULL_STR;
				}
			}

		case TYPE_INT32:
			return Format.FormatInteger(*this);

		case TYPE_ENUM:
			return AsString();

		case TYPE_STRING:
			return AsString();

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).Format(Format);

		case TYPE_ROW_REF:
			return AsString();

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
			return Format.FormatDouble(*this);

		default:
			return Format.FormatDouble(DecodeDouble(m_dwData));
		}
	}

const CDatum::SAnnotation& CDatum::GetAnnotation () const

//	GetAnnotation
//
//	Get annotation

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetAnnotation();

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
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return 0;

		case TYPE_COMPLEX:
			if (GetBasicType() == typeArray)
				return DecodeComplex(m_dwData).GetCount();
			else
				return 1;

		default:
			return 1;
		}
	}

CDatum CDatum::GetArrayElement (int iIndex) const

//	GetArrayElement
//
//	Gets the appropriate element

	{
	ASSERT(iIndex >= 0);

	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			if (GetBasicType() == typeArray)
				return DecodeComplex(m_dwData).GetElement(iIndex);
			else
				return (iIndex == 0 ? *this : CDatum());

		default:
			return (iIndex == 0 ? *this : CDatum());
		}
	}

CDatum::Types CDatum::GetBasicType (void) const

//	GetBasicType
//
//	Returns the type

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return typeNil;

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
					return typeFalse;

				case VALUE_TRUE:
					return typeTrue;

				default:
					ASSERT(false);
					return typeUnknown;
				}
			}

		case TYPE_INT32:
			return typeInteger32;

		case TYPE_ENUM:
			return typeEnum;

		case TYPE_STRING:
			return typeString;

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetBasicType();

		case TYPE_ROW_REF:
			return typeRowRef;

		case TYPE_NAN:
			return typeNaN;

		case TYPE_INFINITY_N:
			return typeNaN;

		case TYPE_INFINITY_P:
			return typeNaN;

		default:
			return typeDouble;
		}
	}

void* CDatum::GetBinaryData (void) const

//	GetBinaryData
//
//	For binary blob objects, this returns a pointer to the block.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetBinaryData();

		default:
			{
			CStringView sData = *this;
			return (char*)sData.GetPointer();
			}
		}
	}

int CDatum::GetBinarySize (void) const

//	GetBinarySize
//
//	For binary blob objects, this returns the total size in bytes.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetBinarySize();

		default:
			{
			CStringView sData = *this;
			return sData.GetLength();
			}
		}
	}

DWORDLONG CDatum::GetBinarySize64 (void) const

//	GetBinarySize64
//
//	For binary blob objects, this returns the total size in bytes.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetBinarySize64();

		default:
			{
			CStringView sData = *this;
			return sData.GetLength();
			}
		}
	}

CDatum CDatum::GetDatatype () const

//	GetDatatype
//
//	Returns a datatype.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return CAEONTypes::Get(IDatatype::NULL_T);

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
				case VALUE_TRUE:
					return CAEONTypes::Get(IDatatype::BOOL);

				default:
					ASSERT(false);
					return CAEONTypes::Get(IDatatype::ANY);
				}
			}

		case TYPE_INT32:
			return CAEONTypes::Get(IDatatype::INT_32);

		case TYPE_ENUM:
			return CAEONTypes::Get(DecodeEnumType(m_dwData));

		case TYPE_STRING:
			return CAEONTypes::Get(IDatatype::STRING);

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetDatatype();

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::GetDatatype(m_dwData);

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
			return CAEONTypes::Get(IDatatype::FLOAT_64);

		default:
			return CAEONTypes::Get(IDatatype::FLOAT_64);
		}
	}

int CDatum::GetDimensions () const

//	GetDimensions
//
//	Returns the number of dimensions of an array.
//	NOTE: Non-arrays are always 0-dimensional.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetDimensions();

		default:
			return 0;
		}
	}

CRGBA32Image *CDatum::GetImageInterface ()

//	GetImageInterface
//
//	Returns an image interface (or NULL).

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetImageInterface();

		default:
			return NULL;
		}
	}

IAEONCanvas *CDatum::GetCanvasInterface ()

//	GetCanvasInterface
//
//	Returns a canvas interface (or NULL).

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetCanvasInterface();

		default:
			return NULL;
		}
	}

const CAEONExpression* CDatum::GetQueryInterface () const

//	GetQueryInterface
//
//	Returns a query interface (or NULL).

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetQueryInterface();

		default:
			return NULL;
		}
	}

IAEONRange* CDatum::GetRangeInterface ()

//	GetRangeInterface
//
//	Returns a range interface (or NULL).

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetRangeInterface();

		default:
			return NULL;
		}
	}

IAEONReanimator* CDatum::GetReanimatorInterface ()

//	GetReanimatorInterface
//
//	Returns a reanimator interface (or NULL).

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetReanimatorInterface();

		default:
			return NULL;
		}
	}

IAEONTable *CDatum::GetTableInterface ()

//	GetTableInterface
//
//	Returns a table interface (or NULL).

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetTableInterface();

		default:
			return NULL;
		}
	}

IAEONTextLines* CDatum::GetTextLinesInterface ()

//	GetTextLinesInterface
//
//	Returns a text lines interface (or NULL).

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetTextLinesInterface();

		default:
			return NULL;
		}
	}

CDatum CDatum::GetUniqueValues (DWORD dwFlags) const

//	GetUniqueValues
//
//	Returns an array of all unique values in this datum. We use CDatum::Compare.

	{
	DWORD dwEnumFlags = 0;
	if (dwFlags & FLAG_ALLOW_NULLS)
		dwEnumFlags |= CDatum::FLAG_ALLOW_NULLS;

	if (dwFlags & FLAG_EXACT)
		{
		TSortMap<CDatum, bool, CKeyCompare<CDatum>> Unique;

		bool bSuccess = EnumElements(dwEnumFlags, [&Unique](CDatum dValue)
			{
			Unique.SetAt(dValue);
			return true;
			});

		if (dwFlags & FLAG_COUNT_ONLY)
			{
			return CDatum(Unique.GetCount());
			}
		else
			{
			CDatum dResult(CDatum::typeArray);
			dResult.GrowToFit(Unique.GetCount());
			for (int i = 0; i < Unique.GetCount(); i++)
				dResult.Append(Unique.GetKey(i));

			return dResult;
			}
		}
	else
		{
		TSortMap<CDatum, bool, CKeyCompareEquivalent<CDatum>> Unique;

		bool bSuccess = EnumElements(dwEnumFlags, [&Unique](CDatum dValue)
			{
			Unique.SetAt(dValue);
			return true;
			});

		if (dwFlags & FLAG_COUNT_ONLY)
			{
			return CDatum(Unique.GetCount());
			}
		else
			{
			CDatum dResult(CDatum::typeArray);
			dResult.GrowToFit(Unique.GetCount());
			for (int i = 0; i < Unique.GetCount(); i++)
				dResult.Append(Unique.GetKey(i));

			return dResult;
			}
		}
	}

void CDatum::GrowToFit (int iCount)

//	GrowToFit
//
//	Grow array to be able to fit the given number of additional elements

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).GrowToFit(iCount);
			break;

		default:
			break;
		}
	}

size_t CDatum::Hash () const

//	Hash
//
//	Returns a hash of the value. Note that this is a case-sensitive hash.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return 0;

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
					return 0;

				case VALUE_TRUE:
					return 1;

				default:
					ASSERT(false);
					return 0;
				}
			}

		case TYPE_INT32:
			return DecodeInt32(m_dwData);

		case TYPE_ENUM:
			{
			int iValue = DecodeEnumValue(m_dwData);
			DWORD dwDatatypeID = DecodeEnumType(m_dwData);
			CDatum dType = CAEONTypes::Get(dwDatatypeID);
			const IDatatype& Datatype = dType;
			int iIndex = Datatype.FindMemberByOrdinal(iValue);

			if (iIndex == -1)
				return 0;
			else
				return std::hash<std::string_view>{}((LPCSTR)Datatype.GetMember(iIndex).sLabel);
			}

		case TYPE_STRING:
			return std::hash<std::string_view>{}((LPCSTR)DecodeString(m_dwData));

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).Hash();

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::Hash(m_dwData);

		case TYPE_NAN:
			return 0;

		case TYPE_INFINITY_N:
			return 0;

		case TYPE_INFINITY_P:
			return 0;

		default:
			return std::hash<double>{}(DecodeDouble(m_dwData));
		}
	}

bool CDatum::HasKeys () const

//	HasKeys
//
//	Returns TRUE if we have keys.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).HasKeys();

		case TYPE_ROW_REF:
			return true;

		default:
			return false;
		}
	}

bool CDatum::IsMemoryBlock (void) const

//	IsMemoryBlock
//
//	Returns TRUE if we can represent this as a contiguous block of memory.
//	If we cannot, then we need to use functions like WriteBinaryToStream.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).IsMemoryBlock();

		case TYPE_ROW_REF:
			return false;

		default:
			{
			CStringView sData = *this;
			return (sData.GetLength() > 0);
			}
		}
	}

void CDatum::WriteBinaryToStream (IByteStream &Stream, int iPos, int iLength, IProgressEvents *pProgress) const

//	WriteBinaryToStream
//
//	Writes a binary blob object to a stream.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).WriteBinaryToStream(Stream, iPos, iLength, pProgress);
			break;

		case TYPE_ROW_REF:
			break;

		default:
			{
			CStringView sData = *this;

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

void CDatum::WriteBinaryToStream (IByteStream64& Stream, DWORDLONG dwPos, DWORDLONG dwLength, IProgressEvents *pProgress) const

//	WriteBinaryToStream
//
//	Writes a binary blob object to a stream.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).WriteBinaryToStream64(Stream, dwPos, dwLength, pProgress);
			break;

		case TYPE_ROW_REF:
			break;

		default:
			{
			CStringView sData = *this;

			if (dwPos >= (DWORDLONG)sData.GetLength() || dwPos > 0x80000000)
				return;

			if (dwLength == 0xffffffffffffffff)
				dwLength = (DWORDLONG)Max((DWORD)0, (DWORD)sData.GetLength() - (DWORD)dwPos);
			else
				dwLength = Min((DWORD)dwLength, (DWORD)sData.GetLength() - (DWORD)dwPos);

			if (pProgress)
				pProgress->OnProgressStart();

			Stream.Write(sData.GetPointer() + (DWORD)dwPos, (int)dwLength);

			if (pProgress)
				pProgress->OnProgressDone();
			break;
			}
		}
	}

void CDatum::CacheInvokeResult (CHexeLocalEnvironment& LocalEnv, CDatum dResult)

//	CacheResult
//
//	Caches a function result.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).CacheInvokeResult(LocalEnv, dResult);
			break;

		default:
			break;
		}
	}

CDatum::ECallType CDatum::GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const

//	GetCallInfo
//
//	Returns info about invocation

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetCallInfo(retdCodeBank, retpIP);

		default:
			return ECallType::None;
		}
	}

IComplexDatum *CDatum::GetComplex (void) const

//	GetComplex
//
//	Returns a pointer to the complex type

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return &DecodeComplex(m_dwData);

		default:
			return NULL;
		}
	}

int CDatum::GetCount (void) const

//	GetCount
//
//	Returns the number of items in the datum

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return 0;

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetCount();

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::GetCount(m_dwData);

		default:
			return 1;
		}
	}

CDatum CDatum::GetElement (IInvokeCtx *pCtx, int iIndex) const

//	GetElement
//
//	Gets the appropriate element

	{
	ASSERT(iIndex >= 0);

	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			{
			if (iIndex == 0)
				{
				IComplexDatum *pComplex = raw_GetComplex();
				if (pComplex->IsContainer())
					return raw_GetComplex()->GetElement(pCtx, 0);
				else
					return *this;
				}
			else
				return raw_GetComplex()->GetElement(pCtx, iIndex);
			}

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::GetElement(m_dwData, pCtx, iIndex);

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

	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			{
			if (iIndex == 0)
				{
				IComplexDatum *pComplex = raw_GetComplex();
				if (pComplex->IsContainer())
					return raw_GetComplex()->GetElement(0);
				else
					return *this;
				}
			else
				return raw_GetComplex()->GetElement(iIndex);
			}

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::GetElement(m_dwData, NULL, iIndex);

		default:
			return (iIndex == 0 ? *this : CDatum());
		}
	}

CDatum CDatum::GetElement (IInvokeCtx *pCtx, const CString &sKey) const

//	GetElement
//
//	Gets the appropriate element

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetElement(pCtx, sKey);

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::GetElement(m_dwData, pCtx, sKey);

		default:
			return CDatum();
		}
	}

CDatum CDatum::GetElement (const CString &sKey) const

//	GetElement
//
//	Gets the appropriate element

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetElement(sKey);

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::GetElement(m_dwData, NULL, sKey);

		default:
			return CDatum();
		}
	}

CDatum CDatum::GetElementAt (int iIndex) const

//	GetElementAt
//
//	Gets the appropriate element

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_STRING:
			{
			CStringView sValue = DecodeString(m_dwData);
			if (iIndex >= 0 && iIndex < sValue.GetLength())
				return CString(sValue.GetParsePointer() + iIndex, 1);
			else
				return CDatum();
			}

		case TYPE_COMPLEX:
			{
			if (iIndex == 0)
				{
				IComplexDatum *pComplex = raw_GetComplex();
				if (pComplex->IsContainer())
					return raw_GetComplex()->GetElementAt(0);
				else
					return *this;
				}
			else
				return raw_GetComplex()->GetElementAt(iIndex);
			}

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::GetElementAt(m_dwData, iIndex);

		default:
			return (iIndex == 0 ? *this : CDatum());
		}
	}

CDatum CDatum::GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const

//	GetElement
//
//	Gets the appropriate element

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_STRING:
			return CAEONStringImpl::GetElementAt(DecodeString(m_dwData), TypeSystem, dIndex);

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetElementAt(TypeSystem, dIndex);

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::GetElementAt(m_dwData, TypeSystem, dIndex);

		default:
			return ((int)dIndex == 0 ? *this : CDatum());
		}
	}

CDatum CDatum::GetElementAt2DA (CDatum dIndex1, CDatum dIndex2) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetElementAt2DA(dIndex1, dIndex2);

		default:
			return CDatum();
		}
	}

CDatum CDatum::GetElementAt3DA (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetElementAt3DA(dIndex1, dIndex2, dIndex3);

		default:
			return CDatum();
		}
	}

CDatum CDatum::GetElementAt2DI (int iIndex1, int iIndex2) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetElementAt2DI(iIndex1, iIndex2);

		default:
			return CDatum();
		}
	}

CDatum CDatum::GetElementAt3DI (int iIndex1, int iIndex2, int iIndex3) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetElementAt3DI(iIndex1, iIndex2, iIndex3);

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
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return 0;

		case TYPE_STRING:
			return DecodeString(m_dwData).GetLength();

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetCount();

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::GetCount(m_dwData);

		default:
			return 1;
		}
	}

CDatum CDatum::GetElementsAtArray (CDatum dIndex) const

//	GetElementsAtArray
//
//	Returns an array of elements for each index in the given array.

	{
	if (!IsContainer())
		return CDatum();

	//	If the index contains a range, then we expand it to a full array of indices.

	dIndex = CAEONTensor::ExpandIndexRange(dIndex);

	//	NOTE: We always create an array preserving the element type.

	const IDatatype& Type = GetDatatype();
	CDatum dResult = CreateArrayAsTypeOfElement(Type.GetElementType());
	dResult.GrowToFit(dIndex.GetCount());

	//	Loop over the index.

	CAEONTypeSystem TypeSystem;
	for (int i = 0; i < dIndex.GetCount(); i++)
		dResult.Append(GetElementAt(TypeSystem, dIndex.GetElement(i)));

	return dResult;
	}

CDatum CDatum::GetElementsAtRange (CDatum dRange) const

//	GetElementsAtRange
//
//	Returns an array of elements for the given range.

	{
	if (!IsContainer())
		return CDatum();

	const IAEONRange* pRange = dRange.GetRangeInterface();
	if (!pRange)
		return CDatum();

	int iStart = pRange->GetStart();
	int iEnd = pRange->GetEnd();
	int iStep = pRange->GetStep();

	//	NOTE: We always create an array preserving the element type.

	const IDatatype& Type = GetDatatype();
	CDatum dResult = CreateArrayAsTypeOfElement(Type.GetElementType());
	if (iStep > 0)
		{
		for (int i = iStart; i <= iEnd; i += iStep)
			{
			int iIndex = CalcArrayIndex(i, GetCount());
			if (iIndex >= 0 && iIndex < GetCount())
				dResult.Append(GetElementAt(iIndex));
			}
		}
	else
		{
		for (int i = iStart; i >= iEnd; i += iStep)
			{
			int iIndex = CalcArrayIndex(i, GetCount());
			if (iIndex >= 0 && iIndex < GetCount())
				dResult.Append(GetElementAt(iIndex));
			}
		}

	return dResult;
	}

CString CDatum::GetKey (int iIndex) const

//	GetKey
//
//	Returns the key at the given index

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetKey(iIndex);

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::GetKey(m_dwData, iIndex);

		default:
			return NULL_STR;
		}
	}

CDatum CDatum::GetKeyEx (int iIndex) const

//	GetKeyEx
//
//	Returns a key.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetKeyEx(iIndex);

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::GetKey(m_dwData, iIndex);

		default:
			return CDatum();
		}
	}

TArray<int> CDatum::GetKeysInSortedOrder () const

//	GetKeysInSortedOrder
//
//	Returns an array of indexes that represent the keys in sorted order.

	{
	TSortMap<CString, int> ColMap;

	for (int i = 0; i < GetCount(); i++)
		ColMap.SetAt(strToLower(GetKey(i)), i);

	TArray<int> Result;
	Result.InsertEmpty(ColMap.GetCount());
	for (int i = 0; i < ColMap.GetCount(); i++)
		Result[i] = ColMap[i];

	return Result;
	}

CDatum CDatum::GetMethod (const CString &sMethod) const

//	GetMethod
//
//	Returns a function definition (or nil).

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return CAEONNilImpl::GetMethod(sMethod);

		case TYPE_STRING:
			return CAEONStringImpl::GetMethod(sMethod);
			
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetMethod(sMethod);

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::GetMethod(m_dwData, sMethod);

		default:
			return CDatum();
		}
	}

void* CDatum::GetMethodThis ()

//	GetMethodThis
//
//	Returns the this pointer for the object.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return NULL;

		case TYPE_STRING:
			return DecodeLPSTR(m_dwData);

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetMethodThis();

		case TYPE_ROW_REF:
			return &m_dwData;

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

	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			if (retiValue)
				*retiValue = 0;
			return typeInteger32;

		case TYPE_CONSTANTS:
			switch (m_dwData)
				{
				case VALUE_FALSE:
					if (retiValue)
						*retiValue = 0;
					return typeInteger32;

				case VALUE_TRUE:
					if (retiValue)
						*retiValue = 1;
					return typeInteger32;

				default:
					ASSERT(false);
					return typeNaN;
				}

		case TYPE_INT32:
			if (retiValue)
				*retiValue = DecodeInt32(m_dwData);
			return typeInteger32;

		case TYPE_ENUM:
			if (retiValue)
				*retiValue = DecodeEnumValue(m_dwData);
			return typeInteger32;

		case TYPE_STRING:
			{
			CDatum dNumberValue;
			if (!CDatum::CreateFromStringValue(*this, &dNumberValue)
					|| !dNumberValue.IsNumber())
				return typeNaN;

			if (retdConverted)
				*retdConverted = dNumberValue;

			return dNumberValue.GetNumberType(retiValue);
			}

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetNumberType(retiValue);

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
		case TYPE_ROW_REF:
			return typeNaN;

		default:
			if (retiValue)
				*retiValue = (int)DecodeDouble(m_dwData);
			return typeDouble;
		}
	}

CDatum CDatum::GetProperty (const CString &sKey) const

//	GetProperty
//
//	Gets the appropriate element/property

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return CAEONNilImpl::GetProperty(sKey);

		case TYPE_STRING:
			return CAEONStringImpl::GetProperty(DecodeString(m_dwData), sKey);

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetProperty(sKey);

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::GetProperty(m_dwData, sKey);

		default:
			return CDatum();
		}
	}

CDatum::Types CDatum::GetStringValueType (const CString &sValue, CString& retsClean)

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

	bool bStripComma = false;
	bool bStripUnderscore = false;

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
				else if (*pPos == ',')
					{
					bStripComma = true;
					iState = stateInteger;
					}
				else if (*pPos == '_')
					{
					bStripUnderscore = true;
					iState = stateInteger;
					}
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
				else if (*pPos == ',')
					bStripComma = true;
				else if (*pPos == '_')
					bStripUnderscore = true;
				else
					return typeString;

				break;
				}

			case stateInteger:
				{
				if (strIsDigit(pPos))
					NULL;
				else if (*pPos == ',')
					bStripComma = true;
				else if (*pPos == '_')
					bStripUnderscore = true;
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
				else if (*pPos == ',')
					bStripComma = true;
				else if (*pPos == '_')
					bStripUnderscore = true;
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

	//	Clean, if necessary

	if (bStripComma)
		{
		retsClean = sValue.StripChar(',');
		}

	if (bStripUnderscore)
		{
		if (retsClean.IsEmpty())
			retsClean = sValue.StripChar('_');
		else
			retsClean = retsClean.StripChar('_');
		}

	//	Return state

	switch (iState)
		{
		case stateStart:
			return typeNil;

		case stateHex:
			{
			CIPInteger Value;
			if (retsClean.IsEmpty())
				Value.InitFromString(sValue);
			else
				Value.InitFromString(retsClean);

			if (Value.FitsAsInteger32Signed())
				return typeInteger32;
			else
				return typeIntegerIP;
			}

		case stateHex0:
			return typeInteger32;

		case stateInteger:
			{
			if (retsClean.IsEmpty())
				{
				if (strOverflowsInteger32(sValue))
					return typeIntegerIP;
				else
					return typeInteger32;
				}
			else
				{
				if (strOverflowsInteger32(retsClean))
					return typeIntegerIP;
				else
					return typeInteger32;
				}
			}

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
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return TYPENAME_NIL;

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
					return TYPENAME_FALSE;

				case VALUE_TRUE:
					return TYPENAME_TRUE;

				default:
					ASSERT(false);
					return NULL_STR;
				}
			}

		case TYPE_INT32:
			return TYPENAME_INT32;

		case TYPE_ENUM:
			return TYPENAME_ENUM;

		case TYPE_STRING:
			return TYPENAME_STRING;

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetTypename();

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::GetTypename(m_dwData);

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
			//	NOTE: It is OK to interpret these as doubles.
			return TYPENAME_DOUBLE;

		default:
			return TYPENAME_DOUBLE;
		}
	}

void CDatum::InsertElementAt (CDatum dIndex, CDatum dValue)

//	InsertElementAt
//
//	Inserts an element at the given index.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).InsertElementAt(dIndex, dValue);
			break;

		default:
			//	Nothing happens
			break;
		}
	}

void CDatum::InsertEmpty (int iCount)

//	InsertEmpty
//
//	Inserts the given number of empty elements.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).InsertEmpty(iCount);
			break;

		default:
			//	Nothing happens
			break;
		}
	}

CDatum CDatum::IteratorBegin () const

//	IteratorBegin
//
//	Returns an iterator for this object at the start of the collection. The
//	iterator may be passed into GetElementAt and IteratorNext. The iterator may
//	be at the end (if the collection is empty). Test against nil.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return CDatum();

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).IteratorBegin();

		default:
			return CDatum(0);
		}
	}

CDatum CDatum::IteratorGetKey (CDatum dIterator) const

//	IteratorGetKey
//
//	Returns the key for the given iterator. If we are at the end of the collection

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).IteratorGetKey(dIterator);

		default:
			return ((int)dIterator == 0 ? CDatum(0) : CDatum());
		}
	}

CDatum CDatum::IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const

//	IteratorGetValue
//
//	Returns the value for the given iterator. If we are at the end of the collection

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).IteratorGetValue(TypeSystem, dIterator);

		default:
			return ((int)dIterator == 0 ? *this : CDatum());
		}
	}

CDatum CDatum::IteratorNext (CDatum dIterator) const

//	IteratorNext
//
//	Returns the next element in the collection. If we are at the end of the
//	collection then we return nil.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).IteratorNext(dIterator);

		default:
			return CDatum();
		}
	}

CDatum CDatum::Join (CStringView sSeparator, CStringView sLastSeparator, DWORD dwFlags) const

//	Join
//
//	Join an array into a string.

	{
	//	If not an array, then just return

	if (!IsArray())
		return AsString();

	//	Make a list of all values to join (taking into account whether we're 
	//	including nulls or not).

	TArray<CDatum> Values;
	Values.GrowToFit(GetCount());

	bool bSuccess = EnumElements(dwFlags, [&Values](CDatum dValue)
		{
		Values.Insert(dValue);
		return true;
		});

	//	Join them

	CStringBuffer Result;
	for (int i = 0; i < Values.GetCount(); i++)
		{
		if (i == 0)
			;
		else if (!sLastSeparator.IsEmpty() && i == Values.GetCount() - 1)
			Result.Write(sLastSeparator);
		else
			Result.Write(sSeparator);

		CDatum dElement = Values[i];
		if (dElement.GetBasicType() == CDatum::typeString)
			Result.Write(dElement.AsStringView());
		else
			Result.Write(dElement.AsString());
		}

	return CDatum(std::move(Result));
	}

bool CDatum::RemoveAll ()

//	RemoveAll
//
//	Removes all elements.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).RemoveAll();

		default:
			//	Nothing happens
			return false;
		}
	}

bool CDatum::RemoveElementAt (CDatum dIndex)

//	RemoveElementAt
//
//	Removes the element at the given index.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).RemoveElementAt(dIndex);

		default:
			//	Nothing happens
			return false;
		}
	}

CDatum::InvokeResult CDatum::Invoke (IInvokeCtx *pCtx, CHexeLocalEnvironment& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult)

//	Invoke
//
//	Invokes the function

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).Invoke(pCtx, LocalEnv, dwExecutionRights, retResult);

		default:
			retResult.iResult = InvokeResult::error;
			return retResult.iResult;
		}
	}

CDatum::InvokeResult CDatum::InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, SAEONInvokeResult& retResult)

//	InvokeContinues
//
//	Invokes the function

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).InvokeContinues(pCtx, dContext, dResult, retResult);

		default:
			retResult.iResult = InvokeResult::error;
			return retResult.iResult;
		}
	}

CDatum::InvokeResult CDatum::InvokeLibrary (IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult)

//	Invoke
//
//	Invokes the function

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).InvokeLibrary(Ctx, LocalEnv, dwExecutionRights, retResult);

		default:
			retResult.iResult = InvokeResult::error;
			return retResult.iResult;
		}
	}

bool CDatum::InvokeMethodImpl (const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult)

//	InvokeMethodImpl
//
//	Invokes a method.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).InvokeMethodImpl(*this, sMethod, Ctx, LocalEnv, retResult);

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::InvokeMethodImpl(m_dwData, sMethod, Ctx, LocalEnv, retResult);

		default:
			retResult.dResult = ERR_NO_METHODS;
			return false;
		}
	}

bool CDatum::IsArray () const

//	IsArray
//
//	Returns TRUE if we are an array type

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).IsArray();

		case TYPE_ROW_REF:
			return true;

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
		case typeFalse:
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
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).IsContainer();

		case TYPE_ROW_REF:
			return true;

		default:
			return false;
		}
	}

bool CDatum::IsEqualCompatible (CDatum dValue) const

//	IsEqualCompatible
//
//	Returns TRUE if the values are equal
//	NOTE: For GridLang, use OpIsEqual instead.

	{
	switch (GetBasicType())
		{
		case typeNil:
			return dValue.IsNil();

		case typeFalse:
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
			return (dValue.GetBasicType() == typeString && strEquals(CStringView(*this), CStringView(dValue)));

		case typeDateTime:
			return (dValue.GetBasicType() == typeDateTime && ((const CDateTime &)*this == (const CDateTime &)dValue));

		case typeTimeSpan:
			return (dValue.GetBasicType() == typeTimeSpan && ((const CTimeSpan &)*this == (const CTimeSpan &)dValue));

		case typeEnum:
			{
			if (dValue.GetBasicType() != typeEnum)
				return false;

			DWORD dwType1 = DecodeEnumType(m_dwData);
			DWORD dwType2 = DecodeEnumType(dValue.m_dwData);
			if (dwType1 != dwType2)
				return false;

			return ((int)*this == (int)dValue);
			}

		case typeDatatype:
			return (dValue.GetBasicType() == typeDatatype && strEquals(((const IDatatype &)*this).GetFullyQualifiedName(), ((const IDatatype &)dValue).GetFullyQualifiedName()));

		case CDatum::typeVector2D:
			return (dValue.GetBasicType() == typeVector2D && (const CVector2D&)*this == (const CVector2D&)dValue);

		case CDatum::typeVector3D:
			return (dValue.GetBasicType() == typeVector3D && (const CVector3D&)*this == (const CVector3D&)dValue);

		//	LATER
		case typeArray:
		case typeBinary:
		case typeImage32:
		case typeStruct:
		case typeSymbol:
		case typeRowRef:
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
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).IsError();

		default:
			return false;
		}
	}

bool CDatum::IsNil (void) const

//	IsNil
//
//	Returns TRUE if this is a nil value

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return true;

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
					return true;

				default:
					return false;
				}
			}

		case TYPE_STRING:
			return (DecodeString(m_dwData).IsEmpty());

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).IsNil();

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::IsNil(m_dwData);

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

bool CDatum::IsStruct () const

//	IsStruct
//
//	Returns TRUE if this datum is a struct-like type with keys and values.
//
//	NOTE: This should only be TRUE for values that can completely be described 
//	by key/value pairs. I.e., it does not include any type that merely has 
//	properties.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).IsStruct();

		case TYPE_ROW_REF:
			return true;

		default:
			return false;
		}
	}

void CDatum::Mark (void)

//	Mark
//
//	Marks the datum so that we know that we cannot free it

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_STRING:
			CAEONStore::MarkString(DecodeLPSTR(m_dwData));
			break;

		case TYPE_ENUM:
			CAEONTypes::Get(DecodeEnumType(m_dwData)).Mark();
			break;

		case TYPE_COMPLEX:
			CAEONStore::MarkComplex(raw_GetComplex());
			break;

		case TYPE_ROW_REF:
			CAEONRowRefImpl::Mark(m_dwData);
			break;

		default:
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

int CDatum::OpCompare (CDatum dValue) const

//	OpCompare
//
//  -2:		If dKey < dKey2 (but not comparable, e.g., string an int)
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2
//	2:		If dKey > dKey2 (but not comparable, e.g., string an int)

	{
	CDatum::Types iType1 = GetBasicType();
	CDatum::Types iType2 = dValue.GetBasicType();

	//	If both types are equal, then compare

	if (iType1 == iType2)
		{
		switch (iType1)
			{
			case CDatum::typeNil:
			case CDatum::typeFalse:
			case CDatum::typeTrue:
			case CDatum::typeNaN:
				return 0;

			case CDatum::typeInteger32:
				return KeyCompare((int)*this, (int)dValue);

			case CDatum::typeInteger64:
				return KeyCompare((DWORDLONG)*this, (DWORDLONG)dValue);

			case CDatum::typeDouble:
				return KeyCompare((double)*this, (double)dValue);

			case CDatum::typeIntegerIP:
				return KeyCompare((const CIPInteger &)*this, (const CIPInteger &)dValue);

			case CDatum::typeString:
				return KeyCompareNoCase(AsStringView(), dValue.AsStringView());

			case CDatum::typeDateTime:
				return KeyCompare((const CDateTime &)*this,  (const CDateTime &)dValue);

			case CDatum::typeTimeSpan:
				return KeyCompare((const CDateTime &)*this,  (const CDateTime &)dValue);

			case CDatum::typeEnum:
				{
				//	If both the symbol and the ordinal of the enum are equal, 
				//	then we return as equal.

				int iSymbolCompare = KeyCompareNoCase(AsString(), dValue.AsString());
				if (iSymbolCompare == 0)
					return KeyCompare((int)*this, (int)dValue);
				else
					return iSymbolCompare;
				}

			case CDatum::typeStruct:
				{
				int iCount = Min(GetCount(), dValue.GetCount());
				for (int i = 0; i < iCount; i++)
					{
					int iCompare = KeyCompareNoCase(GetKey(i), dValue.GetKey(i));
					if (iCompare != 0)
						return iCompare;

					iCompare = GetElement(i).OpCompare(dValue.GetElement(i));
					if (iCompare != 0)
						return iCompare;
					}

				return KeyCompare(GetCount(), dValue.GetCount());
				}

			case CDatum::typeRowRef:
				return CAEONRowRefImpl::OpCompare(m_dwData, dValue.m_dwData);

			case CDatum::typeDatatype:
				return KeyCompare(((const IDatatype &)*this).GetFullyQualifiedName(), ((const IDatatype &)dValue).GetFullyQualifiedName());

			case CDatum::typeBinary:
				return KeyCompare(AsStringView(), dValue.AsStringView());

			default:
				{
				auto pComplex = GetComplex();
				if (pComplex)
					return pComplex->OpCompare(iType2, dValue);
				else
					return KeyCompare(AsString(), dValue.AsString());
				}
			}
		}

	//	If Nan, then everything is greater, except nil.

	else if (iType1 == CDatum::typeNaN)
		{
		switch (iType2)
			{
			case CDatum::typeDouble:
				return (isnan((double)dValue) ? 0 : -1);
					
			default:
				return CompareByType(iType1, iType2);
			}
		}
	else if (iType2 == CDatum::typeNaN)
		{
		switch (iType1)
			{
			case CDatum::typeDouble:
				return (isnan((double)*this) ? 0 : 1);
					
			default:
				return CompareByType(iType1, iType2);
			}
		}

	//	If one of the types is a number, then compare as numbers

	else if (IsNumber() && dValue.IsNumber())
		{
		CNumberValue Number1(*this);
		CNumberValue Number2(dValue);

		//	If either number is invalid, then it counts as 0

		if (Number1.IsValidNumber()
				&& Number2.IsValidNumber())
			return Number1.Compare(Number2);
		else if (Number1.IsValidNumber())
			return 2 * Number1.Compare(CDatum(0));
		else if (Number2.IsValidNumber())
			return 2 * CNumberValue(CDatum(0)).Compare(Number2);
		else
			return 0;
		}

	//	If iType1 is nil then everything is greater than it, except nil.

	else if (iType1 == CDatum::typeNil)
		{
		switch (iType2)
			{
			case CDatum::typeString:
				return ((dValue.AsStringView()).IsEmpty() ? 0 : -1);

			case CDatum::typeArray:
			case CDatum::typeStruct:
				return (dValue.GetCount() == 0 ? 0 : -1);

			default:
				return -1;
			}
		}

	//	If iType2 is nil then everything is greater than it, except nil.

	else if (iType2 == CDatum::typeNil)
		{
		switch (iType1)
			{
			case CDatum::typeString:
				return (AsStringView().IsEmpty() ? 0 : 1);

			case CDatum::typeArray:
			case CDatum::typeStruct:
				return (GetCount() == 0 ? 0 : 1);

			default:
				return 1;
			}
		}

	//	Handle row references specially.

	else if (iType1 == CDatum::typeRowRef && dValue.IsStruct())
		{
		return CAEONRowRefImpl::OpCompare(m_dwData, dValue);
		}
	else if (iType2 == CDatum::typeRowRef && IsStruct())
		{
		return CAEONRowRefImpl::OpCompare(*this, dValue.m_dwData);
		}

	//	Otherwise, let the datum compare

	else
		{
		auto pComplex = GetComplex();
		if (pComplex)
			return pComplex->OpCompare(iType2, dValue);
		else
			return CompareByType(iType1, iType2);
		}
	}

int CDatum::OpCompareExact (CDatum dValue) const

//	OpCompareExact
//
//  -2:		If dKey < dKey2 (but not comparable, e.g., string an int)
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2
//	2:		If dKey > dKey2 (but not comparable, e.g., string an int)

	{
	CDatum::Types iType1 = GetBasicType();
	CDatum::Types iType2 = dValue.GetBasicType();

	//	If both types are equal, then compare

	if (iType1 == iType2)
		{
		switch (iType1)
			{
			case CDatum::typeNil:
			case CDatum::typeFalse:
			case CDatum::typeTrue:
			case CDatum::typeNaN:
				return 0;

			case CDatum::typeInteger32:
				return KeyCompare((int)*this, (int)dValue);

			case CDatum::typeInteger64:
				return KeyCompare((DWORDLONG)*this, (DWORDLONG)dValue);

			case CDatum::typeDouble:
				return KeyCompare((double)*this, (double)dValue);

			case CDatum::typeIntegerIP:
				return KeyCompare((const CIPInteger &)*this, (const CIPInteger &)dValue);

			case CDatum::typeString:
				return KeyCompare(AsStringView(), dValue.AsStringView());

			case CDatum::typeDateTime:
				return KeyCompare((const CDateTime &)*this,  (const CDateTime &)dValue);

			case CDatum::typeTimeSpan:
				return KeyCompare((const CDateTime &)*this,  (const CDateTime &)dValue);

			case CDatum::typeEnum:
				{
				if ((const IDatatype&)(GetDatatype()) == (const IDatatype&)(dValue.GetDatatype()))
					return KeyCompare((int)*this, (int)dValue);
				else
					return GetDatatype().OpCompareExact(dValue.GetDatatype());
				}

			case CDatum::typeStruct:
				{
				int iCount = Min(GetCount(), dValue.GetCount());
				for (int i = 0; i < iCount; i++)
					{
					int iCompare = KeyCompareNoCase(GetKey(i), dValue.GetKey(i));
					if (iCompare != 0)
						return iCompare;

					iCompare = GetElement(i).OpCompareExact(dValue.GetElement(i));
					if (iCompare != 0)
						return iCompare;
					}

				return KeyCompare(GetCount(), dValue.GetCount());
				}

			case CDatum::typeRowRef:
				return CAEONRowRefImpl::OpCompareExact(m_dwData, dValue.m_dwData);

			case CDatum::typeDatatype:
				return KeyCompare(((const IDatatype &)*this).GetFullyQualifiedName(), ((const IDatatype &)dValue).GetFullyQualifiedName());

			case CDatum::typeBinary:
				return KeyCompare(AsStringView(), dValue.AsStringView());

			default:
				{
				auto pComplex = GetComplex();
				if (pComplex)
					return pComplex->OpCompare(iType2, dValue);
				else
					return KeyCompare(AsString(), dValue.AsString());
				}
			}
		}

	//	If Nan, then everything is greater, except nil.

	else if (iType1 == CDatum::typeNaN)
		{
		switch (iType2)
			{
			case CDatum::typeDouble:
				return (isnan((double)dValue) ? 0 : -1);
					
			default:
				return CompareByType(iType1, iType2);
			}
		}
	else if (iType2 == CDatum::typeNaN)
		{
		switch (iType1)
			{
			case CDatum::typeDouble:
				return (isnan((double)*this) ? 0 : 1);
					
			default:
				return CompareByType(iType1, iType2);
			}
		}

	//	If one of the types is a number, then compare as numbers

	else if (IsNumber() && dValue.IsNumber())
		{
		CNumberValue Number1(*this);
		CNumberValue Number2(dValue);

		//	If either number is invalid, then it counts as 0

		if (Number1.IsValidNumber()
				&& Number2.IsValidNumber())
			return Number1.Compare(Number2);
		else if (Number1.IsValidNumber())
			return 2 * Number1.Compare(CDatum(0));
		else if (Number2.IsValidNumber())
			return 2 * CNumberValue(CDatum(0)).Compare(Number2);
		else
			return 0;
		}

	//	If iType1 is nil then everything is greater than it, except nil.

	else if (iType1 == CDatum::typeNil)
		{
		switch (iType2)
			{
			case CDatum::typeString:
				return (dValue.AsStringView().IsEmpty() ? 0 : -1);

			case CDatum::typeArray:
			case CDatum::typeStruct:
				return (dValue.GetCount() == 0 ? 0 : -1);

			default:
				return -1;
			}
		}

	//	If iType2 is nil then everything is greater than it, except nil.

	else if (iType2 == CDatum::typeNil)
		{
		switch (iType1)
			{
			case CDatum::typeString:
				return (AsStringView().IsEmpty() ? 0 : 1);

			case CDatum::typeArray:
			case CDatum::typeStruct:
				return (GetCount() == 0 ? 0 : 1);

			default:
				return 1;
			}
		}

	//	Handle row references specially.

	else if (iType1 == CDatum::typeRowRef && dValue.IsStruct())
		{
		return CAEONRowRefImpl::OpCompareExact(m_dwData, dValue);
		}
	else if (iType2 == CDatum::typeRowRef && IsStruct())
		{
		return CAEONRowRefImpl::OpCompareExact(*this, dValue.m_dwData);
		}

	//	Otherwise, let the datum compare

	else
		{
		auto pComplex = GetComplex();
		if (pComplex)
			return pComplex->OpCompare(iType2, dValue);
		else
			return CompareByType(iType1, iType2);
		}
	}

CDatum CDatum::OpConcatenated (IInvokeCtx& Ctx, CDatum dSrc, int iAxis) const

//	OpConcatenated
//
//	Concatenates two arrays.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).OpConcatenated(Ctx, dSrc, iAxis);

		case TYPE_ROW_REF:
			return CDatum().OpConcatenated(Ctx, dSrc, iAxis);

		default:
			return CreateError(ERR_NOT_SUPPORTED);
		}
	}

bool CDatum::OpContains (CDatum dValue) const

//	OpContains
//
//	Returns TRUE if the value is contained in this datum.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_STRING:
			{
			switch (dValue.GetBasicType())
				{
				case CDatum::typeNil:
				case CDatum::typeNaN:
					return false;

				case CDatum::typeString:
					return strFindNoCase(DecodeString(m_dwData), dValue.AsStringView()) != -1;

				default:
					return false;
				}
			}

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).OpContains(dValue);

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::OpContains(m_dwData, dValue);

		default:
			return false;
		}
	}

bool CDatum::OpIsEqual (CDatum dValue) const

//	OpIsEqual
//
//	Returns TRUE if the two values are equivalent.

	{
	CDatum::Types iType1 = GetBasicType();
	CDatum::Types iType2 = dValue.GetBasicType();

	//	If both types are equal, then compare

	if (iType1 == iType2)
		{
		switch (iType1)
			{
			case CDatum::typeNil:
			case CDatum::typeFalse:
			case CDatum::typeTrue:
			case CDatum::typeNaN:
				return true;

			case CDatum::typeInteger32:
				return (int)*this == (int)dValue;

			case CDatum::typeInteger64:
				return (DWORDLONG)*this == (DWORDLONG)dValue;

			case CDatum::typeDouble:
				return (double)*this == (double)dValue;

			case CDatum::typeIntegerIP:
				return ((const CIPInteger &)*this) == ((const CIPInteger &)dValue);

			case CDatum::typeString:
				return strEqualsNoCase(AsStringView(), dValue.AsStringView());

			case CDatum::typeDateTime:
				return ((const CDateTime &)*this) == ((const CDateTime &)dValue);

			case CDatum::typeTimeSpan:
				return ((const CTimeSpan &)*this) == ((const CTimeSpan &)dValue);

			case CDatum::typeEnum:
				//	If both the symbol and the ordinal of the enum are equal, 
				//	then we return as equivalent (even if the datatypes don't match)

				return ((int)*this == (int)dValue
						&& strEqualsNoCase(AsString(), dValue.AsString()));

			case CDatum::typeStruct:
				{
				if (GetCount() != dValue.GetCount())
					return false;

				for (int i = 0; i < GetCount(); i++)
					if (!strEqualsNoCase(GetKey(i), dValue.GetKey(i))
							|| !GetElement(i).OpIsEqual(dValue.GetElement(i)))
						return false;

				return true;
				}

			case CDatum::typeRowRef:
				return CAEONRowRefImpl::OpIsEqual(m_dwData, dValue.m_dwData);

			case CDatum::typeTable:
				{
				CDatum dSchema1 = GetDatatype();
				CDatum dSchema2 = dValue.GetDatatype();
				if (!dSchema1.OpIsEqual(dSchema2))
					return false;

				const IAEONTable *pTable1 = GetTableInterface();
				const IAEONTable *pTable2 = dValue.GetTableInterface();
				if (pTable1 == NULL || pTable2 == NULL)
					return false;

				for (int i = 0; i < pTable1->GetColCount(); i++)
					{
					CDatum dCol1 = pTable1->GetCol(i);
					CDatum dCol2 = pTable2->GetCol(i);

					if (!dCol1.OpIsEqual(dCol2))
						return false;
					}

				return true;
				}

			case CDatum::typeDatatype:
				return (const IDatatype &)*this == (const IDatatype &)dValue;

			case CDatum::typeImage32:
				return (const CRGBA32Image&)*this == (const CRGBA32Image&)dValue;

			case CDatum::typeVector2D:
				return (const CVector2D&)*this == (const CVector2D&)dValue;

			case CDatum::typeVector3D:
				return (const CVector3D&)*this == (const CVector3D&)dValue;

			case CDatum::typeTextLines:
				{
				const IAEONTextLines *pLines1 = GetTextLinesInterface();
				if (!pLines1)
					return false;

				const IAEONTextLines *pLines2 = dValue.GetTextLinesInterface();
				if (!pLines2)
					return false;

				return (pLines1->CompareNoCase(*pLines2) == 0);
				}

			case CDatum::typeBinary:
				return strEquals(AsStringView(), dValue.AsStringView());

			default:
				{
				auto pComplex = GetComplex();
				if (pComplex)
					return pComplex->OpIsEqual(iType2, dValue);
				else
					return false;
				}
			}
		}

	//	If one of the types is nil, then compare

	else if (iType1 == CDatum::typeNil || iType2 == CDatum::typeNil)
		{
		CDatum dValue1 = *this;
		CDatum dValue2 = dValue;

		if (iType2 == CDatum::typeNil)
			{
			Swap(dValue1, dValue2);
			Swap(iType1, iType2);
			}

		switch (iType2)
			{
			case CDatum::typeFalse:
				return false;

			case CDatum::typeString:
				return dValue2.AsStringView().IsEmpty();

			case CDatum::typeArray:
			case CDatum::typeStruct:
				return dValue2.GetCount() == 0;

			case CDatum::typeVector2D:
				return (const CVector2D&)dValue2 == CVector2D::Null;

			case CDatum::typeVector3D:
				return (const CVector3D&)dValue2 == CVector3D::Null;

			default:
				return dValue2.IsNil();
			}
		}

	//	NaN is also special.

	else if (iType1 == CDatum::typeNaN && iType2 == CDatum::typeDouble)
		{
		return isnan((double)dValue);
		}
	else if (iType1 == CDatum::typeDouble && iType2 == CDatum::typeNaN)
		{
		return isnan((double)*this);
		}

	//	True and false are not equal to anything else, even though they can
	//	be coerced to numbers.

	else if (iType1 == CDatum::typeTrue || iType1 == CDatum::typeFalse
			|| iType2 == CDatum::typeTrue || iType2 == CDatum::typeFalse)
		return false;

	//	If one of the types is a number, then compare as numbers

	else if (IsNumber() && dValue.IsNumber())
		{
		CNumberValue Number1(*this);
		CNumberValue Number2(dValue);

		return (Number1.IsValidNumber()
				&& Number2.IsValidNumber()
				&& Number1.Compare(Number2) == 0);
		}

	//	If one of the types is a string, then compare as strings.

	else if (iType1 == CDatum::typeString)
		{
		switch (iType2)
			{
			case CDatum::typeTextLines:
				return strEqualsNoCase(AsStringView(), dValue.AsString());

			default:
				{
				auto pComplex = dValue.GetComplex();
				if (pComplex)
					return pComplex->OpIsEqual(iType1, *this);
				else
					return false;
				}
			}
		}

	else if (iType2 == CDatum::typeString)
		{
		switch (iType1)
			{
			case CDatum::typeTextLines:
				return strEqualsNoCase(AsString(), dValue.AsStringView());

			default:
				{
				auto pComplex = GetComplex();
				if (pComplex)
					return pComplex->OpIsEqual(iType2, dValue);
				else
					return false;
				}
			}
		}

	//	Handle row references specially.

	else if (iType1 == CDatum::typeRowRef && dValue.IsStruct())
		{
		return CAEONRowRefImpl::OpIsEqual(m_dwData, dValue);
		}
	else if (iType2 == CDatum::typeRowRef && IsStruct())
		{
		return CAEONRowRefImpl::OpIsEqual(*this, dValue.m_dwData);
		}

	//	Otherwise, let the datum compare

	else
		{
		auto pComplex = GetComplex();
		if (pComplex)
			return pComplex->OpIsEqual(iType2, dValue);
		else
			{
			pComplex = dValue.GetComplex();
			if (pComplex)
				return pComplex->OpIsEqual(iType1, *this);
			else
				return false;
			}
		}
	}

bool CDatum::OpIsIdentical (CDatum dValue) const

//	OpIsIdentical
//
//	Returns TRUE if the two values are identical.

	{
	CDatum::Types iType1 = GetBasicType();
	CDatum::Types iType2 = dValue.GetBasicType();

	if (iType1 == iType2)
		{
		switch (iType1)
			{
			case CDatum::typeNil:
			case CDatum::typeFalse:
			case CDatum::typeTrue:
			case CDatum::typeNaN:
				return true;

			case CDatum::typeInteger32:
				return (int)*this == (int)dValue;

			case CDatum::typeInteger64:
				return (DWORDLONG)*this == (DWORDLONG)dValue;

			case CDatum::typeDouble:
				return (double)*this == (double)dValue;

			case CDatum::typeIntegerIP:
				return ((const CIPInteger &)*this) == ((const CIPInteger &)dValue);

			case CDatum::typeString:
				//	Case-sensitive compare
				return strEquals(AsStringView(), dValue.AsStringView());

			case CDatum::typeDateTime:
				return ((const CDateTime &)*this) == ((const CDateTime &)dValue);

			case CDatum::typeTimeSpan:
				return ((const CTimeSpan &)*this) == ((const CTimeSpan &)dValue);

			case CDatum::typeEnum:
				//	For it to be identical, we need both values to be the same
				//	ordinal and datatype.

				return ((const IDatatype&)(GetDatatype()) == (const IDatatype&)(dValue.GetDatatype()))
						&& ((int)*this == (int)dValue);

			case CDatum::typeStruct:
				{
				if (GetCount() != dValue.GetCount())
					return false;

				for (int i = 0; i < GetCount(); i++)
					if (!strEqualsNoCase(GetKey(i), dValue.GetKey(i))
							|| !GetElement(i).OpIsIdentical(dValue.GetElement(i)))
						return false;

				return true;
				}

			case CDatum::typeRowRef:
				return CAEONRowRefImpl::OpIsIdentical(m_dwData, dValue.m_dwData);

			case CDatum::typeTable:
				{
				CDatum dSchema1 = GetDatatype();
				CDatum dSchema2 = dValue.GetDatatype();
				if (!dSchema1.OpIsIdentical(dSchema2))
					return false;

				const IAEONTable *pTable1 = GetTableInterface();
				const IAEONTable *pTable2 = dValue.GetTableInterface();
				if (pTable1 == NULL || pTable2 == NULL)
					return false;

				for (int i = 0; i < pTable1->GetColCount(); i++)
					{
					CDatum dCol1 = pTable1->GetCol(i);
					CDatum dCol2 = pTable2->GetCol(i);

					if (!dCol1.OpIsIdentical(dCol2))
						return false;
					}

				return true;
				}

			case CDatum::typeDatatype:
				return (const IDatatype &)*this == (const IDatatype &)dValue;

			case CDatum::typeImage32:
				return (const CRGBA32Image&)*this == (const CRGBA32Image&)dValue;

			case CDatum::typeVector2D:
				return (const CVector2D&)*this == (const CVector2D&)dValue;

			case CDatum::typeVector3D:
				return (const CVector3D&)*this == (const CVector3D&)dValue;

			case CDatum::typeTextLines:
				{
				const IAEONTextLines *pLines1 = GetTextLinesInterface();
				if (!pLines1)
					return false;

				const IAEONTextLines *pLines2 = dValue.GetTextLinesInterface();
				if (!pLines2)
					return false;

				return (pLines1->Compare(*pLines2) == 0);
				}

			case CDatum::typeBinary:
				return strEquals(AsStringView(), dValue.AsStringView());

			default:
				{
				auto pComplex = GetComplex();
				if (pComplex)
					return pComplex->OpIsIdentical(iType2, dValue);
				else
					return false;
				}
			}
		}
	//	Handle row references specially.

	else if (iType1 == CDatum::typeRowRef && dValue.IsStruct())
		{
		return CAEONRowRefImpl::OpIsIdentical(m_dwData, dValue);
		}
	else if (iType2 == CDatum::typeRowRef && IsStruct())
		{
		return CAEONRowRefImpl::OpIsIdentical(*this, dValue.m_dwData);
		}

	else
		{
		switch (iType1)
			{
			case CDatum::typeInteger32:
				{
				switch (iType2)
					{
					case CDatum::typeInteger64:
						{
						int iValue1 = *this;
						DWORDLONG dwValue2 = dValue;
						return (iValue1 >= 0 && (DWORDLONG)iValue1 == dwValue2);
						}

					case CDatum::typeIntegerIP:
						{
						CIPInteger Value1((int)*this);
						return Value1 == (const CIPInteger &)dValue;
						}

					default:
						return false;
					}
				}

			case CDatum::typeInteger64:
				{
				switch (iType2)
					{
					case CDatum::typeInteger32:
						{
						int iValue2 = dValue;
						DWORDLONG dwValue1 = *this;
						return (iValue2 >= 0 && (DWORDLONG)iValue2 == dwValue1);
						}

					case CDatum::typeIntegerIP:
						{
						CIPInteger Value1((DWORDLONG)*this);
						return Value1 == (const CIPInteger &)dValue;
						}

					default:
						return false;
					}
				}

			case CDatum::typeIntegerIP:
				{
				switch (iType2)
					{
					case CDatum::typeInteger32:
						{
						CIPInteger Value2((int)dValue);
						return Value2 == (const CIPInteger &)*this;
						}

					case CDatum::typeInteger64:
						{
						CIPInteger Value2((DWORDLONG)dValue);
						return Value2 == (const CIPInteger &)*this;
						}

					default:
						return false;
					}
				}

			case CDatum::typeDouble:
				{
				switch (iType2)
					{
					case CDatum::typeNaN:
						return isnan((double)*this);

					default:
						return false;
					}
				}

			case CDatum::typeNaN:
				{
				switch (iType2)
					{
					case CDatum::typeDouble:
						return isnan((double)dValue);

					default:
						return false;
					}
				}

			default:
				{
				auto pComplex = GetComplex();
				if (pComplex)
					return pComplex->OpIsIdentical(iType2, dValue);
				else
					return false;
				}
			}
		}
	}

CDatum CDatum::raw_IteratorGetElement (CBuffer& Iterator) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).raw_IteratorGetElement(Iterator);

		default:
			return CDatum();
		}
	}

CDatum CDatum::raw_IteratorGetKey (CBuffer& Iterator) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).raw_IteratorGetKey(Iterator);

		default:
			return CDatum();
		}
	}

bool CDatum::raw_IteratorHasMore (CBuffer& Iterator) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).raw_IteratorHasMore(Iterator);

		default:
			return false;
		}
	}

void CDatum::raw_IteratorNext (CBuffer& Iterator) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).raw_IteratorNext(Iterator);
			break;

		default:
			break;
		}
	}

void CDatum::raw_IteratorSetElement (CBuffer& Iterator, CDatum dValue)
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).raw_IteratorSetElement(Iterator, dValue);
			break;

		default:
			break;
		}
	}

CBuffer CDatum::raw_IteratorStart () const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).raw_IteratorStart();

		default:
			return CBuffer();
		}
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
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).ResolveDatatypes(TypeSystem);
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
		case EFormat::AEONBinary:
		case EFormat::AEONBinaryLocal:
			{
			//	Write the signature

			Stream.Write(STR_B001);

			//	Now write the rest.

			CAEONSerializedMap Serialized;
			if (iFormat == EFormat::AEONBinaryLocal)
				Serialized.SetLocal();

			SerializeAEON(Stream, Serialized);
			break;
			}

		case EFormat::AEONScript:
		case EFormat::AEONLocal:
			SerializeAEONScript(iFormat, Stream);
			break;

		case EFormat::GridLang:
			SerializeGridLang(Stream);
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
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).SetElement(pCtx, sKey, dValue);
			break;

		case TYPE_ROW_REF:
			CAEONRowRefImpl::SetElement(m_dwData, pCtx, sKey, dValue);
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
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).SetElement(sKey, dValue);
			break;

		case TYPE_ROW_REF:
			CAEONRowRefImpl::SetElement(m_dwData, NULL, sKey, dValue);
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
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).SetElement(iIndex, dValue);
			break;

		case TYPE_ROW_REF:
			CAEONRowRefImpl::SetElement(m_dwData, iIndex, dValue);
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
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).SetElementAt(dIndex, dValue);
			break;

		case TYPE_ROW_REF:
			CAEONRowRefImpl::SetElementAt(m_dwData, dIndex, dValue);
			break;

		default:
			//	Nothing happens
			break;
		}
	}

void CDatum::SetElementAt2DA (CDatum dIndex1, CDatum dIndex2, CDatum dValue)
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).SetElementAt2DA(dIndex1, dIndex2, dValue);
			break;

		default:
			//	Nothing happens
			break;
		}
	}

void CDatum::SetElementAt2DI (int iIndex1, int iIndex2, CDatum dValue)
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).SetElementAt2DI(iIndex1, iIndex2, dValue);
			break;

		default:
			//	Nothing happens
			break;
		}
	}

void CDatum::SetElementAt3DA (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3, CDatum dValue)
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).SetElementAt3DA(dIndex1, dIndex2, dIndex3, dValue);
			break;

		default:
			//	Nothing happens
			break;
		}
	}

void CDatum::SetElementAt3DI (int iIndex1, int iIndex2, int iIndex3, CDatum dValue)
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).SetElementAt3DI(iIndex1, iIndex2, iIndex3, dValue);
			break;

		default:
			//	Nothing happens
			break;
		}
	}

void CDatum::SetElementsAtArray (CDatum dIndex, CDatum dValue)
	{
	if (!IsContainer())
		return;

	//	If the index contains a range, then we expand it to a full array of indices.

	dIndex = CAEONTensor::ExpandIndexRange(dIndex);

	//	Loop over each index and set an element in the value.

	int iCount = dValue.GetCount();
	for (int i = 0; i < dIndex.GetCount(); i++)
		{
		CDatum dElement = dValue.GetElement(i % iCount);
		SetElementAt(dIndex.GetElement(i), dElement);
		}
	}

void CDatum::SetElementsAtRange (CDatum dRange, CDatum dValue)
	{
	if (!IsContainer())
		return;

	const IAEONRange* pRange = dRange.GetRangeInterface();
	if (!pRange)
		return;

	int iStart = pRange->GetStart();
	int iEnd = pRange->GetEnd();
	int iStep = pRange->GetStep();

	int iCount = dValue.GetCount();
	int iSrc = 0;

	if (iStep > 0)
		{
		for (int i = iStart; i <= iEnd; i += iStep, iSrc++)
			{
			int iIndex = CalcArrayIndex(i, GetCount());
			if (iIndex >= 0 && iIndex < GetCount())
				SetElementAt(iIndex, dValue.GetElement(iSrc % iCount));
			}
		}
	else
		{
		for (int i = iStart; i >= iEnd; i += iStep, iSrc++)
			{
			int iIndex = CalcArrayIndex(i, GetCount());
			if (iIndex >= 0 && iIndex < GetCount())
				SetElementAt(iIndex, dValue.GetElement(iSrc % iCount));
			}
		}
	}

void CDatum::SetMethodsExt (EMethodExt iType, TDatumMethodHandler<IComplexDatum> &MethodsExt)

//	SetMethodsExt
//
//	Sets an extension block for methods.

	{
	switch (iType)
		{
		case EMethodExt::Array:
			CComplexArray::SetMethodsExt(MethodsExt);
			CAEONLines::SetMethodsExt(MethodsExt);
			CAEONVectorFloat64::SetMethodsExt(MethodsExt);
			CAEONVectorInt32::SetMethodsExt(MethodsExt);
			CAEONVectorIntIP::SetMethodsExt(MethodsExt);
			CAEONVectorNumber::SetMethodsExt(MethodsExt);
			CAEONVectorString::SetMethodsExt(MethodsExt);
			CAEONVectorTyped::SetMethodsExt(MethodsExt);
			break;

		case EMethodExt::DateTime:
			CComplexDateTime::SetMethodsExt(MethodsExt);
			break;

		case EMethodExt::Dictionary:
			CAEONDictionary::SetMethodsExt(MethodsExt);
			break;

		case EMethodExt::Struct:
			CComplexStruct::SetMethodsExt(MethodsExt);
			break;

		case EMethodExt::Table:
			CAEONTable::SetMethodsExt(MethodsExt);
			CAEONTableRef::SetMethodsExt(MethodsExt);
			break;

		case EMethodExt::Tensor:
			CAEONTensor::SetMethodsExt(MethodsExt);
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

	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			DecodeComplex(m_dwData).Sort(Order, pfCompare, pCtx);
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
