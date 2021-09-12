//	HexeTypes.h
//
//	Hexe header file
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Hexe.h

#pragma once

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
		
	private:
		TIDTable<TUniquePtr<IHexeType>> m_Types;
	};
