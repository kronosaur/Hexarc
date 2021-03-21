//	GridLangVMCompiler.h
//
//	GridLang VM Compiler
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CGridLangCodeBank
	{
	public:
		void AddSymbol (const CString &sSymbol, const IGLType &Type, CDatum dCode) { m_Map.SetAt(sSymbol, SEntry(Type, dCode)); }
		CDatum GetCode (int iIndex) const { return m_Map[iIndex].dCode; }
		TArray<CHexeDocument::SEntryPoint> GetCodeAsEntryPointTable () const;
		int GetCount () const { return m_Map.GetCount(); }
		const CString &GetSymbol (int iIndex) const { return m_Map.GetKey(iIndex); }
		void Mark ();

	private:
		struct SEntry
			{
			SEntry ()
				{ }

			SEntry (const IGLType &TypeArg, CDatum dCodeArg) :
					pType(&TypeArg),
					dCode(dCodeArg)
				{ }

			const IGLType *pType = NULL;
			CDatum dCode;
			};

		TSortMap<CString, SEntry> m_Map;
	};

class CGLVMCodeGenerator
	{
	public:
		CGLVMCodeGenerator () { }

		int CreateDataBlock (CDatum dValue);
		CDatum CreateOutput ();
		int EnterCodeBlock ();
		void ExitCodeBlock ();
		int GetCodeBlock () const { return m_iBlock; }
		void Init ();
		void WriteLongOpCode (OPCODE opCode, DWORD dwData);
		void WriteShortOpCode (OPCODE opCode, DWORD dwOperand = 0);

	private:
		CHexeCodeIntermediate m_Code;
		int m_iBlock = -1;

		TArray<int> m_SavedBlocks;
	};

class CGridLangVMCompiler
	{
	public:
		CGridLangVMCompiler () { }

		bool CompileProgram (IASTNode &Root, CGLTypeSystem &retTypes, CGridLangCodeBank &retOutput, CString *retsError = NULL);

	private:

		bool CompileBinaryOp (IASTNode &AST, EOpCodes iOpCode, CString *retsError = NULL);
		bool CompileDefinition (IASTNode &AST, CString *retsError = NULL);
		bool CompileExitFunction (IASTNode &AST, CString *retsError = NULL);
		bool CompileExpression (IASTNode &AST, CString *retsError = NULL);
		bool CompileFunctionCall (IASTNode &AST, CString *retsError = NULL);
		bool CompileFunctionBlock (IASTNode &AST, CString *retsError = NULL);
		bool CompileFunctionDefinition (IASTNode &AST, CString *retsError = NULL);
		bool CompileGlobalReference (IASTNode &VarRef, CString *retsError = NULL);
		bool CompileReturn (IASTNode &AST, CString *retsError = NULL);
		bool CompileSequence (IASTNode &AST, CString *retsError = NULL);
		bool CompileStatement (IASTNode &AST, CString *retsError = NULL);
		bool CompileVariableDefinition (IASTNode &AST, CString *retsError = NULL);
		bool CompileVariableReference (IASTNode &VarRef, IASTNode &Scope, IASTNode &Pos, int iLevel, CString *retsError = NULL);

		bool ComposeError (const CString &sError, CString *retsError) const;

		CGLTypeSystem *m_pTypes = NULL;

		CGLVMCodeGenerator m_Code;
	};
