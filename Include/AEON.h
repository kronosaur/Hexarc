//	AEON.h
//
//	Archon Engine Object Notation
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	AEON provides a garbage-collected, portable, serializable storage
//	system for various structures.
//
//	1. Requires Foundation
//	2. Include AEON.h
//	3. Link with AEON.lib

#pragma once

#ifdef DEBUG
//#define DEBUG_BLOB_PERF
#endif

class CAEONTypeSystem;
class CComplexStruct;
class CNumberValue;
class IAEONParseExtension;
class IAEONTable;
class IComplexDatum;
class IComplexFactory;
class IDatatype;
class IInvokeCtx;

//	Data encoding constants

const DWORD_PTR AEON_TYPE_STRING =			0x00;
const DWORD_PTR AEON_TYPE_NUMBER =			0x01;
const DWORD_PTR AEON_TYPE_VOID =			0x02;
const DWORD_PTR AEON_TYPE_COMPLEX =			0x03;

const DWORD_PTR AEON_TYPE_MASK =			0x00000003;
const DWORD_PTR AEON_POINTER_MASK =			~AEON_TYPE_MASK;
const DWORD_PTR AEON_MARK_MASK =			0x00000001;

const DWORD_PTR AEON_NUMBER_TYPE_MASK =		0x0000000F;
const DWORD_PTR AEON_NUMBER_CONSTANT =		0x01;
const DWORD_PTR AEON_NUMBER_INTEGER =		0x05;
const DWORD_PTR AEON_NUMBER_DOUBLE =		0x0D;
const DWORD_PTR AEON_NUMBER_MASK =			0xFFFFFFF0;

const DWORD_PTR AEON_MIN_28BIT =			0xF8000000;
const DWORD_PTR AEON_MAX_28BIT =			0x07FFFFFF;

typedef void (*MARKPROC)(void);

//	CDatum
//
//	These values can be passed around at will, but we must be able to call Mark
//	on all values that need to be preserved across garbage-collection runs.
//	
//	A CDatum is a 64-bit atom that encodes different types
//
//	m_dwData format
//
//				6666 5555 5555 5544 4444 4444 3333 3333 3322 2222 2222 1111 1111 1100 0000 0000
//				3210 9875 5432 1098 7654 3210 9876 5432 1098 7654 3210 9876 5432 1098 7654 3210
//
//	String:		[ Pointer to string                                                         ]00		//	NULL == Nil
//
//	Constants:	                                        [ const ID        ] 0000 0000 0000 0001
//	Nil:		                                        0000 0110 0110 0110 0000 0000 0000 0001		//	0x06660001
//	True:		                                        1010 1010 1010 1010 0000 0000 0000 0001		//	0xAAAA0001
//	Free:		                                        1111 1110 1110 1110 0000 0000 0000 0001		//	0xFEEE0001
//
//	Symbols:	                                        [ 27-bit ID                     ]1 0001
//
//	Integer:	[ 32-bit integer                      ] 0000 0000 0000 0000 0000 0000 0000 0101
//	Unused:	                                                                               1001
//	Double:		                                        [ Index to double                ] 1101
//
//	Void:		[ Pointer to void (external)                                                ]10
//	Complex:	[ Pointer to complex obj                                                    ]11
//
//	When we point to a string, we point to an allocated
//	block of memory that conforms to a literal CString
//
//				int		Length of string (if negative, this is a literal string--don't free it)
//	pointer ->	char	first character
//				char	second character
//				...
//				char	'\0'	(to mark the string we [temporarily] change the last char to 0xff)

