//	HexeVM.h
//
//	Hexe EXecution Environment
//	Copyright (c) 2011 GridWhale Corporation. All Rights Reserved.

#pragma once

class CHexeColumnExpressionEval;

typedef DWORD OPCODE;

//	OpCodes --------------------------------------------------------------------
//
//	A code block consists of an array of DWORDs. Each DWORD is either an opcode
//	or data.
//
//	An opcode uses the high-order byte as the opcode. The remaining bits (24)
//	are used as data, if necessary.
//
//	Data DWORDs are either a CDatum or an opcode-specific type (e.g., a jump
//	offset).

enum EOpCodes
	{
	opNoOp =				0x00000000,		//	No operation

	opDefine =				0x01000000,		//	Defines variable in current environment
	opPushIntShort =		0x02000000,		//	Pushes operand as int
	opPushStr =				0x03000000,		//	Pushes operand as string block
	opPushStrNull =			0x04000000,		//	Pushes CDatum("")
	opPushGlobal =			0x05000000,		//	Pushes the global variable
	opMakeFunc =			0x06000000,
	opEnterEnv =			0x07000000,
	opDefineArg =			0x08000000,
	opExitEnv =				0x09000000,
	opReturn =				0x0a000000,
	opPushLocal =			0x0b000000,
	opMakeEnv =				0x0c000000,
	opCall =				0x0d000000,
	opPushInt =				0x0e000000,		//	Pushes operand as int
	opAdd =					0x0f000000,
	opDivide =				0x10000000,
	opMultiply =			0x11000000,
	opSubtract =			0x12000000,
	opJump =				0x13000000,
	opJumpIfNil =			0x14000000,
	opPushNil =				0x15000000,		//	Pushes CDatum()
	opPushTrue =			0x16000000,		//	Pushes CDatum(true)
	opIsEqual =				0x17000000,
	opIsLess =				0x18000000,
	opIsGreater =			0x19000000,
	opIsLessOrEqual =		0x1a000000,
	opIsGreaterOrEqual =	0x1b000000,
	opMakeArray =			0x1c000000,
	opPop =					0x1d000000,
	opHexarcMsg =			0x1e000000,
	opMakeApplyEnv =		0x1f000000,
	opMakePrimitive =		0x20000000,
	opPushDatum =			0x21000000,
	opError =				0x22000000,
	opSetLocal =			0x23000000,
	opSetGlobal =			0x24000000,
	opPopLocal =			0x25000000,
	opMakeBlockEnv =		0x26000000,
	opNot =					0x27000000,
	opSetLocalItem =		0x28000000,
	opSetGlobalItem =		0x29000000,
	opMakeStruct =			0x2a000000,
	opPushLocalLength =		0x2b000000,
	opPushLocalItem =		0x2c000000,
	opCallLib =				0x2d000000,
	opIncLocalInt =			0x2e000000,
	opMakeFlagsFromArray =	0x2f000000,
	opMapResult =			0x30000000,
	opJumpIfNilNoPop =		0x31000000,
	opJumpIfNotNilNoPop =	0x32000000,
	opIsNotEqual =			0x33000000,
	opPushArrayItem =		0x34000000,
	opSetArrayItem =		0x35000000,
	opPower =				0x36000000,
	opMod =					0x37000000,
	opCompareStep =			0x38000000,
	opIncStep =				0x39000000,
	opPushType =			0x3a000000,
	opPushFalse =			0x3b000000,
	opMakeObject =			0x3c000000,
	opPushObjectItem =		0x3d000000,
	opPushObjectMethod =	0x3e000000,
	opMakeMethodEnv =		0x3f000000,
	opMakeAsType =			0x40000000,
	opMakeDatatype =		0x41000000,
	opPushCoreType =		0x42000000,
	opPushNaN =				0x43000000,
	opSetObjectItem =		0x44000000,
	opMakeFunc2 =			0x45000000,
	opIsIdentical =			0x46000000,
	opIsNotIdentical =		0x47000000,
	opPushLiteral =			0x48000000,
	opDefineArgFromCode =	0x49000000,
	opMakeAsTypeCons =		0x4a000000,
	opPushLocalL0 =			0x4b000000,
	opPopLocalL0 =			0x4c000000,
	opSetLocalL0 =			0x4d000000,
	opIncLocalL0 =			0x4e000000,
	opMakeLocalEnv =		0x4f000000,
	opAdd2 =				0x50000000,
	opDivide2 =				0x51000000,
	opMultiply2 =			0x52000000,
	opSubtract2 =			0x53000000,
	opPushArrayItemI =		0x54000000,
	opSetArrayItemI =		0x55000000,

	opMutateArrayItemAdd =	0x56000000,
	opMutateArrayItemSubtract =	0x57000000,
	opMutateArrayItemMultiply =	0x58000000,
	opMutateArrayItemDivide =	0x59000000,
	opMutateObjectItemAdd =	0x5a000000,
	opMutateObjectItemSubtract =	0x5b000000,
	opMutateObjectItemMultiply =	0x5c000000,
	opMutateObjectItemDivide =	0x5d000000,
	opMutateGlobalAdd =		0x5e000000,
	opMutateGlobalSubtract =	0x5f000000,
	opMutateGlobalMultiply =	0x60000000,
	opMutateGlobalDivide =	0x61000000,
	opMutateLocalAdd =		0x62000000,
	opMutateLocalSubtract =	0x63000000,
	opMutateLocalMultiply =	0x64000000,
	opMutateLocalDivide =	0x65000000,

	opCompareForEach =		0x66000000,
	opSetForEachItem =		0x67000000,
	opExitEnvAndJumpIfNil =	0x68000000,
	opInc =					0x69000000,
	opMakeSpread =			0x6A000000,
	opConcat =				0x6B000000,
	opDebugBreak =			0x6C000000,
	opNewObject =			0x6D000000,

	opMutateArrayItemConcat =	0x6E000000,
	opMutateObjectItemConcat =	0x6F000000,
	opMutateGlobalConcat =	0x70000000,
	opMutateLocalConcat =	0x71000000,

	opMutateArrayItemMod =	0x72000000,
	opMutateObjectItemMod =	0x73000000,
	opMutateGlobalMod =		0x74000000,
	opMutateLocalMod =		0x75000000,

	opMutateArrayItemPower =	0x76000000,
	opMutateObjectItemPower =	0x77000000,
	opMutateGlobalPower =	0x78000000,
	opMutateLocalPower =	0x79000000,

	opIsEqualMulti =		0x7A000000,
	opIsNotEqualMulti =		0x7B000000,
	opIsLessMulti =			0x7C000000,
	opIsGreaterMulti =		0x7D000000,
	opIsLessOrEqualMulti =	0x7E000000,
	opIsGreaterOrEqualMulti =	0x7F000000,
	opIsEqualInt =			0x80000000,
	opIsNotEqualInt =		0x81000000,
	opIsLessInt =			0x82000000,
	opIsGreaterInt =		0x83000000,
	opIsLessOrEqualInt =	0x84000000,
	opIsGreaterOrEqualInt =	0x85000000,
	opExitEnvAndJumpIfGreaterInt =	0x86000000,
	opExitEnvAndJumpIfGreaterOrEqualInt =	0x87000000,	//	Unused?
	opPushTensorItemI =		0x88000000,
	opMakeTensorType =		0x89000000,
	opLoopIncAndJump =		0x8A000000,

	opMakeRange =			0x8B000000,
	opNegate =				0x8C000000,
	opMakeEmptyArray =		0x8D000000,
	opMakeEmptyArrayAsType =	0x8E000000,
	opAppendToArray =		0x8F000000,
	opMakeEmptyStruct =		0x90000000,
	opSetObjectItem2 =		0x91000000,
	opPopDeep =				0x92000000,
	opExitEnvAndReturn =	0x93000000,
	opMakeTensor =			0x94000000,
	opPushTensorItem =		0x95000000,
	opSetTensorItem =		0x96000000,
	opMutateTensorItemAdd = 0x97000000,
	opMutateTensorItemSubtract = 0x98000000,
	opMutateTensorItemMultiply = 0x99000000,
	opMutateTensorItemDivide = 0x9A000000,
	opMutateTensorItemConcat = 0x9B000000,
	opMutateTensorItemMod = 0x9C000000,
	opMutateTensorItemPower = 0x9D000000,
	opAddInt =				0x9E000000,
	opSubtractInt =			0x9F000000,
	opInitForEach =			0xA0000000,
	opIncForEach =			0xA1000000,
	opPushInitForEach =		0xA2000000,
	opMakeExpr =			0xA3000000,
	opMakeMapColExpr =		0xA4000000,
	opMakeExprIf =			0xA5000000,
	opIsIn =				0xA6000000,
	opSetTensorItemI =		0xA7000000,

	opHalt =				0xff000000,

	opCodeCount =			256,
	};

