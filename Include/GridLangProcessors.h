//	GridLangProcessors.h
//
//	GridLang Classes
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#pragma once

class CGLArrayMakeProcessor : public TExternalDatum<CGLArrayMakeProcessor>
	{
	public:

		CGLArrayMakeProcessor (CDatum dStart, CDatum dEnd, CDatum dStep, CDatum dMakeFunc, int iMaxSize);

		static const CString &StaticGetTypename (void);

		bool Process (CDatum dSelf, SAEONInvokeResult& retResult);
		bool ProcessContinues (CDatum dSelf, CDatum dResult, SAEONInvokeResult& retResult);

	protected:

		virtual void OnMarked (void) override;

	private:

		int CalcSize (void) const;
		bool IsDone () const;
		void SetNextValue ();

		CDatum::Types m_iIndexType = CDatum::typeNil;
		CDatum m_dStart;
		CDatum m_dEnd;
		CDatum m_dStep;
		CDatum m_dMakeFunc;
		int m_iMaxSize;

		CDatum m_dValue;
		CDatum m_dResult;
		bool m_bNegative = false;
		int m_iPos = -1;
	};

class CGLArrayReduceProcessor : public TExternalDatum<CGLArrayReduceProcessor>
	{
	public:

		CGLArrayReduceProcessor (CDatum dArray, CDatum dInitialValue, CDatum dOptions, CDatum dFunc) :
				m_dArray(dArray),
				m_dOptions(dOptions),
				m_dFunc(dFunc),
				m_dResult(dInitialValue)
			{ }

		static const CString& StaticGetTypename (void);

		bool Process (CDatum dSelf, SAEONInvokeResult& retResult);
		bool ProcessContinues (CDatum dSelf, CDatum dResult, SAEONInvokeResult& retResult);

	protected:

		virtual void OnMarked () override;

	private:

		CDatum m_dArray;
		CDatum m_dOptions;
		CDatum m_dFunc;

		CDatum m_dResult;
		int m_iPos = -1;
	};

class CGLFilterProcessor : public TExternalDatum<CGLFilterProcessor>
	{
	public:

		CGLFilterProcessor (CAEONTypeSystem &TypeSystem, CDatum dList, CDatum dOptions, CDatum dFunc) :
				m_TypeSystem(TypeSystem),
				m_dList(dList),
				m_dOptions(dOptions),
				m_dFunc(dFunc)
			{ }

		static const CString &StaticGetTypename (void);

		bool Process (CDatum dSelf, SAEONInvokeResult& retResult);
		bool ProcessContinues (CDatum dSelf, CDatum dResult, SAEONInvokeResult& retResult);

	protected:

		virtual void OnMarked (void) override;

	private:

		CDatum CreateResult ();
		void ProcessExpression ();

		CAEONTypeSystem &m_TypeSystem;
		CDatum m_dList;
		CDatum m_dOptions;
		CDatum m_dFunc;

		TArray<int> m_Cols;		//	Optional
		int m_iIndex = -1;
		TArray<int> m_Elements;
		TArray<TArray<int>> m_GroupIndex;
	};

class CGLGroupByProcessor : public TExternalDatum<CGLGroupByProcessor>
	{
	public:

		enum class EResultType
			{
			array,
			dictionary,
			table,
			};

		struct SOptions
			{
			EResultType iResultType = EResultType::array;
			CString sKeyCol;
			};

		CGLGroupByProcessor (CAEONTypeSystem &TypeSystem, CDatum dList, CDatum dFunc, const SOptions& Options) :
				m_TypeSystem(TypeSystem),
				m_dList(dList),
				m_dFunc(dFunc),
				m_Options(Options)
			{ }

		static const CString &StaticGetTypename (void);

		bool Process (CDatum dSelf, SAEONInvokeResult& retResult);
		bool ProcessContinues (CDatum dSelf, CDatum dResult, SAEONInvokeResult& retResult);

	protected:

		virtual void OnMarked (void) override;

	private:

		void AddToGroup (CDatum dGroup, int iRow);
		CDatum CreateIndexedTable ();
		CDatum CreateResult ();
		CDatum CreateResultArray ();
		CDatum CreateResultDictionary ();
		bool IsColumnValue () const { return m_iListType == CDatum::typeTable && !m_Options.sKeyCol.IsEmpty() && m_dFunc.IsNil(); }
		bool ProcessTableByColumnValue (const CString& sColName, CDatum& retResult);
		bool ProcessTableByExpression (const CAEONExpression& Expr, CDatum& retResult);

		CAEONTypeSystem& m_TypeSystem;
		SOptions m_Options;
		CDatum m_dList;
		CDatum m_dFunc;

		CDatum::Types m_iListType = CDatum::typeNil;
		int m_iIndex = -1;
		TArray<TArray<int>> m_Groups;
		TSortMap<CDatum, int, CKeyCompareEquivalent<CDatum>> m_Map;

		CDatum m_dKeyType;
		CDatum m_dElementType;
	};

class CGLAddColumnProcessor : public TExternalDatum<CGLAddColumnProcessor>
	{
	public:

		CGLAddColumnProcessor (CDatum dTable, CDatum dFunc, CDatum dSchema) :
				m_dTable(dTable),
				m_dFunc(dFunc),
				m_dSchema(dSchema)
			{ }

		static const CString& StaticGetTypename (void);

		bool Process (CDatum dSelf, SAEONInvokeResult& retResult);
		bool ProcessContinues (CDatum dSelf, CDatum dResult, SAEONInvokeResult& retResult);

	protected:

		virtual void OnMarked (void) override;

	private:

		void AddRow (CDatum dRow);
		CDatum CreateResult ();
		bool ProcessMapColExpression (const CAEONMapColumnExpression& MapColExpr, CDatum& retResult);

		CDatum m_dTable;
		CDatum m_dSchema;
		CDatum m_dFunc;

		int m_iIndex = -1;
		TSortMap<CString, CDatum, CKeyCompareEquivalent<CString>> m_Columns;	//	Column name to values
	};
