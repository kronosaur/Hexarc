//	XLSUtil.h
//
//	XLSUtil Classes
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "Foundation.h"
#include "AEON.h"

class CXLSLoadCtx;

enum class EXLSValueType
	{
	//	NOTE: These values are persisted and must not be changed.

	None		= 0,

	Auto		= 1,
	Bool		= 2,
	Number		= 3,
	DateTime	= 4,
	String		= 5,
	Error		= 6,
	};

struct SXLSCellFill
	{
	int iID = 0;					//	Index into m_Fills (0-based)
	CString sPatternType;			//	"none", "solid", "darkGray", etc.
	CRGBA32 rgbColor =				//	Pattern foreground color
		CRGBA32(0, 0, 0, 255);
	CRGBA32 rgbBackground =			//	Pattern background color
		CRGBA32(255, 255, 255, 255);

	int iRefCount = 0;				//	Number of formats referencing this fill
	};

struct SXLSCellFormat
	{
	int iNumFmtID = 0;				//	Number format
	int iFillID = -1;				//	Index into m_Fills (0-based)

	int iRefCount = 0;				//	Number of cells referencing this format
	};

struct SXLSCellRef
	{
	int iRow = 0;					//	1-based row
	int iCol = 0;					//	1-based column
	};

class CXLSRange
	{
	public:

		CXLSRange (int iSheet = 0, const SXLSCellRef& Start = SXLSCellRef(), const SXLSCellRef& End = SXLSCellRef()) :
				m_iSheet(iSheet),
				m_Start(Start),
				m_End(End)
			{ }

		static bool CreateFromDatum (CDatum dValue, CXLSRange& retRange);
		static CXLSRange CreateFromStream (IByteStream& Stream);
		static bool CreateFromString (const CString& sValue, CXLSRange& retRange);
		static bool IsValid (const SXLSCellRef& Addr) { return Addr.iRow >= 1 && Addr.iRow <= CXLSRange::MAX_ROWS && Addr.iCol >= 1 && Addr.iCol <= CXLSRange::MAX_COLS; }

		static SXLSCellRef CreateCellAddrFromDatum (CDatum dValue);

		size_t CalcSerializeSize () const { return sizeof(DWORD) + sizeof(m_Start) + sizeof(m_End); }
		void Extend (const SXLSCellRef& Addr);
		void ExtendCol (int iCol);
		static CString GetCellName (int iRow, int iCol);
		CXLSRange GetCol (int iIndex) const;
		int GetColCount () const { return (m_Start.iCol == 0 ? 0 : 1 + (m_End.iCol - m_Start.iCol)); }
		static CString GetColName (int iCol);
		const SXLSCellRef& GetEnd () const { return m_End; }
		CString GetRangeName () const { return strPattern("%s:%s", GetCellName(m_Start.iRow, m_Start.iCol), GetCellName(m_End.iRow, m_End.iCol)); }
		int GetRowCount () const { return (m_Start.iCol == 0 ? 0 : 1 + (m_End.iRow - m_Start.iRow)); }
		int GetSheet () const { return m_iSheet; }
		const SXLSCellRef& GetStart () const { return m_Start; }
		bool IsEmpty () const { return m_Start.iRow == 0 || m_End.iRow == 0; }
		bool IsValid () const { return IsEmpty() || (m_Start.iRow > 0 && m_End.iRow >= m_Start.iRow && m_End.iRow <= CXLSRange::MAX_ROWS && m_Start.iCol > 0 && m_End.iCol >= m_Start.iCol && m_End.iCol <= CXLSRange::MAX_COLS); }
		bool IsSingleCell () const { return m_Start.iRow == m_End.iRow && m_Start.iCol == m_End.iCol && m_Start.iCol != 0; }
		void SetEnd (const SXLSCellRef& End) { m_End = End; }
		void SetEndCol (int iCol) { m_End.iCol = iCol; }
		void SetEndRow (int iRow) { m_End.iRow = iRow; }
		void SetStart (const SXLSCellRef& Start) { m_Start = Start; }
		void SetStartCol (int iCol) { m_Start.iCol = iCol; }
		void SetStartRow (int iRow) { m_Start.iRow = iRow; }
		void WriteToStream (IByteStream& Stream) const;

		static constexpr int MAX_COLS = 16384;		//	XLSX limit
		static constexpr int MAX_ROWS = 1048576;	//	XLSX limit

	private:

		static SXLSCellRef ParseCellAddr (const char *pPos, const char **reptPos = NULL);
		static int ParseSheetIndex (const CString& sValue);

		int m_iSheet = 0;					//	Index into m_Sheets (0-based)
		SXLSCellRef m_Start;
		SXLSCellRef m_End;
	};

