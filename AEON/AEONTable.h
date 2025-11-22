//	AEONTable.h
//
//	AEON Table Implementation
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#pragma once

#include <memory>

class CAEONTableIndex
	{
	public:

		enum class EFindResult
			{
			Added,
			Found,
			};

		void Add (CDatum dTable, int iRow);
		void AddRow (CStringView sIndexKey, int iRow);
		void DeleteRow (CDatum dTable, int iRow);
		int Find (CDatum dTable, CDatum dValue) const;
		int Find2 (CDatum dTable, CDatum dKey1, CDatum dKey2) const;
		int Find3 (CDatum dTable, CDatum dKey1, CDatum dKey2, CDatum dKey3) const;
		EFindResult FindOrAdd (CDatum dTable, CDatum dKey, int iNewRow, int *retiRow = NULL);
		int GetCount () const { return m_Index.GetCount(); }
		int GetRow (int iIndex) const { return m_Index[iIndex]; }
		const TSortMap<CString, int>& GetIndex () const { return m_Index; }
		CString GetIndexKey (const IAEONTable& Table, int iRow) const;
		CDatum GetKeyArray (CDatum dTable) const;
		CDatum GetKeyFromColumnStruct (CDatum dTable, CDatum dCols, int iIndex) const;
		CDatum GetKeyFromRow (CDatum dTable, CDatum dRow) const;
		CDatum GetKeyFromRow (CDatum dTable, int iRow) const;
		CDatum GetKeyFromRowArray (CDatum dTable, CDatum dRow) const;
		CDatum GetValueFromKey (CDatum dTable, CDatum dKey, const CString& sCol) const;
		void Init (CDatum dTable, const TArray<int>& Cols);
		void Init (CDatum dTable, const TArray<CString>& Cols);
		void InitColsOnly (CDatum dTable, const TArray<int>& Cols);
		void InitFromArrayOfArrays (CDatum dArray, const TArray<int>& Cols);
		void InitFromArrayOfStructs (CDatum dArray, const TArray<CString>& Cols);
		void InitFromStructOfArrays (CDatum dStruct, const TArray<CString>& Cols);
		TSortMap<CString, int> Merge (const CAEONTableIndex& Src) const;
		void Remove (CDatum dTable, int iRow);
		void ReplaceIndex (TSortMap<CString, int>&& Index) { m_Index = std::move(Index); }
		void SetColumns (const TArray<int>& Cols) { m_Cols = Cols; }

		static TArray<bool> CalcColumnIsKey (CDatum dSchema);
		static const CAEONTableIndex Null;

	private:

		CString AsIndexKeyFromRow (CDatum dTable, CDatum dValue) const;
		CString AsIndexKeyFrom2 (CDatum dTable, CDatum dKey1, CDatum dKey2) const;
		CString AsIndexKeyFrom3 (CDatum dTable, CDatum dKey1, CDatum dKey2, CDatum dKey3) const;
		static CString AsIndexKeyFromArray (CDatum dArray, const TArray<int>& Cols);
		static CString AsIndexKeyFromStruct (CDatum dStruct, const TArray<CString>& Cols);
		static CString AsIndexKeyFromValue (CDatum dValue);

		TArray<int> m_Cols;			//	-1 means missing field
		TSortMap<CString, int> m_Index;
	};

