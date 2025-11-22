//	AEONExpression.h
//
//	AEON Expression
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#pragma once

class CAEONExpression : public IComplexDatum
	{
	public:

		enum class EOp
			{
			//	NOTE: These values are persisted; do not change them.

			None					= 0,

			True					= 1,
			False					= 2,
			Literal					= 3,
			Column					= 4,

			EqualTo					= 5,
			NotEqualTo				= 6,
			GreaterThan				= 7,
			GreaterThanOrEqualTo	= 8,
			In						= 9,
			LessThan				= 10,
			LessThanOrEqualTo		= 11,

			And						= 12,
			If						= 13,
			Or						= 14,
			Not						= 15,

			Add						= 16,
			Divide					= 17,
			Mod						= 18,
			Multiply				= 19,
			Negate					= 20,
			Power					= 21,
			Subtract				= 22,

			All						= 23,
			Any						= 24,
			Average					= 25,
			Count					= 26,
			First					= 27,
			Max						= 28,
			Median					= 29,
			Min						= 30,
			Sum						= 31,
			UniqueCount				= 32,

			Clean					= 33,
			DateTime				= 34,
			Integer					= 35,
			Lowercase				= 36,
			Real					= 37,
			String					= 38,
			TimeSpan				= 40,
			Uppercase				= 41,
			Number					= 42,
			Find					= 43,
			Left					= 44,
			Right					= 45,
			Length					= 46,
			Slice					= 47,
			Deref					= 48,
			Member					= 49,
			Array					= 50,
			UniqueArray				= 51,
			ErrorLiteral			= 52,

			Abs						= 53,
			Ceil					= 54,
			Floor					= 55,
			Round					= 56,
			Sign					= 57,
			StdDev					= 58,
			StdError				= 59,
			};

		struct SNode
			{
			EOp iOp = EOp::None;
			int iLeft = -1;				//	Index of left child
			int iRight = -1;			//	Index of right child

			int iDataID = -1;			//	Data ID for literal, column values
			DWORD dwFlags = 0;
			};

		static CDatum Create (const CAEONExpression& Expr) { return CDatum(new CAEONExpression(Expr)); }
		static CDatum CreateBinaryOp (EOp iOp, const CAEONExpression& Left, const CAEONExpression& Right);
		static CDatum CreateBinaryOp (EOp iOp, const CAEONExpression& Left, CDatum dLiteral);
		static CDatum CreateBinaryOp (EOp iOp, CDatum dLiteral, const CAEONExpression& Right);
		static CDatum CreateColumnRef (const CString& sField);
		static CDatum CreateError (CStringView sError);
		static CDatum CreateFalse ();
		static CDatum CreateIf (const CAEONExpression& Condition, const CAEONExpression& TrueExpr, const CAEONExpression& FalseExpr);
		static CDatum CreateLiteral (CDatum dValue);
		static CDatum CreateTernaryOp (EOp iOp, const CAEONExpression& Arg1, const CAEONExpression& Arg2, const CAEONExpression& Arg3);
		static CDatum CreateTrue ();
		static CDatum CreateUnaryOp (EOp iOp, const CAEONExpression& Expr);
		static CDatum CreateZeroaryOp (EOp iOp);
		static CDatum Parse (CStringView sExpr);

		int GetColumnCount () const { return m_Columns.GetCount(); }
		CStringView GetColumnID (int iColID) const { if (iColID < 0 || iColID >= m_Columns.GetCount()) throw CException(errFail); return m_Columns[iColID]; }
		int GetLiteralCount () const { return m_Literals.GetCount(); }
		CDatum GetLiteral (int iLiteralID) const { if (iLiteralID < 0 || iLiteralID >= m_Literals.GetCount()) throw CException(errFail); return m_Literals[iLiteralID]; }
		int GetNodeCount () const { return m_Nodes.GetCount(); }
		const SNode& GetNode (int iNodeID) const { if (iNodeID < 0 || iNodeID >= m_Nodes.GetCount()) throw CException(errFail); return m_Nodes[iNodeID]; }
		int GetRootIndex () const { return m_iRoot; }
		const SNode& GetRootNode () const { return (m_iRoot >= 0 ? m_Nodes[m_iRoot] : m_NullNode); }
		bool IsEmpty () const { return (m_iRoot == -1); }
		bool IsError (CString* retsError = NULL) const;
		static EOp ParseFunctionName (CStringView sSymbol);

		//	IComplexDatum

		virtual CString AsString (void) const override;
		virtual size_t CalcMemorySize (void) const override;
		virtual DWORD GetBasicDatatype () const override { return IDatatype::EXPRESSION; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeExpression; }
		virtual int GetCount (void) const override { return 0; }
		virtual CDatum GetDatatype () const override { return CAEONTypes::Get(IDatatype::EXPRESSION); }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual const CAEONExpression* GetQueryInterface () const override { return this; }
		virtual const CString &GetTypename (void) const override;
		virtual bool IsArray (void) const override { return false; }
		virtual bool IsImmutable () const override { return true; }
		virtual bool IsNil (void) const override;
		virtual CDatum MathAbs () const { return CreateUnaryOp(EOp::Abs, *this); }
		virtual CDatum MathAverage () const { return CreateUnaryOp(EOp::Average, *this); }
		virtual CDatum MathCeil () const { return CreateUnaryOp(EOp::Ceil, *this); }
		virtual CDatum MathFloor () const { return CreateUnaryOp(EOp::Floor, *this); }
		virtual CDatum MathRound () const { return CreateUnaryOp(EOp::Round, *this); }
		virtual CDatum MathSign () const { return CreateUnaryOp(EOp::Sign, *this); }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);

		static const CAEONExpression Null;

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual void OnMarked (void) override;

	private:

		static CString AsID (EOp iOp);
		static EOp AsOp (const CString& sValue);
		void CopyNode (int iDestID, const CAEONExpression& Src, int iSrcID, int iOffset);
		static EOp FlipOp (EOp iOp);
		static bool NeedsParens (const SNode& Node);
		void SerializeGridLang (IByteStream &Stream, const SNode& Node) const;
		void SerializeGridLangBinaryOp (IByteStream &Stream, const SNode& Node, CStringView sOp) const;
		void SerializeGridLangFunction (IByteStream &Stream, const SNode& Node, CStringView sFunc) const;

		TArray<SNode> m_Nodes;		//	Nodes in the expression tree
		TArray<CString> m_Columns;
		TArray<CDatum> m_Literals;
		int m_iRoot = -1;			//	Index of root node

		static const SNode m_NullNode;
	};

