//	AEONInterfaces.h
//
//	AEON Interfaces
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class IAEONCanvas
	{
	public:

		virtual int GetGraphicCount () const { return 0; }
		virtual SequenceNumber GetSeq () const { return 0; }
		virtual bool InsertGraphic (CDatum dDesc) { return false; }
		virtual CDatum RenderAsHTMLCanvasCommands (SequenceNumber Seq = 0) const { return CDatum(); }
		virtual void SetSeq (SequenceNumber Seq) { }
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
		virtual bool FindCol (const CString &sName, int *retiCol = NULL) const { return false; }
		bool FindRow (int iCol, CDatum dValue, int *retiRow = NULL) const;
		virtual CDatum GetCol (int iIndex) const { return CDatum(); }
		virtual int GetColCount () const { return 0; }
		virtual CString GetColName (int iCol) const { return NULL_STR; }
		virtual CDatum GetDataSlice (int iFirstRow, int iRowCount) const { return CDatum(); }
		virtual CDatum GetFieldValue (int iRow, int iCol) const { return CDatum(); }
		virtual int GetRowCount () const { return 0; }
		virtual SequenceNumber GetSeq () const { return 0; }
		virtual EResult InsertColumn (const CString& sName, CDatum dType, CDatum dValues = CDatum(), int iPos = -1, int *retiCol = NULL) { return EResult::NotImplemented; }
		virtual bool IsSameSchema (CDatum dSchema) const { return false; }
		virtual bool SetFieldValue (int iRow, int iCol, CDatum dValue) { return false; }
		virtual void SetSeq (SequenceNumber Seq) { }
		virtual EResult SetRow (int iRow, CDatum dRow) { return EResult::NotImplemented; }

		static CDatum CreateColumn (CDatum dType);
		static bool CreateRef (CAEONTypeSystem& TypeSystem, CDatum dTable, SSubset&& Subset, CDatum& retdValue);
		static bool CreateSchema (CAEONTypeSystem& TypeSystem, CDatum dTable, SSubset& Subset, CDatum& retdSchema);
		static bool CreateSchemaFromDesc (CAEONTypeSystem& TypeSystem, CDatum dSchemaDesc, CDatum& retdSchema);
		static bool InsertColumnToSchema (CDatum dSchema, const CString& sName, CDatum dType, int iPos, CDatum& retdSchema, int* retiCol = NULL);
	};

