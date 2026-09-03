//	CAEONTypes.cpp
//
//	CAEONTypes class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_CAEONTYPES_ID,	"ID");
DECLARE_CONST_STRING(STR_CAEONTYPES_NAME,	"Name");
DECLARE_CONST_STRING(STR_CAEONTYPES_UTF8,	"UTF8");
DECLARE_CONST_STRING(STR_CAEONTYPES_UTF16_BE,	"UTF16BE");
DECLARE_CONST_STRING(STR_CAEONTYPES_UTF16_LE,	"UTF16LE");
DECLARE_CONST_STRING(STR_CAEONTYPES_UTF32_BE,	"UTF32BE");
DECLARE_CONST_STRING(STR_CAEONTYPES_UTF32_LE,	"UTF32LE");
DECLARE_CONST_STRING(STR_CAEONTYPES_ASCII,	"ASCII");
DECLARE_CONST_STRING(STR_CAEONTYPES_LATIN1,	"Latin1");
DECLARE_CONST_STRING(STR_CAEONTYPES_WINDOWS_1252,	"Windows1252");
DECLARE_CONST_STRING(STR_CAEONTYPES_URL_COMPONENT,	"urlComponent");
DECLARE_CONST_STRING(STR_CAEONTYPES_URL_PATH_SEGMENT,	"urlPathSegment");
DECLARE_CONST_STRING(STR_CAEONTYPES_URL_QUERY_VALUE,	"urlQueryValue");
DECLARE_CONST_STRING(STR_CAEONTYPES_FORM_COMPONENT,	"formComponent");
DECLARE_CONST_STRING(STR_CAEONTYPES_JSON_STRING_LITERAL,	"jsonStringLiteral");
DECLARE_CONST_STRING(STR_CAEONTYPES_JSON_STRING_CONTENT,	"jsonStringContent");
DECLARE_CONST_STRING(STR_CAEONTYPES_C_STRING_LITERAL,	"cStringLiteral");
DECLARE_CONST_STRING(STR_CAEONTYPES_C_STRING_CONTENT,	"cStringContent");
DECLARE_CONST_STRING(STR_CAEONTYPES_HTML_TEXT,	"htmlText");
DECLARE_CONST_STRING(STR_CAEONTYPES_HTML_ATTRIBUTE,	"htmlAttribute");
DECLARE_CONST_STRING(STR_CAEONTYPES_BASE64,	"base64");
DECLARE_CONST_STRING(STR_CAEONTYPES_BASE64_URL,	"base64URL");
DECLARE_CONST_STRING(STR_CAEONTYPES_HEX,	"hex");

DECLARE_CONST_STRING(DATUM_TYPENAME_BINARY,				"binary");
DECLARE_CONST_STRING(DATUM_TYPENAME_DATATYPE,			"datatype")
DECLARE_CONST_STRING(DATUM_TYPENAME_GRID_NAME,			"gridName");
DECLARE_CONST_STRING(DATUM_TYPENAME_MAP_COLUMN_EXPRESSION,"mapColumnExpression");
DECLARE_CONST_STRING(DATUM_TYPENAME_RANGE,				"range");
DECLARE_CONST_STRING(DATUM_TYPENAME_STRING_FORMAT,		"stringFormat");
DECLARE_CONST_STRING(DATUM_TYPENAME_VECTOR_2D,			"vector2D");
DECLARE_CONST_STRING(DATUM_TYPENAME_VECTOR_3D,			"vector3D");

DECLARE_CONST_STRING(ENUM_DEFINITION,					"definition");
DECLARE_CONST_STRING(ENUM_DEFINITION_LABEL,				"Definition");
DECLARE_CONST_STRING(ENUM_EVENT,						"event");
DECLARE_CONST_STRING(ENUM_EVENT_LABEL,					"Event");
DECLARE_CONST_STRING(ENUM_FUNCTION,						"function");
DECLARE_CONST_STRING(ENUM_FUNCTION_LABEL,				"Function");
DECLARE_CONST_STRING(ENUM_PROPERTY,						"property");
DECLARE_CONST_STRING(ENUM_PROPERTY_LABEL,				"Property");
DECLARE_CONST_STRING(ENUM_VARIABLE,						"variable");
DECLARE_CONST_STRING(ENUM_VARIABLE_LABEL,				"Variable");

DECLARE_CONST_STRING(ENUM_MONDAY,						"monday");
DECLARE_CONST_STRING(ENUM_MONDAY_LABEL,					"Monday");
DECLARE_CONST_STRING(ENUM_TUESDAY,						"tuesday");
DECLARE_CONST_STRING(ENUM_TUESDAY_LABEL,				"Tuesday");
DECLARE_CONST_STRING(ENUM_WEDNESDAY,					"wednesday");
DECLARE_CONST_STRING(ENUM_WEDNESDAY_LABEL,				"Wednesday");
DECLARE_CONST_STRING(ENUM_THURSDAY,						"thursday");
DECLARE_CONST_STRING(ENUM_THURSDAY_LABEL,				"Thursday");
DECLARE_CONST_STRING(ENUM_FRIDAY,						"friday");
DECLARE_CONST_STRING(ENUM_FRIDAY_LABEL,					"Friday");
DECLARE_CONST_STRING(ENUM_SATURDAY,						"saturday");
DECLARE_CONST_STRING(ENUM_SATURDAY_LABEL,				"Saturday");
DECLARE_CONST_STRING(ENUM_SUNDAY,						"sunday");
DECLARE_CONST_STRING(ENUM_SUNDAY_LABEL,					"Sunday");

DECLARE_CONST_STRING(FIELD_DATATYPE,					"datatype");
DECLARE_CONST_STRING(FIELD_DESCRIPTION,					"description");
DECLARE_CONST_STRING(FIELD_FORMAT,						"format");
DECLARE_CONST_STRING(FIELD_ID,							"id");
DECLARE_CONST_STRING(FIELD_KEY,							"key");
DECLARE_CONST_STRING(FIELD_LABEL,						"label");
DECLARE_CONST_STRING(FIELD_MEMBER_TYPE,					"memberType");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_ORDINAL,						"ordinal");
DECLARE_CONST_STRING(FIELD_TYPE,						"type");
DECLARE_CONST_STRING(FIELD_UI,							"ui");

DECLARE_CONST_STRING(TYPENAME_ABSTRACT_DICTIONARY,		"AbstractDictionaryType");
DECLARE_CONST_STRING(TYPENAME_ABSTRACT_MUTABLE_DICTIONARY,		"AbstractMutableDictionaryType");
DECLARE_CONST_STRING(TYPENAME_ANY,						"Any");
DECLARE_CONST_STRING(TYPENAME_ARRAY,					"Array");
DECLARE_CONST_STRING(TYPENAME_ARRAY_DATE_TIME,			"ArrayOfDateTime");
DECLARE_CONST_STRING(TYPENAME_ARRAY_FLOAT_64,			"ArrayOfFloat64");
DECLARE_CONST_STRING(TYPENAME_ARRAY_INT_32,				"ArrayOfInt32");
DECLARE_CONST_STRING(TYPENAME_ARRAY_INT_64,				"ArrayOfInt64");
DECLARE_CONST_STRING(TYPENAME_ARRAY_INT_IP,				"ArrayOfIntIP");
DECLARE_CONST_STRING(TYPENAME_ARRAY_NUMBER,				"ArrayOfNumber");
DECLARE_CONST_STRING(TYPENAME_ARRAY_STRING,				"ArrayOfString");
DECLARE_CONST_STRING(TYPENAME_ARRAY_VECTOR_2D,			"ArrayOfVector2D");
DECLARE_CONST_STRING(TYPENAME_ARRAY_VECTOR_3D,			"ArrayOfVector3D");
DECLARE_CONST_STRING(TYPENAME_BINARY,					"Binary");
DECLARE_CONST_STRING(TYPENAME_BITMAP_RGBA8,				"BitmapOfRGBA8");
DECLARE_CONST_STRING(TYPENAME_BOOL,						"Bool");
DECLARE_CONST_STRING(TYPENAME_CANVAS,					"Canvas");
DECLARE_CONST_STRING(TYPENAME_CHAR_SET_TYPE,			"CharSetType");
DECLARE_CONST_STRING(TYPENAME_CLASS_T,					"ClassType");
DECLARE_CONST_STRING(TYPENAME_DATATYPE,					"Datatype");
DECLARE_CONST_STRING(TYPENAME_DATE_TIME,				"DateTime");
DECLARE_CONST_STRING(TYPENAME_DAY_OF_WEEK_ENUM,			"DayOfWeek");
DECLARE_CONST_STRING(TYPENAME_DICTIONARY,				"Dictionary");
DECLARE_CONST_STRING(TYPENAME_ENCODING_TYPE,			"EncodingType");
DECLARE_CONST_STRING(TYPENAME_ENUM,						"Enum");
DECLARE_CONST_STRING(TYPENAME_ERROR,					"ErrorType");
DECLARE_CONST_STRING(TYPENAME_EXPRESSION,				"ExpressionType");
DECLARE_CONST_STRING(TYPENAME_FLOAT,					"Float");
DECLARE_CONST_STRING(TYPENAME_FLOAT_64,					"Float64");
DECLARE_CONST_STRING(TYPENAME_FUNCTION,					"FunctionType");
DECLARE_CONST_STRING(TYPENAME_GRID_NAME_TYPE,			"GridNameType");
DECLARE_CONST_STRING(TYPENAME_INDEXED,					"IndexedType");
DECLARE_CONST_STRING(TYPENAME_INT_8,					"Int8");
DECLARE_CONST_STRING(TYPENAME_INT_16,					"Int16");
DECLARE_CONST_STRING(TYPENAME_INT_32,					"Int32");
DECLARE_CONST_STRING(TYPENAME_INT_64,					"Int64");
DECLARE_CONST_STRING(TYPENAME_INT_IP,					"IntIP");
DECLARE_CONST_STRING(TYPENAME_INTEGER,					"Integer");
DECLARE_CONST_STRING(TYPENAME_MAP_COLUMN_EXPRESSION,	"MapColumnExpressionType");
DECLARE_CONST_STRING(TYPENAME_MATRIX_F64,				"MatrixOfFloat64");
DECLARE_CONST_STRING(TYPENAME_MATRIX_3X3_F64,			"Matrix3X3OfFloat64");
DECLARE_CONST_STRING(TYPENAME_MATRIX_4X4_F64,			"Matrix4X4OfFloat64");
DECLARE_CONST_STRING(TYPENAME_MEMBER_TABLE,				"MemberTableType");
DECLARE_CONST_STRING(TYPENAME_MEMBER_TABLE_SCHEMA,		"MemberTableSchema");
DECLARE_CONST_STRING(TYPENAME_MEMBER_TYPE_ENUM,			"MemberType");
DECLARE_CONST_STRING(TYPENAME_MUTABLE_DICTIONARY,		"MutableDictionaryType");
DECLARE_CONST_STRING(TYPENAME_MUTABLE_INDEXED,			"MutableIndexedType");
DECLARE_CONST_STRING(TYPENAME_NAN,						"NaNType");
DECLARE_CONST_STRING(TYPENAME_NEVER,					"Never");
DECLARE_CONST_STRING(TYPENAME_NULL,						"NullType");
DECLARE_CONST_STRING(TYPENAME_NUMBER,					"Number");
DECLARE_CONST_STRING(TYPENAME_OBJECT,					"Object");
DECLARE_CONST_STRING(TYPENAME_RANGE,					"RangeType");
DECLARE_CONST_STRING(TYPENAME_REAL,						"Real");
DECLARE_CONST_STRING(TYPENAME_SAS_DATE,					"SASDate");
DECLARE_CONST_STRING(TYPENAME_SAS_DATE_TIME,			"SASDateTime");
DECLARE_CONST_STRING(TYPENAME_SAS_TIME,					"SASTime");
DECLARE_CONST_STRING(TYPENAME_SCHEMA,					"SchemaType");
DECLARE_CONST_STRING(TYPENAME_SCHEMA_TABLE,				"SchemaTable");
DECLARE_CONST_STRING(TYPENAME_SCHEMA_TABLE_SCHEMA,		"SchemaTableSchema");
DECLARE_CONST_STRING(TYPENAME_SIGNED,					"Signed");
DECLARE_CONST_STRING(TYPENAME_STRING,					"String");
DECLARE_CONST_STRING(TYPENAME_STRING_FORMAT_TYPE,		"StringFormatType");
DECLARE_CONST_STRING(TYPENAME_STRUCT,					"Struct");
DECLARE_CONST_STRING(TYPENAME_TABLE,					"Table");
DECLARE_CONST_STRING(TYPENAME_TEXT_LINES,				"TextLines");
DECLARE_CONST_STRING(TYPENAME_TIME_SPAN,				"TimeSpan");
DECLARE_CONST_STRING(TYPENAME_UINT_8,					"UInt8");
DECLARE_CONST_STRING(TYPENAME_UINT_16,					"UInt16");
DECLARE_CONST_STRING(TYPENAME_UINT_32,					"UInt32");
DECLARE_CONST_STRING(TYPENAME_UINT_64,					"UInt64");
DECLARE_CONST_STRING(TYPENAME_UNSIGNED,					"Unsigned");
DECLARE_CONST_STRING(TYPENAME_VECTOR_2D,				"Vector2D");
DECLARE_CONST_STRING(TYPENAME_VECTOR_3D,				"Vector3D");
DECLARE_CONST_STRING(TYPENAME_VOID,						"VoidType");
DECLARE_CONST_STRING(TYPENAME_WILDCARD,					"WildcardType");

