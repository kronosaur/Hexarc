//	CAEONTypes.cpp
//
//	CAEONTypes class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(DATUM_TYPENAME_BINARY,				"binary");
DECLARE_CONST_STRING(DATUM_TYPENAME_DATATYPE,			"datatype")
DECLARE_CONST_STRING(DATUM_TYPENAME_GRID_NAME,			"gridName");
DECLARE_CONST_STRING(DATUM_TYPENAME_MAP_COLUMN_EXPRESSION,"mapColumnExpression");
DECLARE_CONST_STRING(DATUM_TYPENAME_RANGE,				"range");
DECLARE_CONST_STRING(DATUM_TYPENAME_STRING_FORMAT,		"stringFormat");

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
DECLARE_CONST_STRING(TYPENAME_BINARY,					"Binary");
DECLARE_CONST_STRING(TYPENAME_BITMAP_RGBA8,				"BitmapOfRGBA8");
DECLARE_CONST_STRING(TYPENAME_BOOL,						"Bool");
DECLARE_CONST_STRING(TYPENAME_CANVAS,					"Canvas");
DECLARE_CONST_STRING(TYPENAME_CLASS_T,					"ClassType");
DECLARE_CONST_STRING(TYPENAME_DATATYPE,					"Datatype");
DECLARE_CONST_STRING(TYPENAME_DATE_TIME,				"DateTime");
DECLARE_CONST_STRING(TYPENAME_DAY_OF_WEEK_ENUM,			"DayOfWeek");
DECLARE_CONST_STRING(TYPENAME_DICTIONARY,				"Dictionary");
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
DECLARE_CONST_STRING(TYPENAME_VECTOR_2D_F64,			"Vector2DOfFloat64");
DECLARE_CONST_STRING(TYPENAME_VECTOR_3D_F64,			"Vector3DOfFloat64");

DECLARE_CONST_STRING(STR_ARG,							"arg");
DECLARE_CONST_STRING(STR_DOLLAR,						"$");

CCriticalSection CAEONTypes::m_cs;
TArray<CDatum> CAEONTypes::m_Types;
TSortMap<CString, DWORD> CAEONTypes::m_CoreTypes;
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
			CDatum dNullableType = CAEONTypeSystem::CreateNullableType(NULL_STR, dType);
			const IDatatype& NullableType = dNullableType;
			retTypes.SetAt(NullableType.GetFullyQualifiedName(), dNullableType);
			}
		}
	}

DWORD CAEONTypes::AddCoreAEON (CStringView sName, const CDatatypeList& Implement, CStringView sDatumTypename, TArray<IDatatype::SMemberDesc>&& Members, bool bCore)
	{
	CSmartLock Lock(m_cs);

	DWORD dwID = Alloc();
	auto* pType = new CDatatypeAEON(MakeFullyQualifiedName(NULL_STR, sName), dwID, Implement, sDatumTypename, std::move(Members));

	CDatum dType = CDatum(new CComplexDatatype(pType));

	SetType(dwID, dType, bCore);
	return dwID;
	}

DWORD CAEONTypes::AddCoreEnum (const CString& sName, const TArray<IDatatype::SMemberDesc>& Values)
	{
	return AddEnum(MakeFullyQualifiedName(NULL_STR, sName), Values, true);
	}

DWORD CAEONTypes::AddCoreSchema (const CString& sName, const TArray<IDatatype::SMemberDesc>& Columns)
	{
	return AddSchema(MakeFullyQualifiedName(NULL_STR, sName), Columns, true);
	}

DWORD CAEONTypes::AddCoreSimple (const CString& sName, const CDatatypeList& Implements, bool bAbstract)
	{
	return AddSimple(MakeFullyQualifiedName(NULL_STR, sName), Implements, bAbstract, true);
	}

DWORD CAEONTypes::AddEnum (const CString& sFullyQualifiedName, const TArray<IDatatype::SMemberDesc>& Values, bool bCore)

//	AddEnum
//
//	Adds a new enum type and returns the ID.

	{
	CSmartLock Lock(m_cs);

	//	Create the new type

	DWORD dwID = Alloc();
	CDatum dEnum = CreateEnum(sFullyQualifiedName, Values, dwID);

	//	Add it to our store

	SetType(dwID, dEnum, bCore);
	return dwID;
	}

