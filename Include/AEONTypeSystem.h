//	AEONTypeSystem.h
//
//	AEON Type System Definitions
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#pragma once

class IDatatype
	{
	public:

		//	NOTE: These numbers may change from version to version. Do not 
		//	serialize them or rely on any specific value.

		static constexpr DWORD UNKNOWN =			0;

		//	These are basic concrete types that are returned by GetBasicDatatype().
		//	We use these numbers as indices into a table, so they must be contiguous.

		static constexpr DWORD ANY =				1;	//	Any type (a CDatum)

		static constexpr DWORD ARRAY =				2;	//	An array of Any
		static constexpr DWORD BINARY =				3;	//	A binary blob
		static constexpr DWORD BOOL =				4;	//	A boolean type (abstract)
		static constexpr DWORD CLASS_T	=			5;	//	An instance of a class (datatype is a class definition)
		static constexpr DWORD DATATYPE =			6;	//	A datatype object
		static constexpr DWORD DATE_TIME =			7;	//	A dateTime
		static constexpr DWORD DICTIONARY =			8;	//	A dictionary (Any to Any)
		static constexpr DWORD ENUM =				9;	//	Any enum type
		static constexpr DWORD ERROR_T =			10;	//	The error type
		static constexpr DWORD EXPRESSION = 		11;	//	A column expression
		static constexpr DWORD FLOAT_64 =			12;	//	A 64-bit float
		static constexpr DWORD FUNCTION =			13;	//	A function
		static constexpr DWORD INT_32 =				14;	//	A 32-bit signed integer (concrete)
		static constexpr DWORD INT_64 =				15;	//	A 64-bit signed integer (concrete)
		static constexpr DWORD INT_IP =				16;	//	An infinite precision signed integer (concrete)
		static constexpr DWORD NAN_CONST =			17;	//	This is a pseudo type representing a NaN constant (not a double)
		static constexpr DWORD NULL_T =				18;	//	The null type (concrete)
		static constexpr DWORD OBJECT =				19;	//	A generic object
		static constexpr DWORD RANGE =				20;	//	A range
		static constexpr DWORD SCHEMA =				21;	//	A typed structure
		static constexpr DWORD STRING =				22;	//	A UTF-8 string
		static constexpr DWORD STRUCT =				23;	//	A struct of Any
		static constexpr DWORD TABLE =				24;	//	A table (datatype is a schema)
		static constexpr DWORD TIME_SPAN =			25;	//	A timespan
		static constexpr DWORD VECTOR_2D_F64 =		26;	//	A 2D vector (concrete)
		static constexpr DWORD VECTOR_3D_F64 =		27;	//	A 3D vector

		static constexpr DWORD LAST_BASIC_TYPE =	28;	//	Technically not the last, but the last+1 because we use in allocating the array
		
		static constexpr DWORD NUMBER =				28;	//	Any number (abstract)
		static constexpr DWORD REAL =				29;	//	A real number (abstract)
		static constexpr DWORD INTEGER =			30;	//	An integer (abstract)
		static constexpr DWORD FLOAT =				31;	//	A floating point number (abstract)
		static constexpr DWORD SIGNED =				32;	//	A signed integer (abstract)
		static constexpr DWORD UNSIGNED =			33;	//	An unsigned integer (abstract)
		static constexpr DWORD UINT_32 =			34;	//	An unsigned 32-bit integer (concrete)
		static constexpr DWORD UINT_64 =			35;	//	An unsigned 64-bit integer (concrete)
		static constexpr DWORD INT_8 =				36;	//	An 8-bit signed integer (subrange)
		static constexpr DWORD INT_16 =				37;	//	A 16-bit signed integer (subrange)
		static constexpr DWORD UINT_8 =				38;	//	An 8-bit unsigned integer (subrange)
		static constexpr DWORD UINT_16 =			39;	//	A 16-bit unsigned integer (subrange)

		static constexpr DWORD INDEXED =			40;	//	Can use [n] operator to read
		static constexpr DWORD MUTABLE_INDEXED =	41;	//	Can use [n] operator to write
		static constexpr DWORD ABSTRACT_DICTIONARY =			42;	//	Can use [key] operator to read
		static constexpr DWORD ABSTRACT_MUTABLE_DICTIONARY =	43;	//	Can use [key] operator to write

		static constexpr DWORD ARRAY_INT_32 =		44;
		static constexpr DWORD ARRAY_FLOAT_64 =		45;
		static constexpr DWORD ARRAY_STRING =		46;
		static constexpr DWORD ARRAY_DATE_TIME =	47;
		static constexpr DWORD ARRAY_INT_64 =		48;
		static constexpr DWORD ARRAY_INT_IP =		49;
		static constexpr DWORD ARRAY_NUMBER =		50;

		static constexpr DWORD SCHEMA_TABLE_SCHEMA =51;	//	The schema for a schema table
		static constexpr DWORD SCHEMA_TABLE =		52;	//	A table describing a schema
		static constexpr DWORD MEMBER_TYPE_ENUM =	53;	//	An enumeration of member types
		static constexpr DWORD MEMBER_TABLE_SCHEMA = 54;	//	The schema for the members table
		static constexpr DWORD MEMBER_TABLE =		55;	//	A table describing members of a type

		static constexpr DWORD MATRIX_F64 =			56;	//	LATER: Any matrix (abstract)
		static constexpr DWORD MATRIX_3X3_F64 =		57;	//	LATER: A 3x3 matrix
		static constexpr DWORD MATRIX_4X4_F64 =		58;	//	LATER: A 4x4 matrix

		static constexpr DWORD CANVAS =				59;	//	A canvas object (concrete)
		static constexpr DWORD BITMAP_RGBA8 =		60;	//	A 32-bit per pixel bitmap (concrete)
		static constexpr DWORD TEXT_LINES =			61;	//	CAEONLines datastructure
		static constexpr DWORD SAS_DATE_TIME =		62;	//	SAS encoded float.
		static constexpr DWORD SAS_DATE =			63;	//	SAS encoded float.
		static constexpr DWORD SAS_TIME =			64;	//	SAS encoded float.
		static constexpr DWORD DAY_OF_WEEK_ENUM =	65;	//	Day of week enumeration

		static constexpr DWORD GRID_NAME_TYPE =		66;
		static constexpr DWORD STRING_FORMAT_TYPE =	67;	//	A string format type
		static constexpr DWORD MAP_COLUMN_EXPRESSION =	68;	//	A map column expression type

		static constexpr DWORD MAX_CORE_TYPE =		68;

		static constexpr DWORD ORDINAL_MEMBER_TYPE_DEF =		0;
		static constexpr DWORD ORDINAL_MEMBER_TYPE_PROPERTY =	1;
		static constexpr DWORD ORDINAL_MEMBER_TYPE_FUNCTION =	2;
		static constexpr DWORD ORDINAL_MEMBER_TYPE_EVENT =		3;
		static constexpr DWORD ORDINAL_MEMBER_TYPE_VAR =		4;

		enum class ECategory
			{
			Unknown,

			Simple,							//	Datatype that does not refer to other types.
			Number,							//	A number type
			Array,							//	An array of some other type.
			ClassDef,						//	A ordered set of fields and types
			Dictionary,						//	A dictionary of some other type.
			Enum,							//	An enumeration type
			Function,						//	A function type
			Tensor,							//	A multidimensional array
			Nullable,						//	A nullable type
			Range,							//	A range type
			Schema,							//	A struct definition
			Table,							//	A table
			};

		enum class EImplementation
			{
			Unknown,

			Any,
			Array,
			Class,
			Enum,
			Function,
			Tensor,
			Null,
			Nullable,
			Number,
			Range,
			Schema,
			Simple,
			};

		enum class EMemberType
			{
			None,

			ArgType,						//	An argument type of a function
			ArrayElement,					//	The element type of an array
			DynamicMember,					//	A member that is dynamically added
			EnumValue,						//	One of the enum values
			IndexElement,					//	The type of an array index
			InstanceKeyVar,					//	A key column for a schema
			InstanceMethod,					//	An object method
			InstanceProperty,				//	An object property
			InstanceReadOnlyProperty,		//	An object read-only property
			InstanceValue,					//	A nullary object method
			InstanceVar,					//	A column or field
			RangeType,						//	The base type of a range
			ReturnType,						//	The return type of a function
			StaticMethod,					//	A static method
			StaticVar,						//	A static variable
			};

		enum class EDisplay
			{
			Default,						//	Default display

			Hidden,							//	Do not display
			ReadOnly,						//	Display but do not allow editing
			Editable,						//	Display and allow editing
			};

		struct SArgDesc
			{
			int iSignature = 0;				//	Signature index (0-based)
			CString sID;
			CDatum dType;
			CString sDesc;					//	Help text.

			bool bVarArg = false;			//	If TRUE, 0 or more of this argument.
											//		Must be at the end.
			};

		enum class EReturnDescType
			{
			Type,							//	Return type is a specific type (dType)

			ArgType,						//	Return type is same as arg type
			ArgLiteral,						//	Return type is literal value in arg
			};

		struct SReturnTypeDesc
			{
			EReturnDescType iType = EReturnDescType::Type;
			CDatum dType;
			int iFromArg = -1;				//	Return type is formed from arg
			};

		static constexpr DWORD MEMBER_FLAG_INFERRED = 0x00000001;	//	Inferred member

		struct SMemberDesc
			{
			EMemberType iType = EMemberType::None;
			CString sID;
			CDatum dType;
			int iOrdinal = 0;
			CString sLabel;					//	Human readable name
			EDisplay iDisplay = EDisplay::Default;
			CString sFormat;				//	Number format
			DWORD dwFlags = 0;
			};

		struct SNumberDesc
			{
			bool bNumber = false;			//	TRUE if this is a number type
			bool bFloat = false;			//	TRUE if this is a floating point number
			bool bUnsigned = false;			//	TRUE if this is an unsigned number
			bool bSubRange = false;			//	TRUE if this is a subrange
			int iBits = 0;					//	Number of bits (0 == infinite precision)
			int iSubRangeMin = 0;			//	Min value for subrange
			int iSubRangeMax = 0;			//	Max value for subrange
			};

		IDatatype (const CString &sFullyQualifiedName, DWORD dwCoreType = 0, bool bForceAnonymous = false);

		IDatatype (const IDatatype &Src) = delete;
		IDatatype (IDatatype &&Src) = delete;

		virtual ~IDatatype () { }

		IDatatype &operator = (const IDatatype &Src) = delete;
		IDatatype &operator = (IDatatype &&Src) = delete;

		bool operator == (const IDatatype &Src) const;
		bool operator != (const IDatatype &Src) const { return !(*this == Src); }

		bool AddImplementation (CDatum dType) { return OnAddImplementation(dType); }
		bool AddMember (const SMemberDesc& Desc, CString *retsError = NULL) { return OnAddMember(Desc, retsError); }
		CDatum ApplyKeyToRow (CDatum dKey, CDatum dRow) const;
		bool CanBeCalledWith (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, CDatum* retdReturnType = NULL, CString* retsError = NULL) const { return OnCanBeCalledWith(dThisType, ArgTypes, ArgLiteralTypes, retdReturnType, retsError); }
		bool CanBeCalledWithArgCount (CDatum dThisType, int iArgCount, CDatum* retdReturnType = NULL, CString* retsError = NULL) const { return OnCanBeCalledWithArgCount(dThisType, iArgCount, retdReturnType, retsError); }
		bool CanBeConstructedFrom (CDatum dType) const { return OnCanBeConstructedFrom(dType); }
		bool CanBeNull () const { return OnCanBeNull(); }
		CDatum CreateAsType (CDatum dValue) const { return OnCreateAsType(dValue); }
		static TUniquePtr<IDatatype> Deserialize (CDatum::EFormat iFormat, DWORD dwType, IByteStream &Stream);
		static TUniquePtr<IDatatype> DeserializeAEON (IByteStream& Stream, DWORD dwType, CAEONSerializedMap &Serialized);
		int FindMember (CStringView sName) const { return OnFindMember(sName); }
		int FindMemberByOrdinal (int iOrdinal) const { return OnFindMemberByOrdinal(iOrdinal); }
		ECategory GetClass () const { return OnGetClass(); }
		DWORD GetCoreType () const { return m_dwCoreType; }
		TArray<CDatum> GetDimensionTypes () const { return OnGetDimensionTypes(); }
		bool GetDisplayMembers (TArray<int>& retMembers) const;
		CDatum GetElementType () const;
		CDatum GetFieldsAsTable () const { return OnGetFieldsAsTable(); }
		const CString &GetFullyQualifiedName () const { return m_sFullyQualifiedName; }
		EImplementation GetImplementation () const { return OnGetImplementation(); }
		CString GetKeyFromKeyValue (CDatum dKey) const;
		CString GetKeyFromKeyValue (const TArray<int>& Keys, CDatum dKey) const;
		CDatum GetKeyFromRow (CDatum dRow) const;
		bool GetKeyMembers (TArray<int>& retKeys) const;
		bool GetKeyMembers (TArray<CString>& retKeys) const;
		CDatum GetKeyType () const { return OnGetKeyType(); }
		SMemberDesc GetMember (int iIndex) const { return OnGetMember(iIndex); }
		CDatum GetMembersAsTable () const;
		int GetMemberCount () const { return OnGetMemberCount(); }
		CString GetName () const { return OnGetName(); }
		SNumberDesc GetNumberDesc () const { return OnGetNumberDesc(); }
		CDatum GetRangeType () const { return OnGetRangeType(); }
		CDatum GetSliceType () const { return OnGetSliceType(); }
		CDatum GetVariantType () const { return OnGetVariantType(); }
		bool HasMember (EMemberType iType, CDatum *retdType = NULL, int* retiOrdinal = NULL) const;
		EMemberType HasMember (CStringView sName, CDatum* retdType = NULL, int* retiOrdinal = NULL) const { return OnHasMember(sName, retdType, retiOrdinal); }
		bool IsA (const IDatatype &Type) const;
		bool IsA (CDatum dType) const { return IsA((const IDatatype &)dType); }
		bool IsA (DWORD dwType) const;
		bool IsAEx (const IDatatype& Type) const;
		bool IsAbstract () const { return OnIsAbstract(); }
		bool IsAnonymous () const { return m_fAnonymous; }
		bool IsAny () const { return OnIsAny(); }
		bool IsCoreType () const { return (m_dwCoreType != 0 && m_dwCoreType <= MAX_CORE_TYPE); }
		bool IsEqualEx(const IDatatype& Src) const;
		bool IsEnum (const TArray<IDatatype::SMemberDesc>& Values) const { return OnIsEnum(Values); }
		bool IsErrorType () const { return IsA(IDatatype::ERROR_T); }
		bool IsNullable () const { return GetClass() == ECategory::Nullable; }
		bool IsNullType () const { return GetCoreType() == IDatatype::NULL_T; }
		bool IsSupersetOf (const IDatatype& Type) const { return OnIsSupersetOf(Type); }
		CDatum IteratorBegin () const { return OnIteratorBegin(); }
		CDatum IteratorGetKey (CDatum dThisType, CDatum dIterator) const { return OnIteratorGetKey(dThisType, dIterator); }
		CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dThisType, CDatum dIterator) const { return OnIteratorGetValue(TypeSystem, dThisType, dIterator); }
		CDatum IteratorNext (CDatum dIterator) const { return OnIteratorNext(dIterator); }
		void Mark () { OnMark(); }
		void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const;
		void SetCoreType (DWORD dwCoreType) { m_dwCoreType = dwCoreType; }
		void SetMemberType (const CString& sName, CDatum dType, DWORD dwFlags = 0) { OnSetMemberType(sName, dType, dwFlags); }

		static CString GetID (EDisplay iDisplay);
		static bool FindMember (const TArray<SMemberDesc>& Members, CStringView sName, EMemberType iType = EMemberType::None, int* retiPos = NULL);
		static EDisplay ParseDisplay (CDatum dValue);

	protected:

		CString DefaultGetName () const;

	private:

		static constexpr DWORD NEW_VERSION_FLAG = 0x80000000;

		//	IDatatype virtuals

		virtual bool OnAddImplementation (CDatum dType) { throw CException(errFail); }
		virtual bool OnAddMember (const SMemberDesc& Desc, CString *retsError = NULL) { throw CException(errFail); }
		virtual bool OnCanBeCalledWith (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, CDatum* retdReturnType = NULL, CString* retsError = NULL) const { return false; }
		virtual bool OnCanBeCalledWithArgCount (CDatum dThisType, int iArgCount, CDatum* retdReturnType = NULL, CString* retsError = NULL) const { return false; }
		virtual bool OnCanBeConstructedFrom (CDatum dType) const { const IDatatype& Type = dType; return (Type.IsAny() || Type.IsA(*this)); }
		virtual bool OnCanBeNull () const { return false; }
		virtual CDatum OnCreateAsType (CDatum dValue) const { return dValue; }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion) = 0;
		virtual bool OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized) = 0;
		virtual bool OnEquals (const IDatatype &Src) const = 0;
		virtual int OnFindMember (CStringView sName) const { return -1; }
		virtual int OnFindMemberByOrdinal (int iOrdinal) const { if (iOrdinal >= 0 && iOrdinal < GetMemberCount()) return iOrdinal; else return - 1; }
		virtual EMemberType OnHasMember (CStringView sName, CDatum* retdType = NULL, int* retiOrdinal = NULL) const { return EMemberType::DynamicMember; }
		virtual ECategory OnGetClass () const = 0;
		virtual TArray<CDatum> OnGetDimensionTypes () const { return TArray<CDatum>(); }
		virtual CDatum OnGetFieldsAsTable () const;
		virtual EImplementation OnGetImplementation () const = 0;
		virtual CDatum OnGetKeyType () const;
		virtual SMemberDesc OnGetMember (int iIndex) const { throw CException(errFail); }
		virtual int OnGetMemberCount () const { return 0; }
		virtual CString OnGetName () const { return DefaultGetName(); }
		virtual SNumberDesc OnGetNumberDesc () const { return SNumberDesc(); }
		virtual CDatum OnGetRangeType () const;
		virtual CDatum OnGetSliceType () const { return CDatum(); }
		virtual CDatum OnGetVariantType () const { return CDatum(); }
		virtual bool OnIsA (const IDatatype &Type) const { return false; }
		virtual bool OnIsAbstract () const { return false; }
		virtual bool OnIsAny () const { return false; }
		virtual bool OnIsEnum (const TArray<IDatatype::SMemberDesc>& Values) const { return false; }
		virtual bool OnIsSupersetOf (const IDatatype& Type) const { return IsAEx(Type); }
		virtual CDatum OnIteratorBegin () const { return CDatum(); }
		virtual CDatum OnIteratorGetKey (CDatum dThisType, CDatum dIterator) const { return CDatum(); }
		virtual CDatum OnIteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dThisType, CDatum dIterator) const { return CDatum(); }
		virtual CDatum OnIteratorNext (CDatum dIterator) const { return CDatum(); }
		virtual void OnMark () { }
		virtual void OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const = 0;
		virtual void OnSetMemberType (const CString& sName, CDatum dType, DWORD dwFlags) { throw CException(errFail); }
		
		static CString AsIndexKeyFromValue (CDatum dValue);

		CString m_sFullyQualifiedName;
		DWORD m_dwCoreType = 0;								//	If non-zero, then we're registered with CAEONTypes

		DWORD m_fAnonymous:1 = false;						//	If TRUE, then this is an anonymous type
	};

