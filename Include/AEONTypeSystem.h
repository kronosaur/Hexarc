//	AEONTypeSystem.h
//
//	AEON Type System Definitions
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class IDatatype
	{
	public:

		static constexpr DWORD UNKNOWN =			0;

		static constexpr DWORD ANY =				1;	//	Any type (a CDatum)
		static constexpr DWORD DATATYPE =			2;	//	A datatype object
		static constexpr DWORD BOOL =				3;	//	A boolean type (abstract)
		static constexpr DWORD NULL_T =				4;	//	The null type (concrete)
		
		static constexpr DWORD NUMBER =				5;	//	Any number (abstract)
		static constexpr DWORD REAL =				6;	//	A real number (abstract)
		static constexpr DWORD INTEGER =			7;	//	An integer (abstract)
		static constexpr DWORD FLOAT =				8;	//	A floating point number (abstract)
		static constexpr DWORD SIGNED =				9;	//	A signed integer (abstract)
		static constexpr DWORD UNSIGNED =			10;	//	An unsigned integer (abstract)
		static constexpr DWORD INT_32 =				11;	//	A 32-bit signed integer (concrete)
		static constexpr DWORD INT_64 =				12;	//	A 64-bit signed integer (concrete)
		static constexpr DWORD INT_IP =				13;	//	An infinite precision signed integer (concrete)
		static constexpr DWORD UINT_32 =			14;	//	An unsigned 32-bit integer (concrete)
		static constexpr DWORD UINT_64 =			15;	//	An unsigned 64-bit integer (concrete)
		static constexpr DWORD FLOAT_64 =			16;	//	A 64-bit float

		static constexpr DWORD STRING =				17;	//	A UTF-8 string
		static constexpr DWORD ARRAY =				18;	//	An array of Any
		static constexpr DWORD STRUCT =				19;	//	A struct of Any
		static constexpr DWORD DATE_TIME =			20;	//	A dateTime
		static constexpr DWORD TIME_SPAN =			21;	//	A timespan
		static constexpr DWORD BINARY =				22;	//	A binary blob
		static constexpr DWORD FUNCTION =			23;	//	A function
		static constexpr DWORD OBJECT =				24;	//	An object (datatype is a class definition)
		static constexpr DWORD TABLE =				25;	//	A table (datatype is a schema)

		static constexpr DWORD ARRAY_INT_32 =		26;
		static constexpr DWORD ARRAY_FLOAT_64 =		27;
		static constexpr DWORD ARRAY_STRING =		28;
		static constexpr DWORD ARRAY_DATE_TIME =	29;
		static constexpr DWORD ARRAY_INT_64 =		30;
		static constexpr DWORD ARRAY_INT_IP =		31;

		static constexpr DWORD SCHEMA_TABLE =		32;	//	A table describing a schema

		static constexpr DWORD VECTOR_2D_F64 =		33;	//	A 2D vector (concrete)
		static constexpr DWORD VECTOR_3D_F64 =		34;	//	LATER: A 3D vector
		static constexpr DWORD MATRIX_F64 =			35;	//	LATER: Any matrix (abstract)
		static constexpr DWORD MATRIX_3X3_F64 =		36;	//	LATER: A 3x3 matrix
		static constexpr DWORD MATRIX_4X4_F64 =		37;	//	LATER: A 4x4 matrix

		static constexpr DWORD CANVAS =				38;	//	A canvas object (concrete)
		static constexpr DWORD BITMAP_RGBA8 =		39;	//	A 32-bit per pixel bitmap (concrete)

		enum class ECategory
			{
			Unknown,

			Simple,							//	Datatype that does not refer to other types.
			Number,							//	A number type
			Array,							//	An array of some other type.
			ClassDef,						//	A ordered set of fields and types
			Function,						//	A function type
			Matrix,							//	An m x n matrix
			Schema,							//	A table definition
			};

		enum class EImplementation
			{
			Unknown,

			Any,
			Array,
			Class,
			Matrix,
			Number,
			Schema,
			Simple,
			};

		enum class EMemberType
			{
			None,

			ArrayElement,
			InstanceKeyVar,
			InstanceMethod,
			InstanceVar,
			StaticMethod,
			StaticVar,
			};

		struct SMemberDesc
			{
			EMemberType iType = EMemberType::None;
			CString sName;
			CDatum dType;
			};

		IDatatype (const CString &sFullyQualifiedName) :
				m_sFullyQualifiedName(sFullyQualifiedName)
			{ }

		IDatatype (const IDatatype &Src) = delete;
		IDatatype (IDatatype &&Src) = delete;

		virtual ~IDatatype () { }

		IDatatype &operator = (const IDatatype &Src) = delete;
		IDatatype &operator = (IDatatype &&Src) = delete;

		bool operator == (const IDatatype &Src) const;
		bool operator != (const IDatatype &Src) const { return !(*this == Src); }

		bool AddMember (const CString &sName, EMemberType iType, CDatum dType, CString *retsError = NULL) { return OnAddMember(sName, iType, dType, retsError); }
		static TUniquePtr<IDatatype> Deserialize (CDatum::EFormat iFormat, IByteStream &Stream);
		int FindMember (const CString &sName) const { return OnFindMember(sName); }
		ECategory GetClass () const { return OnGetClass(); }
		DWORD GetCoreType () const { return OnGetCoreType(); }
		const CString &GetFullyQualifiedName () const { return m_sFullyQualifiedName; }
		EImplementation GetImplementation () const { return OnGetImplementation(); }
		bool GetKeyMembers (TArray<int>& retKeys) const;
		SMemberDesc GetMember (int iIndex) const { return OnGetMember(iIndex); }
		int GetMemberCount () const { return OnGetMemberCount(); }
		CDatum GetMembersAsTable () const { return OnGetMembersAsTable(); }
		CString GetName () const;
		EMemberType HasMember (const CString &sName, CDatum *retdType = NULL) const { return OnHasMember(sName, retdType); }
		bool IsA (const IDatatype &Type) const { return OnIsA(Type); }
		bool IsACoreType (DWORD dwType) const;
		bool IsAbstract () const { return OnIsAbstract(); }
		bool IsAny () const { return OnIsAny(); }
		void Mark () { OnMark(); }
		void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const;

	private:

		//	IDatatype virtuals

		virtual bool OnAddMember (const CString &sName, EMemberType iType, CDatum dType, CString *retsError = NULL) { throw CException(errFail); }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream) = 0;
		virtual bool OnEquals (const IDatatype &Src) const = 0;
		virtual int OnFindMember (const CString &sName) const { return -1; }
		virtual EMemberType OnHasMember (const CString &sName, CDatum *retdType = NULL) const { return EMemberType::None; }
		virtual ECategory OnGetClass () const = 0;
		virtual DWORD OnGetCoreType () const { return UNKNOWN; }
		virtual EImplementation OnGetImplementation () const = 0;
		virtual SMemberDesc OnGetMember (int iIndex) const { throw CException(errFail); }
		virtual int OnGetMemberCount () const { return 0; }
		virtual CDatum OnGetMembersAsTable () const { return CDatum(); }
		virtual bool OnIsA (const IDatatype &Type) const { return (Type == *this) || Type.IsAny(); }
		virtual bool OnIsAbstract () const { return false; }
		virtual bool OnIsAny () const { return false; }
		virtual void OnMark () { }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const = 0;
		
		CString m_sFullyQualifiedName;
	};