DWORD CAEONTypes::AddSchema (const CString& sFullyQualifiedName, const TArray<IDatatype::SMemberDesc>& Columns, bool bCore)

//	AddSchema
//
//	Adds a new schema and returns the ID.

	{
	CSmartLock Lock(m_cs);

	//	Create the new schema.

	DWORD dwID = Alloc();
	CDatum dSchema = CreateSchema(sFullyQualifiedName, Columns, dwID);

	//	Add it to our store

	SetType(dwID, dSchema, bCore);
	return dwID;
	}

DWORD CAEONTypes::AddSimple (const CString& sFullyQualifiedName, const CDatatypeList& Implements, bool bAbstract, bool bCore)

//	AddSimple
//
//	Adds a new simple type and returns the ID.

	{
	CSmartLock Lock(m_cs);

	DWORD dwID = Alloc();
	CDatum dType = CreateSimple(sFullyQualifiedName, Implements, bAbstract, false, dwID);

	SetType(dwID, dType, bCore);
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

CDatum CAEONTypes::CreateAny ()

//	CreateAny
//
//	Creates the Any datatype.

	{
	return CDatum(new CComplexDatatype(new CDatatypeAny(MakeFullyQualifiedName(NULL_STR, TYPENAME_ANY))));
	}

CDatum CAEONTypes::CreateArray (const CString& sFullyQualifiedName, CDatum dElementType, DWORD dwCoreType, bool bForceAnonymous)

//	CreateArray
//
//	Creates an array type.

	{
	return CDatum(new CComplexDatatype(new CDatatypeArray(
		{
			sFullyQualifiedName, 
			dwCoreType,
			dElementType,
			false,

			false,
			CDatum(),
			bForceAnonymous
			})
		));
	}

CDatum CAEONTypes::CreateDayOfWeekEnum ()
	{
	CString sFullyQualifiedName = MakeFullyQualifiedName(NULL_STR, TYPENAME_DAY_OF_WEEK_ENUM);

	TArray<IDatatype::SMemberDesc> DayOfWeekEnum;
	DayOfWeekEnum.Insert({ IDatatype::EMemberType::EnumValue, ENUM_SUNDAY, CDatum(), -1, ENUM_SUNDAY_LABEL });
	DayOfWeekEnum.Insert({ IDatatype::EMemberType::EnumValue, ENUM_MONDAY, CDatum(), -1, ENUM_MONDAY_LABEL });
	DayOfWeekEnum.Insert({ IDatatype::EMemberType::EnumValue, ENUM_TUESDAY, CDatum(), -1, ENUM_TUESDAY_LABEL });
	DayOfWeekEnum.Insert({ IDatatype::EMemberType::EnumValue, ENUM_WEDNESDAY, CDatum(), -1, ENUM_WEDNESDAY_LABEL });
	DayOfWeekEnum.Insert({ IDatatype::EMemberType::EnumValue, ENUM_THURSDAY, CDatum(), -1, ENUM_THURSDAY_LABEL });
	DayOfWeekEnum.Insert({ IDatatype::EMemberType::EnumValue, ENUM_FRIDAY, CDatum(), -1, ENUM_FRIDAY_LABEL });
	DayOfWeekEnum.Insert({ IDatatype::EMemberType::EnumValue, ENUM_SATURDAY, CDatum(), -1, ENUM_SATURDAY_LABEL });

	return CreateEnum(sFullyQualifiedName, DayOfWeekEnum, IDatatype::DAY_OF_WEEK_ENUM);
	}

CDatum CAEONTypes::CreateDictionary (const CString& sFullyQualifiedName, CDatum dKeyType, CDatum dElementType, DWORD dwCoreType)

//	CreateDictionary
//
//	Creates a dictionary type.

	{
	return CDatum(new CComplexDatatype(new CDatatypeArray(
		{
			sFullyQualifiedName, 
			dwCoreType,
			dElementType,
			false,

			true,
			dKeyType,
			})
		));
	}

CDatum CAEONTypes::CreateEnum (const CString& sFullyQualifiedName, const TArray<IDatatype::SMemberDesc>& Values, DWORD dwCoreType)

//	CreateEnum
//
//	Creates an enum type.

	{
	IDatatype* pNewType = new CDatatypeEnum(CDatatypeEnum::SCreate({ sFullyQualifiedName, dwCoreType }));
	CDatum dType(new CComplexDatatype(pNewType));

	for (int i = 0; i < Values.GetCount(); i++)
		{
		IDatatype::SMemberDesc Desc = Values[i];
		Desc.iType = IDatatype::EMemberType::EnumValue;
		Desc.dType = CDatum();
		if (Desc.iOrdinal == -1)
			Desc.iOrdinal = i;

		if (!pNewType->AddMember(Desc))
			throw CException(errFail);
		}

	return dType;
	}

CDatum CAEONTypes::CreateFunctionType (const CString& sArgCode)

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

	return CAEONTypeSystem::CreateDatatypeFunction(CAEONTypes::MakeFullyQualifiedFunctionName(), Return, Args);
	}

