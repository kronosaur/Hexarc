//	GridWhalePermissions.h
//
//	GridWhale Classes and Functions
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.
//
//	The CGridACL defines the access control list for a resource in the grid.

#pragma once

#include "GridWhaleUser.h"

class CProgramRunInfo;

enum class EGridPermission
	{
	None =					0,

	Run =					0x01,
	View =					0x02,

	Contribute =			0x08,
	Modify =				0x10,

	Design =				0x40,
	Admin =					0x80,
	};

class CGridPermissions
	{
	public:

		CGridPermissions () { }
		CGridPermissions (EGridPermission iMeta, EGridPermission iWrite, EGridPermission iRead) :
				m_byBits(Combine(iMeta, iWrite, iRead))
			{ }

		explicit CGridPermissions (BYTE byValue) : m_byBits(byValue) { }
		explicit CGridPermissions (CDatum dValue) : m_byBits((BYTE)(DWORD)dValue) { }

		bool operator== (const CGridPermissions &Src) const = default;
		bool operator!= (const CGridPermissions &Src) const = default;

		void Add (const CGridPermissions &Src) { m_byBits |= Src.m_byBits; }
		CDatum AsDatum () const { return CDatum((DWORD)m_byBits); }
		DWORD AsDWORD () const { return (DWORD)m_byBits; }
		CString AsString () const;
		bool Can (EGridPermission iPermission) const { return (m_byBits & (BYTE)iPermission); }
		bool CanAccess () const { return m_byBits != 0; }
		bool CanAdmin () const { return Can(EGridPermission::Admin); }
		bool CanContribute () const { return Can(EGridPermission::Contribute); }
		bool CanDesign () const { return Can(EGridPermission::Design); }
		bool CanModify () const { return Can(EGridPermission::Modify); }
		bool CanRun () const { return Can(EGridPermission::Run); }
		bool CanView () const { return Can(EGridPermission::View); }
		int Compare (const CGridPermissions& Src) const;

		static CGridPermissions Admin () { return CGridPermissions(FULL_ADMIN_BITS); }
		static CGridPermissions Modify () { return CGridPermissions(FULL_MODIFY_BITS); }
		static CGridPermissions None () { return CGridPermissions(); }
		static CGridPermissions Run () { return CGridPermissions((BYTE)EGridPermission::Run); }
		static CGridPermissions View () { return CGridPermissions(FULL_VIEW_BITS); }

		static EGridPermission Combine (EGridPermission iOriginal, EGridPermission iNew);
		static CString GetName (EGridPermission iPermission);
		static bool Parse (CStringView sValue, CGridPermissions* retPermissions = NULL);
		static EGridPermission ParsePermission (CStringView sValue);

	private:

		static constexpr BYTE READ_MASK =		0x03;
		static constexpr BYTE WRITE_MASK =		0x18;
		static constexpr BYTE META_MASK =		0xC0;

		static constexpr BYTE FULL_ADMIN_BITS =	(BYTE)EGridPermission::Admin | (BYTE)EGridPermission::Design | (BYTE)EGridPermission::Modify | (BYTE)EGridPermission::Contribute | (BYTE)EGridPermission::View | (BYTE)EGridPermission::Run;
		static constexpr BYTE FULL_MODIFY_BITS = (BYTE)EGridPermission::Modify | (BYTE)EGridPermission::Contribute | (BYTE)EGridPermission::View | (BYTE)EGridPermission::Run;
		static constexpr BYTE FULL_VIEW_BITS = (BYTE)EGridPermission::View | (BYTE)EGridPermission::Run;

		static BYTE Combine (EGridPermission iMeta, EGridPermission iWrite, EGridPermission iRead);

		BYTE m_byBits = 0;
	};

