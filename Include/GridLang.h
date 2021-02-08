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

class CGridLangProcess
	{
	public:
		CGridLangProcess (const CGridLangProgram &Program) :
				m_Program(Program)
			{ }

		void Mark () { m_Hexe.Mark(); }
		CHexeProcess::ERunCodes Run (CDatum &dResult);
		CHexeProcess::ERunCodes RunContinues (CDatum dAsyncResult, CDatum &dResult);

	private:
		enum class EState
			{
			None,						//	Need to load program
			Loaded,						//	Program loaded, not yet run
			WaitingForAsync,			//	Waiting for async result.
			Done,						//	Run complete
			};

		bool Load (CDatum &dResult);

		const CGridLangProgram &m_Program;
		CHexeProcess m_Hexe;

		EState m_iState = EState::None;
	};
