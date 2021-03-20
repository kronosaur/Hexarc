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
		void EnterCodeBlock ();
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

		bool CompileDefinition (const IASTNode &AST, CString *retsError = NULL);
		bool CompileFunctionDef (IASTNode &AST, CString *retsError = NULL);
		bool CompileSequence (const IASTNode &AST, CString *retsError = NULL);


		bool CompileExpression (const IASTNode &AST, CString *retsError = NULL);
		bool CompileFunctionCall (const IASTNode &AST, CString *retsError = NULL);
		bool CompileStatement (const IASTNode &AST, CString *retsError = NULL);
		bool CompileVariableDefinition (const IASTNode &AST, CString *retsError = NULL);
		bool CompileVariableReference (const IASTNode &AST, CString *retsError = NULL);
		bool ComposeError (const CString &sError, CString *retsError) const;

		CGLTypeSystem *m_pTypes = NULL;

		CGLVMCodeGenerator m_Code;
	};
