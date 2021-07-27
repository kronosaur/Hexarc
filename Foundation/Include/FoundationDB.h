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
		CDBValue GetProperty (const CString &sProperty) const;
		ETypes GetType (void) const;
		bool IsBlank (void) const;
		bool IsNaN () const;
		bool IsNil (void) const { return m_dwData == 0; }
		void Push (const CDBValue &Value);
		void SetElement (const CString &sKey, const CDBValue &Value);

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
		virtual LONGLONG CastLONGLONG (void) const { return 0; }
		virtual IDBValueObject *Clone (void) const = 0;
		virtual const CDBValue &GetElement (int iIndex) const { return CDBValue::Null; }
		virtual const CDBValue &GetElement (const CString &sKey) const { return CDBValue::Null; }
		virtual int GetElementCount (void) const { return 1; }
		virtual const CString &GetElementKey (int iIndex) const { return NULL_STR; }
		virtual CDBValue GetProperty (const CString &sProperty) const { return GetElement(sProperty); }
		virtual CDBValue::ETypes GetType (void) const = 0;
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

class CDBFormatXLS
	{
	public:
		struct SOptions
			{
			TArray<int> ColOrder;			//	Column order
			TArray<int> SortOrder;			//	Sort by these columns
			int iSheetColumn = -1;			//	If set, create a new sheet per value of this column.
			TArray<int> SheetSortOrder;		//	Sort order for sheets
			CDBTable HeaderRows;			//	Optional header rows
			TArray<int> HeaderColOrder;		//	Column order for optional header rows
			};

		static bool ParseOptions (const CDBTable &Table, const CDBValue &Value, SOptions &retOptions, CString *retsError = NULL);
		static bool Write (IByteStream &Stream, const CDBTable &Table, const SOptions &Options);

	private:
		static CString GetDataValue (const CDBValue &Value);
		static CString GetDataType (CDBValue::ETypes iType);
		static CString GetSortKey (const CDBValue &Value);
		static TArray<int> SortRows (const CDBTable &Table, const TArray<int> &Rows, const TArray<int> &SortOrder);
		static CString MakeSortKey (const CDBTable &Table, const TArray<int> &SortOrder, int iRow, const CString &sDefault = NULL_STR);
		static bool WriteHeaderRow (IByteStream &Stream, const CString &sSheetName, const CDBTable &Table, const SOptions &Options);
		static bool WriteRow (IByteStream &Stream, const CDBTable &Table, int iRow, const TArray<int> &ColOrder, const SOptions &Options);
		static bool WriteSheet (IByteStream &Stream, const CString &sSheetName, const CDBTable &Table, const TArray<int> &Rows, const SOptions &Options);
	};

