//	CAEONTypeSystem.cpp
//
//	CAEONTypeSystem class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DATATYPE,					"datatype");
DECLARE_CONST_STRING(FIELD_DESCRIPTION,					"description");
DECLARE_CONST_STRING(FIELD_KEY_COLUMN,					"keyColumn");
DECLARE_CONST_STRING(FIELD_LABEL,						"label");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_ORDINAL,						"ordinal");

DECLARE_CONST_STRING(TYPENAME_ANY,						"Any");
DECLARE_CONST_STRING(TYPENAME_ARRAY,					"Array");
DECLARE_CONST_STRING(TYPENAME_ARRAY_DATE_TIME,			"ArrayOfDateTime");
DECLARE_CONST_STRING(TYPENAME_ARRAY_FLOAT_64,			"ArrayOfFloat64");
DECLARE_CONST_STRING(TYPENAME_ARRAY_INT_32,				"ArrayOfInt32");
DECLARE_CONST_STRING(TYPENAME_ARRAY_INT_64,				"ArrayOfInt64");
DECLARE_CONST_STRING(TYPENAME_ARRAY_INT_IP,				"ArrayOfIntIP");
DECLARE_CONST_STRING(TYPENAME_ARRAY_STRING,				"ArrayOfString");
DECLARE_CONST_STRING(TYPENAME_BINARY,					"Binary");
DECLARE_CONST_STRING(TYPENAME_BITMAP_RGBA8,				"BitmapOfRGBA8");
DECLARE_CONST_STRING(TYPENAME_BOOL,						"Bool");
DECLARE_CONST_STRING(TYPENAME_CANVAS,					"Canvas");
DECLARE_CONST_STRING(TYPENAME_DATATYPE,					"Datatype");
DECLARE_CONST_STRING(TYPENAME_DATE_TIME,				"DateTime");
DECLARE_CONST_STRING(TYPENAME_ENUM,						"Enum");
DECLARE_CONST_STRING(TYPENAME_FLOAT_64,					"Float64");
DECLARE_CONST_STRING(TYPENAME_FUNCTION,					"FunctionType");
DECLARE_CONST_STRING(TYPENAME_INT_32,					"Int32");
DECLARE_CONST_STRING(TYPENAME_INT_64,					"Int64");
DECLARE_CONST_STRING(TYPENAME_INT_IP,					"IntIP");
DECLARE_CONST_STRING(TYPENAME_INTEGER,					"Integer");
DECLARE_CONST_STRING(TYPENAME_MEMBER_TABLE,				"MemberTable");
DECLARE_CONST_STRING(TYPENAME_NULL,						"NullType");
DECLARE_CONST_STRING(TYPENAME_NUMBER,					"Number");
DECLARE_CONST_STRING(TYPENAME_OBJECT,					"Object");
DECLARE_CONST_STRING(TYPENAME_REAL,						"Real");
DECLARE_CONST_STRING(TYPENAME_SCHEMA_TABLE,				"SchemaTable");
DECLARE_CONST_STRING(TYPENAME_SIGNED,					"Signed");
DECLARE_CONST_STRING(TYPENAME_STRING,					"String");
DECLARE_CONST_STRING(TYPENAME_STRUCT,					"Struct");
DECLARE_CONST_STRING(TYPENAME_TABLE,					"Table");
DECLARE_CONST_STRING(TYPENAME_TEXT_LINES,				"TextLines");
DECLARE_CONST_STRING(TYPENAME_TIME_SPAN,				"TimeSpan");
DECLARE_CONST_STRING(TYPENAME_UINT_32,					"UInt32");
DECLARE_CONST_STRING(TYPENAME_UINT_64,					"UInt64");
DECLARE_CONST_STRING(TYPENAME_UNSIGNED,					"Unsigned");
DECLARE_CONST_STRING(TYPENAME_VECTOR_2D_F64,			"Vector2DOfFloat64");

TArray<CDatum> CAEONTypeSystem::m_CoreTypes;
DWORD CAEONTypeSystem::m_dwNextCoreType = IDatatype::MAX_CORE_TYPE + 1;
CAEONTypeSystem CAEONTypeSystem::m_Null;

void CAEONTypeSystem::AccumulateCoreTypes (TSortMap<CString, CDatum>& retTypes)

