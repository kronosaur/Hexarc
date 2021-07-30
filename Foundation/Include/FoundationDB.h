//	FoundationDB.h
//
//	Foundation header file
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

#include <functional>

class CDBFormatDesc;
class IDBValueObject;

class CDBValue
	{
	public:
		enum ETypes
			{
			typeUnknown =			-1,

			typeNil =				0,
			typeTrue =				1,
			typeInt32 =				2,
			typeInt64 =				3,
			typeDouble =			4,
			typeString =			5,		//	UTF8
			typeArray =				6,
			typeDateTime =			7,
			typeStruct =			8,
			typeTimeSpan =			9,

			typeSASDate =			101,	//	Encoded as float: Days since January 1, 1960
			typeSASDateTime =		102,	//	Encoded as float: Seconds since January 1, 1960
			typeSASTime =			103,	//	Encoded as float: Seconds since midnight
			};

		CDBValue (void) : m_dwData(0) { }
		CDBValue (const CDBValue &Src) { Copy(Src); }
		CDBValue (CDBValue &&Src) noexcept { m_dwData = Src.m_dwData; Src.m_dwData = 0; }
		CDBValue (const CString &sValue);
		CDBValue (const CDateTime &Value);
		CDBValue (const CTimeSpan &Value);
		CDBValue (int iValue);
		CDBValue (double rValue);
		CDBValue (const CDBValue &Value, const CDBFormatDesc &Format);
		explicit CDBValue (bool bValue);
		explicit CDBValue (ETypes iType);

		static CDBValue FromHandoff (CString &Src);

		~CDBValue (void) { CleanUp(); }

		explicit operator bool () const { return !IsNil(); }
		operator int () const;
		operator double () const;
		operator const CDateTime & () const;
		operator LONGLONG () const;
		operator const CString & () const;

		CDBValue &operator= (const CDBValue &Src) { CleanUp(); Copy(Src); return *this; }
		CDBValue &operator= (CDBValue &&Src) noexcept { CleanUp(); m_dwData = Src.m_dwData; Src.m_dwData = 0; return *this; }

		CDateTime AsDateTime (void) const;
		double AsDouble (bool *retbValid = NULL) const;
		int AsInt32 (bool *retbValid = NULL) const;
		CTimeSpan AsTimeSpan (void) const;
		CString AsString (void) const;
		const CDBValue &GetElement (int iIndex) const;
		const CDBValue &GetElement (const CString &sKey) const;
		int GetElementCount (void) const;
		const CString &GetElementKey (int iIndex) const;
		const CDBFormatDesc &GetFormat () const;
		CDBValue GetProperty (const CString &sProperty) const;
		ETypes GetType (void) const;
		bool IsBlank (void) const;
		bool IsNaN () const;
		bool IsNil (void) const { return m_dwData == 0; }
		void Push (const CDBValue &Value);
		void SetElement (const CString &sKey, const CDBValue &Value);
		CDBValue StripFormat () const;

		static ETypes Coerce (ETypes iType1, ETypes iType2);
		static ETypes Coerce (const CDBValue &Value1, const CDBValue &Value2) { return Coerce(Value1.GetType(), Value2.GetType()); }
		static int Compare (const CDBValue &Left, const CDBValue &Right, ETypes *retiType = NULL);
		static CDBValue ConvertSASDate (double rValue);
		static CDBValue ConvertSASDateTime (double rValue);
		static CDBValue ConvertSASTime (double rValue);
		static const CString &TypeName (ETypes iType);

		static const CDBValue Null;

	private:
		static constexpr DWORD DISCRIMINATOR_1_MASK =	0x00000003;
		static constexpr DWORD DISCRIMINATOR_2_MASK =	0x0000000f;

		static constexpr DWORD TYPE_STRING =			0x00;
		static constexpr DWORD TYPE_OBJECT =			0x01;
		static constexpr DWORD TYPE_SPECIAL_60 =		0x03;

		static constexpr DWORD TYPE_INT_32 =			0x03;
		static constexpr DWORD TYPE_INT_60 =			0x07;
		static constexpr DWORD TYPE_DOUBLE =			0x0b;
		static constexpr DWORD TYPE_SPECIAL =			0x0f;

		static constexpr DWORDLONG SPECIAL_TRUE =		0x000000000000010f;

		void CleanUp (void);
		void Copy (const CDBValue &Src);

		static DWORD DecodeDiscriminator1 (DWORDLONG dwData) { return (DWORD)dwData & DISCRIMINATOR_1_MASK; }
		static DWORD DecodeDiscriminator2 (DWORDLONG dwData) { return (DWORD)dwData & DISCRIMINATOR_2_MASK; }
		static IDBValueObject *DecodeObject (DWORDLONG dwData) { return (IDBValueObject *)(dwData & ~(DWORDLONG)DISCRIMINATOR_1_MASK); }
		static int DecodeInt32 (DWORDLONG dwData) { return (int)(DWORD)(dwData >> 32); }
		static LPSTR DecodeString (DWORDLONG dwData) { return (LPSTR)(dwData & ~(DWORDLONG)DISCRIMINATOR_1_MASK); }
		static DWORDLONG EncodeDouble (double rValue);
		static DWORDLONG EncodeInt32 (int iValue) { return ((((DWORDLONG)(DWORD)iValue) << 32) | TYPE_INT_32); }
		static DWORDLONG EncodeObjectPtr (IDBValueObject *pValue) { return (((DWORDLONG)pValue) | TYPE_OBJECT); }
		static DWORDLONG EncodeString (const CString &sValue);

		DWORDLONG m_dwData;
	};

