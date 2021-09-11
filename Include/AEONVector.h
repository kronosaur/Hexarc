//	AEONVector.h
//
//	AEON header file
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.
////	USAGE
//
//	Automatically included by AEON.h

#pragma once

class CAEONVector2D : public TExternalDatum<CAEONVector2D>
	{
	public:
		CAEONVector2D (const CVector2D &vVector);

		inline const CVector2D &GetVector (void) const { return m_vVector; }

		static const CString &StaticGetTypename (void);

		//	IComplexDatum
		virtual int GetCount (void) const override { return 2; }
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual bool IsArray (void) const override { return true; }
		virtual void SetElement (int iIndex, CDatum dDatum) override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override;

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:
		CVector2D m_vVector;
	};

class CAEONPolygon2D : public TExternalDatum<CAEONPolygon2D>
	{
	public:
		CAEONPolygon2D (void);
		CAEONPolygon2D (const CPolygon2D &Poly);
		static CDatum CreateFromHandoff (CPolygon2D &Poly);

		inline const CPolygon2D &GetPolygon (void) const { return m_Polygon; }

		static const CString &StaticGetTypename (void);

		//	IComplexDatum
		virtual int GetCount (void) const override { return 2; }
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual void SetElement (int iIndex, CDatum dDatum) override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override;

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:
		CDatum HolesAsDatum (void) const;
		CDatum OutlineAsDatum (void) const;

		CPolygon2D m_Polygon;
	};