class CMSCompoundFileFormat
	{
	public:
		CMSCompoundFileFormat () { }

		void AddStream (const CString &sName, IMemoryBlock &Data);
		bool Write (IByteStream &Stream);

	private:
		static constexpr int DIFAT_IN_HEADER = 109;
		static constexpr int SECTOR_SIZE = 512;
		static constexpr int DIRECTORY_ENTRIES_PER_SECTOR = 4;
		static constexpr int FAT_ENTRIES_PER_SECTOR = SECTOR_SIZE / sizeof(DWORD);
		static constexpr DWORD ENDOFCHAIN = 0xFFFFFFFE;
		static constexpr DWORD FAT_SECTOR_ENTRY = 0xFFFFFFFD;
		static constexpr DWORD FREESECT = 0xFFFFFFFF;

		struct SStream
			{
			CString sName;
			IMemoryBlock *pData = NULL;

			int iSector = 0;
			int iSizeInSectors = 0;
			};

#pragma pack(push, 1)
		struct CFF_HEADER
			{
			CFF_HEADER ()
				{
				for (int i = 0; i < DIFAT_IN_HEADER; i++)
					DIFAT[i] = FREESECT;
				}

			BYTE Signature[8] = { 0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1 };
			CLSID ClsID = { 0 };
			WORD wMinorVersion = 0x3E;
			WORD wDLLVersion = 3;		//	512-byte sectors
			WORD wByteOrder = 0xFFFE;	//	Intel
			WORD wSectorShift = 9;		//	512-byte sectors (2^9).
			WORD wMiniSectorShift = 6;	//	64-byte minisectors (2^6)
			WORD wReserved1 = 0;
			DWORD dwReserved2 = 0;
			DWORD dwSectDir = 0;
			DWORD dwSectFAT = 0;
			DWORD dwSectDirStart = 0;
			DWORD dwTransSignature = 0;
			DWORD dwMiniSectorCutoff = 4096;
			DWORD dwSectMiniFATStart = ENDOFCHAIN;
			DWORD dwSectMiniFAT = 0;
			DWORD dwSectDIFATStart = ENDOFCHAIN;
			DWORD dwSectDIFAT = 0;
			DWORD DIFAT[DIFAT_IN_HEADER];
			};

		struct CFF_DIRECTORY
			{
			static constexpr BYTE TYPE_STORAGE = 0x01;
			static constexpr BYTE TYPE_STREAM = 0x02;
			static constexpr BYTE TYPE_ROOT = 0x05;

			TCHAR Name[32];
			WORD wNameLen = 0;
			BYTE byObjType = 0;
			BYTE byColor = 0x01;	//	Black
			DWORD dwLeftSibling = 0xFFFFFFFF;
			DWORD dwRightSibling = 0xFFFFFFFF;
			DWORD dwChildID = 0xFFFFFFFF;
			BYTE clsid[16] = { 0 };
			DWORD dwState = 0;
			DWORDLONG dwCreatedOn = 0;
			DWORDLONG dwModifiedOn = 0;
			DWORD dwStartSector = 0;
			DWORDLONG dwStreamSize = 0;
			};
#pragma pack(pop)

		void WriteDirectory (IByteStream &Stream, CFF_HEADER &Header, int iSector, int *retiSectorsWritten);
		void WriteFAT (IByteStream &Stream, CFF_HEADER &Header, int iSector, int iDirectorySectorSize);
		static void SetName (CFF_DIRECTORY &Entry, const CString &sName);

		TArray<SStream> m_Docs;
	};

class CDBFormatXLS97Sheet
	{
	public:
		void AddRow (const CDBTable &Table, int iRow, const TArray<int> &ColOrder);
		void AddRow (const TArray<CDBValue> &Row);
		int GetBlockCount () const { return m_Blocks.GetCount(); }
		const CDBValue &GetCell (int iBlock, int iRow, int iCol) const;
		const CString &GetName () const { return m_sSheetName; }
		int GetRowCount () const { return m_iRowCount; }
		int GetRowCount (int iBlock) const { if (iBlock < 0 || iBlock >= GetBlockCount()) throw CException(errFail); return m_Blocks[iBlock].Rows.GetCount(); }
		void SetName (const CString &sValue) { m_sSheetName = sValue; }

	private:
		static constexpr int RECORDS_PER_BLOCK = 32;

		struct SCell
			{
			int iCol = 0;					//	Column (0-based)
			const CDBValue *pValue = NULL;	//	Value
			};

		struct SRow
			{
			int iRow = 0;					//	Row number (0-based)
			TArray<SCell> Cells;
			};

		struct SRowBlock
			{
			int iBlock = 0;					//	Index of row block.
			TArray<SRow> Rows;				//	Rows in row block.
			};

		SRowBlock &GetCurrentBlock ();

		CString m_sSheetName;
		TArray<SRowBlock> m_Blocks;
		int m_iRowCount = 0;				//	Total row count on all blocks.
	};

class CDBFormatXLS97SST
	{
	public:
		void AddString (const CString &sValue);
		int GetCount () const { return m_SST.GetCount(); }
		int GetStringIndex (const CString &sValue) const;
		void SetOffset (int iIndex, int iOffset) { if (iIndex < 0 || iIndex >= m_SST.GetCount()) throw CException(errFail); m_SST[iIndex].iOffset = iOffset; }

	private:
		struct SEntry
			{
			CString sValue;
			int iOffset = 0;
			};

		int m_iTotalStringInstances = 0;
		TArray<SEntry> m_SST;
		TSortMap<CString, int> m_Index;
	};

