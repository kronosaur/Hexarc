//	AEONImpl.h
//
//	AEON Class Implementations
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "TAEONVector.h"

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

class CAEONVectorInt32 : public TAEONVector<int, CAEONVectorInt32>
	{
	public:
		CAEONVectorInt32 () { }
		CAEONVectorInt32 (const TArray<int> &Src) : TAEONVector<int, CAEONVectorInt32>(Src) { }
		CAEONVectorInt32 (const TArray<CDatum> &Src) : TAEONVector<int, CAEONVectorInt32>(Src) { }

		virtual IComplexDatum *Clone (void) const override { return new CAEONVectorInt32(m_Array); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY_INT_32); }
		virtual const CString &GetTypename (void) const override;
	};

class CAEONVectorFloat64 : public TAEONVector<double, CAEONVectorFloat64>
	{
	public:
		CAEONVectorFloat64 () { }
		CAEONVectorFloat64 (const TArray<double> &Src) : TAEONVector<double, CAEONVectorFloat64>(Src) { }
		CAEONVectorFloat64 (const TArray<CDatum> &Src) : TAEONVector<double, CAEONVectorFloat64>(Src) { }

		virtual IComplexDatum *Clone (void) const override { return new CAEONVectorFloat64(m_Array); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY_FLOAT_64); }
		virtual const CString &GetTypename (void) const override;
	};

class CAEONVectorString : public TAEONVector<CString, CAEONVectorString>
	{
	public:
		CAEONVectorString () { }
		CAEONVectorString (const TArray<CString> &Src) : TAEONVector<CString, CAEONVectorString>(Src) { }
		CAEONVectorString (const TArray<CDatum> &Src) : TAEONVector<CString, CAEONVectorString>(Src) { }

		static CString FromDatum (CDatum dValue) { return dValue.AsString(); }
		static CDatum ToDatum (CString Value) { return CDatum(Value); }

		virtual IComplexDatum *Clone (void) const override { return new CAEONVectorString(m_Array); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY_STRING); }
		virtual const CString &GetTypename (void) const override;
	};
