//	GraphicsImpl.h
//
//	ILuminousGraphics Implementations
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "LuminousCore.h"

class CClearRectGraphic : public ILuminousGraphic
	{
	public:

		CClearRectGraphic (DWORD dwID, const CString& sName, const CVector2D& vUL, const CVector2D& vLR) : ILuminousGraphic(dwID, sName)
			{
			m_Path.Rect(vUL, vLR);
			}

	private:

		virtual const CLuminousPath2D& OnGetShapePath () const override { return m_Path; }
		virtual EType OnGetType () const { return EType::ClearRect; }

		CLuminousPath2D m_Path;
	};

class CImageGraphic : public ILuminousGraphic
	{
	public:

		CImageGraphic (DWORD dwID, const CString& sName, const CRGBA32Image& ImageRef) : ILuminousGraphic(dwID, sName),
				m_Image(ImageRef)
			{ }

	private:

		virtual const CRGBA32Image& OnGetImage () const override { return m_Image; }
		virtual EType OnGetType () const { return EType::Image; }

		const CRGBA32Image& m_Image;
	};

class CMetaCommandGraphic : public ILuminousGraphic
	{
	public:

		CMetaCommandGraphic (DWORD dwID, const CString& sName, EType iType) : ILuminousGraphic(dwID, sName),
				m_iType(iType)
			{ }

	private:

		virtual EType OnGetType () const { return m_iType; }

		EType m_iType = EType::None;
	};

class CResourceGraphic : public ILuminousGraphic
	{
	public:

		CResourceGraphic (DWORD dwID, const CString& sName, const CString& sResourceID) : ILuminousGraphic(dwID, sName),
				m_sResourceID(sResourceID)
			{ }

	private:

		virtual const CString& OnGetResource () const { return m_sResourceID; }
		virtual EType OnGetType () const { return EType::Resource; }

		CString m_sResourceID;
	};

class CShapeGraphic : public ILuminousGraphic
	{
	public:

		CShapeGraphic (DWORD dwID, const CString& sName) : ILuminousGraphic(dwID, sName)
			{ }

		void SetFillStyle (const CLuminousFillStyle& Style) { m_FillStyle = Style; }
		void SetLineStyle (const CLuminousLineStyle& Style) { m_LineStyle = Style; }
		void SetPath (const CLuminousPath2D& Path) { m_Path = Path; }
		void SetShadowStyle (const CLuminousShadowStyle& Style) { m_ShadowStyle = Style; }

	private:

		virtual const CLuminousFillStyle& OnGetFillStyle () const override { return m_FillStyle; }
		virtual const CLuminousLineStyle& OnGetLineStyle () const override { return m_LineStyle; }
		virtual const CLuminousShadowStyle& OnGetShadowStyle () const override { return m_ShadowStyle; }
		virtual const CLuminousPath2D& OnGetShapePath () const override { return m_Path; }
		virtual EType OnGetType () const { return EType::Shape; }

		CLuminousPath2D m_Path;
		CLuminousFillStyle m_FillStyle;
		CLuminousLineStyle m_LineStyle;
		CLuminousShadowStyle m_ShadowStyle;
	};
