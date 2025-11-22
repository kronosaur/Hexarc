//	HyperionServiceImpl.h
//
//	Hyperion Service Implementation
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#pragma once

//	AI1 ------------------------------------------------------------------------

class CAI1Service;

class CAI1Session : public CHyperionSession
	{
	public:
		CAI1Session (CHyperionEngine *pEngine, const CString &sListener, CDatum dSocket, const CString &sNetAddress);

		virtual CString GetDebugInfo (void) const;

	protected:
		//	ISessionHandler virtuals

		virtual void OnEndSession (DWORD dwTicket) override;
		virtual void OnMark (void) override;
		virtual bool OnProcessMessage (const SArchonMessage &Msg) override;
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override;

		//	CHyperionSession virtuals

		virtual void OnGetHyperionStatusReport (CComplexStruct *pStatus) const override;

	private:
		enum EStates
			{
			stateUnknown,
			stateWaitingForData,
			stateWaitingForRPC,
			stateWaitingForCreateAdminRPC,
			stateWaitingForAuthRPC,
			stateWaitingForWriteConfirm,
			stateWaitingForFinalWrite,
			};

		bool ExecuteHexeCommand (const CString &sCommand, CDatum dPayload);
		void FillStream (void);
		bool HandleOnWrite (const SArchonMessage &Msg);
		bool ProcessCommand (const CString &sCommand, CDatum dPayload);
		bool ProcessRPC (const SArchonMessage &RPCMsg);
		bool ProcessRunResult (CHexeProcess::ERun iRun, CDatum dRunResult);
		void SendDisconnect (void);
		bool SendProgress (CDatum dPayload);
		bool SendReply (const CString &sCommand, CDatum dPayload, EStates iNewState = stateWaitingForWriteConfirm);
		bool SendReplyError (const CString &sError);
		bool SendReplyErrorAndDisconnect (const CString &sError);
		bool SendReplyWelcome (void);
		bool SendRPC (const CString &sAddr, const SArchonMessage &Msg, EStates iNewState = stateWaitingForRPC);

		CAI1Service *m_pService;			//	Service to handle the session

		EStates m_iState;					//	State of session
		CAI1Stream m_Stream;				//	Stream building up messages from client
		CHexeProcess m_Process;				//	Hexe process handling this request.
		int m_iWritesOutstanding;			//	Number of writes that we're waiting on acks for.

		CDatum m_dChallenge;				//	Challenge sent to user for auth
	};

class CAI1Service : public IHyperionService
	{
	public:
		static CAI1Service *AsAI1Service (IHyperionService *pService);
		static bool CreateServiceClass (CDatum dServiceDef, const CHexeDocument &Package, IHyperionService **retpService, CString *retsError);

		inline bool AllowsAnonymousAccess (void) { return m_bAllowAnonymousAccess; }
		inline const CString &GetInterface (void) { return m_sInterface; }
		inline const CAttributeList &GetRightsRequired (void) { return m_MinUserRights; }

	protected:
		//	IHyperionService
		virtual CHyperionSession *OnCreateSessionObject (CHyperionEngine *pEngine, const CString &sListener, const CString &sProtocol, CDatum dSocket, const CString &sNetAddress) override { return new CAI1Session(pEngine, sListener, dSocket, sNetAddress); }
		virtual bool OnInit (CDatum dServiceDef, const CHexeDocument &Package, CString *retsError) override;
		virtual void OnInitProcess (CHexeProcess &Process) override { Process.InitFrom(m_ProcessTemplate); }
		virtual bool OnIsListener (void) const override { return true; }
		virtual void OnMark (void) override;

	private:
		CString m_sInterface;				//	Name of the interface we implement
		CAttributeList m_MinUserRights;		//	Minimum rights required to connect
		bool m_bAllowAnonymousAccess;		//	If TRUE login is not required

		CHexeProcess m_ProcessTemplate;
	};

//	HexarcMsg ------------------------------------------------------------------

class CHexarcMsgService : public IHyperionService
	{
	public:
		static bool CreateServiceClass (CDatum dServiceDef, const CHexeDocument &Package, IHyperionService **retpService, CString *retsError);

	protected:
		//	IHyperionService
		virtual bool OnInit (CDatum dServiceDef, const CHexeDocument &Package, CString *retsError) override;
		virtual void OnInitProcess (CHexeProcess &Process) override { Process.InitFrom(m_ProcessTemplate); }
		virtual void OnMark (void) override;

	private:
		CHexeProcess m_ProcessTemplate;
	};