CDatum CAEONTypes::CreateInt32SubRange (const CString& sFullyQualifiedName, int iMin, int iMax, DWORD dwCoreType)

//	CreateInt32SubRange
//
//	Creates a sub-range of int32.

	{
	return CDatum(new CComplexDatatype(new CDatatypeNumber(
		{
			sFullyQualifiedName, 
			dwCoreType,
			{ Get(IDatatype::INT_32) },
			32,
			false,
			false,
			true,
			iMin,
			iMax
		})));
	}

CDatum CAEONTypes::CreateTensor (CStringView sFullyQualifiedName, CDatum dElementType, const TArray<CDatum>& Dimensions, DWORD dwCoreType)
	{
	return CDatum(new CComplexDatatype(new CDatatypeTensor({
			CString(sFullyQualifiedName), 
			dwCoreType,
			dElementType,
			Dimensions,
			})
		));
	}

CDatum CAEONTypes::CreateTensor (const CString& sFullyQualifiedName, CDatum dElementType, int iRows, int iCols, DWORD dwCoreType)

//	CreateTensor
//
//	Creates a tensor (multidimensional array) type.

	{
	return CDatum(new CComplexDatatype(new CDatatypeTensor(
			sFullyQualifiedName, 
			dElementType,
			iRows,
			iCols,
			dwCoreType
			)
		));
	}

CDatum CAEONTypes::CreateMemberTableType ()
	{
	CDatum dSchema = Get(IDatatype::MEMBER_TABLE_SCHEMA);

	CString sFullyQualifiedName = MakeFullyQualifiedName(NULL_STR, TYPENAME_MEMBER_TABLE);
	IDatatype *pNewType = new CDatatypeArray(CDatatypeArray::SCreate({ sFullyQualifiedName, IDatatype::MEMBER_TABLE, dSchema, true }));

	return CDatum(new CComplexDatatype(pNewType));
	}

CDatum CAEONTypes::CreateMemberTableSchema ()
	{
	CString sFullyQualifiedName = MakeFullyQualifiedName(NULL_STR, TYPENAME_MEMBER_TABLE_SCHEMA);

	TArray<IDatatype::SMemberDesc> Columns;
	Columns.Insert({ IDatatype::EMemberType::InstanceKeyVar, FIELD_ID, Get(IDatatype::STRING) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_MEMBER_TYPE, Get(IDatatype::MEMBER_TYPE_ENUM) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_TYPE, Get(IDatatype::DATATYPE) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_LABEL, Get(IDatatype::STRING) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_DESCRIPTION, Get(IDatatype::STRING) });

	return CreateSchema(sFullyQualifiedName, Columns, IDatatype::MEMBER_TABLE_SCHEMA);
	}

CDatum CAEONTypes::CreateMemberTypeEnum ()
	{
	CString sFullyQualifiedName = MakeFullyQualifiedName(NULL_STR, TYPENAME_MEMBER_TYPE_ENUM);

	TArray<IDatatype::SMemberDesc> Values;
	Values.Insert({ IDatatype::EMemberType::EnumValue, ENUM_DEFINITION, CDatum(), -1, ENUM_DEFINITION_LABEL });
	Values.Insert({ IDatatype::EMemberType::EnumValue, ENUM_PROPERTY, CDatum(), -1, ENUM_PROPERTY_LABEL });
	Values.Insert({ IDatatype::EMemberType::EnumValue, ENUM_FUNCTION, CDatum(), -1, ENUM_FUNCTION_LABEL });
	Values.Insert({ IDatatype::EMemberType::EnumValue, ENUM_EVENT, CDatum(), -1, ENUM_EVENT_LABEL });
	Values.Insert({ IDatatype::EMemberType::EnumValue, ENUM_VARIABLE, CDatum(), -1, ENUM_VARIABLE_LABEL });

	return CreateEnum(sFullyQualifiedName, Values, IDatatype::MEMBER_TYPE_ENUM);
	}

