//	GridLangVMCompiler.h
//
//	GridLang VM Compiler
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#pragma once

class IASTNode;

class CGLVMCodeGenerator
	{
	public:
		CGLVMCodeGenerator () { }

		int CreateDataBlock (CDatum dValue);
		CDatum CreateOutput ();
		int EnterCodeBlock ();
		void ExitCodeBlock ();
		int GetCodeBlock () const { return m_iBlock; }
		int GetCodePos () const { return m_Code.GetCodeBlockPos(m_iBlock); }
		void Init ();
		void RewriteShortOpCode (int iPos, OPCODE opCode, DWORD dwOperand = 0) { m_Code.RewriteShortOpCode(m_iBlock, iPos, opCode, dwOperand); }
		void WriteFrameCall (OPCODE opCode, int iArgs, DWORD dwCodeBlock);
		void WriteLibCall (int iArgs, DWORD dwID);
		void WriteLongOpCode (OPCODE opCode, DWORD dwData);
		void WriteParam (DWORD dwData);
		void WriteShortOpCode (OPCODE opCode, DWORD dwOperand = 0);
		void WriteShortOpCode2 (OPCODE opCode, DWORD dwArg, DWORD dwValue);

	private:
		CHexeCodeIntermediate m_Code;
		int m_iBlock = -1;

		TArray<int> m_SavedBlocks;
	};

