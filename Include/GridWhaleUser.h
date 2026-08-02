//	GridWhaleUser.h
//
//	GridWhale Classes and Functions
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "Foundation.h"
#include "GridWhaleData.h"

class CGridUser;

enum class EGridGroup
	{
	None,

	Directors,			//	Root access to Grid
	Architects,			//	Can modify Grid APIs
	Engineers,			//	Can create special programs
	Operators,			//	Can maintain the Grid
	Curators			//	Can modify Archive folder
	};

enum class EGridUser
	{
	None,

	Overlord,			//	Root owner
	};

class CGridName
	{
	public:

		enum class EType
			{
			Unknown,

			Group,
			GroupList,
			Org,
			User,
			};

		CGridName () { }
		explicit CGridName (CDatum dDesc);
		CGridName (const CGridUser& User);
		CGridName (const CString& sID, const CString& sName);
		explicit CGridName (EGridGroup iGroup);
		explicit CGridName (EGridUser iUser);

		inline CGridName& operator= (const CGridUser& User);
		bool operator== (const CGridName& Src) const { return strEquals(m_sID, Src.m_sID); }
		bool operator!= (const CGridName& Src) const { return !strEquals(m_sID, Src.m_sID); }
		bool operator> (const CGridName& Src) const { return KeyCompare(m_sID, Src.m_sID) == 1; }
		bool operator< (const CGridName& Src) const { return KeyCompare(m_sID, Src.m_sID) == -1; }

		CDatum AsGridNameType () const;
		CDatum AsDatum () const;
		CString AsEncoded () const { return Encode(m_sID, m_sName); }
		CDatum AsStruct () const;
		int Compare (const CGridName& Src) const { return KeyCompare(m_sID, Src.m_sID); }
		const CString &GetID () const { return m_sID; }
		const CString &GetName () const { return m_sName; }
		EType GetType (CString *retsRoot = NULL, CString *retsComponent = NULL) const { return ParseComponents(m_sID, retsRoot, retsComponent); }
		bool IsEmpty () const { return m_sID.IsEmpty() || m_sName.IsEmpty(); }
		bool IsOverlord () const;

		static CString Clean (const CString& sValue);
		static CGridName FromName (const CString& sName);
		static EType GetType (const CString& sID) { return ParseComponents(sID); }
		static EType ParseComponents (const CString& sID, CString *retsRoot = NULL, CString *retsComponent = NULL);
		static EType ParseType (const CString& sType);
		static CString ToID (const CString& sName);

		static CGridName Decode (const CString& sEncoded);
		static CString Encode (const CString& sID, const CString& sName);
		static bool Validate (const CString& sName);

	private:

		static bool HasChar (const char *pChar, const CString &sCharList);

		CString m_sID;						//	Canonical ID (always lowercase)
		CString m_sName;					//	Propercase
	};

class CGridNameList
	{
	public:

		CGridNameList () { }
		explicit CGridNameList (CDatum dData);

		bool AddName (const CGridName &Name);
		CDatum AsDatum () const;
		CDatum AsTable () const;
		bool FindName (const CString &sID) const;
		int GetCount () const { return m_List.GetCount(); }
		CGridName GetName (int iIndex) const { return CGridName(m_List.GetKey(iIndex), m_List[iIndex]); }
		bool RemoveName (const CGridName& Name);

	private:

		//	Key is ID; Value is the Name.

		TSortMap<CString, CString> m_List;
	};

enum class EAccountProtocol
	{
	None,

	Apple,									//	Apple services
	AWS,									//	Amazon Web Services
	BasicAuth,								//	Basic authentication account
	FTP,									//	FTP server
	GenericAPI,								//	Generic API key
	GitHub,									//	GitHub App installation
	GridWhale,								//	GridWhale services
	GridWhaleEmail,							//	Auth by email
	Google,									//	Google services
	Hexarc,									//	Authenticate with Hexarc.com
	Microsoft,								//	Microsoft services
	SendGrid,								//	SendGrid services
	Stripe,									//	Stripe services
	};