//	CHexeCodeIntermediate ------------------------------------------------------

class CHexeCodeIntermediate
	{
	public:
		int CreateCodeBlock (void);
		int CreateDatumBlock (CDatum dDatum);
		CDatum CreateOutput (int iBlock) const;
		int GetCodeBlockPos (int iBlock) const { return m_CodeBlocks[iBlock].GetPos(); }
		void RewriteShortOpCode (int iBlock, int iPos, OPCODE opCode, DWORD dwOperand = 0);
		void WriteShortOpCode (int iBlock, OPCODE opCode, DWORD dwOperand = 0);
		void WriteLongOpCode (int iBlock, OPCODE opCode, DWORD dwData);
		void WriteParam (int iBlock, DWORD dwData);

		const CBuffer &GetCodeBlock (int iIndex) const { return m_CodeBlocks[iIndex]; }
		int GetCodeBlockCount (void) const { return m_CodeBlocks.GetCount(); }
		CDatum GetDatumBlock (int iIndex) const { return m_DatumBlocks[iIndex]; }
		int GetDatumBlockCount (void) const { return m_DatumBlocks.GetCount(); }

	private:

		TArray<CBuffer> m_CodeBlocks;
		TArray<CDatum> m_DatumBlocks;

		TSortMap<CString, int> m_StringMap;
	};

