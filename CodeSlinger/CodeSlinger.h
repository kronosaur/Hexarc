//	CodeSlinger.h
//
//	CodeSlinger
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.
//
//	ARC.CODE
//
//	Arc.code has cached compiled code for every program.
//
//	ARC.INSTALL
//
//	Arc.install is a table storing the program installations for every user
//			We also use this to store any per-user, per-program config data,
//			such as permissions.
//
//	ARC.PROGRAMS
//
//	Arc.programs is the authoritative table of program descriptors. Each row is
//	a structure with the following fields:
//
//		id: The ID of the program. This is an 8-character unique ID generated at
//				create time.
//
//		name: The human-readable name of the program. This is case insensitive 
//				and can contain any characters (including spaces), except for
//				':', '/',  '\'. The name cannot have leading or trailing 
//				spaces, or any embedded non-printable characters except space.
//
//		createdBy: User who created the program. This cannot be changed.
//		createdOn: DateTime when the program was created.
//
//		canEdit: A list of users who can edit the program. In addition,
//				administrators can always edit the program. This default to the
//				creator, but can be edited.
//
//		canRun: If empty, then anyone can run the program. Otherwise, a list of
//				users who can run it. We default to the creator.
//
//		history: A reverse chronological list of changes to the descriptor. This
//				does not include changes to source code.
//
//		programType: The type of program. One of the following:
//
//				application: An interactive application.
//				console: A console program.
//
//		properties: A list of user-defined properties for the program. When 
//				running a program we can specify zero or more of these 
//				properties.
//
//		ports: The list of initial ports to create when running the program 
//				(more can be created at runtime). Each port is defined as a 
//				struct with the following fields:
//
//				id: A unique ID for the port. Code can use this ID to 
//						communicate via the port. IDs are case-sensitive and
//						are strictly alphanumeric (no symbols or whitespace).
//						We allow '_'.
//
//						We define the following IDs for all programs (i.e., they
//						do not have to be specified in this list):
//
//						CON: Always maps to a console port.
//						LOG: Always maps to a log port.
//						PROGRESS: Always maps to a progress port.
//						RESULT: Always maps to a return port.
//
//				protocol: One of:
//
//						console: A standard console interface.
//
//						log: A write-only port, generally used to report errors
//								and debug output.
//
//						progress: A port for reporting progress and current
//								running status.
//
//						return: A single datum result.
//
//	ARC.SOURCE
//
//	Arc.source stores the source code (including resources) for all programs.

#pragma once

class CCodeSlingerEngine : public TSimpleEngine<CCodeSlingerEngine>
	{
	public:
		CCodeSlingerEngine (void);
		CCodeSlingerEngine (const CCodeSlingerEngine &Src) = delete;
		CCodeSlingerEngine (CCodeSlingerEngine &&Src) = delete;

		virtual ~CCodeSlingerEngine (void);

		CCodeSlingerEngine &operator= (const CCodeSlingerEngine &Src) = delete;
		CCodeSlingerEngine &operator= (CCodeSlingerEngine &&Src) = delete;

		bool IsVerbose (void) const { return m_bVerbose; }
		void SetAeonInitialized (bool bInitialized = true) { m_bAeonInitialized = bInitialized; }

		static SMessageHandler m_MsgHandlerList[];
		static int m_iMsgHandlerListCount;

	protected:

		//	TSimpleEngine override

		virtual void OnBoot (void) override;
		virtual void OnMarkEx (void) override;
		virtual void OnStartRunning (void) override;
		virtual void OnStopRunning (void) override;

	private:
		static constexpr int DEFAULT_THREADS_PER_MACHINE = 4;
		static constexpr int INITIAL_THREADS = DEFAULT_THREADS_PER_MACHINE + 2;

		//	Message handlers
		void MsgAeonOnStart (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
//		void MsgHousekeeping (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgMandelbrot (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgMandelbrotTask (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgStatus (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);

		CCriticalSection m_cs;
		bool m_bMachineStarted = false;				//	TRUE if we've received Exarch.onMachineStart message
		bool m_bAeonInitialized = false;			//	TRUE if AeonDB has initialized
		bool m_bReady = false;						//	TRUE if we are serving requests
		bool m_bVerbose = true;						//	TRUE if we're in verbose mode (log extra information)
	};
