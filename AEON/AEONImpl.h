//	AEONImpl.h
//
//	AEON Class Implementations
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CAEONObject : public CComplexStruct
	{
	public:
		CAEONObject () { }
		CAEONObject (CDatumTypeID TypeID) : m_TypeID(TypeID) { }
		CAEONObject (CDatumTypeID TypeID, CDatum dSrc) : CComplexStruct(dSrc), m_TypeID(TypeID) { }
		CAEONObject (CDatumTypeID TypeID, const TSortMap<CString, CDatum> &Src) : CComplexStruct(Src), m_TypeID(TypeID) { }
		CAEONObject (CDatumTypeID TypeID, const TSortMap<CString, CString> &Src) : CComplexStruct(Src), m_TypeID(TypeID) { }

		//	IComplexDatum
		virtual IComplexDatum *Clone (void) const override { return new CAEONObject(m_TypeID, m_Map); }
		virtual CDatum::Types GetBasicType (void) const { return CDatum::typeObject; }

	private:
		CDatumTypeID m_TypeID;
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
