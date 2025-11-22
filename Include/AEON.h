//	AEON.h
//
//	Archon Engine Object Notation
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.
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

#if defined(DEBUG_PERF)
#define DEBUG_BLOB_PERF
#elif defined DEBUG
//#define DEBUG_BLOB_PERF
//#define DEBUG_MODIFIED_ON
//#define DEBUG_GC_STATS
#endif

#include <array>
#include <unordered_map>

class CAEONExpression;
class CAEONSerializedMap;
class CAEONTypeSystem;
class CComplexStruct;
class CDatumArrayWrapper;
class CHexeLocalEnvironment;
class CHexeStackEnv;
class CNumberValue;
class IAEONCanvas;
class IAEONParseExtension;
class IAEONRange;
class IAEONReanimator;
class IAEONTable;
class IAEONTextLines;
class IComplexDatum;
class IComplexFactory;
class IDatatype;
class IInvokeCtx;
struct SAEONLibraryFunctionCreate;
struct SAEONInvokeResult;

template <class OBJ> class TDatumMethodHandler;

typedef void (*MARKPROC)(void);

//	CDatum
//
//	These values can be passed around at will, but we must be able to call Mark
//	on all values that need to be preserved across garbage-collection runs.
//	
//	A CDatum is a 64-bit atom that encodes different types. We use NaN boxing to
//	encode the type in the top 16-bits. In IEEE 754, NaN values have the top bit
//	as a sign bit and the next 11 bits (exponent bits) set to 1. The remaining 
//	bits must have a 1 (otherwise it is interpreted as infinity).
//
//	We use the following patterns for the top 16-bits:
// 
//	7FF0		+Infinity (lower bits must be 0).
//	7FF1		Constants. The lower bits are:
//
//				0000	false
//				0001	true
// 
//	7FF2		32-bit integer (lower 32-bits are the integer)
//	7FF3		Enum value:
//
//				xxxx xxxx ----		32-bits of Datatype ID
//				---- ---- xxxx		16-bits of enum value
//
//	7FF4		Symbol (not yet implemented)
//
//	7FF5-7FF7	Reserved for future use
// 
//	7FF8		nan (lower bits must be 0)
//
//	7FF9-7FFF	Reserved for future use
// 
//	FFF0		-Infinity (lower bits must be 0)
//	FFF1		String (lower bits are a pointer to a string)
//	FFF2		Pointer (lower bits are a pointer to a complex object)
//	FFF3		Row reference:
// 
// 				xxxx ---- ----		16-bits of table ID
//				---- xxxx xxxx		32-bits of row index
//
//	FFF4-FFFE	Reserved for future use
// 
//	FFFF		null (lower bits must be 1)
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
			typeExpression =	20,
			typeEnum =			21,
			typeLibraryFunc =	22,
			typeError =			23,
			typeTextLines =		24,
			typeAnnotated =		25,
			typeForeign =		26,
			typeVector3D =		27,
			typeClassInstance =	28,
			typeTensor =		29,
			typeAEONObject =	30,
			typeRange =			31,
			typeFalse =			32,
			typeRowRef =		33,

			typeCount =			34,			//	Number of types

			typeCustom =		100,
			};

		enum class ECallType
			{
			None =				0,			//	Not a function
			Call =				1,			//	A standard function call
			Library =			2,			//	A library function implemented in native code

			Invoke =			3,			//	Hexarc invoke message
			CachedCall =		4,			//	A standard function call, cached
			};

		enum class EClone
			{
			ShallowCopy,					//	Shallow copy of containers
			DeepCopy,						//	Deep copy of containers
			CopyOnWrite,					//	Shallow copy and then copy-on-write
			Isolate,						//	Copy if we rely on a reference (e.g., a row reference obj).
			};

		enum class EFormat
			{
			Unknown =			-1,

			AEONScript =		0,
			JSON =				1,
			AEONLocal =			2,			//	Serialized to a local machine
			TextUTF8 =			3,			//	Plain text (unstructured)
			Binary =			4,			//	Binary
			AEONBinary =		5,			//	AEON binary format
			AEONBinaryLocal =	6,			//	AEON binary serialized to a local machine
			GridLang =			7,			//	GridLang literal
			};

		enum class InvokeResult
			{
			unknown,

			error,
			ok,
			functionCall,

			runFunction,
			runInputRequest,
			runInputRequestDebugSim,
			runInvoke,
			};

		enum class EPropertyResult
			{
			OK,
			NotFound,
			Error,
			};

		struct SAnnotation
			{
			DWORD fSpread:1 = false;
			DWORD dwSpare:31 = 0;
			};

		struct SStatsCtx
			{
			int iCount = 0;					//	Running count
			double rMin = std::numeric_limits<double>::infinity();	//	Minimum value
			double rMax = -std::numeric_limits<double>::infinity();	//	Maximum value
			double rMean = 0.0;				//	Mean value
			int iNullCount = 0;				//	Number of null values
			int iNaNCount = 0;				//	Number of NaN values

			double M2 = 0.0;				//	Running variance
			};

		enum class EMethodExt
			{
			None,
			Array,
			DateTime,
			Dictionary,
			Struct,
			Table,
			Tensor,
			};

		static constexpr DWORD FLAG_ALLOW_NULLS =				0x00000001;
		static constexpr DWORD FLAG_COUNT_ONLY =				0x00000002;
		static constexpr DWORD FLAG_EXACT =						0x00000004;
		static constexpr DWORD FLAG_RECURSIVE =					0x00000008;

		CDatum () : m_dwData(VALUE_NULL) { }
		CDatum (int iValue) : m_dwData(EncodeInt32(iValue)) { }
		CDatum (DWORD dwValue);
		CDatum (DWORDLONG ilValue);
		CDatum (const CString &sString);
		CDatum (CStringView Value) : CDatum((const CString&)Value) { }
		CDatum (CString &&sString);
		CDatum (CStringBuffer &&sString);
		CDatum (double rValue);
		explicit CDatum (IComplexDatum *pValue);
		CDatum (const CDateTime &DateTime);
		CDatum (const CIPInteger &Value);
		CDatum (const CRGBA32Image &Value);
		CDatum (CRGBA32Image &&Value);
		explicit CDatum (const CStringFormat& Value);
		explicit CDatum (CStringFormat&& Value);
		CDatum (const CTimeSpan &TimeSpan);
		CDatum (const CVector2D& Value);
		CDatum (const CVector3D& Value);
		explicit CDatum (Types iType);
		CDatum (Types iType, const CString& sValue);

		//	NOTE: this probably should be explicit. We removed explicit because
		//	otherwise bools got converted to ints. But maybe ints should also be
		//	explicit?

		CDatum (bool bValue) : m_dwData(EncodeBool(bValue)) { }

		//	Delete this because otherwise it turns into a bool.

		CDatum (const void *pValue) = delete;

		static CDatum CreateAnnotated (CDatum dValue, const SAnnotation& Annotation);
		static CDatum CreateArray (const TArray<CString>& Value);
		static CDatum CreateArrayAsType (CDatum dType, CDatum dValue = CDatum());
		static CDatum CreateArrayAsTypeOfElement (CDatum dElementType, CDatum dValue = CDatum(), CDatum dArrayType = CDatum());
		static CDatum CreateAsType (CDatum dType, CDatum dValue = CDatum(), bool bConstruct = false);
		static CDatum CreateBinary (CString&& sData);
		static CDatum CreateBinary (CBuffer64&& Buffer);
		static CDatum CreateBinary (CStringBuffer&& Buffer);
		static CDatum CreateBinary (int iSize);
		static bool CreateBinary (IByteStream &Stream, int iSize, CDatum *retDatum);
		static bool CreateBinaryFromHandoff (CStringBuffer &Buffer, CDatum *retDatum);
		static CDatum CreateDateTime (CDatum dValue, CDatum dOptions = CDatum());
		static CDatum CreateDictionary (CDatum dType, CDatum dValue = CDatum());
		static CDatum CreateEnum (int iValue, DWORD dwTypeID);
		static CDatum CreateEnum (int iValue, CDatum dType);
		static CDatum CreateError (CStringView sErrorDesc, CStringView sErrorCode = CStringView());
		static bool CreateFromAttributeList (const CAttributeList &Attribs, CDatum *retdDatum);
		static bool CreateFromFile (const CString &sFilespec, EFormat iFormat, CDatum *retdDatum, CString *retsError);
		static bool CreateFromStringValue (CStringView sValue, CDatum *retdDatum);
		static bool CreateIPInteger (const CIPInteger &Value, CDatum *retdDatum);
		static bool CreateIPIntegerFromHandoff (CIPInteger &Value, CDatum *retdDatum);
		static CDatum CreateLibraryFunction (const SAEONLibraryFunctionCreate& Create);
		static CDatum CreateNaN ();
		static CDatum CreateObject (CDatum dType, CDatum dValue = CDatum());
		static CDatum CreateObjectEmpty (CDatum dType);
		static CDatum CreateRange (CDatum dStart, CDatum dEnd, CDatum dStep);
		static CDatum CreateRange (int iStart, int iEnd, int iStep);
		static CDatum CreateRowRef (DWORD dwTableID, int iRowIndex);
		static CDatum CreateString (CDatum dValue, CDatum dFormat = CDatum());
		static CDatum CreateString (CStringBuffer&& Buffer);
		static bool CreateStringFromHandoff (CString &sString, CDatum *retDatum);
		static bool CreateStringFromHandoff (CStringBuffer &String, CDatum *retDatum);
		static CDatum CreateTable (CDatum dType, CDatum dValue = CDatum());
		static CDatum CreateTable (CDatum dType, TArray<CDatum>&& Cols);
		static bool CreateTableFromDesc (CAEONTypeSystem &TypeSystem, CDatum dDesc, CDatum &retdDatum);
		static CDatum CreateTensorAsType (CDatum dType, CDatum dValue = CDatum(), bool bConstruct = false);
		static CDatum CreateTextLines (CDatum dValue = CDatum());
		static CDatum Deserialize (IByteStream& Stream, EFormat iFormat, CAEONSerializedMap& Serialized);
		static CDatum DeserializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized);
		static bool Deserialize (EFormat iFormat, IByteStream &Stream, IAEONParseExtension *pExtension, CDatum *retDatum);
		static bool Deserialize (EFormat iFormat, IByteStream &Stream, CDatum *retDatum) { return Deserialize(iFormat, Stream, NULL, retDatum); }
		static bool DeserializeEnumFromAEONScript (CDatum dType, CDatum dValue, CDatum &retdDatum);
		static bool DeserializeEnumFromJSON (const CString& sLabel, const CString& sValue, CDatum &retdDatum);
		static void SetMethodsExt (EMethodExt iType, TDatumMethodHandler<IComplexDatum> &MethodsExt);
		static CDatum VectorOf (Types iType, CDatum dValues = CDatum());

		operator int () const;
		operator DWORD () const;
		operator DWORDLONG () const;
		operator double () const;
		operator const CIPInteger & () const;
		operator CStringView () const;
		operator const CDateTime & () const;
		operator const CRGBA32Image & () const;
		operator const CTimeSpan & () const;
		operator const CVector2D & () const;
		operator const CVector3D & () const;
		operator const IDatatype & () const;
		operator const CString& () const = delete;
		operator const CStringFormat& () const;

		bool operator== (const CDatum& other) const { return OpIsEqual(other); }
	    bool operator!= (const CDatum& other) const { return !(*this == other); }

		//	Standard interface

		void Append (CDatum dValue);
		void AppendArray (CDatum dArray);
		int AsArrayIndex (int iArrayLen, bool* retbFromEnd = NULL) const;
		bool AsBool () const { return !IsNil(); }
		CDateTime AsDateTime () const;
		const CAEONExpression& AsExpression () const;
		CDatum AsFloat (bool bNullIfInvalid = false) const;
		int AsInt32 () const;
		CDatum AsInteger () const;
		CIPInteger AsIPInteger () const;
		CDatum AsMapColumnExpression () const;
		CBuffer AsMemoryBlock () const { return CBuffer(GetBinaryData(), GetBinarySize(), false); }
		CBuffer64 AsMemoryBlock64 () const { return CBuffer64(GetBinaryData(), GetBinarySize64(), false); }
		CDatum AsNumber (bool bNullIfInvalid = false) const;
		CDatum AsOptions (bool *retbConverted = NULL) const;
		IByteStream& AsStream () const;
		CDatum AsStruct () const;
		CString AsString () const;
		CStringView AsStringView () const { return (CStringView)*this; }
		TArray<CString> AsStringArray () const;
		CTimeSpan AsTimeSpan () const;
		DWORD AsUInt32 () const;
		size_t CalcMemorySize () const;
		size_t CalcSerializeSize (EFormat iFormat) const;
		bool CanSum () const;
		CDatum Clone (EClone iMode = EClone::ShallowCopy) const;
		bool Contains (CDatum dValue) const;
		void DeleteElement (int iIndex);

		//	FLAG_ALLOW_NULLS
		//	FLAG_RECURSIVE
		bool EnumElements (DWORD dwFlags, std::function<bool(CDatum)> fn) const;

		bool Find (CDatum dValue, int *retiIndex = NULL) const;
		CDatum FindAll (CDatum dValue) const;
		CDatum FindAllExact (CDatum dValue) const;
		bool FindExact (CDatum dValue, int *retiIndex = NULL) const;
		bool FindElement (const CString &sKey, CDatum *retpValue = NULL);
		int FindMaxElement () const;
		int FindMinElement () const;
		CString Format (const CString& sFormat) const { return Format(CStringFormat(sFormat)); }
		CString Format (const CStringFormat& Format) const;
		const SAnnotation& GetAnnotation () const;
		int GetArrayCount () const;
		CDatum GetArrayElement (int iIndex) const;
		Types GetBasicType () const;
		void* GetBinaryData () const;
		int GetBinarySize () const;
		DWORDLONG GetBinarySize64 () const;
		IComplexDatum *GetComplex () const;
		int GetCount () const;
		inline DWORD GetBasicDatatype () const;
		CDatum GetDatatype () const;
		int GetDimensions () const;
		CDatum GetElement (IInvokeCtx *pCtx, int iIndex) const;
		CDatum GetElement (int iIndex) const;
		CDatum GetElement (IInvokeCtx *pCtx, const CString &sKey) const;
		CDatum GetElement (const CString &sKey) const;
		CDatum GetElementAt (int iIndex) const;
		CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const;
		CDatum GetElementAt2DA (CDatum dIndex1, CDatum dIndex2) const;
		CDatum GetElementAt3DA (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3) const;
		CDatum GetElementAt2DI (int iIndex1, int iIndex2) const;
		CDatum GetElementAt3DI (int iIndex1, int iIndex2, int iIndex3) const;
		int GetElementAtCount () const;
		inline CDatum GetElementOrDefault (const CString &sKey, CDatum dDefault) const;
		CDatum GetElementsAtArray (CDatum dIndex) const;
		CDatum GetElementsAtRange (CDatum dRange) const;
		CString GetKey (int iIndex) const;
		CDatum GetKeyEx (int iIndex) const;
		TArray<int> GetKeysInSortedOrder () const;
		CDatum GetMethod (const CString &sMethod) const;
		void* GetMethodThis ();
		CDatum GetProperty (const CString& sProperty) const;
		const CString &GetTypename () const;

		//	FLAG_ALLOW_NULLS
		//	FLAG_COUNT_ONLY
		//	FLAG_EXACT
		CDatum GetUniqueValues (DWORD dwFlags = 0) const;

		void GrowToFit (int iCount);
		size_t Hash () const;
		bool HasKeys () const;
		void InsertElementAt (CDatum dIndex, CDatum dValue);
		void InsertEmpty (int iCount);
		bool IsArray () const;
		bool IsAtom () const;
		bool IsContainer () const;
		bool IsMemoryBlock () const;
		bool IsEqualCompatible (CDatum dValue) const;
		bool IsError () const;
		bool IsIdenticalTo (CDatum dValue) const { return (m_dwData == dValue.m_dwData); }
		bool IsIdenticalToNaN () const { return (m_dwData == VALUE_NAN); }
		bool IsIdenticalToNil () const { return (m_dwData == VALUE_NULL); }
		bool IsIdenticalToTrue () const { return (m_dwData == VALUE_TRUE); }
		bool IsNaN () const { return (m_dwData == VALUE_NAN) || !std::isfinite((double)(*this)); }
		bool IsNil () const;
		bool IsStruct () const;
		CDatum IteratorBegin () const;
		CDatum IteratorGetKey (CDatum dIterator) const;
		CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const;
		CDatum IteratorNext (CDatum dIterator) const;

		//	FLAG_ALLOW_NULLS
		CDatum Join (CStringView sSeparator, CStringView sLastSeparator = CStringView(), DWORD dwFlags = 0) const;

		void Mark ();
		int OpCompare (CDatum dValue) const;
		int OpCompareExact (CDatum dValue) const;
		CDatum OpConcatenated (IInvokeCtx& Ctx, CDatum dSrc, int iAxis = 0) const;
		bool OpContains (CDatum dValue) const;
		bool OpIsEqual (CDatum dValue) const;
		bool OpIsIdentical (CDatum dValue) const;
		bool RemoveAll ();
		bool RemoveElementAt (CDatum dIndex);
		void ResolveDatatypes (const CAEONTypeSystem &TypeSystem);
		void Serialize (EFormat iFormat, IByteStream &Stream) const;
		void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const;
		CString SerializeToString (EFormat iFormat) const;
		void SetElement (IInvokeCtx *pCtx, const CString &sKey, CDatum dValue);
		void SetElement (const CString &sKey, CDatum dValue);
		void SetElement (int iIndex, CDatum dValue);
		void SetElementAt (CDatum dIndex, CDatum dValue);
		void SetElementAt2DA (CDatum dIndex1, CDatum dIndex2, CDatum dValue);
		void SetElementAt2DI (int iIndex1, int iIndex2, CDatum dValue);
		void SetElementAt3DA (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3, CDatum dValue);
		void SetElementAt3DI (int iIndex1, int iIndex2, int iIndex3, CDatum dValue);
		void SetElementsAtArray (CDatum dIndex, CDatum dValue);
		void SetElementsAtRange (CDatum dRange, CDatum dValue);
		void WriteBinaryToStream (IByteStream& Stream, int iPos = 0, int iLength = -1, IProgressEvents *pProgress = NULL) const;
		void WriteBinaryToStream (IByteStream64& Stream, DWORDLONG dwPos = 0, DWORDLONG dwLength = 0xffffffffffffffff, IProgressEvents *pProgress = NULL) const;

		//	Special Interfaces

		IAEONCanvas *GetCanvasInterface ();
		const IAEONCanvas *GetCanvasInterface () const { return const_cast<CDatum *>(this)->GetCanvasInterface(); }
		CRGBA32Image *GetImageInterface ();
		CRGBA32Image &GetImageInterfaceOrThrow () { auto pValue = GetImageInterface(); if (!pValue) throw CException(errFail); else return *pValue; }
		const CAEONExpression* GetQueryInterface () const;
		IAEONRange* GetRangeInterface ();
		const IAEONRange* GetRangeInterface () const { return const_cast<CDatum*>(this)->GetRangeInterface(); }
		IAEONReanimator *GetReanimatorInterface ();
		const IAEONReanimator *GetReanimatorInterface () const { return const_cast<CDatum *>(this)->GetReanimatorInterface(); }
		IAEONTable *GetTableInterface ();
		const IAEONTable *GetTableInterface () const { return const_cast<CDatum *>(this)->GetTableInterface(); }
		IAEONTextLines* GetTextLinesInterface ();
		const IAEONTextLines* GetTextLinesInterface () const { return const_cast<CDatum *>(this)->GetTextLinesInterface(); }

		static int Compare (CDatum dValue1, CDatum dValue2) { return dValue1.OpCompare(dValue2); }
		static int CompareExact (CDatum dValue1, CDatum dValue2) { return dValue1.OpCompareExact(dValue2); }
		static int DefaultCompare (void *pCtx, const CDatum &dKey1, const CDatum &dKey2) { return dKey1.OpCompare(dKey2); }

		//	Math related methods
		bool FitsAsDWORDLONG () const { Types iType = GetNumberType(NULL); return (iType == typeInteger32 || iType == typeInteger64); }
		Types GetNumberType (int *retiValue, CDatum *retdConverted = NULL) const;
		bool IsNumber () const;
		bool IsNumberInt32 (int *retiValue = NULL) const;
		CDatum MathAbs () const;
		void MathAccumulateStats (SStatsCtx& Stats) const;
		static void MathAccumulateStats (SStatsCtx& Stats, double rValue);
		CDatum MathAverage () const;
		CDatum MathCeil () const;
		CDatum MathFloor () const;
		CDatum MathMax () const;
		CDatum MathMedian () const;
		CDatum MathMin () const;
		CDatum MathRound () const;
		CDatum MathSign () const;
		CDatum MathStats () const;
		CDatum MathStdDev () const;
		CDatum MathStdError () const;
		CDatum MathSum () const;
		CDatum MathAddToElements (CDatum dValue) const;
		CDatum MathAddElementsTo (CDatum dValue) const;
		CDatum MathDivideElementsBy (CDatum dValue) const;
		CDatum MathDivideByElements (CDatum dValue) const;
		CDatum MathExpElementsTo (CDatum dValue) const;
		CDatum MathExpToElements (CDatum dValue) const;
		CDatum MathInvert () const;
		CDatum MathMatMul (CDatum dValue) const;
		CDatum MathModElementsBy (CDatum dValue) const;
		CDatum MathModByElements (CDatum dValue) const;
		CDatum MathMultiplyElements (CDatum dValue) const;
		CDatum MathNegateElements () const;
		CDatum MathSubtractFromElements (CDatum dValue) const;
		CDatum MathSubtractElementsFrom (CDatum dValue) const;

		//	Mutations
		void MutateAdd (int iInc = 1);
		void MutateAddInt32 (int iInc = 1) { m_dwData = EncodeInt32(DecodeInt32(m_dwData) + iInc); }

		//	Function related methods
		void CacheInvokeResult (CHexeLocalEnvironment& LocalEnv, CDatum dResult);
		bool CanInvoke () const;
		ECallType GetCallInfo (CDatum *retdCodeBank = NULL, DWORD **retpIP = NULL) const;
		InvokeResult Invoke (IInvokeCtx *pCtx, CHexeLocalEnvironment& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult);
		InvokeResult InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, SAEONInvokeResult& retResult);
		InvokeResult InvokeLibrary (IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult);
		bool InvokeMethodImpl (const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult);

		//	Utilities
		void AsAttributeList (CAttributeList *retAttribs) const;
		CDatum MergeKeysNoCase () const;
		void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL);

		//	Implementation details
		static int CalcArrayIndex (int iIndex, int iArrayLen, bool* retbFromEnd = NULL) { if (retbFromEnd) *retbFromEnd = (iIndex < 0); return (iIndex < 0 ? iArrayLen + iIndex : iIndex); }
		static bool FindExternalType (const CString &sTypename, IComplexFactory **retpFactory);
		static void MarkAndSweep ();
		static bool RegisterExternalType (const CString &sTypename, IComplexFactory *pFactory);
		static void RegisterMarkProc (MARKPROC fnProc);
		static CDatum raw_AsComplex (const void *pValue) { CDatum dResult; dResult.m_dwData = EncodePointer(pValue); return dResult; }
		DWORDLONG raw_AsEncoded () const { return m_dwData; }
		inline CDatum raw_GetArrayElement (int iIndex) const;
		IComplexDatum* raw_GetComplex () const { return (IComplexDatum *)DecodePointer(m_dwData); }
		inline double raw_GetDouble () const { return DecodeDouble(m_dwData); }
		int raw_GetInt32 () const { return DecodeInt32(m_dwData); }
		CDatum raw_IteratorGetElement (CBuffer& Iterator) const;
		CDatum raw_IteratorGetKey (CBuffer& Iterator) const;
		bool raw_IteratorHasMore (CBuffer& Iterator) const;
		void raw_IteratorNext (CBuffer& Iterator) const;
		void raw_IteratorSetElement (CBuffer& Iterator, CDatum dValue);
		CBuffer raw_IteratorStart () const;
		static CDatum raw_MakeDatum (DWORDLONG dwData) { CDatum dResult; dResult.m_dwData = dwData; return dResult; }
		inline void raw_SetArrayElement (int iIndex, CDatum dValue);
		static void WriteGridLangIdentifier (IByteStream& Stream, CStringView sString);
		static void WriteGridLangString (IByteStream& Stream, CStringView sString);

		static constexpr DWORD SERIALIZE_TYPE_NULL =				0x01000000;
		static constexpr DWORD SERIALIZE_TYPE_TRUE =				0x02000000;
		static constexpr DWORD SERIALIZE_TYPE_NAN =					0x03000000;
		static constexpr DWORD SERIALIZE_TYPE_STRING =				0x04000000;
		static constexpr DWORD SERIALIZE_TYPE_INT24 =				0x05000000;
		static constexpr DWORD SERIALIZE_TYPE_INT32 =				0x06000000;
		static constexpr DWORD SERIALIZE_TYPE_ENUM =				0x07000000;
		static constexpr DWORD SERIALIZE_TYPE_DOUBLE =				0x08000000;

		static constexpr DWORD SERIALIZE_TYPE_ARRAY =				0x09000000;
		static constexpr DWORD SERIALIZE_TYPE_INTIP =				0x0a000000;
		static constexpr DWORD SERIALIZE_TYPE_ANNOTATED =			0x0b000000;
		static constexpr DWORD SERIALIZE_TYPE_BINARY =				0x0c000000;
		static constexpr DWORD SERIALIZE_TYPE_BINARY_FILE =			0x0d000000;
		static constexpr DWORD SERIALIZE_TYPE_DATATYPE =			0x0e000000;
		static constexpr DWORD SERIALIZE_TYPE_VECTOR_TYPED =		0x0f000000;
		static constexpr DWORD SERIALIZE_TYPE_ERROR =				0x10000000;
		static constexpr DWORD SERIALIZE_TYPE_IMAGE32 =				0x11000000;
		static constexpr DWORD SERIALIZE_TYPE_OBJECT =				0x12000000;
		static constexpr DWORD SERIALIZE_TYPE_TABLE =				0x13000000;
		static constexpr DWORD SERIALIZE_TYPE_STRUCT =				0x14000000;
		static constexpr DWORD SERIALIZE_TYPE_VECTOR_INT32 =		0x15000000;
		static constexpr DWORD SERIALIZE_TYPE_VECTOR_INTIP =		0x16000000;
		static constexpr DWORD SERIALIZE_TYPE_VECTOR_FLOAT64 =		0x17000000;
		static constexpr DWORD SERIALIZE_TYPE_VECTOR_STRING =		0x18000000;
		static constexpr DWORD SERIALIZE_TYPE_DATE_TIME =			0x19000000;
		static constexpr DWORD SERIALIZE_TYPE_TIME_SPAN =			0x1a000000;
		static constexpr DWORD SERIALIZE_TYPE_VECTOR_2D =			0x1b000000;
		static constexpr DWORD SERIALIZE_TYPE_TEXT_LINES =			0x1c000000;
		static constexpr DWORD SERIALIZE_TYPE_EXTERNAL =			0x1d000000;
		static constexpr DWORD SERIALIZE_TYPE_LARGE_STRING =		0x1e000000;
		static constexpr DWORD SERIALIZE_TYPE_VECTOR_3D =			0x1f000000;
		static constexpr DWORD SERIALIZE_TYPE_DICTIONARY =			0x20000000;
		static constexpr DWORD SERIALIZE_TYPE_TENSOR =				0x21000000;
		static constexpr DWORD SERIALIZE_TYPE_VECTOR_NUMBER =		0x22000000;
		static constexpr DWORD SERIALIZE_TYPE_RANGE =				0x23000000;
		static constexpr DWORD SERIALIZE_TYPE_FALSE =				0x24000000;
		static constexpr DWORD SERIALIZE_TYPE_EXPRESSION =			0x25000000;
		static constexpr DWORD SERIALIZE_TYPE_TABLE_V2 =			0x26000000;
		static constexpr DWORD SERIALIZE_TYPE_VECTOR_STRING_V2 =	0x27000000;

		static constexpr DWORD SERIALIZE_TYPE_REF =				0x80000000;
		static constexpr DWORD SERIALIZE_TYPE_MASK =			0xff000000;

	private:

		static constexpr DWORDLONG TYPE_MASK =				0xFFFF000000000000;
		static constexpr DWORDLONG VALUE_MASK =				0x0000FFFFFFFFFFFF;
		static constexpr int TYPE_SHIFT =					48;

		static constexpr DWORDLONG ENUM_TYPE_MASK =			0x0000FFFFFFFF0000;
		static constexpr DWORDLONG ENUM_VALUE_MASK =		0x000000000000FFFF;
		static constexpr int ENUM_TYPE_SHIFT =				16;

		static constexpr DWORD TYPE_INFINITY_P =	0x7FF0;
		static constexpr DWORD TYPE_CONSTANTS =		0x7FF1;
		static constexpr DWORD TYPE_INT32 =			0x7FF2;
		static constexpr DWORD TYPE_ENUM =			0x7FF3;
		static constexpr DWORD TYPE_SYMBOL =		0x7FF4;
		static constexpr DWORD TYPE_NAN =			0x7FF8;

		static constexpr DWORD TYPE_INFINITY_N =	0xFFF0;
		static constexpr DWORD TYPE_STRING =		0xFFF1;
		static constexpr DWORD TYPE_COMPLEX =		0xFFF2;
		static constexpr DWORD TYPE_ROW_REF =		0xFFF3;
		static constexpr DWORD TYPE_NULL =			0xFFFF;

		static constexpr DWORDLONG ENCODED_INFINITY_P =		0x7FF0000000000000;
		static constexpr DWORDLONG ENCODED_CONSTANTS =		0x7FF1000000000000;
		static constexpr DWORDLONG ENCODED_INT32 =			0x7FF2000000000000;
		static constexpr DWORDLONG ENCODED_ENUM =			0x7FF3000000000000;
		static constexpr DWORDLONG ENCODED_SYMBOL =			0x7FF4000000000000;
		static constexpr DWORDLONG ENCODED_INFINITY_N =		0xFFF0000000000000;
		static constexpr DWORDLONG ENCODED_STRING =			0xFFF1000000000000;
		static constexpr DWORDLONG ENCODED_COMPLEX =		0xFFF2000000000000;
		static constexpr DWORDLONG ENCODED_ROW_REF =		0xFFF3000000000000;

		static constexpr DWORDLONG VALUE_INFINITY_P =		0x7FF0000000000000;
		static constexpr DWORDLONG VALUE_FALSE =			0x7FF1000000000000;
		static constexpr DWORDLONG VALUE_TRUE =				0x7FF1000000000001;
		static constexpr DWORDLONG VALUE_NAN =				0x7FF8000000000000;
		static constexpr DWORDLONG VALUE_INFINITY_N =		0xFFF0000000000000;
		static constexpr DWORDLONG VALUE_NULL =				0xFFFFFFFFFFFFFFFF;

		size_t CalcSerializeSizeAEONScript (EFormat iFormat) const;
		static int CompareByType (Types iType1, Types iType2);
		static bool DeserializeAEONScript (IByteStream &Stream, IAEONParseExtension *pExtension, CDatum *retDatum);
		static bool DeserializeAEONScript (IByteStream &Stream, CDatum *retDatum) { return DeserializeAEONScript(Stream, NULL, retDatum); }
		static bool DeserializeJSON (IByteStream &Stream, CDatum *retDatum);
		static bool DeserializeTextUTF8 (IByteStream &Stream, CDatum *retDatum);
		static bool DetectFileFormat (const CString &sFilespec, IMemoryBlock &Data, EFormat *retiFormat, CString *retsError);
		static Types GetStringValueType (const CString &sValue, CString& retsClean);
		static bool ParseDateParseOptions (CDatum dOptions, CDateTimeParser::SOptions& retOptions);
		static bool ParseDateParseImpute (CDatum dImpute, CDateTimeParser::SImputeDesc& retImpute);
		static CDateTimeParser::EImpute ParseImputeType (CStringView sValue);

		void SerializeAEONScript (EFormat iFormat, IByteStream &Stream) const;
		void SerializeEnum (EFormat iFormat, IByteStream &Stream) const;
		void SerializeGridLang (IByteStream &Stream) const;
		void SerializeJSON (IByteStream &Stream) const;

		static IComplexDatum& DecodeComplex (DWORDLONG dwData) { return *(IComplexDatum*)DecodePointer(dwData); }
		static double DecodeDouble (DWORDLONG dwData) { return *(double*)&dwData; }
		static DWORD DecodeEnumType (DWORDLONG dwData) { return (DWORD)((dwData & ENUM_TYPE_MASK) >> ENUM_TYPE_SHIFT); }
		static int DecodeEnumValue (DWORDLONG dwData) { return (int)(DWORD)(dwData & ENUM_VALUE_MASK); }
		static int DecodeInt32 (DWORDLONG dwData) { return (int)(DWORD)dwData; }
		static LPSTR DecodeLPSTR (DWORDLONG dwData) { return (LPSTR)DecodePointer(dwData); }
		static void* DecodePointer (DWORDLONG dwData) { return (void *)(dwData & VALUE_MASK); }
		static CStringView DecodeString (DWORDLONG dwData) { return CStringView::FromCStringPtr(DecodeLPSTR(dwData)); }
		static DWORD DecodeType (DWORDLONG dwData) { return (DWORD)((dwData & TYPE_MASK) >> TYPE_SHIFT); }

		static DWORDLONG EncodeBool (bool bValue) { return (bValue ? VALUE_TRUE : VALUE_FALSE); }
		static DWORDLONG EncodeComplex (IComplexDatum& Ptr) { return EncodePointer(&Ptr); }
		static DWORDLONG EncodeEnum (DWORD dwType, int iValue) { return ENCODED_ENUM | ((DWORDLONG)dwType << ENUM_TYPE_SHIFT) | (DWORDLONG)(DWORD)iValue; }
		static DWORDLONG EncodeInt32 (int iValue) { return ENCODED_INT32 | (DWORDLONG)(DWORD)iValue; }
		static DWORDLONG EncodePointer (const void* pPtr) { if ((DWORDLONG)pPtr & TYPE_MASK) throw CException(errFail); return ENCODED_COMPLEX | (DWORDLONG)pPtr; }
		static DWORDLONG EncodeString (LPCSTR pPtr) { if ((DWORDLONG)pPtr & TYPE_MASK) throw CException(errFail); return ENCODED_STRING | (DWORDLONG)pPtr; }

		template<class FUNC> CDatum MathArrayOp () const;

		DWORDLONG m_dwData;

		struct SCompareOrderEntry
			{
			Types iType = typeUnknown;
			int iRank = 0;
			};

		static SAnnotation m_NullAnnotation;
		static constinit std::array<int, typeCount> m_COMPARE_RANK;
		static const SCompareOrderEntry m_COMPARE_ORDER[typeCount];

		friend class CAEONSerializedMap;
	};

