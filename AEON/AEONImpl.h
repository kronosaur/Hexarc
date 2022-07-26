//	AEONImpl.h
//
//	AEON Class Implementations
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "TAEONVector.h"

class CComplexDateTime : public IComplexDatum
	{
	public:
		CComplexDateTime () { }
		CComplexDateTime (const CDateTime DateTime) : m_DateTime(DateTime) { }
		
		static bool CreateFromString (const CString &sString, CDateTime *retDateTime);
		static bool CreateFromString (const CString &sString, CDatum *retdDatum);

		virtual CString AsString () const override;
		virtual size_t CalcMemorySize () const override { return sizeof(CComplexDateTime); }
		virtual const CDateTime &CastCDateTime () const override { return m_DateTime; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeDateTime; }
		virtual int GetCount () const override { return partCount; }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::DATE_TIME); }
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual const CString &GetTypename () const override;
		virtual bool IsArray () const override { return true; }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;

	private:
		enum EParts
			{
			partYear = 0,
			partMonth = 1,
			partDay = 2,
			partHour = 3,
			partMinute = 4,
			partSecond = 5,
			partMillisecond = 6,

			partCount = 7,
			};

		CDateTime m_DateTime;
	};

class CComplexImage32 : public IComplexDatum
	{
	public:
		CComplexImage32 () { }
		CComplexImage32 (const CRGBA32Image &Src) :
				m_Image(Src)
			{ }

		CComplexImage32 (CRGBA32Image &&Src) :
				m_Image(std::move(Src))
			{ }

		//	IComplexDatum
		virtual CString AsString () const override { return strPattern("Image %dx%d", m_Image.GetWidth(), m_Image.GetHeight()); }
		virtual size_t CalcMemorySize () const override { return (size_t)m_Image.GetWidth() * (size_t)m_Image.GetHeight() * sizeof(DWORD); }
		virtual const CRGBA32Image &CastCRGBA32Image () const override { return m_Image; }
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeImage32; }
		virtual int GetCount () const override { return 1; }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual CRGBA32Image *GetImageInterface () override { return &m_Image; }
		virtual const CString &GetTypename () const override;
		virtual bool IsArray () const override { return false; }
		virtual bool IsNil () const override { return m_Image.IsEmpty(); }

	protected:
		//	IComplexDatum
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:
		CRGBA32Image m_Image;
	};

class CComplexInteger : public IComplexDatum
	{
	public:
		CComplexInteger (void) { }
		CComplexInteger (DWORDLONG ilValue) : m_Value(ilValue) { }
		CComplexInteger (const CIPInteger &Value) : m_Value(Value) { }

		void TakeHandoff (CIPInteger &Value) { m_Value.TakeHandoff(Value); }

		//	IComplexDatum
		virtual int AsArrayIndex () const override;
		virtual CString AsString (void) const override { return m_Value.AsString(); }
		virtual size_t CalcMemorySize (void) const override { return m_Value.GetSize(); }
		virtual const CIPInteger &CastCIPInteger (void) const override { return m_Value; }
		virtual double CastDouble () const override { return m_Value.AsDouble(); }
		virtual DWORDLONG CastDWORDLONG (void) const override;
		virtual int CastInteger32 (void) const override;
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeIntegerIP; }
		virtual int GetCount (void) const override { return 1; }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::INT_IP); }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual CDatum::Types GetNumberType (int *retiValue) override;
		virtual const CString &GetTypename (void) const override;
		virtual bool IsArray (void) const override { return false; }
		virtual bool IsIPInteger (void) const override { return true; }
		virtual CDatum MathAbs () const override;
		virtual CDatum MathAverage () const override { return CDatum::raw_AsComplex(this); }
		virtual CDatum MathMax () const override { return CDatum::raw_AsComplex(this); }
		virtual CDatum MathMedian () const override { return CDatum::raw_AsComplex(this); }
		virtual CDatum MathMin () const override { return CDatum::raw_AsComplex(this); }
		virtual CDatum MathSum () const override { return CDatum::raw_AsComplex(this); }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	protected:
		//	IComplexDatum
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override { return CIPInteger::Deserialize(Stream, &m_Value); }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override { m_Value.Serialize(Stream); }

	private:
		CIPInteger m_Value;
	};

class CAEONObject : public CComplexStruct
	{
	public:
		CAEONObject () { }
		CAEONObject (CDatum dType) : m_dType(dType) { }
		CAEONObject (CDatum dType, CDatum dSrc) : CComplexStruct(dSrc), m_dType(dType) { }
		CAEONObject (CDatum dType, const TSortMap<CString, CDatum> &Src) : CComplexStruct(Src), m_dType(dType) { }
		CAEONObject (CDatum dType, const TSortMap<CString, CString> &Src) : CComplexStruct(Src), m_dType(dType) { }

		//	IComplexDatum

		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override { return new CAEONObject(m_dType, m_Map); }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeObject; }
		virtual CDatum GetDatatype () const override { return m_dType; }

	private:
		CDatum m_dType;
	};

