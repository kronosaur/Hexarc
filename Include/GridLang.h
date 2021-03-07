//	GridLang.h
//
//	GridLang Classes
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "AEON.h"
#include "Hexe.h"
#include "GridLangAST.h"
#include "GridLangTypes.h"
#include "GridLangVMCompiler.h"

class CGridLangCoreLibrary
	{
	public:
		static void Define (const IGLType &IsA, CGLTypeNamespace &Namespace);
		static void Register ();

	private:
		static bool m_bRegistered;
	};

class CGridLangProgram
	{
	public:
		const CGridLangCodeBank &GetCode () const { return m_Code; }
		const CGLTypeNamespace &GetDefinitions () const { return m_Types.GetDefinitions(); }
		const CString &GetMainFunction () const;
		bool Load (IMemoryBlock &Stream, CString *retsError);
		void Mark () { m_Code.Mark(); }

	private:
		CGLTypeSystem m_Types;
		CGridLangCodeBank m_Code;
	};

class IGridLangEnvironment
	{
	public:
		virtual ~IGridLangEnvironment () { }
		virtual bool GetInput (const CString &sPort, const CString &sPrompt, CDatum *retdResult) = 0;
		virtual void Output (const CString &sPort, CDatum dValue) = 0;

		static IGridLangEnvironment *Get (IInvokeCtx &Ctx, CDatum *retdResult);
	};

class CGridLangProcess
	{
	public:
		CGridLangProcess () { }
		CGridLangProcess (const CGridLangProgram &Program)
			{ Init(Program); }

		void Init (const CGridLangProgram &Program);
		void Init (const CGridLangProgram &Program, IGridLangEnvironment &Environment);
		void Mark () { m_Hexe.Mark(); }
		CHexeProcess::ERun Run (CDatum &dResult);
		CHexeProcess::ERun RunContinues (CDatum dAsyncResult, CDatum &dResult);
		void SetEnvironment (IGridLangEnvironment &Environment) { m_pEnvironment = &Environment; m_pDefaultEnv.Delete(); }
		void SetExecutionRights (DWORD dwFlags);
		void SetMaxExecutionTime (DWORD dwMaxTime);

	private:
		enum class EState
			{
			None,						//	Need to load program
			Loaded,						//	Program loaded, not yet run
			WaitingForAsync,			//	Waiting for async result.
			Done,						//	Run complete
			};

		bool Load (CDatum &dResult);

		const CGridLangProgram *m_pProgram = NULL;
		IGridLangEnvironment *m_pEnvironment = NULL;
		CHexeProcess m_Hexe;

		EState m_iState = EState::None;

		TUniquePtr<IGridLangEnvironment> m_pDefaultEnv;
	};
