//	CHexeTypeSystem.cpp
//
//	CHexeTypeSystem class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

void CHexeTypeSystem::AddType (TUniquePtr<IHexeType> &&NewType)

//	AddType
//
//	Adds a new type. We will take ownership of the pointer.

	{
	DWORD dwID;
	auto pSlot = m_Types.Insert(&dwID);
	*pSlot = std::move(NewType);

	(*pSlot)->SetTypeIDIndex(dwID);
	}