template <> class CKeyCompare<CDatum>
	{
	public:
		static int Compare(const CDatum& key1, const CDatum& key2) { return CDatum::CompareExact(key1, key2); }
	};

template <> class CKeyCompareEquivalent<CDatum>
	{
	public:
		static int Compare(const CDatum& key1, const CDatum& key2) { return CDatum::Compare(key1, key2); }
	};

struct SAEONLibraryFunctionCreate
	{
	CString sName;
	std::function<bool(IInvokeCtx&, DWORD, CHexeStackEnv&, CDatum, CDatum, SAEONInvokeResult&)> fnInvoke;
	DWORD dwData = 0;
	DWORD dwExecFlags = 0;
	CDatum dType;
	};

struct SAEONInvokeResult
	{
	CDatum::InvokeResult iResult = CDatum::InvokeResult::unknown;

	//	ok:
	//		dResult = result
	//
	//	error:
	//		dResult = error message
	// 
	//	functionCall:
	//		dResult = function to run
	//		Args = function args
	//		dContext = context
	//
	//	runInputRequest:
	//	runInputRequestDebugSim:
	//
	//	runInvoke: Message
	//		dResult = message
	//		dContext = payload

	CDatum dResult;
	CDatum dContext;
	TArray<CDatum> Args;
	};