class CDatum
	{
	public:
		enum Types
			{
			typeUnknown	=		-1,

			typeNil =			0,
			typeTrue =			1,
			typeInteger32 =		2,
			typeString =		3,
			typeArray =			4,
			typeSymbol =		5,
			typeDouble =		6,
			typeStruct =		7,
			typeDateTime =		8,
			typeIntegerIP =		9,
			typeInteger64 =		10,
			typeBinary =		11,
			typeVoid =			12,
			typeImage32 =		13,
			typeObject =		14,			//	A typed struct
			typeTable =			15,			//	A table (each column is a typed array)
			typeDatatype =		16,			//	A datatype (IDatatype)
			typeTimeSpan =		17,	
			typeNaN =			18,

			typeCustom =		100,
			};

		enum class ECallType
			{
			None =				0,			//	Not a function
			Call =				1,			//	A standard function call
			Library =			2,			//	A library function implemented in native code

			Invoke =			3,			//	Hexarc invoke message
			};

		enum class EFormat
			{
			Unknown =			-1,

			AEONScript =		0,
			JSON =				1,
			AEONLocal =			2,			//	Serialized to a local machine
			TextUTF8 =			3,			//	Plain text (unstructured)
			Binary =			4,			//	Binary
			};

		enum class InvokeResult
			{
			unknown,

			error,
			ok,
			runFunction,
			runInputRequest,
			runInvoke,
			};

		CDatum () : m_dwData(0) { }
		CDatum (int iValue);
		CDatum (DWORD dwValue);
		CDatum (DWORDLONG ilValue);
		CDatum (const CString &sString);
		CDatum (CString &&sString);
		CDatum (CStringBuffer &&sString);
		CDatum (double rValue);
		CDatum (IComplexDatum *pValue);
		CDatum (const CDateTime &DateTime);
		CDatum (const CIPInteger &Value);
		CDatum (const CRGBA32Image &Value);
		CDatum (CRGBA32Image &&Value);
		CDatum (const CTimeSpan &TimeSpan);
		explicit CDatum (Types iType);
		explicit CDatum (bool bValue) : m_dwData(bValue ? CONST_TRUE : 0) { }

		static CDatum CreateArrayAsType (CDatum dType, CDatum dValue = CDatum());
		static CDatum CreateAsType (CDatum dType, CDatum dValue = CDatum());
		static bool CreateBinary (IByteStream &Stream, int iSize, CDatum *retDatum);
		static bool CreateBinaryFromHandoff (CStringBuffer &Buffer, CDatum *retDatum);
		static bool CreateFromAttributeList (const CAttributeList &Attribs, CDatum *retdDatum);
		static bool CreateFromFile (const CString &sFilespec, EFormat iFormat, CDatum *retdDatum, CString *retsError);
		static bool CreateFromStringValue (const CString &sValue, CDatum *retdDatum);
		static bool CreateIPInteger (const CIPInteger &Value, CDatum *retdDatum);
		static bool CreateIPIntegerFromHandoff (CIPInteger &Value, CDatum *retdDatum);
		static CDatum CreateNaN ();
		static CDatum CreateObject (CDatum dType, CDatum dValue = CDatum());
		static bool CreateStringFromHandoff (CString &sString, CDatum *retDatum);
		static bool CreateStringFromHandoff (CStringBuffer &String, CDatum *retDatum);
		static CDatum CreateTable (CDatum dType, CDatum dValue = CDatum());
		static bool CreateTableFromDesc (CAEONTypeSystem &TypeSystem, CDatum dDesc, CDatum &retdDatum);
		static bool Deserialize (EFormat iFormat, IByteStream &Stream, IAEONParseExtension *pExtension, CDatum *retDatum);
		static bool Deserialize (EFormat iFormat, IByteStream &Stream, CDatum *retDatum) { return Deserialize(iFormat, Stream, NULL, retDatum); }
		static Types GetStringValueType (const CString &sValue);
		static CDatum VectorOf (Types iType, CDatum dValues = CDatum());

		operator int () const;
		operator DWORD () const;
		operator DWORDLONG () const;
		operator double () const;
		operator const CIPInteger & () const;
		operator const CString & () const;
		operator const CDateTime & () const;
		operator const CRGBA32Image & () const;
		operator const CTimeSpan & () const;
		operator const IDatatype & () const;

		//	Standard interface
		void Append (CDatum dValue);
		int AsArrayIndex () const;
		CDateTime AsDateTime () const;
		CIPInteger AsIPInteger () const;
		CDatum AsOptions (bool *retbConverted = NULL) const;
		CString AsString () const;
		TArray<CString> AsStringArray () const;
		size_t CalcMemorySize () const;
		size_t CalcSerializeSize (EFormat iFormat) const;
		CDatum Clone () const;
		bool Contains (CDatum dValue, TArray<IComplexDatum *> &retChecked) const;
		bool EnumElements (std::function<bool(CDatum)> fn);
		bool Find (CDatum dValue, int *retiIndex = NULL) const;
		bool FindElement (const CString &sKey, CDatum *retpValue);
		int GetArrayCount () const;
		CDatum GetArrayElement (int iIndex) const;
		Types GetBasicType () const;
		int GetBinarySize () const;
		IComplexDatum *GetComplex () const;
		int GetCount () const;
		CDatum GetDatatype () const;
		CDatum GetElement (IInvokeCtx *pCtx, int iIndex) const;
		CDatum GetElement (int iIndex) const;
		CDatum GetElement (IInvokeCtx *pCtx, const CString &sKey) const;
		CDatum GetElement (const CString &sKey) const;
		CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const;
		CRGBA32Image *GetImageInterface ();
		CRGBA32Image &GetImageInterfaceOrThrow () { auto pValue = GetImageInterface(); if (!pValue) throw CException(errFail); else return *pValue; }
		CString GetKey (int iIndex) const;
		CDatum GetMethod (const CString &sMethod) const;
		IAEONTable *GetTableInterface ();
		const IAEONTable *GetTableInterface () const { return const_cast<CDatum *>(this)->GetTableInterface(); }
		const CString &GetTypename () const;
		void GrowToFit (int iCount);
		bool IsArray () const;
		bool IsContainer () const;
		bool IsMemoryBlock () const;
		bool IsEqual (CDatum dValue) const;
		bool IsError () const;
		bool IsIdenticalToNil () const { return (m_dwData == 0); }
		bool IsNaN () const { return (m_dwData == CONST_NAN); }
		bool IsNil () const;
		void Mark ();
		void ResolveDatatypes (const CAEONTypeSystem &TypeSystem);
		void Serialize (EFormat iFormat, IByteStream &Stream) const;
		CString SerializeToString (EFormat iFormat) const;
		void SetElement (IInvokeCtx *pCtx, const CString &sKey, CDatum dValue);
		void SetElement (const CString &sKey, CDatum dValue);
		void SetElement (int iIndex, CDatum dValue);
		void SetElementAt (CDatum dIndex, CDatum dValue);
		void WriteBinaryToStream (IByteStream &Stream, int iPos = 0, int iLength = -1, IProgressEvents *pProgress = NULL) const;

		static int Compare (CDatum dValue1, CDatum dValue2) { return DefaultCompare(NULL, dValue1, dValue2); }

		//	Math related methods
		bool FitsAsDWORDLONG () const { Types iType = GetNumberType(NULL); return (iType == typeInteger32 || iType == typeInteger64); }
		Types GetNumberType (int *retiValue, CDatum *retdConverted = NULL) const;
		bool IsNumber () const;
		bool IsNumberInt32 (int *retiValue = NULL) const;
		CDatum MathAbs () const;
		CDatum MathAverage () const;
		CDatum MathMax () const;
		CDatum MathMedian () const;
		CDatum MathMin () const;
		CDatum MathSum () const;

		//	Function related methods
		bool CanInvoke () const;
		ECallType GetCallInfo (CDatum *retdCodeBank = NULL, DWORD **retpIP = NULL) const;
		InvokeResult Invoke (IInvokeCtx *pCtx, CDatum dLocalEnv, DWORD dwExecutionRights, CDatum *retdResult);
		InvokeResult InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, CDatum *retdResult);
		bool InvokeMethodImpl (const CString &sMethod, IInvokeCtx &Ctx, CDatum dLocalEnv, CDatum &retdResult);

		//	Utilities
		void AsAttributeList (CAttributeList *retAttribs) const;
		CDatum MergeKeysNoCase () const;
		void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL);

		//	Implementation details
		static bool FindExternalType (const CString &sTypename, IComplexFactory **retpFactory);
		static void MarkAndSweep ();
		static bool RegisterExternalType (const CString &sTypename, IComplexFactory *pFactory);
		static void RegisterMarkProc (MARKPROC fnProc);
		static CDatum raw_AsComplex (const void *pValue) { CDatum dResult; dResult.m_dwData = ((DWORD_PTR)pValue | AEON_TYPE_COMPLEX); return dResult; }
		IComplexDatum *raw_GetComplex () const { return (IComplexDatum *)(m_dwData & AEON_POINTER_MASK); }

	private:

		static constexpr DWORD_PTR CONST_NAN =		0x12020001;
		static constexpr DWORD_PTR CONST_TRUE =		0xaaaa0001;
		static constexpr DWORD_PTR CONST_FREE =		0xfeee0001;

		size_t CalcSerializeSizeAEONScript (EFormat iFormat) const;
		static int DefaultCompare (void *pCtx, const CDatum &dKey1, const CDatum &dKey2);
		static bool DeserializeAEONScript (IByteStream &Stream, IAEONParseExtension *pExtension, CDatum *retDatum);
		static bool DeserializeAEONScript (IByteStream &Stream, CDatum *retDatum) { return DeserializeAEONScript(Stream, NULL, retDatum); }
		static bool DeserializeJSON (IByteStream &Stream, CDatum *retDatum);
		static bool DeserializeTextUTF8 (IByteStream &Stream, CDatum *retDatum);
		static bool DetectFileFormat (const CString &sFilespec, IMemoryBlock &Data, EFormat *retiFormat, CString *retsError);
		DWORD GetNumberIndex () const { return (DWORD)(m_dwData >> 4); }
		double raw_GetDouble () const;
		const CString &raw_GetString () const { ASSERT(AEON_TYPE_STRING == 0x00); return *(CString *)&m_dwData; }
		void SerializeAEONScript (EFormat iFormat, IByteStream &Stream) const;
		void SerializeJSON (IByteStream &Stream) const;

		template<class FUNC> CDatum MathArrayOp () const;

		DWORD_PTR m_dwData;
	};