class CXLSStyles
	{
	public:

		CXLSStyles () { InitDefaultStyles(); }

		static CXLSStyles CreateFromStream (IByteStream& Stream, DWORD dwVersion);

		int AddReference (int iFormat) { if (iFormat >= 0 && iFormat < m_Formats.GetCount()) { m_Formats[iFormat].iRefCount++; return iFormat; } else return -1; }
		static CDatum AsDatum (const SXLSCellFill& Fill);
		int Compare (const CXLSStyles& Src) const;
		bool DebugValidateRefCounts (const TSortMap<int, int>& RefCounts) const;
		int GetFill (const SXLSCellFill& Fill);
		const SXLSCellFill& GetFill (int iID) const { if (iID >= 0 && iID < m_Fills.GetCount()) return m_Fills[iID]; else return m_DefaultFill; }
		int GetFormat (const SXLSCellFormat& Format);
		const SXLSCellFormat& GetFormat (int iID) const { if (iID >= 0 && iID < m_Formats.GetCount()) return m_Formats[iID]; else return m_DefaultFormat; }
		int GetNumFmt (CStringView sNumFmt);
		bool Init (CXLSLoadCtx& Load, const CXMLElement& StylesXML, CString* retsError = NULL);
		bool IsDateFormat (int iNumFmt) const;
		int MapNumFmt (CStringView sFormat);
		static bool ParseCellFillFromDatum (CDatum dValue, SXLSCellFill& retFill);
		void ReleaseReference (int iFormat) { if (iFormat >= 0 && iFormat < m_Formats.GetCount()) m_Formats[iFormat].iRefCount--; }
		int SetFormat (const SXLSCellFormat& Format, int iOldFormat = -1);
		void WriteToStream (IByteStream& Stream) const;
		void WriteToXML (IByteStream& Stream) const;

		static bool Equals (const SXLSCellFill& Value1, const SXLSCellFill& Value2);
		static bool Equals (const SXLSCellFormat& Format1, const SXLSCellFormat& Format2);
		static bool ParseFormatCode (CStringView sCode, CString* retsFormatCode = NULL, CString* retsLocale = NULL, CString* retsSuffix = NULL);

		static constexpr int NUMFMT_DEFAULT =			0;
		static constexpr int NUMFMT_0 =					1;
		static constexpr int NUMFMT_0_00 =				2;
		static constexpr int NUMFMT_0_COMMA =			3;
		static constexpr int NUMFMT_0_00_COMMA =		4;
		static constexpr int NUMFMT_0_PERCENT =			9;
		static constexpr int NUMFMT_0_00_PERCENT =		10;
		static constexpr int NUMFMT_MM_DD_YY =			14;
		static constexpr int NUMFMT_D_MMM_YY =			15;
		static constexpr int NUMFMT_DATE_TIME =			22;

		static constexpr int NUMFMT_FIRST_CUSTOM =		164;	//	First custom format

	private:

		struct SNumFmt
			{
			CString sFormatCode;
			bool bIsDate = false;

			int iRefCount = 0;				//	Number of formats referencing this format
			};

		void AddRef (SXLSCellFormat& Format);
		int CountCustomNumFmts () const;
		bool DebugValidateFillRefCounts () const;
		void InitDefaultStyles ();
		static bool IsDateFormat (CStringView sCode);
		void Release (SXLSCellFormat& Format);
		void WriteToXML (IByteStream& Stream, const SXLSCellFormat& Format) const;
		void WriteToXML (IByteStream& Stream, const SXLSCellFill& Fill) const;

		TSortMap<int, SNumFmt> m_NumFmts;
		TArray<SXLSCellFormat> m_Formats;
		TArray<SXLSCellFill> m_Fills;

		static SXLSCellFormat m_DefaultFormat;
		static SXLSCellFill m_DefaultFill;
	};

