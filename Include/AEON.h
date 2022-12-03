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

class CAEONQuery;
class CAEONTypeSystem;
class CComplexStruct;
class CNumberValue;
class IAEONCanvas;
class IAEONParseExtension;
class IAEONTable;
class IAEONTextLines;
class IComplexDatum;
class IComplexFactory;
class IDatatype;
class IInvokeCtx;
struct SAEONLibraryFunctionCreate;

//	Data encoding constants

constexpr DWORD_PTR AEON_TYPE_STRING =			0x00;
constexpr DWORD_PTR AEON_TYPE_NUMBER =			0x01;
constexpr DWORD_PTR AEON_TYPE_VOID =			0x02;
constexpr DWORD_PTR AEON_TYPE_COMPLEX =			0x03;

constexpr DWORD_PTR AEON_TYPE_MASK =			0x00000003;
constexpr DWORD_PTR AEON_POINTER_MASK =			~AEON_TYPE_MASK;
constexpr DWORD_PTR AEON_MARK_MASK =			0x00000001;

constexpr DWORD_PTR AEON_NUMBER_TYPE_MASK =		0x0000000F;
constexpr DWORD_PTR AEON_NUMBER_CONSTANT =		0x01;
constexpr DWORD_PTR AEON_NUMBER_INTEGER =		0x05;
constexpr DWORD_PTR AEON_NUMBER_ENUM =			0x09;
constexpr DWORD_PTR AEON_NUMBER_DOUBLE =		0x0D;
constexpr DWORD_PTR AEON_NUMBER_MASK =			0xFFFFFFF0;