inline int KeyCompare (const CDatum &dKey1, const CDatum &dKey2) { return CDatum::Compare(dKey1, dKey2); }

//	Type System ----------------------------------------------------------------

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

		enum class ECategory
			{
			Unknown,

			Simple,							//	Datatype that does not refer to other types.
			Number,							//	A number type
			Array,							//	An array of some other type.
			ClassDef,						//	A ordered set of fields and types
			Function,						//	A function type
			Schema,							//	A table definition
			};

		enum class EImplementation
			{
			Unknown,

			Any,
			Array,
			Class,
			Number,
			Schema,
			Simple,
			};

		enum class EMemberType
			{
			None,

			ArrayElement,
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
		ECategory GetClass () const { return OnGetClass(); }
		DWORD GetCoreType () const { return OnGetCoreType(); }
		const CString &GetFullyQualifiedName () const { return m_sFullyQualifiedName; }
		EImplementation GetImplementation () const { return OnGetImplementation(); }
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
		virtual EMemberType OnHasMember (const CString &sName, CDatum *retdType = NULL) const { return EMemberType::None; }
		virtual ECategory OnGetClass () const = 0;
		virtual DWORD OnGetCoreType () const { return UNKNOWN; }
		virtual EImplementation OnGetImplementation () const = 0;
		virtual SMemberDesc OnGetMember (int iIndex) const { throw CException(errFail); }
		virtual int OnGetMemberCount () const { return 0; }
		virtual CDatum OnGetMembersAsTable () const { return CDatum(); }
		virtual bool OnIsA (const IDatatype &Type) const { return (&Type == this) || Type.IsAny(); }
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

//	IAEONTable Interface -------------------------------------------------------

class IAEONTable
	{
	public:
		enum class EResult
			{
			OK,

			InvalidParam,
			NotATable,
			NotImplemented,
			NotMutable,
			};

		struct SSubset
			{
			TArray<int> Cols;		//	If empty, all columns
			TArray<int> Rows;		//	Rows
			bool bAllRows = false;
			};

		virtual EResult AppendColumn (CDatum dColumn) { return EResult::NotImplemented; }
		virtual EResult AppendRow (CDatum dRow) { return EResult::NotImplemented; }
		virtual EResult AppendSlice (CDatum dSlice) { return EResult::NotImplemented; }
		virtual EResult AppendTable (CDatum dTable) { return EResult::NotImplemented; }
		virtual EResult DeleteAllRows () { return EResult::NotImplemented; }
		virtual bool FindCol (const CString &sName, int *retiCol = NULL) const { return false; }
		virtual CDatum GetCol (int iIndex) const { return CDatum(); }
		virtual int GetColCount () const { return 0; }
		virtual CString GetColName (int iCol) const { return NULL_STR; }
		virtual CDatum GetDataSlice (int iFirstRow, int iRowCount) const { return CDatum(); }
		virtual CDatum GetFieldValue (int iRow, int iCol) const { return CDatum(); }
		virtual int GetRowCount () const { return 0; }
		virtual bool IsSameSchema (CDatum dSchema) const { return false; }
		virtual bool SetFieldValue (int iRow, int iCol, CDatum dValue) { return false; }

		static bool CreateRef (CAEONTypeSystem& TypeSystem, CDatum dTable, SSubset&& Subset, CDatum& retdValue);
		static bool CreateSchema (CAEONTypeSystem& TypeSystem, CDatum dTable, SSubset& Subset, CDatum& retdSchema);
		static bool CreateSchemaFromDesc (CAEONTypeSystem& TypeSystem, CDatum dSchemaDesc, CDatum& retdSchema);
	};

//	IComplexDatum --------------------------------------------------------------

class IComplexDatum
	{
	public:
		static constexpr DWORD FLAG_SERIALIZE_AS_STRUCT =	0x00000001;
		static constexpr DWORD FLAG_SERIALIZE_NO_TYPENAME =	0x00000002;

		IComplexDatum () { }
		virtual ~IComplexDatum () { }

		virtual void Append (CDatum dDatum) { }
		virtual int AsArrayIndex () const { return -1; }
		virtual CString AsString () const { return NULL_STR; }
		virtual size_t CalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const;
		virtual size_t CalcMemorySize () const = 0;
		virtual bool CanInvoke () const { return false; }
		virtual const CDateTime &CastCDateTime () const { return NULL_DATETIME; }
		virtual const CIPInteger &CastCIPInteger () const { return NULL_IPINTEGER; }
		virtual const CRGBA32Image &CastCRGBA32Image () const { return CRGBA32Image::Null(); }
		virtual const CString &CastCString () const { return NULL_STR; }
		virtual const CTimeSpan &CastCTimeSpan () const { return CTimeSpan::Null(); }
		virtual double CastDouble () const { return CDatum::CreateNaN(); }
		virtual DWORDLONG CastDWORDLONG () const { return 0; }
		virtual const IDatatype &CastIDatatype () const;
		virtual int CastInteger32 () const { return 0; }
		void ClearMark () { m_bMarked = false; }
		virtual IComplexDatum *Clone () const = 0;
		virtual bool Contains (CDatum dValue, TArray<IComplexDatum *> &retChecked) const { return false; }
		bool DeserializeAEONScript (CDatum::EFormat iFormat, const CString &sTypename, CCharStream *pStream);
		virtual bool DeserializeJSON (const CString &sTypename, const TArray<CDatum> &Data);
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const { return false; }
		virtual bool FindElement (const CString &sKey, CDatum *retpValue) { return false; }
		virtual CDatum::Types GetBasicType () const = 0;
		virtual int GetBinarySize () const { return CastCString().GetLength(); }
		virtual CDatum::ECallType GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const { return CDatum::ECallType::None; }
		virtual int GetCount () const = 0;
		virtual CDatum GetDatatype () const;
		virtual CDatum GetElement (IInvokeCtx *pCtx, int iIndex) const { return GetElement(iIndex); }
		virtual CDatum GetElement (int iIndex) const = 0;
		virtual CDatum GetElement (IInvokeCtx *pCtx, const CString &sKey) const { return GetElement(sKey); }
		virtual CDatum GetElement (const CString &sKey) const { return CDatum(); }
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const;
		virtual CRGBA32Image *GetImageInterface () { return NULL; }
		virtual CString GetKey (int iIndex) const { return NULL_STR; }
		virtual CDatum GetMethod (const CString &sMethod) const { return CDatum(); }
		virtual CDatum::Types GetNumberType (int *retiValue) { return CDatum::typeNaN; }
		virtual IAEONTable *GetTableInterface () { return NULL; }
		virtual const CString &GetTypename () const = 0;
		virtual void GrowToFit (int iCount) { }
		virtual CDatum::InvokeResult Invoke (IInvokeCtx *pCtx, CDatum dLocalEnv, DWORD dwExecutionRights, CDatum *retdResult) { *retdResult = CDatum(); return CDatum::InvokeResult::ok; }
		virtual CDatum::InvokeResult InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, CDatum *retdResult) { *retdResult = CDatum(); return CDatum::InvokeResult::ok; }
		virtual bool InvokeMethodImpl(CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CDatum dLocalEnv, CDatum &retdResult) { retdResult = CString("Methods not supported."); return false; }
		virtual bool IsArray () const = 0;
		virtual bool IsContainer () const { return false; }
		virtual bool IsError () const { return false; }
		virtual bool IsIPInteger () const { return false; }
		bool IsMarked () const { return m_bMarked; }
		virtual bool IsMemoryBlock () const { const CString &sData = CastCString(); return (sData.GetLength() > 0); }
		virtual bool IsNil () const { return false; }
		void Mark () { if (!m_bMarked) { m_bMarked = true; OnMarked(); } }	//	Check m_bMarked to avoid infinite recursion
		virtual CDatum MathAbs () const { return CDatum::CreateNaN(); }
		virtual CDatum MathAverage () const;
		virtual CDatum MathMax () const { return CDatum::CreateNaN(); }
		virtual CDatum MathMedian () const { return CDatum::CreateNaN(); }
		virtual CDatum MathMin () const { return CDatum::CreateNaN(); }
		virtual CDatum MathSum () const;
		virtual void ResolveDatatypes (const CAEONTypeSystem &TypeSystem) { }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const;
		virtual void SetElement (IInvokeCtx *pCtx, const CString &sKey, CDatum dDatum) { SetElement(sKey, dDatum); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) { }
		virtual void SetElement (int iIndex, CDatum dDatum) { }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum);
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) { }
		virtual void WriteBinaryToStream (IByteStream &Stream, int iPos = 0, int iLength = -1, IProgressEvents *pProgress = NULL) const;

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const { return 0; }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) { ASSERT(false); return false; }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct);
		virtual DWORD OnGetSerializeFlags () const { return 0; }
		virtual void OnMarked () { }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const { ASSERT(false); }
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const;

		size_t CalcSerializeAsStructSize (CDatum::EFormat iFormat) const;
		void SerializeAsStruct (CDatum::EFormat iFormat, IByteStream &Stream) const;
		CString StructAsString () const;

	private:
		bool m_bMarked = false;
	};