//	AccumulateCoreTypes
//
//	Adds core types to the map (but excluded library registered ones.

	{
	for (int i = 0; i <= IDatatype::MAX_CORE_TYPE; i++)
		{
		CDatum dType = GetCoreTypes()[i];
		if (dType.IsNil())
			continue;

		if (dType.GetBasicType() != CDatum::typeDatatype)
			throw CException(errFail);

		const IDatatype &Type = dType;
		retTypes.SetAt(Type.GetFullyQualifiedName(), dType);
		}
	}

CDatum CAEONTypeSystem::AddAnonymousSchema (const TArray<IDatatype::SMemberDesc> &Columns)

//	AddAnonymousSchema
//
//	Adds a new schema.

	{
	CDatum dNewSchema = CreateAnonymousSchema(Columns);
	const IDatatype& NewSchema = dNewSchema;

	//	See if this schema already exists.

	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		if (NewSchema == m_Types[i])
			return m_Types[i];
		}

	if (!AddType(dNewSchema))
		return CDatum();

	return dNewSchema;
	}

void CAEONTypeSystem::AddCoreType (IDatatype *pNewDatatype)

//	AddCoreType
//
//	Adds the given core type.

	{
	AddCoreType(CDatum(new CComplexDatatype(pNewDatatype)));
	}

void CAEONTypeSystem::AddCoreType (CDatum dType)

//	AddCoreType
//
//	Adds the given core type.

	{
	const IDatatype &Datatype = dType;
	DWORD dwType = Datatype.GetCoreType();
	if (dwType == IDatatype::UNKNOWN)
		throw CException(errFail);

	if ((int)dwType + 1 > m_CoreTypes.GetCount())
		{
		m_CoreTypes.InsertEmpty(dwType + 1 - m_CoreTypes.GetCount());
		}

	m_CoreTypes[dwType] = dType;
	}

bool CAEONTypeSystem::AddType (CDatum dType)

//	AddType
//
//	Adds a type and returns FALSE if we failed.

	{
	if (dType.GetBasicType() != CDatum::typeDatatype)
		return false;

	const IDatatype &Datatype = dType;
	m_Types.SetAt(strToLower(Datatype.GetFullyQualifiedName()), dType);

	return true;
	}

CDatum CAEONTypeSystem::CreateAnonymousSchema (const TArray<IDatatype::SMemberDesc> &Columns)

//	CreateAnonymousSchema
//
//	Create an anonymous schema.

	{
	CString sFullyQualifiedName = CAEONTypeSystem::MakeFullyQualifiedName(NULL_STR, strPattern("AnonymousTable%08x", mathRandom()));

	IDatatype *pNewType = new CDatatypeSchema(CDatatypeSchema::SCreate({ sFullyQualifiedName, { CAEONTypeSystem::GetCoreType(IDatatype::TABLE) } }));

	for (int i = 0; i < Columns.GetCount(); i++)
		{
		if (Columns[i].iType != IDatatype::EMemberType::InstanceKeyVar
				&& Columns[i].iType != IDatatype::EMemberType::InstanceVar)
			throw CException(errFail);

		if (!pNewType->AddMember(Columns[i].sName, Columns[i].iType, Columns[i].dType))
			return CDatum();
		}

	CDatum dType(new CComplexDatatype(pNewType));
	return dType;
	}

DWORD CAEONTypeSystem::CreateCoreTypeSchema (const CString& sName, const TArray<IDatatype::SMemberDesc> &Columns)

//	CreateCoreTypeSchema
//
//	Creates a core type schema and returns the core type ID.

	{
	if (m_CoreTypes.GetCount() == 0)
		InitCoreTypes();

	CString sFullyQualifiedName = MakeFullyQualifiedName(NULL_STR, sName);
	DWORD dwCoreType = RegisterCoreType();
	IDatatype *pNewType = new CDatatypeSchema(CDatatypeSchema::SCreate({ sFullyQualifiedName, { CAEONTypeSystem::GetCoreType(IDatatype::TABLE) }, dwCoreType }));

	for (int i = 0; i < Columns.GetCount(); i++)
		{
		if (Columns[i].iType != IDatatype::EMemberType::InstanceKeyVar
				&& Columns[i].iType != IDatatype::EMemberType::InstanceVar)
			throw CException(errFail);

		if (!pNewType->AddMember(Columns[i].sName, Columns[i].iType, Columns[i].dType))
			throw CException(errFail);
		}

	AddCoreType(pNewType);

	return dwCoreType;
	}

DWORD CAEONTypeSystem::CreateCoreTypeSimple (const CString& sName, const CDatatypeList& Implements, bool bAbstract)

