//	GridWhaleLicensing.h
//
//	GridWhale Classes and Functions
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#pragma once

class CGridLicensing
	{
	public:

		static bool LicenseMutateDeleteAt (CDatum dCurValue, CDatum dChange, const CProgramRunInfo& ProgramInfo, CDatum& retdReply);
		static bool LicenseMutatePush (CDatum dCurValue, CDatum dChange, const CProgramRunInfo& ProgramInfo, CDatum& retdReply);
		static bool LicenseMutateSetAt (CDatum dCurValue, CDatum dChange, const CProgramRunInfo& ProgramInfo, CDatum& retdReply);
		static bool OfferingMutateDeleteAt (CDatum dCurValue, CDatum dChange, const CProgramRunInfo& ProgramInfo, CDatum& retdReply);
		static bool OfferingMutatePush (CDatum dCurValue, CDatum dChange, const CProgramRunInfo& ProgramInfo, CDatum& retdReply);
		static bool OfferingMutateSetAt (CDatum dCurValue, CDatum dChange, const CProgramRunInfo& ProgramInfo, CDatum& retdReply);
		static bool ParseID (CStringView sID, CString* retOwner = NULL, CString* retsName = NULL);

	private:

		static bool CanUseLicensesInOffering (CDatum dLicenses, const CGridUser& User, CString* retsError = NULL);
	};

class CGridKeyCode
	{
	public:

		enum class EStatus
			{
			Unknown,

			Unassigned,
			Active,
			Paused,
			Expired,
			Revoked,
			};

		enum class EAssignResult
			{
			OK,									//	Assignment successful
			AlreadyAssigned,					//	Key code is already assigned to that user (no changes needed)

			Error,								//	Could not assign the key code
			};

		enum class ERevokeResult
			{
			OK,
			AlreadyRevoked,

			Error,
			};

		struct SInitialTime
			{
			int iDays = 0;
			int iMonths = 0;
			int iYears = 0;
			};

		struct SCreate
			{
			CString sOfferingID;
			CGridName CreatedBy;
			TArray<CString> Tags;
			SInitialTime InitialTime;
			};

		CGridKeyCode () { }
		explicit CGridKeyCode (CDatum dValue) { InitFromDatum(dValue); }
		static CGridKeyCode Create (const SCreate& Create);

		CDatum AsDatum () const;
		EAssignResult AssignTo (const CGridName& User, CString* retsError = NULL);
		CDateTime GetExpiresOn () const { return m_ExpiresOn; }
		CStringView GetID () const { return m_sKeyCode; }
		int GetInitialTimeDays (const CDateTime& Start) const;
		CStringView GetOfferingID () const { return m_sOfferingID; }
		EStatus GetStatus () const { return m_iStatus; }
		const CGridName& GetUsername () const { return m_Username; }
		bool InitFromDatum (CDatum dValue);
		ERevokeResult Revoke (const CGridName& RevokedBy, CStringView sRevokeReason, CString* retsError = NULL);
		void SetExpiresOn (const CDateTime& ExpiresOn) { m_ExpiresOn = ExpiresOn; }

		static CString AsID (EStatus iStatus);
		static CDateTime CalcExpiration (const CDateTime& CurrentExpiration, const SInitialTime& TimeAdj);
		static EStatus ParseID (CStringView sValue);
		static bool ParseTime (CDatum dTime, SInitialTime* retTime);

	private:

		static CString MakeKeyCode ();

		CString m_sKeyCode;						//	The key code as a string of characters
		CString m_sOfferingID;					//	Offering ID
		EStatus m_iStatus = EStatus::Unknown;	//	Status of the key code
		DWORD m_dwVersion;						//	Version number for optimistic concurrency

		CGridName m_CreatedBy;					//	User that created the key code
		CDateTime m_CreatedOn;					//	Date the key code was created
		TArray<CString> m_Tags;					//	Tags associated with the key code

		CGridName m_Username;					//	User that owns the key code (if assigned)
		CDateTime m_ActivatedOn;				//	Date the key code was activated

		CDateTime m_ExpiresOn;					//	Date the key code expires
		SInitialTime m_InitialTime;				//	Initial time to set when activated.

		CGridName m_RevokedBy;					//	User that revoked the key code
		CDateTime m_RevokedOn;					//	Date the key code was revoked
		CString m_sRevokedReason;				//	Reason the key code was revoked
	};