class CXLSLoadCtx
	{
	public:

		enum class EFileFormat
			{
			Unknown,						//	Not a known Excel format.

			OfficeOpenXML,
			};

		CXLSLoadCtx (CXLSStyles& Styles);

		static EFileFormat Detect (const IMemoryBlock& FileData);
		bool Open (const IMemoryBlock& FileData, CString* retsError = NULL);

		const CXMLElement* FindFileByRelID (const CString& sRelID) const;
		const CXMLElement& GetFileByRelID (const CString& sRelID) const;
		const CString& GetString (int iID) const { if (iID >= 0 && iID < m_Strings.GetCount()) return m_Strings[iID]; else return NULL_STR; }
		const SXLSCellFormat& GetStyle (int iID) const { return m_Styles.GetFormat(iID); }
		CXLSStyles& GetStyles () { return m_Styles; }
		const CXLSStyles& GetStyles () const { return m_Styles; }
		const CXMLElement& GetWorkbook () const;

		static CDateTime AsDateTime (double rValue) { return CDateTimeParser::FromExcel(rValue); }
		CString LoadStringTableEntry (const CXMLElement& StringXML);
		static bool ParseCellAddress (const CString& sAddr, int* retiRow = NULL, int* retiCol = NULL);

		SXMLNameID ATTRIB_APPLY_FILL;
		SXMLNameID ATTRIB_APPLY_NUMBER_FORMAT;
		SXMLNameID ATTRIB_FILL_ID;
		SXMLNameID ATTRIB_FORMAT_CODE;
		SXMLNameID ATTRIB_ID;
		SXMLNameID ATTRIB_NUM_FMT_ID;
		SXMLNameID ATTRIB_PATTERN_TYPE;
		SXMLNameID ATTRIB_R;
		SXMLNameID ATTRIB_R_ID;
		SXMLNameID ATTRIB_RGB;
		SXMLNameID ATTRIB_S;
		SXMLNameID ATTRIB_T;
		SXMLNameID ATTRIB_TARGET;
		SXMLNameID ATTRIB_TYPE;

		SXMLNameID ELEMENT_CELL_XFS;
		SXMLNameID ELEMENT_FILL;
		SXMLNameID ELEMENT_FILLS;
		SXMLNameID ELEMENT_FG_COLOR;
		SXMLNameID ELEMENT_IS;
		SXMLNameID ELEMENT_PATTERN_FILL;
		SXMLNameID ELEMENT_NUM_FMT;
		SXMLNameID ELEMENT_NUM_FMTS;
		SXMLNameID ELEMENT_ROW;
		SXMLNameID ELEMENT_SHEET_DATA;
		SXMLNameID ELEMENT_T;
		SXMLNameID ELEMENT_V;

	private:

		void CleanUp ();
		bool LoadStringTable (const CString& sRelID, CString* retsError = NULL);
		bool LoadStylesTable (const CString& sRelID, CString* retsError = NULL);
		TUniquePtr<const CXMLElement> LoadXMLFile (CZipFormatReader& ZipFile, const CString& sFilespec, CString* retsError = NULL);
		CString ResolvePath (const CString& sPath) const;

		const IMemoryBlock* m_pFileData = NULL;
		CZipFormatReader m_ZipFile;
		CXMLStore m_XMLStore;

		TUniquePtr<const CXMLElement> m_pWorkbook;
		TSortMap<CString, TUniquePtr<const CXMLElement>> m_FilesByRelID;
		TArray<CString> m_Strings;
		CXLSStyles& m_Styles;
	};