//	HTTP -----------------------------------------------------------------------

class CHTTPService;

enum EHTTPProcessingStatus
	{
	pstatNone,

	pstatResponseReady,						//	Ctx.Response is valid and initialized
	pstatResponseReadyProxy,				//	Ctx.Response is ready and should be returned without modification
	pstatRPCReady,							//	Ctx.sRPCAddr and Ctx.RPCMsg initialized
	pstatFilePathReady,						//	Ctx.sFilePath is a filepath to response
	pstatFileDataReady,						//	Ctx.dFileData and Ctx.dFileDesc are valid
	pstatFileError,							//	Error getting file
	pstatUpgradeToWebSocket,				//	Upgrade to web socket
	pstatUpgradeToWebSocketNoOp,			//	Service already upgrades; done with session
	};

struct SHTTPRequestCtx
	{
	~SHTTPRequestCtx (void) { if (pProcess) delete pProcess; if (pHexeEval) delete pHexeEval; Request.DeleteBodyBuilder(); }

	inline void Mark (void)
		{
		pBodyBuilder->Mark();
		dFileData.Mark();

		if (pProcess)
			pProcess->Mark();

		if (pHexeEval)
			pHexeEval->Mark();

		RPCMsg.dPayload.Mark();
		}

	//	Current request
	
	CHTTPMessage Request;					//	Initialized by CHTTPSession when it receives the request
	CEsperBodyBuilderPtr pBodyBuilder = CEsperBodyBuilderPtr(new CEsperBodyBuilder);	//	Used to parse the body

	//	Processing state

	CHyperionSession *pSession = NULL;		//	Session object
	CHTTPService *pService = NULL;			//	Service handling request.

	int iFileRecursion = 0;					//	Number of times we've requested a file this session
	CDatum dFileData;						//	If file has been split, this contains data that we have
											//		downloaded so far.

	CHTTPService *pProcessService = NULL;	//	Service that initialized the process.
	CHexeProcess *pProcess = NULL;			//	Hexe process handling this request (may be NULL).
											//		This object is initialized by HandleRequest (but freed by us)

	CHexeMarkupEvaluator *pHexeEval = NULL;	//	Markup evaluator handling request (may be NULL).
											//		This object is initialized by HandleRequest (but freed by us)

	//	Current response

	EHTTPProcessingStatus iStatus = pstatNone;	//	Processing status
	CHTTPMessage Response;					//	Initialized by CHTTPService (or derived classes)

	CString sRPCAddr;						//	RPC address
	SArchonMessage RPCMsg;					//	RPC message

	CString sFilePath;						//	filePath
	CDatum dFileDesc;						//	fileDesc (no need to mark because it does not persist)
	TArray<CHTTPMessage::SHeader> AdditionalHeaders;
	};

