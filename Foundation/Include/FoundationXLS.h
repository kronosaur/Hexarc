//	FoundationXLS.h
//
//	Foundation header file
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

#include <initializer_list>

class CXLSColorPalette
	{
	public:
		static constexpr int DEFAULT_BLACK =		0;
		static constexpr int DEFAULT_WHITE =		1;
		static constexpr int DEFAULT_RED =			2;
		static constexpr int DEFAULT_GREEN =		3;
		static constexpr int DEFAULT_BLUE =			4;
		static constexpr int DEFAULT_YELLOW =		5;
		static constexpr int DEFAULT_MAGENTA =		6;
		static constexpr int DEFAULT_CYAN =			7;

		CXLSColorPalette ();

		int GetCount () const { return m_Colors.GetCount(); }
		const CRGBA32 &GetColor (int iIndex) const { if (iIndex < 0 || iIndex >= m_Colors.GetCount()) throw CException(errFail); return m_Colors[iIndex]; }
		int FindColor (const CRGBA32 &Color) const;

	private:
		TArray<CRGBA32> m_Colors;

		static std::initializer_list<CRGBA32> m_Defaults;
	};

class CXLSFontTable
	{
	public:
		static constexpr int DEFAULT_FONT_ID =	0;

		static constexpr int POINT_SIZE_SCALE = 20;
		static constexpr int DEFAULT_HEIGHT_IN_POINTS = 10;
		static constexpr int POINTS_11 = 11 * POINT_SIZE_SCALE;

		static constexpr int WEIGHT_DEFAULT = -1;
		static constexpr int WEIGHT_NORMAL = 400;
		static constexpr int WEIGHT_BOLD = 700;

		enum class EFamily
			{
			Unknown =			0x00,
			Roman =				0x01,
			Swiss =				0x02,
			Modern =			0x03,
			Script =			0x04,
			Decorative =		0x05,
			};

		enum class ECharacterSet
			{
			Default =			0xFF,

			ANSILatin =			0x00,
			SystemDefault =		0x01,
			Symbol =			0x02,
			};

		enum class EEscapement
			{
			Default =			0xFF,

			None =				0x00,
			Superscript =		0x01,
			Subscript =			0x02,
			};

		enum class EItalic
			{
			Default =			0xFF,

			None =				0x00,
			Italic =			0x01,
			};

		enum class EStrikeout
			{
			Default =			0xFF,

			None =				0x00,
			Strikeout =			0x01,
			};

		enum class EUnderline
			{
			Default =			0xFF,

			None =				0x00,
			Single =			0x01,
			Double =			0x02,
			SingleAccounting =	0x21,
			DoubleAccounting =	0x22,
			};

		struct SFont
			{
			EFamily iFamily = EFamily::Unknown;							//	Unknown == default
			CString sName;												//	NULL_STR == default
			int iSize = 0;												//	Size in 1/20th of a point (0 == default)
			int iWeight = WEIGHT_DEFAULT;
			EItalic iItalic = EItalic::Default;
			EStrikeout iStrikeout = EStrikeout::Default;
			EUnderline iUnderline = EUnderline::Default;
			EEscapement iEscapement = EEscapement::Default;
			CRGBA32 Color = CRGBA32::Null();							//	Default
			ECharacterSet iCharacterSet = ECharacterSet::Default;
			};

		CXLSFontTable ();

		int GetCount () const { return m_Table.GetCount(); }
		const SFont &GetFont (int iIndex) const { if (iIndex < 0 || iIndex >= m_Table.GetCount()) throw CException(errFail); return m_Table[iIndex]; }
		int GetFont (const CDBFormatDesc &Desc);

	private:
		SFont Create (const CDBFormatDesc &Desc);
		static bool Equals (const SFont &Src, const SFont &Dest);

		TArray<SFont> m_Table;

		static std::initializer_list<SFont> m_Defaults;
	};