enum class EAccountService
	{
	None,

	API,
	Auth,
	Email,
	Files,
	};

class IExternalAccount
	{
	public:

		static constexpr DWORD FLAG_USER_CREATED = 0x00000001;

		struct SCreate
			{
			CString sName;
			CString sSourceName;
			CString sIconID;

			DWORD dwFlags = 0;
			};

		struct SAPIDesc
			{
			CString sID;						//	API ID in the external system.
			CString sName;						//	API name (human-readable)
			};

		struct SConnectionCreate
			{
			CGridName User;
			CString sID;
			CString sName;
			CString sSourceName;
			CString sIconID;
			EAccountProtocol iProtocol = EAccountProtocol::None;
			TArray<EAccountService> Services;

			TArray<SAPIDesc> APIList;
			CString sDomain;
			CDatum dData;
			};

		struct SSendEmail
			{
			CEmailAddress From;
			CEmailAddress To;
			CString sSubject;
			CDatum dBody;
			};

		enum EDeleteAction
			{
			None,

			HexarcMsg,							//	Send a Hexarc message
			HTTPPost,							//	POST to URL
			};

		struct SDeleteInfo
			{
			EDeleteAction iAction = EDeleteAction::None;
			CString sURL;

			CString sAddr;
			CString sMsg;
			CDatum dPayload;
			};

		IExternalAccount (const CString& sID, EAccountProtocol iProtocol = EAccountProtocol::None) :
				m_sID(sID),
				m_iProtocol(iProtocol)
			{ }

		IExternalAccount (const CString& sID, EAccountProtocol iProtocol, const SCreate& Create) :
				m_sID(sID),
				m_sName(Create.sName),
				m_sSourceName(Create.sSourceName),
				m_sIconID(Create.sIconID),
				m_iProtocol(iProtocol),
				m_bUserCreated(Create.dwFlags & FLAG_USER_CREATED)
			{ }

		IExternalAccount (CDatum dDesc);
		virtual ~IExternalAccount () { }

		static TUniquePtr<IExternalAccount> Create (EAccountProtocol iProtocol, CDatum dData, CStringView sCryptID);
		static TArray<TUniquePtr<IExternalAccount>> CreateAccounts (CDatum dDesc);
		static TUniquePtr<IExternalAccount> CreateEmailAuthAccount (const CEmailAddress& Email);
		static TUniquePtr<IExternalAccount> CreatePasswordAuthAccount (CStringView sUserID, EAccountProtocol iProtocol);

		CDatum AsDatum (bool bPublic = false) const;
		CDatum AsGridConnection (const CGridName& User) const;
		TUniquePtr<IExternalAccount> Clone () const { return OnClone(); }
		bool GenerateSendEmail (const SSendEmail& Options, SHexarcMsg& retMsg) const { return OnGenerateSendEmail(Options, retMsg); }
		CString GetClass () const { return OnGetClass(); }
		CString GetCryptID () const { return OnGetCryptID(); }
		CDatum GetData () const { return OnGetData(); }
		SDeleteInfo GetDeleteInfo () const { return OnGetDeleteInfo(); }
		CDatum GetFTPAuth () const { return OnGetFTPAuth(); }
		CDatum GetHTTPAuth () const { return OnGetHTTPAuth(); }
		CStringView GetIcon () const { return m_sIconID; }
		const CString& GetID () const { return m_sID; }
		const CString& GetName () const { return m_sName; }
		CString GetOAuthState (const CGridName& User) { return OnGetOAuthState(User); }
		CString GetServiceIDList () const;
		TArray<EAccountService> GetServiceList () const;
		EAccountProtocol GetProtocol () const { return m_iProtocol; }
		CStringView GetSourceName () const { return m_sSourceName; }
		bool HasService (EAccountService iService) const { return OnHasService(iService); }
		bool IsCreatedByUser () const { return m_bUserCreated; }
		bool MatchesFindData (CDatum dData) const;
		bool OAuthConnect (const CString& sOAuthState, CDatum dData) { return OnOAuthConnect(sOAuthState, dData); }
		void SetName (const CString& sName) { m_sName = sName; }
		bool Update (CDatum dData);
		bool ValidateAuth (CDatum dValue) const { return OnValidateAuth(dValue); }

		static CString AsID (EAccountService iService);
		static CString AsID (EAccountProtocol iProtocol);
		static EAccountService AsService (const CString& sValue);
		static TArray<EAccountService> AsServiceList (const CString& sValue);
		static EAccountProtocol AsSource (const CString& sValue);
		static bool ParseOAuthState (const CString& sOAuthState, CGridName* retUser = NULL, CString* retsConnectionID = NULL, CString* retsCode = NULL);
		static bool ValidateCreate (EAccountProtocol iProtocol, CDatum dData, CString* retsError = NULL);

	protected:

		IExternalAccount (const IExternalAccount& Src) = default;

		void SetCreatedByUser (bool bValue = true) { m_bUserCreated = bValue; }

	private:

		virtual void OnAccumulateConnectionCreate (SConnectionCreate& Create) const { }
		virtual void OnAccumulateDatum (CDatum dDesc, bool bPublic) const { }
		virtual TUniquePtr<IExternalAccount> OnClone () const = 0;
		virtual bool OnGenerateSendEmail (const SSendEmail& Options, SHexarcMsg& retMsg) const { return false; }
		virtual CString OnGetClass () const = 0;
		virtual CString OnGetCryptID () const { return NULL_STR; }
		virtual CDatum OnGetData () const { return CDatum(); }
		virtual SDeleteInfo OnGetDeleteInfo () const { return SDeleteInfo(); }
		virtual CDatum OnGetFTPAuth () const { return CDatum(); }
		virtual CDatum OnGetHTTPAuth () const { return CDatum(); }
		virtual CString OnGetOAuthState (const CGridName& User) { return NULL_STR; }
		virtual bool OnHasService (EAccountService iService) const { return false; }
		virtual bool OnMatchesFindData (CDatum dData) const { return true; }
		virtual bool OnOAuthConnect (const CString& sOAuthState, CDatum dData) { return false; }
		virtual bool OnUpdate (CDatum dData) { return false; }
		virtual bool OnValidateAuth (CDatum dValue) const { return false; }

		CString m_sID;						//	Account ID (globally unique)
		CString m_sName;					//	User-visible account name
		CString m_sSourceName;				//	Source name (e.g., "Google")
		CString m_sIconID;					//	GridID for icon
		EAccountProtocol m_iProtocol = EAccountProtocol::None;
		bool m_bUserCreated = false;		//	TRUE if this account was created by a user (as opposed to a system account)
	};

