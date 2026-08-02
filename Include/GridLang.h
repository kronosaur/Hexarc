//	GridLang.h
//
//	GridLang Classes
//	Copyright (c) 2020 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "AEON.h"
#include "Hexe.h"
#include "GridLangAST.h"
#include "GridLangCFG.h"
#include "GridLangMacros.h"
#include "GridLangVMCompiler.h"
#include "GridLangLibraries.h"
#include "GridLangProcessors.h"

class IGridLangEnvironment
	{
	public:
		virtual ~IGridLangEnvironment () { }
		virtual const CGridLangLibraries &GetLibraries () const = 0;

		static IGridLangEnvironment *Get (IInvokeCtx &Ctx, CDatum *retdResult);

	private:
		bool Lib_UnsuportedFeature (CDatum &retdResult);
	};

class CGridLangProcess
	{
	public:
		CGridLangProcess (IHexeVMHost& VMHost) :
				m_Hexe(VMHost)
			{ }

		CGridLangProcess (IHexeVMHost& VMHost, const CHexeProgram &Program, const CGridLangLibraries &Libraries) :
				m_Hexe(VMHost)
			{ Init(Program, Libraries); }

		CGridLangProcess (IHexeVMHost& VMHost, const CHexeProgram &Program, IGridLangEnvironment &Environment) :
				m_Hexe(VMHost)
			{ Init(Program, Environment); }

		CHexeProcess &GetHexeProcess () { return m_Hexe; }
		const CHexeProcess &GetHexeProcess () const { return m_Hexe; }
		const IInvokeCtx::SLimits& GetLimits () const { return m_Hexe.GetLimits(); }
		CDatum GetTypeList () const { return m_Hexe.GetTypeList(); }
		void Init (const CHexeProgram &Program, const CGridLangLibraries &Libraries);
		void Init (const CHexeProgram &Program, IGridLangEnvironment &Environment);
		bool InitFromSerialized (CDatum dSerialized, IGridLangEnvironment &Environment, CString *retsError = NULL);
		void Mark () { m_Hexe.Mark(); }
		CHexeProcess::ERun Run (CDatum &dResult);
		CHexeProcess::ERun RunContinues (CDatum dAsyncResult, CDatum &dResult);
		CHexeProcess::ERun RunContinuesFromStopCheck (CDatum &dResult);
		CHexeProcess::ERun RunEntryPoint (const CString &sEntryPoint, CDatum dArgs, CDatum &dResult);
		CHexeProcess::ERun RunEventHandler (CDatum dFunc, const TArray<CDatum> &Args, CDatum &dResult);
		CHexeProcess::ERun RunFunc (CDatum dFunc, const TArray<CDatum> &Args, CDatum &dResult);
		CHexeProcess::ERun RunFuncContinues (CDatum dAsyncResult, CDatum &dResult) { return RunContinues(dAsyncResult, dResult); }
		CDatum Serialize (CDatum dFunc) const;
		void SetEnvironment (IGridLangEnvironment &Environment) { m_pEnvironment = &Environment; m_pDefaultEnv.Delete(); }
		void SetExecutionRights (DWORD dwFlags);
		void SetExecutionLimits (const IInvokeCtx::SLimits& Limits) { m_Hexe.SetLimits(Limits); }
		void SignalPause (bool bPause) { m_Hexe.SignalPause(bPause); }

		bool CheckArrayLimit (DWORDLONG dwSize, CDatum& retdResult) const;
		CDatum GetArrayLimitError () const;

	private:
		enum class EState
			{
			None,						//	Need to load program
			Loaded,						//	Program loaded, not yet run
			WaitingToContinue,			//	Waiting for async result.
			Done,						//	Run complete
			};

		bool Load (CDatum &dResult);

		const CHexeProgram *m_pProgram = NULL;
		IGridLangEnvironment *m_pEnvironment = NULL;
		CHexeProcess m_Hexe;

		EState m_iState = EState::None;

		TUniquePtr<IGridLangEnvironment> m_pDefaultEnv;
	};

class CGridLang
	{
	public:

		static bool CompileExpressionType (const IMemoryBlock& Stream, CDatum& retdExpression, CGridLangResult& retResult);
	};

#include "GridLangObjImpl.h"