DECLARE_CONST_STRING(STR_ARG,							"arg");
DECLARE_CONST_STRING(STR_DOLLAR,						"$");

CCriticalSection CAEONTypes::m_cs;
TArray<CDatum> CAEONTypes::m_Types;
TSortMap<CString, DWORD> CAEONTypes::m_BuiltInTypes;
TArray<int> CAEONTypes::m_FreeTypes;
bool CAEONTypes::m_bInitDone = false;
DWORD CAEONTypes::m_dwNextAnonymousID = 1;

void CAEONTypes::AccumulateCoreTypes (TSortMap<CString, CDatum>& retTypes)

//	AccumulateCoreTypes
//
//	Adds core types to the map (but excluded library registered ones.

	{
	for (int i = 1; i <= IDatatype::MAX_CORE_TYPE; i++)
		{
		CDatum dType = Get(i);
		if (dType.IsNil())
			continue;

		if (dType.GetBasicType() != CDatum::typeDatatype)
			throw CException(errFail);

		const IDatatype &Type = dType;
		retTypes.SetAt(Type.GetFullyQualifiedName(), dType);

		//	If this type is not intrinsically nullable, then add a nullable version.

		if (!Type.CanBeNull())
			{
			CDatum dNullableType = CAEONTypes::CreateNullableType(NULL_STR, dType);
			const IDatatype& NullableType = dNullableType;
			retTypes.SetAt(NullableType.GetFullyQualifiedName(), dNullableType);
			}
		}
	}

DWORD CAEONTypes::AddEnum (CStringView sFullyQualifiedName, const TArray<IDatatype::SMemberDesc>& Values, CString* retsError)

//	AddEnum
//
//	Adds a new enum type and returns the ID.

	{
	CSmartLock Lock(m_cs);

	//	Create the new type

	DWORD dwID = Alloc();
	CDatum dEnum = CreateEnum(dwID, sFullyQualifiedName, Values, false, retsError);
	if (dEnum.IsNil())
		return 0;

	//	Add it to our store

	SetType(dEnum);
	return dwID;
	}

DWORD CAEONTypes::Alloc ()

//	Alloc
//
//	Allocates a new entry.

	{
	CSmartLock Lock(m_cs);

	if (!m_bInitDone)
		InitCoreTypes();

	if (m_FreeTypes.GetCount() > 0)
		{
		int iNextFree = m_FreeTypes.GetCount() - 1;
		int iID = m_FreeTypes[iNextFree];
		m_FreeTypes.Delete(iNextFree);

		return (DWORD)iID;
		}
	else
		{
		int iID = m_Types.GetCount();
		m_Types.Insert();
		return (DWORD)iID;
		}
	}

CDatum CAEONTypes::CreateArray (CStringView sFullyQualifiedName, CDatum dElementType)

//	CreateArray
//
//	Creates an array type.
//	If sFullyQualifiedName is empty, then we generate an anonymous name.

	{
	if (dElementType.GetBasicType() != CDatum::typeDatatype)
		throw CException(errFail);

	CDatatypeArray::SCreate Create;
	Create.sFullyQualifiedName = sFullyQualifiedName;
	Create.dElementType = dElementType;

	return CDatum(new CComplexDatatype(new CDatatypeArray(Create)));
	}

CDatum CAEONTypes::CreateClassStub (CStringView sFullyQualifiedName, IDatatype **retpNewType)

//	CreateClassStub
//
//	Creates a new datatype.

	{
	CDatatypeClass::SCreate Create;
	Create.sFullyQualifiedName = sFullyQualifiedName;
	Create.Implements = { Get(IDatatype::CLASS_T) };

	IDatatype *pNewType = new CDatatypeClass(Create);
	CDatum dNewType(new CComplexDatatype(pNewType));

	if (retpNewType)
		*retpNewType = pNewType;

	return dNewType;
	}

CDatum CAEONTypes::CreateDictionary (CStringView sFullyQualifiedName, CDatum dKeyType, CDatum dElementType)

//	CreateDictionary
//
//	Creates a dictionary type.

	{
	if (dElementType.GetBasicType() != CDatum::typeDatatype || dKeyType.GetBasicType() != CDatum::typeDatatype)
		throw CException(errFail);

	CDatatypeArray::SCreate Create;
	Create.sFullyQualifiedName = sFullyQualifiedName;
	Create.bDictionary = true;
	Create.dElementType = dElementType;
	Create.dKeyType = dKeyType;

	return CDatum(new CComplexDatatype(new CDatatypeArray(Create)));
	}

CDatum CAEONTypes::CreateEnum (DWORD dwCoreType, CStringView sFullyQualifiedName, const TArray<IDatatype::SMemberDesc>& Values, bool bBuiltIn, CString* retsError)

//	CreateEnum
//
//	Creates an enum type.

	{
	CDatatypeEnum::SCreate Create;
	Create.sFullyQualifiedName = sFullyQualifiedName;
	Create.dwCoreType = dwCoreType;
	Create.bBuiltIn = bBuiltIn;

	IDatatype* pNewType = new CDatatypeEnum(Create);
	CDatum dType(new CComplexDatatype(pNewType));

	for (int i = 0; i < Values.GetCount(); i++)
		{
		IDatatype::SMemberDesc Desc = Values[i];
		Desc.iType = IDatatype::EMemberType::EnumValue;
		Desc.dType = CDatum();
		if (Desc.iOrdinal == INT_MIN)
			Desc.iOrdinal = i;

		if (!pNewType->AddMember(Desc, retsError))
			{
			if (retsError)
				return CDatum();
			else
				throw CException(errFail);
			}
		}

	return dType;
	}

CDatum CAEONTypes::CreateFunction (CStringView sFullyQualifiedName, const IDatatype::SReturnTypeDesc& Return, TArray<IDatatype::SArgDesc>&& Args)

//	CreateDatatypeFunction
//
//	Creates a new function.

	{
	CString sID;
	if (sFullyQualifiedName.IsEmpty())
		sID = MakeFullyQualifiedName(NULL_STR, strPattern("AnonymousFunction%08x", mathRandom()));
	else
		sID = sFullyQualifiedName;

	CDatatypeFunction::SCreate Create;
	Create.sFullyQualifiedName = sID;
	Create.Return = Return;
	Create.Args = std::move(Args);

	IDatatype* pNewType = new CDatatypeFunction(CDatatypeFunction::SCreate(Create));
	CDatum dNewType(new CComplexDatatype(pNewType));

	return dNewType;
	}

CDatum CAEONTypes::CreateGenericFunctionFromArgs (CStringView sArgCode)
	{
	return CDatum(new CComplexDatatype(new CDatatypeGenericFunction(sArgCode)));
	}