CDatum CAEONTypes::CreateNumber (const CString& sFullyQualifiedName, const CDatatypeList& Implements, int iBits, bool bFloat, bool bUnsigned, DWORD dwCoreType, bool bAbstract, bool bCanBeNull)

//	CreateNumber
//
//	Creates a number type.

	{
	return CDatum(new CComplexDatatype(new CDatatypeNumber(
		{
			sFullyQualifiedName, 
			dwCoreType,
			Implements,
			iBits,
			bFloat,
			bUnsigned,

			false,
			0,
			0,

			bAbstract,
			bCanBeNull,
			})
		));
	}

CDatum CAEONTypes::CreateNull ()

//	CreateNull
//
//	Creates the Null datatype.

	{
	return CDatum(new CComplexDatatype(new CDatatypeNull(MakeFullyQualifiedName(NULL_STR, TYPENAME_NULL))));
	}

CDatum CAEONTypes::CreatePropertyType (const char* pPos)
	{
	return ParseTypeFromArgCode(pPos);
	}

CDatum CAEONTypes::CreateSchema (const CString& sFullyQualifiedName, const TArray<IDatatype::SMemberDesc>& Columns, DWORD dwCoreType)

//	CreateSchema
//
//	Creates a schema datatype.

	{
	IDatatype *pNewType = new CDatatypeSchema(CDatatypeSchema::SCreate({ sFullyQualifiedName, { Get(IDatatype::SCHEMA) }, dwCoreType }));

	for (int i = 0; i < Columns.GetCount(); i++)
		{
		if (Columns[i].iType != IDatatype::EMemberType::InstanceKeyVar
				&& Columns[i].iType != IDatatype::EMemberType::InstanceVar)
			throw CException(errFail);

		if (!pNewType->AddMember(Columns[i]))
			throw CException(errFail);
		}

	return CDatum(new CComplexDatatype(pNewType));
	}

CDatum CAEONTypes::CreateSchemaTable ()

//	CreateSchemaTable
//
//	Creates the datatype for a schema table.

	{
	CDatum dSchema = Get(IDatatype::SCHEMA_TABLE_SCHEMA);

	CString sFullyQualifiedName = MakeFullyQualifiedName(NULL_STR, TYPENAME_SCHEMA_TABLE);
	IDatatype *pNewType = new CDatatypeArray(CDatatypeArray::SCreate({ sFullyQualifiedName, IDatatype::SCHEMA_TABLE, dSchema, true }));

	return CDatum(new CComplexDatatype(pNewType));
	}

CDatum CAEONTypes::CreateSchemaTableSchema ()

//	CreateSchemaTableSchema
//
//	Creates the datatype for the schema for a schema table.

	{
	CString sSchemaName = MakeFullyQualifiedName(NULL_STR, TYPENAME_SCHEMA_TABLE_SCHEMA);

	TArray<IDatatype::SMemberDesc> Columns;
	Columns.Insert({ IDatatype::EMemberType::InstanceKeyVar, FIELD_ID, Get(IDatatype::STRING) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_KEY, Get(IDatatype::BOOL) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_TYPE, Get(IDatatype::DATATYPE) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_LABEL, Get(IDatatype::STRING) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_ORDINAL, Get(IDatatype::INT_32) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_DESCRIPTION, Get(IDatatype::STRING) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_FORMAT, Get(IDatatype::STRING) });
	Columns.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_UI, Get(IDatatype::STRING) });

	return CreateSchema(sSchemaName, Columns, IDatatype::SCHEMA_TABLE_SCHEMA);
	}

CDatum CAEONTypes::CreateSimple (const CString& sFullyQualifiedName, const CDatatypeList& Implements, bool bAbstract, bool bCanBeNull, DWORD dwCoreType, bool bNoMembers)