class CGridAccountList
	{
	public:

		CGridAccountList () { }
		CGridAccountList (const CGridAccountList& Src) { Copy(Src); }
		CGridAccountList (CGridAccountList&& Src) noexcept = default;
		explicit CGridAccountList (TArray<TUniquePtr<IExternalAccount>>&& Src) :
				m_List(std::move(Src))
			{ }

		explicit CGridAccountList (CDatum dDesc) :
				m_List(CreateGenericAccounts(dDesc))
			{ }

		CGridAccountList& operator= (const CGridAccountList& Src) { CleanUp(); Copy(Src); return *this; }
		CGridAccountList& operator= (CGridAccountList&& Src) noexcept = default;
			
		bool AddAccount (TUniquePtr<IExternalAccount>&& pAccount);
		CDatum AsDatum () const;
		CDatum AsGridUser () const;
		CDatum AsTable () const;
		const IExternalAccount* FindAccount (const CString& sID) const { return const_cast<CGridAccountList*>(this)->FindAccount(sID); }
		IExternalAccount* FindAccount (const CString& sID);
		const IExternalAccount* FindAccount (EAccountService iService, EAccountProtocol iProtocol = EAccountProtocol::None) const;
		const IExternalAccount* FindAccount (EAccountService iService, CDatum dFindData) const;
		void FindAccounts (EAccountService iService, CDatum dFindData, TArray<const IExternalAccount*> &retAccounts) const;
		bool GenerateSendEmail (const IExternalAccount::SSendEmail& Options, SHexarcMsg& retMsg) const;
		const IExternalAccount& GetAccount (int iIndex) const { if (iIndex >= 0 && iIndex < m_List.GetCount()) return *m_List[iIndex]; else throw CException(errFail); }
		int GetCount () const { return m_List.GetCount(); }
		bool HasService (EAccountService iService, EAccountProtocol iProtocol = EAccountProtocol::None, CDatum* retdData = NULL) const;
		bool RemoveAccount (const CString& sID, IExternalAccount::SDeleteInfo& retDeleteInfo);
		bool ReplaceAccount (TUniquePtr<IExternalAccount>&& pAccount);

		static TArray<TUniquePtr<IExternalAccount>> CreateGenericAccounts (CDatum dDesc);
		static CString MakeID (EAccountProtocol iProtocol, EAccountService iService, CStringView sCryptID = CStringView(), CStringView sSourceName = CStringView());

	private:

		void CleanUp () { m_List.DeleteAll(); }
		void Copy (const CGridAccountList& Src);

		TArray<TUniquePtr<IExternalAccount>> m_List;
	};