class CDatatypeList
	{
	public:
		CDatatypeList (std::initializer_list<CDatum> List = {});
		CDatatypeList (std::initializer_list<DWORD> List);

		bool operator == (const CDatatypeList &Src) const;
		bool operator != (const CDatatypeList &Src) const { return !(*this == Src); }

		void AddType (CDatum dType) { m_Types.Insert(dType); }
		void DebugDump () const;
		static bool Deserialize (CDatum::EFormat iFormat, IByteStream &Stream, CDatatypeList &retList);
		static bool DeserializeAEON (IByteStream& Stream, CAEONSerializedMap &Serialized, CDatatypeList& retList);
		bool IsA (const IDatatype &Type) const;
		void Mark ();
		void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const;

	private:
		TArray<CDatum> m_Types;
	};

class CAEONTypeSystem
	{
	public:

		static constexpr DWORD NULL_ATOM = 0xffffffff;

		CDatum AddAnonymousArray (CDatum dElementType);
		CDatum AddAnonymousDictionary (CDatum dKeyType, CDatum dElementType);
		CDatum AddAnonymousRange (int iMin, int iMax);
		CDatum AddAnonymousSchema (const TArray<IDatatype::SMemberDesc> &Columns);
		CDatum AddAnonymousTensor (CDatum dElementType, const TArray<CDatum>& Dimensions);
		bool AddType (CDatum dType);
		DWORD Atomize (CStringView sFullyQualifiedName);

		static CDatum CreateDatatypeClass (const CString& sFullyQualifiedName, IDatatype** retpNewType = NULL);
		static CDatum CreateDatatypeEnum (const CString& sFullyQualifiedName, const TArray<IDatatype::SMemberDesc>& Values, IDatatype** retpNewType = NULL, CString* retsError = NULL);
		static CDatum CreateDatatypeFunction (const CString& sFullyQualifiedName, const IDatatype::SReturnTypeDesc& Return, const TArray<IDatatype::SArgDesc>& Args, IDatatype** retpNewType = NULL, CString* retsError = NULL);
		static CDatum CreateDatatypeSchema (const CString& sFullyQualifiedName, IDatatype** retpNewType = NULL);
		static CDatum FindCoreType (const CString& sFullyQualifiedName, const IDatatype** retpDatatype = NULL);
		CDatum FindType (const CString& sFullyQualifiedName, const IDatatype** retpDatatype = NULL) const;
		CDatum FindType (CDatum dType) const;
		CDatum Get (DWORD dwAtom) const { return (dwAtom < (DWORD)m_Types.GetCount() ? m_Types[dwAtom] : CDatum()); }
		static CDatum GetCoreType (DWORD dwType);
		CDatum GetTypeList () const;
		bool InitFrom (CDatum dSerialized, CString *retsError = NULL);
		bool IsEmpty () const { return m_Types.GetCount() == 0; }
		static CString MakeFullyQualifiedName (const CString& sFullyQualifiedScope, const CString& sName);
		static CString MakeFullyQualifiedName (const TArray<CString>& Names);
		void Mark ();
		CDatum ResolveType (CDatum dType) const;
		CDatum Serialize () const;

		static CDatum CreateAnonymousArray (const CString& sFullyQualifiedName, CDatum dElementType);
		static CDatum CreateAnonymousDictionary (const CString& sFullyQualifiedName, CDatum dKeyType, CDatum dElementType);
		static CDatum CreateAnonymousSchema (const TArray<IDatatype::SMemberDesc>& Columns);
		static CDatum CreateAnonymousTable (const CString& sFullyQualifiedName, CDatum dSchema);
		static CDatum CreateNullableType (const CString& sFullyQualifiedName, CDatum dVariantType);
		static CAEONTypeSystem& Null () { return m_Null; }

	private:

		TArray<CDatum> m_Types;
		TSortMap<CString, DWORD> m_Index;
		DWORD m_dwNextAnonymousID = 1;

		static CAEONTypeSystem m_Null;
	};

