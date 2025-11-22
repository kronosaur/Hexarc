//	AEONInterfaces.h
//
//	AEON Interfaces
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

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

class IAEONRange
	{
	public:

		virtual CDatum GetEnd () const { return CDatum(); }
		virtual int GetLength () const { return 0; }
		virtual CDatum GetStart () const { return CDatum(); }
		virtual CDatum GetStep () const { return CDatum(); }
	};

class IAEONReanimator
	{
	public:

		virtual int GetObjCount () const { return 0; }
		virtual SequenceNumber GetSeq () const { return 0; }
		virtual void Play (int iStartFrame = 0) { }
		virtual CDatum RenderAsHTMLCanvasCommands (SequenceNumber Seq = 0) const { return CDatum(); }
		virtual void SetSeq (SequenceNumber Seq) { }
		virtual void Stop () { }
	};

class CAEONTableGroupIndex
	{
	public:
		CAEONTableGroupIndex () { }
		CAEONTableGroupIndex (TArray<TArray<int>>&& Index) : m_Index(std::move(Index)) { }

		void DeleteAll () { m_Index.DeleteAll(); }
		int GetCount () const { return m_Index.GetCount(); }
		const TArray<int>& GetGroupIndex (int iIndex) const
			{
			if (iIndex < 0 || iIndex >= m_Index.GetCount())
				throw CException(errFail);
			return m_Index[iIndex];
			}
		bool IsEmpty () const { return m_Index.GetCount() == 0; }

	private:
		TArray<TArray<int>> m_Index;
	};

class CAEONTableGroupDefinition
	{
	public:
		CAEONTableGroupDefinition () { }

		void AddExpression (const CAEONExpression& Expr) { m_Expressions.Insert(Expr); }
		static CAEONTableGroupDefinition DeserializeAEON (IByteStream& Stream, CAEONSerializedMap &Serialized);
		int GetCount () const { return m_Expressions.GetCount(); }
		const CAEONExpression& GetExpression (int iIndex) const { if (iIndex < 0 || iIndex >= m_Expressions.GetCount()) throw CException(errFail); return m_Expressions[iIndex]; }
		bool IsEmpty () const { return m_Expressions.GetCount() == 0; }
		void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const;

	private:
		TArray<CAEONExpression> m_Expressions;
	};

