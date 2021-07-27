//	FoundationXLS.h
//
//	Foundation header file
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

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
		int GetInstanceCount () const { return m_iTotalStringInstances; }
		const CString &GetString (int iIndex) const { if (iIndex < 0 || iIndex >= m_SST.GetCount()) throw CException(errFail); return m_SST[iIndex].sValue; }
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

		static constexpr WORD DEFAULT_ROW_HEIGHT = 300;
		static constexpr WORD DEFAULT_TAB_RATIO = 600;

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

		struct R_CONTINUE
			{
			WORD wID = 0x003C;
			WORD wLen = 0;
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
			WORD wCodePage = CODEPAGE_UTF16;
			};

		struct R_INTERFACEEND
			{
			WORD wID = 0x00E2;
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

		struct R_LABELSST
			{
			WORD wID = 0x00FD;
			WORD wLen = sizeof(R_LABELSST) - sizeof(DWORD);
			WORD wRow = 0;
			WORD wCol = 0;
			WORD wXF = 0;
			DWORD dwIndex = 0;
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
			WORD wRowHeight = DEFAULT_ROW_HEIGHT;
			WORD wReserved1 = 0;
			WORD wReserved2 = 0;
			WORD wOptions = 0;
			WORD wXFE = 0;
			};

		struct R_SST
			{
			WORD wID = 0x00FC;
			WORD wLen = 0;
			DWORD dwTotalStrings = 0;
			DWORD dwUniqueStrings = 0;
			};

		struct R_WINDOW1
			{
			WORD wID = 0x003D;
			WORD wLen = sizeof(R_WINDOW1) - sizeof(DWORD);
			WORD xWin = 0;
			WORD yWin = 0;
			WORD dxWin = 0;
			WORD dyWin = 0;
			WORD wOptions = 0x0038;	//	fDspHScroll | fDspVScroll | fBotAdornment
			WORD wCurSheet = 0;
			WORD wFirstSheetShow = 0;
			WORD wSheetsSelected = 1;
			WORD wTabRatio = DEFAULT_TAB_RATIO;
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
		void AddToSST (const CDBValue &Value);
		static CBuffer Encode (const CString &sString, int iLenFieldSize = 2);
		static CString GetDataValue (const CDBValue &Value);
		static CString GetSortKey (const CDBValue &Value);
		void InitData ();
		void InitRowData (CDBFormatXLS97Sheet &SheetData, const CDBTable &Table, int iRow, const TArray<int> ColOrder);
		void InitRowData (CDBFormatXLS97Sheet &SheetData, const TArray<CDBValue> &Row);
		void InitSheetData (CDBFormatXLS97Sheet &SheetData, const CString &sSheetName, const TArray<int> &Rows);
		CString MakeSortKey (const TArray<int> &SortOrder, int iRow, const CString &sDefault = NULL_STR);
		void RequestFixup (EFixup iType, int iOffset, DWORD dwData);
		void ResolveFixup (EFixup iType, DWORD dwData, int iResolvedOffset);
		TArray<int> SortRows (const TArray<int> &Rows, const TArray<int> &SortOrder);
		bool WriteCell (const CDBValue &Value, int iRow, int iCol);
		bool WriteSheet (const CDBFormatXLS97Sheet &Sheet, int iSheetIndex);
		void WriteSST ();
		void WriteWorkbookDefinitions ();

		void WriteBOF (WORD wSubstream = SUBSTREAM_TYPE_WORKSHEET);
		void WriteDBCELL (int iRowOffset, const TArray<int> &CellOffsets);
		void WriteEOF ();
		void WriteBOUNDSHEET (const CString &sSheetName, int iSheetIndex);
		void WriteDIMENSIONS (int iRowCount, int iColCount);
		void WriteINDEX (int iRowCount, int iBlockCount);
		void WriteINTERFACEHDR ();
		void WriteLABEL (const CString &sValue, int iRow, int iCol);
		void WriteLABELSST (int iIndex, int iFormat, int iRow, int iCol);
		void WriteROW (int iRowNumber, int iColCount);
		void WriteWINDOW1 ();
		void WriteFiller (int iBytes);

		IByteStream &m_Stream;
		const CDBTable &m_Table;
		const SOptions &m_Options;

		TArray<CDBValue> m_Header;			//	Default header
		TArray<CDBFormatXLS97Sheet> m_Data;
		TArray<SFixup> m_Fixups;			//	List of fixups needed
		CDBFormatXLS97SST m_SST;			//	Shared String Table
	};