class CDatatypeList
	{
	public:
		CDatatypeList (const std::initializer_list<CDatum> &List = {});

		bool operator == (const CDatatypeList &Src) const;
		bool operator != (const CDatatypeList &Src) const { return !(*this == Src); }

		void AddType (CDatum dType) { m_Types.Insert(dType); }
		static bool Deserialize (CDatum::EFormat iFormat, IByteStream &Stream, CDatatypeList &retList);
		bool IsA (const IDatatype &Type) const;
		void Mark ();
		void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const;

	private:
		TArray<CDatum> m_Types;
	};

class CAEONTypeSystem
	{
	public:
		CDatum AddAnonymousSchema (const TArray<IDatatype::SMemberDesc> &Columns);
		bool AddType (CDatum dType);
		static CDatum CreateDatatypeClass (const CString &sFullyQualifiedName, const CDatatypeList &Implements, IDatatype **retpNewType = NULL);
		static CDatum CreateDatatypeSchema (const CString &sFullyQualifiedName, const CDatatypeList &Implements, IDatatype **retpNewType = NULL);
		static CDatum CreateDatatypeSchema (const CString &sFullyQualifiedName, IDatatype **retpNewType = NULL)
			{ return CreateDatatypeSchema(sFullyQualifiedName, { CAEONTypeSystem::GetCoreType(IDatatype::TABLE) }, retpNewType); }
		CDatum FindType (const CString &sFullyQualifiedName, const IDatatype **retpDatatype = NULL) const;
		static CDatum GetCoreType (DWORD dwType);
		static int GetCoreTypeCount () { return m_CoreTypes.GetCount(); }
		static const TArray<CDatum> &GetCoreTypes () { if (m_CoreTypes.GetCount() == 0) InitCoreTypes(); return m_CoreTypes; }
		bool InitFrom (CDatum dSerialized, CString *retsError = NULL);
		bool IsEmpty () const { return m_Types.GetCount() == 0; }
		static CString MakeFullyQualifiedName (const CString &sFullyQualifiedScope, const CString &sName);
		void Mark ();
		static void MarkCoreTypes ();
		static CString ParseNameFromFullyQualifiedName (const CString &sValue);
		CDatum ResolveType (CDatum dType) const;
		CDatum Serialize () const;

		static CDatum CreateAnonymousSchema (const TArray<IDatatype::SMemberDesc> &Columns);
		static CAEONTypeSystem &Null () { return m_Null; }

	private:
		static void AddCoreType (IDatatype *pNewDatatype);
		static IDatatype *CreateSchemaTable ();
		static void InitCoreTypes ();

		TSortMap<CString, CDatum> m_Types;
		DWORD m_dwNextAnonymousID = 1;

		static TArray<CDatum> m_CoreTypes;
		static CAEONTypeSystem m_Null;
	};