class CGridLangVMCompiler : public IMacroCompilerImpl
	{
	public:

		CGridLangVMCompiler () { }

		static bool Compile (CGridLangAST& AST, CHexeProgram& retOutput, CGridLangResult& Result);
		static bool Compile (const CString& sFilename, IMemoryBlock &Stream, const CGridLangLibraries &Libraries, CHexeProgram& retOutput, CGridLangResult& Result);

	private:

		struct SCtx
			{
			CString sScope;
			CDatum dReturnType;
			IASTNode* pFunction = NULL;
			IASTNode* pStackFrameFunction = NULL;
			};

		enum class ECoercion
			{
			Implicit,
			ConstructIfNeeded,
			Construct,
			};

		static constexpr DWORD INVALID_VAR_ID = 0xffffffff;

		bool CompileProgram (CGridLangAST& AST, CHexeProgram& retOutput, CGridLangResult& Result);

		bool CompileArrayConstructor (SCtx& Ctx, IASTNode& ArrayDef, IASTNode& Call, CString* retsError = NULL);
		bool CompileArrayDynamicConstructor (SCtx &Ctx, IASTNode& ArrayDef, IASTNode &AST, CString *retsError = NULL);
		bool CompileArrayDynamicDef (SCtx &Ctx, IASTNode &AST, EOpCodes iOpCode, bool bConstruct, CString *retsError = NULL);
		bool CompileAssignment (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileAssignmentArrayLValue (SCtx &Ctx, IASTNode &ArrayOp, IASTNode &Scope, IASTNode &Pos, int iLevel, CString *retsError = NULL);
		bool CompileAssignmentStructLValue (SCtx &Ctx, IASTNode &StructOp, IASTNode &Scope, IASTNode &Pos, int iLevel, CString *retsError = NULL);
		bool CompileAssignmentTensorLValue (SCtx &Ctx, IASTNode &Tensor, IASTNode &Scope, IASTNode &Pos, int iLevel, CString *retsError = NULL);
		bool CompileAssignmentVariableLValue (SCtx &Ctx, IASTNode &VarRef, IASTNode &Scope, IASTNode &Pos, int iLevel, CString *retsError = NULL);
		bool CompileBinaryOp (SCtx &Ctx, IASTNode &AST, EOpCodes iOpCode, CString *retsError = NULL);
		bool CompileClassDefinition (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileConstructor (SCtx &Ctx, IASTNode &ClassDef, IASTNode &Call, CString *retsError = NULL);
		bool CompileCoercedExpression (SCtx &Ctx, IASTNode &Value, CDatum dRequiredType, ECoercion iCoercion, CString *retsError = NULL);
		bool CompileMemberInit (SCtx &Ctx, const IASTNode &ClassDef, CString *retsError = NULL);
		bool CompileDatatypeRef (SCtx &Ctx, const IDatatype &Type, CString *retsError = NULL);
		bool CompileDictionaryConstructor (SCtx& Ctx, IASTNode& DictionaryDef, IASTNode& Call, CString* retsError = NULL);
		bool CompileDo (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileDoTerm (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL, bool bDiscardResult = false);
		bool CompileEnumValue (SCtx& Ctx, IASTNode& EnumDef, IASTNode& Value, CString* retsError = NULL);
		bool CompileExitFunction (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileExpression (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL, bool bDiscardResult = false);
		bool CompileForEachLoop (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileForLoop (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileFunctionCall (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileFunctionCallColExpression (SCtx &Ctx, IASTNode &AST, CAEONExpression::EOp iOp, CString *retsError = NULL);
		bool CompileFunctionCallExpression (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileFunctionCallObjMethod (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileFunctionCallString (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileFunctionCallVarRef (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileFunctionBlock (SCtx &Ctx, IASTNode &AST, IASTNode* pClassAST = NULL, CString *retsError = NULL);
		bool CompileFunctionDefinition (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileFunctionObj (SCtx &Ctx, IASTNode &AST, IASTNode* pClassAST, CString *retsError = NULL);
		bool CompileGlobalReference (SCtx &Ctx, IASTNode &VarRef, CString *retsError = NULL);
		bool CompileIf (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileIfStatement (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileIn (SCtx &Ctx, IASTNode &AST, EOpCodes iOpCode, CString *retsError = NULL);
		bool CompileLiteralArray (SCtx &Ctx, IASTNode& Array, CString *retsError = NULL);
		bool CompileLiteralInt (SCtx &Ctx, DWORD dwValue, CString *retsError = NULL);
		bool CompileLiteralStruct (SCtx &Ctx, IASTNode& Struct, CString *retsError = NULL);
		bool CompileLiteralValue (SCtx &Ctx, CDatum dValue, CString *retsError = NULL);
		bool CompileLogicalAnd (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileLogicalOr (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileMacroCall (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileMacroNew (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileMapColExpression (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileMemberAccessOp (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileMethodAccessOp (SCtx &Ctx, IASTNode &AST, IASTNode& FuncCallAST, int &retiExtraArgs, CString *retsError = NULL);
		bool CompileMemberFunctionDefinition (SCtx &Ctx, IASTNode &ClassAST, IASTNode &FunctionAST, CString *retsError = NULL);
		bool CompileMutate (SCtx &Ctx, IASTNode &AST, EASTType iOp, CString* retsError = NULL);
		bool CompileMutateArrayLValue (SCtx &Ctx, IASTNode &ArrayOp, IASTNode &Scope, IASTNode &Pos, int iLevel, EASTType iOp, CString *retsError = NULL);
		bool CompileMutateStructLValue (SCtx &Ctx, IASTNode &StructOp, IASTNode &Scope, IASTNode &Pos, int iLevel, EASTType iOp, CString *retsError = NULL);
		bool CompileMutateTensorLValue (SCtx &Ctx, IASTNode &Tensor, IASTNode &Scope, IASTNode &Pos, int iLevel, EASTType iOp, CString *retsError = NULL);
		bool CompileMutateVariableByConstInt (SCtx& Ctx, IASTNode& AST, EASTType iOp, bool bNeedResult, bool* retbCompiled, CString* retsError = NULL);
		bool CompileMutateVariableLValue (SCtx &Ctx, IASTNode &VarRef, IASTNode &Scope, IASTNode &Pos, int iLevel, EASTType iOp, CString *retsError = NULL);
		bool CompileReturn (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileSchemaLiteralConstructor (SCtx &Ctx, const IDatatype &SchemaType, IASTNode &Struct, bool *retbCompiled, CString *retsError = NULL);
		bool CompileSequence (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileStatement (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileTableConstructor (SCtx& Ctx, IASTNode& TableDef, IASTNode& Call, CString* retsError = NULL);
		bool CompileUnaryOp (SCtx &Ctx, IASTNode &AST, EOpCodes iOpCode, CString *retsError = NULL);
		bool CompileVariableDefinition (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);
		bool CompileVariableReference (SCtx &Ctx, IASTNode &VarRef, IASTNode &Scope, IASTNode &Pos, int iLevel, CString *retsError = NULL);
		bool CompileWhileLoop (SCtx &Ctx, IASTNode &AST, CString *retsError = NULL);

		bool ComposeError (const IASTNode& AST, const CString &sError, CString *retsError) const;
		DWORD FindLocalVariableID (IASTNode &VarRef, IASTNode &Scope, IASTNode &Pos, int iLevel) const;
		virtual bool MacroCompileCoercedExpression (void* pCtx, const IASTNode& Value, CDatum dRequiredType, bool bExplicit, CString* retsError = NULL) override;
		virtual bool MacroCompileDatatypeValue (void* pCtx, CDatum dType, CString* retsError = NULL) override;
		virtual bool MacroCompileDatatypeRef (void* pCtx, CDatum dType, CString* retsError = NULL) override;
		virtual bool MacroCompileExpression (void* pCtx, const IASTNode& Value, CString* retsError = NULL) override;
		virtual bool MacroComposeError (void* pCtx, const IASTNode& AST, const CString& sError, CString* retsError = NULL) override;
		static bool IsFrameArgRef (const SCtx& Ctx, const IASTNode& Def);
		static bool IsArgL0 (DWORD dwArgPos) { return ((dwArgPos >> 8) == 0); }
		static bool LoadEntryPoints (const IASTNode& AST, CHexeProgram& Output, CGridLangResult& Result);

		CAEONTypeSystem *m_pTypes = NULL;

		CGLVMCodeGenerator m_Code;
	};