class CHTTPSession : public CHyperionSession
	{
	public:
		CHTTPSession (CHyperionEngine *pEngine, const CString &sListener, const CString &sProtocol, CDatum dSocket, const CString &sNetAddress);

		virtual CString GetDebugInfo (void) const;

	protected:
		//	ISessionHandler virtuals

		virtual void OnEndSession (DWORD dwTicket) override;
		virtual void OnMark (void) override;
		virtual bool OnProcessMessage (const SArchonMessage &Msg) override;
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override;

		//	CHyperionSession virtuals

		virtual void OnGetHyperionStatusReport (CComplexStruct *pStatus) const override;

	private:
		enum EFlags
			{
			//	SendResponse
			FLAG_NO_ERROR_LOG =				0x00000001,
			FLAG_PROXY =					0x00000002,
			};

		enum class State
			{
			unknown,
			waitingForRequest,
			responseSent,
			responseSentPartial,
			waitingForRPCResult,
			waitingForRPCLongPoll,
			waitingForFileData,
			disconnected,
			};

		bool CalcRequestIP (const CHTTPMessage &Msg, CString *retsAddress) const;
		bool Disconnect (const SArchonMessage &Msg);
		bool GetRequest (const SArchonMessage &Msg, bool bContinued = false);
		CString GetRequestDescription (void) const;
		bool ProcessFileResult (SHTTPRequestCtx &Ctx, CDatum dFileDesc, CDatum dFileData, const SArchonMessage &Msg);
		bool ProcessServiceResult (SHTTPRequestCtx &Ctx, const SArchonMessage &Msg);
		bool ProcessStateDisconnected (const SArchonMessage &Msg);
		bool ProcessStateResponseSent (const SArchonMessage &Msg);
		bool ProcessStateResponseSentPartial (const SArchonMessage &Msg);
		bool ProcessStateWaitingForFileData (const SArchonMessage &Msg);
		bool ProcessStateWaitingForRequest (const SArchonMessage &Msg);
		bool ProcessStateWaitingForRPCResult (const SArchonMessage &Msg);
		bool ProcessStateWaitingForRPCLongPollResult (const SArchonMessage &Msg);
		bool SendFile (SHTTPRequestCtx &Ctx, const SArchonMessage &Msg);
		bool SendReadFileRequest (SHTTPRequestCtx &Ctx, const SArchonMessage &Msg, int iOffset);
		bool SendResponse (SHTTPRequestCtx &Ctx, const SArchonMessage &Msg, DWORD dwFlags = 0);
		bool SendResponseChunk (const SArchonMessage &Msg, CHTTPMessage &Response);
		bool SendRPC (SHTTPRequestCtx &Ctx);
		bool SendRPCUpgradeWebSocket (SHTTPRequestCtx &Ctx);

		State m_iState = State::unknown;		//	State of session
		SHTTPRequestCtx m_Ctx;					//	Processing context

		DWORDLONG m_dwStartRequest = 0;			//	Tick when we started a request
		DWORD m_dwPartialSend = 0;				//	Total bytes already sent on a partial response

		//	We store some status information here. These variables are accessed
		//	by OnGetHyperionStatusReport and should be protected by the main
		//	Hyperion critical section.

		CString m_sLastRequestMethod;
		CString m_sLastRequestURL;
		CString m_sLastRequestIP;
		DWORDLONG m_dwLastRequestTime = 0;		//	Tick count of last request
	};

class CHTTPService : public IHyperionService
	{
	public:
		static CHTTPService *AsHTTPService (IHyperionService *pService);
		static bool CreateServiceClass (CDatum dServiceDef, const CHexeDocument &Package, IHyperionService **retpService, CString *retsError);

		bool HandleHexmFile (SHTTPRequestCtx &Ctx, CDatum dFileDesc, CDatum dData) { return OnHandleHexmFile(Ctx, dFileDesc, dData); }
		bool HandleRequest (SHTTPRequestCtx &Ctx);
		bool HandleRPCResult (SHTTPRequestCtx &Ctx, const SArchonMessage &RPCResult) { return OnHandleRPCResult(Ctx, RPCResult); }
		int MatchHostAndURL (const CString &sHost, const CString &sURL);

	protected:
		enum ETLSTypes
			{
			tlsNone,
			tlsOptional,
			tlsRequired,
			};

		//	IHyperionService
		virtual CHyperionSession *OnCreateSessionObject (CHyperionEngine *pEngine, const CString &sListener, const CString &sProtocol, CDatum dSocket, const CString &sNetAddress) override { return new CHTTPSession(pEngine, sListener, sProtocol, dSocket, sNetAddress); }
		virtual void OnGetListeners (TArray<SListenerDesc> &Listeners) const override;
		virtual bool OnInit (CDatum dServiceDef, const CHexeDocument &Package, CString *retsError) override;
		virtual bool OnIsListener (void) const override { return true; }
		virtual void OnMark (void) override { OnHTTPMark(); }

		//	CHTTPService
		virtual bool OnHandleHexmFile (SHTTPRequestCtx &Ctx, CDatum dFileDesc, CDatum dData) { return false; }
		virtual bool OnHandleRequest (SHTTPRequestCtx &Ctx) = 0;
		virtual bool OnHandleRPCResult (SHTTPRequestCtx &Ctx, const SArchonMessage &RPCResult) = 0;
		virtual bool OnHTTPInit (CDatum dServiceDef, const CHexeDocument &Package, CString *retsError) { return true; }
		virtual void OnHTTPMark (void) { }

		//	Helpers
		CString MakePathCanonical (const CString &sPath);
		CString MakeRelativePath (const CString &sPath);

	private:
		struct SRedirectDesc
			{
			SRedirectDesc (void) :
					fMatchAllPaths(false),
					fMatchExactPath(true),
					fMatchHost(false),
					fReplaceWildcard(false),
					fMatchDir(false)
				{ }

			CString sHost;
			CString sPath;
			CString sRedirect;
			CString sPathWithSlash;			//	If fMatchDir, this is sPath with a terminating slash

			DWORD fMatchAllPaths:1;			//	Match any path
			DWORD fMatchExactPath:1;		//	Path has no wildcards (otherwise, we match start of path)
			DWORD fMatchHost:1;				//	Only if host matches
			DWORD fReplaceWildcard:1;		//	If TRUE, the redirect pattern has a wildcard
			DWORD fMatchDir:1;				//	Path is a directory

			DWORD dwSpare:27;
			};

		bool MatchRedirect (const SRedirectDesc &Desc, const CString &sURLHost, const CString &sURLPath, CString *retsRedirect) const;
			 
		CString m_sService;
		CString m_sFormat;
		ETLSTypes m_iTLS;
		TArray<CString> m_HostsToServe;
		TArray<CString> m_PathsToServe;
		TArray<SRedirectDesc> m_Redirects;
	};

