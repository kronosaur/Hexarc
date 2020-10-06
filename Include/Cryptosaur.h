//	Cryptosaur.h
//
//	Cryptosaur Engine Implementation
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#pragma once

#include "Foundation.h"
#include "OpenSSLUtil.h"
#include "AEON.h"
#include "ArchonEngine.h"

//	CCryptosaurEngine ----------------------------------------------------------

class CCryptosaurEngine : public TSimpleEngine<CCryptosaurEngine>
	{
	public:
		CCryptosaurEngine (void);
		virtual ~CCryptosaurEngine (void);

		//	Sessions
		bool DoesAdminExist (void) { return m_bAdminExists; }
		bool FindKey (const CString &sKeyName) { return m_Keys.Find(sKeyName); }
		inline void InsertCertificate (const CString &sDomain, CDatum dCert) { CSmartLock Lock(m_cs); m_Certificates.SetAt(sDomain, dCert); }
		inline void InsertKey (const CString &sKeyName, const CIPInteger &Key) { CSmartLock Lock(m_cs); m_Keys.Insert(sKeyName, Key); }
		bool IsRunning (CString *retsError = NULL);
		void SetAdminExists (bool bExists = true);
		inline void SetAeonInitialized (bool bInitialized = true) { m_bAeonInitialized = bInitialized; }
		bool ValidateAuthDescActual (CDatum dAuthDesc, const CString &sScope, CDatum dUserData);

		//	Helpers
		CDatum GenerateAuthToken (CDatum dData, DWORD dwLifetime);
		static CString ValidateSandbox (const CString &sSandbox);
		static bool ValidateUsername (const CString &sUsername, CString *retsError);

		//	TSimpleEngine
		static SMessageHandler m_MsgHandlerList[];
		static int m_iMsgHandlerListCount;

	protected:
		virtual void OnBoot (void);
		virtual void OnMarkEx (void);
		virtual void OnStartRunning (void);
		virtual void OnStopRunning (void);

	private:
		bool ValidateAuthDescCreate (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx, CDatum dAuthDesc);
		bool ValidateMessage (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx, bool bRequireAdmin);
		bool ValidateRightsGrant (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx, const CAttributeList &Rights);

		static bool LoadPEMCertAndKey (CDatum dData, CDatum *retKey, CDatum *retRecord, CString *retsError);

		//	Messages
		void MsgAddRights (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgAeonOnStart (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgChangePassword (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgCheckPasswordSHA1 (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgCreateAdmin (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgCreateScopedCredentials (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgCreateUser (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgGetCertificate (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgGetKey (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgGetUser (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgHasRights (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgLoginUser (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgRemoveRights (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgRequestLogin (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgResetPasswordManual (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgSetCertificate (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgSignData (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgValidateAuthToken (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);

		CCriticalSection m_cs;
		bool m_bAeonInitialized;
		bool m_bAdminExists;

		//	Cached
		TSortMap<CString, CIPInteger> m_Keys;
		TSortMap<CString, CDatum> m_Certificates;
	};
