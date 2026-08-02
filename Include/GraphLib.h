//	GraphLib.h
//
//	GraphLib class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "LuminousAEON.h"

enum class EGraphLine
	{
	Default =				0,

	Solid =					1,
	Dashed =				2,
	Dotted =				3,
	};

enum class EGraphMarker
	{
	Default =				0,

	Circle =				100,
	Diamond =				101,
	Square =				102,
	TriangleDown =			103,
	TriangleLeft =			104,
	TriangleRight =			105,
	TriangleUp =			106,
	};

enum class EGraphSource
	{
	None,									//	No data
	Error,									//	Invalid data source (sField is error)

	Array1DIndex,							//	Use the index of a 1D array
	Array1DValue,							//	Take the value of a 1D array
	ConstDouble,							//	m_rSelector is value
	ConstInt,								//	m_iSelector is value
	ConstString,							//	m_sSelector is value
	ConstArrayDouble,						//	m_ArrayDataDouble is value
	ConstArrayInt,							//	m_ArrayDataDouble is value
	ConstArrayString,						//	m_ArrayDataString is value
	TableColumn,							//	m_sSelector is table column
	};

enum class EGraphType
	{
	Unknown,

	Area,
	Bar,
	Line,
	Scatter,
	Step,
	};

class CGraphSource
	{
	public:
		CGraphSource () { }
		CGraphSource (EGraphSource iType, const CString &sDataID);
		CGraphSource (EGraphSource iType, const CString &sDataID, int iSelector);
		CGraphSource (EGraphSource iType, const CString &sDataID, double rSelector);
		CGraphSource (EGraphSource iType, const CString &sDataID, const CString &sSelector);
		CGraphSource (EGraphSource iType, const CString &sDataID, CDatum dSelector);
		CGraphSource (EGraphSource iType, const CString &sDataID, TArray<double>&& Array);
		CGraphSource (EGraphSource iType, const CString &sDataID, TArray<CString>&& Array);

		CDatum GetValue (int iIndex) const;
		int GetValueCount () const;
		CString GetValueFormat () const;
		EGraphSource GetType () const { return m_iType; }
		bool Resolve (const CString &sSourceName, CDatum dData, CString *retsError = NULL);

	private:
		EGraphSource m_iType = EGraphSource::None;
		CString m_sDataID;
		int m_iSelector = 0;
		double m_rSelector = 0.0;
		CString m_sSelector;
		TArray<double> m_ArrayDataDouble;
		TArray<CString> m_ArrayDataString;

		//	NOTE: These values are cached inside of Resolve. Do not call 
		//	GetValue or GetValueCount without first calling Resolve, and do not
		//	call them after exiting the function that called Resolve.

		CDatum m_dData;
		int m_iSubData = -1;
	};

class CGraphAxisDesc
	{
	public:

		enum class EType
			{
			Unknown,
			None,

			Continuous,
			Categories,
			};

		struct SRowRef
			{
			int iSeries = 0;
			int iRow = 0;
			};

		enum class ECategoryOrder
			{
			Unknown,
			None,							//	Same order as data

			Ascending,						//	Ascending by sort key
			Descending,						//	Descending by sort key
			};

		struct SCategoryEntry
			{
			CString sLabel;
			int iSortOrder = 0;				//	The position of the category in the m_SortedCategories array.
			double rPos = 0.0;				//	For bar-graphs, this is the center of the bar.
			TArray<SRowRef> Rows;			//	For bar-graphs, this is the rows at this entry.
			};

		CGraphAxisDesc () { }

		void Accumulate (const CGraphAxisDesc &Axis);
		CDatum AsEChartsDesc (bool bIncludeResolvedRange = false, int iSplitNumber = 0) const;
		CDatum AsRenderDesc () const;
		const SCategoryEntry& GetCategory (int iIndex) const { return m_Categories[m_SortedCategories[iIndex]]; }
		int GetCategoryCount () const { return m_Categories.GetCount(); }
		const CGraphSource& GetData () const { return m_Value; }
		double GetMax () const { return (m_iResolvedType != EType::Unknown ? m_rResolvedMax : GetFixedMax()); }
		double GetMin () const { return (m_iResolvedType != EType::Unknown ? m_rResolvedMin : GetFixedMin()); }
		bool GetPos (CDatum dValue, double& retrPos) const;
		EType GetType () const { return m_iResolvedType; }
		bool InitFromDesc (CDatum dDesc, CString *retsError = NULL);
		bool InRange (double rValue) const;
		bool IsMaxFixed () const { return m_bFixedMax; }
		bool IsMinFixed () const { return m_bFixedMin; }
		bool IsCompatible (const CGraphAxisDesc &Axis) const;
		bool ResolveSources (const CString &sSourceName, CDatum dData, CString *retsError = NULL);
		void ResolveType (std::function<bool(int)> fnInclude = nullptr);
		void SetBarDistance (double rValue) { m_rBarDistance = rValue; }
		void SetCategories (TSortMap<CString, SCategoryEntry>&& Categories) { m_Categories = std::move(Categories); InitCategoryOrder(); }
		void SetNoPaddingMax (bool bValue = true) { m_bNoPaddingMax = bValue; }
		void SetNoPaddingMin (bool bValue = true) { m_bNoPaddingMin = bValue; }
		void SetRange (double rMin, double rMax);

		static CString CalcCategoryKey (CDatum dValue) { return dValue.AsString(); }
		static EType ParseType (const CString &sType);

	private:

		static constexpr int MAX_SOURCE_SAMPLE = 100;

		double GetFixedMax () const { return m_rFixedMax; }
		double GetFixedMin () const { return m_rFixedMin; }
		void InitCategoryOrder ();

		CGraphSource m_Value;
		EType m_iType = EType::None;

		ECategoryOrder m_iCategoryOrder = ECategoryOrder::None;
		CString m_sFormat;
		double m_rFixedMax = 0.0;
		double m_rFixedMin = 0.0;
		bool m_bFixedMin = false;
		bool m_bFixedMax = false;
		bool m_bLogScale = false;

		//	The following fields are only valid after we've resolved the source.

		TSortMap<CString, SCategoryEntry> m_Categories;
		TArray<int> m_SortedCategories;			//	Indices into m_Categories sorted by position.
		EType m_iResolvedType = EType::Unknown;
		double m_rResolvedMax = 0.0;
		double m_rResolvedMin = 0.0;
		double m_rBarDistance = 0.0;
		int m_iElementsToDraw = 0;
		CStringFormat m_ResolvedFormat;
		bool m_bNoPaddingMax = false;
		bool m_bNoPaddingMin = false;
	};

