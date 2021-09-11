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

class CComplexStruct;
class CNumberValue;
class IAEONParseExtension;
class IComplexDatum;
class IComplexFactory;
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
const DWORD_PTR AEON_NUMBER_28BIT =			0x05;
const DWORD_PTR AEON_NUMBER_32BIT =			0x09;
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
//	A CDatum is a 32-bit atom that encodes different types
//
//	m_dwData format (32-bit version)
//
//				3322 2222 2222 1111 1111 1100 0000 0000
//				1098 7654 3210 9876 5432 1098 7654 3210
//
//	String:		[ Pointer to string                 ]00		//	NULL == Nil
//
//	Constants:	[ const ID        ] 0000 0000 0000 0001
//	Nil:		0000 0110 0110 0110 0000 0000 0000 0001		//	0x06660001
//	True:		1010 1010 1010 1010 0000 0000 0000 0001		//	0xAAAA0001
//	Free:		1111 1110 1110 1110 0000 0000 0000 0001		//	0xFEEE0001
//
//	Symbols:	[ 27-bit ID                     ]1 0001
//
//	Integer:	[ 28-bit integer                 ] 0101
//	Integer:	[ Index to 32-bit integer        ] 1001
//	Double:		[ Index to double                ] 1101
//
//	Void:		[ Pointer to void (external)        ]10
//	Complex:	[ Pointer to complex obj            ]11
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

		CDatum (void) : m_dwData(0) { }
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
		explicit CDatum (Types iType);
		explicit CDatum (bool bValue) : m_dwData(bValue ? CONST_TRUE : 0) { }

		static bool CreateBinary (IByteStream &Stream, int iSize, CDatum *retDatum);
		static bool CreateBinaryFromHandoff (CStringBuffer &Buffer, CDatum *retDatum);
		static bool CreateFromAttributeList (const CAttributeList &Attribs, CDatum *retdDatum);
		static bool CreateFromFile (const CString &sFilespec, EFormat iFormat, CDatum *retdDatum, CString *retsError);
		static bool CreateFromStringValue (const CString &sValue, CDatum *retdDatum);
		static bool CreateIPInteger (const CIPInteger &Value, CDatum *retdDatum);
		static bool CreateIPIntegerFromHandoff (CIPInteger &Value, CDatum *retdDatum);
		static bool CreateStringFromHandoff (CString &sString, CDatum *retDatum);
		static bool CreateStringFromHandoff (CStringBuffer &String, CDatum *retDatum);
		static bool Deserialize (EFormat iFormat, IByteStream &Stream, IAEONParseExtension *pExtension, CDatum *retDatum);
		static bool Deserialize (EFormat iFormat, IByteStream &Stream, CDatum *retDatum) { return Deserialize(iFormat, Stream, NULL, retDatum); }
		static Types GetStringValueType (const CString &sValue);

		operator int () const;
		operator DWORD () const;
		operator DWORDLONG () const;
		operator double () const;
		operator const CIPInteger & () const;
		operator const CString & () const;
		operator const CDateTime & () const;
		operator const CRGBA32Image & () const;

		//	Standard interface
		void Append (CDatum dValue);
		CDateTime AsDateTime (void) const;
		CDatum AsOptions (bool *retbConverted = NULL) const;
		CString AsString (void) const;
		TArray<CString> AsStringArray (void) const;
		size_t CalcMemorySize (void) const;
		size_t CalcSerializeSize (EFormat iFormat) const;
		CDatum Clone (void) const;
		bool Find (CDatum dValue, int *retiIndex = NULL) const;
		bool FindElement (const CString &sKey, CDatum *retpValue);
		int GetArrayCount (void) const;
		CDatum GetArrayElement (int iIndex) const;
		Types GetBasicType (void) const;
		int GetBinarySize (void) const;
		IComplexDatum *GetComplex (void) const;
		int GetCount (void) const;
		CDatum GetElement (IInvokeCtx *pCtx, int iIndex) const;
		CDatum GetElement (int iIndex) const;
		CDatum GetElement (IInvokeCtx *pCtx, const CString &sKey) const;
		CDatum GetElement (const CString &sKey) const;
		CString GetKey (int iIndex) const;
		const CString &GetTypename (void) const;
		void GrowToFit (int iCount);
		bool IsMemoryBlock (void) const;
		bool IsEqual (CDatum dValue) const;
		bool IsError (void) const;
		bool IsNil (void) const;
		void Mark (void);
		void Serialize (EFormat iFormat, IByteStream &Stream) const;
		CString SerializeToString (EFormat iFormat) const;
		void SetElement (IInvokeCtx *pCtx, const CString &sKey, CDatum dValue);
		void SetElement (const CString &sKey, CDatum dValue);
		void SetElement (int iIndex, CDatum dValue);
		void WriteBinaryToStream (IByteStream &Stream, int iPos = 0, int iLength = -1, IProgressEvents *pProgress = NULL) const;

		static int Compare (CDatum dValue1, CDatum dValue2) { return DefaultCompare(NULL, dValue1, dValue2); }

		//	Math related methods
		bool FitsAsDWORDLONG (void) const { Types iType = GetNumberType(NULL); return (iType == typeInteger32 || iType == typeInteger64); }
		Types GetNumberType (int *retiValue, CDatum *retdConverted = NULL) const;
		bool IsNumber (void) const;

		//	Function related methods
		bool CanInvoke (void) const;
		ECallType GetCallInfo (CDatum *retdCodeBank = NULL, DWORD **retpIP = NULL) const;
		InvokeResult Invoke (IInvokeCtx *pCtx, CDatum dLocalEnv, DWORD dwExecutionRights, CDatum *retdResult);
		InvokeResult InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, CDatum *retdResult);

		//	Utilities
		void AsAttributeList (CAttributeList *retAttribs) const;
		void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL);

		//	Implementation details
		static bool FindExternalType (const CString &sTypename, IComplexFactory **retpFactory);
		static void MarkAndSweep (void);
		static bool RegisterExternalType (const CString &sTypename, IComplexFactory *pFactory);
		static void RegisterMarkProc (MARKPROC fnProc);

	private:
		static constexpr DWORD_PTR CONST_TRUE =		0xaaaa0001;
		static constexpr DWORD_PTR CONST_FREE =		0xfeee0001;

		size_t CalcSerializeSizeAEONScript (EFormat iFormat) const;
		static int DefaultCompare (void *pCtx, const CDatum &dKey1, const CDatum &dKey2);
		static bool DeserializeAEONScript (IByteStream &Stream, IAEONParseExtension *pExtension, CDatum *retDatum);
		static bool DeserializeAEONScript (IByteStream &Stream, CDatum *retDatum) { return DeserializeAEONScript(Stream, NULL, retDatum); }
		static bool DeserializeJSON (IByteStream &Stream, CDatum *retDatum);
		static bool DeserializeTextUTF8 (IByteStream &Stream, CDatum *retDatum);
		static bool DetectFileFormat (const CString &sFilespec, IMemoryBlock &Data, EFormat *retiFormat, CString *retsError);
		DWORD GetNumberIndex (void) const { return (DWORD)(m_dwData >> 4); }
		bool IsAllocatedInteger (void) const { return (m_dwData & AEON_NUMBER_TYPE_MASK) == AEON_NUMBER_32BIT; }
		IComplexDatum *raw_GetComplex (void) const { return (IComplexDatum *)(m_dwData & AEON_POINTER_MASK); }
		double raw_GetDouble (void) const;
		int raw_GetInt32 (void) const;
		const CString &raw_GetString (void) const { ASSERT(AEON_TYPE_STRING == 0x00); return *(CString *)&m_dwData; }
		void SerializeAEONScript (EFormat iFormat, IByteStream &Stream) const;
		void SerializeJSON (IByteStream &Stream) const;

		DWORD_PTR m_dwData;
	};