class IComplexFactory
	{
	public:
		virtual ~IComplexFactory () { }

		virtual IComplexDatum *Create () = 0;
	};

//	CComplexArray

class CComplexArray : public IComplexDatum
	{
	public:
		CComplexArray () { }
		CComplexArray (CDatum dSrc);
		CComplexArray (const TArray<CString> &Src);
		CComplexArray (const TArray<CDatum> &Src);
		explicit CComplexArray (int iCount) { m_Array.InsertEmpty(iCount); }

		void Delete (int iIndex) { m_Array.Delete(iIndex); }
		bool FindElement (CDatum dValue, int *retiIndex = NULL) const;
		void Insert (CDatum Element, int iIndex = -1) { m_Array.Insert(Element, iIndex); }
		void InsertEmpty (int iCount = 1, int iIndex = -1) { m_Array.InsertEmpty(iCount, iIndex); }
		void SetAt (int iIndex, CDatum dValue) { m_Array[iIndex] = dValue; }

		//	IComplexDatum
		virtual void Append (CDatum dDatum) override { m_Array.Insert(dDatum); }
		virtual CString AsString () const override;
		virtual size_t CalcMemorySize () const override;
		virtual IComplexDatum *Clone () const override { return new CComplexArray(m_Array); }
		virtual bool Contains (CDatum dValue, TArray<IComplexDatum *> &retChecked) const override;
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const override { return FindElement(dValue, retiIndex); }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeArray; }
		virtual int GetCount () const override { return m_Array.GetCount(); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY); }
		virtual CDatum GetElement (int iIndex) const override { return ((iIndex >= 0 && iIndex < m_Array.GetCount()) ? m_Array[iIndex] : CDatum()); }
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override;
		virtual const CString &GetTypename () const override;
		virtual void GrowToFit (int iCount) override { m_Array.GrowToFit(iCount); }
		virtual bool IsArray () const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil () const override { return (GetCount() == 0); }
		virtual CDatum MathAbs () const override;
		virtual CDatum MathMax () const override;
		virtual CDatum MathMin () const override;
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { if (pfCompare) m_Array.Sort(pCtx, pfCompare, Order); else m_Array.Sort(Order); }
		virtual void SetElement (int iIndex, CDatum dDatum) override { if (iIndex >= 0 && iIndex < m_Array.GetCount()) m_Array[iIndex] = dDatum; }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override;

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual void OnMarked () override;

		TArray<CDatum> m_Array;
	};

