//	Processors.h
//
//	Compute Processors
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#pragma once

class CArrayMapProcessor : public TExternalDatum<CArrayMapProcessor>
	{
	public:

		CArrayMapProcessor (CDatum dArray, CDatum dOptions, CDatum dMapFunc);

		static const CString &StaticGetTypename (void);

		bool Process (CDatum dSelf, SAEONInvokeResult& retResult);
		bool ProcessContinues (CDatum dSelf, CDatum dResult, SAEONInvokeResult& retResult);

	protected:

		virtual void OnMarked (void) override;

	private:

		CDatum m_dArray;
		CDatum m_dOptions;
		CDatum m_dMapFunc;
		bool m_bAllowNull = false;

		CDatum m_dResult;
		int m_iPos = -1;
	};

class CDictionaryMapProcessor : public TExternalDatum<CDictionaryMapProcessor>
	{
	public:

		CDictionaryMapProcessor (CDatum dDictionary, CDatum dOptions, CDatum dMapFunc, int iFuncArgs, CDatum dResultType);

		static const CString &StaticGetTypename (void);

		bool Process (CDatum dSelf, SAEONInvokeResult &retResult);
		bool ProcessContinues (CDatum dSelf, CDatum dResult, SAEONInvokeResult &retResult);

	protected:

		virtual void OnMarked (void) override;

	private:

		CDatum m_dDictionary;
		CDatum m_dOptions;
		CDatum m_dMapFunc;
		int m_iFuncArgs = 0;
		bool m_bAllowNull = false;

		CDatum m_dResult;
		int m_iPos = -1;
	};

class CTableMapProcessor : public TExternalDatum<CTableMapProcessor>
	{
	public:

		CTableMapProcessor (CAEONTypeSystem &TypeSystem, CDatum dTable, CDatum dOptions, CDatum dMapFunc);

		static const CString &StaticGetTypename (void);

		bool Process (CDatum dSelf, SAEONInvokeResult& retResult);
		bool ProcessContinues (CDatum dSelf, CDatum dResult, SAEONInvokeResult& retResult);

	protected:

		virtual void OnMarked (void) override;

	private:

		CDatum CreateSchemaFromStruct (CDatum dStruct);
		bool ProcessMapColExpression (const CAEONMapColumnExpression& MapColExpr, CDatum& retResult);
		void NextCol ();

		CAEONTypeSystem &m_TypeSystem;
		CDatum m_dTable;
		CDatum m_dOptions;
		CDatum m_dMapFunc;

		CDatum m_dRow;
		CDatum m_dRowResult;
		CDatum m_dResult;
		int m_iRow = -1;
		int m_iCol = -1;
	};

class CTensorMapProcessor : public TExternalDatum<CTensorMapProcessor>
	{
	public:

		enum class EResultType
			{
			Unknown,

			Array,
			Tensor,
			};

		CTensorMapProcessor (CDatum dTensor, CDatum dOptions, CDatum dMapFunc, int iFuncArgs, CDatum dResultType);

		static const CString& StaticGetTypename (void);

		bool Process (CDatum dSelf, SAEONInvokeResult& retResult);
		bool ProcessContinues (CDatum dSelf, CDatum dResult, SAEONInvokeResult& retResult);

		static bool Impl_TensorMap (IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retdResult);
		static EResultType ParseResultType (CStringView sType);

	protected:

		virtual void OnMarked (void) override;

	private:

		static CDatum CalcTensorMapType (IInvokeCtx& Ctx, CDatum dTensor, CDatum dMapFunc, EResultType iResultType, int& retiArgs);

		CDatum m_dTensor;
		CDatum m_dOptions;
		CDatum m_dMapFunc;
		int m_iFuncArgs = 0;
		EResultType m_iResultType = EResultType::Unknown;
		bool m_bAllowNull = false;

		CDatum m_dResult;
		CBuffer m_Pos;
	};