//	CHexeExpressionEval ---------------------------------------------------------

class CHexeTableExpressionEval
	{
	public:

		static CDatum Sort (CDatum dTable, CDatum dSortColumns);

	private:

		static TArray<int> SortRows (CDatum dTable, CDatum dSortColumns, const TArray<int>* pRows = NULL);
	};

class CHexeExpressionEval
	{
	public:

		CHexeExpressionEval (const CAEONExpression& Expr, CDatum dTable) :
				m_Expr(Expr),
				m_dTable(dTable)
			{ }

		CDatum Average () const;
		CDatum Map () const;
		CDatum Max () const;
		CDatum MaxRow () const;
		CDatum Median () const;
		CDatum Min () const;
		CDatum MinRow () const;
		CDatum Sum () const;

	private:

		template<class FN> CDatum Compute () const
			{
			const IAEONTable* pTable = m_dTable.GetTableInterface();
			if (!pTable)
				throw CException(errFail);

			//	If the table has groups, then we need to process as groups, and we 
			//	return an array of values, one for each group.

			const CAEONTableGroupIndex& GroupIndex = pTable->GetGroupIndex();
			if (!GroupIndex.IsEmpty())
				{
				CDatum dResult(CDatum::typeArray);

				for (int i = 0; i < GroupIndex.GetCount(); i++)
					{
					CHexeColumnExpressionEval Eval(m_Expr, m_dTable, &GroupIndex.GetGroupIndex(i));
					CDatum dValue = FN::Compute(Eval);
					if (dValue.IsNil())
						continue;

					dResult.Append(dValue);
					}

				return dResult;
				}

			//	Otherwise, we return a single result.

			else
				{
				CHexeColumnExpressionEval Eval(m_Expr, m_dTable);
				return FN::Compute(Eval);
				}
			}

		static CDatum CreateGroupedTableFromRows (CDatum dTable, TArray<int>&& Rows);

		const CAEONExpression& m_Expr;
		CDatum m_dTable;
	};