class CAEONTable : public IComplexDatum, public IAEONTable
	{
	public:

		CAEONTable (CDatum dSchema) { SetSchema(dSchema); }
		CAEONTable (CDatum dSchema, TArray<CDatum>&& Cols);
		~CAEONTable () { if (m_dwTableID != 0xffffffff) CAEONStore::FreeTableID(m_dwTableID);}

		static CDatum Create (CDatum dSchema, TArray<CDatum>&& Cols);

		//	IAEONTable

		virtual EResult AppendEmptyRow (int iCount = 1) override;
		virtual EResult AppendRow (CDatum dRow, int* retiRow = NULL) override;
		virtual EResult AppendTable (CDatum dTable) override;
		virtual CDatum CombineSubset (SSubset& ioSubset) const override { return CDatum::raw_AsComplex(this); }
		virtual EResult DeleteAllRows () override;
		virtual EResult DeleteCol (int iCol) override;
		virtual EResult DeleteRowByID (CDatum dKey) override;
		virtual bool FindCol (const CString &sName, int *retiCol = NULL) const override;
		virtual bool FindRowByID (CDatum dValue, int *retiRow = NULL) const override;
		virtual bool FindRowByID2 (CDatum dKey1, CDatum dKey2, int* retiRow = NULL) const override;
		virtual bool FindRowByID3 (CDatum dKey1, CDatum dKey2, CDatum dKey3, int* retiRow = NULL) const override;
		virtual CDatum GetCol (int iIndex) const override { return ((iIndex >= 0 && iIndex < m_Cols.GetCount()) ? m_Cols[iIndex] : CDatum()); }
		virtual CDatum GetCol (CAEONTypeSystem& TypeSystem, CDatum dColName) const override;
		virtual int GetColCount () const override;
		virtual CString GetColName (int iCol) const override;
		virtual CDatum GetDataSlice (int iFirstRow, int iRowCount) const override;
		virtual CDatum GetFieldValue (int iRow, int iCol) const override;
		virtual const CAEONTableGroupDefinition& GetGroups () const override { return m_GroupDef; }
		virtual const CAEONTableGroupIndex& GetGroupIndex () const override { CSmartLock Lock(m_cs); return m_GroupIndex; }
		virtual CDatum GetKeyFromRow (CDatum dTable, CDatum dRow) const override;
		virtual TSortMap<CDatum, int> GetKeyIndex (CDatum dTable) const override;
		virtual EKeyType GetKeyType () const override { return m_iKeyType; }
		virtual SequenceNumber GetNextID () const override { return m_NextID; }
		virtual CDatum GetRow (int iRow) const override;
		virtual int GetRowCount () const override;
		virtual CDatum GetRowID (int iRow) const override;
		virtual CDatum GetSchema () const override { return ((const IDatatype&)m_dDatatype).GetMember(0).dType; }
		virtual SequenceNumber GetSeq () const override { return m_Seq; }
		virtual EResult InsertColumn (const CString& sName, CDatum dType, CDatum dValues = CDatum(), int iPos = -1, int *retiCol = NULL) override;
		virtual void InvalidateKeys () override { CSmartLock Lock(m_cs); m_pKeyIndex = NULL; m_GroupIndex.DeleteAll(); }
		virtual bool IsSameSchema (CDatum dSchema) const override;
		virtual SequenceNumber MakeID () override { return ++m_NextID; }
		virtual CDatum Query (const CAEONExpression& Expr) const override;
		virtual EResult SetColumn (int iCol, IDatatype::SMemberDesc& ColDesc, CDatum dValues = CDatum()) override;
		virtual bool SetFieldValue (int iRow, int iCol, CDatum dValue) override;
		virtual void SetGroups (const CAEONTableGroupDefinition& Groups) override { CSmartLock Lock(m_cs); m_GroupDef = Groups; m_GroupIndex.DeleteAll(); }
		virtual void SetGroupIndex (CAEONTableGroupIndex&& Index) { CSmartLock Lock(m_cs); m_GroupIndex = std::move(Index); }
		virtual EResult SetNextID (SequenceNumber NextID) override { m_NextID = NextID; return EResult::OK; }
		virtual void SetReadOnly (bool bValue = true) override { m_bReadOnly = bValue; }
		virtual EResult SetRow (int iRow, CDatum dRow) override;
		virtual EResult SetRowByID (CDatum dKey, CDatum dRow, int *retiRow = NULL) override;
		virtual void SetSeq (SequenceNumber Seq) override { m_Seq = Seq; }
		virtual CDatum Sort (const TArray<SSort>& Sort) const override { return IAEONTable::CreateSorted(CDatum::raw_AsComplex(this), Sort); }

		//	IComplexDatum

		virtual void Append (CDatum dDatum) override;
		virtual CString AsString (void) const override;
		virtual size_t CalcMemorySize (void) const override;
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual void DeleteElement (int iIndex) override { DeleteRow(iIndex); }
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const override { throw CException(errFail); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::TABLE; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeTable; }
		virtual int GetCount (void) const override { return m_iRows; }
		virtual CDatum GetDatatype () const override { return m_dDatatype; }
		virtual int GetDimensions () const override { return 1; }
		virtual CDatum GetElement (int iIndex) const override { return GetRow(iIndex); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElementAt (int iIndex) const override { return GetElementAt_Impl(CDatum::raw_AsComplex(this), iIndex); }
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override { return GetElementAt_Impl(TypeSystem, CDatum::raw_AsComplex(this), dIndex); }
		virtual CDatum GetElementAt2DA (CDatum dIndex1, CDatum dIndex2) const override { return GetElementAt2DA_Impl(CDatum::raw_AsComplex(this), dIndex1, dIndex2); }
		virtual CDatum GetElementAt3DA (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3) const override { return GetElementAt3DA_Impl(CDatum::raw_AsComplex(this), dIndex1, dIndex2, dIndex3); }
		virtual CDatum GetElementAt2DI (int iIndex1, int iIndex2) const override { return GetElementAt2DI_Impl(CDatum::raw_AsComplex(this), iIndex1, iIndex2); }
		virtual CDatum GetElementAt3DI (int iIndex1, int iIndex2, int iIndex3) const override { return GetElementAt3DI_Impl(CDatum::raw_AsComplex(this), iIndex1, iIndex2, iIndex3); }
		virtual CString GetKey (int iIndex) const override { return GetKeyEx(iIndex).AsString(); }
		virtual CDatum GetKeyEx (int iIndex) const override;
		virtual CDatum GetMethod (const CString &sMethod) const override { if (m_pMethodsExt) return m_pMethodsExt->GetMethod(sMethod); else return CDatum(); }
		virtual IAEONTable *GetTableInterface () { return this; }
		virtual const CString &GetTypename (void) const override;
		virtual void GrowToFit (int iCount) override;
		virtual bool HasKeys () const override { return m_iKeyType != EKeyType::None; }
		virtual void InsertElementAt (CDatum dIndex, CDatum dDatum) override;
		virtual bool IsArray (void) const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil (void) const override { return (GetCount() == 0); }
		virtual CDatum IteratorGetKey (CDatum dIterator) const override { return GetKeyEx((int)dIterator); };
		virtual CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const override { return GetRow((int)dIterator); }
		virtual bool OpContains (CDatum dValue) const override;
		virtual bool RemoveAll () override;
		virtual bool RemoveElementAt (CDatum dIndex) override;
		virtual void ResolveDatatypes (const CAEONTypeSystem &TypeSystem) override;
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { throw CException(errFail); }
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void SetElement (int iIndex, CDatum dDatum) override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { OnModify(); m_Properties.SetProperty(*this, sKey, dDatum, NULL); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override;
		virtual void SetElementAt2DA (CDatum dIndex1, CDatum dIndex2, CDatum dValue) override;
		virtual void SetElementAt2DI (int iIndex1, int iIndex2, CDatum dValue) override { SetElementAt2DA(iIndex1, iIndex2, dValue); }
		virtual void SetElementAt3DA (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3, CDatum dValue) override;
		virtual void SetElementAt3DI (int iIndex1, int iIndex2, int iIndex3, CDatum dValue) override { SetElementAt3DA(iIndex1, iIndex2, iIndex3, dValue); }

		static bool CreateTableFromArray (CAEONTypeSystem &TypeSystem, CDatum dValue, CDatum &retdDatum);
		static bool CreateTableFromDatatype (CAEONTypeSystem &TypeSystem, CDatum dType, CDatum &retdDatum);
		static bool CreateTableFromNil (CAEONTypeSystem& TypeSystem, CDatum& retdDatum);
		static bool CreateTableFromStruct (CAEONTypeSystem &TypeSystem, CDatum dValue, CDatum &retdDatum);
		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static CDatum DeserializeAEON_v1 (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static int FindMethodByKey (const CString& sKey) { return (m_pMethodsExt ? m_pMethodsExt->FindMethod(sKey) : -1); }
		static int FindPropertyByKey (const CString& sKey) { return m_Properties.FindProperty(sKey); }
		static int GetMethodCount () { return (m_pMethodsExt ? m_pMethodsExt->GetCount() : 0); }
		static CString GetMethodKey (int iIndex) { return (m_pMethodsExt ? m_pMethodsExt->GetMethodName(iIndex) : NULL_STR);}
		static CDatum GetMethodType (int iIndex) { return (m_pMethodsExt ? m_pMethodsExt->GetMethodType(iIndex) : CAEONTypes::Get(IDatatype::FUNCTION)); }
		static int GetPropertyCount () { return m_Properties.GetCount(); }
		static CString GetPropertyKey (int iIndex) { return m_Properties.GetPropertyName(iIndex); }
		static CDatum GetPropertyType (int iIndex) { return m_Properties.GetPropertyType(iIndex); }
		static void SetMethodsExt (TDatumMethodHandler<IComplexDatum> &MethodsExt) { m_pMethodsExt = &MethodsExt; }

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		CAEONTable () { }
		CAEONTable (const CAEONTable& Src) = default;
		CAEONTable (CAEONTable&& Src) noexcept = default;

		EResult AppendColumn (CDatum dColumn);
		EResult AppendRowArray (CDatum dRow, int* retiRow = NULL);
		EResult AppendRowStruct (CDatum dRow, int* retiRow = NULL);
		EResult AppendSlice (CDatum dSlice);
		static CDatum CalcColumnDatatype (CDatum dValue);
		void CloneContents (CDatum::EClone iMode);
		EResult DeleteRow (int iRow);
		std::shared_ptr<CAEONTableIndex> GetIndex () const;
		EResult MergeArrayOfArrays (CDatum dArray);
		EResult MergeArrayOfStructs (CDatum dArray);
		EResult MergeStructOfColumns (CDatum dStruct);
		EResult MergeTable (CDatum dTable);
		bool OnModify ();
		void SetIndex (const CAEONTableIndex& Index);
		void SetIndex (CAEONTableIndex&& Index);
		EResult SetRowFromColumnStruct (CDatum dCols, int iIndex, int *retiRow = NULL);
		void SetSchema (CDatum dDatatype, bool bNoColCreate = false);

		int m_iRows = 0;
		TArray<CDatum> m_Cols;
		TArray<bool> m_IsKeyCol;	//	True if this column is a key column

		//	NOTE: The constructor guarantees that we have a valid schema.
		//	We can always rely on m_dDatatype being of type ECategory::Table.

		CDatum m_dDatatype;
		CAEONTableGroupDefinition m_GroupDef;	//	Definition of table groups
		SequenceNumber m_NextID = 0;

		SequenceNumber m_Seq = 0;
		bool m_bCopyOnWrite = false;
		bool m_bReadOnly = false;

		EKeyType m_iKeyType = EKeyType::None;
		mutable CCriticalSection m_cs;	//	Protects m_pKeyIndex
		mutable std::shared_ptr<CAEONTableIndex> m_pKeyIndex;
		mutable CAEONTableGroupIndex m_GroupIndex;		//	Index for table groups
		mutable DWORD m_dwTableID = 0xffffffff;			//	Runtime table ID (for row references)

		static TDatumPropertyHandler<CAEONTable> m_Properties;
		static TDatumMethodHandler<IComplexDatum> *m_pMethodsExt;

		friend class CAEONScriptParser;
		friend class CJSONParser;
	};

class CAEONTableRef : public IComplexDatum, public IAEONTable
	{
	public:

		~CAEONTableRef () { if (m_dwTableID != 0xffffffff) CAEONStore::FreeTableID(m_dwTableID);}

		static bool Create (CAEONTypeSystem& TypeSystem, CDatum dTable, SSubset&& Subset, CDatum& retdValue);

		//	IAEONTable

		virtual EResult AppendEmptyRow (int iCount = 1) override;
		virtual EResult AppendRow (CDatum dRow, int* retiRow = NULL) override;
		virtual EResult AppendTable (CDatum dTable) override;
		virtual CDatum CombineSubset (SSubset& ioSubset) const override;
		virtual EResult DeleteAllRows () override;
		virtual EResult DeleteCol (int iCol) override;
		virtual EResult DeleteRowByID (CDatum dKey) override;
		virtual bool FindCol (const CString& sName, int *retiCol = NULL) const override;
		virtual bool FindRowByID (CDatum dValue, int *retiRow = NULL) const override;
		virtual bool FindRowByID2 (CDatum dKey1, CDatum dKey2, int* retiRow = NULL) const override;
		virtual bool FindRowByID3 (CDatum dKey1, CDatum dKey2, CDatum dKey3, int* retiRow = NULL) const override;
		virtual CDatum GetCol (int iIndex) const override { return GetColRef(iIndex); }
		virtual CDatum GetCol (CAEONTypeSystem& TypeSystem, CDatum dColName) const override;
		virtual int GetColCount () const override { return m_Cols.GetCount(); }
		virtual CString GetColName (int iCol) const override;
		virtual CDatum GetDataSlice (int iFirstRow, int iRowCount) const override;
		virtual CDatum GetFieldValue (int iRow, int iCol) const override;
		virtual const CAEONTableGroupDefinition& GetGroups () const override { return m_GroupDef; }
		virtual const CAEONTableGroupIndex& GetGroupIndex () const override { CSmartLock Lock(m_cs); return m_GroupIndex; }
		virtual CDatum GetKeyFromRow (CDatum dTable, CDatum dRow) const override;
		virtual EKeyType GetKeyType () const override;
		virtual SequenceNumber GetNextID () const override;
		virtual CDatum GetRow (int iRow) const override;
		virtual int GetRowCount () const override;
		virtual CDatum GetRowID (int iRow) const override;
		virtual CDatum GetSchema () const override { return ((const IDatatype&)m_dDatatype).GetMember(0).dType; }
		virtual SequenceNumber GetSeq () const override;
		virtual EResult InsertColumn (const CString& sName, CDatum dType, CDatum dValues = CDatum(), int iPos = -1, int *retiCol = NULL) override;
		virtual void InvalidateKeys () override;
		virtual bool IsSameSchema (CDatum dDatatype) const override;
		virtual SequenceNumber MakeID () override;
		virtual CDatum Query (const CAEONExpression& Expr) const override { throw CException(errFail); }
		virtual EResult SetColumn (int iCol, IDatatype::SMemberDesc& ColDesc, CDatum dValues = CDatum()) override;
		virtual bool SetFieldValue (int iRow, int iCol, CDatum dValue) override;
		virtual void SetGroups (const CAEONTableGroupDefinition& Groups) override { CSmartLock Lock(m_cs); m_GroupDef = Groups; m_GroupIndex.DeleteAll(); }
		virtual void SetGroupIndex (CAEONTableGroupIndex&& Index) { CSmartLock Lock(m_cs); m_GroupIndex = std::move(Index); }
		virtual EResult SetNextID (SequenceNumber NextID) override;
		virtual void SetReadOnly (bool bValue = true) override;
		virtual EResult SetRow (int iRow, CDatum dRow) override;
		virtual EResult SetRowByID (CDatum dKey, CDatum dRow, int *retiRow = NULL) override;
		virtual void SetSeq (SequenceNumber Seq) override;
		virtual CDatum Sort (const TArray<SSort>& Sort) const override { return IAEONTable::CreateSorted(CDatum::raw_AsComplex(this), Sort); }

		//	IComplexDatum

		virtual void Append (CDatum dDatum) override { Materialize(); m_dTable.Append(dDatum); ApplyDatatype(m_dTable.GetDatatype()); }
		virtual CString AsString (void) const override;
		virtual size_t CalcMemorySize (void) const override;
		virtual IComplexDatum* Clone (CDatum::EClone iMode) const override;
		virtual bool Find (CDatum dValue, int* retiIndex = NULL) const override { throw CException(errFail); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::TABLE; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeTable; }
		virtual int GetCount (void) const override { return (m_bAllRows ? m_dTable.GetCount() : m_Rows.GetCount()); }
		virtual CDatum GetDatatype () const override { return m_dDatatype; }
		virtual int GetDimensions () const override { return 1; }
		virtual CDatum GetElement (int iIndex) const override { return GetRow(iIndex); }
		virtual CDatum GetElement (const CString& sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElementAt (int iIndex) const override { return GetElementAt_Impl(CDatum::raw_AsComplex(this), iIndex); }
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override { return GetElementAt_Impl(TypeSystem, CDatum::raw_AsComplex(this), dIndex); }
		virtual CDatum GetElementAt2DA (CDatum dIndex1, CDatum dIndex2) const override { return GetElementAt2DA_Impl(CDatum::raw_AsComplex(this), dIndex1, dIndex2); }
		virtual CDatum GetElementAt3DA (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3) const override { return GetElementAt3DA_Impl(CDatum::raw_AsComplex(this), dIndex1, dIndex2, dIndex3); }
		virtual CDatum GetElementAt2DI (int iIndex1, int iIndex2) const override { return GetElementAt2DI_Impl(CDatum::raw_AsComplex(this), iIndex1, iIndex2); }
		virtual CDatum GetElementAt3DI (int iIndex1, int iIndex2, int iIndex3) const override { return GetElementAt3DI_Impl(CDatum::raw_AsComplex(this), iIndex1, iIndex2, iIndex3); }
		virtual CString GetKey (int iIndex) const override { return GetKeyEx(iIndex).AsString(); }
		virtual CDatum GetKeyEx (int iIndex) const override;
		virtual CDatum GetMethod (const CString &sMethod) const override { if (m_pMethodsExt) return m_pMethodsExt->GetMethod(sMethod); else return CDatum(); }
		virtual IAEONTable* GetTableInterface () { return this; }
		virtual const CString& GetTypename (void) const override;
		virtual void GrowToFit (int iCount) override { Materialize(); m_dTable.GrowToFit(iCount); }
		virtual bool HasKeys () const override;
		virtual void InsertElementAt (CDatum dIndex, CDatum dDatum) override { Materialize(); m_dTable.InsertElementAt(dIndex, dDatum); }
		virtual bool IsArray (void) const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil (void) const override { return (GetCount() == 0); }
		virtual CDatum IteratorGetKey (CDatum dIterator) const override { return GetKeyEx((int)dIterator); };
		virtual CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const override { return GetRow((int)dIterator); }
		virtual bool OpContains (CDatum dValue) const override;
		virtual bool RemoveElementAt (CDatum dIndex) override { Materialize(); return m_dTable.RemoveElementAt(dIndex); }
		virtual void ResolveDatatypes (const CAEONTypeSystem& TypeSystem) override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void SetElement (int iIndex, CDatum dDatum) override;
		virtual void SetElement (const CString& sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override;
		virtual void SetElementAt2DA (CDatum dIndex1, CDatum dIndex2, CDatum dValue) override;
		virtual void SetElementAt2DI (int iIndex1, int iIndex2, CDatum dValue) override { SetElementAt2DA(iIndex1, iIndex2, dValue); }
		virtual void SetElementAt3DA (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3, CDatum dValue) override;
		virtual void SetElementAt3DI (int iIndex1, int iIndex2, int iIndex3, CDatum dValue) override { SetElementAt3DA(iIndex1, iIndex2, iIndex3, dValue); }
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { throw CException(errFail); }

		static void SetMethodsExt (TDatumMethodHandler<IComplexDatum> &MethodsExt) { m_pMethodsExt = &MethodsExt; }

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		void ApplyDatatype (CDatum dDatatype);
		void InitSourceRow () const;
		void Materialize ();

		CAEONTableRef () { }
		CDatum GetColRef (int iCol) const;

		CDatum m_dTable;						//	Guaranteed to be a valid table
		CDatum m_dDatatype;						//	The datatype for this reference
												//		(could be same as table
												//		schema).
		TArray<int> m_Cols;						//	Columns to include (cache of schema)
		TArray<int> m_Rows;						//	Row indices
		bool m_bAllRows = false;				//	If TRUE, all rows in table.
		bool m_bCopyOnWrite = false;			//	If TRUE, make a copy before mutating
		CAEONTableGroupDefinition m_GroupDef;	//	Definition of table groups

		mutable CCriticalSection m_cs;				//	Protects m_GroupIndex
		mutable CAEONTableGroupIndex m_GroupIndex;	//	Index for table groups
		mutable DWORD m_dwTableID = 0xffffffff;			//	Runtime table ID (for row references)

		mutable TArray<int> m_SourceRow;		//	Index of source row
		mutable bool m_bSourceRowValid = false;	//	TRUE if m_SourceRow is valid

		static TDatumPropertyHandler<CAEONTableRef> m_Properties;
		static TDatumMethodHandler<IComplexDatum> *m_pMethodsExt;

		friend class CAEONScriptParser;
	};

class CAEONTableRowRef : public IComplexDatum
	{
	public:

		CAEONTableRowRef (CDatum dTable, int iRow) : m_dTable(dTable), m_iRow(iRow) { }

		//	IComplexDatum

		virtual CString AsString () const override;
		virtual size_t CalcMemorySize () const override;
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual bool Contains (CDatum dValue) const override;
		virtual bool FindElement (const CString &sKey, CDatum *retpValue) const override;
		virtual DWORD GetBasicDatatype () const override { return IDatatype::SCHEMA; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeClassInstance; }
		virtual int GetCount () const override;
		virtual CDatum GetDatatype () const override;
		virtual int GetDimensions () const override { return 1; }
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override;
		virtual CString GetKey (int iIndex) const override;
		virtual CDatum GetMethod (const CString &sMethod) const override;
		virtual CDatum GetProperty (const CString& sProperty) const override;
		virtual const CString &GetTypename () const override;
		virtual bool IsArray () const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil () const override;
		virtual bool IsStruct () const override { return true; }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool OpContains (CDatum dValue) const override { return !GetElement(dValue.AsString()).IsIdenticalToNil(); }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override;
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override { SerializeAEONAsStruct(Stream, Serialized); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override;
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override;

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript(CDatum::EFormat iFormat) const override;
		virtual void OnMarked (void) override { m_dTable.Mark(); }

		const IAEONTable* ResolveTable () const;
		IAEONTable* ResolveTable ();

		CDatum m_dTable;
		int m_iRow = -1;

		static TDatumPropertyHandler<CAEONTableRowRef> m_Properties;
		static TDatumMethodHandler<CAEONTableRowRef> m_Methods;
	};