class CGridGroupHierarchy
	{
	public:
		CGridGroupHierarchy () { }
		CGridGroupHierarchy (CDatum dData);

		void AddChain (const CString& sChain);
		void AddChain (const CGridName& Name) { AddChain(Name.AsEncoded()); }
		void AddChain (const CString& sRoot, const CGridName& Name)
			{ AddChain(MakeChain(sRoot, Name)); }
		CDatum AsDatum () const;
		CDatum AsTable () const;
		int GetCount () const { return m_Groups.GetCount(); }
		CString GetChain (int iIndex) const { return m_Groups[iIndex]; }
		CGridNameList GetGroupList () const;
		bool FindChain (const CString& sChain) const;
		bool FindGroup (const CGridName& Group) const;
		CString MakeChain (int iIndex, const CGridName& Name) const { return MakeChain(GetChain(iIndex), Name); }
		bool RemoveChain (const CString& sChain);
		bool RemoveChain (const CGridName& Name) { return RemoveChain(Name.AsEncoded()); }

		static CString MakeChain (const CString& sRoot, const CGridName& Name);

	private:
		static CString GetGroupFromChain (const CString& sChain);

		TArray<CString> m_Groups;
	};

class CGridUserEmail
	{
	public:

		CGridUserEmail () { }
		CGridUserEmail (const CString& sEmail) :
				m_sEmail(sEmail)
			{ }
		CGridUserEmail (CDatum dDesc);

		CDatum AsDatum () const;
		const CString& GetEmail () const { return m_sEmail; }
		bool IsEmpty () const { return m_sEmail.IsEmpty(); }
		bool IsVerified () const { return !IsEmpty() && m_VerifiedOn.IsValid(); }
		void SetVerified (bool bValue = true) { m_VerifiedOn = (bValue ? CDateTime(CDateTime::Now) : CDateTime()); }

	private:

		CString m_sEmail;
		CDateTime m_VerifiedOn;				//	Set if we've verified the email.
	};

class CGridUserProfile
	{
	public:

		static constexpr DWORD FLAG_PUBLIC_INFO = 0x00000001;

		CGridUserProfile () {}
		CGridUserProfile (CDatum dDesc);

		CDatum AsDatum (DWORD dwFlags = 0) const;
		const CGridID& GetImage () const { return m_Image; }
		void SetImage (const CGridID& GridID) { m_Image = GridID; }
		void SetPrimaryEmail (const CGridUserEmail& Email) { m_PrimaryEmail = Email; }

	private:

		CGridUserEmail m_PrimaryEmail;
		CGridID m_Image;
	};

