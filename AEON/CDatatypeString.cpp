//	CDatatypeString.cpp
//
//	CDatatypeString class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

bool CDatatypeString::OnEquals (const IDatatype &Src) const
	{
	auto &Other = (const CDatatypeSimple &)Src;

	if (GetCoreType() != Other.GetCoreType())
		return false;

	return true;
	}

int CDatatypeString::OnFindMember (CStringView sName) const

//	OnFindMember
//
//	Returns the index of the given member (by ID) or returns -1.

	{
	int iIndex = CAEONStringImpl::FindPropertyByKey(sName);
	if (iIndex != -1)
		return 1 + iIndex;

	iIndex = CAEONStringImpl::FindMethodByKey(sName);
	if (iIndex != -1)
		return 1 + iIndex + CAEONStringImpl::GetPropertyCount();

	return -1;
	}

IDatatype::SMemberDesc CDatatypeString::OnGetMember (int iIndex) const

//	OnGetMember
//
//	Get the given member.

	{
	if (iIndex == 0)
		//	LATER: Element type should be Char (but we don't have that datatype yet).
		return SMemberDesc({ EMemberType::ArrayElement, NULL_STR, CAEONTypeSystem::GetCoreType(IDatatype::STRING) });
	else
		{
		iIndex--;

		if (iIndex < CAEONStringImpl::GetPropertyCount())
			return SMemberDesc({ EMemberType::InstanceProperty, CAEONStringImpl::GetPropertyKey(iIndex), CAEONStringImpl::GetPropertyType(iIndex) });
		else
			{
			iIndex -= CAEONStringImpl::GetPropertyCount();

			if (iIndex < CAEONStringImpl::GetMethodCount())
				return SMemberDesc({ EMemberType::InstanceMethod, CAEONStringImpl::GetMethodKey(iIndex), CAEONStringImpl::GetMethodType(iIndex) });
			else
				throw CException(errFail);
			}
		}
	}

int CDatatypeString::OnGetMemberCount () const

//	OnGetMemberCount
//
//	Returns the total number of members.

	{
	//	The first entry is the element type. Next we return the set of built-in
	//	properties. And last is the set of built-in methods.

	return 1 + CAEONStringImpl::GetPropertyCount() + CAEONStringImpl::GetMethodCount();
	}

IDatatype::EMemberType CDatatypeString::OnHasMember (CStringView sName, CDatum* retdType, int* retiOrdinal) const

//	OnHasMember
//
//	Looks for the given member and returns the type.

	{
	int iIndex = FindMember(sName);
	if (iIndex == -1)
		return EMemberType::None;

	SMemberDesc Member = GetMember(iIndex);
	if (retdType)
		*retdType = Member.dType;

	if (retiOrdinal)
		*retiOrdinal = Member.iOrdinal;

	return Member.iType;
	}

bool CDatatypeString::OnIsA (const IDatatype &Type) const

//	OnIsA
//
//	Returns TRUE if we are the given type or a subtype.
	
	{
	//	We can be indexed (but not mutated)

	if (Type.GetCoreType() == IDatatype::INDEXED)
		return true;

	//	Not it.

	else
		return false;
	}
