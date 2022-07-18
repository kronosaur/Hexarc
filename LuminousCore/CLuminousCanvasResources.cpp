//	CLuminousCanvasResources.cpp
//
//	CLuminousCanvasResources Class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"
#include "LuminousHTMLCanvas.h"

void CLuminousCanvasResources::AddResource (const ILuminousGraphic& Graphic, CDatum dResource)

//	AddResource
//
//	Adds the given resource.

	{
	auto pEntry = m_Table.SetAt(Graphic.GetID());
	*pEntry = dResource;
	}

CDatum CLuminousCanvasResources::GetResource (const ILuminousGraphic& Graphic) const

//	GetResource
//
//	Returns the resource.

	{
	auto pEntry = m_Table.GetAt(Graphic.GetID());
	if (!pEntry)
		return CDatum();

	return *pEntry;
	}

void CLuminousCanvasResources::Mark ()

//	Mark
//
//	Marks data in use.

	{
	for (int i = 0; i < m_Table.GetCount(); i++)
		m_Table[i].Mark();
	}
