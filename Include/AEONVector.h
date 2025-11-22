//	AEONVector.h
//
//	AEON header file
//	Copyright (c) 2014 by GridWhale Corporation. All Rights Reserved.
////	USAGE
//
//	Automatically included by AEON.h

#pragma once

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

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override { }
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override { }

		CPolygon2D m_Polygon;
	};