class CDBFormatXLS97
	{
	public:
		static constexpr int MAX_RECORD_SIZE = 8228;

		struct SOptions
			{
			TArray<int> ColOrder;			//	Column order
			TArray<int> SortOrder;			//	Sort by these columns
			int iSheetColumn = -1;			//	If set, create a new sheet per value of this column.
			TArray<int> SheetSortOrder;		//	Sort order for sheets
			CDBTable HeaderRows;			//	Optional header rows
			TArray<int> HeaderColOrder;		//	Column order for optional header rows
			};

		CDBFormatXLS97 (IByteStream &Stream, const CDBTable &Table, const SOptions &Options) :
				m_Stream(Stream),
				m_Table(Table),
				m_Options(Options)
			{ }

		static bool ParseOptions (const CDBTable &Table, const CDBValue &Value, SOptions &retOptions, CString *retsError = NULL);
		bool Write ();

	private:
		static constexpr WORD SUBSTREAM_TYPE_GLOBALS = 0x0005;
		static constexpr WORD SUBSTREAM_TYPE_WORKSHEET = 0x0010;

		static constexpr WORD BIFF5_VERSION = 0x0500;
		static constexpr WORD BIFF8_VERSION = 0x0600;
		static constexpr WORD EXCEL97_BUILD_ID = 0x0DBB;
		static constexpr WORD EXCEL97_BUILD_YEAR = 1997;
		static constexpr WORD EXCEL2106_BUILD_ID = 0x4F5A;
		static constexpr DWORD EXCEL2106_FILE_HISTORY = 0x000200C1;
		static constexpr DWORD EXCEL2106_LOWEST_VERSION = 0x0000806;

		static constexpr WORD CODEPAGE_UTF16 = 0x04B0;

		enum class EFixup
			{
			None,

			Worksheet,
			DBCell,
			};

		struct SFixup
			{
			EFixup iType = EFixup::None;
			DWORD dwData = 0;
			int iOffset = 0;
			};

#pragma pack(push, 1)
		struct R_BOF
			{
			WORD wID = 0x0809;
			WORD wLen = sizeof(R_BOF) - sizeof(DWORD);
			WORD wVersion = BIFF8_VERSION;
			WORD wSubstream = SUBSTREAM_TYPE_WORKSHEET;
			WORD wBuild = EXCEL2106_BUILD_ID;			//	Build 2106 saved as 97 XLS
			WORD wYear = EXCEL97_BUILD_YEAR;			//	Saved as 97 XLS
			DWORD dwFileHistory = EXCEL2106_FILE_HISTORY;
			DWORD dwLowestVersion = EXCEL2106_LOWEST_VERSION;
			};

		struct R_BOUNDSHEET
			{
			WORD wID = 0x0085;
			WORD wLen = sizeof(R_BOUNDSHEET) - sizeof(DWORD);
			DWORD dwOffset = 0;
			WORD wOptions = 0;
			//	Sheet name string
			};

		struct R_DBCELL
			{
			WORD wID = 0x00D7;
			WORD wLen = sizeof(R_DBCELL) - sizeof(DWORD);
			DWORD dwRowOffset = 0;
			//	WORD cell offsets
			};

		struct R_DIMENSIONS
			{
			WORD wID = 0x0200;
			WORD wLen = sizeof(R_DIMENSIONS) - sizeof(DWORD);
			DWORD dwFirstRow = 0;
			DWORD dwLastRowPlus1 = 1;
			WORD wFirstCol = 0;
			WORD wLastColPlus1 = 1;
			WORD wReserved = 0;
			};

		struct R_EOF
			{
			WORD wID = 0x000A;
			WORD wLen = 0;
			};

		struct R_INDEX
			{
			WORD wID = 0x020B;
			WORD wLen = sizeof(R_INDEX) - sizeof(DWORD);
			DWORD dwReserved1 = 0;
			DWORD dwFirstRow = 0;
			DWORD dwLastRowPlus1 = 1;
			DWORD dwReserved2 = 0;
			//	Array of file offsets
			};

		struct R_INTERFACEHDR
			{
			WORD wID = 0x00E1;
			WORD wLen = sizeof(R_INTERFACEHDR) - sizeof(DWORD);
			WORD wCodePage = 0x04E4;
			};

		struct R_INTERFACEEND
			{
			WORD wID = CODEPAGE_UTF16;
			WORD wLen = 0;
			};

		struct R_LABEL
			{
			WORD wID = 0x0204;
			WORD wLen = sizeof(R_LABEL) - sizeof(DWORD);
			WORD wRow = 0;
			WORD wCol = 0;
			WORD wXF = 0;
			//	Cell value
			};

		struct R_MMS
			{
			WORD wID = 0x00C1;
			WORD wLen = sizeof(R_MMS) - sizeof(DWORD);
			BYTE byAddMenuGroups = 0;
			BYTE byDelMenuGroups = 0;
			};

		struct R_ROW
			{
			WORD wID = 0x0208;
			WORD wLen = sizeof(R_ROW) - sizeof(DWORD);
			WORD wRow = 0;
			WORD wFirstCol = 0;
			WORD wLastColPlus1 = 1;
			WORD wRowHeight = 9;
			WORD wReserved1 = 0;
			WORD wReserved2 = 0;
			WORD wOptions = 0;
			WORD wXFE = 0;
			};

		struct R_FRT
			{
			WORD wID = 0xA000;
			WORD wLen = sizeof(R_FRT) - sizeof(DWORD);
			WORD wRT = 0xA000;
			WORD wFlags = 0;
			};

#pragma pack(pop)

		void AddHeader (CDBFormatXLS97Sheet &Blocks, const CString &sSheetName);
		static CBuffer Encode (const CString &sString, int iLenFieldSize = 2);
		static CString GetDataValue (const CDBValue &Value);
		static CString GetSortKey (const CDBValue &Value);
		void InitData ();
		void InitSheetData (CDBFormatXLS97Sheet &SheetData, const CString &sSheetName, const TArray<int> &Rows);
		CString MakeSortKey (const TArray<int> &SortOrder, int iRow, const CString &sDefault = NULL_STR);
		void RequestFixup (EFixup iType, int iOffset, DWORD dwData);
		void ResolveFixup (EFixup iType, DWORD dwData, int iResolvedOffset);
		TArray<int> SortRows (const TArray<int> &Rows, const TArray<int> &SortOrder);
		bool WriteCell (const CDBValue &Value, int iRow, int iCol);
		bool WriteSheet (const CDBFormatXLS97Sheet &Sheet, int iSheetIndex);
		bool WriteWorkbook (const TSortMap<CString, TArray<int>> &Sheets, const TSortMap<CString, int> &Order);
		void WriteWorkbookDefinitions ();

		void WriteBOF (WORD wSubstream = SUBSTREAM_TYPE_WORKSHEET);
		void WriteDBCELL (int iRowOffset, const TArray<int> &CellOffsets);
		void WriteEOF ();
		void WriteBOUNDSHEET (const CString &sSheetName, int iSheetIndex);
		void WriteDIMENSIONS (int iRowCount, int iColCount);
		void WriteINDEX (int iRowCount, int iBlockCount);
		void WriteINTERFACEHDR ();
		void WriteLABEL (const CString &sValue, int iRow, int iCol);
		void WriteROW (int iRowNumber, int iColCount);
		void WriteFiller (int iBytes);

		IByteStream &m_Stream;
		const CDBTable &m_Table;
		const SOptions &m_Options;

		TArray<CDBValue> m_Header;			//	Default header
		TArray<CDBFormatXLS97Sheet> m_Data;
		TArray<SFixup> m_Fixups;			//	List of fixups needed
		CDBFormatXLS97SST m_SST;			//	Shared String Table
	};
