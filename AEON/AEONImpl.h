//	AEONImpl.h
//
//	AEON Class Implementations
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CAEONObject : public CComplexStruct
	{
	public:
		CAEONObject () { }
		CAEONObject (CDatum dType) : m_dType(dType) { }
		CAEONObject (CDatum dType, CDatum dSrc) : CComplexStruct(dSrc), m_dType(dType) { }
		CAEONObject (CDatum dType, const TSortMap<CString, CDatum> &Src) : CComplexStruct(Src), m_dType(dType) { }
		CAEONObject (CDatum dType, const TSortMap<CString, CString> &Src) : CComplexStruct(Src), m_dType(dType) { }

		//	IComplexDatum

		virtual IComplexDatum *Clone (void) const override { return new CAEONObject(m_dType, m_Map); }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeObject; }
		virtual CDatum GetDatatype () const override { return m_dType; }

	private:
		CDatum m_dType;
	};

#if 0
//	LATER: Turn into a template

class CAEONVector : public CComplexArray
	{
	public:
		CAEONVector () { }
		CAEONVector (CDatumTypeID TypeID) : m_TypeID(TypeID) { }
		CAEONVector (CDatumTypeID TypeID, CDatum dSrc) : CComplexArray(dSrc), m_TypeID(TypeID) { }
		CAEONVector (CDatumTypeID TypeID, const TArray<CString> &Src) : CComplexArray(Src), m_TypeID(TypeID) { }
		CAEONVector (CDatumTypeID TypeID, const TArray<CDatum> &Src) : CComplexArray(Src), m_TypeID(TypeID) { }

		//	IComplexDatum
		virtual IComplexDatum *Clone (void) const override { return new CAEONVector(m_TypeID, m_Array); }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeVector; }

	private:
		CDatumTypeID m_TypeID;				//	Type of array elements
	};
#endif
