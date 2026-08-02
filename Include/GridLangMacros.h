//	GridLangMacros.h
//
//	GridLang Compile-Time Macros
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "AEON.h"
#include "GridLangAST.h"

class IMacroCompilerImpl
	{
	public:
		virtual ~IMacroCompilerImpl () { }

		virtual bool MacroCompileCoercedExpression (void* pCtx, const IASTNode& Value, CDatum dRequiredType, bool bExplicit, CString* retsError = NULL) = 0;
		virtual bool MacroCompileDatatypeValue (void* pCtx, CDatum dType, CString* retsError = NULL) = 0;
		virtual bool MacroCompileDatatypeRef (void* pCtx, CDatum dType, CString* retsError = NULL) = 0;
		virtual bool MacroCompileExpression (void* pCtx, const IASTNode& Value, CString* retsError = NULL) = 0;
		virtual bool MacroComposeError (void* pCtx, const IASTNode& AST, const CString& sError, CString* retsError = NULL) = 0;
	};

class CGLMacroHandler
	{
	public:
		struct SDef
			{
			LPCSTR pMacro = NULL;
			LPCSTR pArgList = NULL;
			LPCSTR pHelp = NULL;

			std::function<bool(IMacroCompilerImpl& Compiler, void* pCtx, const IASTNode& MacroCall, CString* retsError)> fnInvoke;
			std::function<CDatum(const IASTNode& MacroCall)> fnGetStaticType;
			};

		struct SEntry
			{
			CString sName;
			CString sArgs;
			CString sHelp;
			CDatum dFuncType;

			std::function<bool(IMacroCompilerImpl& Compiler, void* pCtx, const IASTNode& MacroCall, CString* retsError)> fnInvoke;
			std::function<CDatum(const IASTNode& MacroCall)> fnGetStaticType;
			};

		CGLMacroHandler (const std::initializer_list<SDef>& Table)
			{
			m_Macros.GrowToFit(Table.size());
			for (auto& entry : Table)
				{
				int iIndex = m_Macros.GetCount();
				auto pEntry = m_Macros.Insert();

				pEntry->sName = CString(entry.pMacro);
				pEntry->sArgs = CString(entry.pArgList);
				pEntry->sHelp = CString(entry.pHelp);
				pEntry->dFuncType = CAEONTypes::CreateFunctionFromArgs(pEntry->sArgs);
				pEntry->fnInvoke = entry.fnInvoke;
				pEntry->fnGetStaticType = entry.fnGetStaticType;

				bool bNew;
				m_Table.SetAt(pEntry->sName, iIndex, &bNew);
				if (!bNew)
					throw CException(errFail);

				if (!pEntry->dFuncType.IsNil())
					{
					if (!m_bMarkProcRegistered)
						{
						CDatum::RegisterMarkProc(MarkProc);
						m_bMarkProcRegistered = true;
						}

					m_Mark.Insert(pEntry->dFuncType);
					}
				}
			}

		bool CanBeCalledWith (const IASTNode& MacroCall, CDatum* retdReturnType = NULL) const
			{
			const SEntry* pEntry = FindMacro(MacroCall.GetRoot().GetName());
			if (!pEntry || pEntry->dFuncType.IsNil())
				return false;

			TArray<CDatum> ArgTypes;
			TArray<CDatum> ArgLiteralTypes;
			for (int i = 0; i < MacroCall.GetChildCount(); i++)
				{
				const IASTNode& Arg = MacroCall.GetChild(i);

				ArgTypes.Insert(Arg.GetStaticType());

				CDatum dType;
				if (Arg.EvalConstExpression(dType) && dType.GetBasicType() == CDatum::typeDatatype)
					ArgLiteralTypes.Insert(dType);
				else
					ArgLiteralTypes.Insert(CDatum());
				}

			const IDatatype& FuncType = pEntry->dFuncType;
			return FuncType.CanBeCalledWith(CDatum(), ArgTypes, ArgLiteralTypes, retdReturnType);
			}

		bool CanBeCalledWithArgCount (const CString& sMacro, int iArgCount, CDatum* retdReturnType = NULL) const
			{
			const SEntry* pEntry = FindMacro(sMacro);
			if (!pEntry || pEntry->dFuncType.IsNil())
				return false;

			const IDatatype& FuncType = pEntry->dFuncType;
			return FuncType.CanBeCalledWithArgCount(CDatum(), iArgCount, retdReturnType);
			}

		const SEntry* FindMacro (const CString& sMacro) const
			{
			const int* pEntry = m_Table.GetAt(sMacro);
			if (!pEntry)
				return NULL;

			return &m_Macros[*pEntry];
			}

		CDatum GetReturnType (const IASTNode& MacroCall) const
			{
			CDatum dReturnType;
			if (!CanBeCalledWith(MacroCall, &dReturnType))
				return CAEONTypes::Get(IDatatype::ANY);

			return dReturnType;
			}

		bool GetStaticType (const IASTNode& MacroCall, CDatum* retdStaticType = NULL) const
			{
			const SEntry* pEntry = FindMacro(MacroCall.GetRoot().GetName());
			if (!pEntry || !pEntry->fnGetStaticType)
				return false;

			CDatum dStaticType = pEntry->fnGetStaticType(MacroCall);
			if (dStaticType.IsNil())
				dStaticType = CAEONTypes::Get(IDatatype::ANY);

			if (retdStaticType)
				*retdStaticType = dStaticType;

			return true;
			}

		bool Invoke (IMacroCompilerImpl& Compiler, void* pCtx, const IASTNode& MacroCall, CString* retsError = NULL) const
			{
			const SEntry* pEntry = FindMacro(MacroCall.GetRoot().GetName());
			if (!pEntry || !pEntry->fnInvoke)
				return false;

			return pEntry->fnInvoke(Compiler, pCtx, MacroCall, retsError);
			}

	private:
		static void MarkProc ()
			{
			for (int i = 0; i < m_Mark.GetCount(); i++)
				m_Mark[i].Mark();
			}

		TSortMap<CString, int> m_Table;
		TArray<SEntry> m_Macros;

		inline static bool m_bMarkProcRegistered = false;
		inline static TArray<CDatum> m_Mark;
	};
