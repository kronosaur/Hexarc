//	AEONInterfaces.h
//
//	AEON Interfaces
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class IAEONCanvas
	{
	public:

		virtual void DeleteAllGraphics () { }
		virtual int GetGraphicCount () const { return 0; }
		virtual SequenceNumber GetSeq () const { return 0; }
		virtual bool InsertGraphic (CDatum dDesc) { return false; }
		virtual CDatum RenderAsHTMLCanvasCommands (SequenceNumber Seq = 0) const { return CDatum(); }
		virtual void SetGraphicSeq (int iIndex, SequenceNumber Seq) { }
		virtual void SetGraphicSeq (SequenceNumber Seq) { }
		virtual void SetSeq (SequenceNumber Seq) { }

		virtual void* raw_GetGraphicByID (DWORD dwID) const { return NULL; }
	};

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
			NotFound,
			};

		struct SSubset
			{
			TArray<int> Cols;		//	If empty, all columns
			TArray<int> Rows;		//	Rows
			bool bAllRows = false;
			};

		virtual EResult AppendColumn (CDatum dColumn) { return EResult::NotImplemented; }
		virtual EResult AppendEmptyRow (int iCount = 1) { return EResult::NotImplemented; }
		virtual EResult AppendRow (CDatum dRow) { return EResult::NotImplemented; }
		virtual EResult AppendSlice (CDatum dSlice) { return EResult::NotImplemented; }
		virtual EResult AppendTable (CDatum dTable) { return EResult::NotImplemented; }
		virtual EResult DeleteAllRows () { return EResult::NotImplemented; }
		virtual EResult DeleteRow (int iRow) { return EResult::NotImplemented; }
		virtual EResult DeleteRowByID (CDatum dKey) { return EResult::NotImplemented; }
		virtual bool FindCol (const CString &sName, int *retiCol = NULL) const { return false; }
		bool FindRow (int iCol, CDatum dValue, int *retiRow = NULL) const;
		virtual bool FindRowByID (CDatum dValue, int *retiRow = NULL) const { return false; }
		virtual CDatum GetCol (int iIndex) const { return CDatum(); }
		virtual CDatum GetCol (CAEONTypeSystem& TypeSystem, CDatum dColName) const { return CDatum(); }
		virtual int GetColCount () const { return 0; }
		virtual CString GetColName (int iCol) const { return NULL_STR; }
		virtual CDatum GetDataSlice (int iFirstRow, int iRowCount) const { return CDatum(); }
		virtual CDatum GetFieldValue (int iRow, int iCol) const { return CDatum(); }
		virtual CDatum GetRow (int iRow) const;
		virtual int GetRowCount () const { return 0; }
		virtual CDatum GetRowID (int iRow) const { return CDatum(); }
		virtual CDatum GetSchema () const { return CDatum(); }
		virtual SequenceNumber GetSeq () const { return 0; }
		virtual bool HasKeys () const { return false; }
		virtual EResult InsertColumn (const CString& sName, CDatum dType, CDatum dValues = CDatum(), int iPos = -1, int *retiCol = NULL) { return EResult::NotImplemented; }
		virtual bool IsSameSchema (CDatum dSchema) const { return false; }
		virtual CDatum Query (const CAEONQuery& Expr) const { return CDatum(); }
		virtual bool SetFieldValue (int iRow, int iCol, CDatum dValue) { return false; }
		virtual EResult SetRow (int iRow, CDatum dRow) { return EResult::NotImplemented; }
		virtual EResult SetRowByID (CDatum dKey, CDatum dRow, int *retiRow = NULL) { return EResult::NotImplemented; }
		virtual void SetSeq (SequenceNumber Seq) { }

		static bool CombineSchema (CDatum dSchema1, CDatum dSchema2, CDatum& retdSchema);
		static CDatum CreateColumn (CDatum dType);
		static bool CreateRef (CAEONTypeSystem& TypeSystem, CDatum dTable, SSubset&& Subset, CDatum& retdValue);
		static bool CreateSchema (CAEONTypeSystem& TypeSystem, CDatum dTable, SSubset& Subset, CDatum& retdSchema);
		static bool CreateSchemaFromDesc (CAEONTypeSystem& TypeSystem, CDatum dSchemaDesc, CDatum& retdSchema);
		static bool InsertColumnToSchema (CDatum dSchema, const CString& sName, CDatum dType, int iPos, CDatum& retdSchema, int* retiCol = NULL);
	};

class CAEONTextLinesDiff
	{
	public:

		enum class EType
			{
			None,

			Delete,
			Insert,
			Same,
			};

		struct SEntry
			{
			EType iType;
			CDatum dParam;
			};

		CAEONTextLinesDiff (CDatum dDiff);

		int GetCount () const { return m_Diff.GetCount(); }
		const SEntry& GetOp (int iIndex) const { return m_Diff[iIndex]; }
		void Mark ();

	private:

		TArray<SEntry> m_Diff;
	};

class CAEONTextLinesSelection
	{
	public:

		struct SPos
			{
			int iLine = 0;					//	0 = unknown/invalid. 1-total lines.
			int iChar = 0;					//	0 = start of line.
			};

		CAEONTextLinesSelection () { }
		CAEONTextLinesSelection (int iLine, int iChar);
		CAEONTextLinesSelection (int iStartLine, int iStartChar, int iEndLine, int iEndChar);
		CAEONTextLinesSelection (CDatum dValue);

		CDatum AsAPIDatum () const;
		CDatum AsDatum () const;
		bool IsCaret () const { return (m_End.iLine == 0 && !IsEmpty()); }
		bool IsEmpty () const { return m_Start.iLine == 0; }
		bool IsRange () const { return (m_Start.iLine != 0 && m_End.iLine != 0); }

	private:

		SPos m_Start;
		SPos m_End;
	};

class IAEONTextLines
	{
	public:

		virtual void ApplyDiff (const CAEONTextLinesDiff& Diff) { throw CException(errFail); }
		virtual SequenceNumber GetSeq () const { return 0; }
		virtual void SetSeq (SequenceNumber Seq) { throw CException(errFail); }
	};