class IAEONTable
	{
	public:

		enum class EKeyType
			{
			None,							//	No index

			SingleInt32,					//	Single column, Int32 index
			SingleFloat,					//	Single column, Float index
			SingleNumber,					//	Single column, Real or Number
			Single,							//	Single column index
			Multiple,						//	Multiple column index
			};

		enum class EResult
			{
			OK,

			InvalidParam,
			NotATable,
			NotImplemented,
			NotMutable,
			NotFound,
			AlreadyExists,
			};

		enum class ESort
			{
			None,

			Ascending,
			Descending,
			};

		struct SSort
			{
			ESort iOrder = ESort::None;
			int iCol = -1;
			};

		struct SSubset
			{
			TArray<int> Cols;		//	If empty, all columns
			TArray<int> Rows;		//	Rows
			bool bAllRows = false;

			CAEONTableGroupDefinition GroupDef;
			CAEONTableGroupIndex GroupIndex;
			};

		struct SDiffOptions
			{
			TArray<CString> ExcludeCols;
			bool bCollapsed = false;
			bool bCleaned = false;
			};

		struct SFilterModifiedOptions
			{
			bool bExcludeNull = false;
			};

		virtual EResult AppendEmptyRow (int iCount = 1) = 0;
		virtual EResult AppendRow (CDatum dRow, int* retiRow = NULL) = 0;
		EResult AppendRowIfNew (CDatum dTable, CDatum dRow, int* retiRow = NULL);
		virtual EResult AppendTable (CDatum dTable) = 0;
		EResult AppendTableColumns (CDatum dTable, CDatum dSrcTable);
		virtual CDatum CombineSubset (SSubset& ioSubset) const = 0;
		CDatum CreateFormattedTable (CDatum dTable) const;
		virtual EResult DeleteAllRows () = 0;
		virtual EResult DeleteCol (int iCol) = 0;
		virtual EResult DeleteRowByID (CDatum dKey) = 0;
		bool Diff (CDatum dTable, CDatum dOriginalTable, const SDiffOptions& Options, CArrayDiff::Results& retResults, CString* retsError = NULL) const;
		virtual bool FindCol (const CString &sName, int *retiCol = NULL) const = 0;
		bool FindColExt (CDatum dColID, int *retiCol = NULL) const;
		bool FindRow (int iCol, CDatum dValue, int *retiRow = NULL) const;
		virtual bool FindRowByID (CDatum dValue, int *retiRow = NULL) const = 0;
		virtual bool FindRowByID2 (CDatum dKey1, CDatum dKey2, int* retiRow = NULL) const = 0;
		virtual bool FindRowByID3 (CDatum dKey1, CDatum dKey2, CDatum dKey3, int* retiRow = NULL) const = 0;
		int FindRowByMaxColValue (int iCol) const;
		int FindRowByMinColValue (int iCol) const;
		virtual CDatum GetCol (int iIndex) const = 0;
		virtual CDatum GetCol (CAEONTypeSystem& TypeSystem, CDatum dColName) const = 0;
		virtual int GetColCount () const = 0;
		IDatatype::SMemberDesc GetColDesc (CDatum dTable, int iCol) const;
		virtual CString GetColName (int iCol) const = 0;
		TArray<int> GetColsInSortedOrder () const;
		virtual CDatum GetDataSlice (int iFirstRow, int iRowCount) const = 0;
		CDatum GetElementAtIndex (CAEONTypeSystem& TypeSystem, CDatum dTable, CDatum dIndex) const;
		virtual CDatum GetFieldValue (int iRow, int iCol) const = 0;
		virtual const CAEONTableGroupDefinition& GetGroups () const = 0;
		virtual const CAEONTableGroupIndex& GetGroupIndex () const = 0;
		virtual CDatum GetKeyFromRow (CDatum dTable, CDatum dRow) const = 0;
		virtual TSortMap<CDatum, int> GetKeyIndex (CDatum dTable) const;
		virtual EKeyType GetKeyType () const = 0;
		virtual SequenceNumber GetNextID () const = 0;
		virtual CDatum GetRow (int iRow) const;
		virtual int GetRowCount () const = 0;
		virtual CDatum GetRowID (int iRow) const = 0;
		virtual CDatum GetSchema () const = 0;
		virtual SequenceNumber GetSeq () const = 0;
		virtual EResult InsertColumn (const CString& sName, CDatum dType, CDatum dValues = CDatum(), int iPos = -1, int *retiCol = NULL) = 0;
		virtual void InvalidateKeys () = 0;
		virtual bool IsSameSchema (CDatum dDatatype) const = 0;
		virtual SequenceNumber MakeID () = 0;
		CDatum MakeSortDesc (CDatum dTable, const TArray<IAEONTable::SSort>& SortOrder) const;
		bool ParseSort (CDatum dValue, TArray<SSort>& retSort) const;
		virtual CDatum Query (const CAEONExpression& Expr) const = 0;
		virtual EResult SetColumn (int iCol, IDatatype::SMemberDesc& ColDesc, CDatum dValues = CDatum()) = 0;
		void SetElementAtIndex (CDatum dTable, CDatum dIndex, CDatum dValue, int* retiIndex = NULL);
		virtual bool SetFieldValue (int iRow, int iCol, CDatum dValue) = 0;
		virtual void SetGroups (const CAEONTableGroupDefinition& Groups) = 0;
		virtual void SetGroupIndex (CAEONTableGroupIndex&& Index) = 0;
		virtual EResult SetNextID (SequenceNumber NextID) = 0;
		virtual void SetReadOnly (bool bValue = true) = 0;
		virtual EResult SetRow (int iRow, CDatum dRow) = 0;
		virtual EResult SetRowByID (CDatum dKey, CDatum dRow, int *retiRow = NULL) = 0;
		virtual void SetSeq (SequenceNumber Seq) = 0;
		virtual CDatum Sort (const TArray<SSort>& Sort) const = 0;

		static EKeyType CalcKeyType (CDatum dDatatype, TArray<int>* retpKeyCols = NULL);
		static CString CalcUniqueColName (const IDatatype& Schema, CStringView sName);
		static bool CombineSchema (CDatum dDatatype1, CDatum dDatatype2, CDatum& retdDatatype);
		static CDatum CreateColumn (CDatum dType);
		static TArray<CDatum> CreateColumns (const IDatatype& Schema, TArray<bool>* retpIsKeyCol = NULL, int iGrowToFit = 0);
		static bool CreateRef (CAEONTypeSystem& TypeSystem, CDatum dTable, SSubset&& Subset, CDatum& retdValue);
		static bool CreateSchemaFromDesc (CAEONTypeSystem& TypeSystem, CDatum dSchemaDesc, CDatum& retdDatatype);
		static bool CreateTableDatatype (CAEONTypeSystem& TypeSystem, CDatum dTable, SSubset& Subset, CDatum& retdDatatype);
		static bool CreateTableDatatypeFromDesc (CAEONTypeSystem& TypeSystem, CDatum dSchemaDesc, CDatum& retdDatatype);
		static CDatum CreateSorted (CDatum dTable, const TArray<SSort>& Sort);
		static bool DeleteColumnFromSchema (CDatum dSchema, int iCol, CDatum& retdSchema);
		static bool DerefByPosition (EKeyType iKeyType, CDatum dIndex);
		static CDatum FilterModified (CDatum dTable, const CDateTime& ModifiedOn);
		static CDatum FilterModifiedByTable (CDatum dTable, CDatum dOriginalTable, const SFilterModifiedOptions& Options);
		static bool FindColName (CStringView sCol, const TArray<CString>& Cols);
		static bool InsertColumnToSchema (CDatum dSchema, const CString& sName, CDatum dType, int iPos, CDatum& retdSchema, int* retiCol = NULL);
		static bool IsValidMemberType (IDatatype::EMemberType iType)
			{ return (iType == IDatatype::EMemberType::InstanceKeyVar || iType == IDatatype::EMemberType::InstanceVar); }
		static bool ParseColumnDesc (CDatum dDesc, IDatatype::SMemberDesc& retDesc, bool bAllowPartial = false);
		static bool SetColumnSchema (CDatum dSchema, int iCol, IDatatype::SMemberDesc& ColDesc, CDatum& retdSchema);

	protected:

		CDatum GetElementAt_Impl (CDatum dTable, int iIndex) const;
		CDatum GetElementAt_Impl (CAEONTypeSystem& TypeSystem, CDatum dTable, CDatum dIndex) const;
		CDatum GetElementAt2DA_Impl (CDatum dTable, CDatum dIndex1, CDatum dIndex2) const;
		CDatum GetElementAt3DA_Impl (CDatum dTable, CDatum dIndex1, CDatum dIndex2, CDatum dIndex3) const;
		CDatum GetElementAt2DI_Impl (CDatum dTable, int iIndex1, int iIndex2) const;
		CDatum GetElementAt3DI_Impl (CDatum dTable, int iIndex1, int iIndex2, int iIndex3) const;
		CDatum GetElementAtKey (CAEONTypeSystem& TypeSystem, CDatum dTable, CDatum dKey) const;
		bool ParseSort (CDatum dValue, SSort& retSort) const;
		void SetElementAt_Impl (CDatum dTable, CDatum dIndex, CDatum dRow);
		void SetElementAt2DA_Impl (CDatum dTable, CDatum dIndex1, CDatum dIndex2, CDatum dValue);
		void SetElementAt3DA_Impl (CDatum dTable, CDatum dIndex1, CDatum dIndex2, CDatum dIndex3, CDatum dValue);
		void SetElementAtKey (CDatum dTable, CDatum dKey, CDatum dRow);

		bool ValidateSort (const TArray<SSort>& Sort) const;
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
		int Compare (const IAEONTextLines& Src) const;
		int CompareNoCase (const IAEONTextLines& Src) const;
		virtual const CString& GetLine (int iLine) const { return NULL_STR; }
		virtual int GetLineCount () const { return 0; }
		virtual SequenceNumber GetSeq () const { return 0; }
		virtual bool SetLine (int iLine, CStringView sLine) { return false; }
		virtual void SetSeq (SequenceNumber Seq) { throw CException(errFail); }
	};