inline int KeyCompare (const CDatum &dKey1, const CDatum &dKey2) { return CDatum::Compare(dKey1, dKey2); }

//	IComplexDatum

class IComplexDatum
	{
	public:
		static constexpr DWORD FLAG_SERIALIZE_AS_STRUCT =	0x00000001;
		static constexpr DWORD FLAG_SERIALIZE_NO_TYPENAME =	0x00000002;

		IComplexDatum (void) : m_bMarked(false) { }
		virtual ~IComplexDatum (void) { }

		virtual void Append (CDatum dDatum) { }
		virtual CString AsString (void) const { return NULL_STR; }
		virtual size_t CalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const;
		virtual size_t CalcMemorySize (void) const = 0;
		virtual bool CanInvoke (void) const { return false; }
		virtual const CDateTime &CastCDateTime (void) const { return NULL_DATETIME; }
		virtual const CIPInteger &CastCIPInteger (void) const { return NULL_IPINTEGER; }
		virtual const CRGBA32Image &CastCRGBA32Image (void) const { return CRGBA32Image::Null(); }
		virtual const CString &CastCString (void) const { return NULL_STR; }
		virtual DWORDLONG CastDWORDLONG (void) const { return 0; }
		virtual int CastInteger32 (void) const { return 0; }
		void ClearMark (void) { m_bMarked = false; }
		virtual IComplexDatum *Clone (void) const = 0;
		bool DeserializeAEONScript (CDatum::EFormat iFormat, const CString &sTypename, CCharStream *pStream);
		virtual bool DeserializeJSON (const CString &sTypename, const TArray<CDatum> &Data);
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const { return false; }
		virtual bool FindElement (const CString &sKey, CDatum *retpValue) { return false; }
		virtual CDatum::Types GetBasicType (void) const = 0;
		virtual int GetBinarySize (void) const { return CastCString().GetLength(); }
		virtual CDatum::ECallType GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const { return CDatum::ECallType::None; }
		virtual int GetCount (void) const = 0;
		virtual CDatum GetElement (IInvokeCtx *pCtx, int iIndex) const { return GetElement(iIndex); }
		virtual CDatum GetElement (int iIndex) const = 0;
		virtual CDatum GetElement (IInvokeCtx *pCtx, const CString &sKey) const { return GetElement(sKey); }
		virtual CDatum GetElement (const CString &sKey) const { return CDatum(); }
		virtual CString GetKey (int iIndex) const { return NULL_STR; }
		virtual CDatum::Types GetNumberType (int *retiValue) { return CDatum::typeUnknown; }
		virtual const CString &GetTypename (void) const = 0;
		virtual void GrowToFit (int iCount) { }
		virtual CDatum::InvokeResult Invoke (IInvokeCtx *pCtx, CDatum dLocalEnv, DWORD dwExecutionRights, CDatum *retdResult) { *retdResult = CDatum(); return CDatum::InvokeResult::ok; }
		virtual CDatum::InvokeResult InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, CDatum *retdResult) { *retdResult = CDatum(); return CDatum::InvokeResult::ok; }
		virtual bool IsArray (void) const = 0;
		virtual bool IsError (void) const { return false; }
		virtual bool IsIPInteger (void) const { return false; }
		bool IsMarked (void) const { return m_bMarked; }
		virtual bool IsMemoryBlock (void) const { const CString &sData = CastCString(); return (sData.GetLength() > 0); }
		virtual bool IsNil (void) const { return false; }
		void Mark (void) { if (!m_bMarked) { m_bMarked = true; OnMarked(); } }	//	Check m_bMarked to avoid infinite recursion
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const;
		virtual void SetElement (IInvokeCtx *pCtx, const CString &sKey, CDatum dDatum) { SetElement(sKey, dDatum); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) { }
		virtual void SetElement (int iIndex, CDatum dDatum) { }
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) { }
		virtual void WriteBinaryToStream (IByteStream &Stream, int iPos = 0, int iLength = -1, IProgressEvents *pProgress = NULL) const;

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const { return 0; }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) { ASSERT(false); return false; }
		virtual DWORD OnGetSerializeFlags (void) const { return 0; }
		virtual void OnMarked (void) { }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const { ASSERT(false); }
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const;

	private:
		bool m_bMarked;
	};