static bool IsGenericFunctionArgCode (CStringView sArgCode)
	{
	const char* pStart = sArgCode.GetParsePointer();
	const char* pPos = pStart;
	const char* pEnd = pStart + sArgCode.GetLength();

	while (pPos < pEnd && strIsWhitespace(pPos))
		pPos++;

	if (pPos == pEnd || *pPos != '(')
		return false;

	int iDepth = 0;
	bool bInString = false;
	while (pPos < pEnd)
		{
		if (bInString)
			{
			if (*pPos == '\\' && pPos + 1 < pEnd)
				pPos++;
			else if (*pPos == '"')
				bInString = false;
			}
		else if (*pPos == '"')
			bInString = true;
		else if (*pPos == '(')
			iDepth++;
		else if (*pPos == ')')
			{
			iDepth--;
			if (iDepth == 0)
				{
				pPos++;
				while (pPos < pEnd && strIsWhitespace(pPos))
					pPos++;

				return (pPos + 1 < pEnd && pPos[0] == '-' && pPos[1] == '>');
				}
			}

		pPos++;
		}

	return false;
	}

CDatum CAEONTypes::CreateFunctionFromArgs (CStringView sArgCode)

//	CreateFunctionType
//
//	Creates a function type from a string encoding argument types.
//
//	The first type is always the return type, and is delimited by a colon. 
//	Next we have 0 or more signatures, separated by semicolons.
//
//	EXAMPLE
//
//	s:X=n,Y=n; s:X=s,Y=s
//
//	The following characters encode types:
//
//	?	Any
//	%	Datatype
//	a	Array
//	b	Bool
//	d	DateTime
//	F	Float64
//	I	Integer32
//	i	Integer (any integer type)
//	m	TimeSpan
//	n	Number (any numeric type)
//	s	String
//	t	Table
//	x	Struct
//	y	Dictionary
//
//	0	As a return type, this means same type as the object
//	1	As a return type, this means same type as first arg

	{
	if (IsGenericFunctionArgCode(sArgCode))
		return CreateGenericFunctionFromArgs(sArgCode);

	//	If is just * then we default to no type.

	const char* pPos = sArgCode.GetParsePointer();
	if (*pPos == '*' || *pPos == '\0')
		return CDatum();

	//	If -> then this is a def like System object which is invoked without
	//	arguments and only has a value type.

	else if (*pPos == '-' && pPos[1] == '>')
		{
		pPos += 2;
		return ParseTypeFromArgCode(pPos);
		}

	//	Parse the return type

	IDatatype::SReturnTypeDesc Return;

	if (*pPos == '%' && pPos[1] >= '1' && pPos[1] <= '9')
		{
		Return.iType = IDatatype::EReturnDescType::ArgLiteral;
		Return.iFromArg = (pPos[1] - '0');
		Return.dType = Get(IDatatype::ANY);
		pPos += 2;
		}
	else if (*pPos >= '0' && *pPos <= '9')
		{
		Return.iType = IDatatype::EReturnDescType::ArgType;
		Return.iFromArg = (*pPos - '0');
		Return.dType = Get(IDatatype::ANY);
		pPos++;
		}
	else
		{
		Return.iType = IDatatype::EReturnDescType::Type;
		Return.dType = ParseTypeFromArgCode(pPos);
		if (Return.dType.IsNil())
			throw CException(errFail);
		}

	//	:

	if (*pPos++ != ':')
		throw CException(errFail);

	//	Parse the arguments.

	TArray<IDatatype::SArgDesc> Args;

	int iSignature = 0;
	bool bExpectReturnType = false;

	while (*pPos != '\0')
		{
		//	| means another signature

		if (*pPos == ';')
			{
			pPos++;
			iSignature++;
			bExpectReturnType = true;
			continue;
			}
		else if (*pPos == '|')
			{
			pPos++;
			iSignature++;
			continue;
			}
		else if (*pPos == ',')
			{
			pPos++;
			continue;
			}
		else if (strIsWhitespace(pPos))
			{
			pPos++;
			continue;
			}

		//	Parse the return type, if necessary.

		if (bExpectReturnType)
			{
			CDatum dReturnType = ParseTypeFromArgCode(pPos);
			if (dReturnType.IsNil())
				throw CException(errFail);

			if (*pPos++ != ':')
				throw CException(errFail);

			//	Add as an argument type with no name.

			Args.Insert({ iSignature, NULL_STR, dReturnType, NULL_STR, false });

			bExpectReturnType = false;
			}

		//	A star means we support any number of arguments.

		if (*pPos == '*')
			{
			pPos++;

			//	Parse an optional type, but if not found, assume Any

			CDatum dType;
			if (*pPos == '=')
				{
				pPos++;

				dType = ParseTypeFromArgCode(pPos);
				if (dType.IsNil())
					throw CException(errFail);
				}
			else
				{
				dType = CAEONTypes::Get(IDatatype::ANY);
				}

			Args.Insert({ iSignature, STR_ARG, dType, NULL_STR, true });
			}

		//	Otherwise, we expect a set of arguments.

		else
			{
			//	Parse the argument name.

			const char* pStart = pPos;
			while (*pPos != '\0' && *pPos != '=')
				pPos++;

			CString sArgName(pStart, pPos - pStart);
			if (*pPos++ != '=')
				throw CException(errFail);

			//	Parse the type

			CDatum dType = ParseTypeFromArgCode(pPos);
			if (dType.IsNil())
				throw CException(errFail);

			//	Add

			Args.Insert({ iSignature, sArgName, dType, NULL_STR, false });
			}
		}

	//	Create function type.

	return CAEONTypes::CreateFunction(CAEONTypes::MakeFullyQualifiedFunctionName(), Return, std::move(Args));
	}

CDatum CAEONTypes::CreateLiteralStruct (CDatum dSchema)

//	CreateLiteralStruct
//
//	Creates a literal struct type.

	{
	if (dSchema.GetBasicType() != CDatum::typeDatatype)
		throw CException(errFail);

	return CDatum(new CComplexDatatype(new CDatatypeLiteralStruct(dSchema)));
	}

CDatum CAEONTypes::CreateQualified (CDatum dType, DWORD dwQualifier)

//	CreateQualified
//
//	Creates a qualified type.

	{
	if (dType.GetBasicType() != CDatum::typeDatatype)
		throw CException(errFail);

	if (dwQualifier == 0)
		return dType;

	return CDatum(new CComplexDatatype(new CDatatypeQualified(dType, dwQualifier)));
	}

CDatum CAEONTypes::CreateInt32SubRange (CStringView sFullyQualifiedName, int iMin, int iMax)

//	CreateInt32SubRange
//
//	Creates a sub-range of int32.

	{
	CDatatypeNumber::SCreate Create;
	Create.sFullyQualifiedName = sFullyQualifiedName;
	Create.Implements = { Get(IDatatype::INT_32) };
	Create.iBits = 32;
	Create.bFloat = false;
	Create.bUnsigned = false;
	Create.bSubRange = true;
	Create.iSubRangeMin = iMin;
	Create.iSubRangeMax = iMax;
	Create.bAbstract = false;
	Create.bCanBeNull = false;
	Create.bBuiltIn = false;

	return CDatum(new CComplexDatatype(new CDatatypeNumber(Create)));
	}

CDatum CAEONTypes::CreateNullableType (CStringView sFullyQualifiedName, CDatum dVariantType)

//	CreateNullableType
//
//	Creates a nullable type.

	{
	const IDatatype& VariantType = dVariantType;

	//	Certain types are intrinsically nullable, which means we can just return
	//	them.

	if (VariantType.CanBeNull())
		return dVariantType;

	CString sID;
	if (sFullyQualifiedName.IsEmpty())
		{
		if (VariantType.GetCoreType())
			sID = MakeFullyQualifiedName(NULL_STR, strPattern("%s?", VariantType.GetName()));
		else
			sID = MakeFullyQualifiedName(NULL_STR, strPattern("NullableType%08x", mathRandom()));
		}
	else
		sID = sFullyQualifiedName;

	CDatatypeNullable::SCreate Create;
	Create.sFullyQualifiedName = sID;
	Create.dVariantType = dVariantType;

	return CDatum(new CComplexDatatype(new CDatatypeNullable(CDatatypeNullable::SCreate(Create))));
	}

CDatum CAEONTypes::CreateTable (CStringView sFullyQualifiedName, CDatum dSchema)

//	CreateTable
//
//	Creates a table.
//	If sFullyQualifiedName is empty, then we generate an anonymous name.

	{
	return CreateTable(0, sFullyQualifiedName, dSchema, false);
	}

CDatum CAEONTypes::CreateTable (DWORD dwCoreType, CStringView sFullyQualifiedName, CDatum dSchema, bool bBuiltIn)

//	CreateTable
//
//	Create a table datatype.

	{
	if (((const IDatatype&)dSchema).GetClass() != IDatatype::ECategory::Schema)
		throw CException(errFail);

	CDatatypeArray::SCreate Create;
	Create.sFullyQualifiedName = sFullyQualifiedName;
	Create.dwCoreType = dwCoreType;
	Create.dElementType = dSchema;
	Create.bTable = true;
	Create.bBuiltIn = bBuiltIn;

	return CDatum(new CComplexDatatype(new CDatatypeArray(Create)));
	}

CDatum CAEONTypes::CreateTensor (CStringView sFullyQualifiedName, CDatum dElementType, TArray<CDatum>&& Dimensions)
	{
	CDatatypeTensor::SCreate Create;
	Create.sFullyQualifiedName = sFullyQualifiedName;
	Create.dElementType = dElementType;
	Create.Dimensions = std::move(Dimensions);

	return CDatum(new CComplexDatatype(new CDatatypeTensor(Create)));
	}

CDatum CAEONTypes::CreatePropertyType (const char* pPos)
	{
	return ParseTypeFromArgCode(pPos);
	}

CDatum CAEONTypes::CreateSchema (CStringView sFullyQualifiedName, const TArray<IDatatype::SMemberDesc>& Columns)