#include "AEONTypeSystem.h"
#include "AEONInterfaces.h"
#include "AEONArrayImpl.h"

//	Serialization --------------------------------------------------------------

class CAEONSerializedMap
	{
	public:

		CAEONSerializedMap ();
		DWORD Add (CDatum dValue);
		void Add (DWORD dwID, CDatum dValue);
		DWORD AddIfNew (CDatum dValue, bool* retbNew = NULL);
		DWORD AddIfNew (const IComplexDatum* pThis, bool* retbNew = NULL) { return AddIfNew(CDatum::raw_AsComplex(pThis), retbNew); }
		bool Find (CDatum dValue, DWORD* retdwID = NULL) const;
		bool Find (const IComplexDatum* pThis, DWORD* retdwID = NULL) const { return Find(CDatum::raw_AsComplex(pThis), retdwID); }
		CDatum Get (DWORD dwID) const;
		bool IsLocal () const { return m_bLocal; }
		void SetLocal (bool bValue = true) { m_bLocal = bValue; }
		bool WriteID (IByteStream& Stream, CDatum dValue, DWORD dwType);
		bool WriteID (IByteStream& Stream, const IComplexDatum* pThis, DWORD dwType) { return WriteID(Stream, CDatum::raw_AsComplex(pThis), dwType); }

	private:

		TArray<CDatum> m_Map;						//	Indexed by ID
		std::unordered_map<DWORDLONG, DWORD> m_ReverseMap;
		bool m_bLocal = false;						//	If TRUE, we are serializing to a local machine
	};