class CXLSXFTable
	{
	public:
		static constexpr int FORMAT_GENERAL =			0x00;
		static constexpr int FORMAT_NUMBER_INT =		0x01;
		static constexpr int FORMAT_NUMBER_DEC2 =		0x02;
		static constexpr int FORMAT_NUMBER_DEC2_COMMA =	0x04;

		static constexpr int FORMAT_DATE_M_D_YY =		0x0E;
		static constexpr int FORMAT_DATE_D_MMM_YY =		0x0F;
		static constexpr int FORMAT_DATE_D_MMM =		0x10;
		static constexpr int FORMAT_DATE_MMM_YY =		0x11;

		static constexpr int DEFAULT_CELL_XF =			15;

		static constexpr int NO_PARENT = -1;

		enum class EAlign
			{
			General =			0x00,
			Left =				0x01,
			Center =			0x02,
			Right =				0x03,
			Fill =				0x04,
			Justify =			0x05,
			CenterAcross =		0x06,
			};

		enum class EVerticalAlign
			{
			Top =				0x00,
			Center =			0x01,
			Bottom =			0x02,
			Justify =			0x03,
			};

		enum class EReadOrder
			{
			Context =			0x00,
			LeftToRight =		0x01,
			RightToLeft =		0x02,
			};

		enum class ELineStyle
			{
			None =				0x00,
			Thin =				0x01,
			Medium =			0x02,
			Dashed =			0x03,
			Dotted =			0x04,
			Thick =				0x05,
			Double =			0x06,
			Hair =				0x07,
			MediumDashed =		0x08,
			DashDot =			0x09,
			MediumDashDot =		0x0A,
			DashDotDot =		0x0B,
			MediumDashDotDot =	0x0C,
			SlantedDashDot =	0x0D,
			};

		enum class EDiagonal
			{
			None =				0x00,
			Down =				0x01,
			Up =				0x02,
			Both =				0x03,
			};

		enum class EPattern
			{
			None =				0x00,
			Solid =				0x01,
			Checker =			0x02,
			Pattern3 =			0x03,
			Pattern4 =			0x04,
			ThickHorzLines =	0x05,
			ThickVertLines =	0x06,
			Pattern7 =			0x07,
			Pattern8 =			0x08,
			ThickChecker =		0x09,
			Pattern10 =			0x0A,
			HorzLines =			0x0B,
			VertLines =			0x0C,
			Pattern13 =			0x0D,
			Pattern14 =			0x0E,
			Grid =				0x0F,
			Pattern16 =			0x10,
			Pattern17 =			0x11,
			Pattern18 =			0x12,
			};

		struct SBorder
			{
			ELineStyle iStyle = ELineStyle::None;
			int iColor = CXLSColorPalette::DEFAULT_BLACK;
			};

		static constexpr DWORD FLAG_LOCKED =			0x00000001;
		static constexpr DWORD FLAG_HIDDEN =			0x00000002;
		static constexpr DWORD FLAG_STYLE =				0x00000004;
		static constexpr DWORD FLAG_123PREFIX =			0x00000008;
		static constexpr DWORD FLAG_WRAP_TEXT =			0x00000010;
		static constexpr DWORD FLAG_JUST_LAST =			0x00000020;
		static constexpr DWORD FLAG_SHRINK_TO_FIT =		0x00000040;
		static constexpr DWORD FLAG_MERGE_CELL =		0x00000080;
		static constexpr DWORD FLAG_ATR_NUM =			0x00000100;
		static constexpr DWORD FLAG_ATR_FNT =			0x00000200;
		static constexpr DWORD FLAG_ATR_ALC =			0x00000400;
		static constexpr DWORD FLAG_ATR_BDR =			0x00000800;
		static constexpr DWORD FLAG_ATR_PAT =			0x00001000;
		static constexpr DWORD FLAG_ATR_PROT =			0x00002000;
		static constexpr DWORD FLAG_HAS_XFEXT =			0x00004000;
		static constexpr DWORD FLAG_SX_BUTTON =			0x00008000;

		struct SXF
			{
			int iFont = 0;					//	Index to font in table
			int iFormat = 0;				//	Index to format
			int iParentXF = 0;
			EAlign iAlign = EAlign::General;
			EVerticalAlign iVerticalAlign = EVerticalAlign::Top;
			int iRotation = 0;
			int iIndentLevel = 0;
			EReadOrder iReadOrder = EReadOrder::Context;

			SBorder LeftBorder;
			SBorder RightBorder;
			SBorder TopBorder;
			SBorder BottomBorder;

			EDiagonal iDiagonal = EDiagonal::None;
			SBorder DiagonalBorder;

			EPattern iFillPattern = EPattern::None;
			int iPatternColor = CXLSColorPalette::DEFAULT_BLACK;
			int iPatternBackColor = CXLSColorPalette::DEFAULT_WHITE;

			DWORD dwFlags = 0;
			};

		CXLSXFTable ();

		int GetCount () const { return m_Table.GetCount(); }
		const CXLSFontTable &GetFonts () const { return m_Fonts; }
		const CXLSColorPalette &GetPalette () const { return m_Palette; }
		int GetXF (int iFormat, const CDBFormatDesc &Desc);
		const SXF &GetXF (int iIndex) const { if (iIndex < 0 || iIndex >= m_Table.GetCount()) throw CException(errFail); return m_Table[iIndex]; }

	private:
		SXF CreateXF (int iFormat, const CDBFormatDesc &Desc);

		CXLSColorPalette m_Palette;
		CXLSFontTable m_Fonts;
		TArray<SXF> m_Table;
		TSortMap<SXF, int> m_Index;

		static std::initializer_list<SXF> m_Defaults;
	};