class CXLSWorksheet
	{
	public:

		struct SCellView
			{
			SCellView () { }
			SCellView (int iRowArg, int iColArg, EXLSValueType iTypeArg, CStringView sValueArg, int iFormatArg) :
					iRow(iRowArg), iCol(iColArg), iType(iTypeArg), sValue(sValueArg), iFormat(iFormatArg)
				{ }

			int iRow = -1;
			int iCol = -1;
			EXLSValueType iType = EXLSValueType::None;
			CStringView sValue;
			int iFormat = -1;				//	Index into m_Formats (0-based)
			};

		struct SRowInfo
			{
			int iRow = 0;					//	1-based row
			int iCellCount = 0;				//	Number of cells in row
			int iColStart = 0;				//	1-based col of first cell in row
			int iColEnd = 0;				//	1-based col of last cell in row
			};

		CXLSWorksheet (CXLSStyles& Styles, int iID = 0, CStringView sName = NULL_STR) :
				m_pStyles(&Styles),
				m_iID(iID),
				m_sName(sName)
			{ }

		static TUniquePtr<CXLSWorksheet> Create (CXLSLoadCtx& Load, int iSheet, const CString& sRelID, const CString& sName, CString* retsError = NULL);
		static TUniquePtr<CXLSWorksheet> CreateEmpty (CXLSStyles& Styles, int iSheet, const CString& sName);
		static TUniquePtr<CXLSWorksheet> CreateEmpty (CXLSStyles& Styles, int iSheet, CDatum dInfo);
		static TUniquePtr<CXLSWorksheet> CreateFromStream (CXLSStyles& Styles, IByteStream& Stream, DWORD dwVersion, int iSheet);

		TArray<CDatum> CalcAEONDatatypes (const CXLSRange& Range) const;
		size_t CalcSerializeSize () const;
		void Clear ();
		int Compare (const CXLSWorksheet& Other) const;
		void DebugAccumulateRefCounts (TSortMap<int, int>& RefCounts) const;
		bool FindTableRange (const CXLSRange& Range, CXLSRange& retTableRange) const;
		SXLSCellFill GetCellFill (const SXLSCellRef& Addr) const;
		CDatum GetCellValue (const SXLSCellRef& Addr) const { return GetCellValue(GetCell(Addr.iRow, Addr.iCol)); }
		SCellView GetCellView (int iRowIndex, int iCellIndex) const;
		const CXLSRange& GetExtent () const { return m_Extent; }
		int GetID () const { return m_iID; }
		CDatum GetInfo () const;
		const CString& GetName () const { return m_sName; }
		TArray<CDatum> GetRow (const CXLSRange& Range) const;
		int GetRowCount () const { return m_Rows.GetCount(); }
		SRowInfo GetRowInfo (int iRowIndex) const;
		CDatum GetTable (const CXLSRange& Range = CXLSRange()) const;
		CXLSRange GetTableRangeAt (const SXLSCellRef& Addr) const;
		bool SetCellFill (const CXLSRange& Range, const SXLSCellFill& Fill);
		void SetCellValue (const SXLSCellRef& Addr, CDatum dValue);
		void SetID (int iID) { m_iID = iID; }
		bool SetInfo (CDatum dInfo, CString* retsError = NULL);
		void SetStyles (CXLSStyles& Styles) { m_pStyles = &Styles; }
		bool SetValue (CDatum dValue, CString* retsError = NULL);
		void WriteToStream (IByteStream& Stream) const;

	private:

		struct SCell
			{
			int iCol = 0;					//	1-based column
			EXLSValueType iType = EXLSValueType::None;
			CString sValue;
			int iFormat = -1;				//	Index into m_Formats (0-based)
			};

		struct SRow
			{
			int iRow = 0;					//	1-based row.
			TSortMap<int, SCell> Cells;
			};

		struct SIterator
			{
			SIterator (const CXLSRange& RangeArg) :
					Range(RangeArg)
				{ }

			CXLSRange Range;
			int iRow = 0;
			int iCol = 0;
			int iRowIndex = -1;
			int iCellIndex = -1;
			};

		CXLSRange CalcExtent (int iSheet = 0) const;
		static size_t CalcSerializeSize (const SRow& Row);
		static TSortMap<int, SCell> CreateCellsFromStream (CXLSStyles& Styles, IByteStream& Stream);
		const SCell& GetCell (int iRow, int iCell) const;
		CDatum GetCellValue (const SCell& Cell) const;
		bool IsCellBlank (const SCell& Cell) const;
		bool IsRowBlank (const SRow& Row, int iStartCol = 1, int iEndCol = -1) const;
		SCell& SetCell (SRow& Row, int iCol);
		SCell& SetCell (int iRow, int iCol) { return SetCell(SetRow(iRow), iCol); }
		SCell& SetCell (const SXLSCellRef& Addr) { return SetCell(SetRow(Addr.iRow), Addr.iCol); }
		void SetCellValue (SCell& Cell, CDatum dValue, CStringView sFormat = NULL_STR);
		SRow& SetRow (int iRow);
		bool SetValueAsTable (CDatum dValue, CString* retsError = NULL);
		void WriteToStream (IByteStream& Stream, const SRow& Row) const;

		static DWORD CalcAEONDatatype (const SCell& Cell);
		static bool LoadCell (CXLSLoadCtx& Load, const CXMLElement& CellXML, SCell& retCell, CString* retsError = NULL);
		static bool LoadRow (CXLSLoadCtx& Load, const CXMLElement& RowXML, SRow& retRow, CString* retsError = NULL);
		static CString MakeID (const CString& sValue);
		static TArray<CString> MakeUniqueIDs (const TArray<CString>& Values);
		static TArray<CString> MakeUniqueIDs (const TArray<CDatum>& Values);
		static DWORD MergeDatatype (DWORD dwType1, DWORD dwType2);

		bool HasMoreRows (SIterator& i) const;
		bool HasMoreCols (SIterator& i) const;
		const SCell& GetNextCell (SIterator& i, int *retiRow = NULL, int *retiCol = NULL) const;
		void NextRow (SIterator& i) const;

		CXLSStyles* m_pStyles = NULL;
		int m_iID = 0;						//	Index into m_Sheets on workbook
		CString m_sName;
		TSortMap<int, SRow> m_Rows;
		CXLSRange m_Extent;

		static SCell m_EmptyCell;
	};

