//	FoundationDB.h
//
//	Foundation header file
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

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
		CDBValue (CDBValue &&Src) { m_dwData = Src.m_dwData; Src.m_dwData = 0; }
		CDBValue (const CString &sValue);
		CDBValue (const CDateTime &Value);
		CDBValue (const CTimeSpan &Value);
		CDBValue (int iValue);
		CDBValue (double rValue);
		explicit CDBValue (bool bValue);
		explicit CDBValue (ETypes iType);

		static CDBValue FromHandoff (CString &Src);

		~CDBValue (void) { CleanUp(); }

		operator int () const;
		operator double () const;
		operator const CDateTime & () const;
		operator LONGLONG () const;
		operator const CString & () const;

		CDBValue &operator= (const CDBValue &Src) { CleanUp(); Copy(Src); return *this; }
		CDBValue &operator= (CDBValue &&Src) { CleanUp(); m_dwData = Src.m_dwData; Src.m_dwData = 0; return * this; }

		CDateTime AsDateTime (void) const;
		CTimeSpan AsTimeSpan (void) const;
		CString AsString (void) const;
		const CDBValue &GetElement (const CString &sKey) const;
		ETypes GetType (void) const;
		bool IsNil (void) const { return m_dwData == 0; }
		void SetElement (const CString &sKey, const CDBValue &Value);

		static ETypes Coerce (const CDBValue &Value1, const CDBValue &Value2);
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
		inline static DWORDLONG EncodeInt32 (int iValue) { return ((((DWORDLONG)(DWORD)iValue) << 32) | TYPE_INT_32); }
		inline static DWORDLONG EncodeObjectPtr (IDBValueObject *pValue) { return (((DWORDLONG)pValue) | TYPE_OBJECT); }
		static DWORDLONG EncodeString (const CString &sValue);

		DWORDLONG m_dwData;
	};

class IDBValueObject
	{
	public:
		virtual ~IDBValueObject (void) { }

		virtual CDateTime AsDateTime (void) const { return NULL_DATETIME; }
		virtual CTimeSpan AsTimeSpan (void) const { return CTimeSpan(); }
		virtual CString AsString (void) const { return NULL_STR; }
		virtual const CDateTime &CastDateTime (void) const { return NULL_DATETIME; }
		virtual double CastDouble (void) const { return 0.0; }
		virtual LONGLONG CastLONGLONG (void) const { return 0; }
		virtual IDBValueObject *Clone (void) const = 0;
		virtual const CDBValue &GetElement (const CString &sKey) const { return CDBValue::Null; }
		virtual CDBValue::ETypes GetType (void) const = 0;
		virtual void SetElement (const CString &sKey, const CDBValue &Value) { }
	};

class CDBColumnDef
	{
	public:
		CDBColumnDef (void) { }
		CDBColumnDef (const CString &sID, CDBValue::ETypes iType, int iOrdinal = -1);

		const CString &GetDisplayName (void) const { return m_sDisplayName; }
		DWORD GetExtra (void) const { return m_dwExtra; }
		const CString &GetID (void) const { return m_sID; }
		const CString &GetName (void) const { return m_sName; }
		int GetOrdinal (void) const { return m_iOrdinal; }
		CDBValue::ETypes GetType (void) const { return m_iType; }
		void SetExtra (DWORD dwExtra) { m_dwExtra = dwExtra; }
		void SetID (const CString &sID);
		void SetDesc (const CString &sDesc) { m_sDesc = sDesc; }
		void SetOrdinal (int iOrdinal) { m_iOrdinal = iOrdinal; }
		void SetType (CDBValue::ETypes iType) { m_iType = iType; m_iDisplayType = iType; }

	private:
		CString m_sID;						//	ID of column (always lowercase; must be unique)
		CString m_sName;					//	Same as ID, but with case preserved
		CString m_sDesc;					//	Description of column (may have spaces).
		CDBValue::ETypes m_iType = CDBValue::typeUnknown;
		int m_iOrdinal = -1;				//	Column ordinal in table (-1 = unknown)
		DWORD m_dwExtra = 0;

		CString m_sDisplayName;				//	Display name of column
		CDBValue::ETypes m_iDisplayType;	//	Convert to this type on display or export
	};

class CDBTable
	{
	public:
		bool AddCol (const CDBColumnDef &ColDef);
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
		int GetRowCount (void) const { return (GetColCount() > 0 ? m_Rows.GetCount() / GetColCount() : 0); }
		void GrowToFit (int iRows) { m_Rows.GrowToFit(iRows * GetColCount()); }
		void SetColumnDefs (const TArray<CDBColumnDef> &Cols);
		bool SetField (int iCol, int iRow, const CDBValue &Value);
		void TakeHandoff (CDBTable &Src);

	private:
		void InitColIndex (void) const;
		void InvalidateColIndex (void) { m_bColIndexValid = false; }

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
			};

		static bool Load (IByteStream &Stream, const SOptions &Options, CDBTable &Table, CString *retsError = NULL);
	};
