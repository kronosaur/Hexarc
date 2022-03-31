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

class CAEONTimeSpan : public IComplexDatum
	{
	public:
		CAEONTimeSpan (void) { }
		CAEONTimeSpan (const CTimeSpan &TimeSpan) : m_TimeSpan(TimeSpan) { }

		virtual CString AsString (void) const override;
		virtual size_t CalcMemorySize (void) const override { return sizeof(CAEONTimeSpan); }
		virtual const CTimeSpan &CastCTimeSpan () const { return m_TimeSpan; }
		virtual IComplexDatum *Clone (void) const override { return new CAEONTimeSpan(m_TimeSpan); }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeTimeSpan; }
		virtual int GetCount (void) const { return PART_COUNT; };
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::TIME_SPAN); }
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual const CString &GetTypename (void) const override;
		virtual bool IsArray (void) const override { return true; }

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

class CAEONTable : public IComplexDatum, public IAEONTable
	{
	public:
		CAEONTable (CDatum dSchema) { SetSchema(dSchema); }

		//	IAEONTable

		virtual EResult AppendColumn (CDatum dColumn) override;
		virtual EResult AppendRow (CDatum dRow) override;
		virtual EResult AppendSlice (CDatum dSlice) override;
		virtual EResult AppendTable (CDatum dTable) override;
		virtual EResult DeleteAllRows () override;
		virtual bool FindCol (const CString &sName, int *retiCol = NULL) const override;
		virtual CDatum GetCol (int iIndex) const override { return ((iIndex >= 0 && iIndex < m_Cols.GetCount()) ? m_Cols[iIndex] : CDatum()); }
		virtual int GetColCount () const override;
		virtual CString GetColName (int iCol) const override;
		virtual CDatum GetDataSlice (int iFirstRow, int iRowCount) const override;
		virtual CDatum GetFieldValue (int iRow, int iCol) const override;
		virtual int GetRowCount () const override;
		virtual bool IsSameSchema (CDatum dSchema) const override;

		//	IComplexDatum

		virtual void Append (CDatum dDatum) override;
		virtual CString AsString (void) const override;
		virtual size_t CalcMemorySize (void) const override;
		virtual IComplexDatum *Clone (void) const override { return new CAEONTable(*this); }
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const override { throw CException(errFail); }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeTable; }
		virtual int GetCount (void) const override { return m_iRows; }
		virtual CDatum GetDatatype () const override { return m_dSchema; }
//		virtual CDatum GetElement (CDatum dIndex) const override;
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual IAEONTable *GetTableInterface () { return this; }
		virtual const CString &GetTypename (void) const override;
		virtual void GrowToFit (int iCount) override;
		virtual bool IsArray (void) const override { return true; }
		virtual bool IsNil (void) const override { return (GetCount() == 0); }
		virtual void ResolveDatatypes (const CAEONTypeSystem &TypeSystem) override;
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { throw CException(errFail); }
//		virtual void SetElement (CDatum dIndex, CDatum dDatum) override;
		virtual void SetElement (int iIndex, CDatum dDatum) override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

		static bool CreateTableFromArray (CAEONTypeSystem &TypeSystem, CDatum dValue, CDatum &retdDatum);
		static bool CreateTableFromDatatype (CAEONTypeSystem &TypeSystem, CDatum dType, CDatum &retdDatum);
		static bool CreateTableFromStruct (CAEONTypeSystem &TypeSystem, CDatum dValue, CDatum &retdDatum);

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		CAEONTable () { }

		static CDatum CalcColumnDatatype (CDatum dValue);
		void SetSchema (CDatum dSchema);

		int m_iRows = 0;
		TArray<CDatum> m_Cols;

		//	NOTE: The constructor guarantees that we have a valid schema.
		//	We can always rely on m_dSchema being of type ECategory::Schema.

		CDatum m_dSchema;

		static TDatumPropertyHandler<CAEONTable> m_Properties;

		friend class CAEONScriptParser;
	};