class CGridActiveKeyCode
	{
	public:

		CGridActiveKeyCode () { }
		CGridActiveKeyCode (const CGridKeyCode& KeyCode, CDatum dOffering);
		explicit CGridActiveKeyCode (CDatum dValue) { InitFromDatum(dValue); }

		CDatum AsDatum () const;
		void DeleteOffering ();
		CStringView GetDesc () const { return m_sDesc; }
		CDateTime GetExpiresOn () const { return m_ExpiresOn; }
		CGridID GetIcon () const { return m_Icon; }
		CStringView GetKeyCode () const { return m_sKeyCode; }
		const TArray<CString>& GetLicenses () const { return m_Licenses; }
		CStringView GetName () const { return m_sName; }
		CStringView GetOfferingID () const { return m_sOfferingID; }
		CDateTime GetOfferingModifiedOn () const { return m_OfferingModifiedOn; }
		CGridKeyCode::EStatus GetStatus () const { return m_iStatus; }
		bool InitFromDatum (CDatum dValue);
		bool IsActive () const { return m_iStatus == CGridKeyCode::EStatus::Active; }
		void Revoke () { m_iStatus = CGridKeyCode::EStatus::Revoked; }
		void UpdateOffering (CDatum dOffering);

	private:

		CString m_sKeyCode;						//	Key code that we used
		CGridKeyCode::EStatus m_iStatus = CGridKeyCode::EStatus::Active;		//	Status of the key code
		CDateTime m_ExpiresOn;					//	Date the key code expires

		CString m_sOfferingID;					//	Offering ID
		CString m_sName;						//	Name of the offering
		CGridID m_Icon;							//	Icon for the offering
		CString m_sDesc;						//	Description of the offering
		CDateTime m_OfferingModifiedOn;			//	Date the offering was last modified

		TArray<CString> m_Licenses;				//	List of licenses granted by the offering
	};

class CGridKeyCodeList
	{
	public:

		CGridKeyCodeList () { }
		explicit CGridKeyCodeList (CDatum dValue) { InitFromDatum(dValue); }

		void AddKeyCode (const CGridActiveKeyCode& KeyCode);
		CDatum AsDatum () const;
		CDatum AsTable () const;
		TArray<CString> GetLicenseList () const;
		CDatum GetOfferingTimeStamps () const;
		bool HasLicense (CStringView sLicense) const;
		bool InitFromDatum (CDatum dValue);
		void RevokeKeyCode (CStringView sKeyCode);
		bool UpdateOfferings (CDatum dModified, CDatum dDeleted);

	private:

		void InitLicenses ();

		TSortMap<CString, CGridActiveKeyCode> m_List;
		TSortMap<CString, CString> m_Licenses; 
	};

class CGridTransaction
	{
	public:

		enum class EType
			{
			Unknown,

			Activate,								//	Activate the key
			Adjust,									//	Subtract time on the key
			Credit,									//	Add time to the key
			Purchase,								//	Purchase time for the key
			Refund,									//	Refund money to the user
			Revoke,									//	Revoke the key
			};

		CGridTransaction () { }
		explicit CGridTransaction (CDatum dValue) { InitFromDatum(dValue); }

		CDatum AsDatum () const;
		int GetAmount () const { return m_iAmount; }
		const CGridName& GetBy () const { return m_By; }
		CStringView GetCurrency () const { return m_sCurrency; }
		double GetFees () const { return m_rFees; }
		CStringView GetID () const { return m_sID; }
		CStringView GetKeyCode () const { return m_sKeyCode; }
		CStringView GetNotes () const { return m_sNotes; }
		CStringView GetOfferingID () const { return m_sOfferingID; }
		CDateTime GetOn () const { return m_On; }
		CStringView GetPaymentDetails () const { return m_sPaymentDetails; }
		double GetPrice () const { return m_rPrice; }
		double GetTaxes () const { return m_rTaxes; }
		double GetTotal () const { return m_rTotal; }
		EType GetType () const { return m_iType; }
		const CGridName& GetUsername () const { return m_Username; }
		bool InitFromDatum (CDatum dValue);
		bool InitFromDesc (const CGridName& Username, EType iType, CDatum dDesc);
		void SetID (CStringView sID) { m_sID = sID; }

		static CString AsID (EType iType);
		static CString MakeID ();
		static EType ParseID (CStringView sValue);

	private:

		CString m_sID;								//	Unique transaction ID
		CString m_sKeyCode;							//	Key code for this transaction
		CGridName m_Username;						//	User who owns the key code
		CString m_sOfferingID;						//	Offering for this transaction

		CDateTime m_On;								//	Date/time of the transaction
		CGridName m_By;								//	User who initiated the transaction
		EType m_iType = EType::Unknown;				//	Type of transaction

		double m_rPrice = 0.0;						//	Price of the transaction
		double m_rTaxes = 0.0;						//	Taxes paid
		double m_rFees = 0.0;						//	Transaction fees
		double m_rTotal = 0.0;						//	Total amount paid (or credited)
		CString m_sCurrency;						//	Currency used (for now, always "USD")
		CString m_sPaymentDetails;					//	Details of the payment

		int m_iAmount = 0;							//	Amount of time added or subtracted (days)
		CString m_sNotes;							//	Notes about the transaction
	};