class CHexeColumnExpressionEval
	{
	public:

		CHexeColumnExpressionEval (const CAEONExpression& Expr, CDatum dTable, const TArray<int>* pRows = NULL);

		static constexpr DWORD FLAG_ALLOW_NULL = 0x00000001;	//	If set, then we allow NULL values in the table.

		bool All () const { return All(m_Expr.GetRootNode()); }
		bool Any () const { return Any(m_Expr.GetRootNode()); }
		void AppendValues (CDatum dColumn) const { AppendValues(m_Expr.GetRootNode(), dColumn); }
		CDatum Average () const { return Average(m_Expr.GetRootNode()); }
		CDatum Column () const { return Column(m_Expr.GetRootNode()); }
		CDatum Eval () const;
		TArray<CDatum> EvalColumn (const CAEONExpression::SNode& Node) const;
		CDatum EvalRow (int iRow) const;
		TArray<int> FilterRows () const { return FilterRows(m_Expr.GetRootNode()); }
		CDatum First () const { return First(m_Expr.GetRootNode()); }
		bool IsError (CString* retsError = NULL) const;
		CAEONTableGroupIndex MakeGroupIndex () const;
		TSortMap<CDatum, TArray<int>, CKeyCompareEquivalent<CDatum>> GroupRows (DWORD dwFlags = 0) const { return GroupRows(m_Expr.GetRootNode(), dwFlags); }
		CDatum Map () const { return Map(m_Expr.GetRootNode());}
		void Mark ();
		CDatum Max () const { return Max(m_Expr.GetRootNode()); }
		int MaxRow () const { return MaxRow(m_Expr.GetRootNode()); }
		CDatum Median () const { return Median(m_Expr.GetRootNode()); }
		CDatum Min () const { return Min(m_Expr.GetRootNode()); }
		int MinRow () const { return MinRow(m_Expr.GetRootNode()); }
		CDatum Sum () const { return Sum(m_Expr.GetRootNode()); }

	private:

		struct SEvalCtx
			{
			const IAEONTable* pTable = NULL;
			int iRow = -1;
			};

		struct SColumnCtx
			{
			int iIndex = -1;		//	Index of the column in m_dTable
			CDatum dType;			//	Type of the column
			};

		bool All (const CAEONExpression::SNode& Node) const;
		bool Any (const CAEONExpression::SNode& Node) const;
		void AppendValues (const CAEONExpression::SNode& Node, CDatum dColumn) const;
		CDatum Array (const CAEONExpression::SNode& Node) const;

		CDatum Average (const CAEONExpression::SNode& Node) const;
		CDatum AverageOfColumn (SEvalCtx& Ctx, int iColIndex) const;
		CDatum AverageOfExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const;

		CDatum Column (const CAEONExpression::SNode& Node) const;
		int Count (const CAEONExpression::SNode& Node) const;

		TArray<int> FilterRows (const CAEONExpression::SNode& Node) const;
		TArray<int> FilterRowsColumnEqLit (SEvalCtx& Ctx, int iColIndex, CDatum dLiteral) const;
		TArray<int> FilterRowsColumnGreaterLit (SEvalCtx& Ctx, int iColIndex, CDatum dLiteral) const;
		TArray<int> FilterRowsColumnGreaterEqLit (SEvalCtx& Ctx, int iColIndex, CDatum dLiteral) const;
		TArray<int> FilterRowsColumnLesserLit (SEvalCtx& Ctx, int iColIndex, CDatum dLiteral) const;
		TArray<int> FilterRowsColumnLesserEqLit (SEvalCtx& Ctx, int iColIndex, CDatum dLiteral) const;
		TArray<int> FilterRowsColumnNeqLit (SEvalCtx& Ctx, int iColIndex, CDatum dLiteral) const;
		TArray<int> FilterRowsExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const;

		CDatum First (const CAEONExpression::SNode& Node) const;

		TSortMap<CDatum, TArray<int>, CKeyCompareEquivalent<CDatum>> GroupRows (const CAEONExpression::SNode& Node, DWORD dwFlags = 0) const;

		CAEONTableGroupIndex MakeGroupIndexByColumn (SEvalCtx& Ctx, int iColIndex) const;
		CAEONTableGroupIndex MakeGroupIndexExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const;

		CDatum Map (const CAEONExpression::SNode& Node) const;

		CDatum Max (const CAEONExpression::SNode& Node) const;
		CDatum MaxOfColumn (SEvalCtx& Ctx, int iColIndex) const;
		CDatum MaxOfExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const;

		int MaxRow (const CAEONExpression::SNode& Node) const;
		int MaxRowOfColumn (SEvalCtx& Ctx, int iColIndex) const;
		int MaxRowOfExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const;

		CDatum Median (const CAEONExpression::SNode& Node) const;
		CDatum MedianOfColumn (SEvalCtx& Ctx, int iColIndex) const;
		CDatum MedianOfExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const;

		CDatum Min (const CAEONExpression::SNode& Node) const;
		CDatum MinOfColumn (SEvalCtx& Ctx, int iColIndex) const;
		CDatum MinOfExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const;

		int MinRow (const CAEONExpression::SNode& Node) const;
		int MinRowOfColumn (SEvalCtx& Ctx, int iColIndex) const;
		int MinRowOfExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const;

		CDatum Sum (const CAEONExpression::SNode& Node) const;
		CDatum SumOfColumn (SEvalCtx& Ctx, int iColIndex) const;
		CDatum SumOfExpr (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const;

		CDatum UniqueArray (const CAEONExpression::SNode& Node) const;
		int UniqueCount (const CAEONExpression::SNode& Node) const;

		CDatum EvalNode (SEvalCtx& Ctx, const CAEONExpression::SNode& Node) const;
		bool IsColumnCompare (const CAEONExpression::SNode& Node) const;
		void MapColumns ();

		const CAEONExpression& m_Expr;
		CDatum m_dTable;
		const TArray<int>* m_pRows = NULL;

		TArray<SColumnCtx> m_Col;	//	Maps from an expression ColID to 
									//	the index of the column in m_dTable.
	};

class CHexeMapColumnExpressionEval
	{
	public:

		CHexeMapColumnExpressionEval (const CAEONMapColumnExpression& ColExpr, CDatum dTable);

		CDatum Summarize (CDatum dSchema = CDatum()) const;

	private:

		TArray<int> CreateColumnIndicesFromSchema (CDatum& dSchema) const;

		const CAEONMapColumnExpression& m_ColExpr;
		CDatum m_dTable;
	};

class CHexeTableGroupEval
	{
	public:

		CHexeTableGroupEval (const CAEONTableGroupDefinition& GroupDef, CDatum dTable) :
				m_GroupDef(GroupDef),
				m_dTable(dTable)
			{ }

		CAEONTableGroupIndex CreateIndex () const;
		TArray<CDatum> GetKeysFromIndex (const CAEONTableGroupIndex& Index) const;

	private:

		CString MakeGroupKey (const TArray<CHexeColumnExpressionEval>& Eval, int iRow) const;

		const CAEONTableGroupDefinition& m_GroupDef;
		CDatum m_dTable;
	};

