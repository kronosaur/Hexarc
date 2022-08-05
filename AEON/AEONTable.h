//	AEONTable.h
//
//	AEON Table Implementation
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CAEONTableIndex
	{
	public:

		void Add (CDatum dTable, int iRow);
		int Find (CDatum dTable, CDatum dValue) const;
		void Init (CDatum dTable, const TArray<int>& Cols);
		void Remove (CDatum dTable, int iRow);

		static const CAEONTableIndex Null;

	private:

		CString AsIndexKeyFromRow (CDatum dTable, CDatum dValue) const;
		static CString AsIndexKeyFromValue (CDatum dValue);
		CString GetIndexKey (const IAEONTable& Table, int iRow) const;

		TArray<int> m_Cols;
		TSortMap<CString, int> m_Index;
	};

class CAEONTable : public IComplexDatum, public IAEONTable
	{
	public:
		CAEONTable (CDatum dSchema) { SetSchema(dSchema); }

		//	IAEONTable

		virtual EResult AppendColumn (CDatum dColumn) override;
		virtual EResult AppendEmptyRow (int iCount = 1) override;
		virtual EResult AppendRow (CDatum dRow) override;
		virtual EResult AppendSlice (CDatum dSlice) override;
		virtual EResult AppendTable (CDatum dTable) override;
		virtual EResult DeleteAllRows () override;
		virtual EResult DeleteRow (int iRow) override;
		virtual bool FindCol (const CString &sName, int *retiCol = NULL) const override;
		virtual bool FindRowByID (CDatum dValue, int *retiRow = NULL) const override;
		virtual CDatum GetCol (int iIndex) const override { return ((iIndex >= 0 && iIndex < m_Cols.GetCount()) ? m_Cols[iIndex] : CDatum()); }
		virtual int GetColCount () const override;
		virtual CString GetColName (int iCol) const override;
		virtual CDatum GetDataSlice (int iFirstRow, int iRowCount) const override;
		virtual CDatum GetFieldValue (int iRow, int iCol) const override;
		virtual int GetRowCount () const override;
		virtual SequenceNumber GetSeq () const override { return m_Seq; }
		virtual EResult InsertColumn (const CString& sName, CDatum dType, CDatum dValues = CDatum(), int iPos = -1, int *retiCol = NULL) override;
		virtual bool IsSameSchema (CDatum dSchema) const override;
		virtual CDatum Query (const CAEONQuery& Expr) const override;
		virtual bool SetFieldValue (int iRow, int iCol, CDatum dValue) override;
		virtual void SetSeq (SequenceNumber Seq) override { m_Seq = Seq; }
		virtual EResult SetRow (int iRow, CDatum dRow) override;

		//	IComplexDatum

		virtual void Append (CDatum dDatum) override;
		virtual CString AsString (void) const override;
		virtual size_t CalcMemorySize (void) const override;
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual void DeleteElement (int iIndex) override { DeleteRow(iIndex); }
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const override { throw CException(errFail); }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeTable; }
		virtual int GetCount (void) const override { return m_iRows; }
		virtual CDatum GetDatatype () const override { return m_dSchema; }
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override;
		virtual IAEONTable *GetTableInterface () { return this; }
		virtual const CString &GetTypename (void) const override;
		virtual void GrowToFit (int iCount) override;
		virtual bool IsArray (void) const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil (void) const override { return (GetCount() == 0); }
		virtual void ResolveDatatypes (const CAEONTypeSystem &TypeSystem) override;
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { throw CException(errFail); }
		virtual void SetElement (int iIndex, CDatum dDatum) override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { OnModify(); m_Properties.SetProperty(*this, sKey, dDatum, NULL); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override;

		static bool CreateTableFromArray (CAEONTypeSystem &TypeSystem, CDatum dValue, CDatum &retdDatum);
		static bool CreateTableFromDatatype (CAEONTypeSystem &TypeSystem, CDatum dType, CDatum &retdDatum);
		static bool CreateTableFromNil (CAEONTypeSystem& TypeSystem, CDatum& retdDatum);
		static bool CreateTableFromStruct (CAEONTypeSystem &TypeSystem, CDatum dValue, CDatum &retdDatum);
		static bool IsValidMemberType (IDatatype::EMemberType iType)
			{ return (iType == IDatatype::EMemberType::InstanceKeyVar || iType == IDatatype::EMemberType::InstanceVar); }

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		CAEONTable () { }

		static CDatum CalcColumnDatatype (CDatum dValue);
		void CloneContents ();
		const CAEONTableIndex& GetIndex () const;
		void OnModify ();
		void SetSchema (CDatum dSchema);

		int m_iRows = 0;
		TArray<CDatum> m_Cols;

		//	NOTE: The constructor guarantees that we have a valid schema.
		//	We can always rely on m_dSchema being of type ECategory::Schema.

		CDatum m_dSchema;

		SequenceNumber m_Seq = 0;
		bool m_bCopyOnWrite = false;
		bool m_bHasIndex = false;

		mutable TUniquePtr<CAEONTableIndex> m_pPrimaryIndex;

		static TDatumPropertyHandler<CAEONTable> m_Properties;

		friend class CAEONScriptParser;
		friend class CJSONParser;
	};