class CAEONTimeSpan : public IComplexDatum
	{
	public:
		CAEONTimeSpan (void) { }
		CAEONTimeSpan (const CTimeSpan &TimeSpan) : m_TimeSpan(TimeSpan) { }

		virtual CString AsString (void) const override;
		virtual size_t CalcMemorySize (void) const override { return sizeof(CAEONTimeSpan); }
		virtual const CTimeSpan &CastCTimeSpan () const { return m_TimeSpan; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeTimeSpan; }
		virtual int GetCount (void) const { return PART_COUNT; };
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::TIME_SPAN); }
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual CDatum::Types GetNumberType (int *retiValue) { return CDatum::typeTimeSpan; }
		virtual const CString &GetTypename (void) const override;
		virtual bool IsArray (void) const override { return true; }
		virtual CDatum MathAbs () const override;

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:
		static constexpr int PART_COUNT = 3;
		static constexpr int PART_DAYS = 0;
		static constexpr int PART_MILLISECONDS = 1;
		static constexpr int PART_NEGATIVE = 2;

		CTimeSpan m_TimeSpan;
	};

class CAEONVectorInt32 : public TAEONVector<int, CAEONVectorInt32>
	{
	public:
		CAEONVectorInt32 () { }
		CAEONVectorInt32 (const TArray<int> &Src) : TAEONVector<int, CAEONVectorInt32>(Src) { }
		CAEONVectorInt32 (const TArray<CDatum> &Src) : TAEONVector<int, CAEONVectorInt32>(Src) { }

		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override { return new CAEONVectorInt32(m_Array); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY_INT_32); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual const CString &GetTypename (void) const override;
		virtual CDatum MathAbs () const override;
		virtual CDatum MathAverage () const override;
		virtual CDatum MathMax () const override;
		virtual CDatum MathMin () const override;
		virtual CDatum MathSum () const override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

	private:

		static TDatumPropertyHandler<CAEONVectorInt32> m_Properties;
	};

class CAEONVectorIntIP : public TAEONVector<CIPInteger, CAEONVectorIntIP>
	{
	public:
		CAEONVectorIntIP () { }
		CAEONVectorIntIP (const TArray<CIPInteger> &Src) : TAEONVector<CIPInteger, CAEONVectorIntIP>(Src) { }
		CAEONVectorIntIP (const TArray<CDatum> &Src) : TAEONVector<CIPInteger, CAEONVectorIntIP>(Src) { }

		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override { return new CAEONVectorIntIP(m_Array); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY_INT_IP); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual const CString &GetTypename (void) const override;
		virtual CDatum MathAbs () const override;
		virtual CDatum MathAverage () const override;
		virtual CDatum MathMax () const override;
		virtual CDatum MathMin () const override;
		virtual CDatum MathSum () const override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

	private:

		static TDatumPropertyHandler<CAEONVectorIntIP> m_Properties;
	};

class CAEONVectorFloat64 : public TAEONVector<double, CAEONVectorFloat64>
	{
	public:
		CAEONVectorFloat64 () { }
		CAEONVectorFloat64 (const TArray<double> &Src) : TAEONVector<double, CAEONVectorFloat64>(Src) { }
		CAEONVectorFloat64 (const TArray<CDatum> &Src) : TAEONVector<double, CAEONVectorFloat64>(Src) { }

		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override { return new CAEONVectorFloat64(m_Array); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY_FLOAT_64); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual const CString &GetTypename (void) const override;
		virtual CDatum MathAbs () const override;
		virtual CDatum MathAverage () const override;
		virtual CDatum MathMax () const override;
		virtual CDatum MathMin () const override;
		virtual CDatum MathSum () const override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

	private:

		static TDatumPropertyHandler<CAEONVectorFloat64> m_Properties;
	};

class CAEONVectorString : public TAEONVector<CString, CAEONVectorString>
	{
	public:
		CAEONVectorString () { }
		CAEONVectorString (const TArray<CString> &Src) : TAEONVector<CString, CAEONVectorString>(Src) { }
		CAEONVectorString (const TArray<CDatum> &Src) : TAEONVector<CString, CAEONVectorString>(Src) { }

		static CString FromDatum (CDatum dValue) { return dValue.AsString(); }
		static CDatum ToDatum (CString Value) { return CDatum(Value); }

		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override { return new CAEONVectorString(m_Array); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY_STRING); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual const CString &GetTypename (void) const override;
		virtual CDatum MathAbs () const override;
		virtual CDatum MathAverage () const override;
		virtual CDatum MathMax () const override;
		virtual CDatum MathMin () const override;
		virtual CDatum MathSum () const override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

	private:

		static TDatumPropertyHandler<CAEONVectorString> m_Properties;
	};

class CAEONVector2D : public TExternalDatum<CAEONVector2D>
	{
	public:
		CAEONVector2D () { }
		CAEONVector2D (const CVector2D &vVector);

		const CVector2D &GetVector (void) const { return m_vVector; }

		static const CString &StaticGetTypename (void);

		//	IComplexDatum

		virtual const CVector2D& CastCVector2D () const override { return m_vVector; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeVector2D; }
		virtual int GetCount (void) const override { return 2; }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::VECTOR_2D_F64); }
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

