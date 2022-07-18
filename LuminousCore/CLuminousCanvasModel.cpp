//	CLuminousCanvasModel.cpp
//
//	CLuminousCanvasModel Class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

TUniquePtr<ILuminousGraphic> CLuminousCanvasModel::CreateBeginUpdate (const CString& sName)

//	CreateBeginUpdate
//
//	Creates a begin update entry.

	{
	return TUniquePtr<ILuminousGraphic>(new CMetaCommandGraphic(0, sName, ILuminousGraphic::EType::BeginUpdate));
	}

TUniquePtr<ILuminousGraphic> CLuminousCanvasModel::CreateClearRect (const CVector2D& vUL, const CVector2D& vLR, const CString& sName)

//	CreateClearRect
//
//	Creates a clear rect command.

	{
	return TUniquePtr<ILuminousGraphic>(new CClearRectGraphic(0, sName, vUL, vLR));
	}

TUniquePtr<ILuminousGraphic> CLuminousCanvasModel::CreateImage (const CRGBA32Image& ImageRef, const CString& sName)

//	CreateImage
//
//	Creates an image shape. Callers must guarantee that the image reference is
//	valid for the lifetime of the canvas model.

	{
	return TUniquePtr<ILuminousGraphic>(new CImageGraphic(0, sName, ImageRef));
	}

TUniquePtr<ILuminousGraphic> CLuminousCanvasModel::CreateShape (const SShapeOptions& Options, const CString& sName)

//	CreateShape
//
//	Creates a shape based on the current draw context.

	{
	auto* pNewGraphic = new CShapeGraphic(0, sName);

	pNewGraphic->SetPath(Options.Path);
	pNewGraphic->SetFillStyle(Options.FillStyle);
	pNewGraphic->SetLineStyle(Options.LineStyle);
	pNewGraphic->SetShadowStyle(Options.ShadowStyle);

	return TUniquePtr<ILuminousGraphic>(pNewGraphic);
	}

void CLuminousCanvasModel::DeleteAll ()

//	DeleteAll
//
//	Delete all shapes and reset to defaults.

	{
	m_Graphics.DeleteAll();
	m_ZOrder.DeleteAll();
	m_ByName.DeleteAll();
	}

const ILuminousGraphic* CLuminousCanvasModel::FindGraphicByID (DWORD dwID) const

//	FindGraphicByID
//
//	Returns a graphic (or NULL).

	{
	auto ppEntry = m_Graphics.GetAt(dwID);
	if (!ppEntry || !(*ppEntry))
		return NULL;

	return (*ppEntry);
	}

ILuminousGraphic& CLuminousCanvasModel::GetGraphicByID (DWORD dwID)

//	GetGraphicByID
//
//	Returns a reference to the given graphic.

	{
	auto ppEntry = m_Graphics.GetAt(dwID);
	if (!ppEntry || !(*ppEntry))
		throw CException(errFail);

	return *(*ppEntry);
	}

ILuminousGraphic& CLuminousCanvasModel::InsertGraphic (TUniquePtr<ILuminousGraphic>&& pGraphic)

//	InsertGraphic
//
//	Adds the graphic to the model.

	{
	//	Take ownership. pGraphic will be NULL after this, so we keep a 
	//	reference.

	ILuminousGraphic &Ref = *pGraphic;
	Ref.SetID(m_dwNextID++);
	m_Graphics.SetAt(Ref.GetID(), std::move(pGraphic));

	//	Insert in order

	m_ZOrder.Insert(&Ref);

	//	Insert by name

	if (!Ref.GetName().IsEmpty())
		m_ByName.SetAt(Ref.GetName(), &Ref);

	return Ref;
	}

void CLuminousCanvasModel::SetSeq (SequenceNumber Seq)

//	SetSeq
//
//	Sets the sequence number for all graphics.

	{
	for (int i = 0; i < m_Graphics.GetCount(); i++)
		m_Graphics[i]->SetSeq(Seq);
	}