class IComplexFactory
	{
	public:
		virtual ~IComplexFactory (void) { }

		virtual IComplexDatum *Create (void) = 0;
	};

//	CComplexArray

class CComplexArray : public IComplexDatum
	{
	public:
		CComplexArray (void) { }
		CComplexArray (CDatum dSrc);
		CComplexArray (const TArray<CString> &Src);
		CComplexArray (const TArray<CDatum> &Src);

		void Delete (int iIndex) { m_Array.Delete(iIndex); }
		bool FindElement (CDatum dValue, int *retiIndex = NULL) const;
		void Insert (CDatum Element, int iIndex = -1) { m_Array.Insert(Element, iIndex); }
		void InsertEmpty (int iCount = 1, int iIndex = -1) { m_Array.InsertEmpty(iCount, iIndex); }

		//	IComplexDatum
		virtual void Append (CDatum dDatum) override { m_Array.Insert(dDatum); }
		virtual CString AsString (void) const override;
		virtual size_t CalcMemorySize (void) const override;
		virtual IComplexDatum *Clone (void) const override { return new CComplexArray(m_Array); }
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const override { return FindElement(dValue, retiIndex); }
		virtual int GetCount (void) const override { return m_Array.GetCount(); }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeArray; }
		virtual CDatum GetElement (int iIndex) const override { return ((iIndex >= 0 && iIndex < m_Array.GetCount()) ? m_Array[iIndex] : CDatum()); }
		virtual const CString &GetTypename (void) const override;
		virtual void GrowToFit (int iCount) override { m_Array.GrowToFit(iCount); }
		virtual bool IsArray (void) const override { return true; }
		virtual bool IsNil (void) const override { return (GetCount() == 0); }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { if (pfCompare) m_Array.Sort(pCtx, pfCompare, Order); else m_Array.Sort(Order); }
		virtual void SetElement (int iIndex, CDatum dDatum) override { m_Array[iIndex] = dDatum; }

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const;
		virtual void OnMarked (void);

	private:
		TArray<CDatum> m_Array;
	};

