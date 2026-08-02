//	GridWhaleCX.h
//
//	GridWhale Classes and Functions
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "GridWhaleUser.h"

class CProgramPermissions
	{
	public:
		enum class EDefault
			{
			Safe,							//	Default safe permissions
			Trusted,						//	Default for trusted programs
			};

		CProgramPermissions (EDefault iDefault = EDefault::Safe);
		CProgramPermissions (CDatum dData);

		CDatum AsDatum () const;
		CString GetDesc () const;
		const CFilePermissions& GetPermissionsForClass (EFileClass iClass) const
			{
			if ((int)iClass < 0 || (int)iClass >= 3)
				throw CException(errFail);

			return m_PermissionsByClass[(int)iClass];
			}

		static CString GetID (EFileClass iClass);
		static EFileClass ParseClass (const CString& sString);

	private:
		CFilePermissions m_PermissionsByClass[(int)EFileClass::Count];
	};

class CProgramLimits
	{
	public:

		CProgramLimits (); 
		CProgramLimits (CDatum dData);

		static CProgramLimits Max ();

		CDatum AsDatum () const;
		const IInvokeCtx::SLimits &GetComputeLimits () const { return m_ComputeLimits; }
		DWORD GetMaxMarkTime () const { return m_dwMaxMarkTime; }
		int GetMaxProcessCount () const { return m_iMaxProcessCount; }
		int GetMaxProcessCountPerMinute () const { return DEFAULT_MAX_PROCESS_PER_MINUTE; }
		DWORD GetMaxUtilizationPeriod () const { return m_dwMaxUtilizationPeriod; }
		double GetMaxUtilizationRate () const { return m_rMaxUtilizationRate; }
		void SetComputeLimits (const IInvokeCtx::SLimits &Limits) { m_ComputeLimits = Limits; }
		void SetMaxMarkTime (DWORD dwTime) { m_dwMaxMarkTime = dwTime; }
		void SetMaxProcessCount (int iCount) { m_iMaxProcessCount = iCount; }
		void SetMaxUtilizationPeriod (DWORD dwPeriod) { m_dwMaxUtilizationPeriod = dwPeriod; }
		void SetMaxUtilizationRate (double rRate) { m_rMaxUtilizationRate = rRate; }

	private:

		static constexpr double DEFAULT_MAX_UTILIZATION_RATE = 0.05;
		static constexpr DWORD DEFAULT_MAX_UTILIZATION_PERIOD = 60;		//	Max time in seconds before we check utilization
		static constexpr int DEFAULT_MAX_COMPUTE_SLICE = 5;				//	Time before StopCheck is called (seconds)
		static constexpr int DEFAULT_MAX_PROCESS_COUNT = 10;
		static constexpr int DEFAULT_MAX_PROCESS_PER_MINUTE = 100;
		static constexpr int DEFAULT_MAX_MARK_TIME = 1000;				//	Max time to mark object (milliseconds)

		int m_iMaxProcessCount = DEFAULT_MAX_PROCESS_COUNT;
		IInvokeCtx::SLimits m_ComputeLimits;
		DWORD m_dwMaxUtilizationPeriod = DEFAULT_MAX_UTILIZATION_PERIOD;	//	Max time in seconds before we check utilization
		DWORD m_dwMaxMarkTime = DEFAULT_MAX_MARK_TIME;						//	Max time to mark object (milliseconds)
		double m_rMaxUtilizationRate = DEFAULT_MAX_UTILIZATION_RATE;
													//	Max fraction of wall time spent computing
													//		This helps prevent runaway programs from
													//		consuming all quota.
	};

class CProgramRunInfo
	{
	public:

		CProgramRunInfo () { }
		CProgramRunInfo (CStringView sInstanceID, CStringView sProgramID, CStringView sProgramName, const CGridName& Owner, const CGridUser& RunBy, const CProgramPermissions& Permissions, const CProgramLimits& Limits) :
				m_sInstanceID(sInstanceID),
				m_dwInstanceID(ParseInstanceID(sInstanceID)),
				m_sID(sProgramID),
				m_sProgramName(sProgramName),
				m_Owner(Owner),
				m_RunBy(RunBy),
				m_Permissions(Permissions),
				m_Limits(Limits)
			{ }
		explicit CProgramRunInfo (const CGridUser& RunBy);
		explicit CProgramRunInfo (CDatum dInfo);

		CDatum AsDatum () const;
		CGridID GetDatasetID () const { return CGridID::ComposeInstance(GetInstanceID()); }
		const CString& GetID () const { return m_sID; }
		DWORD GetInstanceID_Old () const { return m_dwInstanceID; }
		CString GetInstanceID () const { return m_sInstanceID; }
		CStringView GetName () const { return m_sProgramName; }
		const CGridName& GetOwner () const { return m_Owner; }
		const CProgramPermissions& GetPermissions () const { return m_Permissions; }
		const CGridUser& GetRunBy () const { return m_RunBy; }
		bool InitFromOptions (CDatum dOptions);
		bool InitFromOptionsWithGridUser (CDatum dOptions);
		bool IsValid () const { return !m_sID.IsEmpty() && !m_Owner.IsEmpty() && !m_RunBy.IsEmpty(); }

		static CString ComposeInstanceID (const CString& sNodeID, DWORD dwID) { return strPattern("%s/%08x", sNodeID, dwID); }
		static DWORD ParseInstanceID (const CString& sInstanceID);

	private:

		CString m_sInstanceID;				//	Program instance ID
		DWORD m_dwInstanceID = 0;			//	Old instance ID
		CString m_sID;						//	Program ID
		CString m_sProgramName;				//	Program name
		CGridName m_Owner;					//	Program owner
		CGridUser m_RunBy;					//	User who ran the program

		CProgramPermissions m_Permissions;
		CProgramLimits m_Limits;
	};

class CGridProgramRunRecord
	{
	public:

		CGridProgramRunRecord () { }
		explicit CGridProgramRunRecord (CDatum dValue);

		CDatum AsDatum () const;
		CDatum AsStorage () const;
		void SetProgram (const CGridID& ID, CStringView sName, CStringView sIcon, CStringView sVersion, const CGridName& Owner, const CDateTime& CreatedOn, const CDateTime& ModifiedOn, CStringView sDesc);
		void SetUserRun (const CGridName& Username);

	private:

		CGridID m_ID;						//	GridID of the program file
		CString m_sProgramID;				//	Program ID
		CGridName m_Username;				//	User who ran the program
		CString m_sProgramName;				//	Program name
		CString m_sProgramIcon;				//	Program icon
		CString m_sProgramVersion;			//	Program version
		CGridName m_ProgramOwner;			//	Program owner
		CDateTime m_ProgramCreatedOn;		//	Program created on
		CDateTime m_ProgramModifiedOn;		//	Program last modified on
		CString m_sDesc;					//	Program description

		CDateTime m_LastRunOn;				//	Date/time program started by user
		CDateTime m_LastUsedOn;				//	Date/time program last used by user
	};