class CGraphLineDesc
	{
	public:

		enum class EType
			{
			Unknown,
			None,

			Horizontal,
			Vertical,
			};

		enum class ELabelPos
			{
			Unknown,
			None,
			
			Start,
			Middle,
			End,

			InsideStartTop,
			InsideStartBottom,
			InsideMiddleTop,
			InsideMiddleBottom,
			InsideEndTop,
			InsideEndBottom,
			};

		void AccumulateEChartsDesc (CDatum dData) const;
		bool InitFromDesc (CDatum dDesc, CString *retsError = NULL);

		static CString AsID (EType iType);
		static CString AsID (ELabelPos iPos);
		static EType ParseType (CStringView sValue);
		static ELabelPos ParseLabelPos (CStringView sValue);

	private:

		EType m_iType = EType::None;
		CString m_sName;
		double m_rValue = 0.0;

		CString m_sLabel;
		ELabelPos m_iLabelPos = ELabelPos::End;
	};

class CGraphColorDesc
	{
	public:

		enum class EType
			{
			Unknown,
			None,

			Constant,
			Continuous,
			Categories,
			};

		CGraphColorDesc () { }

		CLuminousColor GetColor (int iRow) const;
		const CGraphSource& GetData () const { return m_Value; }
		EType GetType () const { return (m_iResolvedType != EType::Unknown ? m_iResolvedType : m_iType); }
		bool InitFromDesc (CDatum dDesc, CString* retsError = NULL);
		bool ResolveSources (const CString& sSourceName, CDatum dData, CString* retsError = NULL);

		static EType ParseType (const CString &sType);

	private:

		bool InitColorTable (CDatum dColorTable, CString* retsError = NULL);

		CGraphSource m_Value;
		EType m_iType = EType::None;
		EType m_iResolvedType = EType::Unknown;
		TSortMap<CString, CLuminousColor> m_ColorTable;
		TSortMap<double, CLuminousColor> m_ColorTableContinuous;
	};

struct SGraphSizeDesc
	{
	CGraphSource Value;
	};

struct SGraphShapeDesc
	{
	CGraphSource Value;
	};

struct SGraphLineConnectionDesc
	{
	CGraphSource LineBy;
	CGraphSource OrderBy;

	CGraphColorDesc Color;
	int iWidth = 2;
	};

//	IGraphGenerator
//
//	The IGraphGenerator class contains a description of a graph suitable for 
//	rendering to the UI. The description is an ordered list of elements, each
//	of which is a primitive, such as a marker, a line, a bar, etc.
//
//	NOTE: This class represents the content area of the chart, excluding the 
//	title, legend, and axes. For tiled or paned charts, we use a collection of
//	CGraphDesc objects.

class IGraphGenerator
	{
	public:
		IGraphGenerator (const CString &sID) : 
				m_sID(sID)
			{ }

		struct SRenderOptions
			{
			};

		static TUniquePtr<IGraphGenerator> Create (const CString &sType, CDatum dDesc, CString *retsError = NULL);
		virtual ~IGraphGenerator () { }

		CDatum RenderAsDatum (const SRenderOptions& Options) const { return OnRenderAsDatum(Options); }
		CDatum RenderAsECharts (const SRenderOptions& Options) const { return OnRenderAsECharts(Options); }
		void ResetDeltas () { OnResetDeltas(); }
		void SetData (CDatum dData) { OnSetData(dData); }

		static CString GetStyleID (EGraphLine iLine);
		static CString GetStyleID (EGraphMarker iMarker);
		static bool ParseDataSource (CDatum dValue, CGraphSource& retSource, CString *retsError = NULL);
		static bool ParseDataSourceList (CDatum dValue, TArray<CGraphSource>& retSource, CString *retsError = NULL);
		static EGraphSource ParseDataSourceType (const CString &sType);
		static EGraphType ParseGraphType (const CString &sType, EGraphType iDefault = EGraphType::Unknown);
		static bool ParseLineConnectionDesc (CDatum dDesc, SGraphLineConnectionDesc &retDesc, CString *retsError = NULL);
		static bool ParseShapeDesc (CDatum dDesc, SGraphShapeDesc &retDesc, CString *retsError = NULL);
		static bool ParseSizeDesc (CDatum dDesc, SGraphSizeDesc &retDesc, CString *retsError = NULL);

	protected:

		static constexpr int DEFAULT_LINE_WIDTH = 2;

	private:

		virtual CDatum OnRenderAsDatum (const SRenderOptions& Options) const = 0;
		virtual CDatum OnRenderAsECharts (const SRenderOptions& Options) const = 0;
		virtual void OnResetDeltas () { }
		virtual void OnSetData (CDatum dData) { }

		CString m_sID;
	};

