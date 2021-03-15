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

class CGridLangVMCompiler
	{
	public:
		CGridLangVMCompiler (const CGLTypeSystem &Types, CGridLangCodeBank &Output) :
				m_Types(Types),
				m_Output(Output)
			{ }

		bool CompileDefinition (const IASTNode &AST, CString *retsError = NULL);
		bool CompileSequence (const IASTNode &AST, const CString &sSymbol, CString *retsError = NULL);

	private:
		struct SCompilerCtx
			{
			SCompilerCtx (const CGLTypeSystem &Types) :
					Scope(Types)
				{
				iBlock = Code.CreateCodeBlock();
				}

			int CreateDataBlock (CDatum dValue)
				{
				return Code.CreateDatumBlock(dValue);
				}

			void WriteLongOpCode (OPCODE opCode, DWORD dwData)
				{
				Code.WriteLongOpCode(iBlock, opCode, dwData);
				}

			void WriteLongOpCode (OPCODE opCode, CDatum dData)
				{
				Code.WriteLongOpCode(iBlock, opCode, dData);
				}

			void WriteShortOpCode (OPCODE opCode, DWORD dwOperand = 0)
				{
				Code.WriteShortOpCode(iBlock, opCode, dwOperand);
				}

			CHexeCodeIntermediate Code;
			int iBlock = -1;

			CDatum m_dCurrentEnv;
			CHexeLocalEnvironment *m_pCurrentEnv = NULL;

			CGLNamespaceCtx Scope;
			};

		bool CompileExpression (SCompilerCtx &Ctx, const IASTNode &AST, CString *retsError = NULL);
		bool CompileFunctionCall (SCompilerCtx &Ctx, const IASTNode &AST, CString *retsError = NULL);
		bool CompileStatement (SCompilerCtx &Ctx, const IASTNode &AST, CString *retsError = NULL);
		bool CompileVariableDefinition (SCompilerCtx &Ctx, const IASTNode &AST, CString *retsError = NULL);
		bool CompileVariableReference (SCompilerCtx &Ctx, const IASTNode &AST, CString *retsError = NULL);
		bool ComposeError (SCompilerCtx &Ctx, const CString &sError, CString *retsError) const;

		const CGLTypeSystem &m_Types;
		CGridLangCodeBank &m_Output;
	};