//	CreateSimple
//
//	Create a simple type.

	{
	return CDatum(new CComplexDatatype(new CDatatypeSimple(
		{
			sFullyQualifiedName, 
			dwCoreType,
			Implements,
			bAbstract,
			bCanBeNull,
			bNoMembers,
			})
		));
	}

void CAEONTypes::InitGridNameType ()
	{
	//	We need to create the type in two phases because some methods refer to the 
	//	type itself.

	auto* pType = new CDatatypeAEON(MakeFullyQualifiedName(NULL_STR, TYPENAME_GRID_NAME_TYPE), IDatatype::GRID_NAME_TYPE, CDatatypeList({ IDatatype::STRING }), DATUM_TYPENAME_GRID_NAME, TArray<IDatatype::SMemberDesc>(), true);
	SetCoreType(IDatatype::GRID_NAME_TYPE, CDatum(new CComplexDatatype(pType)));

	//	NOTE: We need to set these members AFTER we create the type because the
	//	members refer to the type itself.

	TArray<IDatatype::SMemberDesc> Members;
	Members.Insert({ IDatatype::EMemberType::InstanceProperty, CString("id"), Get(IDatatype::STRING), 0, CString("ID") });
	Members.Insert({ IDatatype::EMemberType::InstanceProperty, CString("name"), Get(IDatatype::STRING), 1, CString("Name") });

	pType->SetMembers(std::move(Members));
	}

CDatum CAEONTypes::CreateStringType (const CString& sFullyQualifiedName)

//	CreateStringType
//
//	Create a simple type.

	{
	return CDatum(new CComplexDatatype(new CDatatypeString(sFullyQualifiedName)));
	}

CDatum CAEONTypes::FindCoreType (const CString& sFullyQualifiedName, const IDatatype** retpDatatype)