class CComplexBinary : public IComplexDatum
	{
	public:
		CComplexBinary () : m_pData(NULL) { }
		CComplexBinary (IByteStream &Stream, int iLength);
		~CComplexBinary ();

		int GetLength () const { return (m_pData ? ((CString *)&m_pData)->GetLength() : 0); }
		void TakeHandoff (CStringBuffer &Buffer);

		//	IComplexDatum
		virtual void Append (CDatum dDatum) override;
		virtual CString AsString () const override;
		virtual size_t CalcMemorySize () const override { return CastCString().GetLength() + sizeof(DWORD) + 1; }
		virtual const CString &CastCString () const override;
		virtual IComplexDatum *Clone () const override;
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeBinary; }
		virtual int GetBinarySize () const override { return GetLength(); }
		virtual int GetCount () const override { return 1; }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::BINARY); }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual const CString &GetTypename () const override;
		virtual bool IsArray () const override { return false; }
		virtual bool IsNil () const override { return (m_pData == NULL); }

	protected:
		//	IComplexDatum
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:
		LPSTR GetBuffer () const { return (m_pData - sizeof(DWORD)); }

		LPSTR m_pData;						//	Points to data (previous DWORD is length)
	};

class CComplexBinaryFile : public IComplexDatum
	{
	public:
		CComplexBinaryFile () : m_dwLength(0) { }
		CComplexBinaryFile (IByteStream &Stream, int iLength);
		~CComplexBinaryFile ();

		void Append (IMemoryBlock &Data);
		int GetLength () const { return m_dwLength; }

		//	IComplexDatum
		virtual void Append (CDatum dDatum) override;
		virtual size_t CalcMemorySize () const override { return sizeof(CComplexBinaryFile); }
		virtual const CString &CastCString () const override;
		virtual IComplexDatum *Clone () const override;
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeBinary; }
		virtual int GetBinarySize () const override { return m_dwLength; }
		virtual int GetCount () const override { return 1; }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual const CString &GetTypename () const override;
		virtual bool IsArray () const override { return false; }
		virtual bool IsMemoryBlock () const override { return false; }
		virtual bool IsNil () const override { return (m_dwLength == 0); }
		virtual void WriteBinaryToStream (IByteStream &Stream, int iPos = 0, int iLength = -1, IProgressEvents *pProgress = NULL) const override;

	protected:
		//	IComplexDatum
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnMarked () override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:
		struct SHeader
			{
			DWORD dwSignature;
			DWORD dwRefCount;
			};

		void CreateBinaryFile (CString *retsFilespec, CFile *retFile);
		bool DecrementRefCount ();
		void IncrementRefCount () const;

		CString m_sFilespec;
		mutable CFile m_File;
		DWORD m_dwLength;
	};