class CHexeCodeRPCService : public CHTTPService
	{
	protected:
		//	CHTTPService
		virtual bool OnHandleRequest (SHTTPRequestCtx &Ctx) override;
		virtual bool OnHandleRPCResult (SHTTPRequestCtx &Ctx, const SArchonMessage &RPCResult) override;
		virtual bool OnHTTPInit (CDatum dServiceDef, const CHexeDocument &Package, CString *retsError) override;
		virtual void OnHTTPMark (void) override;

	private:
		bool ComposeResponse (SHTTPRequestCtx &Ctx, CHexeProcess::ERun iRun, CDatum dResult);

		CHexeProcess m_ProcessTemplate;
		CString m_sOutputContentType;
	};

class CHTTPProxyService : public CHTTPService
	{
	protected:
		//	CHTTPService
		virtual bool OnHandleRequest (SHTTPRequestCtx &Ctx) override;
		virtual bool OnHandleRPCResult (SHTTPRequestCtx &Ctx, const SArchonMessage &RPCResult) override;
		virtual bool OnHTTPInit (CDatum dServiceDef, const CHexeDocument &Package, CString *retsError) override;

	private:
		CString m_sDestHost;
		DWORD m_dwDestPort;
	};

class CWebSocketService : public CHTTPService
	{
	protected:

		//	CHTTPService

		virtual bool OnHandleRequest (SHTTPRequestCtx &Ctx) override;
		virtual bool OnHandleRPCResult (SHTTPRequestCtx &Ctx, const SArchonMessage &RPCResult) override;
		virtual bool OnHTTPInit (CDatum dServiceDef, const CHexeDocument &Package, CString *retsError) override;

	private:

		CString GetAPI () const;

		CString m_sConnectMsg;
		TArray<CString> m_APIs;
	};

class CWWWService : public CHTTPService
	{
	protected:
		//	CHTTPService
		virtual CString OnGetFileRoot (void) const override;
		virtual bool OnHandleHexmFile (SHTTPRequestCtx &Ctx, CDatum dFileDesc, CDatum dData) override;
		virtual bool OnHandleRequest (SHTTPRequestCtx &Ctx) override;
		virtual bool OnHandleRPCResult (SHTTPRequestCtx &Ctx, const SArchonMessage &RPCResult) override;
		virtual bool OnHTTPInit (CDatum dServiceDef, const CHexeDocument &Package, CString *retsError) override;
		virtual void OnHTTPMark (void) override;

	private:
		CString GetFilePath (const CString &sPath);

		CString m_sWebPath;
		CString m_sFilePath;
		CString m_sDefaultFile;

		CHexeProcess m_ProcessTemplate;
		TArray<CDatum> m_HexeDefinitions;
		bool m_bSingleFile = false;
	};

//	Miscellaneous --------------------------------------------------------------

void RegisterSessionLibrary (void);