//	CreateSchema
//
//	Creates a schema datatype.
//	If sFullyQualifiedName is empty, then we generate an anonymous name.

	{
	return CreateSchema(0, sFullyQualifiedName, Columns, false);
	}

CDatum CAEONTypes::CreateSchema (DWORD dwCoreType, CStringView sFullyQualifiedName, const TArray<IDatatype::SMemberDesc>& Columns, bool bBuiltIn)

//	CreateSchema
//
//	Creates a new schema.

	{
	CDatatypeSchema::SCreate Create;
	Create.sFullyQualifiedName = sFullyQualifiedName;
	Create.Implements = { Get(IDatatype::SCHEMA) };
	Create.dwCoreType = dwCoreType;
	Create.bBuiltIn = bBuiltIn;

	IDatatype* pNewType = new CDatatypeSchema(Create);
	CDatum dType(new CComplexDatatype(pNewType));

	for (int i = 0; i < Columns.GetCount(); i++)
		{
		if (Columns[i].iType != IDatatype::EMemberType::InstanceKeyVar
				&& Columns[i].iType != IDatatype::EMemberType::InstanceVar)
			throw CException(errFail);

		if (!pNewType->AddMember(Columns[i]))
			throw CException(errFail);
		}

	return dType;
	}

CDatum CAEONTypes::CreateSchemaStub (CStringView sFullyQualifiedName, IDatatype **retpNewType)

//	CreateDatatypeSchema
//
//	Creates a new datatype.

	{
	CDatatypeSchema::SCreate Create;
	Create.sFullyQualifiedName = sFullyQualifiedName;
	Create.Implements = { Get(IDatatype::SCHEMA) };

	IDatatype* pNewType = new CDatatypeSchema(Create);
	CDatum dType(new CComplexDatatype(pNewType));

	if (retpNewType)
		*retpNewType = pNewType;

	return dType;
	}

void CAEONTypes::InitGridNameType ()
	{
	//	We need to create the type in two phases because some methods refer to the 
	//	type itself.

	auto* pType = new CDatatypeAEON(MakeFullyQualifiedName(NULL_STR, TYPENAME_GRID_NAME_TYPE), IDatatype::GRID_NAME_TYPE, CDatatypeList({ IDatatype::STRING }), DATUM_TYPENAME_GRID_NAME, TArray<IDatatype::SMemberDesc>(), true);
	SetType(CDatum(new CComplexDatatype(pType)));

	//	NOTE: We need to set these members AFTER we create the type because the
	//	members refer to the type itself.

	TArray<IDatatype::SMemberDesc> Members;
	Members.Insert({ IDatatype::EMemberType::InstanceProperty, FIELD_ID, Get(IDatatype::STRING), 0, STR_CAEONTYPES_ID });
	Members.Insert({ IDatatype::EMemberType::InstanceProperty, FIELD_NAME, Get(IDatatype::STRING), 1, STR_CAEONTYPES_NAME });

	pType->SetMembers(std::move(Members));
	}

CDatum CAEONTypes::FindBuiltInType (CStringView sFullyQualifiedName, const IDatatype** retpDatatype)

//	FindCoreType
//
//	Returns a core type (or null)

	{
	CSmartLock Lock(m_cs);

	if (!m_bInitDone)
		InitCoreTypes();

	auto pEntry = m_BuiltInTypes.GetAt(strToLower(sFullyQualifiedName));
	if (!pEntry)
		{
		if (retpDatatype)
			*retpDatatype = NULL;

		return CDatum();
		}

	CDatum dType = Get(*pEntry);
	if (retpDatatype)
		*retpDatatype = &(const IDatatype &)dType;

	return dType;
	}

CDatum CAEONTypes::FindEnumOrAdd (CDatum dType)

//	FindEnum
//
//	Returns an enum type that matches the type.

	{
	const IDatatype& TypeToFind = dType;
	if (TypeToFind.GetClass() != IDatatype::ECategory::Enum)
		throw CException(errFail);

	const CDatatypeEnum& EnumToFind = (const CDatatypeEnum&)TypeToFind;

	CSmartLock Lock(m_cs);

	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		const IDatatype& OtherType = (const IDatatype&)m_Types[i];
		if (OtherType.GetClass() == IDatatype::ECategory::Enum)
			{
			const CDatatypeEnum& EnumType = (const CDatatypeEnum&)OtherType;
			if (EnumType.IsEqual(EnumToFind))
				return m_Types[i];
			}
		}

	//	If we get this far it means we didn't find the type, so we have to add
	//	it.

	//	Make sure the type itself has an ID. This is a HACK because we're 
	//	casting, but it is OK because we own these objects.

	DWORD dwID = Alloc();
	const_cast<IDatatype&>(TypeToFind).SetCoreType(dwID);

	//	Add it

	SetType(dType);

	return dType;
	}

CDatum CAEONTypes::FindEnum (const TArray<IDatatype::SMemberDesc>& Values)

//	FindEnum
//
//	Returns an enum type that matches the given members (or Nil)

	{
	CSmartLock Lock(m_cs);

	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		if (((const IDatatype&)m_Types[i]).IsEnum(Values))
			return m_Types[i];
		}

	return CDatum();
	}

CDatum CAEONTypes::FindTableOrAdd (CDatum dSchema)

//	FindTableOrAdd
//
//	Looks for a table with the given schema. If not found, we add it.

	{
	CSmartLock Lock(m_cs);

	const IDatatype& SchemaToFind = dSchema;

	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		const IDatatype& TableType = (const IDatatype&)m_Types[i];
		if (TableType.GetClass() == IDatatype::ECategory::Table)
			{
			const IDatatype& Schema = TableType.GetElementType();
			if (Schema == SchemaToFind)
				return m_Types[i];
			}
		}

	//	Add it.

	DWORD dwID = Alloc();

	IDatatype *pNewType = new CDatatypeArray(CDatatypeArray::SCreate({ NULL_STR, dwID, dSchema, true }));
	CDatum dTable = CDatum(new CComplexDatatype(pNewType));

	//	Add it to our store

	SetType(dTable);
	return dTable;
	}

CDatum CAEONTypes::FindType (CDatum dType)

//	FindType
//
//	Finds a type that is equal to the given type. Returns Nil if not found.

	{
	const IDatatype& TypeToFind = dType;

	CSmartLock Lock(m_cs);

	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		if ((const IDatatype&)m_Types[i] == TypeToFind)
			return m_Types[i];
		}

	return CDatum();
	}

CDatum CAEONTypes::Get (DWORD dwID)

//	Get
//
//	Return a type from an ID.

	{
	CSmartLock Lock(m_cs);

	if (!m_bInitDone)
		InitCoreTypes();

	if (dwID <= 0 || dwID >= (DWORD)m_Types.GetCount())
		throw CException(errFail);

	return m_Types[dwID];
	}

CDatum CAEONTypes::GetCompatibleType (CDatum dLeft, CDatum dRight)

//	GetCompatibleType
//
//	Returns a type that encompases both dLeft and dRight. That is, both dLeft
//	and dRight is-a result type.

	{
	const IDatatype& LeftType = (const IDatatype &)dLeft;
	const IDatatype& RightType = (const IDatatype &)dRight;

	//	If one type is a subtype or equal to the other, then we use that type.

	CDatum dNewDatatype;
	if (RightType.IsA(LeftType))
		return dLeft;
	else if (LeftType.IsA(RightType))
		return dRight;

	//	Otherwise, if both are numbers, then number

	else if (LeftType.IsA(IDatatype::NUMBER) && RightType.IsA(IDatatype::NUMBER))
		return Get(IDatatype::NUMBER);

	//	If both are arrays, then array type

	else if (LeftType.IsA(IDatatype::ARRAY) && RightType.IsA(IDatatype::ARRAY))
		return Get(IDatatype::ARRAY);

	//	Otherwise, Any

	else
		return Get(IDatatype::ANY);
	}

CDatum CAEONTypes::GetCompatibleType (const TArray<CDatum>& Types)
	{
	if (Types.GetCount() == 0)
		return CDatum();

	CDatum dCommonType = Types[0];
	for (int i = 1; i < Types.GetCount(); i++)
		{
		CDatum dValueType = Types[i];
		if (!((const IDatatype&)dValueType).IsA(dCommonType))
			{
			//	If the reverse is true, then we can switch to the more 
			//	general type.

			if (((const IDatatype&)dCommonType).IsA(dValueType))
				dCommonType = dValueType;

			//	Otherwise, we get the compatible type.

			else
				{
				dCommonType = GetCompatibleType(dCommonType, dValueType);
				if (((const IDatatype&)dCommonType).IsAny())
					return dCommonType;
				}
			}
		}

	return dCommonType;
	}

CDatum CAEONTypes::Get_NoError (DWORD dwID)

//	Get_NoError
//
//	Return a type from an ID.

	{
	CSmartLock Lock(m_cs);

	if (!m_bInitDone)
		InitCoreTypes();

	if (dwID <= 0 || dwID >= (DWORD)m_Types.GetCount())
		return CDatum();

	return m_Types[dwID];
	}

void CAEONTypes::InitCoreTypes ()

