//	Cryptosaur.h
//
//	Cryptosaur Engine Implementation
//	Copyright (c) 2011 GridWhale Corporation. All Rights Reserved.
//
//	ARC.ACCOUNTS
//
//	Arc.accounts allows users to connect to external accounts, such as Google 
//	Drive, etc. It has one entry per user per account. It is indexed by
//	accountID, which is globally unique.
//
//	Each entry has the following fields:
//
//		accessCode: A structure containing access code information. The actual
//			fields depend on the type of account. For a Google account, this
//			includes:
//
//				access_token
//				expires_in
//				refresh_token
//				scope
//				token_type
//
//		createdOn: DateTime when the account entry was created.
//		name: Human-readable name (which may be changed by user)
//		type: The type of account. One of:
//
//			googleDrive
//
//		username: The username this account belongs to.

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
		void InsertKey (const CString &sKeyName, CDatum dKey);
		bool IsRunning (CString *retsError = NULL);
		void SetAdminExists (bool bExists = true);
		inline void SetAeonInitialized (bool bInitialized = true) { m_bAeonInitialized = bInitialized; }
		bool ValidateAuthDescActual (CDatum dAuthDesc, const CString &sScope, CDatum dUserData);

		//	Helpers
		CDatum GenerateAuthToken (CDatum dData, DWORD dwLifetime);
		static bool IsKnownExternalKey (const CString &sID);
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
		void MsgCreateAuthToken (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgCreateScopedCredentials (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgCreateUser (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgDeleteUser (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgGetCertificate (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgGetKey (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgGetUser (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgHasRights (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgListKeys (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgLoginUser (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgRemoveRights (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgRequestLogin (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgResetPasswordManual (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgSetCertificate (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgSetKey (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgSignData (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);
		void MsgValidateAuthToken (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx);

		CCriticalSection m_cs;
		bool m_bAeonInitialized;
		bool m_bAdminExists;

		//	Cached
		TSortMap<CString, CIPInteger> m_Keys;
		TSortMap<CString, CString> m_ExternalKeys;
		TSortMap<CString, CDatum> m_Certificates;
	};