class CComplexBinary : public IComplexDatum
	{
	public:
		CComplexBinary (void) : m_pData(NULL) { }
		CComplexBinary (IByteStream &Stream, int iLength);
		~CComplexBinary (void);

		int GetLength (void) const { return (m_pData ? ((CString *)&m_pData)->GetLength() : 0); }
		void TakeHandoff (CStringBuffer &Buffer);

		//	IComplexDatum
		virtual void Append (CDatum dDatum);
		virtual CString AsString (void) const;
		virtual size_t CalcMemorySize (void) const override { return CastCString().GetLength() + sizeof(DWORD) + 1; }
		virtual const CString &CastCString (void) const;
		virtual IComplexDatum *Clone (void) const override;
		virtual CDatum::Types GetBasicType (void) const { return CDatum::typeBinary; }
		virtual int GetBinarySize (void) const { return GetLength(); }
		virtual int GetCount (void) const { return 1; }
		virtual CDatum GetElement (int iIndex) const { return CDatum(); }
		virtual const CString &GetTypename (void) const;
		virtual bool IsArray (void) const { return false; }
		virtual bool IsNil (void) const { return (m_pData == NULL); }

	protected:
		//	IComplexDatum
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream);
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const;

	private:
		LPSTR GetBuffer (void) const { return (m_pData - sizeof(DWORD)); }

		LPSTR m_pData;						//	Points to data (previous DWORD is length)
	};

class CComplexBinaryFile : public IComplexDatum
	{
	public:
		CComplexBinaryFile (void) : m_dwLength(0) { }
		CComplexBinaryFile (IByteStream &Stream, int iLength);
		~CComplexBinaryFile (void);

		void Append (IMemoryBlock &Data);
		int GetLength (void) const { return m_dwLength; }

		//	IComplexDatum
		virtual void Append (CDatum dDatum);
		virtual size_t CalcMemorySize (void) const override { return sizeof(CComplexBinaryFile); }
		virtual const CString &CastCString (void) const;
		virtual IComplexDatum *Clone (void) const override;
		virtual CDatum::Types GetBasicType (void) const { return CDatum::typeBinary; }
		virtual int GetBinarySize (void) const { return m_dwLength; }
		virtual int GetCount (void) const { return 1; }
		virtual CDatum GetElement (int iIndex) const { return CDatum(); }
		virtual const CString &GetTypename (void) const;
		virtual bool IsArray (void) const { return false; }
		virtual bool IsMemoryBlock (void) const { return false; }
		virtual bool IsNil (void) const { return (m_dwLength == 0); }
		virtual void WriteBinaryToStream (IByteStream &Stream, int iPos = 0, int iLength = -1, IProgressEvents *pProgress = NULL) const;

	protected:
		//	IComplexDatum
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream);
		virtual void OnMarked (void);
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const;

	private:
		struct SHeader
			{
			DWORD dwSignature;
			DWORD dwRefCount;
			};

		void CreateBinaryFile (CString *retsFilespec, CFile *retFile);
		bool DecrementRefCount (void);
		void IncrementRefCount (void) const;

		CString m_sFilespec;
		mutable CFile m_File;
		DWORD m_dwLength;
	};

