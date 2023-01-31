//	CLuminousCanvasResources.cpp
//
//	CLuminousCanvasResources Class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"
#include "LuminousHTMLCanvas.h"

void CLuminousCanvasResources::AddNamedResource (const CString& sName, CDatum dResource, SequenceNumber Seq)

//	AddNamedResource
//
//	Adds a named resource.

	{
	auto pEntry = m_NamedResources.SetAt(sName);
	pEntry->dResource = dResource;
	pEntry->Seq = Seq;
	}

void CLuminousCanvasResources::AddResource (const ILuminousGraphic& Graphic, CDatum dResource)

//	AddResource
//
//	Adds the given resource.

	{
	auto pEntry = m_Table.SetAt(Graphic.GetID());
	*pEntry = dResource;
	}

int CLuminousCanvasResources::FindNamedResource (const CString& sName) const

//	FindNamedResource
//
//	Looks for the named resource. Returns -1 if not found.

	{
	int iPos;
	if (!m_NamedResources.FindPos(sName, &iPos))
		return -1;

	return iPos;
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

	for (int i = 0; i < m_NamedResources.GetCount(); i++)
		m_NamedResources[i].dResource.Mark();
	}

void CLuminousCanvasResources::SetSeq (SequenceNumber Seq)

//	SetSeq
//
//	Sets sequence number.

	{
	for (int i = 0; i < m_NamedResources.GetCount(); i++)
		m_NamedResources[i].Seq = Seq;
	}

CLuminousCanvasResources CLuminousCanvasResources::Read (IByteStream& Stream)

//	Read
//
//	Read from a stream.

	{
	//	LATER...
	return CLuminousCanvasResources();
	}

void CLuminousCanvasResources::Write (IByteStream& Stream) const

//	Write
//
//	Write to stream.

	{
	//	LATER...
	}