//	FindCoreType
//
//	Returns a core type (or null)

	{
	CSmartLock Lock(m_cs);

	if (!m_bInitDone)
		InitCoreTypes();

	auto pEntry = m_CoreTypes.GetAt(strToLower(sFullyQualifiedName));
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

	DWORD dwID = Alloc();
	SetType(dwID, dType, false);

	//	Make sure the type itself has an ID. This is a HACK because we're 
	//	casting, but it is OK because we own these objects.

	const_cast<IDatatype&>(TypeToFind).SetCoreType(dwID);
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

	SetType(dwID, dTable, false);
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

			//	Otherwise, we have no choice but to default to Any.

			else
				return CAEONTypeSystem::GetCoreType(IDatatype::ANY);
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

	SetCoreType(IDatatype::ANY, CreateAny());
	SetCoreType(IDatatype::BOOL, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_BOOL), { }, true, false, IDatatype::BOOL, true));
	SetCoreAEONType(IDatatype::DATATYPE, TYPENAME_DATATYPE, DATUM_TYPENAME_DATATYPE, CComplexDatatype::GetMembers);
	SetCoreType(IDatatype::DAY_OF_WEEK_ENUM, CreateDayOfWeekEnum());
	SetCoreType(IDatatype::NULL_T, CreateNull());
	SetCoreType(IDatatype::ERROR_T, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_ERROR), { }, false, false, IDatatype::ERROR_T));
	SetCoreType(IDatatype::NUMBER, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_NUMBER), { }, true, true, IDatatype::NUMBER, true));
	SetCoreType(IDatatype::REAL, CreateNumber(MakeFullyQualifiedName(NULL_STR, TYPENAME_REAL), { IDatatype::NUMBER }, 0, true, false, IDatatype::REAL, true, true));
	SetCoreType(IDatatype::INTEGER, CreateNumber(MakeFullyQualifiedName(NULL_STR, TYPENAME_INTEGER), { IDatatype::NUMBER }, 0, false, false, IDatatype::INTEGER, true, true));
	SetCoreType(IDatatype::FLOAT, CreateNumber(MakeFullyQualifiedName(NULL_STR, TYPENAME_FLOAT), { IDatatype::REAL }, 0, true, false, IDatatype::FLOAT, true, false));
	SetCoreType(IDatatype::SIGNED, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_SIGNED), { IDatatype::INTEGER }, true, true, IDatatype::SIGNED, true));
	SetCoreType(IDatatype::UNSIGNED, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_UNSIGNED), { IDatatype::INTEGER }, true, true, IDatatype::UNSIGNED, true));
	SetCoreType(IDatatype::INT_32, CreateNumber(MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_32), { IDatatype::SIGNED }, 32, false, false, IDatatype::INT_32));
	SetCoreType(IDatatype::INT_64, CreateNumber(MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_64), { IDatatype::SIGNED }, 64, false, false, IDatatype::INT_64));
	SetCoreType(IDatatype::INT_IP, CreateNumber(MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_IP), { IDatatype::SIGNED }, 0, false, false, IDatatype::INT_IP));
	SetCoreType(IDatatype::UINT_32, CreateNumber(MakeFullyQualifiedName(NULL_STR, TYPENAME_UINT_32), { IDatatype::UNSIGNED }, 32, false, true, IDatatype::UINT_32));
	SetCoreType(IDatatype::UINT_64, CreateNumber(MakeFullyQualifiedName(NULL_STR, TYPENAME_UINT_64), { IDatatype::UNSIGNED }, 64, false, true, IDatatype::UINT_64));
	SetCoreType(IDatatype::INT_8, CreateInt32SubRange(MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_8), -128, 127, IDatatype::INT_8));
	SetCoreType(IDatatype::INT_16, CreateInt32SubRange(MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_16), -32768, 32767, IDatatype::INT_16));
	SetCoreType(IDatatype::UINT_8, CreateInt32SubRange(MakeFullyQualifiedName(NULL_STR, TYPENAME_UINT_8), 0, 255, IDatatype::UINT_8));
	SetCoreType(IDatatype::UINT_16, CreateInt32SubRange(MakeFullyQualifiedName(NULL_STR, TYPENAME_UINT_16), 0, 65535, IDatatype::UINT_16));
	SetCoreType(IDatatype::NAN_CONST, CreateNumber(MakeFullyQualifiedName(NULL_STR, TYPENAME_NAN), { IDatatype::FLOAT }, 64, true, false, IDatatype::NAN_CONST));
	SetCoreType(IDatatype::FLOAT_64, CreateNumber(MakeFullyQualifiedName(NULL_STR, TYPENAME_FLOAT_64), { IDatatype::FLOAT }, 64, true, false, IDatatype::FLOAT_64));
	SetCoreType(IDatatype::ABSTRACT_DICTIONARY, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_ABSTRACT_DICTIONARY), { }, true, false, IDatatype::ABSTRACT_DICTIONARY));
	SetCoreType(IDatatype::INDEXED, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_INDEXED), { }, true, false, IDatatype::INDEXED));
	SetCoreAEONType(IDatatype::RANGE, TYPENAME_RANGE, DATUM_TYPENAME_RANGE, CAEONRange::GetMembers);
	SetCoreType(IDatatype::ABSTRACT_MUTABLE_DICTIONARY, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_ABSTRACT_MUTABLE_DICTIONARY), { }, true, false, IDatatype::ABSTRACT_MUTABLE_DICTIONARY));
	SetCoreType(IDatatype::MUTABLE_INDEXED, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_MUTABLE_INDEXED), { }, true, false, IDatatype::MUTABLE_INDEXED));
	SetCoreType(IDatatype::STRING, CreateStringType(MakeFullyQualifiedName(NULL_STR, TYPENAME_STRING)));
	SetCoreType(IDatatype::ARRAY, CreateArray(MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY), Get(IDatatype::ANY), IDatatype::ARRAY));
	SetCoreType(IDatatype::STRUCT, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_STRUCT), { IDatatype::ABSTRACT_DICTIONARY, IDatatype::ABSTRACT_MUTABLE_DICTIONARY }, false, false, IDatatype::STRUCT));
	SetCoreType(IDatatype::SCHEMA, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_SCHEMA), { IDatatype::STRUCT }, true, false, IDatatype::SCHEMA));
	SetCoreType(IDatatype::DICTIONARY, CreateDictionary(MakeFullyQualifiedName(NULL_STR, TYPENAME_DICTIONARY), Get(IDatatype::ANY), Get(IDatatype::ANY), IDatatype::DICTIONARY));
	SetCoreType(IDatatype::DATE_TIME, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_DATE_TIME), { }, false, false, IDatatype::DATE_TIME));
	SetCoreType(IDatatype::TIME_SPAN, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_TIME_SPAN), { }, false, false, IDatatype::TIME_SPAN));
	SetCoreAEONType(IDatatype::BINARY, TYPENAME_BINARY, DATUM_TYPENAME_BINARY, CComplexBinary::GetMembers);
	SetCoreType(IDatatype::FUNCTION, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_FUNCTION), { }, true, false, IDatatype::FUNCTION));
	SetCoreType(IDatatype::EXPRESSION, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_EXPRESSION), { }, false, true, IDatatype::EXPRESSION));
	SetCoreType(IDatatype::CLASS_T, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_CLASS_T), { IDatatype::ABSTRACT_DICTIONARY, IDatatype::ABSTRACT_MUTABLE_DICTIONARY }, true, false, IDatatype::CLASS_T));
	SetCoreType(IDatatype::OBJECT, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_OBJECT), { IDatatype::ABSTRACT_DICTIONARY, IDatatype::ABSTRACT_MUTABLE_DICTIONARY }, true, false, IDatatype::OBJECT));
	SetCoreType(IDatatype::TABLE, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_TABLE), { IDatatype::ABSTRACT_DICTIONARY, IDatatype::INDEXED, IDatatype::ABSTRACT_MUTABLE_DICTIONARY, IDatatype::MUTABLE_INDEXED  }, true, false, IDatatype::TABLE));
	SetCoreType(IDatatype::ARRAY_INT_32, CreateArray(MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_INT_32), Get(IDatatype::INT_32), IDatatype::ARRAY_INT_32, true));
	SetCoreType(IDatatype::ARRAY_FLOAT_64, CreateArray(MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_FLOAT_64), Get(IDatatype::FLOAT_64), IDatatype::ARRAY_FLOAT_64, true));
	SetCoreType(IDatatype::ARRAY_STRING, CreateArray(MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_STRING), Get(IDatatype::STRING), IDatatype::ARRAY_STRING, true));
	SetCoreType(IDatatype::ARRAY_DATE_TIME, CreateArray(MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_DATE_TIME), Get(IDatatype::DATE_TIME), IDatatype::ARRAY_DATE_TIME, true));
	SetCoreType(IDatatype::ARRAY_INT_64, CreateArray(MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_INT_64), Get(IDatatype::INT_64), IDatatype::ARRAY_INT_64, true));
	SetCoreType(IDatatype::ARRAY_INT_IP, CreateArray(MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_INT_IP), Get(IDatatype::INT_IP), IDatatype::ARRAY_INT_IP, true));
	SetCoreType(IDatatype::ARRAY_NUMBER, CreateArray(MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_NUMBER), Get(IDatatype::NUMBER), IDatatype::ARRAY_NUMBER, true));
	SetCoreType(IDatatype::MEMBER_TYPE_ENUM, CreateMemberTypeEnum());
	SetCoreType(IDatatype::MEMBER_TABLE_SCHEMA, CreateMemberTableSchema());
	SetCoreType(IDatatype::MEMBER_TABLE, CreateMemberTableType());
	SetCoreType(IDatatype::SCHEMA_TABLE_SCHEMA, CreateSchemaTableSchema());
	SetCoreType(IDatatype::SCHEMA_TABLE, CreateSchemaTable());
	SetCoreType(IDatatype::VECTOR_2D_F64, CreateTensor(MakeFullyQualifiedName(NULL_STR, TYPENAME_VECTOR_2D_F64), Get(IDatatype::FLOAT_64), 1, 2, IDatatype::VECTOR_2D_F64));
	SetCoreType(IDatatype::VECTOR_3D_F64, CreateTensor(MakeFullyQualifiedName(NULL_STR, TYPENAME_VECTOR_3D_F64), Get(IDatatype::FLOAT_64), 1, 3, IDatatype::VECTOR_3D_F64));
	SetCoreType(IDatatype::MATRIX_F64, CreateTensor(MakeFullyQualifiedName(NULL_STR, TYPENAME_MATRIX_F64), Get(IDatatype::FLOAT_64), 0, 0, IDatatype::MATRIX_F64));
	SetCoreType(IDatatype::MATRIX_3X3_F64, CreateTensor(MakeFullyQualifiedName(NULL_STR, TYPENAME_MATRIX_3X3_F64), Get(IDatatype::FLOAT_64), 3, 3, IDatatype::MATRIX_3X3_F64));
	SetCoreType(IDatatype::MATRIX_4X4_F64, CreateTensor(MakeFullyQualifiedName(NULL_STR, TYPENAME_MATRIX_4X4_F64), Get(IDatatype::FLOAT_64), 4, 4, IDatatype::MATRIX_4X4_F64));
	SetCoreType(IDatatype::CANVAS, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_CANVAS), { }, false, false, IDatatype::CANVAS));
	SetCoreType(IDatatype::BITMAP_RGBA8, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_BITMAP_RGBA8), { }, false, false, IDatatype::BITMAP_RGBA8));
	SetCoreType(IDatatype::ENUM, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_ENUM), { }, true, false, IDatatype::ENUM));
	SetCoreType(IDatatype::TEXT_LINES, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_TEXT_LINES), { }, false, false, IDatatype::TEXT_LINES));
	SetCoreType(IDatatype::SAS_DATE_TIME, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_SAS_DATE_TIME), { }, false, false, IDatatype::SAS_DATE_TIME));
	SetCoreType(IDatatype::SAS_DATE, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_SAS_DATE), { }, false, false, IDatatype::SAS_DATE));
	SetCoreType(IDatatype::SAS_TIME, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_SAS_TIME), { }, false, false, IDatatype::SAS_TIME));
	SetCoreAEONType(IDatatype::STRING_FORMAT_TYPE, TYPENAME_STRING_FORMAT_TYPE, DATUM_TYPENAME_STRING_FORMAT, CAEONStringFormat::GetMembers);
	SetCoreAEONType(IDatatype::MAP_COLUMN_EXPRESSION, TYPENAME_MAP_COLUMN_EXPRESSION, DATUM_TYPENAME_MAP_COLUMN_EXPRESSION, CAEONMapColumnExpression::GetMembers);
	InitGridNameType();