//	InitCoreTypes
//
//	Registers all core types.

	{
	ASSERT(m_cs.IsLockedByCurrentThread());

#ifdef DEBUG
	if (m_Types.GetCount() > 0 || m_bInitDone)
		throw CException(errFail);
#endif

	m_bInitDone = true;

	//	+1 because the count is always one more than the max index.

	m_Types.InsertEmpty(IDatatype::MAX_CORE_TYPE + 1);

	//	Add all core types.
	// 
	//	NOTE: When specifying an implementation, make sure the implementation 
	//	has already been defined.

	RegisterAny();
	RegisterSimple(IDatatype::BOOL, MakeFullyQualifiedName(NULL_STR, TYPENAME_BOOL), { }, true, false, true);
	RegisterAEONType(IDatatype::DATATYPE, MakeFullyQualifiedName(NULL_STR, TYPENAME_DATATYPE), DATUM_TYPENAME_DATATYPE, CComplexDatatype::GetMembers);
	RegisterDayOfWeekEnum();
	RegisterEncodingTypeEnum();
	RegisterCharSetTypeEnum();
	RegisterNever();
	RegisterNull();
	RegisterVoid();
	RegisterSimple(IDatatype::ERROR_T, MakeFullyQualifiedName(NULL_STR, TYPENAME_ERROR), { }, false, false);
	RegisterSimple(IDatatype::NUMBER, MakeFullyQualifiedName(NULL_STR, TYPENAME_NUMBER), { }, true, true, true);
	RegisterNumber(IDatatype::REAL, MakeFullyQualifiedName(NULL_STR, TYPENAME_REAL), { IDatatype::NUMBER }, 0, true, false, true, true);
	RegisterNumber(IDatatype::INTEGER, MakeFullyQualifiedName(NULL_STR, TYPENAME_INTEGER), { IDatatype::NUMBER }, 0, false, false, true, true);
	RegisterNumber(IDatatype::FLOAT, MakeFullyQualifiedName(NULL_STR, TYPENAME_FLOAT), { IDatatype::REAL }, 0, true, false, true, false);
	RegisterSimple(IDatatype::SIGNED, MakeFullyQualifiedName(NULL_STR, TYPENAME_SIGNED), { IDatatype::INTEGER }, true, true, true);
	RegisterSimple(IDatatype::UNSIGNED, MakeFullyQualifiedName(NULL_STR, TYPENAME_UNSIGNED), { IDatatype::INTEGER }, true, true, true);
	RegisterNumber(IDatatype::INT_32, MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_32), { IDatatype::SIGNED }, 32, false, false);
	RegisterNumber(IDatatype::INT_64, MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_64), { IDatatype::SIGNED }, 64, false, false);
	RegisterNumber(IDatatype::INT_IP, MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_IP), { IDatatype::SIGNED }, 0, false, false);
	RegisterNumber(IDatatype::UINT_32, MakeFullyQualifiedName(NULL_STR, TYPENAME_UINT_32), { IDatatype::UNSIGNED }, 32, false, true);
	RegisterNumber(IDatatype::UINT_64, MakeFullyQualifiedName(NULL_STR, TYPENAME_UINT_64), { IDatatype::UNSIGNED }, 64, false, true);
	RegisterInt32SubRange(IDatatype::INT_8, MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_8), -128, 127);
	RegisterInt32SubRange(IDatatype::INT_16, MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_16), -32768, 32767);
	RegisterInt32SubRange(IDatatype::UINT_8, MakeFullyQualifiedName(NULL_STR, TYPENAME_UINT_8), 0, 255);
	RegisterInt32SubRange(IDatatype::UINT_16, MakeFullyQualifiedName(NULL_STR, TYPENAME_UINT_16), 0, 65535);
	RegisterNumber(IDatatype::NAN_CONST, MakeFullyQualifiedName(NULL_STR, TYPENAME_NAN), { IDatatype::FLOAT }, 64, true, false);
	RegisterNumber(IDatatype::FLOAT_64, MakeFullyQualifiedName(NULL_STR, TYPENAME_FLOAT_64), { IDatatype::FLOAT }, 64, true, false);
	RegisterSimple(IDatatype::ABSTRACT_DICTIONARY, MakeFullyQualifiedName(NULL_STR, TYPENAME_ABSTRACT_DICTIONARY), { }, true, false);
	RegisterSimple(IDatatype::INDEXED, MakeFullyQualifiedName(NULL_STR, TYPENAME_INDEXED), { }, true, false);
	RegisterAEONType(IDatatype::RANGE, MakeFullyQualifiedName(NULL_STR, TYPENAME_RANGE), DATUM_TYPENAME_RANGE, CAEONRange::GetMembers);
	RegisterSimple(IDatatype::ABSTRACT_MUTABLE_DICTIONARY, MakeFullyQualifiedName(NULL_STR, TYPENAME_ABSTRACT_MUTABLE_DICTIONARY), { }, true, false);
	RegisterSimple(IDatatype::MUTABLE_INDEXED, MakeFullyQualifiedName(NULL_STR, TYPENAME_MUTABLE_INDEXED), { }, true, false);
	RegisterStringType();
	RegisterArray(IDatatype::ARRAY, MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY), Get(IDatatype::ANY));
	RegisterSimple(IDatatype::SCHEMA, MakeFullyQualifiedName(NULL_STR, TYPENAME_SCHEMA), { IDatatype::ABSTRACT_DICTIONARY, IDatatype::ABSTRACT_MUTABLE_DICTIONARY }, true, false);
	RegisterSimpleEx(IDatatype::STRUCT, MakeFullyQualifiedName(NULL_STR, TYPENAME_STRUCT), { IDatatype::ABSTRACT_DICTIONARY, IDatatype::ABSTRACT_MUTABLE_DICTIONARY }, { IDatatype::SCHEMA }, false, false);
	RegisterDictionary(IDatatype::DICTIONARY, MakeFullyQualifiedName(NULL_STR, TYPENAME_DICTIONARY), Get(IDatatype::ANY), Get(IDatatype::ANY));
	RegisterSimple(IDatatype::DATE_TIME, MakeFullyQualifiedName(NULL_STR, TYPENAME_DATE_TIME), { }, false, false);
	RegisterSimple(IDatatype::TIME_SPAN, MakeFullyQualifiedName(NULL_STR, TYPENAME_TIME_SPAN), { }, false, false);
	RegisterAEONType(IDatatype::BINARY, MakeFullyQualifiedName(NULL_STR, TYPENAME_BINARY), DATUM_TYPENAME_BINARY, CComplexBinary::GetMembers);
	RegisterSimple(IDatatype::FUNCTION, MakeFullyQualifiedName(NULL_STR, TYPENAME_FUNCTION), { }, true, false);
	RegisterSimple(IDatatype::EXPRESSION, MakeFullyQualifiedName(NULL_STR, TYPENAME_EXPRESSION), { }, false, true);
	RegisterSimple(IDatatype::CLASS_T, MakeFullyQualifiedName(NULL_STR, TYPENAME_CLASS_T), { IDatatype::ABSTRACT_DICTIONARY, IDatatype::ABSTRACT_MUTABLE_DICTIONARY }, true, false);
	RegisterSimple(IDatatype::OBJECT, MakeFullyQualifiedName(NULL_STR, TYPENAME_OBJECT), { IDatatype::ABSTRACT_DICTIONARY, IDatatype::ABSTRACT_MUTABLE_DICTIONARY }, true, false);
	RegisterSimple(IDatatype::TABLE, MakeFullyQualifiedName(NULL_STR, TYPENAME_TABLE), { IDatatype::ABSTRACT_DICTIONARY, IDatatype::INDEXED, IDatatype::ABSTRACT_MUTABLE_DICTIONARY, IDatatype::MUTABLE_INDEXED  }, true, false);
	RegisterArray(IDatatype::ARRAY_INT_32, MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_INT_32), Get(IDatatype::INT_32), true);
	RegisterArray(IDatatype::ARRAY_FLOAT_64, MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_FLOAT_64), Get(IDatatype::FLOAT_64), true);
	RegisterArray(IDatatype::ARRAY_STRING, MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_STRING), Get(IDatatype::STRING), true);
	RegisterArray(IDatatype::ARRAY_DATE_TIME, MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_DATE_TIME), Get(IDatatype::DATE_TIME), true);
	RegisterArray(IDatatype::ARRAY_INT_64, MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_INT_64), Get(IDatatype::INT_64), true);
	RegisterArray(IDatatype::ARRAY_INT_IP, MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_INT_IP), Get(IDatatype::INT_IP), true);
	RegisterArray(IDatatype::ARRAY_NUMBER, MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_NUMBER), Get(IDatatype::NUMBER), true);
	RegisterAEONType(IDatatype::VECTOR_2D_F64, MakeFullyQualifiedName(NULL_STR, TYPENAME_VECTOR_2D), DATUM_TYPENAME_VECTOR_2D, CAEONVector2D::GetMembers);
	RegisterAEONType(IDatatype::VECTOR_3D_F64, MakeFullyQualifiedName(NULL_STR, TYPENAME_VECTOR_3D), DATUM_TYPENAME_VECTOR_2D, CAEONVector3D::GetMembers);
	RegisterArray(IDatatype::ARRAY_VECTOR_2D, MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_VECTOR_2D), Get(IDatatype::VECTOR_2D_F64), true);
	RegisterArray(IDatatype::ARRAY_VECTOR_3D, MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_VECTOR_3D), Get(IDatatype::VECTOR_3D_F64), true);
	RegisterMemberTypeEnum();
	RegisterMemberTableSchema();
	RegisterMemberTableType();
	RegisterSchemaTableSchema();
	RegisterSchemaTableType();
	RegisterTensor(IDatatype::MATRIX_F64, MakeFullyQualifiedName(NULL_STR, TYPENAME_MATRIX_F64), Get(IDatatype::FLOAT_64), 0, 0);
	RegisterTensor(IDatatype::MATRIX_3X3_F64, MakeFullyQualifiedName(NULL_STR, TYPENAME_MATRIX_3X3_F64), Get(IDatatype::FLOAT_64), 3, 3);
	RegisterTensor(IDatatype::MATRIX_4X4_F64, MakeFullyQualifiedName(NULL_STR, TYPENAME_MATRIX_4X4_F64), Get(IDatatype::FLOAT_64), 4, 4);
	RegisterSimple(IDatatype::CANVAS, MakeFullyQualifiedName(NULL_STR, TYPENAME_CANVAS), { }, false, false);
	RegisterSimple(IDatatype::BITMAP_RGBA8, MakeFullyQualifiedName(NULL_STR, TYPENAME_BITMAP_RGBA8), { }, false, false);
	RegisterSimple(IDatatype::ENUM, MakeFullyQualifiedName(NULL_STR, TYPENAME_ENUM), { }, true, false);
	RegisterArray(IDatatype::TEXT_LINES, MakeFullyQualifiedName(NULL_STR, TYPENAME_TEXT_LINES), Get(IDatatype::STRING));
	RegisterSimple(IDatatype::SAS_DATE_TIME, MakeFullyQualifiedName(NULL_STR, TYPENAME_SAS_DATE_TIME), { }, false, false);
	RegisterSimple(IDatatype::SAS_DATE, MakeFullyQualifiedName(NULL_STR, TYPENAME_SAS_DATE), { }, false, false);
	RegisterSimple(IDatatype::SAS_TIME, MakeFullyQualifiedName(NULL_STR, TYPENAME_SAS_TIME), { }, false, false);
	RegisterAEONType(IDatatype::STRING_FORMAT_TYPE, MakeFullyQualifiedName(NULL_STR, TYPENAME_STRING_FORMAT_TYPE), DATUM_TYPENAME_STRING_FORMAT, CAEONStringFormat::GetMembers);
	RegisterAEONType(IDatatype::MAP_COLUMN_EXPRESSION, MakeFullyQualifiedName(NULL_STR, TYPENAME_MAP_COLUMN_EXPRESSION), DATUM_TYPENAME_MAP_COLUMN_EXPRESSION, CAEONMapColumnExpression::GetMembers);
	RegisterSimple(IDatatype::WILDCARD, MakeFullyQualifiedName(NULL_STR, TYPENAME_WILDCARD), { }, false, false, true);
	InitGridNameType();

