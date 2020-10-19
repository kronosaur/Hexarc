//	EnvHexeLisp.h
//
//	CHexeLispEnvironment Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CHexeLispEnvironment : public IProgramInstance
	{
	public:
		CHexeLispEnvironment (DWORD dwID) : IProgramInstance(dwID)
			{ }

	protected:

		virtual bool OnCreate (CDatum dCode, const SRunOptions &Options, CString *retsError = NULL) override;
		virtual void OnMark () override;
		virtual SRunResult OnRun () override;

	private:
		SRunResult HandleRunResult (CHexeProcess::ERunCodes iRunCode, CDatum dResult);

		CHexeProcess m_Process;
		
		static constexpr DWORD EXECUTION_QUANTUM = 30 * 1000;
	};