class CComplexDateTime : public IComplexDatum
	{
	public:
		CComplexDateTime () { }
		CComplexDateTime (const CDateTime DateTime) : m_DateTime(DateTime) { }
		
		static bool CreateFromString (const CString &sString, CDateTime *retDateTime);
		static bool CreateFromString (const CString &sString, CDatum *retdDatum);

		virtual CString AsString () const override;
		virtual size_t CalcMemorySize () const override { return sizeof(CComplexDateTime); }
		virtual const CDateTime &CastCDateTime () const override { return m_DateTime; }
		virtual IComplexDatum *Clone () const override { return new CComplexDateTime(m_DateTime); }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeDateTime; }
		virtual int GetCount () const override { return partCount; }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::DATE_TIME); }
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual const CString &GetTypename () const override;
		virtual bool IsArray () const override { return true; }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;

	private:
		enum EParts
			{
			partYear = 0,
			partMonth = 1,
			partDay = 2,
			partHour = 3,
			partMinute = 4,
			partSecond = 5,
			partMillisecond = 6,

			partCount = 7,
			};

		CDateTime m_DateTime;
	};

class CComplexImage32 : public IComplexDatum
	{
	public:
		CComplexImage32 () { }
		CComplexImage32 (const CRGBA32Image &Src) :
				m_Image(Src)
			{ }

		CComplexImage32 (CRGBA32Image &&Src) :
				m_Image(std::move(Src))
			{ }

		//	IComplexDatum
		virtual CString AsString () const override { return strPattern("Image %dx%d", m_Image.GetWidth(), m_Image.GetHeight()); }
		virtual size_t CalcMemorySize () const override { return (size_t)m_Image.GetWidth() * (size_t)m_Image.GetHeight() * sizeof(DWORD); }
		virtual const CRGBA32Image &CastCRGBA32Image () const override { return m_Image; }
		virtual IComplexDatum *Clone () const override;
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeImage32; }
		virtual int GetCount () const override { return 1; }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual CRGBA32Image *GetImageInterface () override { return &m_Image; }
		virtual const CString &GetTypename () const override;
		virtual bool IsArray () const override { return false; }
		virtual bool IsNil () const override { return m_Image.IsEmpty(); }

	protected:
		//	IComplexDatum
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:
		CRGBA32Image m_Image;
	};