#ifdef DEBUG
	if (!m_Types[IDatatype::UNKNOWN].IsIdenticalToNil())
		throw CException(errFail);

	for (int i = 1; i <= IDatatype::MAX_CORE_TYPE; i++)
		{
		if (m_Types[i].GetBasicType() != CDatum::typeDatatype)
			throw CException(errFail);

		if (!((const IDatatype&)m_Types[i]).IsBuiltIn())
			throw CException(errFail);
		}
#endif
	}

CString CAEONTypes::MakeAnonymousName (const CString& sType)

//	MakeAnonymousName
//
//	Returns a new name.

	{
	CSmartLock Lock(m_cs);
	DWORD dwID = m_dwNextAnonymousID++;
	Lock.Unlock();

	return strPattern("Anonymous%s%08x", sType, dwID);
	}

CString CAEONTypes::MakeAnonymousName (const CString& sFullyQualifiedScope, const CString& sType)

//	MakeAnonymousName
//
//	Returns a fully qualified anonymous type name.

	{
	CSmartLock Lock(m_cs);
	DWORD dwID = m_dwNextAnonymousID++;
	Lock.Unlock();

	return strPattern("%s$Anonymous%s%08x", sFullyQualifiedScope, sType, dwID);
	}

CString CAEONTypes::MakeFullyQualifiedFunctionName (const CString& sFullyQualifiedScope)

//	MakeFullyQualifiedFunctionName
//
//	Returns a fully qualified function name.

	{
	CSmartLock Lock(m_cs);
	DWORD dwID = m_dwNextAnonymousID++;
	Lock.Unlock();

	return strPattern("%s$Function%08x", sFullyQualifiedScope, dwID);
	}

CString CAEONTypes::MakeFullyQualifiedName (const CString &sFullyQualifiedScope, const CString &sName)

//	MakeFullyQualifiedName
//
//	Returns a fully qualified name from a scope and name. A scope of NULL means
//	global scope.

	{
	return strPattern("%s$%s", sFullyQualifiedScope, sName);
	}

void CAEONTypes::MarkAndSweep ()

//	MarkAndSweep
//
//	Remove any types that are no longer being used. We assume this is called 
//	after all datums are marked but before sweep.

	{
	//	NOTE: No need to lock because GC is always single-threaded.

	if (m_Types.GetCount() == 0)
		return;

	//	No need to lock because garbage collection is always single-threaded.

	//	Mark all core types.

	for (int i = 0; i < m_BuiltInTypes.GetCount(); i++)
		m_Types[m_BuiltInTypes[i]].Mark();

	//	For all other types, if not marked, then we can discard them.

	for (int i = IDatatype::MAX_CORE_TYPE + 1; i < m_Types.GetCount(); i++)
		{
		if (!m_Types[i].IsIdenticalToNil())
			{
			auto pComplex = m_Types[i].GetComplex();
			if (pComplex && !pComplex->IsMarked())
				{
				m_Types[i] = CDatum();
				m_FreeTypes.Insert(i);
				}
			}
		}
	}

CString CAEONTypes::ParseNameFromFullyQualifiedName (const CString &sValue, bool bAbsolute)

//	ParseNameFromFullyQualifiedName
//
//	A fully-qualified name has the form [$SCOPE]$NAME. In some cases $SCOPE is null
//	(because its a global scope). Also, sometimes SCOPE is itself a fully-
//	qualified name.
//
//	We return the name separated by dots. For example:
//
//	$MyClass -> MyClass
//	$MyClass$MyRecord -> MyClass.MyRecord

	{
	TArray<CString> Parts;
	strSplit(sValue, STR_DOLLAR, &Parts, -1, SSP_FLAG_NO_EMPTY_ITEMS);

	//	Now join

	if (Parts.GetCount() == 0)
		return NULL_STR;
	else if (Parts.GetCount() == 1)
		return Parts[0];
	else if (!bAbsolute)
		return Parts[Parts.GetCount() - 1];
	else
		{
		CString sResult = Parts[0];
		for (int i = 1; i < Parts.GetCount(); i++)
			sResult = strPattern("%s.%s", sResult, Parts[i]);

		return sResult;
		}
	}

CDatum CAEONTypes::ParseTypeFromArgCode (const char*& pPos)

//	ParseTypeFromArgCode
//
//	Parses the next type from the given string. Returns Nil if we cannot parse
//
//	?		Any
//	%		Datatype
//	a		Array
//	b		Bool
//	d		DateTime
//	f		Float
//	F		Float64
//	I		Integer32
//	i		Integer (any integer type)
//	m		TimeSpan
//	n		Number (any numeric type)
//	q		Column Expression
//	s		String
//	t		Table
//	v		Binary
//	V2		Vector2D
//	V3		Vector3D
//	x		Struct
//	$TYPE	Named type
//	(args)	Function type

	{
	switch (*pPos)
		{
		case '?':
			pPos++;
			return CAEONTypes::Get(IDatatype::ANY);

		case '$':
			{
			const char* pStart = pPos;
			while (*pPos != '\0' && *pPos != ';' && *pPos != ',' && *pPos != ':' && *pPos != '|')
				pPos++;

			CString sTypename(pStart, pPos - pStart);
			if (*pPos == ';')
				pPos++;

			return FindBuiltInType(sTypename);
			}

		case '%':
			pPos++;
			return CAEONTypes::Get(IDatatype::DATATYPE);

		case 'a':
			pPos++;
			return CAEONTypes::Get(IDatatype::ARRAY);

		case 'b':
			pPos++;
			return CAEONTypes::Get(IDatatype::BOOL);

		case 'd':
			pPos++;
			return CAEONTypes::Get(IDatatype::DATE_TIME);

		case 'f':
			pPos++;
			return CAEONTypes::Get(IDatatype::FLOAT);

		case 'F':
			pPos++;
			return CAEONTypes::Get(IDatatype::FLOAT_64);

		case 'I':
			pPos++;
			return CAEONTypes::Get(IDatatype::INT_32);

		case 'i':
			pPos++;
			return CAEONTypes::Get(IDatatype::INTEGER);

		case 'm':
			pPos++;
			return CAEONTypes::Get(IDatatype::TIME_SPAN);

		case 'n':
			pPos++;
			return CAEONTypes::Get(IDatatype::NUMBER);

		case 'q':
			pPos++;
			return CAEONTypes::Get(IDatatype::EXPRESSION);

		case 'r':
			pPos++;
			return CAEONTypes::Get(IDatatype::REAL);

		case 's':
			pPos++;
			return CAEONTypes::Get(IDatatype::STRING);

		case 't':
			{
			pPos++;
			if (*pPos == '$')
				{
				const char* pStart = pPos;
				while (*pPos != '\0' && *pPos != ';' && *pPos != ',' && *pPos != ':' && *pPos != '|')
					pPos++;

				CString sTypename(pStart, pPos - pStart);
				if (*pPos == ';')
					pPos++;

				CDatum dSchema = FindBuiltInType(sTypename);
				if (dSchema.IsNil())
					return CDatum();

				const IDatatype& Schema = dSchema;
				if (Schema.GetClass() != IDatatype::ECategory::Schema)
					return CDatum();

				return FindTableOrAdd(dSchema);
				}
			else
				return CAEONTypes::Get(IDatatype::TABLE);
			}

		case 'v':
			pPos++;
			return CAEONTypes::Get(IDatatype::BINARY);

		case 'V':
			pPos++;
			if (*pPos == '2')
				{
				pPos++;
				return CAEONTypes::Get(IDatatype::VECTOR_2D_F64);
				}
			else if (*pPos == '3')
				{
				pPos++;
				return CAEONTypes::Get(IDatatype::VECTOR_3D_F64);
				}
			else
				return CDatum();

		case 'x':
			pPos++;
			return CAEONTypes::Get(IDatatype::STRUCT);

		case 'y':
			pPos++;
			return CAEONTypes::Get(IDatatype::DICTIONARY);

		case '(':
			{
			//	Keep advancing until we find the closing paren (we support 
			//	nesting).

			int iDepth = 1;
			const char* pStart = pPos + 1;
			while (*pPos != '\0' && iDepth > 0)
				{
				if (*pPos == '(')
					iDepth++;
				else if (*pPos == ')')
					iDepth--;

				if (iDepth > 0)
					pPos++;
				}

			if (*pPos == '\0' && iDepth > 0)
				return CDatum();

			//	Create a function type with the given args

			CString sFunctionArgs(pStart, pPos - pStart);
			if (*pStart == '*' || sFunctionArgs.IsEmpty())
				return CAEONTypes::Get(IDatatype::FUNCTION);
			else
				return CreateFunctionFromArgs(CString(pStart, pPos - pStart));
			}

		default:
			return CDatum();
		}
	}

void CAEONTypes::RegisterAny ()

//	CreateAny
//
//	Creates the Any datatype.

	{
	CDatum dType(new CComplexDatatype(new CDatatypeAny(MakeFullyQualifiedName(NULL_STR, TYPENAME_ANY))));
	SetType(dType);
	}