int KeyCompare (const CXLSXFTable::SXF &Key1, const CXLSXFTable::SXF &Key2);
int KeyCompare (const CXLSXFTable::SBorder &Key1, const CXLSXFTable::SBorder &Key2);

class CDBFormatXLS97Sheet
	{
	public:
		struct SRowAddr
			{
			int iBlock = -1;
			int iRowIndex = -1;				//	Index in block
			};

		SRowAddr AddRow (const CDBTable &Table, int iRow, const TArray<int> &ColOrder);
		SRowAddr AddRow (const TArray<CDBValue> &Row);
		int GetBlockCount () const { return m_Blocks.GetCount(); }
		const CDBValue &GetCell (int iBlock, int iRow, int iCol, int *retiXF = NULL) const;
		const CString &GetName () const { return m_sSheetName; }
		int GetRowCount () const { return m_iRowCount; }
		int GetRowCount (int iBlock) const { if (iBlock < 0 || iBlock >= GetBlockCount()) throw CException(errFail); return m_Blocks[iBlock].Rows.GetCount(); }
		void SetCellXF (const SRowAddr &Row, int iCol, int iXF) { GetCell(Row, iCol).iXF = iXF; }
		void SetName (const CString &sValue) { m_sSheetName = sValue; }

	private:
		static constexpr int RECORDS_PER_BLOCK = 32;

		struct SCell
			{
			int iCol = 0;					//	Column (0-based)
			int iXF = 0;
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
		SCell &GetCell (const SRowAddr &Row, int iCol);

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
		struct R_BLANK
			{
			WORD wID = 0x0201;
			WORD wLen = sizeof(R_BLANK) - sizeof(DWORD);
			WORD wRow = 0;
			WORD wCol = 0;
			WORD wXF = 0;
			};

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

		struct R_FONT
			{
			WORD wID = 0x0031;
			WORD wLen = sizeof(R_FONT) - sizeof(DWORD);
			WORD wHeight = 0;
			WORD wOptions = 0;
			WORD wColor = 0;
			WORD wWeight = 0;
			WORD wExcapement = 0;
			BYTE byUnderline = 0;
			BYTE byFamily = 0;
			BYTE byCharacterSet = 0;
			BYTE byUnused1 = 0;
			BYTE byNameLen = 0;
			BYTE byNameCharSet = 1;
			//	Unicode font name
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

		struct R_NUMBER
			{
			WORD wID = 0x0203;
			WORD wLen = sizeof(R_NUMBER) - sizeof(DWORD);
			WORD wRow = 0;
			WORD wCol = 0;
			WORD wXF = 0;
			double rValue = 0.0;
			};

		struct R_PALETTE
			{
			WORD wID = 0x0092;
			WORD wLen = sizeof(R_PALETTE) - sizeof(DWORD);
			WORD wCount = 0;
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

		struct R_XF
			{
			WORD wID = 0x00E0;
			WORD wLen = sizeof(R_XF) - sizeof(DWORD);
			WORD wFont = 0;
			WORD wFormat = 0;
			WORD wParent = 0;
			BYTE byAlign = 0;
			BYTE byRotation = 0;
			BYTE byIndent = 0;
			BYTE byUsed = 0;
			WORD wBorder1 = 0;
			WORD wBorder2 = 0;
			DWORD dwBorder3 = 0;
			WORD wPattern = 0;
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
		static double ConvertDate (const CDateTime &Date);
		static CBuffer Encode (const CString &sString, int iLenFieldSize = 2);
		static CString GetDataValue (const CDBValue &Value);
		static CString GetSortKey (const CDBValue &Value);
		void InitCellSST (const CDBValue &Value);
		int InitCellXF (const CDBValue &Value);
		void InitData ();
		void InitRowData (CDBFormatXLS97Sheet &SheetData, const CDBTable &Table, int iRow, const TArray<int> ColOrder);
		void InitRowData (CDBFormatXLS97Sheet &SheetData, const TArray<CDBValue> &Row);
		void InitSheetData (CDBFormatXLS97Sheet &SheetData, const CString &sSheetName, const TArray<int> &Rows);
		CString MakeSortKey (const TArray<int> &SortOrder, int iRow, const CString &sDefault = NULL_STR);
		void RequestFixup (EFixup iType, int iOffset, DWORD dwData);
		void ResolveFixup (EFixup iType, DWORD dwData, int iResolvedOffset);
		TArray<int> SortRows (const TArray<int> &Rows, const TArray<int> &SortOrder);
		bool WriteCell (const CDBValue &Value, int iRow, int iCol, int iXF);
		void WriteFontTable ();
		bool WriteSheet (const CDBFormatXLS97Sheet &Sheet, int iSheetIndex);
		void WriteSST ();
		void WriteXFTable ();

		void WriteBLANK (int iXF, int iRow, int iCol);
		void WriteBOF (WORD wSubstream = SUBSTREAM_TYPE_WORKSHEET);
		void WriteBOUNDSHEET (const CString &sSheetName, int iSheetIndex);
		void WriteDBCELL (int iRowOffset, const TArray<int> &CellOffsets);
		void WriteDIMENSIONS (int iRowCount, int iColCount);
		void WriteEOF ();
		void WriteFONT (const CXLSFontTable::SFont &Font);
		void WriteINDEX (int iRowCount, int iBlockCount);
		void WriteINTERFACEHDR ();
		void WriteLABEL (const CString &sValue, int iRow, int iCol);
		void WriteLABELSST (int iIndex, int iXF, int iRow, int iCol);
		void WriteNUMBER (double rValue, int iXF, int iRow, int iCol);
		void WritePALETTE (const CXLSColorPalette &Palette);
		void WriteROW (int iRowNumber, int iColCount);
		void WriteWINDOW1 ();
		void WriteXF (const CXLSXFTable::SXF &XF);
		void WriteFiller (int iBytes);

		IByteStream &m_Stream;
		const CDBTable &m_Table;
		const SOptions &m_Options;

		TArray<CDBValue> m_Header;			//	Default header
		TArray<CDBFormatXLS97Sheet> m_Data;
		TArray<SFixup> m_Fixups;			//	List of fixups needed
		CDBFormatXLS97SST m_SST;			//	Shared String Table
		CXLSXFTable m_XFTable;				//	XF table
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