class CAEONTypes
	{
	public:

		static void AccumulateCoreTypes (TSortMap<CString, CDatum>& retTypes);
		static DWORD AddCoreAEON (CStringView sName, const CDatatypeList& Implement, CStringView sDatumTypename, TArray<IDatatype::SMemberDesc>&& Members, bool bCore = false);
		static DWORD AddCoreEnum (const CString& sName, const TArray<IDatatype::SMemberDesc>& Values);
		static DWORD AddCoreSchema (const CString& sName, const TArray<IDatatype::SMemberDesc>& Columns);
		static DWORD AddCoreSimple (const CString& sName, const CDatatypeList& Implements, bool bAbstract);
		static DWORD AddEnum (const CString& sFullyQualifiedName, const TArray<IDatatype::SMemberDesc>& Values, bool bCore = false);
		static DWORD AddSchema (const CString& sFullyQualifiedName, const TArray<IDatatype::SMemberDesc>& Columns, bool bCore = false);
		static DWORD AddSimple (const CString& sFullyQualifiedName, const CDatatypeList& Implements, bool bAbstract, bool bCore = false);
		static CDatum CreateFunctionType (const CString& sArgCode);
		static CDatum CreatePropertyType (const char* pPos);
		static CDatum CreateInt32SubRange (const CString& sFullyQualifiedName, int iMin, int iMax, DWORD dwCoreType = 0);
		static CDatum CreateTensor (CStringView sFullyQualifiedName, CDatum dElementType, const TArray<CDatum>& Dimensions, DWORD dwCoreType = 0);
		static CDatum CreateSchema (const CString& sFullyQualifiedName, const TArray<IDatatype::SMemberDesc>& Columns, DWORD dwCoreType);
		static CDatum FindCoreType (const CString& sFullyQualifiedName, const IDatatype** retpDatatype = NULL);
		static CDatum FindEnum (const TArray<IDatatype::SMemberDesc>& Values);
		static CDatum FindEnumOrAdd (CDatum dType);
		static CDatum FindTableOrAdd (CDatum dSchema);
		static CDatum FindType (CDatum dType);
		static CDatum Get (DWORD dwID);
		static CDatum Get_NoError (DWORD dwID);
		static int GetCount () { CSmartLock Lock(m_cs); return m_Types.GetCount(); }
		static CDatum GetCompatibleType (CDatum dLeft, CDatum dRight);
		static CDatum GetCompatibleType (const TArray<CDatum>& Types);
		static CString MakeAnonymousName (const CString& sType);
		static CString MakeAnonymousName (const CString& sFullyQualifiedScope, const CString& sType);
		static CString MakeFullyQualifiedFunctionName (const CString& sFullyQualifiedScope = NULL_STR);
		static CString MakeFullyQualifiedName (const CString &sFullyQualifiedScope, const CString &sName);
		static void MarkAndSweep ();
		static CString ParseNameFromFullyQualifiedName (const CString &sValue, bool bAbsolute = false);

	private:

		static DWORD Alloc ();
		static CDatum CreateAny ();
		static CDatum CreateArray (const CString& sFullyQualifiedName, CDatum dElementType, DWORD dwCoreType, bool bForceAnonymous = false);
		static CDatum CreateDayOfWeekEnum ();
		static CDatum CreateDictionary (const CString& sFullyQualifiedName, CDatum dKeyType, CDatum dElementType, DWORD dwCoreType);
		static CDatum CreateEnum (const CString& sFullyQualifiedName, const TArray<IDatatype::SMemberDesc>& Values, DWORD dwCoreType);
		static CDatum CreateMemberTableType ();
		static CDatum CreateMemberTableSchema ();
		static CDatum CreateMemberTypeEnum ();
		static CDatum CreateNull ();
		static CDatum CreateNumber (const CString& sFullyQualifiedName, const CDatatypeList& Implements, int iBits, bool bFloat, bool bUnsigned, DWORD dwCoreType, bool bAbstract = false, bool bCanBeNull = false);
		static CDatum CreateSimple (const CString& sFullyQualifiedName, const CDatatypeList& Implements, bool bAbstract, bool bCanBeNull, DWORD dwCoreType, bool bNoMembers = false);
		static CDatum CreateSchemaTable ();
		static CDatum CreateSchemaTableSchema ();
		static CDatum CreateStringType (const CString& sFullyQualifiedName);
		static CDatum CreateTensor (const CString& sFullyQualifiedName, CDatum dElementType, int iRows, int iCols, DWORD dwCoreType);
		static void InitCoreTypes ();
		static void InitGridNameType ();
		static CDatum ParseTypeFromArgCode (const char*& pPos);
		static void SetCoreType (DWORD dwCoreType, CDatum dType) { SetType(dwCoreType, dType, true); }
		static void SetCoreAEONType (DWORD dwCoreType, CStringView sTypename, CStringView sDatumTypename, std::function<TArray<IDatatype::SMemberDesc>()> fnMembers);
		static void SetType (DWORD dwID, CDatum dType, bool bCore);

		static CCriticalSection m_cs;
		static TArray<CDatum> m_Types;
		static TSortMap<CString, DWORD> m_CoreTypes;
		static TArray<int> m_FreeTypes;
		static bool m_bInitDone;
		static DWORD m_dwNextAnonymousID;
	};
