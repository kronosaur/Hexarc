//	AEONOperators.h
//
//	AEON Operators
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#pragma once

class IAEONOperatorCtx
	{
	public:

		virtual ~IAEONOperatorCtx () { }

		virtual size_t GetMaxArraySize () const { return DEFAULT_MAX_ARRAY_SIZE; }
		virtual size_t GetMaxStringSize () const { return DEFAULT_MAX_STRING_SIZE; }

		static IAEONOperatorCtx Default;

	private:

		static constexpr size_t DEFAULT_MAX_ARRAY_SIZE = 10'000'000;
		static constexpr size_t DEFAULT_MAX_STRING_SIZE = 1'000'000;
	};

class CAEONOperatorTable
	{
	public:

		using OpFunc = CDatum (*) (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx);

		CAEONOperatorTable (OpFunc pOp = NULL)
			{
			for (int i = 0; i < IDatatype::LAST_BASIC_TYPE; i++)
				for (int j = 0; j < IDatatype::LAST_BASIC_TYPE; j++)
					m_Ops[i][j] = pOp;
			}

		OpFunc GetOp (DWORD dwLeftType, DWORD dwRightType) const { return m_Ops[dwLeftType][dwRightType]; }
		void SetOp (DWORD dwLeftType, DWORD dwRightType, OpFunc pOp) { m_Ops[dwLeftType][dwRightType] = pOp; }
		void SetOpSymmetric (DWORD dwLeftType, DWORD dwRightType, OpFunc pOp) { m_Ops[dwLeftType][dwRightType] = pOp; m_Ops[dwRightType][dwLeftType] = pOp; }
		void SetOpLeft (DWORD dwLeftType, OpFunc pOp)
			{
			for (DWORD dwRightType = 0; dwRightType < IDatatype::LAST_BASIC_TYPE; dwRightType++)
				m_Ops[dwLeftType][dwRightType] = pOp;
			}
		void SetOpRight (DWORD dwRightType, OpFunc pOp)
			{
			for (DWORD dwLeftType = 0; dwLeftType < IDatatype::LAST_BASIC_TYPE; dwLeftType++)
				m_Ops[dwLeftType][dwRightType] = pOp;
			}

	private:

		OpFunc m_Ops[IDatatype::LAST_BASIC_TYPE][IDatatype::LAST_BASIC_TYPE];
	};

class CAEONOperatorTableNew
	{
	public:

		using OpFunc = CDatum (*) (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);

		CAEONOperatorTableNew (OpFunc pOp = NULL)
			{
			for (int i = 0; i < IDatatype::LAST_BASIC_TYPE; i++)
				for (int j = 0; j < IDatatype::LAST_BASIC_TYPE; j++)
					m_Ops[i][j] = pOp;
			}

		OpFunc GetOp (DWORD dwLeftType, DWORD dwRightType) const { return m_Ops[dwLeftType][dwRightType]; }
		void SetOp (DWORD dwLeftType, DWORD dwRightType, OpFunc pOp) { m_Ops[dwLeftType][dwRightType] = pOp; }
		void SetOpSymmetric (DWORD dwLeftType, DWORD dwRightType, OpFunc pOp) { m_Ops[dwLeftType][dwRightType] = pOp; m_Ops[dwRightType][dwLeftType] = pOp; }
		void SetOpLeft (DWORD dwLeftType, OpFunc pOp)
			{
			for (DWORD dwRightType = 0; dwRightType < IDatatype::LAST_BASIC_TYPE; dwRightType++)
				m_Ops[dwLeftType][dwRightType] = pOp;
			}
		void SetOpRight (DWORD dwRightType, OpFunc pOp)
			{
			for (DWORD dwLeftType = 0; dwLeftType < IDatatype::LAST_BASIC_TYPE; dwLeftType++)
				m_Ops[dwLeftType][dwRightType] = pOp;
			}

	private:

		OpFunc m_Ops[IDatatype::LAST_BASIC_TYPE][IDatatype::LAST_BASIC_TYPE];
	};

class CAEONUnaryOpTable
	{
	public:

		using OpFunc = CDatum (*) (CDatum dValue, IAEONOperatorCtx& Ctx);

		CAEONUnaryOpTable (OpFunc pOp = NULL)
			{
			for (int i = 0; i < IDatatype::LAST_BASIC_TYPE; i++)
				m_Ops[i] = pOp;
			}

		OpFunc GetOp (DWORD dwType) const { return m_Ops[dwType]; }
		void SetOp (DWORD dwType, OpFunc pOp) { m_Ops[dwType] = pOp; }

	private:

		OpFunc m_Ops[IDatatype::LAST_BASIC_TYPE];
	};