DWORD CAEONTypes::RegisterAEON (CStringView sTypename, CDatatypeList&& Implement, CStringView sDatumTypename, TArray<IDatatype::SMemberDesc>&& Members)
	{
	CSmartLock Lock(m_cs);

	DWORD dwID = Alloc();
	auto* pType = new CDatatypeAEON(MakeFullyQualifiedName(NULL_STR, sTypename), dwID, std::move(Implement), sDatumTypename, std::move(Members));

	CDatum dType = CDatum(new CComplexDatatype(pType));

	SetType(dType);
	return dwID;
	}

DWORD CAEONTypes::RegisterAEON (CStringView sTypename, CDatatypeList&& Implement, CStringView sDatumTypename, std::function<TArray<IDatatype::SMemberDesc>()> fnMembers)
	{
	CSmartLock Lock(m_cs);

	DWORD dwID = Alloc();
	auto* pType = new CDatatypeAEON(MakeFullyQualifiedName(NULL_STR, sTypename), dwID, std::move(Implement), sDatumTypename);

	CDatum dType = CDatum(new CComplexDatatype(pType));

	SetType(dType);

	//	NOTE: We can't generate the list of members until AFTER we have 
	//	registered the type because member functions need to refer to the type.

	pType->SetMembers(fnMembers());
	return dwID;
	}

void CAEONTypes::RegisterAEONType (DWORD dwCoreType, CStringView sFullyQualifiedName, CStringView sDatumTypename, std::function<TArray<IDatatype::SMemberDesc>()> fnMembers)
	{
	//	We need to create the type in two phases because some methods refer to the 
	//	type itself.

	auto* pType = new CDatatypeAEON(sFullyQualifiedName, dwCoreType, CDatatypeList(), sDatumTypename);
	CDatum dType(new CComplexDatatype(pType));
	SetType(dType);

	//	NOTE: We can't generate the list of members until AFTER we have 
	//	registered the type because member functions need to refer to the type.

	pType->SetMembers(fnMembers());
	}

void CAEONTypes::RegisterArray (DWORD dwCoreType, CStringView sFullyQualifiedName, CDatum dElementType, bool bForceAnonymous)

//	RegisterArray
//
//	Creates an array type.

	{
	//	NOTE: Most array types (like array of Int32) are marked as "anonymous" 
	//	because we want their name to read as "array of Int32" instead "ArrayOfInt32".

	CDatatypeArray::SCreate Create;
	Create.sFullyQualifiedName = sFullyQualifiedName;
	Create.dwCoreType = dwCoreType;
	Create.dElementType = dElementType;
	Create.bAnonymous = bForceAnonymous;
	Create.bBuiltIn = true;

	CDatum dType(new CComplexDatatype(new CDatatypeArray(Create)));
	SetType(dType);
	}

void CAEONTypes::RegisterDayOfWeekEnum ()
	{
	CString sFullyQualifiedName = MakeFullyQualifiedName(NULL_STR, TYPENAME_DAY_OF_WEEK_ENUM);

	TArray<IDatatype::SMemberDesc> DayOfWeekEnum;
	DayOfWeekEnum.Insert({ IDatatype::EMemberType::EnumValue, ENUM_SUNDAY, CDatum(), INT_MIN, ENUM_SUNDAY_LABEL });
	DayOfWeekEnum.Insert({ IDatatype::EMemberType::EnumValue, ENUM_MONDAY, CDatum(), INT_MIN, ENUM_MONDAY_LABEL });
	DayOfWeekEnum.Insert({ IDatatype::EMemberType::EnumValue, ENUM_TUESDAY, CDatum(), INT_MIN, ENUM_TUESDAY_LABEL });
	DayOfWeekEnum.Insert({ IDatatype::EMemberType::EnumValue, ENUM_WEDNESDAY, CDatum(), INT_MIN, ENUM_WEDNESDAY_LABEL });
	DayOfWeekEnum.Insert({ IDatatype::EMemberType::EnumValue, ENUM_THURSDAY, CDatum(), INT_MIN, ENUM_THURSDAY_LABEL });
	DayOfWeekEnum.Insert({ IDatatype::EMemberType::EnumValue, ENUM_FRIDAY, CDatum(), INT_MIN, ENUM_FRIDAY_LABEL });
	DayOfWeekEnum.Insert({ IDatatype::EMemberType::EnumValue, ENUM_SATURDAY, CDatum(), INT_MIN, ENUM_SATURDAY_LABEL });

	CDatum dType = CreateEnum(IDatatype::DAY_OF_WEEK_ENUM, sFullyQualifiedName, DayOfWeekEnum, true);

	SetType(dType);
	}

void CAEONTypes::RegisterCharSetTypeEnum ()
	{
	CString sFullyQualifiedName = MakeFullyQualifiedName(NULL_STR, TYPENAME_CHAR_SET_TYPE);

	TArray<IDatatype::SMemberDesc> CharSetTypeEnum;
	CharSetTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_UTF8, CDatum(), INT_MIN, STR_CAEONTYPES_UTF8 });
	CharSetTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_UTF16_BE, CDatum(), INT_MIN, STR_CAEONTYPES_UTF16_BE });
	CharSetTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_UTF16_LE, CDatum(), INT_MIN, STR_CAEONTYPES_UTF16_LE });
	CharSetTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_UTF32_BE, CDatum(), INT_MIN, STR_CAEONTYPES_UTF32_BE });
	CharSetTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_UTF32_LE, CDatum(), INT_MIN, STR_CAEONTYPES_UTF32_LE });
	CharSetTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_ASCII, CDatum(), INT_MIN, STR_CAEONTYPES_ASCII });
	CharSetTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_LATIN1, CDatum(), INT_MIN, STR_CAEONTYPES_LATIN1 });
	CharSetTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_WINDOWS_1252, CDatum(), INT_MIN, STR_CAEONTYPES_WINDOWS_1252 });

	CDatum dType = CreateEnum(IDatatype::CHAR_SET_TYPE_ENUM, sFullyQualifiedName, CharSetTypeEnum, true);

	SetType(dType);
	}

void CAEONTypes::RegisterDictionary (DWORD dwCoreType, CStringView sFullyQualifiedName, CDatum dKeyType, CDatum dElementType, bool bForceAnonymous)

//	RegisterDictionary
//
//	Creates an array type.

	{
	CDatatypeArray::SCreate Create;
	Create.sFullyQualifiedName = sFullyQualifiedName;
	Create.dwCoreType = dwCoreType;
	Create.bDictionary = true;
	Create.dElementType = dElementType;
	Create.dKeyType = dKeyType;
	Create.bAnonymous = bForceAnonymous;
	Create.bBuiltIn = true;

	CDatum dType(new CComplexDatatype(new CDatatypeArray(Create)));
	SetType(dType);
	}

DWORD CAEONTypes::RegisterEnum (CStringView sTypename, const TArray<IDatatype::SMemberDesc>& Values)
	{
	CSmartLock Lock(m_cs);

	DWORD dwID = Alloc();
	CDatum dType = CreateEnum(dwID, MakeFullyQualifiedName(NULL_STR, sTypename), Values, true);
	SetType(dType);

	return dwID;
	}

void CAEONTypes::RegisterEncodingTypeEnum ()
	{
	CString sFullyQualifiedName = MakeFullyQualifiedName(NULL_STR, TYPENAME_ENCODING_TYPE);

	TArray<IDatatype::SMemberDesc> EncodingTypeEnum;
	EncodingTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_URL_COMPONENT, CDatum(), INT_MIN, STR_CAEONTYPES_URL_COMPONENT });
	EncodingTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_URL_PATH_SEGMENT, CDatum(), INT_MIN, STR_CAEONTYPES_URL_PATH_SEGMENT });
	EncodingTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_URL_QUERY_VALUE, CDatum(), INT_MIN, STR_CAEONTYPES_URL_QUERY_VALUE });
	EncodingTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_FORM_COMPONENT, CDatum(), INT_MIN, STR_CAEONTYPES_FORM_COMPONENT });
	EncodingTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_JSON_STRING_LITERAL, CDatum(), INT_MIN, STR_CAEONTYPES_JSON_STRING_LITERAL });
	EncodingTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_JSON_STRING_CONTENT, CDatum(), INT_MIN, STR_CAEONTYPES_JSON_STRING_CONTENT });
	EncodingTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_C_STRING_LITERAL, CDatum(), INT_MIN, STR_CAEONTYPES_C_STRING_LITERAL });
	EncodingTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_C_STRING_CONTENT, CDatum(), INT_MIN, STR_CAEONTYPES_C_STRING_CONTENT });
	EncodingTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_HTML_TEXT, CDatum(), INT_MIN, STR_CAEONTYPES_HTML_TEXT });
	EncodingTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_HTML_ATTRIBUTE, CDatum(), INT_MIN, STR_CAEONTYPES_HTML_ATTRIBUTE });
	EncodingTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_BASE64, CDatum(), INT_MIN, STR_CAEONTYPES_BASE64 });
	EncodingTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_BASE64_URL, CDatum(), INT_MIN, STR_CAEONTYPES_BASE64_URL });
	EncodingTypeEnum.Insert({ IDatatype::EMemberType::EnumValue, STR_CAEONTYPES_HEX, CDatum(), INT_MIN, STR_CAEONTYPES_HEX });

	CDatum dType = CreateEnum(IDatatype::ENCODING_TYPE_ENUM, sFullyQualifiedName, EncodingTypeEnum, true);

	SetType(dType);
	}

void CAEONTypes::RegisterInt32SubRange (DWORD dwCoreType, CStringView sFullyQualifiedName, int iMin, int iMax)

//	CreateInt32SubRange
//
//	Creates a sub-range of int32.

	{
	CDatatypeNumber::SCreate Create;
	Create.sFullyQualifiedName = sFullyQualifiedName;
	Create.dwCoreType = dwCoreType;
	Create.Implements = { Get(IDatatype::INT_32) };
	Create.iBits = 32;
	Create.bFloat = false;
	Create.bUnsigned = false;
	Create.bSubRange = true;
	Create.iSubRangeMin = iMin;
	Create.iSubRangeMax = iMax;
	Create.bAbstract = false;
	Create.bCanBeNull = false;
	Create.bBuiltIn = true;

	CDatum dType(new CComplexDatatype(new CDatatypeNumber(Create)));
	SetType(dType);
	}