class CDBFormatDesc
	{
	public:
		enum class EBold
			{
			Default,
			Normal,
			Bold,
			};

		enum class EItalic
			{
			Default,
			Normal,
			Italic,
			};

		enum class EStrikeout
			{
			Default,
			Normal,
			Strikeout,
			};

		enum class EUnderline
			{
			Default,
			Normal,
			Underline,
			};

		bool Parse (const CDBValue &Value, CString *retsError = NULL);

		explicit operator bool () const { return !IsEmpty(); }

		const CRGBA32 &GetColor () const { return m_rgbColor; }
		bool IsEmpty () const 
			{ return m_sFont.IsEmpty() && m_rHeight == 0.0 && m_rgbColor.IsNull()
				&& m_iBold == EBold::Default && m_iItalic == EItalic::Default && m_iStrikeout == EStrikeout::Default && m_iUnderline == EUnderline::Default
				&& m_rgbBack.IsNull(); }

		static const CDBFormatDesc &Null () { return m_Null; }

	private:
		static bool ParseColor (const CDBValue &Value, CRGBA32 &retColor, CString *retsError = NULL);
		bool ParseFromStruct (const CDBValue &Value, CString *retsError = NULL);

		CString m_sFont;								//	Blank means default font
		double m_rHeight = 0.0;							//	Height in points (0 == default)
		CRGBA32 m_rgbColor = CRGBA32::Null();			//	Null == default

		EBold m_iBold = EBold::Default;
		EItalic m_iItalic = EItalic::Default;
		EStrikeout m_iStrikeout = EStrikeout::Default;
		EUnderline m_iUnderline = EUnderline::Default;

		CRGBA32 m_rgbBack = CRGBA32::Null();			//	Null == default

		static const CDBFormatDesc m_Null;
	};

class IDBValueObject
	{
	public:
		virtual ~IDBValueObject (void) { }

		virtual CDateTime AsDateTime (void) const { return NULL_DATETIME; }
		virtual double AsDouble (bool *retbValid = NULL) const { if (retbValid) *retbValid = false; return 0.0; }
		virtual int AsInt32 (bool *retbValid = NULL) const { if (retbValid) *retbValid = false; return 0; }
		virtual CTimeSpan AsTimeSpan (void) const { return CTimeSpan(); }
		virtual CString AsString (void) const { return NULL_STR; }
		virtual const CDateTime &CastDateTime (void) const { return NULL_DATETIME; }
		virtual double CastDouble (void) const { return 0.0; }
		virtual int CastInt32 () const { return 0; }
		virtual LONGLONG CastLONGLONG (void) const { return 0; }
		virtual const CString &CastString (void) const { return NULL_STR; }
		virtual IDBValueObject *Clone (void) const = 0;
		virtual const CDBValue &GetElement (int iIndex) const { return CDBValue::Null; }
		virtual const CDBValue &GetElement (const CString &sKey) const { return CDBValue::Null; }
		virtual int GetElementCount (void) const { return 1; }
		virtual const CString &GetElementKey (int iIndex) const { return NULL_STR; }
		virtual const CDBFormatDesc &GetFormat () const { return CDBFormatDesc::Null(); }
		virtual CDBValue GetProperty (const CString &sProperty) const { return GetElement(sProperty); }
		virtual CDBValue::ETypes GetType (void) const = 0;
		virtual bool GetValueWithoutFormat (CDBValue *retValue = NULL) const { return false; }
		virtual void Push (const CDBValue &Value) { }
		virtual void SetElement (const CString &sKey, const CDBValue &Value) { }
	};