class CComplexStruct : public IComplexDatum
	{
	public:
		CComplexStruct () { }
		CComplexStruct (CDatum dSrc);
		CComplexStruct (const TSortMap<CString, CString> &Src);
		CComplexStruct (const TSortMap<CString, CDatum> &Src);

		void DeleteElement (const CString &sKey) { m_Map.DeleteAt(sKey); }

		//	IComplexDatum
		virtual void Append (CDatum dDatum) override { AppendStruct(dDatum); }
		virtual CString AsString () const override;
		virtual size_t CalcMemorySize () const override;
		virtual IComplexDatum *Clone () const override { return new CComplexStruct(m_Map); }
		virtual bool Contains (CDatum dValue, TArray<IComplexDatum *> &retChecked) const override;
		virtual bool FindElement (const CString &sKey, CDatum *retpValue) override;
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeStruct; }
		virtual int GetCount () const override { return m_Map.GetCount(); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::STRUCT); }
		virtual CDatum GetElement (int iIndex) const override { return ((iIndex >= 0 && iIndex < m_Map.GetCount()) ? m_Map[iIndex] : CDatum()); }
		virtual CDatum GetElement (const CString &sKey) const override { CDatum *pValue = m_Map.GetAt(sKey); return (pValue ? *pValue : CDatum()); }
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override;
		virtual CString GetKey (int iIndex) const override { return m_Map.GetKey(iIndex); }
		virtual const CString &GetTypename () const override;
		virtual void GrowToFit (int iCount) override { m_Map.GrowToFit(iCount); }
		virtual bool IsArray () const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil () const override { return (GetCount() == 0); }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override { SerializeAsStruct(iFormat, Stream); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Map.SetAt(sKey, dDatum); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override;

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override { return CalcSerializeAsStructSize(iFormat); }
		virtual void OnMarked () override;

		void AppendStruct (CDatum dDatum);

		TSortMap<CString, CDatum> m_Map;
	};

template <class VALUE> class TExternalDatum : public IComplexDatum
	{
	public:
		TExternalDatum ()
			{ }

		static VALUE *Upconvert (CDatum dData)
			{
			IComplexDatum *pComplex = dData.GetComplex();
			if (pComplex == NULL)
				return NULL;

			if (!strEquals(pComplex->GetTypename(), VALUE::StaticGetTypename()))
				return NULL;

			return (VALUE *)pComplex;
			}

		static VALUE *UpconvertRaw (CDatum dData)
			{
#ifdef DEBUG
			VALUE *pResult = Upconvert(dData);
			if (!pResult)
				throw CException(errFail);

			return pResult;
#else
			IComplexDatum *pComplex = dData.raw_GetComplex();
			return (VALUE *)pComplex;
#endif
			}

		static void RegisterFactory ()
			{
			CDatum::RegisterExternalType(VALUE::StaticGetTypename(), new CFactory);
			}

		//	IComplexDatum
		virtual CString AsString () const override { return strPattern("[%s]", GetTypename()); }
		virtual size_t CalcMemorySize () const override { return sizeof(VALUE); }
		virtual IComplexDatum *Clone () const override { ASSERT(false); return NULL; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeCustom; }
		virtual int GetCount () const override { return 1; }
		virtual CDatum GetElement (IInvokeCtx *pCtx, int iIndex) const override { return GetElement(iIndex); }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual CDatum GetElement (IInvokeCtx *pCtx, const CString &sKey) const override { return GetElement(sKey); }
		virtual CDatum GetElement (const CString &sKey) const override { return CDatum(); }
		virtual CString GetKey (int iIndex) const override { return NULL_STR; }
		virtual const CString &GetTypename () const override { return VALUE::StaticGetTypename(); }
		virtual bool IsArray () const override { return false; }

	protected:

	private:
		class CFactory : public IComplexFactory
			{
			public:
				CFactory ()
					{ }

				//	IComplexFactory
				virtual IComplexDatum *Create () { return new VALUE; }
			};
	};

//	Numbers --------------------------------------------------------------------