//	CreateCoreTypeSimple
//
//	Creates a simple core type and returns the core type ID.

	{
	if (m_CoreTypes.GetCount() == 0)
		InitCoreTypes();

	CString sFullyQualifiedName = MakeFullyQualifiedName(NULL_STR, sName);
	DWORD dwCoreType = RegisterCoreType();
	AddCoreType(new CDatatypeSimple({ sFullyQualifiedName, dwCoreType, Implements, bAbstract }));

	return dwCoreType;
	}

CDatum CAEONTypeSystem::CreateDatatypeClass (const CString &sFullyQualifiedName, const CDatatypeList &Implements, IDatatype **retpNewType)

//	CreateDatatypeClass
//
//	Creates a new datatype.

	{
	IDatatype *pNewType = new CDatatypeClass({ sFullyQualifiedName, Implements });
	CDatum dNewType(new CComplexDatatype(pNewType));

	if (retpNewType)
		*retpNewType = pNewType;

	return dNewType;
	}

CDatum CAEONTypeSystem::CreateDatatypeEnum (const CString& sFullyQualifiedName, const TArray<IDatatype::SMemberDesc>& Values, IDatatype** retpNewType, CString* retsError)

//	CreateDatatypeEnum
//
//	Creates an enum.

	{
	IDatatype* pNewType = new CDatatypeEnum(CDatatypeEnum::SCreate({ sFullyQualifiedName }));
	CDatum dType(new CComplexDatatype(pNewType));

	for (int i = 0; i < Values.GetCount(); i++)
		{
		if (!pNewType->AddMember(Values[i].sName, IDatatype::EMemberType::EnumValue, CDatum(), retsError))
			return CDatum();
		}

	if (retpNewType)
		*retpNewType = pNewType;

	return dType;
	}

CDatum CAEONTypeSystem::CreateDatatypeSchema (const CString &sFullyQualifiedName, const CDatatypeList &Implements, IDatatype **retpNewType)

//	CreateDatatypeSchema
//
//	Creates a new datatype.

	{
	IDatatype *pNewType = new CDatatypeSchema({ sFullyQualifiedName, Implements });
	CDatum dNewType(new CComplexDatatype(pNewType));

	if (retpNewType)
		*retpNewType = pNewType;

	return dNewType;
	}

IDatatype *CAEONTypeSystem::CreateMemberTable ()

//	CreateMemberTable
//
//	Creates a datatype representing the table used to define types

	{
	CString sFullyQualifiedName = CAEONTypeSystem::MakeFullyQualifiedName(NULL_STR, TYPENAME_MEMBER_TABLE);

	IDatatype *pNewType = new CDatatypeSchema(CDatatypeSchema::SCreate({ sFullyQualifiedName, { CAEONTypeSystem::GetCoreType(IDatatype::TABLE) }, IDatatype::MEMBER_TABLE }));

	pNewType->AddMember(FIELD_NAME, IDatatype::EMemberType::InstanceKeyVar, GetCoreType(IDatatype::STRING));
	pNewType->AddMember(FIELD_DATATYPE, IDatatype::EMemberType::InstanceVar, GetCoreType(IDatatype::DATATYPE));
	pNewType->AddMember(FIELD_LABEL, IDatatype::EMemberType::InstanceVar, GetCoreType(IDatatype::STRING));
	pNewType->AddMember(FIELD_DESCRIPTION, IDatatype::EMemberType::InstanceVar, GetCoreType(IDatatype::STRING));
	pNewType->AddMember(FIELD_KEY_COLUMN, IDatatype::EMemberType::InstanceVar, GetCoreType(IDatatype::BOOL));
	pNewType->AddMember(FIELD_ORDINAL, IDatatype::EMemberType::InstanceKeyVar, GetCoreType(IDatatype::INT_32));

	return pNewType;
	}

IDatatype *CAEONTypeSystem::CreateSchemaTable ()

//	CreateSchemaTable
//
//	Creates a datatype representing the table used to define a table schema.

	{
	CString sFullyQualifiedName = CAEONTypeSystem::MakeFullyQualifiedName(NULL_STR, TYPENAME_SCHEMA_TABLE);

	IDatatype *pNewType = new CDatatypeSchema(CDatatypeSchema::SCreate({ sFullyQualifiedName, { CAEONTypeSystem::GetCoreType(IDatatype::TABLE) }, IDatatype::SCHEMA_TABLE }));

	pNewType->AddMember(FIELD_NAME, IDatatype::EMemberType::InstanceKeyVar, GetCoreType(IDatatype::STRING));
	pNewType->AddMember(FIELD_DATATYPE, IDatatype::EMemberType::InstanceVar, GetCoreType(IDatatype::DATATYPE));
	pNewType->AddMember(FIELD_LABEL, IDatatype::EMemberType::InstanceVar, GetCoreType(IDatatype::STRING));
	pNewType->AddMember(FIELD_DESCRIPTION, IDatatype::EMemberType::InstanceVar, GetCoreType(IDatatype::STRING));
	pNewType->AddMember(FIELD_KEY_COLUMN, IDatatype::EMemberType::InstanceVar, GetCoreType(IDatatype::BOOL));

	return pNewType;
	}