class CGridUserPreferences
	{
	public:

		struct SProgramPreferences
			{
			CGridID ID;						//	Program GridID (e.g., "/file/{programID}")
			bool bEnableNotifications = false;	//	TRUE if notifications are enabled
			};

		CGridUserPreferences () { InitDefaults(); }
		explicit CGridUserPreferences (CDatum dDesc);

		CDatum AsDatum () const;
		static SProgramPreferences DefaultProgramPreferences (CGridID ProgramID);
		bool FindProgram (CGridID ProgramID, SProgramPreferences* retPreferences = NULL) const;
		void SetProgram (CGridID ProgramID, const SProgramPreferences& Preferences);

	private:

		static void InitColumnIndexes ();
		void InitDefaults ();
		static TSortMap<CString, SProgramPreferences> LoadProgramPreferences (CDatum dData);

		CString m_sUITheme;					//	Theme (GridID or "default")
		CString m_sFileManager;				//	Default file manager (GridID or "default")

		CString m_sLanguage;				//	Language (IETF BCP 47)
		CString m_sTimeZone;				//	Time zone (IANA time zone ID)
		CString m_sCurrency;				//	Currency (ISO 4217)

		static int m_ENABLE_NOTIFICATIONS_COL;
		static int m_ID_COL;

		TSortMap<CString, SProgramPreferences> m_ProgramPreferences;
	};

class CGridUser
	{
	public:

		CGridUser () { }
		explicit CGridUser (const CGridName& Username) : m_Username(Username) { }
		explicit CGridUser (CDatum dDesc);

		CDatum AsDatum () const;
		CDatum AsDesc () const;
		const CGridAccountList& GetAccounts () const { return m_Accounts; }
		const CGridNameList& GetGroups () const { return m_Groups; }
		const CString& GetID () const { return m_Username.GetID(); }
		const CGridName& GetName () const { return m_Username; }
		const CGridName& GetOrg () const { return m_OrgName; }
		const CGridUserPreferences& GetPreferences () const { return m_Preferences; }
		const CGridUserProfile& GetProfile () const { return m_Profile; }
		bool HasAnyLicense () const { return m_Licenses.GetCount() > 0; }
		bool HasLicense (CStringView sLicenseID) const { return m_Licenses.GetAt(strToLower(sLicenseID)) != NULL; }
		bool InGroup (const CGridName& Group) const { return m_Groups.FindName(Group.GetID()); }
		bool InGroup (EGridGroup iGroup) const { return InGroup(CGridName(iGroup)); }
		bool IsDisposable () const { return m_bDisposable; }
		bool IsEmpty () const { return m_Username.IsEmpty(); }
		bool IsValid () const { return m_Seq > 0; }
		void SetAccounts (const CGridAccountList& Accounts) { m_Accounts = Accounts; }
		void SetDisposable (bool bValue = true) { m_bDisposable = bValue; }
		void SetGroups (const CGridNameList& Groups) { m_Groups = Groups; }
		void SetGroupsOwned (const CGridNameList& Groups) { m_GroupsOwned = Groups; }
		void SetPreferences (const CGridUserPreferences& Preferences) { m_Preferences = Preferences; }
		void SetProfile (const CGridUserProfile& Profile) { m_Profile = Profile; }
		void SetSeq (SequenceNumber Seq) { m_Seq = Seq; }

	private:

		static CDatum AsDatum (const TSortMap<CString, bool>& Licenses);
		TSortMap<CString, bool> ParseLicenses (CDatum dValue);

		CGridName m_Username;				//	Username
		CGridName m_OrgName;				//	Org that the user is in.
		CGridNameList m_Groups;				//	Groups that we belong to

		CGridUserProfile m_Profile;			//	Profile
		CGridUserPreferences m_Preferences;	//	Preferences
		CGridAccountList m_Accounts;		//	Accounts owned by this user
		CGridNameList m_GroupsOwned;		//	Groups that are owned by this user
		TSortMap<CString, bool> m_Licenses;	//	List of licenses

		bool m_bDisposable = false;

		SequenceNumber m_Seq = 0;
	};

//	Inlines --------------------------------------------------------------------

inline CGridName& CGridName::operator= (const CGridUser& User)
	{ *this = User.GetName(); return *this; }