//	IComplexDatum --------------------------------------------------------------

class IComplexDatum
	{
	public:

		class CRecursionGuard
			{
			public:
				CRecursionGuard (const IComplexDatum& This) : 
						m_pThis(&This),
						m_bInRecursion(This.m_bMarked)
					{ if (!m_bInRecursion) This.m_bMarked = true; }

				~CRecursionGuard (void)
					{ if (!m_bInRecursion) m_pThis->m_bMarked = false; }

				bool InRecursion (void) const { return m_bInRecursion; }

			private:
				const IComplexDatum* m_pThis;
				bool m_bInRecursion;
			};

		static constexpr DWORD FLAG_SERIALIZE_AS_STRUCT =	0x00000001;
		static constexpr DWORD FLAG_SERIALIZE_NO_TYPENAME =	0x00000002;

		IComplexDatum () { }
		virtual ~IComplexDatum () { }

		virtual void Append (CDatum dDatum) { }
		virtual void AppendArray (CDatum dDatum);
		virtual int AsArrayIndex (int iArrayLen, bool* retbFromEnd = NULL) const { return -1; }
		virtual IByteStream& AsStream () const { return NULL_STREAM; }
		virtual CString AsString () const { return NULL_STR; }
		virtual CDatum AsStruct () const;
		virtual void CacheInvokeResult (CHexeLocalEnvironment& LocalEnv, CDatum dResult) { }
		virtual size_t CalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const;
		virtual size_t CalcMemorySize () const = 0;
		virtual bool CanInvoke () const { return false; }
		virtual const CDateTime &CastCDateTime () const { return NULL_DATETIME; }
		virtual const CIPInteger &CastCIPInteger () const { return NULL_IPINTEGER; }
		virtual const CRGBA32Image &CastCRGBA32Image () const { return CRGBA32Image::Null(); }
		virtual CStringView CastCString () const { return CStringView(); }
		virtual const CStringFormat& CastCStringFormat () const { return CStringFormat::Null; }
		virtual const CTimeSpan &CastCTimeSpan () const { return CTimeSpan::Null(); }
		virtual const CVector2D& CastCVector2D () const { return CVector2D::Null; }
		virtual const CVector3D& CastCVector3D () const { return CVector3D::Null; }
		virtual double CastDouble () const { return CDatum::CreateNaN(); }
		virtual DWORDLONG CastDWORDLONG () const { return 0; }
		virtual const IDatatype &CastIDatatype () const;
		virtual int CastInteger32 () const { return 0; }
		void ClearMark () { m_bMarked = false; }
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const { return NULL; }
		virtual bool Contains (CDatum dValue) const { return false; }
		virtual void DeleteElement (int iIndex) { }
		bool DeserializeAEONScript (CDatum::EFormat iFormat, const CString &sTypename, CCharStream *pStream);
		virtual bool DeserializeJSON (const CString &sTypename, const TArray<CDatum> &Data);
		virtual bool EnumElements (DWORD dwFlags, std::function<bool(CDatum)> fn) const;
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const { return false; }
		virtual CDatum FindAll (CDatum dValue) const { return CDatum(); }
		virtual CDatum FindAllExact (CDatum dValue) const { return CDatum(); }
		virtual bool FindExact (CDatum dValue, int *retiIndex = NULL) const { return false; }
		virtual bool FindElement (const CString &sKey, CDatum *retpValue) const { return false; }
		virtual int FindMaxElement () const { return (GetCount() > 0 ? 0 : -1); }
		virtual int FindMinElement () const { return (GetCount() > 0 ? 0 : -1); }
		virtual CString Format (const CStringFormat& Format) const { return AsString(); }
		virtual const CDatum::SAnnotation& GetAnnotation () const { return CDatum().GetAnnotation(); }
		virtual CDatum GetArrayElementUnchecked (int iIndex) const { return GetElement(iIndex); }
		virtual DWORD GetBasicDatatype () const = 0;
		virtual CDatum::Types GetBasicType () const = 0;
		virtual void* GetBinaryData () const { return (void*)CastCString().GetPointer(); }
		virtual int GetBinarySize () const { return CastCString().GetLength(); }
		virtual DWORDLONG GetBinarySize64 () const { return GetBinarySize(); }
		virtual CDatum::ECallType GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const { return CDatum::ECallType::None; }
		virtual IAEONCanvas *GetCanvasInterface () { return NULL; }
		virtual int GetCount () const = 0;
		virtual CDatum GetDatatype () const;
		virtual int GetDimensions () const { return 0; }
		virtual CDatum GetElement (IInvokeCtx *pCtx, int iIndex) const { return GetElement(iIndex); }
		virtual CDatum GetElement (int iIndex) const = 0;
		virtual CDatum GetElement (IInvokeCtx *pCtx, const CString &sKey) const { return GetElement(sKey); }
		virtual CDatum GetElement (const CString &sKey) const { return CDatum(); }
		virtual CDatum GetElementAt (int iIndex) const { return GetElement(iIndex); }
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const;
		virtual CDatum GetElementAt2DA (CDatum dIndex1, CDatum dIndex2) const { return CDatum(); }
		virtual CDatum GetElementAt3DA (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3) const { return CDatum(); }
		virtual CDatum GetElementAt2DI (int iIndex1, int iIndex2) const { return CDatum(); }
		virtual CDatum GetElementAt3DI (int iIndex1, int iIndex2, int iIndex3) const { return CDatum(); }
		virtual CRGBA32Image *GetImageInterface () { return NULL; }
		virtual CString GetKey (int iIndex) const { return NULL_STR; }
		virtual CDatum GetKeyEx (int iIndex) const { return CDatum(GetKey(iIndex)); }
		virtual CDatum GetMethod (const CString &sMethod) const { return CDatum(); }
		virtual IComplexDatum* GetMethodThis () { return this; }
		virtual CDatum::Types GetNumberType (int *retiValue) { return CDatum::typeNaN; }
		virtual CDatum GetProperty (const CString& sProperty) const { return GetElement(sProperty); }
		virtual const CAEONExpression* GetQueryInterface () const { return NULL; }
		virtual IAEONRange* GetRangeInterface () { return NULL; }
		virtual IAEONReanimator* GetReanimatorInterface () { return NULL; }
		virtual IAEONTable *GetTableInterface () { return NULL; }
		virtual IAEONTextLines *GetTextLinesInterface () { return NULL; }
		virtual const CString &GetTypename () const = 0;
		virtual void GrowToFit (int iCount) { }
		virtual size_t Hash () const;
		virtual bool HasKeys () const { return IsStruct(); }
		virtual void InsertElementAt (CDatum dIndex, CDatum dDatum) { }
		virtual void InsertEmpty (int iCount);
		virtual CDatum::InvokeResult Invoke (IInvokeCtx *pCtx, CHexeLocalEnvironment& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult) { retResult.iResult = CDatum::InvokeResult::ok; return CDatum::InvokeResult::ok; }
		virtual CDatum::InvokeResult InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, SAEONInvokeResult& retResult) { retResult.iResult = CDatum::InvokeResult::ok; return CDatum::InvokeResult::ok; }
		virtual CDatum::InvokeResult InvokeLibrary (IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult) { retResult.iResult = CDatum::InvokeResult::ok; return CDatum::InvokeResult::ok; }
		virtual bool InvokeMethodImpl (CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) { retResult.dResult = CString("Methods not supported."); return false; }
		virtual bool IsArray () const = 0;
		virtual bool IsContainer () const { return false; }
		virtual bool IsError () const { return false; }
		virtual bool IsImmutable () const { return false; }
		virtual bool IsIPInteger () const { return false; }
		bool IsMarked () const { return m_bMarked; }

		//	Implies that GetBinaryData and GetBinarySize work and are valid.
		//	NOTE: In previous versions this implied that you could cast to CString,
		//	but we've changed this to support 64-bit memory blocks.

		virtual bool IsMemoryBlock () const { return (GetBinaryData() != NULL); }

		virtual bool IsNil () const { return false; }
		virtual bool IsStruct () const { return false; }
		virtual CDatum IteratorBegin () const { return (0 < GetCount() ? CDatum(0) : CDatum()); }
		virtual CDatum IteratorGetKey (CDatum dIterator) const;
		virtual CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const;
		virtual CDatum IteratorNext (CDatum dIterator) const { int iNext = (int)dIterator + 1; return (iNext < GetCount() ? CDatum(iNext) : CDatum()); }
		void Mark () { if (!m_bMarked) { m_bMarked = true; OnMarked(); } }	//	Check m_bMarked to avoid infinite recursion
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const { return KeyCompareNoCase(AsString(), dValue.AsString()); }
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const { return KeyCompare(AsString(), dValue.AsString()); }
		virtual CDatum OpConcatenated (IInvokeCtx& Ctx, CDatum dSrc, int iAxis = 0) const { return CDatum().OpConcatenated(Ctx, dSrc, iAxis); }
		virtual bool OpContains (CDatum dValue) const { return Find(dValue); }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const { return OpCompare(iValueType, dValue) == 0; }
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const { return OpCompareExact(iValueType, dValue) == 0; }
		virtual CDatum MathAbs () const { return CDatum::CreateNaN(); }
		virtual void MathAccumulateStats (CDatum::SStatsCtx& Stats) const { if (IsNil()) Stats.iNullCount++; else Stats.iNaNCount++; }
		virtual CDatum MathAddToElements (CDatum dValue) const;
		virtual CDatum MathAddElementsTo (CDatum dValue) const;
		virtual CDatum MathCeil () const { return CDatum::CreateNaN(); }
		virtual CDatum MathDivideElementsBy (CDatum dValue) const;
		virtual CDatum MathDivideByElements (CDatum dValue) const;
		virtual CDatum MathExpElementsTo (CDatum dValue) const;
		virtual CDatum MathExpToElements (CDatum dValue) const;
		virtual CDatum MathFloor () const { return CDatum::CreateNaN(); }
		virtual CDatum MathInvert () const { return CDatum(); }
		virtual CDatum MathMatMul (CDatum dValue) const { return CDatum(); }
		virtual CDatum MathModElementsBy (CDatum dValue) const;
		virtual CDatum MathModByElements (CDatum dValue) const;
		virtual CDatum MathMultiplyElements (CDatum dValue) const;
		virtual CDatum MathNegateElements () const;
		virtual CDatum MathSubtractFromElements (CDatum dValue) const;
		virtual CDatum MathSubtractElementsFrom (CDatum dValue) const;
		virtual CDatum MathAverage () const;
		virtual CDatum MathMax () const { return CDatum::CreateNaN(); }
		virtual CDatum MathMedian () const { return CDatum::CreateNaN(); }
		virtual CDatum MathMin () const { return CDatum::CreateNaN(); }
		virtual CDatum MathRound () const { return CDatum::CreateNaN(); }
		virtual CDatum MathSign () const { return CDatum::CreateNaN(); }
		virtual CDatum MathSum () const;
		virtual bool RemoveAll () { return false; }
		virtual bool RemoveElementAt (CDatum dIndex) { return false; }
		virtual void ResolveDatatypes (const CAEONTypeSystem &TypeSystem) { }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const = 0;
		virtual void SetArrayElementUnchecked (int iIndex, CDatum dDatum) { SetElement(iIndex, dDatum); }
		virtual void SetElement (IInvokeCtx *pCtx, const CString &sKey, CDatum dDatum) { SetElement(sKey, dDatum); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) { }
		virtual void SetElement (int iIndex, CDatum dDatum) { }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum);
		virtual void SetElementAt2DA (CDatum dIndex1, CDatum dIndex2, CDatum dValue) { }
		virtual void SetElementAt2DI (int iIndex1, int iIndex2, CDatum dValue) { }
		virtual void SetElementAt3DA (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3, CDatum dValue) { }
		virtual void SetElementAt3DI (int iIndex1, int iIndex2, int iIndex3, CDatum dValue) { }
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) { }
		virtual void WriteBinaryToStream (IByteStream &Stream, int iPos = 0, int iLength = -1, IProgressEvents *pProgress = NULL) const;
		virtual void WriteBinaryToStream64 (IByteStream64 &Stream, DWORDLONG dwPos = 0, DWORDLONG dwLength = 0xffffffffffffffff, IProgressEvents *pProgress = NULL) const;

		static CDatum DeserializeAEONAsExternal (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const { return 0; }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) { ASSERT(false); return false; }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct);
		virtual DWORD OnGetSerializeFlags () const { return 0; }
		virtual void OnMarked () { }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const { ASSERT(false); }
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const;

		virtual CDatum raw_IteratorGetElement (CBuffer& Iterator) const { int* pI = (int *)Iterator.GetPointer(); return GetElement(*pI); }
		virtual CDatum raw_IteratorGetKey (CBuffer& Iterator) const { int* pI = (int *)Iterator.GetPointer(); return GetKey(*pI); }
		virtual bool raw_IteratorHasMore (CBuffer& Iterator) const { int* pI = (int *)Iterator.GetPointer(); return (*pI < GetCount()); }
		virtual void raw_IteratorNext (CBuffer& Iterator) const { int* pI = (int *)Iterator.GetPointer(); (*pI)++; }
		virtual void raw_IteratorSetElement (CBuffer& Iterator, CDatum dValue) { int* pI = (int *)Iterator.GetPointer(); SetElement(*pI, dValue); }
		virtual CBuffer raw_IteratorStart () const { CBuffer Buffer(sizeof(int)); *(int*)Buffer.GetPointer() = 0; return Buffer; }

		CString AsAddress () const;
		size_t CalcSerializeAsStructSize (CDatum::EFormat iFormat) const;
		void SerializeAEONAsExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) const;
		void SerializeAEONAsStruct (IByteStream& Stream, CAEONSerializedMap& Serialized) const;
		void SerializeAsStruct (CDatum::EFormat iFormat, IByteStream &Stream) const;
		CString StructAsString () const;

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) { throw CException(errFail); }
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const { throw CException(errFail); }

		mutable bool m_bMarked = false;
	};