class CXLSWorkbook
	{
	public:

		struct SLoadOptions
			{
			};

		CXLSWorkbook () { }
		CXLSWorkbook (const CXLSWorkbook& Src) { Copy(Src); }
		CXLSWorkbook (CXLSWorkbook&& Src) noexcept { Move(std::move(Src)); }

		CXLSWorkbook& operator= (const CXLSWorkbook& Src) { Copy(Src); return *this; }
		CXLSWorkbook& operator= (CXLSWorkbook&& Src) noexcept { Move(std::move(Src)); return *this; }

		static bool Create (const IMemoryBlock& FileData, const SLoadOptions& Options, CXLSWorkbook& retWorkbook, CString* retsError = NULL);
		static bool CreateFromStream (IByteStream& Stream, CXLSWorkbook& retWorkbook, CString* retsError = NULL);

		size_t CalcSerializeSize () const;
		int Compare (const CXLSWorkbook& Other) const;
		SXLSCellFill GetCellFill (int iSheet, const SXLSCellRef& Addr) const { return GetSheet(iSheet).GetCellFill(Addr); }
		CDatum GetCellValue (int iSheet, const SXLSCellRef& Addr) const { return GetSheet(iSheet).GetCellValue(Addr);}
		const CXLSWorksheet& GetSheet (int iSheet) const;
		int GetSheetCount () const;
		CDatum GetSheetInfo (int iSheet) const;
		CXLSStyles& GetStyles () { return m_Styles; }
		const CXLSStyles& GetStyles () const { return m_Styles; }
		CDatum GetTable (const CXLSRange& Range = CXLSRange()) const;
		void InsertEmptySheet (const CString& sName = NULL_STR, int iIndex = -1);
		void InsertSheet (TUniquePtr<CXLSWorksheet>&& pSheet, int iIndex = -1);
		bool SetCellFill (int iSheet, const CXLSRange& Range, const SXLSCellFill& Fill);
		bool SetCellValue (int iSheet, const SXLSCellRef& Addr, CDatum dValue);
		bool SetSheetInfo (int iSheet, CDatum dInfo, CString* retsError = NULL);
		bool SetSheetValue (int iSheet, CDatum dValue, CString* retsError = NULL);
		void WriteToStream (IByteStream& Stream) const;

	private:

		static constexpr DWORD SERIALIZED_VERSION = 2;

		void DebugValidRefCounts () const;
		void Copy (const CXLSWorkbook& Src);
		void Move (CXLSWorkbook&& Src);

		TArray<TUniquePtr<CXLSWorksheet>> m_Sheets;
		CXLSStyles m_Styles;

		static CXLSStyles m_DefaultStyles;
		static CXLSWorksheet m_Empty;
	};