class CAEONMapColumnExpression : public TExternalDatum<CAEONMapColumnExpression>
	{
	public:

		CAEONMapColumnExpression () { }

		static CDatum Create (CDatum dType, CDatum dExpressions) { return CDatum(new CAEONMapColumnExpression(dType, dExpressions)); }
		static CDatum CreateFromStruct (CDatum dStruct);
		static CDatum CreateIdentity (CDatum dTable);
		static CDatum CreateSummary (CDatum dTable, CStringView sColName, CDatum dColExpr);
		static CDatum CreateWithGroups (CDatum dType, CDatum dExpressions, CDatum dTable);
		static TArray<IDatatype::SMemberDesc> GetMembers (void);
		static const CString& StaticGetTypename (void);

		int GetColCount () const { return m_dExpressions.GetCount(); }
		CDatum GetColExpression (int iIndex) const { return m_dExpressions.GetElement(iIndex); }
		CDatum GetSchema () const { return m_dType; }

		//	IComplexDatum

		virtual CString AsString () const override { return NULL_STR; }
		virtual size_t CalcMemorySize () const override;
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeAEONObject; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString& sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString& sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl(CDatum dObj, const CString& sMethod, IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override 
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual bool IsImmutable () const override { return true; }
		virtual bool IsNil () const override { return m_dExpressions.GetCount() == 0; }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return OpCompare(iValueType, dValue); }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override;
		virtual void SetElement (const CString& sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override { return CalcMemorySize() + sizeof(DWORD); }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString& sTypename, IByteStream& Stream) override;
		virtual void OnMarked (void) override { m_dType.Mark(); m_dExpressions.Mark(); }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream& Stream) const override;

	private:

		CAEONMapColumnExpression (CDatum dType, CDatum dExpressions) :
				m_dType(dType),
				m_dExpressions(dExpressions)
			{ }

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		CDatum m_dType;
		CDatum m_dExpressions;	//	Expressions for each column

		static TDatumPropertyHandler<CAEONMapColumnExpression> m_Properties;
		static TDatumMethodHandler<CAEONMapColumnExpression> m_Methods;
	};