class IComplexFactory
	{
	public:
		virtual ~IComplexFactory () { }

		virtual IComplexDatum* Create () = 0;
		virtual CDatum CreateAsType (CDatum dValue) { return CDatum(); }
	};

class CDatumArrayWrapper
	{
	public:
		CDatumArrayWrapper (CDatum dArray) : m_dArray(dArray) { }

		CDatum operator [] (int iIndex) const { return m_dArray.GetElement(iIndex); }

		int GetCount () const { return m_dArray.GetCount(); }
		bool IsElementEqual (int iIndex, const CDatumArrayWrapper& Src, int iSrcIndex) const { return m_dArray.GetElement(iIndex) == Src.m_dArray.GetElement(iSrcIndex); }
		bool IsIndexValid (int iIndex) const { return iIndex >= 0 && iIndex < m_dArray.GetCount(); }

	private:
		CDatum m_dArray;
	};

class CDatumTableWrapper
	{
	public:
		CDatumTableWrapper (CDatum dTable) : m_dTable(dTable), m_pTable(dTable.GetTableInterface()) 
			{
			if (!m_pTable) throw CException(errFail);
			}

		CDatum operator [] (int iIndex) const { return m_dTable.GetElement(iIndex); }

		int GetCount () const { return m_dTable.GetCount(); }
		bool IsElementEqual (int iIndex, const CDatumTableWrapper& Src, int iSrcIndex) const;
		bool IsIndexValid (int iIndex) const { return iIndex >= 0 && iIndex < m_dTable.GetCount(); }

	private:
		CDatum m_dTable;
		const IAEONTable* m_pTable = NULL;
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

		static CDatum CreateAsType (CDatum dValue) { return CDatum(); }

		static void RegisterFactory ()
			{
			CDatum::RegisterExternalType(VALUE::StaticGetTypename(), new CFactory);
			}

		//	IComplexDatum

		virtual CString AsString () const override { return strPattern("[%s]", GetTypename()); }
		virtual size_t CalcMemorySize () const override { return sizeof(VALUE); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeCustom; }
		virtual int GetCount () const override { return 1; }
		virtual CDatum GetElement (IInvokeCtx *pCtx, int iIndex) const override { return GetElement(iIndex); }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual CDatum GetElement (IInvokeCtx *pCtx, const CString &sKey) const override { return GetElement(sKey); }
		virtual CDatum GetElement (const CString &sKey) const override { return CDatum(); }
		virtual CString GetKey (int iIndex) const override { return NULL_STR; }
		virtual const CString &GetTypename () const override { return VALUE::StaticGetTypename(); }
		virtual bool IsArray () const override { return false; }

		//	Subclasses may either override this method or override 
		//	SerializeAEONExternal and DeserializeAEONExternal.

		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override { SerializeAEONAsExternal(Stream, Serialized); }

	protected:

	private:
		class CFactory : public IComplexFactory
			{
			public:
				CFactory ()
					{ }

				//	IComplexFactory
				virtual IComplexDatum* Create () override { return new VALUE; }
				virtual CDatum CreateAsType (CDatum dValue) override { return VALUE::CreateAsType(dValue); }
			};
	};