class CNumberValue
	{
	public:
		CNumberValue () { }
		CNumberValue (CDatum dValue) { InitFrom(dValue); }
		CNumberValue (const CNumberValue& Src) = delete;
		CNumberValue (CNumberValue&& Src) = delete;

		CNumberValue& operator = (const CNumberValue& Src) = delete;
		CNumberValue& operator = (CNumberValue&& Src) = delete;

		void Abs ();
		void Add (CDatum dValue);
		double AsDouble () const;
		CIPInteger AsIPInteger () const;
		int Compare (CDatum dValue) const {  CNumberValue Src(dValue); return Compare(Src); }
		int Compare (const CNumberValue &Value) const;
		void ConvertToDouble ();
		void ConvertToIPInteger ();
		bool Divide (CDatum dValue);
		bool DivideReversed (CDatum dValue);
		CDatum GetDatum ();
		double GetDouble () const { return m_rValue; }
		int GetInteger () const { return (int)(DWORD_PTR)m_pValue; }
		DWORDLONG GetInteger64 () const { return m_ilValue; }
		const CIPInteger &GetIPInteger () const { return *(CIPInteger *)m_pValue; }
		const CTimeSpan &GetTimeSpan () const { return *(CTimeSpan *)m_pValue; }
		CDatum::Types GetType () const { return m_iType; }
		bool IsNil () const { return m_iType == CDatum::typeNil; }
		bool IsNegative () const;
		bool IsValidNumber () const { return !m_bNotANumber; }
		void Max (CDatum dValue);
		void Min (CDatum dValue);
		bool Mod (CDatum dValue);
		bool ModClock (CDatum dValue);
		void Multiply (CDatum dValue);
		void Power (CDatum dValue);
		void SetDouble (double rValue) { m_rValue = rValue; m_bUpconverted = true; m_iType = CDatum::typeDouble; }
		void SetInteger (int iValue) { m_pValue = (void *)(DWORD_PTR)iValue; m_bUpconverted = true; m_iType = CDatum::typeInteger32; }
		void SetInteger64 (DWORDLONG ilValue) { m_ilValue = ilValue; m_bUpconverted = true; m_iType = CDatum::typeInteger64; }
		void SetIPInteger (const CIPInteger &Value) { m_ipValue = Value; m_pValue = &m_ipValue; m_bUpconverted = true; m_iType = CDatum::typeIntegerIP; }
		void SetTimeSpan (const CTimeSpan &Value) { m_tsValue = Value; m_pValue = &m_tsValue; m_bUpconverted = true; m_iType = CDatum::typeTimeSpan; }
		void SetNaN () { m_bUpconverted = true; m_iType = CDatum::typeNaN; }
		void SetNil () { m_bUpconverted = true; m_iType = CDatum::typeNil; }
		void Subtract (CDatum dValue);
		void Upconvert (CNumberValue &Src);

	private:
		void InitFrom (CDatum dValue);

		CDatum m_dOriginalValue;
		CDatum::Types m_iType = CDatum::typeNil;
		bool m_bUpconverted = false;
		bool m_bNotANumber = false;

		void *m_pValue = NULL;
		double m_rValue = 0.0;
		DWORDLONG m_ilValue = 0;
		CIPInteger m_ipValue;
		CTimeSpan m_tsValue;

		static constexpr int MAX_EXP_FOR_INT32 = 30;
		static const int MAX_BASE_FOR_EXP[MAX_EXP_FOR_INT32 + 1];
	};

//	IInvokeCtx -----------------------------------------------------------------

class IInvokeCtx
	{
	public:
		virtual ~IInvokeCtx () { }

		virtual void *GetLibraryCtx (const CString &sLibrary) { return NULL; }
		virtual void SetUserSecurity (const CString &sUsername, const CAttributeList &Rights) { }
	};

//	CAEONScriptParser ----------------------------------------------------------

class IAEONParseExtension
	{
	public:
		virtual bool ParseAEONArray (CCharStream &Stream, CDatum *retDatum) { return false; }
	};

class CAEONScriptParser
	{
	public:
		enum ETokens
			{
			tkEOF,
			tkError,
			tkDatum,
			tkCloseParen,
			tkCloseBrace,
			tkCloseBracket,
			tkColon,
			tkComma,
			};

		CAEONScriptParser (CCharStream *pStream, IAEONParseExtension *pExtension = NULL) : m_pStream(pStream), m_pExtension(pExtension) { }
		bool ParseDatum (CDatum *retDatum);
		ETokens ParseToken (CDatum *retDatum);

		static bool HasSpecialChars (const CString &sString);

	private:
		ETokens ParseArray (CDatum *retDatum);
		ETokens ParseDateTime (CDatum *retDatum);
		ETokens ParseExternal (CDatum *retDatum);
		ETokens ParseInteger (int *retiValue);
		bool ParseComment ();
		ETokens ParseLiteral (CDatum *retDatum);
		ETokens ParseNumber (CDatum *retDatum);
		ETokens ParseString (CDatum *retDatum);
		ETokens ParseStruct (CDatum *retDatum);

		CCharStream *m_pStream;
		IAEONParseExtension *m_pExtension;
	};

//	Helpers

bool urlParseQuery (const CString &sURL, CString *retsPath, CDatum *retdQuery);

//	Some implementation details

#include "AEONAllocator.h"
#include "AEONVector.h"
#include "AEONUtil.h"
