//	HexeTypes.h
//
//	Hexe header file
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Hexe.h

#pragma once

enum class EHexeMemberType
	{
	None,

	InstanceMethod,
	InstanceVar,
	StaticMethod,
	StaticVar,
	};

class IHexeType
	{
	public:
		IHexeType (const CString &sName = NULL_STR, const CString &sFullyQualifiedName = NULL_STR) :
				m_sName(sName),
				m_sFullyQualifiedName(sFullyQualifiedName.IsEmpty() ? sName : sFullyQualifiedName)
			{ }

		IHexeType (const IHexeType &Src) = delete;
		IHexeType (IHexeType &&Src) = delete;

		virtual ~IHexeType () { }

		IHexeType &operator= (const IHexeType &Src) = delete;
		IHexeType &operator= (IHexeType &&Src) = delete;

		virtual bool AddMember (const CString &sName, EHexeMemberType iType, IHexeType &Type, CString *retsError = NULL) { throw CException(errFail); }
		virtual EHexeMemberType HasMember (const CString &sName) const { return EHexeMemberType::None; }
		virtual bool IsA (const IHexeType &Type) const { return (&Type == this) || Type.IsAny(); }
		virtual bool IsAbstract () const { return false; }
		virtual bool IsAny () const { return false; }

		const CString &GetFullyQualifiedName () const { return m_sFullyQualifiedName; }
		const CString &GetName () const { return m_sName; }
		DWORD GetTypeIDIndex () const { return m_dwTypeIDIndex; }
		void SetTypeIDIndex (DWORD dwID) { m_dwTypeIDIndex = dwID; }

	protected:
		CString m_sName;
		CString m_sFullyQualifiedName;
		DWORD m_dwTypeIDIndex = 0;
	};

class CHexeTypeSystem
	{
	public:
		void AddType (TUniquePtr<IHexeType> &&NewType);
		void DeleteAll () { m_Types.DeleteAll(); }
		const IHexeType *FindType (DWORD dwIndex) const { if (m_Types.IsValid(dwIndex)) return m_Types.GetAt(dwIndex); else return NULL; }
		
	private:
		TIDTable<TUniquePtr<IHexeType>> m_Types;
	};
