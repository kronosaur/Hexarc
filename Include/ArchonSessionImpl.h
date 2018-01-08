//	ArchonSessionImpl.h
//
//	Session Implementations
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by ArchonEngine.h

#pragma once

class CAeonFileDownloadSession : public ISessionHandler
	{
	public:
		CAeonFileDownloadSession (const CString &sReplyAddr, const CString &sFilePath, const CHexeSecurityCtx *pSecurityCtx = NULL, const CString &sRoot = NULL_STR, const CDateTime &IfModifiedAfter = CDateTime(), DWORD dwChunkSize = 100000);

	protected:
		//	CAeonFileDownloadSession virtuals
		virtual void OnFileDownloaded (CDatum dFileDesc, CDatum dData) { }
		virtual void OnFileUnmodified (void) { }

		//	ISessionHandler virtuals
		virtual void OnMark (void) override { m_dFileData.Mark(); }
		virtual bool OnProcessMessage (const SArchonMessage &Msg) override;
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override;

	private:
		bool SendFileDownloadRequest (int iOffset);

		CString m_sReplyAddr;
		CString m_sFilePath;
		const CHexeSecurityCtx *m_pSecurityCtx;
		CString m_sRoot;
		CDateTime m_IfModifiedAfter;
		DWORD m_dwChunkSize;

		CDatum m_dFileData;
	};

class CAeonInsertSession : public ISessionHandler
	{
	public:
		CAeonInsertSession (const CString &sReplyAddr, const CString &sTableName, CDatum dKeyPath, CDatum dData);

	protected:
		inline CDatum GetData (void) { return m_dData; }
		inline CDatum GetKeyPath (void) { return m_dKeyPath; }

		//	CAeonInsertSession virtuals
		virtual void OnSuccess (void) { }

		//	ISessionHandler virtuals
		virtual void OnMark (void) override;
		virtual bool OnProcessMessage (const SArchonMessage &Msg) override;
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override;

	private:
		CString m_sReplyAddr;
		CString m_sTableName;
		CDatum m_dKeyPath;
		CDatum m_dData;
	};

class CAeonMutateSession : public ISessionHandler
	{
	public:
		CAeonMutateSession (const CString &sReplyAddr, const CString &sTableName, CDatum dKeyPath, CDatum dData, CDatum dMutateDesc);

	protected:
		//	CAeonMutateSession virtuals
		virtual void OnSuccess (CDatum dData);

		//	ISessionHandler virtuals
		virtual void OnMark (void) override;
		virtual bool OnProcessMessage (const SArchonMessage &Msg) override;
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override;

	private:
		CString m_sReplyAddr;
		CString m_sTableName;
		CDatum m_dKeyPath;
		CDatum m_dData;
		CDatum m_dMutateDesc;
	};

class CMessageDispatchSession : public ISessionHandler
	{
	public:
		CMessageDispatchSession (const CString &sAddress, const CString &sMsg, const CString &sReplyAddr, CDatum dPayload);

	protected:
		//	ISessionHandler virtuals
		virtual void OnMark (void) override;
		virtual bool OnProcessMessage (const SArchonMessage &Msg) override;
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override;

	private:
		enum EStates
			{
			stateNone,

			stateWaitForMsg,
			};

		EStates m_iState;
		CString m_sAddress;
		CString m_sMsg;
		CString m_sReplyAddr;
		CDatum m_dPayload;
	};

class CMessageSerializerSession : public ISessionHandler
	{
	public:
		CMessageSerializerSession (void);

		inline void AddMessage (const SArchonEnvelope &Msg) { ASSERT(m_iPos == -1); m_List.Insert(Msg); }

	protected:
		//	CMessageSerializerSession virtuals
		virtual void OnComplete (void) { }
		virtual void OnReply (const SArchonEnvelope &Msg, const SArchonMessage &Reply) { }

		//	ISessionHandler virtuals
		virtual void OnMark (void) override;
		virtual bool OnProcessMessage (const SArchonMessage &Msg) override;
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override;

	private:
		TArray<SArchonEnvelope> m_List;
		int m_iPos;
	};