class CAEONOp
	{
	public:

		static CDatum Add (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx = IAEONOperatorCtx::Default) { return m_Add.GetOp(dLeft.GetBasicDatatype(), dRight.GetBasicDatatype())(dLeft, dRight, Ctx); }
		static CDatum CompEqual (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return m_CompEqual.GetOp(dLeft.GetBasicDatatype(), dRight.GetBasicDatatype())(Ctx, dLeft, dRight); }
		static CDatum CompGreaterThan (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return m_CompGreaterThan.GetOp(dLeft.GetBasicDatatype(), dRight.GetBasicDatatype())(Ctx, dLeft, dRight); }
		static CDatum CompGreaterThanOrEqual (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return m_CompGreaterThanOrEqual.GetOp(dLeft.GetBasicDatatype(), dRight.GetBasicDatatype())(Ctx, dLeft, dRight); }
		static CDatum CompIdentical (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return m_CompIdentical.GetOp(dLeft.GetBasicDatatype(), dRight.GetBasicDatatype())(Ctx, dLeft, dRight); }
		static CDatum CompLessThan (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return m_CompLessThan.GetOp(dLeft.GetBasicDatatype(), dRight.GetBasicDatatype())(Ctx, dLeft, dRight); }
		static CDatum CompLessThanOrEqual (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return m_CompLessThanOrEqual.GetOp(dLeft.GetBasicDatatype(), dRight.GetBasicDatatype())(Ctx, dLeft, dRight); }
		static CDatum CompNotEqual (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return m_CompNotEqual.GetOp(dLeft.GetBasicDatatype(), dRight.GetBasicDatatype())(Ctx, dLeft, dRight); }
		static CDatum CompNotIdentical (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return m_CompNotIdentical.GetOp(dLeft.GetBasicDatatype(), dRight.GetBasicDatatype())(Ctx, dLeft, dRight); }
		static CDatum Concatenate (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight) { return m_Concatenate.GetOp(dLeft.GetBasicDatatype(), dRight.GetBasicDatatype())(Ctx, dLeft, dRight); }
		static CDatum Divide (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx = IAEONOperatorCtx::Default) { return m_Divide.GetOp(dLeft.GetBasicDatatype(), dRight.GetBasicDatatype())(dLeft, dRight, Ctx); }
		static CDatum In (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight);
		static CDatum Mod (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx = IAEONOperatorCtx::Default) { return m_Mod.GetOp(dLeft.GetBasicDatatype(), dRight.GetBasicDatatype())(dLeft, dRight, Ctx); }
		static CDatum Multiply (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx = IAEONOperatorCtx::Default) { return m_Multiply.GetOp(dLeft.GetBasicDatatype(), dRight.GetBasicDatatype())(dLeft, dRight, Ctx); }
		static CDatum Negate (CDatum dValue, IAEONOperatorCtx& Ctx = IAEONOperatorCtx::Default) { return m_Negate.GetOp(dValue.GetBasicDatatype())(dValue, Ctx); }
		static CDatum Power (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx = IAEONOperatorCtx::Default) { return m_Power.GetOp(dLeft.GetBasicDatatype(), dRight.GetBasicDatatype())(dLeft, dRight, Ctx); }
		static CDatum Subtract (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx = IAEONOperatorCtx::Default) { return m_Subtract.GetOp(dLeft.GetBasicDatatype(), dRight.GetBasicDatatype())(dLeft, dRight, Ctx); }

		static CDatum CalcAddType (CDatum dLeftType, CDatum dRightType);
		static CDatum CalcCompType (CDatum dLeftType, CDatum dRightType);
		static CDatum CalcConcatType (CDatum dLeftType, CDatum dRightType);
		static CDatum CalcDivideType (CDatum dLeftType, CDatum dRightType);
		static CDatum CalcInType (CDatum dLeftType, CDatum dRightType);
		static CDatum CalcLogicalAndType (CDatum dLeftType, CDatum dRightType);
		static CDatum CalcLogicalOrType (CDatum dLeftType, CDatum dRightType);
		static CDatum CalcModType (CDatum dLeftType, CDatum dRightType);
		static CDatum CalcMultiplyType (CDatum dLeftType, CDatum dRightType);
		static CDatum CalcNegateType (CDatum dType);
		static CDatum CalcPowerType (CDatum dLeftType, CDatum dRightType);
		static CDatum CalcSubtractType (CDatum dLeftType, CDatum dRightType);

		static void CalcSliceParams (CDatum dStart, CDatum dEnd, int iLength, int& retStart, int& retLen);

		static CDatum ExecConcatenateArray_Array (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight, int iAxis = 0);
		static CDatum ExecLeft (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx) { return dLeft; }
		static CDatum ExecNaN (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx) { return CDatum::CreateNaN(); }
		static CDatum ExecRight (CDatum dLeft, CDatum dRight, IAEONOperatorCtx& Ctx) { return dRight; }

		static CDatum ExecUnaryNaN (CDatum dValue, IAEONOperatorCtx& Ctx) { return CDatum::CreateNaN(); }
		static CDatum ExecUnaryNull (CDatum dValue, IAEONOperatorCtx& Ctx) { return CDatum(); }

		static CDatum ErrorTooBig (IInvokeCtx& Ctx);

	private:

		static CAEONOperatorTable m_Add;
		static CAEONOperatorTableNew m_CompEqual;
		static CAEONOperatorTableNew m_CompGreaterThan;
		static CAEONOperatorTableNew m_CompGreaterThanOrEqual;
		static CAEONOperatorTableNew m_CompIdentical;
		static CAEONOperatorTableNew m_CompLessThan;
		static CAEONOperatorTableNew m_CompLessThanOrEqual;
		static CAEONOperatorTableNew m_CompNotEqual;
		static CAEONOperatorTableNew m_CompNotIdentical;
		static CAEONOperatorTableNew m_Concatenate;
		static CAEONOperatorTable m_Divide;
		static CAEONOperatorTable m_Mod;
		static CAEONOperatorTable m_Multiply;
		static CAEONUnaryOpTable m_Negate;
		static CAEONOperatorTable m_Power;
		static CAEONOperatorTable m_Subtract;
	};