#include "AEONInvoke.h"
#include "AEONUtil.h"

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
		virtual CString AsString () const override { return Format(CStringFormat()); }
		virtual size_t CalcMemorySize () const override;
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual bool Contains (CDatum dValue) const override;
		virtual void DeleteElement (int iIndex) override;
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const override;
		virtual CDatum FindAll (CDatum dValue) const override;
		virtual CDatum FindAllExact (CDatum dValue) const override;
		virtual bool FindExact (CDatum dValue, int *retiIndex = NULL) const override;
		virtual int FindMaxElement () const override { return FindMaxElementInArray(m_Array); }
		virtual int FindMinElement () const override { return FindMinElementInArray(m_Array); }
		virtual CString Format (const CStringFormat& Format) const override;
		virtual CDatum GetArrayElementUnchecked (int iIndex) const override { return m_Array[iIndex]; }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::ARRAY; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeArray; }
		virtual int GetCount () const override { return m_Array.GetCount(); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY); }
		virtual int GetDimensions () const override { return 1; }
		virtual CDatum GetElement (int iIndex) const override { return ((iIndex >= 0 && iIndex < m_Array.GetCount()) ? m_Array[iIndex] : CDatum()); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElementAt (int iIndex) const override;
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override { return TArrayImpl<CComplexArray, CDatum>::GetElementAt(this, m_Array, dIndex); }
		virtual CDatum GetMethod (const CString &sMethod) const override { if (m_pMethodsExt) return m_pMethodsExt->GetMethod(sMethod); else return CDatum(); }
		virtual const CString &GetTypename () const override;
		virtual void GrowToFit (int iCount) override { OnCopyOnWrite(); m_Array.GrowToFit(iCount); }
		virtual void InsertElementAt (CDatum dIndex, CDatum dDatum) override;
		virtual bool IsArray () const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil () const override { return (GetCount() == 0); }
		virtual CDatum IteratorGetKey (CDatum dIterator) const override { return dIterator; };
		virtual CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const override { return TArrayImpl<CComplexArray, CDatum>::GetElementAt(this, m_Array, dIterator); }
		virtual CDatum MathAbs () const override;
		virtual void MathAccumulateStats (CDatum::SStatsCtx& Stats) const override;
		virtual CDatum MathMax () const override { CRecursionGuard Guard(*this); if (Guard.InRecursion()) return CDatum(); return CalcMax(m_Array); }
		virtual CDatum MathMedian () const override { CRecursionGuard Guard(*this); if (Guard.InRecursion()) return CDatum(); return CalcMedian(m_Array); }
		virtual CDatum MathMin () const override { CRecursionGuard Guard(*this); if (Guard.InRecursion()) return CDatum(); return CalcMin(m_Array); }
		virtual CDatum MathRound () const override;
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override { return CompareArray(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return CompareArrayExact(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual CDatum OpConcatenated (IInvokeCtx& Ctx, CDatum dSrc, int iAxis = 0) const override;
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override { return IsArrayEqual(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override { return IsArrayIdentical(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual bool RemoveAll () override { OnCopyOnWrite(); m_Array.DeleteAll(); return true; }
		virtual bool RemoveElementAt (CDatum dIndex) override;
		virtual void ResolveDatatypes (const CAEONTypeSystem &TypeSystem) override;
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { OnCopyOnWrite(); if (pfCompare) m_Array.Sort(pCtx, pfCompare, Order); else m_Array.Sort(Order); }
		virtual void SetArrayElementUnchecked (int iIndex, CDatum dValue) override { m_Array[iIndex] = dValue; }
		virtual void SetElement (int iIndex, CDatum dDatum) override { OnCopyOnWrite(); if (iIndex >= 0 && iIndex < m_Array.GetCount()) m_Array[iIndex] = dDatum; }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { OnCopyOnWrite(); m_Properties.SetProperty(*this, sKey, dDatum, NULL); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override { OnCopyOnWrite(); TArrayImpl<CComplexArray, CDatum>::SetElementAt(this, m_Array, dIndex, dDatum); }

		static CDatum CalcMax (const TArray<CDatum>& Array);
		static CDatum CalcMedian (const TArray<CDatum>& Array);
		static CDatum CalcMin (const TArray<CDatum>& Array);
		static int CompareArray (CDatum dSrc, CDatum::Types iValueType, CDatum dValue);
		static int CompareArrayExact (CDatum dSrc, CDatum::Types iValueType, CDatum dValue);
		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static int FindMaxElementInArray (const TArray<CDatum>& Array);
		static int FindMinElementInArray (const TArray<CDatum>& Array);
		static int FindMethodByKey (const CString& sKey) { return (m_pMethodsExt ? m_pMethodsExt->FindMethod(sKey) : -1); }
		static int FindPropertyByKey (const CString& sKey) { return m_Properties.FindProperty(sKey); }
		static CString Format (const TArray<CDatum>& Array, const CStringFormat& Format);
		static CDatum FromDatum (CDatum dValue) { return dValue; }
		static CDatum GetIndices (CDatum dArray);
		static int GetMethodCount () { return (m_pMethodsExt ? m_pMethodsExt->GetCount() : 0); }
		static CString GetMethodKey (int iIndex) { return (m_pMethodsExt ? m_pMethodsExt->GetMethodName(iIndex) : NULL_STR); }
		static CDatum GetMethodType (int iIndex) { return (m_pMethodsExt ? m_pMethodsExt->GetMethodType(iIndex) : CAEONTypes::Get(IDatatype::FUNCTION)); }
		static int GetPropertyCount () { return m_Properties.GetCount(); }
		static CString GetPropertyKey (int iIndex) { return m_Properties.GetPropertyName(iIndex); }
		static CDatum GetPropertyType (int iIndex) { return m_Properties.GetPropertyType(iIndex); }
		static bool IsArrayEqual (CDatum dSrc, CDatum::Types iValueType, CDatum dValue);
		static bool IsArrayIdentical (CDatum dSrc, CDatum::Types iValueType, CDatum dValue);
		static CDatum MakeNullElement () { return CDatum(); }
		static void SetMethodsExt (TDatumMethodHandler<IComplexDatum> &MethodsExt) { m_pMethodsExt = &MethodsExt; }
		static CDatum ToDatum (CDatum dValue) { return dValue; }

	protected:
		void CloneContents ();
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual void OnMarked () override;
		void OnCopyOnWrite ();

		bool m_bCopyOnWrite = false;
		TArray<CDatum> m_Array;

		static TDatumPropertyHandler<CComplexArray> m_Properties;
		static TDatumMethodHandler<IComplexDatum> *m_pMethodsExt;
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
		virtual IByteStream& AsStream () const { m_File.Seek(sizeof(SHeader)); return m_File; }
		virtual size_t CalcMemorySize () const override { return sizeof(CComplexBinaryFile); }
		virtual CStringView CastCString () const override;
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual DWORD GetBasicDatatype () const override { return IDatatype::BINARY; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeBinary; }
		virtual int GetBinarySize () const override { return m_dwLength; }
		virtual int GetCount () const override { return 1; }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual const CString &GetTypename () const override;
		virtual bool IsArray () const override { return false; }
		virtual bool IsMemoryBlock () const override { return false; }
		virtual bool IsNil () const override { return (m_dwLength == 0); }
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void WriteBinaryToStream (IByteStream &Stream, int iPos = 0, int iLength = -1, IProgressEvents *pProgress = NULL) const override;
		virtual void WriteBinaryToStream64 (IByteStream64 &Stream, DWORDLONG dwPos = 0, DWORDLONG dwLength = 0xffffffffffffffff, IProgressEvents *pProgress = NULL) const override;

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);

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

		static void CreateBinaryFile (CString *retsFilespec, CFile *retFile);
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

		bool DeleteElement (const CString &sKey) { OnCopyOnWrite(); return m_Map.DeleteAt(sKey); }
		bool SetElementIfNew (const CString& sKey, CDatum dValue);

		//	IComplexDatum
		virtual void Append (CDatum dDatum) override { AppendStruct(dDatum); }
		virtual CString AsString () const override;
		virtual size_t CalcMemorySize () const override;
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual bool Contains (CDatum dValue) const override;
		virtual bool FindElement (const CString &sKey, CDatum *retpValue) const override;
		virtual DWORD GetBasicDatatype () const override { return IDatatype::STRUCT; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeStruct; }
		virtual int GetCount () const override { return m_Map.GetCount(); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::STRUCT); }
		virtual CDatum GetElement (int iIndex) const override { return ((iIndex >= 0 && iIndex < m_Map.GetCount()) ? m_Map[iIndex] : CDatum()); }
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override;
		virtual CString GetKey (int iIndex) const override { return m_Map.GetKey(iIndex); }
		virtual CDatum GetKeyEx (int iIndex) const override { return CDatum(m_Map.GetKey(iIndex)); }
		virtual CDatum GetMethod (const CString &sMethod) const override;
		virtual CDatum GetProperty (const CString& sProperty) const override;
		virtual const CString &GetTypename () const override;
		virtual void GrowToFit (int iCount) override { OnCopyOnWrite(); m_Map.GrowToFit(iCount); }
		virtual bool HasKeys () const override { return true; }
		virtual void InsertElementAt (CDatum dIndex, CDatum dDatum) override { SetElementAt(dIndex, dDatum); }
		virtual bool IsArray () const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil () const override { return (GetCount() == 0); }
		virtual bool IsStruct () const override { return true; }
		virtual CDatum IteratorGetKey (CDatum dIterator) const override { return GetKeyEx((int)dIterator); };
		virtual CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const override { return GetElement((int)dIterator); }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool OpContains (CDatum dValue) const override { return !GetElement(dValue.AsString()).IsIdenticalToNil(); }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool RemoveAll () override { OnCopyOnWrite(); m_Map.DeleteAll(); return true; }
		virtual bool RemoveElementAt (CDatum dIndex) override;
		virtual void ResolveDatatypes (const CAEONTypeSystem &TypeSystem) override;
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override { SerializeAsStruct(iFormat, Stream); }
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override { SerializeAEONAsStruct(Stream, Serialized); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { OnCopyOnWrite(); m_Map.SetAt(sKey, dDatum); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override;

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static int FindPropertyByKey (CStringView sKey) { return m_Properties.FindProperty(sKey); }
		static CDatum GetPropertyType (int iIndex) { return m_Properties.GetPropertyType(iIndex); }
		static void SetMethodsExt (TDatumMethodHandler<IComplexDatum> &MethodsExt) { m_pMethodsExt = &MethodsExt; }

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override { return CalcSerializeAsStructSize(iFormat); }
		virtual void OnMarked () override;

		void AppendStruct (CDatum dDatum);
		void CloneContents ();
		void OnCopyOnWrite ();

		bool m_bCopyOnWrite = false;
		TSortMap<CString, CDatum> m_Map;

		static TDatumPropertyHandler<CComplexStruct> m_Properties;
		static TDatumMethodHandler<IComplexDatum> *m_pMethodsExt;
	};

//	Numbers --------------------------------------------------------------------

class CNumberValue
	{
	public:
		CNumberValue () { }
		CNumberValue (CDatum dValue) { InitFrom(dValue); }
		CNumberValue (const CNumberValue& Src) = default;
		CNumberValue (CNumberValue&& Src) = default;

		CNumberValue& operator = (const CNumberValue& Src) = default;
		CNumberValue& operator = (CNumberValue&& Src) = default;

		void Abs ();
		void Add (CDatum dValue);
		double AsDouble () const;
		CIPInteger AsIPInteger () const;
		void Ceil ();
		int Compare (CDatum dValue) const {  CNumberValue Src(dValue); return Compare(Src); }
		int Compare (const CNumberValue &Value) const;
		void ConvertToDouble ();
		void ConvertToIPInteger ();
		bool Divide (CDatum dValue);
		bool DivideReversed (CDatum dValue);
		void Floor ();
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
		void Round ();
		void SetDouble (double rValue) { m_rValue = rValue; m_bUpconverted = true; m_iType = CDatum::typeDouble; }
		void SetInteger (int iValue) { m_pValue = (void *)(DWORD_PTR)iValue; m_bUpconverted = true; m_iType = CDatum::typeInteger32; }
		void SetInteger64 (DWORDLONG ilValue) { m_ilValue = ilValue; m_bUpconverted = true; m_iType = CDatum::typeInteger64; }
		void SetIPInteger (const CIPInteger &Value) { m_ipValue = Value; m_pValue = &m_ipValue; m_bUpconverted = true; m_iType = CDatum::typeIntegerIP; }
		void SetTimeSpan (const CTimeSpan &Value) { m_tsValue = Value; m_pValue = &m_tsValue; m_bUpconverted = true; m_iType = CDatum::typeTimeSpan; }
		void SetNaN () { m_bUpconverted = true; m_iType = CDatum::typeNaN; }
		void SetNil () { m_bUpconverted = true; m_iType = CDatum::typeNil; }
		void Sign ();
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
	};

inline int KeyCompare (const CNumberValue &dKey1, const CNumberValue &dKey2) { return dKey1.Compare(dKey2); }

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
		ETokens ParseObject (CDatum& retDatum);
		ETokens ParseString (CDatum *retDatum);
		ETokens ParseStruct (CDatum *retDatum);

		CCharStream *m_pStream;
		IAEONParseExtension *m_pExtension;
	};

//	Helpers

bool urlParseQuery (const CString &sURL, CString *retsPath, CDatum *retdQuery);

//	Some implementation details

#include "AEONAllocator.h"
#include "AEONOperators.h"
#include "AEONExpression.h"
#include "AEONVector.h"
#include "AEONAlgorithms.h"
#include "AEONInlines.h"