class CComplexDateTime : public IComplexDatum
	{
	public:
		CComplexDateTime (void) { }
		CComplexDateTime (const CDateTime DateTime) : m_DateTime(DateTime) { }
		
		static bool CreateFromString (const CString &sString, CDateTime *retDateTime);
		static bool CreateFromString (const CString &sString, CDatum *retdDatum);

		virtual CString AsString (void) const;
		virtual size_t CalcMemorySize (void) const override { return sizeof(CComplexDateTime); }
		virtual const CDateTime &CastCDateTime (void) const { return m_DateTime; }
		virtual IComplexDatum *Clone (void) const override { return new CComplexDateTime(m_DateTime); }
		virtual CDatum::Types GetBasicType (void) const { return CDatum::typeDateTime; }
		virtual int GetCount (void) const { return partCount; }
		virtual CDatum GetElement (int iIndex) const;
		virtual CDatum GetElement (const CString &sKey) const;
		virtual const CString &GetTypename (void) const;
		virtual bool IsArray (void) const { return true; }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const;

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const;

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
		CComplexImage32 (void) { }
		CComplexImage32 (const CRGBA32Image &Src) :
				m_Image(Src)
			{ }

		CComplexImage32 (CRGBA32Image &&Src) :
				m_Image(std::move(Src))
			{ }

		//	IComplexDatum
		virtual CString AsString (void) const override { return strPattern("Image %dx%d", m_Image.GetWidth(), m_Image.GetHeight()); }
		virtual size_t CalcMemorySize (void) const override { return (size_t)m_Image.GetWidth() * (size_t)m_Image.GetHeight() * sizeof(DWORD); }
		virtual const CRGBA32Image &CastCRGBA32Image (void) const override { return m_Image; }
		virtual IComplexDatum *Clone (void) const override;
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeImage32; }
		virtual int GetCount (void) const override { return 1; }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual const CString &GetTypename (void) const override;
		virtual bool IsArray (void) const override { return false; }
		virtual bool IsNil (void) const override { return m_Image.IsEmpty(); }

	protected:
		//	IComplexDatum
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:
		CRGBA32Image m_Image;
	};

class CComplexInteger : public IComplexDatum
	{
	public:
		CComplexInteger (void) { }
		CComplexInteger (DWORDLONG ilValue) : m_Value(ilValue) { }
		CComplexInteger (const CIPInteger &Value) : m_Value(Value) { }

		void TakeHandoff (CIPInteger &Value) { m_Value.TakeHandoff(Value); }

		//	IComplexDatum
		virtual CString AsString (void) const { return m_Value.AsString(); }
		virtual size_t CalcMemorySize (void) const override { return m_Value.GetSize(); }
		virtual const CIPInteger &CastCIPInteger (void) const { return m_Value; }
		virtual DWORDLONG CastDWORDLONG (void) const;
		virtual int CastInteger32 (void) const;
		virtual IComplexDatum *Clone (void) const override { return new CComplexInteger(m_Value); }
		virtual CDatum::Types GetBasicType (void) const { return CDatum::typeIntegerIP; }
		virtual int GetCount (void) const { return 1; }
		virtual CDatum GetElement (int iIndex) const { return CDatum(); }
		virtual CDatum::Types GetNumberType (int *retiValue);
		virtual const CString &GetTypename (void) const;
		virtual bool IsArray (void) const { return false; }
		virtual bool IsIPInteger (void) const { return true; }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const;

	protected:
		//	IComplexDatum
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) { return CIPInteger::Deserialize(Stream, &m_Value); }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const { m_Value.Serialize(Stream); }

	private:
		CIPInteger m_Value;
	};