CDatum CAEONTypeSystem::FindType (const CString &sFullyQualifiedName, const IDatatype **retpDatatype) const

//	FindType
//
//	Looks for the datatype by name. Returns NULL if not found.

	{
	auto pEntry = m_Types.GetAt(strToLower(sFullyQualifiedName));
	if (!pEntry)
		{
		if (retpDatatype)
			*retpDatatype = NULL;

		return CDatum();
		}

	if (retpDatatype)
		*retpDatatype = &(const IDatatype &)(*pEntry);

	return *pEntry;
	}

CDatum CAEONTypeSystem::GetCoreType (DWORD dwType)

//	GetCoreType
//
//	Returns the given core type.

	{
	//	Initialize, if not already.

	if (m_CoreTypes.GetCount() == 0)
		InitCoreTypes();

	if (dwType == 0 || (int)dwType >= m_CoreTypes.GetCount())
		throw CException(errFail);

	return m_CoreTypes[dwType];
	}

CDatum CAEONTypeSystem::GetTypeList () const

//	GetTypeList
//
//	Returns all types defined.

	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(m_Types.GetCount());

	for (int i = 0; i < m_Types.GetCount(); i++)
		dResult.Append(m_Types[i]);

	return dResult;
	}

void CAEONTypeSystem::InitCoreTypes ()