void CAEONTypes::RegisterMemberTableSchema ()
	{
	CString sFullyQualifiedName = MakeFullyQualifiedName(NULL_STR, TYPENAME_MEMBER_TABLE_SCHEMA);

	TArray<IDatatype::SMemberDesc> Columns;
	Columns.Insert({ IDatatype::EMemberType::InstanceKeyVar, FIELD_ID, Get(IDatatype::STRING) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_MEMBER_TYPE, Get(IDatatype::MEMBER_TYPE_ENUM) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_TYPE, Get(IDatatype::DATATYPE) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_LABEL, Get(IDatatype::STRING) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_DESCRIPTION, Get(IDatatype::STRING) });

	CDatum dType = CreateSchema(IDatatype::MEMBER_TABLE_SCHEMA, sFullyQualifiedName, Columns, true);

	SetType(dType);
	}

void CAEONTypes::RegisterMemberTableType ()
	{
	CDatum dSchema = Get(IDatatype::MEMBER_TABLE_SCHEMA);
	CString sFullyQualifiedName = MakeFullyQualifiedName(NULL_STR, TYPENAME_MEMBER_TABLE);

	CDatum dType = CreateTable(IDatatype::MEMBER_TABLE, sFullyQualifiedName, dSchema, true);

	SetType(dType);
	}

void CAEONTypes::RegisterMemberTypeEnum ()
	{
	CString sFullyQualifiedName = MakeFullyQualifiedName(NULL_STR, TYPENAME_MEMBER_TYPE_ENUM);

	TArray<IDatatype::SMemberDesc> Values;
	Values.Insert({ IDatatype::EMemberType::EnumValue, ENUM_DEFINITION, CDatum(), INT_MIN, ENUM_DEFINITION_LABEL });
	Values.Insert({ IDatatype::EMemberType::EnumValue, ENUM_PROPERTY, CDatum(), INT_MIN, ENUM_PROPERTY_LABEL });
	Values.Insert({ IDatatype::EMemberType::EnumValue, ENUM_FUNCTION, CDatum(), INT_MIN, ENUM_FUNCTION_LABEL });
	Values.Insert({ IDatatype::EMemberType::EnumValue, ENUM_EVENT, CDatum(), INT_MIN, ENUM_EVENT_LABEL });
	Values.Insert({ IDatatype::EMemberType::EnumValue, ENUM_VARIABLE, CDatum(), INT_MIN, ENUM_VARIABLE_LABEL });

	CDatum dType = CreateEnum(IDatatype::MEMBER_TYPE_ENUM, sFullyQualifiedName, Values, true);

	SetType(dType);
	}

void CAEONTypes::RegisterNever ()

//	RegisterNever
//
//	Creates the Never datatype.

	{
	CDatum dType(new CComplexDatatype(new CDatatypeNever()));
	SetType(dType);
	}

void CAEONTypes::RegisterNull ()

//	RegisterNull
//
//	Creates the Null datatype.

	{
	CDatum dType(new CComplexDatatype(new CDatatypeNull(MakeFullyQualifiedName(NULL_STR, TYPENAME_NULL))));
	SetType(dType);
	}

void CAEONTypes::RegisterVoid ()

//	RegisterVoid
//
//	Creates the VoidType datatype.

	{
	RegisterSimple(IDatatype::VOID_T, MakeFullyQualifiedName(NULL_STR, TYPENAME_VOID), { }, false, false, true);
	}

void CAEONTypes::RegisterNumber (DWORD dwCoreType, CStringView sFullyQualifiedName, CDatatypeList&& Implements, int iBits, bool bFloat, bool bUnsigned, bool bAbstract, bool bCanBeNull)

//	RegisterNumber
//
//	Creates a number type.

	{
	CDatatypeNumber::SCreate Create;
	Create.sFullyQualifiedName = sFullyQualifiedName;
	Create.dwCoreType = dwCoreType;
	Create.Implements = std::move(Implements);
	Create.iBits = iBits;
	Create.bFloat = bFloat;
	Create.bUnsigned = bUnsigned;
	Create.bAbstract = bAbstract;
	Create.bCanBeNull = bCanBeNull;
	Create.bBuiltIn = true;

	CDatum dType(new CComplexDatatype(new CDatatypeNumber(Create)));
	SetType(dType);
	}

DWORD CAEONTypes::RegisterSchema (CStringView sTypename, const TArray<IDatatype::SMemberDesc>& Columns)

//	RegisterSchema
//
//	Adds a new schema and returns the ID.

	{
	CSmartLock Lock(m_cs);

	DWORD dwID = Alloc();
	CDatum dType = CreateSchema(dwID, MakeFullyQualifiedName(NULL_STR, sTypename), Columns, true);
	SetType(dType);

	return dwID;
	}

void CAEONTypes::RegisterSchemaTableSchema ()

//	RegisterSchemaTableSchema
//
//	Creates the datatype for the schema for a schema table.

	{
	CString sFullyQualifiedName = MakeFullyQualifiedName(NULL_STR, TYPENAME_SCHEMA_TABLE_SCHEMA);

	TArray<IDatatype::SMemberDesc> Columns;
	Columns.Insert({ IDatatype::EMemberType::InstanceKeyVar, FIELD_ID, Get(IDatatype::STRING) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_KEY, Get(IDatatype::BOOL) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_TYPE, Get(IDatatype::DATATYPE) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_LABEL, Get(IDatatype::STRING) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_ORDINAL, Get(IDatatype::INT_32) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_DESCRIPTION, Get(IDatatype::STRING) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_FORMAT, Get(IDatatype::STRING) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_UI, Get(IDatatype::STRING) });

	CDatum dType = CreateSchema(IDatatype::SCHEMA_TABLE_SCHEMA, sFullyQualifiedName, Columns, true);

	SetType(dType);
	}

void CAEONTypes::RegisterSchemaTableType ()

//	RegisterSchemaTable
//
//	Creates the datatype for a schema table.

	{
	CDatum dSchema = Get(IDatatype::SCHEMA_TABLE_SCHEMA);
	CString sFullyQualifiedName = MakeFullyQualifiedName(NULL_STR, TYPENAME_SCHEMA_TABLE);

	CDatum dType = CreateTable(IDatatype::SCHEMA_TABLE, sFullyQualifiedName, dSchema, true);

	SetType(dType);
	}

DWORD CAEONTypes::RegisterSimple (CStringView sTypename, CDatatypeList&& Implements, bool bAbstract)
	{
	CSmartLock Lock(m_cs);

	DWORD dwID = Alloc();
	RegisterSimple(dwID, MakeFullyQualifiedName(NULL_STR, sTypename), std::move(Implements), bAbstract, false, false);

	return dwID;
	}

void CAEONTypes::RegisterSimple (DWORD dwCoreType, CStringView sFullyQualifiedName, CDatatypeList&& Implements, bool bAbstract, bool bCanBeNull, bool bNoMembers)
	{
	CDatatypeSimple::SCreate Create;
	Create.sFullyQualifiedName = sFullyQualifiedName;
	Create.dwCoreType = dwCoreType;
	Create.Implements = std::move(Implements);
	Create.bAbstract = bAbstract;
	Create.bCanBeNull = bCanBeNull;
	Create.bNoMembers = bNoMembers;
	Create.bBuiltIn = true;

	CDatum dType(new CComplexDatatype(new CDatatypeSimple(Create)));
	SetType(dType);
	}

void CAEONTypes::RegisterSimpleEx (DWORD dwCoreType, CStringView sFullyQualifiedName, CDatatypeList&& Implements, CDatatypeList&& ConstructFrom, bool bAbstract, bool bCanBeNull, bool bNoMembers)
	{
	CDatatypeSimple::SCreate Create;
	Create.sFullyQualifiedName = sFullyQualifiedName;
	Create.dwCoreType = dwCoreType;
	Create.Implements = std::move(Implements);
	Create.ConstructFrom = std::move(ConstructFrom);
	Create.bAbstract = bAbstract;
	Create.bCanBeNull = bCanBeNull;
	Create.bNoMembers = bNoMembers;
	Create.bBuiltIn = true;

	CDatum dType(new CComplexDatatype(new CDatatypeSimple(Create)));
	SetType(dType);
	}

void CAEONTypes::RegisterStringType ()

//	RegisterStringType
//
//	Create a simple type.

	{
	CDatum dType(new CComplexDatatype(new CDatatypeString(MakeFullyQualifiedName(NULL_STR, TYPENAME_STRING))));
	SetType(dType);
	}

void CAEONTypes::RegisterTensor (DWORD dwCoreType, CStringView sFullyQualifiedName, CDatum dElementType, int iRows, int iCols)

//	RegisterTensor
//
//	Creates a tensor (multidimensional array) type.

	{
	CDatatypeTensor::SCreate Create;
	Create.sFullyQualifiedName = sFullyQualifiedName;
	Create.dwCoreType = dwCoreType;
	Create.dElementType = dElementType;
	Create.Dimensions = CDatatypeTensor::MakeDimensions(iRows, iCols);
	Create.bBuiltIn = true;

	CDatum dType(new CComplexDatatype(new CDatatypeTensor(Create)));
	SetType(dType);
	}

void CAEONTypes::SetType (CDatum dType)
	{
	ASSERT(m_cs.IsLockedByCurrentThread());

	const IDatatype& Type = dType;
	DWORD dwID = Type.GetCoreType();
	if (dwID)
		m_Types[dwID] = dType;

	if (Type.IsBuiltIn())
		{
		ASSERT(dwID != 0);
		ASSERT(!Type.GetFullyQualifiedName().IsEmpty());

		bool bNew;
		m_BuiltInTypes.SetAt(strToLower(Type.GetFullyQualifiedName()), dwID, &bNew);
		ASSERT(bNew);
		}
	}