class CGridACL
	{
	public:

		CGridACL ();
		explicit CGridACL (CDatum dValue);
		CGridACL (const CGridName& Owner, const CString& sProgramID = NULL_STR, const CGridPermissions& PublicPermissions = CGridPermissions::None(), const CGridPermissions& AllProgramPermissions = CGridPermissions::None());
		CGridACL (const CGridName& Owner, const CString& sProgramID, CDatum dValue);

		static CGridACL FromFileDesc (CDatum dFileDesc);

		bool operator== (const CGridACL& Src) const { return IsEqual(Src); }
		bool operator!= (const CGridACL& Src) const { return !IsEqual(Src); }

		CDatum AsDatum () const;
		CDatum AsGridACLType () const;
		CDatum AsStruct () const;
		bool Can (const CProgramRunInfo& ProgramInfo, EGridPermission iPermission) const;
		bool Can (const CGridUser& User, const CString& sProgramID, EGridPermission iPermission) const;
		bool CanAccess (const CProgramRunInfo& ProgramInfo) const;
		bool CanAccess (const CGridUser& User, const CString& sProgramID) const;
		int Compare (const CGridACL& Src) const;
		static CGridACL Deserialize (IByteStream& Stream);
		CGridPermissions GetAllProgramPermissions () const { return m_AllPrograms; }
		const CGridName& GetOwner () const { return m_Owner; }
		CGridPermissions GetPermissions (const CProgramRunInfo& ProgramInfo) const;
		const CString& GetProgramID () const { return m_sProgramID; }
		CDatum GetProgramPermissionsAsTable () const;
		CGridPermissions GetPublicPermissions () const { return m_Public; }
		CDatum GetUserPermissionsAsTable () const;
		bool InitFromDatum (CDatum dValue);
		void Serialize (IByteStream& Stream) const;
		void SetAllProgramPermissions (const CGridPermissions& Permissions) { m_AllPrograms = Permissions; }
		void SetOwner (const CGridName& Owner) { m_Owner = Owner; }
		void SetPermission (const CGridName& User, const CGridPermissions& Permissions);
		void SetProgramID (const CString& sProgramID) { m_sProgramID = sProgramID; }
		bool SetProgramPermission (CStringView sProgramID, const CGridPermissions& Permissions);
		bool SetProgramPermissions (CDatum dValue, CString* retsError = NULL);
		void SetPublicPermissions (const CGridPermissions& Permissions) { m_Public = Permissions; }
		void SetPublicPermissionsFromShare (CDatum dShare);
		bool SetUserPermission (const CGridName& User, const CGridPermissions& Permissions);
		bool SetUserPermissions (CDatum dValue, CString* retsError = NULL);

		static CGridACL Private (const CGridName& Owner, const CString& sProgramID = NULL_STR);
		static CGridACL PublicModify (const CGridName& Owner, const CString& sProgramID = NULL_STR);
		static CGridACL PublicView (const CGridName& Owner, const CString& sProgramID = NULL_STR);

	private:

		static CGridPermissions AllProgramsFromBase (DWORD dwValue);
		DWORD BaseAsDWORD () const;
		bool IsEqual (const CGridACL& Src) const;
		bool ProgramHasAccess (const CString& sProgramID) const;
		bool ProgramHasPermission (const CString& sProgramID, EGridPermission iPermission) const;
		static CGridPermissions PublicFromBase (DWORD dwValue);
		static TSortMap<CString, CGridPermissions> ReadProgramPermissions (CDatum dValue);
		static TSortMap<CGridName, CGridPermissions> ReadUserPermissions (CDatum dValue);
		bool UserHasAccess (const CGridUser& User) const;
		bool UserHasPermission (const CGridUser& User, EGridPermission iPermission) const;

		CGridName m_Owner;						//	Owner of resource
		CString m_sProgramID;					//	Owning program ID ("ABCD1234"). If blank, all programs have access.
		CGridPermissions m_Public;				//	Permissions for public
		CGridPermissions m_AllPrograms;			//	Permissions for all programs (other than m_sProgramID)
		TSortMap<CGridName, CGridPermissions> m_UserPermissions;	//	Permissions for specific users/groups
		TSortMap<CString, CGridPermissions> m_ProgramPermissions;	//	Permissions for specific programs
	};