//	InitCoreType
//
//	Initializes all core types.

	{
	AddCoreType(new CDatatypeAny(MakeFullyQualifiedName(NULL_STR, TYPENAME_ANY)));

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_DATATYPE), 
			IDatatype::DATATYPE,
			{  },
			false
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_BOOL), 
			IDatatype::BOOL,
			{ },
			true
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_NULL), 
			IDatatype::NULL_T,
			{ },
			false
			})
		);

	//	Numbers

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_NUMBER), 
			IDatatype::NUMBER,
			{ },
			true
			})
		);

	//	Integers

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_INTEGER), 
			IDatatype::INTEGER,
			{ GetCoreType(IDatatype::NUMBER) },
			true
			})
		);

	//	Signed Integers

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_SIGNED), 
			IDatatype::SIGNED,
			{ GetCoreType(IDatatype::INTEGER) },
			true
			})
		);

	AddCoreType(new CDatatypeNumber(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_32), 
			IDatatype::INT_32,
			{ GetCoreType(IDatatype::SIGNED) },
			32,
			false,
			false
			})
		);

	AddCoreType(new CDatatypeNumber(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_64), 
			IDatatype::INT_64,
			{ GetCoreType(IDatatype::SIGNED) },
			64,
			false,
			false
			})
		);

	AddCoreType(new CDatatypeNumber(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_IP), 
			IDatatype::INT_IP,
			{ GetCoreType(IDatatype::SIGNED) },
			0,
			false,
			false
			})
		);

	//	Unsigned Integers

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_UNSIGNED), 
			IDatatype::UNSIGNED,
			{ GetCoreType(IDatatype::INTEGER) },
			true
			})
		);

	AddCoreType(new CDatatypeNumber(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_UINT_32), 
			IDatatype::UINT_32,
			{ GetCoreType(IDatatype::UNSIGNED) },
			32,
			false,
			true
			})
		);

	AddCoreType(new CDatatypeNumber(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_UINT_64), 
			IDatatype::UINT_64,
			{ GetCoreType(IDatatype::UNSIGNED) },
			32,
			false,
			true
			})
		);

	//	Reals

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_REAL), 
			IDatatype::REAL,
			{ GetCoreType(IDatatype::NUMBER) },
			true
			})
		);

	AddCoreType(new CDatatypeNumber(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_FLOAT_64), 
			IDatatype::FLOAT_64,
			{ GetCoreType(IDatatype::REAL) },
			64,
			true,
			false
			})
		);

	//	Other

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_STRING), 
			IDatatype::STRING,
			{  },
			false
			})
		);

	AddCoreType(new CDatatypeArray(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY), 
			IDatatype::ARRAY,
			GetCoreType(IDatatype::ANY)
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_STRUCT), 
			IDatatype::STRUCT,
			{  },
			false
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_DATE_TIME), 
			IDatatype::DATE_TIME,
			{  },
			false
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
		MakeFullyQualifiedName(NULL_STR, TYPENAME_TIME_SPAN), 
		IDatatype::TIME_SPAN,
		{  },
		false
		})
	);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_BINARY), 
			IDatatype::BINARY,
			{  },
			false
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_FUNCTION), 
			IDatatype::FUNCTION,
			{  },
			true
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_OBJECT), 
			IDatatype::OBJECT,
			{  },
			true
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_TABLE), 
			IDatatype::TABLE,
			{  },
			true
			})
		);

	AddCoreType(new CDatatypeArray(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_INT_32), 
			IDatatype::ARRAY_INT_32,
			GetCoreType(IDatatype::INT_32)
			})
		);

	AddCoreType(new CDatatypeArray(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_FLOAT_64), 
			IDatatype::ARRAY_FLOAT_64,
			GetCoreType(IDatatype::FLOAT_64)
			})
		);

	AddCoreType(new CDatatypeArray(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_STRING), 
			IDatatype::ARRAY_STRING,
			GetCoreType(IDatatype::STRING)
			})
		);

	AddCoreType(new CDatatypeArray(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_DATE_TIME), 
			IDatatype::ARRAY_DATE_TIME,
			GetCoreType(IDatatype::DATE_TIME)
			})
		);

	AddCoreType(new CDatatypeArray(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_INT_64), 
			IDatatype::ARRAY_INT_64,
			GetCoreType(IDatatype::INT_64)
			})
		);

	AddCoreType(new CDatatypeArray(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_INT_IP), 
			IDatatype::ARRAY_INT_IP,
			GetCoreType(IDatatype::INT_IP)
			})
		);

	AddCoreType(CreateSchemaTable());
	AddCoreType(CreateMemberTable());

	AddCoreType(new CDatatypeMatrix(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_VECTOR_2D_F64), 
			IDatatype::VECTOR_2D_F64,
			GetCoreType(IDatatype::FLOAT_64),
			1,
			2
			})
		);

	//	Canvas and Bitmaps

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_CANVAS), 
			IDatatype::CANVAS,
			{  },
			false
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_BITMAP_RGBA8), 
			IDatatype::BITMAP_RGBA8,
			{  },
			false
			})
		);

	//	Enum

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_ENUM), 
			IDatatype::ENUM,
			{  },
			false
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_TEXT_LINES), 
			IDatatype::TEXT_LINES,
			{  },
			false
			})
		);
	}

bool CAEONTypeSystem::InitFrom (CDatum dSerialized, CString *retsError)

//	InitFrom
//
//	Initialize from a serialized struct.

	{
	return true;
	}

CString CAEONTypeSystem::MakeFullyQualifiedName (const CString &sFullyQualifiedScope, const CString &sName)

//	MakeFullyQualifiedName
//
//	Returns a fully qualified name from a scope and name. A scope of NULL means
//	global scope.

	{
	return CAEONTypes::MakeFullyQualifiedName(sFullyQualifiedScope, sName);
	}

void CAEONTypeSystem::Mark ()

//	Mark
//
//	Mark types in use.

	{
	for (int i = 0; i < m_Types.GetCount(); i++)
		m_Types[i].Mark();
	}

void CAEONTypeSystem::MarkCoreTypes ()

//	MarkCoreTypes
//
//	Marks all core types.

	{
	for (int i = 0; i < m_CoreTypes.GetCount(); i++)
		m_CoreTypes[i].Mark();
	}

CDatum CAEONTypeSystem::ResolveType (CDatum dType) const

//	ResolveType
//
//	Compares dType against our list of types. If this refers to one of our types
//	then we return it unchanged. If the type is equal to one of our types, then
//	we return our type (this helps when we deserialize a type). Otherwise, we
//	return the type unchanged.

	{
	if (dType.GetBasicType() != CDatum::typeDatatype)
		return dType;

	const IDatatype &Type = dType;
	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		const IDatatype &OurType = m_Types[i];
		if (&Type == &OurType)
			return dType;
		else if (Type == OurType)
			return m_Types[i];
		}

	return dType;
	}

CDatum CAEONTypeSystem::Serialize () const

//	Serialize
//
//	Serializes all types.

	{
	return CDatum();
	}