class CAEONTableRef : public IComplexDatum, public IAEONTable
	{
	public:
		static bool Create (CAEONTypeSystem& TypeSystem, CDatum dTable, SSubset&& Subset, CDatum& retdValue);

		//	IAEONTable

		virtual EResult AppendColumn (CDatum dColumn) override { return EResult::NotMutable; }
		virtual EResult AppendRow (CDatum dRow) override { return EResult::NotMutable; }
		virtual EResult AppendSlice (CDatum dSlice) override { return EResult::NotMutable; }
		virtual EResult AppendTable (CDatum dTable) override { return EResult::NotMutable; }
		virtual EResult DeleteAllRows () override { return EResult::NotMutable; }
		virtual bool FindCol (const CString& sName, int *retiCol = NULL) const override;
		virtual CDatum GetCol (int iIndex) const override { return GetColRef(iIndex); }
		virtual int GetColCount () const override { return m_Cols.GetCount(); }
		virtual CString GetColName (int iCol) const override;
		virtual CDatum GetDataSlice (int iFirstRow, int iRowCount) const override;
		virtual CDatum GetFieldValue (int iRow, int iCol) const override;
		virtual int GetRowCount () const override;
		virtual bool IsSameSchema (CDatum dSchema) const override;
		virtual bool SetFieldValue (int iRow, int iCol, CDatum dValue) override;
		virtual EResult SetRow (int iRow, CDatum dRow) override;

		//	IComplexDatum

		virtual void Append (CDatum dDatum) override { }
		virtual CString AsString (void) const override;
		virtual size_t CalcMemorySize (void) const override;
		virtual IComplexDatum* Clone (CDatum::EClone iMode) const override { return new CAEONTableRef(*this); }
		virtual bool Find (CDatum dValue, int* retiIndex = NULL) const override { throw CException(errFail); }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeTable; }
		virtual int GetCount (void) const override { return (m_bAllRows ? m_dTable.GetCount() : m_Rows.GetCount()); }
		virtual CDatum GetDatatype () const override { return m_dSchema; }
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString& sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override;
		virtual IAEONTable* GetTableInterface () { return this; }
		virtual const CString& GetTypename (void) const override;
		virtual void GrowToFit (int iCount) override { }
		virtual bool IsArray (void) const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil (void) const override { return (GetCount() == 0); }
		virtual void ResolveDatatypes (const CAEONTypeSystem& TypeSystem) override;
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { throw CException(errFail); }
		virtual void SetElement (int iIndex, CDatum dDatum) override;
		virtual void SetElement (const CString& sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override;

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		CAEONTableRef () { }
		CDatum GetColRef (int iCol) const;

		CDatum m_dTable;					//	Guaranteed to be a valid table
		CDatum m_dSchema;					//	The schema for this reference
											//		(could be same as table
											//		schema).
		TArray<int> m_Cols;					//	Columns to include (cache of schema)
		TArray<int> m_Rows;					//	Row indices
		bool m_bAllRows = false;			//	If TRUE, all rows in table.

		static TDatumPropertyHandler<CAEONTableRef> m_Properties;

		friend class CAEONScriptParser;
	};

class CAEONTableProcessor
	{
	public:
		CAEONTableProcessor (CDatum dSchema, const CAEONQuery& Expr);

		CDatum Eval (const IAEONTable& Table) const;
		bool Eval (const IAEONTable& Table, int iRow) const;

	private:

		enum class EOp
			{
			None,

			EqualTo,
			GreaterThan,
			GreaterThanOrEqualTo,
			In,
			LessThan,
			LessThanOrEqualTo,

			And,
			Or,
			};

		enum class EParamType
			{
			None,

			True,
			False,
			Literal,						//	dValue is value.
			Field,							//	iValue is field index.
			Op,								//	iValue is index in m_Code.
			};

		struct SParam
			{
			EParamType iType = EParamType::None;
			CDatum dValue;
			int iValue = 0;
			};

		struct SEntry
			{
			EOp iOp = EOp::None;

			SParam Left;
			SParam Right;
			};

		void Compile (CDatum dSchema, const CAEONQuery& Expr);

		CDatum m_dSchema;
		const CAEONQuery& m_Expr;

		TArray<SEntry> m_Code;
		int m_iStart = -1;
	};