#ifdef DEBUG
	if (!m_Types[IDatatype::UNKNOWN].IsIdenticalToNil())
		throw CException(errFail);

	for (int i = 1; i <= IDatatype::MAX_CORE_TYPE; i++)
		if (m_Types[i].GetBasicType() != CDatum::typeDatatype)
			throw CException(errFail);
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
	if (m_Types.GetCount() == 0)
		return;

	//	No need to lock because garbage collection is always single-threaded.

	//	Mark all core types.

	for (int i = 0; i < m_CoreTypes.GetCount(); i++)
		m_Types[m_CoreTypes[i]].Mark();

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

			return FindCoreType(sTypename);
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

				CDatum dSchema = FindCoreType(sTypename);
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
				return CreateFunctionType(CString(pStart, pPos - pStart));
			}

		default:
			return CDatum();
		}
	}

void CAEONTypes::SetCoreAEONType (DWORD dwCoreType, CStringView sTypename, CStringView sDatumTypename, std::function<TArray<IDatatype::SMemberDesc>()> fnMembers)
	{
	//	We need to create the type in two phases because some methods refer to the 
	//	type itself.

	auto* pType = new CDatatypeAEON(MakeFullyQualifiedName(NULL_STR, sTypename), dwCoreType, CDatatypeList(), sDatumTypename);
	SetCoreType(dwCoreType, CDatum(new CComplexDatatype(pType)));

	//	NOTE: We can't generate the list of members until AFTER we have 
	//	registered the type because member functions need to refer to the type.

	pType->SetMembers(fnMembers());
	}

void CAEONTypes::SetType (DWORD dwID, CDatum dType, bool bCore)

//	SetType
//
//	Sets a type (must be locked).

	{
#ifdef DEBUG
	if (dwID == 0 || dwID >= (DWORD)m_Types.GetCount())
		throw CException(errFail);

	if (!m_Types[dwID].IsIdenticalToNil())
		throw CException(errFail);

	if (bCore && dwID <= IDatatype::MAX_CORE_TYPE)
		{
		const IDatatype& Datatype = dType;
		if (Datatype.GetCoreType() != dwID)
			throw CException(errFail);
		}
#endif

	m_Types[dwID] = dType;

	if (bCore)
		{
		const IDatatype& Datatype = dType;
		m_CoreTypes.SetAt(strToLower(Datatype.GetFullyQualifiedName()), dwID);
		}
	}