constexpr DWORD_PTR AEON_MIN_28BIT =			0xF8000000;
constexpr DWORD_PTR AEON_MAX_28BIT =			0x07FFFFFF;

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
//	NaN												    0001 0010 0000 0010 0000 0000 0000 0001		//	0x12020001
//	True:		                                        1010 1010 1010 1010 0000 0000 0000 0001		//	0xAAAA0001
//	Free:		                                        1111 1110 1110 1110 0000 0000 0000 0001		//	0xFEEE0001
// 
//	Symbols:	[ 32-bit ID                           ] 0000 0000 0000 0000 0000 0000 0001 0001		//	Not Yet Implemented
//
//	Integer:	[ 32-bit integer                      ] 0000 0000 0000 0000 0000 0000 0000 0101
//	Enum:	    [ 32-bit integer                      ] [ 28-bit Datatype ID             ] 1001
//	Double:		                                        [ Index to double                ] 1101
//
//	Unused:		[                                                                           ]10
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
			typeVector2D =		19,
			typeQuery =			20,
			typeEnum =			21,
			typeLibraryFunc =	22,
			typeError =			23,
			typeTextLines =		24,
			typeAnnotated =		25,

			typeCustom =		100,
			};

		enum class ECallType
			{
			None =				0,			//	Not a function
			Call =				1,			//	A standard function call
			Library =			2,			//	A library function implemented in native code

			Invoke =			3,			//	Hexarc invoke message
			};

		enum class EClone
			{
			ShallowCopy,					//	Shallow copy of containers
			DeepCopy,						//	Deep copy of containers
			CopyOnWrite,					//	Shallow copy and then copy-on-write
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

		struct SAnnotation
			{
			DWORD fSpread:1 = false;
			DWORD dwSpare:31 = 0;
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
		CDatum (const CVector2D& Value);
		explicit CDatum (Types iType);
		CDatum (Types iType, const CString& sValue);

		//	NOTE: this probably should be explicit. We removed explicit because
		//	otherwise bools got converted to ints. But maybe ints should also be
		//	explicit?

		CDatum (bool bValue) : m_dwData(bValue ? CONST_TRUE : 0) { }

		//	Delete this because otherwise it turns into a bool.

		CDatum (const void *pValue) = delete;

		static CDatum CreateAnnotated (CDatum dValue, const SAnnotation& Annotation);
		static CDatum CreateArrayAsType (CDatum dType, CDatum dValue = CDatum());
		static CDatum CreateAsType (CDatum dType, CDatum dValue = CDatum());
		static bool CreateBinary (IByteStream &Stream, int iSize, CDatum *retDatum);
		static bool CreateBinaryFromHandoff (CStringBuffer &Buffer, CDatum *retDatum);
		static CDatum CreateEnum (int iValue, DWORD dwTypeID);
		static CDatum CreateEnum (int iValue, CDatum dType);
		static CDatum CreateError (const CString& sErrorDesc, const CString& sErrorCode = NULL_STR);
		static bool CreateFromAttributeList (const CAttributeList &Attribs, CDatum *retdDatum);
		static bool CreateFromFile (const CString &sFilespec, EFormat iFormat, CDatum *retdDatum, CString *retsError);
		static bool CreateFromStringValue (const CString &sValue, CDatum *retdDatum);
		static bool CreateIPInteger (const CIPInteger &Value, CDatum *retdDatum);
		static bool CreateIPIntegerFromHandoff (CIPInteger &Value, CDatum *retdDatum);
		static CDatum CreateLibraryFunction (const SAEONLibraryFunctionCreate& Create);
		static CDatum CreateNaN ();
		static CDatum CreateObject (CDatum dType, CDatum dValue = CDatum());
		static bool CreateStringFromHandoff (CString &sString, CDatum *retDatum);
		static bool CreateStringFromHandoff (CStringBuffer &String, CDatum *retDatum);
		static CDatum CreateTable (CDatum dType, CDatum dValue = CDatum());
		static bool CreateTableFromDesc (CAEONTypeSystem &TypeSystem, CDatum dDesc, CDatum &retdDatum);
		static CDatum CreateTextLines (CDatum dValue = CDatum());
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
		operator const CVector2D & () const;
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
		CDatum Clone (EClone iMode = EClone::ShallowCopy) const;
		bool Contains (CDatum dValue, TArray<IComplexDatum *> &retChecked) const;
		void DeleteElement (int iIndex);
		bool EnumElements (std::function<bool(CDatum)> fn);
		bool Find (CDatum dValue, int *retiIndex = NULL) const;
		bool FindElement (const CString &sKey, CDatum *retpValue);
		const SAnnotation& GetAnnotation () const;
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
		inline CDatum GetElementOrDefault (const CString &sKey, CDatum dDefault) const;
		CString GetKey (int iIndex) const;
		CDatum GetMethod (const CString &sMethod) const;
		const CString &GetTypename () const;
		void GrowToFit (int iCount);
		bool IsArray () const;
		bool IsContainer () const;
		bool IsMemoryBlock () const;
		bool IsEqual (CDatum dValue) const;
		bool IsError () const;
		bool IsIdenticalTo (CDatum dValue) const { return (m_dwData == dValue.m_dwData); }
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

		//	Special Interfaces

		IAEONCanvas *GetCanvasInterface ();
		const IAEONCanvas *GetCanvasInterface () const { return const_cast<CDatum *>(this)->GetCanvasInterface(); }
		CRGBA32Image *GetImageInterface ();
		CRGBA32Image &GetImageInterfaceOrThrow () { auto pValue = GetImageInterface(); if (!pValue) throw CException(errFail); else return *pValue; }
		const CAEONQuery* GetQueryInterface () const;
		IAEONTable *GetTableInterface ();
		const IAEONTable *GetTableInterface () const { return const_cast<CDatum *>(this)->GetTableInterface(); }
		IAEONTextLines* GetTextLinesInterface ();
		const IAEONTextLines* GetTextLinesInterface () const { return const_cast<CDatum *>(this)->GetTextLinesInterface(); }

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
		DWORD GetNumberIndex () const { return ((DWORD)m_dwData) >> 4; }
		double raw_GetDouble () const;
		const CString &raw_GetString () const { ASSERT(AEON_TYPE_STRING == 0x00); return *(CString *)&m_dwData; }
		void SerializeAEONScript (EFormat iFormat, IByteStream &Stream) const;
		void SerializeEnum (EFormat iFormat, IByteStream &Stream) const;
		void SerializeJSON (IByteStream &Stream) const;

		template<class FUNC> CDatum MathArrayOp () const;

		DWORD_PTR m_dwData;

		static SAnnotation m_NullAnnotation;
	};

inline int KeyCompare (const CDatum &dKey1, const CDatum &dKey2) { return CDatum::Compare(dKey1, dKey2); }

struct SAEONLibraryFunctionCreate
	{
	CString sName;
	std::function<bool(IInvokeCtx&, DWORD, CDatum, CDatum, CDatum&)> fnInvoke;
	DWORD dwData = 0;
	DWORD dwExecutionRights = 0;
	CDatum dType;
	};

#include "AEONTypeSystem.h"
#include "AEONInterfaces.h"
#include "AEONUtil.h"

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
		virtual const CVector2D& CastCVector2D () const { return CVector2D::Null; }
		virtual double CastDouble () const { return CDatum::CreateNaN(); }
		virtual DWORDLONG CastDWORDLONG () const { return 0; }
		virtual const IDatatype &CastIDatatype () const;
		virtual int CastInteger32 () const { return 0; }
		void ClearMark () { m_bMarked = false; }
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const { return NULL; }
		virtual bool Contains (CDatum dValue, TArray<IComplexDatum *> &retChecked) const { return false; }
		virtual void DeleteElement (int iIndex) { }
		bool DeserializeAEONScript (CDatum::EFormat iFormat, const CString &sTypename, CCharStream *pStream);
		virtual bool DeserializeJSON (const CString &sTypename, const TArray<CDatum> &Data);
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const { return false; }
		virtual bool FindElement (const CString &sKey, CDatum *retpValue) { return false; }
		virtual const CDatum::SAnnotation& GetAnnotation () const { return CDatum().GetAnnotation(); }
		virtual CDatum::Types GetBasicType () const = 0;
		virtual int GetBinarySize () const { return CastCString().GetLength(); }
		virtual CDatum::ECallType GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const { return CDatum::ECallType::None; }
		virtual IAEONCanvas *GetCanvasInterface () { return NULL; }
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
		virtual const CAEONQuery* GetQueryInterface () const { return NULL; }
		virtual IAEONTable *GetTableInterface () { return NULL; }
		virtual IAEONTextLines *GetTextLinesInterface () { return NULL; }
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

//	Implementation -------------------------------------------------------------
//
//	LATER: These should move to internal headers. Callers shouldn't have to
//	deal with the implementation classes.

//	CComplexArray

class CComplexArray : public IComplexDatum
	{
	public:
		CComplexArray () { }
		CComplexArray (CDatum dSrc);
		CComplexArray (const TArray<CString> &Src);
		CComplexArray (const TArray<CDatum> &Src);
		explicit CComplexArray (int iCount) { m_Array.InsertEmpty(iCount); }

		void Delete (int iIndex) { OnCopyOnWrite(); m_Array.Delete(iIndex); }
		bool FindElement (CDatum dValue, int *retiIndex = NULL) const;
		void Insert (CDatum Element, int iIndex = -1) { OnCopyOnWrite(); m_Array.Insert(Element, iIndex); }
		void InsertEmpty (int iCount = 1, int iIndex = -1) { OnCopyOnWrite(); m_Array.InsertEmpty(iCount, iIndex); }
		void SetAt (int iIndex, CDatum dValue) { OnCopyOnWrite(); m_Array[iIndex] = dValue; }

		//	IComplexDatum
		virtual void Append (CDatum dDatum) override { OnCopyOnWrite(); m_Array.Insert(dDatum); }
		virtual CString AsString () const override;
		virtual size_t CalcMemorySize () const override;
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual bool Contains (CDatum dValue, TArray<IComplexDatum *> &retChecked) const override;
		virtual void DeleteElement (int iIndex) override;
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const override { return FindElement(dValue, retiIndex); }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeArray; }
		virtual int GetCount () const override { return m_Array.GetCount(); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY); }
		virtual CDatum GetElement (int iIndex) const override { return ((iIndex >= 0 && iIndex < m_Array.GetCount()) ? m_Array[iIndex] : CDatum()); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override;
		virtual const CString &GetTypename () const override;
		virtual void GrowToFit (int iCount) override { OnCopyOnWrite(); m_Array.GrowToFit(iCount); }
		virtual bool IsArray () const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil () const override { return (GetCount() == 0); }
		virtual CDatum MathAbs () const override;
		virtual CDatum MathMax () const override;
		virtual CDatum MathMin () const override;
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { OnCopyOnWrite(); if (pfCompare) m_Array.Sort(pCtx, pfCompare, Order); else m_Array.Sort(Order); }
		virtual void SetElement (int iIndex, CDatum dDatum) override { OnCopyOnWrite(); if (iIndex >= 0 && iIndex < m_Array.GetCount()) m_Array[iIndex] = dDatum; }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { OnCopyOnWrite(); m_Properties.SetProperty(*this, sKey, dDatum, NULL); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override;

	protected:
		void CloneContents ();
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual void OnMarked () override;
		void OnCopyOnWrite ();

		bool m_bCopyOnWrite = false;
		TArray<CDatum> m_Array;

		static TDatumPropertyHandler<CComplexArray> m_Properties;
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
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
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
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
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

class CComplexStruct : public IComplexDatum
	{
	public:
		CComplexStruct () { }
		CComplexStruct (CDatum dSrc);
		CComplexStruct (const TSortMap<CString, CString> &Src);
		CComplexStruct (const TSortMap<CString, CDatum> &Src);

		void DeleteElement (const CString &sKey) { OnCopyOnWrite(); m_Map.DeleteAt(sKey); }

		//	IComplexDatum
		virtual void Append (CDatum dDatum) override { AppendStruct(dDatum); }
		virtual CString AsString () const override;
		virtual size_t CalcMemorySize () const override;
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
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
		virtual void GrowToFit (int iCount) override { OnCopyOnWrite(); m_Map.GrowToFit(iCount); }
		virtual bool IsArray () const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil () const override { return (GetCount() == 0); }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override { SerializeAsStruct(iFormat, Stream); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { OnCopyOnWrite(); m_Map.SetAt(sKey, dDatum); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override;

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override { return CalcSerializeAsStructSize(iFormat); }
		virtual void OnMarked () override;

		void AppendStruct (CDatum dDatum);
		void CloneContents ();
		void OnCopyOnWrite ();

		bool m_bCopyOnWrite = false;
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

		static CDatum Divide (const CIPInteger &Dividend, const CIPInteger &Divisor);

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
#include "AEONQuery.h"
#include "AEONVector.h"

inline CDatum CDatum::GetElementOrDefault (const CString &sKey, CDatum dDefault) const
	{
	CDatum dResult = GetElement(sKey);
	if (dResult.IsNil())
		return dDefault;
	else
		return dResult;
	}