class CXLSWriter
	{
	public:

		struct SOptions
			{
			CString sTitle;					//	Title of the Workbook
			CString sCreatedBy;				//	Username of creator
			CDateTime CreatedOn;			//	Date/time of creation
			CString sModifiedBy;			//	Username of last modifier
			CDateTime ModifiedOn;			//	Date/time of last modification
			CString sCompany;				//	Company that produced the document

			CString sApp;					//	App that wrote out the XLS
			CString sAppVersion;			//	Version of app
			};

		CXLSWriter (const CXLSWorkbook& Workbook, const SOptions& Options) :
				m_Workbook(Workbook)
			{ InitOptions(Options); }

		static double AsDateTime (const CDateTime& Value);
		void WriteToStream (IByteStream& Stream) const;

	private:

		struct SWorksheetCtx
			{
			TArray<CString> SharedStrings;
			TSortMap<CString, int> SharedStringMap;
			int iTotalStrings = 0;
			};

		CString GenerateContentTypes () const;
		CString GenerateDocPropsApp () const;
		CString GenerateDocPropsCore () const;
		CString GenerateRelsRels () const;
		CString GenerateSharedStrings (SWorksheetCtx& Ctx) const;
		CString GenerateStyles () const;
		CString GenerateTheme () const;
		CString GenerateWorkbook () const;
		CString GenerateWorkbookRels () const;
		CString GenerateWorksheet (const CXLSWorksheet& Worksheet, SWorksheetCtx& Ctx) const;
		void InitOptions (const SOptions& Options);
		static void WriteBlankCell (CStringBuffer& Output, const CXLSWorksheet::SCellView& Cell);
		static void WriteDateTimeCell (CStringBuffer& Output, const CXLSWorksheet::SCellView& Cell);
		static void WriteNumberCell (CStringBuffer& Output, const CXLSWorksheet::SCellView& Cell);
		static void WriteStringCell (SWorksheetCtx& Ctx, CStringBuffer& Output, const CXLSWorksheet::SCellView& Cell);
		void WriteElement (CStringBuffer& Output, const CString& sTag, const CString& sContent) const;
		void WriteVariantLPSTR (CStringBuffer& Output, const CString& sContent) const;

		const CXLSWorkbook& m_Workbook;
		SOptions m_Options;
	};