class CDBColumnDef
	{
	public:
		CDBColumnDef (void) { }
		CDBColumnDef (const CString &sID, CDBValue::ETypes iType, int iOrdinal = -1, int iSize = -1);
		CDBColumnDef (const CString &sID, const CString &sName, CDBValue::ETypes iType, int iOrdinal = -1, int iSize = -1);

		const CString &GetDesc (void) const { return m_sDesc; }
		const CString &GetDisplayName (void) const { return m_sDisplayName; }
		DWORD GetExtra (void) const { return m_dwExtra; }
		const CString &GetID (void) const { return m_sID; }
		const CString &GetName (void) const { return m_sName; }
		int GetOrdinal (void) const { return m_iOrdinal; }
		int GetSize (void) const { return m_iSize; }
		CDBValue::ETypes GetType (void) const { return m_iType; }
		void SetExtra (DWORD dwExtra) { m_dwExtra = dwExtra; }
		void SetID (const CString &sID);
		void SetDesc (const CString &sDesc) { m_sDesc = sDesc; }
		void SetOrdinal (int iOrdinal) { m_iOrdinal = iOrdinal; }
		void SetType (CDBValue::ETypes iType) { m_iType = iType; m_iDisplayType = iType; }

		static CString GenerateID (const CString &sValue);

	private:
		CString m_sID;						//	ID of column (always lowercase; must be unique)
		CString m_sName;					//	Same as ID, but with case preserved
		CString m_sDesc;					//	Description of column (may have spaces).
		CDBValue::ETypes m_iType = CDBValue::typeUnknown;
		int m_iOrdinal = -1;				//	Column ordinal in table (-1 = unknown)
		int m_iSize = -1;					//	Column size (-1 = default)
		DWORD m_dwExtra = 0;

		CString m_sDisplayName;				//	Display name of column
		CDBValue::ETypes m_iDisplayType = CDBValue::typeUnknown;	//	Convert to this type on display or export
	};

class CDBTable
	{
	public:
		bool AddCol (const CDBColumnDef &ColDef);
		bool AddColUnique (const CDBColumnDef &ColDef);
		int AddRow (void);
		void CleanUp (void);
		int FindColByName (const CString &sName) const;
		bool FindColsByName (const TArray<CString> &Names, TArray<int> &retIndices) const;
		const CDBColumnDef &GetCol (int iIndex) const { return m_Cols[iIndex]; }
		CDBColumnDef &GetCol (int iIndex) { return m_Cols[iIndex]; }
		const TArray<CDBColumnDef> &GetColDef (void) const { return m_Cols; }
		int GetColCount (void) const { return m_Cols.GetCount(); }
		const CDBValue &GetField (int iCol, int iRow) const;
		CDBValue *GetField (int iCol, int iRow);
		const CString &GetName (void) const { return m_sName; }
		int GetRowCount (void) const { return (GetColCount() > 0 ? m_Rows.GetCount() / GetColCount() : 0); }
		void GrowToFit (int iRows) { m_Rows.GrowToFit(iRows * GetColCount()); }
		bool IsEmpty () const { return (m_Cols.GetCount() == 0); }
		void SetColumnDefs (const TArray<CDBColumnDef> &Cols);
		bool SetField (int iCol, int iRow, const CDBValue &Value);
		void SetName (const CString &sName) { m_sName = sName; }
		void TakeHandoff (CDBTable &Src);

	private:
		void InitColIndex (void) const;
		void InvalidateColIndex (void) { m_bColIndexValid = false; }

		CString m_sName;
		TArray<CDBColumnDef> m_Cols;
		TArray<CDBValue> m_Rows;

		mutable bool m_bColIndexValid = false;
		mutable TSortMap<CString, int> m_ColIndex;
	};

class CDBFormatCSV
	{
	public:
		struct SOptions
			{
			bool bUseUTF8 = false;
			bool bAllowShortRows = false;
			char chDelimiter = ',';

			std::function<void(int, const CString &)> fnOnProgress = NULL;
			};

		static bool Load (IByteStream64 &Stream, const SOptions &Options, CDBTable &Table, CString *retsError = NULL);

	private:
		static constexpr int EstimateRowCount (DWORDLONG dwStreamSize);
	};

class CDBFormatRTF
	{
	public:
		static void WriteEoF (IByteStream &Stream);
		static void WriteHeader (IByteStream &Stream);
		static void WriteText (IByteStream &Stream, const CString &sText);
	};

#include "FoundationXLS.h"