class CComplexStruct : public IComplexDatum
	{
	public:
		CComplexStruct (void) { }
		CComplexStruct (CDatum dSrc);
		CComplexStruct (const TSortMap<CString, CString> &Src);
		CComplexStruct (const TSortMap<CString, CDatum> &Src);

		void DeleteElement (const CString &sKey) { m_Map.DeleteAt(sKey); }

		//	IComplexDatum
		virtual void Append (CDatum dDatum) { AppendStruct(dDatum); }
		virtual CString AsString (void) const;
		virtual size_t CalcMemorySize (void) const override;
		virtual IComplexDatum *Clone (void) const override { return new CComplexStruct(m_Map); }
		virtual bool FindElement (const CString &sKey, CDatum *retpValue);
		virtual CDatum::Types GetBasicType (void) const { return CDatum::typeStruct; }
		virtual int GetCount (void) const { return m_Map.GetCount(); }
		virtual CDatum GetElement (int iIndex) const { return ((iIndex >= 0 && iIndex < m_Map.GetCount()) ? m_Map[iIndex] : CDatum()); }
		virtual CDatum GetElement (const CString &sKey) const { CDatum *pValue = m_Map.GetAt(sKey); return (pValue ? *pValue : CDatum()); }
		virtual CString GetKey (int iIndex) const { return m_Map.GetKey(iIndex); }
		virtual const CString &GetTypename (void) const;
		virtual void GrowToFit (int iCount) override { m_Map.GrowToFit(iCount); }
		virtual bool IsArray (void) const { return true; }
		virtual bool IsNil (void) const { return (GetCount() == 0); }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const;
		virtual void SetElement (const CString &sKey, CDatum dDatum) { m_Map.SetAt(sKey, dDatum); }

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const;
		virtual void OnMarked (void);

	private:
		void AppendStruct (CDatum dDatum);

		TSortMap<CString, CDatum> m_Map;
	};

template <class VALUE> class TExternalDatum : public IComplexDatum
	{
	public:
		TExternalDatum (void)
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

		static void RegisterFactory (void)
			{
			CDatum::RegisterExternalType(VALUE::StaticGetTypename(), new CFactory);
			}

		//	IComplexDatum
		virtual CString AsString (void) const override { return strPattern("[%s]", GetTypename()); }
		virtual size_t CalcMemorySize (void) const override { return sizeof(VALUE); }
		virtual IComplexDatum *Clone (void) const override { ASSERT(false); return NULL; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeCustom; }
		virtual int GetCount (void) const override { return 1; }
		virtual CDatum GetElement (IInvokeCtx *pCtx, int iIndex) const override { return GetElement(iIndex); }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual CDatum GetElement (IInvokeCtx *pCtx, const CString &sKey) const override { return GetElement(sKey); }
		virtual CDatum GetElement (const CString &sKey) const override { return CDatum(); }
		virtual CString GetKey (int iIndex) const override { return NULL_STR; }
		virtual const CString &GetTypename (void) const override { return VALUE::StaticGetTypename(); }
		virtual bool IsArray (void) const override { return false; }

	protected:

	private:
		class CFactory : public IComplexFactory
			{
			public:
				CFactory (void)
					{ }

				//	IComplexFactory
				virtual IComplexDatum *Create (void) { return new VALUE; }
			};
	};

//	Numbers --------------------------------------------------------------------

class CNumberValue
	{
	public:
		CNumberValue (CDatum dValue);

		void Add (CDatum dValue);
		int Compare (CDatum dValue) const {  CNumberValue Src(dValue); return Compare(Src); }
		int Compare (const CNumberValue &Value) const;
		void ConvertToDouble (void);
		void ConvertToIPInteger (void);
		bool Divide (CDatum dValue);
		bool DivideReversed (CDatum dValue);
		CDatum GetDatum (void);
		double GetDouble (void) const { return m_rValue; }
		int GetInteger (void) const { return (int)(DWORD_PTR)m_pValue; }
		DWORDLONG GetInteger64 (void) const { return m_ilValue; }
		const CIPInteger &GetIPInteger (void) const { return *(CIPInteger *)m_pValue; }
		bool IsValidNumber (void) const { return !m_bNotANumber; }
		bool IsNegative () const;
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
		void Subtract (CDatum dValue);
		void Upconvert (CNumberValue &Src);

	private:
		CDatum m_dOriginalValue;
		CDatum::Types m_iType;
		bool m_bUpconverted;
		bool m_bNotANumber;

		void *m_pValue;
		double m_rValue;
		DWORDLONG m_ilValue;
		CIPInteger m_ipValue;
	};

//	IInvokeCtx -----------------------------------------------------------------

class IInvokeCtx
	{
	public:
		virtual ~IInvokeCtx (void) { }

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
		bool ParseComment (void);
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
