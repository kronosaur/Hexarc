//	AEONImpl.h
//
//	AEON Class Implementations
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "TAEONVector.h"

class CComplexBinary : public IComplexDatum
	{
	public:

		CComplexBinary () : m_pData(NULL) { }
		CComplexBinary (IByteStream &Stream, int iLength);
		explicit CComplexBinary (int iLength);
		explicit CComplexBinary (CString&& sData);
		explicit CComplexBinary (const CBuffer64& Buffer);
		~CComplexBinary ();

		int GetLength () const { return (m_pData ? ((CString *)&m_pData)->GetLength() : 0); }
		CString ReadAt (int iOffset, int iLength) const;
		void TakeHandoff (CStringBuffer &Buffer);
		int WriteAt (int iOffset, const BYTE* pData, int iLength);

		//	IComplexDatum
		virtual void Append (CDatum dDatum) override;
		virtual CString AsString () const override;
		virtual size_t CalcMemorySize () const override { return CastCString().GetLength() + sizeof(DWORD) + 1; }
		virtual CStringView CastCString () const override;
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual CString Format (const CStringFormat& Format) const override;
		virtual DWORD GetBasicDatatype () const override { return IDatatype::BINARY; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeBinary; }
		virtual int GetBinarySize () const override { return GetLength(); }
		virtual int GetCount () const override { return 1; }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::BINARY); }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual CDatum GetProperty (const CString& sProperty) const override { return m_Properties.GetProperty(*this, sProperty); }
		virtual const CString &GetTypename () const override;
		virtual bool IsArray () const override { return false; }
		virtual bool IsNil () const override { return (m_pData == NULL); }
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static TArray<IDatatype::SMemberDesc> GetMembers (void);

	protected:

		//	IComplexDatum
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:

		LPSTR GetBuffer () const { return (m_pData - sizeof(DWORD)); }

		LPSTR m_pData;						//	Points to data (previous DWORD is length)

		static TDatumPropertyHandler<CComplexBinary> m_Properties;
		static TDatumMethodHandler<CComplexBinary> m_Methods;
	};

class CComplexBinary64 : public IComplexDatum
	{
	public:
		CComplexBinary64 () { }
		CComplexBinary64 (IByteStream64& Stream, DWORDLONG dwLength);
		~CComplexBinary64 ();

		DWORDLONG GetLength () const { return m_Data.GetLength(); }
		void TakeHandoff (CBuffer64& Buffer);

		//	IComplexDatum

		virtual void Append (CDatum dDatum) override;
		virtual CString AsString () const override;
		virtual size_t CalcMemorySize () const override { return GetLength(); }
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual DWORD GetBasicDatatype () const override { return IDatatype::BINARY; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeBinary; }
		virtual void* GetBinaryData () const override { return m_Data.GetPointer(); }
		virtual int GetBinarySize () const override { return (int)Min(GetLength(), (DWORDLONG)INT_MAX); }
		virtual DWORDLONG GetBinarySize64 () const override { return GetLength(); }
		virtual int GetCount () const override { return 1; }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::BINARY); }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual CDatum GetProperty (const CString& sProperty) const override { return m_Properties.GetProperty(*this, sProperty); }
		virtual const CString &GetTypename () const override;
		virtual bool IsArray () const override { return false; }
		virtual bool IsNil () const override { return (GetLength() == 0); }
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static TArray<IDatatype::SMemberDesc> GetMembers (void);

	protected:

		//	IComplexDatum

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:

		CBuffer64 m_Data;

		static TDatumPropertyHandler<CComplexBinary64> m_Properties;
		static TDatumMethodHandler<CComplexBinary64> m_Methods;
	};

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
		virtual CString Format (const CStringFormat& Format) const override { return Format.FormatDateTime(CastCDateTime()); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::DATE_TIME; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeDateTime; }
		virtual int GetCount () const override { return partCount; }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::DATE_TIME); }
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual CDatum GetMethod (const CString &sMethod) const override { if (m_pMethodsExt) return m_pMethodsExt->GetMethod(sMethod); else return CDatum(); }
		virtual CDatum GetProperty (const CString& sProperty) const override { return m_Properties.GetProperty(*this, sProperty); }
		virtual const CString &GetTypename () const override;
		virtual bool IsArray () const override { return false; }
		virtual bool IsImmutable () const override { return true; }
		virtual CDatum MathMax () const override { return CDatum::raw_AsComplex(this); }
		virtual CDatum MathMin () const override { return CDatum::raw_AsComplex(this); }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static void SetMethodsExt (TDatumMethodHandler<IComplexDatum> &MethodsExt) { m_pMethodsExt = &MethodsExt; }

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

		static TDatumPropertyHandler<CComplexDateTime> m_Properties;
		static TDatumMethodHandler<IComplexDatum> *m_pMethodsExt;
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
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeImage32; }
		virtual int GetCount () const override { return 1; }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual CRGBA32Image *GetImageInterface () override { return &m_Image; }
		virtual const CString &GetTypename () const override;
		virtual bool IsArray () const override { return false; }
		virtual bool IsNil () const override { return m_Image.IsEmpty(); }
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);

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
		virtual int AsArrayIndex (int iArrayLen, bool* retbFromEnd = NULL) const override;
		virtual CString AsString (void) const override { return m_Value.AsString(); }
		virtual size_t CalcMemorySize (void) const override { return m_Value.GetSize(); }
		virtual const CIPInteger &CastCIPInteger (void) const override { return m_Value; }
		virtual double CastDouble () const override { return m_Value.AsDouble(); }
		virtual DWORDLONG CastDWORDLONG (void) const override;
		virtual int CastInteger32 (void) const override;
		virtual CString Format (const CStringFormat& Format) const override { return Format.FormatIPInteger(CastCIPInteger()); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::INT_IP; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeIntegerIP; }
		virtual int GetCount (void) const override { return 1; }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::INT_IP); }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual CDatum::Types GetNumberType (int *retiValue) override;
		virtual const CString &GetTypename (void) const override;
		virtual bool IsArray (void) const override { return false; }
		virtual bool IsIPInteger (void) const override { return true; }
		virtual bool IsImmutable () const override { return true; }
		virtual CDatum MathAbs () const override;
		virtual void MathAccumulateStats (CDatum::SStatsCtx& Stats) const override { CDatum::MathAccumulateStats(Stats, m_Value.AsDouble());}
		virtual CDatum MathAverage () const override { return CDatum::raw_AsComplex(this); }
		virtual CDatum MathCeil () const override { return CDatum::raw_AsComplex(this); }
		virtual CDatum MathFloor () const override { return CDatum::raw_AsComplex(this); }
		virtual CDatum MathMax () const override { return CDatum::raw_AsComplex(this); }
		virtual CDatum MathMedian () const override { return CDatum::raw_AsComplex(this); }
		virtual CDatum MathMin () const override { return CDatum::raw_AsComplex(this); }
		virtual CDatum MathRound () const override { return CDatum::raw_AsComplex(this); }
		virtual CDatum MathSign () const override { return (m_Value.IsZero() ? CDatum(0) : (m_Value.IsNegative() ? CDatum(-1) : CDatum(1))); }
		virtual CDatum MathSum () const override { return CDatum::raw_AsComplex(this); }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);

	protected:
		//	IComplexDatum
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override { return CIPInteger::Deserialize(Stream, &m_Value); }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override { m_Value.Serialize(Stream); }

	private:
		CIPInteger m_Value;
	};

class CAEONAnnotated : public IComplexDatum
	{
	public:

		CAEONAnnotated () { }

		static CDatum Create (CDatum dValue, const CDatum::SAnnotation& Annotations) { return CDatum(new CAEONAnnotated(dValue, Annotations)); }

		//	IComplexDatum

		virtual CString AsString (void) const override { CRecursionGuard Guard(*this); if (Guard.InRecursion()) return AsAddress(); return m_dValue.AsString(); }
		virtual size_t CalcMemorySize () const override { return m_dValue.CalcMemorySize() + sizeof(m_Annotations); }
		virtual CStringView CastCString (void) const override { return m_dValue.AsStringView(); }
		virtual const CDatum::SAnnotation& GetAnnotation () const override { return m_Annotations; }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeAnnotated; }
		virtual int GetCount () const override { return 1; }
		virtual CDatum GetElement (IInvokeCtx *pCtx, int iIndex) const override { return GetElement(iIndex); }
		virtual CDatum GetElement (int iIndex) const override { return m_dValue; }
		virtual CDatum GetElement (IInvokeCtx *pCtx, const CString &sKey) const override { return GetElement(sKey); }
		virtual CDatum GetElement (const CString &sKey) const override { return CDatum(); }
		virtual CString GetKey (int iIndex) const override { return NULL_STR; }
		virtual const CString &GetTypename () const override;
		virtual bool IsArray () const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsError (void) const override { return false; }
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);

	protected:

		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override { m_dValue.Mark(); }
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		CAEONAnnotated (CDatum dValue, const CDatum::SAnnotation& Annotations) :
				m_dValue(dValue),
				m_Annotations(Annotations)
			{ }

		CDatum m_dValue;
		CDatum::SAnnotation m_Annotations;
	};

class CAEONCopyOnWrite : public IComplexDatum,
		public IAEONTable
	{
	public:

		CAEONCopyOnWrite (CDatum dValue) : m_dValue(dValue) { if (!m_dValue.GetComplex()) throw CException(errFail); }

		//	IAEONTable
		// 
		//	NOTE: We only return this interface if the underlying datum
		//	has a valid interface, so we can guarantee that the interface
		//	pointer is valid.

		virtual EResult AppendEmptyRow (int iCount = 1) override { OnModify(); return GetTableInterfaceImpl()->AppendEmptyRow(iCount); }
		virtual EResult AppendRow (CDatum dRow, int* retiRow = NULL) override { OnModify(); return GetTableInterfaceImpl()->AppendRow(dRow, retiRow); }
		virtual EResult AppendTable (CDatum dTable) override { OnModify(); return GetTableInterfaceImpl()->AppendTable(dTable); }
		virtual CDatum CombineSubset (SSubset& ioSubset) const override { return GetTableInterfaceImpl()->CombineSubset(ioSubset); }
		virtual EResult DeleteAllRows () override { OnModify(); return GetTableInterfaceImpl()->DeleteAllRows(); }
		virtual EResult DeleteCol (int iCol) override { OnModify(); return GetTableInterfaceImpl()->DeleteCol(iCol); }
		virtual EResult DeleteRowByID (CDatum dKey) override { OnModify(); return GetTableInterfaceImpl()->DeleteRowByID(dKey); }
		virtual bool FindCol (const CString& sName, int* retiCol = NULL) const override { return GetTableInterfaceImpl()->FindCol(sName, retiCol); }
		virtual bool FindRowByID (CDatum dValue, int* retiRow = NULL) const override { return GetTableInterfaceImpl()->FindRowByID(dValue, retiRow); }
		virtual bool FindRowByID2 (CDatum dKey1, CDatum dKey2, int* retiRow = NULL) const override { return GetTableInterfaceImpl()->FindRowByID2(dKey1, dKey2, retiRow); }
		virtual bool FindRowByID3 (CDatum dKey1, CDatum dKey2, CDatum dKey3, int* retiRow = NULL) const override { return GetTableInterfaceImpl()->FindRowByID3(dKey1, dKey2, dKey3, retiRow); }
		virtual CDatum GetCol (int iIndex) const override { return GetTableInterfaceImpl()->GetCol(iIndex); }
		virtual CDatum GetCol (CAEONTypeSystem& TypeSystem, CDatum dColName) const override { return GetTableInterfaceImpl()->GetCol(TypeSystem, dColName); }
		virtual int GetColCount () const override { return GetTableInterfaceImpl()->GetColCount(); }
		virtual CString GetColName (int iCol) const override { return GetTableInterfaceImpl()->GetColName(iCol); }
		virtual CDatum GetDataSlice (int iFirstRow, int iRowCount) const override { return GetTableInterfaceImpl()->GetDataSlice(iFirstRow, iRowCount); }
		virtual CDatum GetFieldValue (int iRow, int iCol) const override { return GetTableInterfaceImpl()->GetFieldValue(iRow, iCol); }
		virtual const CAEONTableGroupDefinition& GetGroups () const override { return GetTableInterfaceImpl()->GetGroups(); }
		virtual const CAEONTableGroupIndex& GetGroupIndex () const override { return GetTableInterfaceImpl()->GetGroupIndex(); }
		virtual CDatum GetKeyFromRow (CDatum dTable, CDatum dRow) const override { return GetTableInterfaceImpl()->GetKeyFromRow(dTable, dRow); }
		virtual TSortMap<CDatum, int> GetKeyIndex (CDatum dTable) const override { return GetTableInterfaceImpl()->GetKeyIndex(dTable); }
		virtual EKeyType GetKeyType () const override { return GetTableInterfaceImpl()->GetKeyType(); }
		virtual SequenceNumber GetNextID () const override { return GetTableInterfaceImpl()->GetNextID(); }
		virtual CDatum GetRow (int iRow) const override { return GetTableInterfaceImpl()->GetRow(iRow); }
		virtual int GetRowCount () const override { return GetTableInterfaceImpl()->GetRowCount(); }
		virtual CDatum GetRowID (int iRow) const override { return GetTableInterfaceImpl()->GetRowID(iRow); }
		virtual CDatum GetSchema () const override { return GetTableInterfaceImpl()->GetSchema(); }
		virtual SequenceNumber GetSeq () const override { return GetTableInterfaceImpl()->GetSeq(); }
		virtual EResult InsertColumn (const CString& sName, CDatum dType, CDatum dValues = CDatum(), int iPos = -1, int* retiCol = NULL) override { OnModify(); return GetTableInterfaceImpl()->InsertColumn(sName, dType, dValues, iPos, retiCol); }
		virtual void InvalidateKeys () override { OnModify(); GetTableInterfaceImpl()->InvalidateKeys(); }
		virtual bool IsSameSchema (CDatum dSchema) const override { return GetTableInterfaceImpl()->IsSameSchema(dSchema); }
		virtual SequenceNumber MakeID () override { OnModify(); return GetTableInterfaceImpl()->MakeID(); }
		virtual CDatum Query (const CAEONExpression& Expr) const override { return GetTableInterfaceImpl()->Query(Expr); }
		virtual EResult SetColumn (int iCol, IDatatype::SMemberDesc& ColDesc, CDatum dValues = CDatum()) override { OnModify(); return GetTableInterfaceImpl()->SetColumn(iCol, ColDesc, dValues); }
		virtual bool SetFieldValue (int iRow, int iCol, CDatum dValue) override { OnModify(); return GetTableInterfaceImpl()->SetFieldValue(iRow, iCol, dValue); }
		virtual void SetGroups (const CAEONTableGroupDefinition& Groups) override { OnModify(); GetTableInterfaceImpl()->SetGroups(Groups); }
		virtual void SetGroupIndex (CAEONTableGroupIndex&& Index) override { OnModify(); GetTableInterfaceImpl()->SetGroupIndex(std::move(Index)); }
		virtual EResult SetNextID (SequenceNumber NextID) override { OnModify(); return GetTableInterfaceImpl()->SetNextID(NextID); }
		virtual void SetReadOnly (bool bValue = true) override { OnModify(); GetTableInterfaceImpl()->SetReadOnly(bValue); }
		virtual EResult SetRow (int iRow, CDatum dRow) override { OnModify(); return GetTableInterfaceImpl()->SetRow(iRow, dRow); }
		virtual EResult SetRowByID (CDatum dKey, CDatum dRow, int* retiRow = NULL) override { OnModify(); return GetTableInterfaceImpl()->SetRowByID(dKey, dRow, retiRow); }
		virtual void SetSeq (SequenceNumber Seq) override { OnModify(); GetTableInterfaceImpl()->SetSeq(Seq); }
		virtual CDatum Sort (const TArray<SSort>& Sort) const override { return GetTableInterfaceImpl()->Sort(Sort); }

		//	IComplexDatum

		virtual void Append (CDatum dDatum) override { OnModify(); m_dValue.raw_GetComplex()->Append(dDatum); }
		virtual void AppendArray (CDatum dDatum) override { OnModify(); m_dValue.raw_GetComplex()->AppendArray(dDatum); }
		virtual int AsArrayIndex (int iArrayLen, bool* retbFromEnd = NULL) const override { return m_dValue.raw_GetComplex()->AsArrayIndex(iArrayLen, retbFromEnd); }
		virtual CString AsString () const override { return m_dValue.raw_GetComplex()->AsString(); }
		virtual size_t CalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override { return m_dValue.raw_GetComplex()->CalcSerializeSizeAEONScript(iFormat); }
		virtual size_t CalcMemorySize () const override { return m_dValue.raw_GetComplex()->CalcMemorySize(); }
		virtual bool CanInvoke () const override { return m_dValue.raw_GetComplex()->CanInvoke(); }
		virtual const CDateTime &CastCDateTime () const override { return m_dValue.raw_GetComplex()->CastCDateTime(); }
		virtual const CIPInteger &CastCIPInteger () const override { return m_dValue.raw_GetComplex()->CastCIPInteger(); }
		virtual const CRGBA32Image &CastCRGBA32Image () const override { return m_dValue.raw_GetComplex()->CastCRGBA32Image(); }
		virtual CStringView CastCString () const override { return m_dValue.raw_GetComplex()->CastCString(); }
		virtual const CStringFormat& CastCStringFormat () const override { return m_dValue.raw_GetComplex()->CastCStringFormat(); }
		virtual const CTimeSpan &CastCTimeSpan () const override { return m_dValue.raw_GetComplex()->CastCTimeSpan(); }
		virtual const CVector2D& CastCVector2D () const override { return m_dValue.raw_GetComplex()->CastCVector2D(); }
		virtual double CastDouble () const override { return m_dValue.raw_GetComplex()->CastDouble(); }
		virtual DWORDLONG CastDWORDLONG () const override { return m_dValue.raw_GetComplex()->CastDWORDLONG(); }
		virtual const IDatatype &CastIDatatype () const override { return m_dValue.raw_GetComplex()->CastIDatatype(); }
		virtual int CastInteger32 () const override { return m_dValue.raw_GetComplex()->CastInteger32(); }
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override { return m_dValue.raw_GetComplex()->Clone(iMode); }
		virtual bool Contains (CDatum dValue) const override { return m_dValue.raw_GetComplex()->Contains(dValue); }
		virtual void DeleteElement (int iIndex) override { OnModify(); m_dValue.raw_GetComplex()->DeleteElement(iIndex); }
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const override { return m_dValue.raw_GetComplex()->Find(dValue, retiIndex); }
		virtual CDatum FindAll (CDatum dValue) const override { return m_dValue.raw_GetComplex()->FindAll(dValue); }
		virtual CDatum FindAllExact (CDatum dValue) const override { return m_dValue.raw_GetComplex()->FindAllExact(dValue); }
		virtual bool FindExact (CDatum dValue, int *retiIndex = NULL) const override { return m_dValue.raw_GetComplex()->FindExact(dValue, retiIndex); }
		virtual bool FindElement (const CString &sKey, CDatum *retpValue) const override { return m_dValue.raw_GetComplex()->FindElement(sKey, retpValue); }
		virtual int FindMaxElement () const override { return m_dValue.raw_GetComplex()->FindMaxElement(); }
		virtual int FindMinElement () const override { return m_dValue.raw_GetComplex()->FindMinElement(); }
		virtual const CDatum::SAnnotation& GetAnnotation () const override { return m_dValue.raw_GetComplex()->GetAnnotation(); }
		virtual DWORD GetBasicDatatype () const override { return m_dValue.raw_GetComplex()->GetBasicDatatype(); }
		virtual CDatum::Types GetBasicType () const override { return m_dValue.raw_GetComplex()->GetBasicType(); }
		virtual void* GetBinaryData () const override { return m_dValue.raw_GetComplex()->GetBinaryData(); }
		virtual int GetBinarySize () const override { return m_dValue.raw_GetComplex()->GetBinarySize(); }
		virtual DWORDLONG GetBinarySize64 () const override { return m_dValue.raw_GetComplex()->GetBinarySize64(); }
		virtual CDatum::ECallType GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const override { return m_dValue.raw_GetComplex()->GetCallInfo(retdCodeBank, retpIP); }
		virtual IAEONCanvas *GetCanvasInterface () override { return m_dValue.raw_GetComplex()->GetCanvasInterface(); }
		virtual int GetCount () const override { return m_dValue.raw_GetComplex()->GetCount(); }
		virtual CDatum GetDatatype () const override { return m_dValue.raw_GetComplex()->GetDatatype(); }
		virtual int GetDimensions () const override { return m_dValue.raw_GetComplex()->GetDimensions(); }
		virtual CDatum GetElement (IInvokeCtx *pCtx, int iIndex) const override { return m_dValue.raw_GetComplex()->GetElement(pCtx, iIndex); }
		virtual CDatum GetElement (int iIndex) const override { return m_dValue.raw_GetComplex()->GetElement(iIndex); }
		virtual CDatum GetElement (IInvokeCtx *pCtx, const CString &sKey) const override { return m_dValue.raw_GetComplex()->GetElement(pCtx, sKey); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_dValue.raw_GetComplex()->GetElement(sKey); }
		virtual CDatum GetElementAt (int iIndex) const override { return m_dValue.raw_GetComplex()->GetElementAt(iIndex); }
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override { return m_dValue.raw_GetComplex()->GetElementAt(TypeSystem, dIndex); }
		virtual CRGBA32Image *GetImageInterface () override { return m_dValue.raw_GetComplex()->GetImageInterface(); }
		virtual CString GetKey (int iIndex) const override { return m_dValue.raw_GetComplex()->GetKey(iIndex); }
		virtual CDatum GetKeyEx (int iIndex) const override { return m_dValue.raw_GetComplex()->GetKeyEx(iIndex); }
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_dValue.raw_GetComplex()->GetMethod(sMethod); }
		virtual IComplexDatum* GetMethodThis () override { return this; }
		virtual CDatum::Types GetNumberType (int *retiValue) override { return m_dValue.raw_GetComplex()->GetNumberType(retiValue); }
		virtual CDatum GetProperty (const CString& sProperty) const override { return m_dValue.raw_GetComplex()->GetProperty(sProperty); }
		virtual const CAEONExpression* GetQueryInterface () const override { return m_dValue.raw_GetComplex()->GetQueryInterface(); }
		virtual IAEONReanimator* GetReanimatorInterface () override { return m_dValue.raw_GetComplex()->GetReanimatorInterface(); }
		virtual IAEONTable *GetTableInterface () override { return (GetTableInterfaceImpl() ? this : NULL); }
		virtual IAEONTextLines *GetTextLinesInterface () override { return m_dValue.raw_GetComplex()->GetTextLinesInterface(); }
		virtual const CString &GetTypename () const override { return m_dValue.raw_GetComplex()->GetTypename(); }
		virtual void GrowToFit (int iCount) override { OnModify(); m_dValue.raw_GetComplex()->GrowToFit(iCount); }
		virtual bool HasKeys () const override { return m_dValue.raw_GetComplex()->HasKeys(); }
		virtual void InsertElementAt (CDatum dIndex, CDatum dDatum) override { OnModify(); m_dValue.raw_GetComplex()->InsertElementAt(dIndex, dDatum); }
		virtual CDatum::InvokeResult Invoke (IInvokeCtx *pCtx, CHexeLocalEnvironment& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult) override { OnModify(); return m_dValue.raw_GetComplex()->Invoke(pCtx, LocalEnv, dwExecutionRights, retResult); }
		virtual CDatum::InvokeResult InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, SAEONInvokeResult& retResult) override { OnModify(); return m_dValue.raw_GetComplex()->InvokeContinues(pCtx, dContext, dResult, retResult); }
		virtual CDatum::InvokeResult InvokeLibrary (IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult) override { OnModify(); return m_dValue.raw_GetComplex()->InvokeLibrary(Ctx, LocalEnv, dwExecutionRights, retResult); }
		virtual bool InvokeMethodImpl(CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override { OnModify(); return m_dValue.raw_GetComplex()->InvokeMethodImpl(dObj, sMethod, Ctx, LocalEnv, retResult); }
		virtual bool IsArray () const override { return m_dValue.raw_GetComplex()->IsArray(); }
		virtual bool IsContainer () const override { return m_dValue.raw_GetComplex()->IsContainer(); }
		virtual bool IsError () const override { return m_dValue.raw_GetComplex()->IsError(); }
		virtual bool IsIPInteger () const override { return m_dValue.raw_GetComplex()->IsIPInteger(); }
		virtual bool IsMemoryBlock () const override { return m_dValue.raw_GetComplex()->IsMemoryBlock(); }
		virtual bool IsNil () const override { return m_dValue.raw_GetComplex()->IsNil(); }
		virtual bool IsStruct () const override { return m_dValue.raw_GetComplex()->IsStruct(); }
		virtual CDatum IteratorBegin () const { return m_dValue.raw_GetComplex()->IteratorBegin(); }
		virtual CDatum IteratorGetKey (CDatum dIterator) const override { return m_dValue.raw_GetComplex()->IteratorGetKey(dIterator); };
		virtual CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const override { return m_dValue.raw_GetComplex()->IteratorGetValue(TypeSystem, dIterator); }
		virtual CDatum IteratorNext (CDatum dIterator) const { return m_dValue.raw_GetComplex()->IteratorNext(dIterator); }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override { return m_dValue.raw_GetComplex()->OpCompare(iValueType, dValue); }
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return m_dValue.raw_GetComplex()->OpCompareExact(iValueType, dValue); }
		virtual CDatum OpConcatenated (IInvokeCtx& Ctx, CDatum dSrc, int iAxis) const override { return m_dValue.raw_GetComplex()->OpConcatenated(Ctx, dSrc, iAxis); }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override { return m_dValue.raw_GetComplex()->OpIsEqual(iValueType, dValue); }
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override { return m_dValue.raw_GetComplex()->OpIsIdentical(iValueType, dValue); }
		virtual CDatum MathAbs () const override { return m_dValue.raw_GetComplex()->MathAbs(); }
		virtual void MathAccumulateStats (CDatum::SStatsCtx& Stats) const override { m_dValue.raw_GetComplex()->MathAccumulateStats(Stats); }
		virtual CDatum MathAverage () const override { return m_dValue.raw_GetComplex()->MathAverage(); }
		virtual CDatum MathCeil () const override { return m_dValue.raw_GetComplex()->MathCeil(); }
		virtual CDatum MathFloor () const override { return m_dValue.raw_GetComplex()->MathFloor(); }
		virtual CDatum MathMax () const override { return m_dValue.raw_GetComplex()->MathMax(); }
		virtual CDatum MathMedian () const override { return m_dValue.raw_GetComplex()->MathMedian(); }
		virtual CDatum MathMin () const override { return m_dValue.raw_GetComplex()->MathMin(); }
		virtual CDatum MathRound () const override { return m_dValue.raw_GetComplex()->MathRound(); }
		virtual CDatum MathSign () const override { return m_dValue.raw_GetComplex()->MathSign(); }
		virtual CDatum MathSum () const override { return m_dValue.raw_GetComplex()->MathSum(); }
		virtual bool RemoveAll () override { OnModify(); return m_dValue.raw_GetComplex()->RemoveAll(); }
		virtual bool RemoveElementAt (CDatum dIndex) override { OnModify(); return m_dValue.raw_GetComplex()->RemoveElementAt(dIndex); }
		virtual void ResolveDatatypes (const CAEONTypeSystem &TypeSystem) override { OnModify(); m_dValue.raw_GetComplex()->ResolveDatatypes(TypeSystem); }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override { m_dValue.raw_GetComplex()->Serialize(iFormat, Stream); }
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override { m_dValue.raw_GetComplex()->SerializeAEON(Stream, Serialized); }
		virtual void SetElement (IInvokeCtx *pCtx, const CString &sKey, CDatum dDatum) override { OnModify(); m_dValue.raw_GetComplex()->SetElement(pCtx, sKey, dDatum); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { OnModify(); m_dValue.raw_GetComplex()->SetElement(sKey, dDatum); }
		virtual void SetElement (int iIndex, CDatum dDatum) override { OnModify(); m_dValue.raw_GetComplex()->SetElement(iIndex, dDatum); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override { OnModify(); m_dValue.raw_GetComplex()->SetElementAt(dIndex, dDatum); }
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { OnModify(); m_dValue.raw_GetComplex()->Sort(Order, pfCompare, pCtx); }
		virtual void WriteBinaryToStream (IByteStream &Stream, int iPos = 0, int iLength = -1, IProgressEvents *pProgress = NULL) const override { m_dValue.raw_GetComplex()->WriteBinaryToStream(Stream, iPos, iLength, pProgress); }
		virtual void WriteBinaryToStream64 (IByteStream64 &Stream, DWORDLONG dwPos = 0, DWORDLONG dwLength = 0xffffffffffffffff, IProgressEvents *pProgress = NULL) const override { m_dValue.raw_GetComplex()->WriteBinaryToStream64(Stream, dwPos, dwLength, pProgress); }

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override { return m_dValue.raw_GetComplex()->OnCalcSerializeSizeAEONScript(iFormat); }
		virtual DWORD OnGetSerializeFlags () const override { return m_dValue.raw_GetComplex()->OnGetSerializeFlags(); }
		virtual void OnMarked () override { m_dValue.Mark(); }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override { m_dValue.raw_GetComplex()->OnSerialize(iFormat, Stream); }
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override { m_dValue.raw_GetComplex()->OnSerialize(iFormat, pStruct); }

		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override { m_dValue.raw_GetComplex()->SerializeAEONExternal(Stream, Serialized); }

	private:

		IAEONTable* GetTableInterfaceImpl () { return m_dValue.raw_GetComplex()->GetTableInterface(); }
		const IAEONTable* GetTableInterfaceImpl () const { return m_dValue.raw_GetComplex()->GetTableInterface(); }

		void OnModify ()
			{
			if (m_bCopyOnWrite)
				{
				m_bCopyOnWrite = false;
				m_dValue = m_dValue.Clone(CDatum::EClone::DeepCopy);
				}
			}

		CDatum m_dValue;
		bool m_bCopyOnWrite = true;
	};

class CAEONDictionary : public IComplexDatum
	{
	public:

		CAEONDictionary (CDatum dDatatype = CDatum(), CDatum dSrc = CDatum());
		CAEONDictionary (CDatum dDatatype, const TSortMap<CDatum, CDatum>& Src);

		bool DeleteElement (CDatum dKey) { OnCopyOnWrite(); return m_Map.DeleteAt(dKey); }
		bool SetElementIfNew (CDatum dKey, CDatum dValue);

		//	IComplexDatum

		virtual void Append (CDatum dDatum) override { AppendStruct(dDatum); }
		virtual CString AsString () const override;
		virtual size_t CalcMemorySize () const override;
		virtual IComplexDatum* Clone (CDatum::EClone iMode) const override;
		virtual bool Contains (CDatum dValue) const override;
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const override;
		virtual CDatum FindAll (CDatum dValue) const override;
		virtual CDatum FindAllExact (CDatum dValue) const override;
		virtual bool FindElement (const CString& sKey, CDatum *retpValue) const override;
		virtual bool FindExact (CDatum dValue, int *retiIndex = NULL) const override;
		virtual DWORD GetBasicDatatype () const override { return IDatatype::STRUCT; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeStruct; }
		virtual int GetCount () const override { return m_Map.GetCount(); }
		virtual CDatum GetDatatype () const override { return m_dDatatype; }
		virtual int GetDimensions () const override { return 1; }
		virtual CDatum GetElement (int iIndex) const override { return ((iIndex >= 0 && iIndex < m_Map.GetCount()) ? m_Map[iIndex] : CDatum()); }
		virtual CDatum GetElement (const CString& sKey) const override;
		virtual CDatum GetElementAt (CAEONTypeSystem& TypeSystem, CDatum dIndex) const override;
		virtual CString GetKey (int iIndex) const override { return m_Map.GetKey(iIndex).AsString(); }
		virtual CDatum GetKeyEx (int iIndex) const override { return CDatum(m_Map.GetKey(iIndex)); }
		virtual CDatum GetMethod (const CString& sMethod) const override;
		virtual CDatum GetProperty (const CString& sProperty) const override;
		virtual const CString& GetTypename () const override;
		virtual void GrowToFit (int iCount) override { OnCopyOnWrite(); m_Map.GrowToFit(iCount); }
		virtual bool HasKeys () const override { return true; }
		virtual void InsertElementAt (CDatum dIndex, CDatum dDatum) override { SetElementAt(dIndex, dDatum); }
		virtual bool IsArray () const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil () const override { return (GetCount() == 0); }
		virtual bool IsStruct () const override { return true; }
		virtual CDatum IteratorGetKey (CDatum dIterator) const override { return GetKeyEx((int)dIterator); };
		virtual CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const override { return GetElement((int)dIterator); }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool OpContains (CDatum dValue) const override { return m_Map.GetAt(dValue) != NULL; }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool RemoveAll () override { OnCopyOnWrite(); m_Map.DeleteAll(); return true; }
		virtual bool RemoveElementAt (CDatum dIndex) override;
		virtual void ResolveDatatypes (const CAEONTypeSystem& TypeSystem) override;
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream& Stream) const override { SerializeAsStruct(iFormat, Stream); }
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void SetElement (const CString& sKey, CDatum dDatum) override { OnCopyOnWrite(); m_Map.SetAt(sKey, dDatum); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override;

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap& Serialized);
		static int FindMethodByKey (const CString& sKey) { return (m_pMethodsExt ? m_pMethodsExt->FindMethod(sKey) : -1); }
		static int FindPropertyByKey (const CString& sKey) { return m_Properties.FindProperty(sKey); }
		static int GetPropertyCount () { return m_Properties.GetCount(); }
		static CString GetPropertyKey (int iIndex) { return m_Properties.GetPropertyName(iIndex); }
		static CDatum GetPropertyType (int iIndex) { return m_Properties.GetPropertyType(iIndex); }
		static int GetMethodCount () { return (m_pMethodsExt ? m_pMethodsExt->GetCount() : 0); }
		static CString GetMethodKey (int iIndex) { return (m_pMethodsExt ? m_pMethodsExt->GetMethodName(iIndex) : NULL_STR); }
		static CDatum GetMethodType (int iIndex) { return (m_pMethodsExt ? m_pMethodsExt->GetMethodType(iIndex) : CAEONTypes::Get(IDatatype::FUNCTION)); }
		static void SetMethodsExt (TDatumMethodHandler<IComplexDatum>& MethodsExt) { m_pMethodsExt = &MethodsExt; }

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override { return CalcSerializeAsStructSize(iFormat); }
		virtual void OnMarked () override;

		void AppendStruct (CDatum dDatum);
		void CloneContents ();
		CDatum DatumToElement (CDatum dValue) const;
		CDatum DatumToKey (CDatum dValue) const;
		void InitDatatype (CDatum dDatatype);
		void OnCopyOnWrite ();

		bool m_bCopyOnWrite = false;
		TSortMap<CDatum, CDatum> m_Map;
		CDatum m_dDatatype;
		CDatum m_dKeyType;
		CDatum m_dElementType;

		static TDatumPropertyHandler<CAEONDictionary> m_Properties;
		static TDatumMethodHandler<IComplexDatum>* m_pMethodsExt;
	};

class CAEONError : public IComplexDatum
	{
	public:
		CAEONError () { }

		static CDatum Create (const CString &sErrorCode, const CString &sErrorDesc) { return CDatum(new CAEONError(sErrorCode, sErrorDesc)); }

		//	IComplexDatum

		virtual CString AsString (void) const override { return m_sDescription; }
		virtual size_t CalcMemorySize () const override { return sizeof(CAEONError); }
		virtual CStringView CastCString (void) const override { return m_sDescription; }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::ERROR_T; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeError; }
		virtual int GetCount () const override { return 1; }
		virtual CDatum GetElement (IInvokeCtx *pCtx, int iIndex) const override { return GetElement(iIndex); }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual CDatum GetElement (IInvokeCtx *pCtx, const CString &sKey) const override { return GetElement(sKey); }
		virtual CDatum GetElement (const CString &sKey) const override { return CDatum(); }
		virtual CString GetKey (int iIndex) const override { return NULL_STR; }
		virtual const CString &GetTypename () const override;
		virtual bool IsArray () const override { return false; }
		virtual bool IsError (void) const override { return true; }
		virtual bool IsImmutable () const override { return true; }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);

	protected:

		//	IComplexDatum

		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:

		CAEONError (const CString& sErrorCode, const CString& sErrorDesc) :
				m_sError(sErrorCode),
				m_sDescription(sErrorDesc)
			{ }

		CString m_sError;
		CString m_sDescription;
	};

class CAEONForeign : public IComplexDatum
	{
	public:
		CAEONForeign () { }
		CAEONForeign (const CString& sTypename, CDatum dData) :
				m_sTypename(sTypename),
				m_dData(dData)
			{ }

		static CDatum Create (const CString& sTypename, CString&& sData);
		static bool CreateFromStream (const CString& sTypename, CCharStream& Stream, CDatum& retdDatum);

		//	IComplexDatum

		virtual CString AsString (void) const override { return m_dData.AsString(); }
		virtual size_t CalcMemorySize () const override { return sizeof(CAEONForeign) + m_sTypename.GetLength() + m_dData.CalcMemorySize(); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeForeign; }
		virtual int GetCount () const override { return 1; }
		virtual CDatum GetElement (IInvokeCtx *pCtx, int iIndex) const override { return GetElement(iIndex); }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual CDatum GetElement (IInvokeCtx *pCtx, const CString &sKey) const override { return GetElement(sKey); }
		virtual CDatum GetElement (const CString &sKey) const override { return CDatum(); }
		virtual CString GetKey (int iIndex) const override { return NULL_STR; }
		virtual const CString &GetTypename () const override { return m_sTypename; }
		virtual bool IsArray () const override { return false; }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

	protected:

		//	IComplexDatum

		virtual void OnMarked (void) override { m_dData.Mark(); }

	private:

		CString m_sTypename;
		CDatum m_dData;
	};

class CAEONLibraryFunction : public IComplexDatum
	{
	public:

		static CDatum Create (const SAEONLibraryFunctionCreate& Def) { return CDatum(new CAEONLibraryFunction(Def)); }

		//	IComplexDatum

		virtual CString AsString () const override { return strPattern("[%s]", GetTypename()); }
		virtual bool CanInvoke (void) const override { return true; }
		virtual size_t CalcMemorySize () const override { return sizeof(CAEONLibraryFunction); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::FUNCTION; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeLibraryFunc; }
		virtual CDatum::ECallType GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const override { return CDatum::ECallType::Library; }
		virtual int GetCount () const override { return 1; }
		virtual CDatum GetDatatype () const override { return m_dType; }
		virtual CDatum GetElement (IInvokeCtx *pCtx, int iIndex) const override { return GetElement(iIndex); }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual CDatum GetElement (IInvokeCtx *pCtx, const CString &sKey) const override { return GetElement(sKey); }
		virtual CDatum GetElement (const CString &sKey) const override { return CDatum(); }
		virtual CString GetKey (int iIndex) const override { return NULL_STR; }
		virtual const CString &GetTypename (void) const override;
		virtual CDatum::InvokeResult Invoke (IInvokeCtx *pCtx, CHexeLocalEnvironment& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult) override;
		virtual CDatum::InvokeResult InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, SAEONInvokeResult& retResult) override;
		virtual CDatum::InvokeResult InvokeLibrary (IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult) override;
		virtual bool IsArray () const override { return false; }
		virtual bool IsImmutable () const override { return true; }

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override { return (size_t)m_sName.GetLength() + 2; }
		virtual void OnMarked (void) override { m_dType.Mark(); }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

	private:

		CAEONLibraryFunction (const SAEONLibraryFunctionCreate& Def) :
				m_sName(Def.sName),
				m_fnInvoke(Def.fnInvoke),
				m_dwData(Def.dwData),
				m_dType(Def.dType),
				m_dwExecFlags(Def.dwExecFlags)
			{ }

		static CDatum::InvokeResult HandleSpecialResult (SAEONInvokeResult& retResult);

		CString m_sName;
		std::function<bool(IInvokeCtx&, DWORD, CHexeStackEnv&, CDatum, CDatum, SAEONInvokeResult&)> m_fnInvoke;
		DWORD m_dwData = 0;
		CDatum m_dType;
		DWORD m_dwExecFlags = 0;
	};

class CAEONNilImpl
	{
	public:

		static CDatum GetProperty (const CString &sKey) { return m_Properties.GetProperty(0, sKey); }
		static CDatum GetMethod (const CString &sMethod) { return m_Methods.GetStaticMethod(sMethod); }
		bool InvokeMethodImpl(const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult)
			{ return m_Methods.InvokeStaticMethod(sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }

	private:
		static TDatumPropertyHandler<int> m_Properties;
		static TDatumMethodHandler<int> m_Methods;
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
		virtual DWORD GetBasicDatatype () const override { return ((const IDatatype&)m_dType).GetClass() == IDatatype::ECategory::ClassDef ? IDatatype::CLASS_T : IDatatype::SCHEMA; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeClassInstance; }
		virtual CDatum GetDatatype () const override { return m_dType; }
		virtual CDatum GetElement (int iIndex) const override;
		virtual CString GetKey (int iIndex) const override;
		virtual const CString &GetTypename () const override;
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil () const { return false; }	//	Objects are never nil
		virtual void OnMarked (void) override { m_dType.Mark(); CComplexStruct::OnMarked(); }
		virtual void ResolveDatatypes (const CAEONTypeSystem &TypeSystem) override;
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override;
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override;

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);

	private:
		CDatum m_dType;
	};

class CAEONRange : public IComplexDatum, public IAEONRange
	{
	public:

		static CDatum Create (int iStart, int iEnd, int iStep);

		//	IAEONRange

		virtual CDatum GetEnd () const override { return CDatum(m_iEnd); }
		virtual int GetLength () const override;
		virtual CDatum GetStart () const override { return CDatum(m_iStart); }
		virtual CDatum GetStep () const override { return CDatum(m_iStep); }

		//	IComplexDatum

		virtual CString AsString (void) const override;
		virtual size_t CalcMemorySize (void) const override { return 3 * sizeof(int); }
		virtual bool Contains (CDatum dValue) const override;
		virtual bool FindElement (const CString &sKey, CDatum *retpValue) const override { return m_Properties.FindProperty(sKey) != -1; }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::RANGE; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeRange; }
		virtual int GetCount (void) const override { return m_Properties.GetCount(); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::RANGE); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElement (int iIndex) const override { return m_Properties.GetProperty(*this, iIndex); }
		virtual CString GetKey (int iIndex) const override { return m_Properties.GetPropertyName(iIndex); }
		virtual CDatum GetMethod (const CString& sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual IAEONRange* GetRangeInterface () override { return this; }
		virtual const CString &GetTypename (void) const override;
		virtual bool IsArray (void) const override { return false; }
		virtual bool IsImmutable () const override { return true; }
		virtual bool IsNil (void) const override { return m_iStep == 0; }
		virtual CDatum IteratorBegin () const { return (GetLength() > 0 ? CDatum(m_iStart) : CDatum()); }
		virtual CDatum IteratorGetKey (CDatum dIterator) const override { return dIterator; };
		virtual CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const override { return dIterator; }
		virtual CDatum IteratorNext (CDatum dIterator) const { int iValue = dIterator; if (iValue == m_iEnd) return CDatum(); else return CDatum(iValue + m_iStep); }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return OpCompare(iValueType, dValue); }
		virtual bool OpContains (CDatum dValue) const override;
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override { return OpIsEqual(iValueType, dValue); }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static int FindPropertyByKey (const CString& sKey) { return m_Properties.FindProperty(sKey); }
		static TArray<IDatatype::SMemberDesc> GetMembers (void);

	private:

		//	IComplexDatum

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override { throw CException(errFail); }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override { throw CException(errFail); }
		virtual void OnMarked (void) override { }

		int m_iStart = 0;
		int m_iEnd = 0;
		int m_iStep = 0;	//	0 means null

		static TDatumPropertyHandler<CAEONRange> m_Properties;
		static TDatumMethodHandler<CAEONRange> m_Methods;
	};

class CAEONRowRefImpl
	{
	public:

		static CString AsString (DWORDLONG dwData);
		static CDatum AsStruct (DWORDLONG dwData);
		static size_t CalcMemorySize (DWORDLONG dwData);
		static size_t CalcSerializeSizeAEONScript (DWORDLONG dwData, CDatum::EFormat iFormat);
		static CDatum Clone (DWORDLONG dwData, CDatum::EClone iMode);
		static bool Contains (DWORDLONG dwData, CDatum dValue);
		static bool EnumElements (DWORDLONG dwData, DWORD dwFlags, std::function<bool(CDatum)> fn);
		static bool Find (DWORDLONG dwData, CDatum dValue, int* retiIndex = NULL);
		static CDatum FindAll (DWORDLONG dwData, CDatum dValue);
		static CDatum FindAllExact (DWORDLONG dwData, CDatum dValue);
		static bool FindElement (DWORDLONG dwData, CStringView sKey, CDatum* retpValue);
		static bool FindExact (DWORDLONG dwData, CDatum dValue, int* retiIndex = NULL);
		static CDatum GetDatatype (DWORDLONG dwData);
		static int GetCount (DWORDLONG dwData);
		static CDatum GetElement (DWORDLONG dwData, IInvokeCtx* pCtx, int iIndex);
		static CDatum GetElement (DWORDLONG dwData, IInvokeCtx* pCtx, CStringView sKey);
		static CDatum GetElementAt (DWORDLONG dwData, int iIndex);
		static CDatum GetElementAt (DWORDLONG dwData, CAEONTypeSystem& TypeSystem, CDatum dIndex);
		static CString GetKey (DWORDLONG dwData, int iIndex);
		static CDatum GetKeys (DWORDLONG dwData);
		static CDatum GetMethod (DWORDLONG dwData, const CString &sMethod);
		static int GetMethodCount () { return m_Methods.GetCount(); }
		static CString GetMethodKey (int iIndex) { return m_Methods.GetMethodName(iIndex); }
		static CDatum GetMethodType (int iIndex) { return m_Methods.GetMethodType(iIndex); }
		static CDatum GetProperty (DWORDLONG dwData, const CString &sKey);
		static int GetPropertyCount () { return m_Properties.GetCount(); }
		static CString GetPropertyKey (int iIndex) { return m_Properties.GetPropertyName(iIndex); }
		static CDatum GetPropertyType (int iIndex) { return m_Properties.GetPropertyType(iIndex); }
		static const CString& GetTypename (DWORDLONG dwData);
		static bool InvokeMethodImpl(DWORDLONG dwData, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult)
			{ return m_Methods.InvokeMethod(dwData, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		static bool IsNil (DWORDLONG dwData);
		static size_t Hash (DWORDLONG dwData);
		static void Mark (DWORDLONG dwData);
		static int OpCompare (DWORDLONG dwSrcData, DWORDLONG dwDestData);
		static int OpCompare (DWORDLONG dwSrcData, CDatum dDestStruct);
		static int OpCompare (CDatum dSrcStruct, DWORDLONG dwDestData);
		static int OpCompareExact (DWORDLONG dwSrcData, DWORDLONG dwDestData);
		static int OpCompareExact (DWORDLONG dwSrcData, CDatum dDestStruct);
		static int OpCompareExact (CDatum dSrcStruct, DWORDLONG dwDestData);
		static bool OpContains (DWORDLONG dwData, CDatum dValue);
		static bool OpIsEqual (DWORDLONG dwSrcData, DWORDLONG dwDestData);
		static bool OpIsEqual (DWORDLONG dwSrcData, CDatum dDestStruct) { return OpCompare(dwSrcData, dDestStruct) == 0; }
		static bool OpIsEqual (CDatum dSrcStruct, DWORDLONG dwDestData) { return OpCompare(dSrcStruct, dwDestData) == 0; }
		static bool OpIsIdentical (DWORDLONG dwSrcData, DWORDLONG dwDestData);
		static bool OpIsIdentical (DWORDLONG dwSrcData, CDatum dDestStruct) { return OpCompareExact(dwSrcData, dDestStruct) == 0; }
		static bool OpIsIdentical (CDatum dSrcStruct, DWORDLONG dwDestData) { return OpCompareExact(dSrcStruct, dwDestData) == 0; }
		static void Serialize (DWORDLONG dwData, CDatum::EFormat iFormat, IByteStream& Stream);
		static void SerializeAEON (DWORDLONG dwData, IByteStream& Stream, CAEONSerializedMap& Serialized);
		static void SetElement (DWORDLONG dwData, IInvokeCtx* pCtx, const CString& sKey, CDatum dDatum);
		static void SetElement (DWORDLONG dwData, int iIndex, CDatum dDatum);
		static void SetElementAt (DWORDLONG dwData, CDatum dKey, CDatum dDatum);

		static DWORDLONG Encode (DWORD dwTableID, int iRowIndex) { return (((DWORDLONG)dwTableID) << 32) | (iRowIndex & 0x7FFFFFFF); }

	private:

		static DWORD DecodeTableID (DWORDLONG dwData) { return (DWORD)(dwData >> 32) & 0xFFFF; }
		static int DecodeRowIndex (DWORDLONG dwData) { return (int)(dwData & 0x7FFFFFFF); }
		static CDatum GetTable (DWORDLONG dwData) { return CAEONStore::GetTableByID(DecodeTableID(dwData)); }
		static int GetRowIndex (DWORDLONG dwData) { return DecodeRowIndex(dwData); }
		static IAEONTable* ResolveTable (DWORDLONG dwData, int& retiRowIndex);

		static TDatumPropertyHandler<DWORDLONG> m_Properties;
		static TDatumMethodHandler<DWORDLONG> m_Methods;
	};

class CAEONStringImpl
	{
	public:

		static int FindMethodByKey (const CString& sKey) { return m_Methods.FindMethod(sKey); }
		static int FindPropertyByKey (const CString& sKey) { return m_Properties.FindProperty(sKey); }
		static CDatum GetProperty (const CString& sValue, const CString &sKey) { return m_Properties.GetProperty(sValue, sKey); }
		static CDatum GetElementAt (const CString& sValue, CAEONTypeSystem &TypeSystem, CDatum dIndex);
		static CDatum GetMethod (const CString &sMethod) { return m_Methods.GetMethod(sMethod); }
		static int GetMethodCount () { return m_Methods.GetCount(); }
		static CString GetMethodKey (int iIndex) { return m_Methods.GetMethodName(iIndex); }
		static CDatum GetMethodType (int iIndex) { return m_Methods.GetMethodType(iIndex); }
		static int GetPropertyCount () { return m_Properties.GetCount(); }
		static CString GetPropertyKey (int iIndex) { return m_Properties.GetPropertyName(iIndex); }
		static CDatum GetPropertyType (int iIndex) { return m_Properties.GetPropertyType(iIndex); }
		bool InvokeMethodImpl(const CString& sValue, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult)
			{ return m_Methods.InvokeMethod(sValue, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }

	private:

		static CDatum CreateSplitItem (const char *pStart, const char *pEnd);
		static CDatum ExecuteSplit (const CString& sString, const CString& sDelimiter);
		static CDatum ExecuteSplitByArray (const CString& sString, CDatum dDelimiters);
		static CDatum ExecuteSplitChars (const CString& sString);
		static CDatum ExecuteSplitDoubleLine (const CString& sString);
		static CDatum ExecuteSplitLines (const CString& sString);
		static CDatum ExecuteSplitWhitespace (const CString& sString);
		static bool StartsWith (const char *pSrc, const char *pSrcEnd, const CString& sValue);

		static TDatumPropertyHandler<LPCSTR> m_Properties;
		static TDatumMethodHandler<char> m_Methods;
	};

class CAEONStringFormat : public TExternalDatum<CAEONStringFormat>
	{
	public:

		CAEONStringFormat () { }
		CAEONStringFormat (const CStringFormat& Src) :
				m_Format(Src)
			{ }
		CAEONStringFormat (CStringFormat&& Src) :
				m_Format(std::move(Src))
			{ }

		static CDatum Create (CStringView sFormat) { return CDatum(new CAEONStringFormat(sFormat)); }
		static CDatum CreateAsType (CDatum dValue) { return Create(dValue.AsString()); }
		static TArray<IDatatype::SMemberDesc> GetMembers (void);
		static const CString& StaticGetTypename (void);

		//	IComplexDatum

		virtual CString AsString () const override { return CString(m_Format.GetFormatString()); }
		virtual size_t CalcMemorySize () const override;
		virtual CStringView CastCString () const override { return m_Format.GetFormatString(); }
		virtual const CStringFormat& CastCStringFormat () const override { return m_Format; }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeAEONObject; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString& sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString& sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl(CDatum dObj, const CString& sMethod, IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override 
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual bool IsImmutable () const override { return true; }
		virtual bool IsNil () const override { return m_Format.IsEmpty(); }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return OpCompare(iValueType, dValue); }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override;
		virtual void SetElement (const CString& sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override { return CalcMemorySize() + sizeof(DWORD); }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString& sTypename, IByteStream& Stream) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream& Stream) const override;

	private:

		CAEONStringFormat (CStringView sFormat) :
				m_Format(sFormat)
			{ }

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		CStringFormat m_Format;

		static TDatumPropertyHandler<CAEONStringFormat> m_Properties;
		static TDatumMethodHandler<CAEONStringFormat> m_Methods;
	};

class CAEONTensor : public IComplexDatum
	{
	public:
		CAEONTensor (CDatum dDatatype, CDatum dInitialData = CDatum()) : 
				m_dDatatype(dDatatype),
				m_Dims(CalcDims(dDatatype)),
				m_dElementType(CalcElementType(dDatatype))
			{
			m_bStandardDims = CalcIsStandardDims();
			m_dData = CalcData(dInitialData);
			}

		virtual CString AsString (void) const override { return Format(CStringFormat()); }
		virtual size_t CalcMemorySize (void) const override { return m_dData.CalcMemorySize(); }
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual bool EnumElements (DWORD dwFlags, std::function<bool(CDatum)> fn) const override;
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const override;
		virtual CDatum FindAll (CDatum dValue) const override;
		virtual CDatum FindAllExact (CDatum dValue) const override;
		virtual bool FindExact (CDatum dValue, int *retiIndex = NULL) const override;
		virtual CString Format (const CStringFormat& Format) const override;
		virtual DWORD GetBasicDatatype () const override { return IDatatype::ARRAY; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeTensor; }
		virtual CDatum GetDatatype () const override { return m_dDatatype; }
		virtual int GetCount (void) const override;
		virtual int GetDimensions () const override { return m_Dims.GetCount(); }
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElementAt (int iIndex) const override;
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override;
		virtual CDatum GetElementAt2DA (CDatum dIndex1, CDatum dIndex2) const override;
		virtual CDatum GetElementAt3DA (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3) const override;
		virtual CDatum GetElementAt2DI (int iIndex1, int iIndex2) const override;
		virtual CDatum GetElementAt3DI (int iIndex1, int iIndex2, int iIndex3) const override;
		virtual CDatum GetMethod (const CString &sMethod) const override { if (m_pMethodsExt) return m_pMethodsExt->GetMethod(sMethod); else return CDatum(); }
		virtual const CString &GetTypename (void) const override;
		virtual bool IsArray (void) const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil (void) const override { return false; }
		virtual CDatum IteratorBegin () const override;
		virtual CDatum IteratorNext (CDatum dIterator) const override;
		virtual CDatum MathAbs () const override;
		virtual void MathAccumulateStats (CDatum::SStatsCtx& Stats) const override;
		virtual CDatum MathCeil () const override;
		virtual CDatum MathFloor () const override;
		virtual CDatum MathMax () const override;
		virtual CDatum MathMedian () const override;
		virtual CDatum MathMin () const override;
		virtual CDatum MathRound () const override;
		virtual CDatum MathSign () const override;
		virtual CDatum MathAddToElements (CDatum dValue) const override;
		virtual CDatum MathAddElementsTo (CDatum dValue) const override;
		virtual CDatum MathDivideElementsBy (CDatum dValue) const override;
		virtual CDatum MathDivideByElements (CDatum dValue) const override;
		virtual CDatum MathExpElementsTo (CDatum dValue) const override;
		virtual CDatum MathExpToElements (CDatum dValue) const override;
		virtual CDatum MathInvert () const override;
		virtual CDatum MathMatMul (CDatum dValue) const override;
		virtual CDatum MathModElementsBy (CDatum dValue) const override;
		virtual CDatum MathModByElements (CDatum dValue) const override;
		virtual CDatum MathMultiplyElements (CDatum dValue) const override;
		virtual CDatum MathNegateElements () const override;
		virtual CDatum MathSubtractFromElements (CDatum dValue) const override;
		virtual CDatum MathSubtractElementsFrom (CDatum dValue) const override;
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override;
		virtual CDatum OpConcatenated (IInvokeCtx& Ctx, CDatum dSrc, int iAxis) const override;
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override;
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void SetElement (int iIndex, CDatum dDatum) override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override;
		virtual void SetElementAt2DA (CDatum dIndex1, CDatum dIndex2, CDatum dValue) override;
		virtual void SetElementAt2DI (int iIndex1, int iIndex2, CDatum dValue) override;
		virtual void SetElementAt3DA (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3, CDatum dValue) override;
		virtual void SetElementAt3DI (int iIndex1, int iIndex2, int iIndex3, CDatum dValue) override;

		virtual CDatum raw_IteratorGetElement (CBuffer& Buffer) const override;
		virtual CDatum raw_IteratorGetKey (CBuffer& Buffer) const override;
		virtual bool raw_IteratorHasMore (CBuffer& Buffer) const override;
		virtual void raw_IteratorNext (CBuffer& Buffer) const override;
		virtual void raw_IteratorSetElement (CBuffer& Buffer, CDatum dValue) override;
		virtual CBuffer raw_IteratorStart () const override;

		static CDatum Create2DTensor (int iRows, int iCols, CDatum dElementType, CDatum dData = CDatum());
		static CDatum CreateNTensor (const TArray<int>& Dims, CDatum dElementType, CDatum dData = CDatum());
		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static void SetMethodsExt (TDatumMethodHandler<IComplexDatum> &MethodsExt) { m_pMethodsExt = &MethodsExt; }
		static CDatum ExpandIndexRange (CDatum dArray);

	private:

		struct SDimDesc
			{
			CDatum dType;
			int iOrigin = 0;			//	Origin of the dimension (e.g., for arrays like [-100..100])
			int iLength = 0;			//	Length of the dimension
			int iStride = 0;			//	Stride of the dimension (in m_dData)
			int iFlatStride = 0;		//	Stride of the dimension (in the slice)
			bool bEnum = false;
			};

		class Iterator 
			{
			public:

				Iterator (const CAEONTensor& Tensor) :
						m_Tensor(Tensor)
					{
					for (int i = 0; i < Tensor.m_Dims.GetCount(); i++)
						m_Cursor.Insert(Tensor.m_Dims[i].iOrigin);

					m_iPos = m_Tensor.CalcIndex(m_Cursor);
					}

				Iterator (const CAEONTensor& Tensor, const TArray<int>& Cursor) :
						m_Tensor(Tensor)
					{
					for (int i = 0; i < Tensor.m_Dims.GetCount(); i++)
						m_Cursor.Insert(Min(Max(Tensor.m_Dims[i].iOrigin, Cursor[i]), Tensor.m_Dims[i].iOrigin + Tensor.m_Dims[i].iLength - 1));

					m_iPos = m_Tensor.CalcIndex(m_Cursor);
					}

				Iterator& operator++ ()
					{
					// Increment logic considering strides and dimensions
					for (int i = m_Tensor.m_Dims.GetCount() - 1; i >= 0; --i)
						{
						m_Cursor[i]++;
						if (m_Cursor[i] < m_Tensor.m_Dims[i].iOrigin + m_Tensor.m_Dims[i].iLength)
							{
							m_iPos += m_Tensor.m_Dims[i].iStride;
							return *this;
							}

						m_Cursor[i] = m_Tensor.m_Dims[i].iOrigin;
						if (i == 0)
							{
							m_iPos = -1;
							break;
							}
						else
							m_iPos -= m_Tensor.m_Dims[i].iStride * (m_Tensor.m_Dims[i].iLength - 1);
						}

					return *this;
					}

				CDatum operator* () const { return m_Tensor.m_dData.GetElement(m_iPos); }
				bool operator== (const Iterator& other) const { return m_iPos == other.m_iPos; }
				bool operator!= (const Iterator& other) const { return m_iPos != other.m_iPos; }
				Iterator& operator= (const Iterator& other)
					{
					m_Cursor = other.m_Cursor;
					m_iPos = other.m_iPos;
					return *this;
					}

				static Iterator end (const CAEONTensor& Tensor)
					{
					Iterator result(Tensor);
					result.m_iPos = -1;
					return result;
					}

				int AsFlatIndex () const { return m_Tensor.CalcFlatIndex(m_Cursor); }
				const TArray<int>& AsIndices () const { return m_Cursor; }
				CDatum AsIndexDatum () const
					{
					CDatum dResult(CDatum::typeArray);
					for (int i = 0; i < m_Cursor.GetCount(); i++)
						{
						if (m_Tensor.m_Dims[i].bEnum)
							dResult.Append(CDatum::CreateEnum(m_Cursor[i], m_Tensor.m_Dims[i].dType));
						else
							dResult.Append(m_Cursor[i]);
						}
					return dResult;
					}
				Iterator LastOfDim (int iDim) const
					{
					if (iDim < 0 || iDim >= m_Tensor.m_Dims.GetCount())
						throw CException(errFail);

					Iterator result(*this);
					result.m_Cursor[iDim] = m_Tensor.m_Dims[iDim].iOrigin + m_Tensor.m_Dims[iDim].iLength - 1;
					result.m_iPos = m_Tensor.CalcIndex(result.m_Cursor);
					return result;
					}

				int GetDataIndex () const { return m_iPos; }
				int GetDimCount() const { return m_Tensor.m_Dims.GetCount(); }
				int GetDimLength (int iDim) const
					{
					if (iDim < 0 || iDim >= m_Tensor.m_Dims.GetCount())
						throw CException(errFail);
					return m_Tensor.m_Dims[iDim].iLength;
					}
				void IncDim (int iDim)
					{
					//	Increments along the given dimension. If we are at the end,
					//	we set to the end.

					if (iDim < 0 || iDim >= m_Tensor.m_Dims.GetCount())
						throw CException(errFail);

					if (m_Cursor[iDim] < m_Tensor.m_Dims[iDim].iOrigin + m_Tensor.m_Dims[iDim].iLength - 1)
						{
						m_Cursor[iDim]++;
						m_iPos += m_Tensor.m_Dims[iDim].iStride;
						}
					else
						{
						*this = m_Tensor.end();
						}
					}
				void SetDim (int iDim, int iIndex)
					{
					if (iDim < 0 || iDim >= m_Tensor.m_Dims.GetCount())
						throw CException(errFail);

					if (iIndex < m_Tensor.m_Dims[iDim].iOrigin || iIndex >= m_Tensor.m_Dims[iDim].iOrigin + m_Tensor.m_Dims[iDim].iLength)
						*this = m_Tensor.end();
					else
						{
						m_Cursor[iDim] = iIndex;
						m_iPos = m_Tensor.CalcIndex(m_Cursor);
						}
					}

			private:

				const CAEONTensor& m_Tensor;
				TArray<int> m_Cursor;
				int m_iPos = 0;
			};

		class Iterator1D 
			{
			public:

				Iterator1D (const CAEONTensor& Tensor) :
						m_Tensor(Tensor)
					{
					if (Tensor.m_Dims.GetCount() != 1)
						throw CException(errFail);

					m_iCursor = Tensor.m_Dims[0].iOrigin;
					m_iPos = m_Tensor.CalcIndex1D(m_iCursor);
					}

				Iterator1D (const CAEONTensor& Tensor, int iCursor) :
						m_Tensor(Tensor)
					{
					if (Tensor.m_Dims.GetCount() != 1)
						throw CException(errFail);

					m_iCursor = Min(Max(Tensor.m_Dims[0].iOrigin, iCursor), Tensor.m_Dims[0].iOrigin + Tensor.m_Dims[0].iLength - 1);
					m_iPos = m_Tensor.CalcIndex1D(m_iCursor);
					}

				Iterator1D& operator++ ()
					{
					m_iCursor++;
					if (m_iCursor < m_Tensor.m_Dims[0].iOrigin + m_Tensor.m_Dims[0].iLength)
						{
						m_iPos += m_Tensor.m_Dims[0].iStride;
						return *this;
						}

					m_iPos = -1;
					return *this;
					}

				CDatum operator* () const { return m_Tensor.m_dData.GetElement(m_iPos); }
				bool operator== (const Iterator1D& other) const { return m_iPos == other.m_iPos; }
				bool operator!= (const Iterator1D& other) const { return m_iPos != other.m_iPos; }

				bool isValid () const { return m_iPos != -1; }

				static Iterator1D end (const CAEONTensor& Tensor)
					{
					Iterator1D result(Tensor);
					result.m_iPos = -1;
					return result;
					}

			private:

				const CAEONTensor& m_Tensor;
				int m_iCursor = 0;
				int m_iPos = 0;
			};

		struct SSliceDesc
			{
			bool IsEmpty () const { return dSliceType.IsNil() && iPos == -1; }
			bool IsSingle () const { return iPos != -1; }
			bool IsSlice () const { return !dSliceType.IsNil() && iPos == -1; }

			CDatum dSliceType;				//	Type of the slice
			TArray<CDatum> SliceIndex;		//	Indices for each dimension
			int iPos = -1;					//	If a single element, this is the position in m_dData
			};

		CAEONTensor () { }

		bool AddDim (TArray<CDatum>& ResultDims, int iDim, CDatum dDim) const;
		static void ApplyBatchIndices (Iterator& Pos, const TArray<int>& Indices, int iLowerDims = 2);
		static const CAEONTensor& AsTensor (CDatum dTensor);
		CDatum CalcData (CDatum dInitialData) const;
		CDatum CalcDataFromArray (CDatum dArray) const;
		int CalcDataCount () const;
		int CalcDataSize () const;
		static TArray<SDimDesc> CalcDims (CDatum dType, bool bColumnMajor = false);
		static CDatum CalcElementType (CDatum dType);
		int CalcFlatIndex (const TArray<int>& Indices) const;
		int CalcIndex (const TArray<int>& Indices) const;
		int CalcIndex (CDatum dIndices) const;
		int CalcIndex1D (int iIndex) const;
		int CalcIndex2D (int iIndex1, int iIndex2) const;
		int CalcIndex3D (int iIndex1, int iIndex2, int iIndex3) const;
		static TArray<TArray<int>> CalcIndexPermutations (const TArray<int>& Dims);
		static TArray<int> CalcBroadcastDims (const TArray<int>& Dims1, const TArray<int>& Dims2);
		TArray<int> CalcIndicesFromFlat (int iIndex) const;
		bool CalcIsStandardDims () const;
		SSliceDesc CalcSlice (CDatum dIndex) const;
		SSliceDesc CalcSlice2D (CDatum dIndex1, CDatum dIndex2) const;
		SSliceDesc CalcSlice3D (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3) const;
		static int CompareTensor (const TArray<SDimDesc>& Dims1, int iDataStart1, CDatum dData1, const TArray<SDimDesc>& Dims2, int iDataStart2, CDatum dData2, int iDim);
		static int CompareTensorExact (const TArray<SDimDesc>& Dims1, int iDataStart1, CDatum dData1, const TArray<SDimDesc>& Dims2, int iDataStart2, CDatum dData2, int iDim);
		void FormatOutput (CStringBuffer& Buffer, int iLevel, const Iterator& Pos, const CStringFormat& Format) const;
		CDatum FromDatum (CDatum dValue) const;
		void GetSliceElements (CAEONTensor& Dest, const TArray<CDatum>& SrcIndex, int iSrcLevel, const Iterator& SrcPos, int iDestLevel, const Iterator& DestPos) const;
		bool IsSameSize (const CAEONTensor& Src) const;
		bool IsSquareMatrix () const;
		CDatum MakeNullElement () const { return FromDatum(CDatum()); }
		CDatum OpConcatenatedMinus1 (IInvokeCtx& Ctx, CDatum dSrc, int iAxis) const;
		CDatum OpIdentity () const;
		void SetSliceElements (const CAEONTensor& Src, int iSrcLevel, const Iterator& SrcPos, const TArray<CDatum>& DestIndex, int iDestLevel, const Iterator& DestPos);
		CDatum SqueezeLeadingDims () const;
		CDatum SqueezeTrailingDims () const;
		bool SwapNonZeroRow (CAEONTensor& Dest, int iRow) const;

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual void OnMarked (void) override;

		Iterator begin() const { return Iterator(*this); }
	    Iterator end() const { return Iterator::end(*this); }

		CDatum m_dData;
		int m_iDataStart = 0;
		bool m_bColumnMajor = false;
		bool m_bStandardDims = false;
		TArray<SDimDesc> m_Dims;
		CDatum m_dDatatype;
		CDatum m_dElementType;

		static TDatumPropertyHandler<CAEONTensor> m_Properties;
		static TDatumMethodHandler<IComplexDatum> *m_pMethodsExt;
	};

class CAEONTimeSpan : public IComplexDatum
	{
	public:

		CAEONTimeSpan (void) { }
		CAEONTimeSpan (const CTimeSpan &TimeSpan) : m_TimeSpan(TimeSpan) { }

		virtual CString AsString (void) const override;
		virtual size_t CalcMemorySize (void) const override { return sizeof(CAEONTimeSpan); }
		virtual const CTimeSpan &CastCTimeSpan () const override { return m_TimeSpan; }
		virtual CString Format (const CStringFormat& Format) const override { return Format.FormatTimeSpan(CastCTimeSpan()); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::TIME_SPAN; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeTimeSpan; }
		virtual int GetCount (void) const override { return PART_COUNT; };
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::TIME_SPAN); }
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual CDatum::Types GetNumberType (int *retiValue) override { return CDatum::typeTimeSpan; }
		virtual CDatum GetProperty (const CString& sProperty) const override { return m_Properties.GetProperty(*this, sProperty); }
		virtual const CString &GetTypename (void) const override;
		virtual bool IsArray (void) const override { return false; }
		virtual bool IsImmutable () const override { return true; }
		virtual CDatum MathAbs () const override;
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);

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

		static TDatumPropertyHandler<CAEONTimeSpan> m_Properties;
	};

class CAEONVectorInt32 : public TAEONVector<int, CAEONVectorInt32>
	{
	public:
		CAEONVectorInt32 () { }
		CAEONVectorInt32 (const TArray<int> &Src) : TAEONVector<int, CAEONVectorInt32>(Src) { }
		CAEONVectorInt32 (const TArray<CDatum> &Src) : TAEONVector<int, CAEONVectorInt32>(Src) { }

		static int FromDatum (CDatum dValue) { return (int)dValue; }
		static CDatum MakeNullElement () { return CDatum(0); }
		static CDatum ToDatum (int Value) { return CDatum(Value); }

		virtual int FindMaxElement () const override;
		virtual int FindMinElement () const override;
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY_INT_32); }
		virtual int GetDimensions () const override { return 1; }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override { return TArrayImpl<CAEONVectorInt32, int>::GetElementAt(this, m_Array, dIndex); }
		virtual CDatum GetMethod (const CString &sMethod) const override { if (m_pMethodsExt) return m_pMethodsExt->GetMethod(sMethod); else return CDatum(); }
		virtual const CString &GetTypename (void) const override;
		virtual void InsertEmpty (int iCount) override;
		virtual CDatum IteratorGetKey (CDatum dIterator) const override { return dIterator; };
		virtual CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const override { return TArrayImpl<CAEONVectorInt32, int>::GetElementAt(this, m_Array, dIterator); }
		virtual CDatum MathAbs () const override;
		virtual void MathAccumulateStats (CDatum::SStatsCtx& Stats) const override;
		virtual CDatum MathAverage () const override;
		virtual CDatum MathCeil () const override { return CDatum::raw_AsComplex(this).Clone(); }
		virtual CDatum MathFloor () const override { return CDatum::raw_AsComplex(this).Clone(); }
		virtual CDatum MathMax () const override;
		virtual CDatum MathMin () const override;
		virtual CDatum MathRound () const override { return CDatum::raw_AsComplex(this).Clone(); }
		virtual CDatum MathSign () const override;
		virtual CDatum MathSum () const override;
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::CompareArray(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::CompareArrayExact(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual CDatum OpConcatenated (IInvokeCtx& Ctx, CDatum dSrc, int iAxis = 0) const override { return CAEONOp::ExecConcatenateArray_Array(Ctx, CDatum::raw_AsComplex(this), dSrc, iAxis); }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::IsArrayEqual(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::IsArrayIdentical(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override { TArrayImpl<CAEONVectorInt32, int>::SetElementAt(this, m_Array, dIndex, dDatum); }

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static void SetMethodsExt (TDatumMethodHandler<IComplexDatum> &MethodsExt) { m_pMethodsExt = &MethodsExt; }

	private:

		static TDatumPropertyHandler<CAEONVectorInt32> m_Properties;
		static TDatumMethodHandler<IComplexDatum> *m_pMethodsExt;
	};

class CAEONVectorIntIP : public TAEONVector<CIPInteger, CAEONVectorIntIP>
	{
	public:
		CAEONVectorIntIP () { }
		CAEONVectorIntIP (const TArray<CIPInteger> &Src) : TAEONVector<CIPInteger, CAEONVectorIntIP>(Src) { }
		CAEONVectorIntIP (const TArray<CDatum> &Src) : TAEONVector<CIPInteger, CAEONVectorIntIP>(Src) { }

		static CIPInteger FromDatum (CDatum dValue);
		static CDatum MakeNullElement () { return CDatum(CIPInteger(0)); }
		static CDatum ToDatum (const CIPInteger &Value) { return CDatum(Value); }

		virtual int FindMaxElement () const override;
		virtual int FindMinElement () const override;
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY_INT_IP); }
		virtual int GetDimensions () const override { return 1; }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override { return TArrayImpl<CAEONVectorIntIP, CIPInteger>::GetElementAt(this, m_Array, dIndex); }
		virtual CDatum GetMethod (const CString &sMethod) const override { if (m_pMethodsExt) return m_pMethodsExt->GetMethod(sMethod); else return CDatum(); }
		virtual const CString &GetTypename (void) const override;
		virtual void InsertEmpty (int iCount) override;
		virtual CDatum IteratorGetKey (CDatum dIterator) const override { return dIterator; };
		virtual CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const override { return TArrayImpl<CAEONVectorIntIP, CIPInteger>::GetElementAt(this, m_Array, dIterator); }
		virtual CDatum MathAbs () const override;
		virtual void MathAccumulateStats (CDatum::SStatsCtx& Stats) const override;
		virtual CDatum MathAverage () const override;
		virtual CDatum MathCeil () const override { return CDatum::raw_AsComplex(this).Clone(); }
		virtual CDatum MathFloor () const override { return CDatum::raw_AsComplex(this).Clone(); }
		virtual CDatum MathMax () const override;
		virtual CDatum MathMin () const override;
		virtual CDatum MathRound () const override { return CDatum::raw_AsComplex(this).Clone(); }
		virtual CDatum MathSign () const override;
		virtual CDatum MathSum () const override;
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::CompareArray(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::CompareArrayExact(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual CDatum OpConcatenated (IInvokeCtx& Ctx, CDatum dSrc, int iAxis = 0) const override { return CAEONOp::ExecConcatenateArray_Array(Ctx, CDatum::raw_AsComplex(this), dSrc, iAxis); }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::IsArrayEqual(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::IsArrayIdentical(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override { TArrayImpl<CAEONVectorIntIP, CIPInteger>::SetElementAt(this, m_Array, dIndex, dDatum); }

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static void SetMethodsExt (TDatumMethodHandler<IComplexDatum> &MethodsExt) { m_pMethodsExt = &MethodsExt; }

	private:

		static TDatumPropertyHandler<CAEONVectorIntIP> m_Properties;
		static TDatumMethodHandler<IComplexDatum> *m_pMethodsExt;
	};

class CAEONVectorFloat64 : public TAEONVector<double, CAEONVectorFloat64>
	{
	public:
		CAEONVectorFloat64 () { }
		CAEONVectorFloat64 (const TArray<double> &Src) : TAEONVector<double, CAEONVectorFloat64>(Src) { }
		CAEONVectorFloat64 (const TArray<CDatum> &Src) : TAEONVector<double, CAEONVectorFloat64>(Src) { }

		static double FromDatum (CDatum dValue) { return (double)dValue; }
		static CDatum MakeNullElement () { return CDatum(0.0); }
		static CDatum ToDatum (double Value) { return CDatum(Value); }

		virtual int FindMaxElement () const override;
		virtual int FindMinElement () const override;
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY_FLOAT_64); }
		virtual int GetDimensions () const override { return 1; }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override { return TArrayImpl<CAEONVectorFloat64, double>::GetElementAt(this, m_Array, dIndex); }
		virtual CDatum GetMethod (const CString &sMethod) const override { if (m_pMethodsExt) return m_pMethodsExt->GetMethod(sMethod); else return CDatum(); }
		virtual const CString &GetTypename (void) const override;
		virtual void InsertEmpty (int iCount) override;
		virtual CDatum IteratorGetKey (CDatum dIterator) const override { return dIterator; };
		virtual CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const override { return TArrayImpl<CAEONVectorFloat64, double>::GetElementAt(this, m_Array, dIterator); }
		virtual CDatum MathAbs () const override;
		virtual void MathAccumulateStats (CDatum::SStatsCtx& Stats) const override;
		virtual CDatum MathAverage () const override;
		virtual CDatum MathCeil () const override;
		virtual CDatum MathFloor () const override;
		virtual CDatum MathMax () const override;
		virtual CDatum MathMin () const override;
		virtual CDatum MathRound () const override;
		virtual CDatum MathSign () const override;
		virtual CDatum MathSum () const override;
		virtual CDatum MathAddToElements (CDatum dValue) const override;
		virtual CDatum MathAddElementsTo (CDatum dValue) const override;
		virtual CDatum MathDivideElementsBy (CDatum dValue) const override;
		virtual CDatum MathDivideByElements (CDatum dValue) const override;
		virtual CDatum MathExpElementsTo (CDatum dValue) const override;
		virtual CDatum MathExpToElements (CDatum dValue) const override;
		virtual CDatum MathModElementsBy (CDatum dValue) const override;
		virtual CDatum MathModByElements (CDatum dValue) const override;
		virtual CDatum MathMultiplyElements (CDatum dValue) const override;
		virtual CDatum MathNegateElements () const override;
		virtual CDatum MathSubtractFromElements (CDatum dValue) const override;
		virtual CDatum MathSubtractElementsFrom (CDatum dValue) const override;
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::CompareArray(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::CompareArrayExact(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual CDatum OpConcatenated (IInvokeCtx& Ctx, CDatum dSrc, int iAxis = 0) const override { return CAEONOp::ExecConcatenateArray_Array(Ctx, CDatum::raw_AsComplex(this), dSrc, iAxis); }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::IsArrayEqual(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::IsArrayIdentical(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override { TArrayImpl<CAEONVectorFloat64, double>::SetElementAt(this, m_Array, dIndex, dDatum); }

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static void SetMethodsExt (TDatumMethodHandler<IComplexDatum> &MethodsExt) { m_pMethodsExt = &MethodsExt; }

	private:

		static bool IsFloat (CDatum dValue, double& retrValue);

		static TDatumPropertyHandler<CAEONVectorFloat64> m_Properties;
		static TDatumMethodHandler<IComplexDatum> *m_pMethodsExt;
	};

class CAEONVectorNumber : public TAEONVector<CDatum, CAEONVectorNumber>
	{
	public:
		CAEONVectorNumber () { }
		CAEONVectorNumber (const TArray<CDatum> &Src) : TAEONVector<CDatum, CAEONVectorNumber>(Src) { }

		static CDatum FromDatum (CDatum dValue);
		static CDatum MakeNullElement () { return CDatum(); }
		static CDatum ToDatum (CDatum dValue) { return dValue; }

		virtual int FindMaxElement () const override { return CComplexArray::FindMaxElementInArray(m_Array); }
		virtual int FindMinElement () const override { return CComplexArray::FindMinElementInArray(m_Array); }
		virtual CDatum GetDatatype () const override { return CAEONTypes::Get(IDatatype::ARRAY_NUMBER); }
		virtual int GetDimensions () const override { return 1; }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override { return TArrayImpl<CAEONVectorNumber, CDatum>::GetElementAt(this, m_Array, dIndex); }
		virtual CDatum GetMethod (const CString &sMethod) const override { if (m_pMethodsExt) return m_pMethodsExt->GetMethod(sMethod); else return CDatum(); }
		virtual const CString &GetTypename (void) const override;
		virtual void InsertEmpty (int iCount) override;
		virtual CDatum IteratorGetKey (CDatum dIterator) const override { return dIterator; };
		virtual CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const override { return TArrayImpl<CAEONVectorNumber, CDatum>::GetElementAt(this, m_Array, dIterator); }
		virtual CDatum MathAbs () const override;
		virtual void MathAccumulateStats (CDatum::SStatsCtx& Stats) const override;
		virtual CDatum MathCeil () const override;
		virtual CDatum MathFloor () const override;
		virtual CDatum MathMax () const override { return CComplexArray::CalcMax(m_Array); }
		virtual CDatum MathMedian () const override { return CComplexArray::CalcMedian(m_Array); }
		virtual CDatum MathMin () const override { return CComplexArray::CalcMin(m_Array); }
		virtual CDatum MathRound () const override;
		virtual CDatum MathSign () const override;
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::CompareArray(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::CompareArrayExact(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual CDatum OpConcatenated (IInvokeCtx& Ctx, CDatum dSrc, int iAxis = 0) const override { return CAEONOp::ExecConcatenateArray_Array(Ctx, CDatum::raw_AsComplex(this), dSrc, iAxis); }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::IsArrayEqual(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::IsArrayIdentical(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override { TArrayImpl<CAEONVectorNumber, CDatum>::SetElementAt(this, m_Array, dIndex, dDatum); }
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { m_Array.Sort(pCtx, CDatum::DefaultCompare, Order); }

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static void SetMethodsExt (TDatumMethodHandler<IComplexDatum> &MethodsExt) { m_pMethodsExt = &MethodsExt; }

	private:

		static TDatumPropertyHandler<CAEONVectorNumber> m_Properties;
		static TDatumMethodHandler<IComplexDatum> *m_pMethodsExt;
	};

class CAEONVectorString : public IComplexDatum
	{
	public:
		CAEONVectorString () { }
		CAEONVectorString (const TArray<CString> &Src);
		CAEONVectorString (const TArray<CDatum> &Src) : m_Array(Src) { }

		static CDatum FromDatum (CDatum dValue) { return dValue; }
		static CDatum ToDatum (CStringView sValue) { return CDatum(sValue); }

		virtual void Append (CDatum dDatum) override { if (dDatum.GetBasicType() == CDatum::typeString) m_Array.Insert(dDatum); else m_Array.Insert(CDatum(dDatum.AsString())); }
		virtual CString AsString (void) const override;
		virtual size_t CalcMemorySize (void) const override;
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual void DeleteElement (int iIndex) override { if (iIndex >= 0 && iIndex < m_Array.GetCount()) m_Array.Delete(iIndex); }
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const override;
		virtual CDatum FindAll (CDatum dValue) const override;
		virtual CDatum FindAllExact (CDatum dValue) const override;
		virtual bool FindExact (CDatum dValue, int *retiIndex = NULL) const override;
		virtual int FindMaxElement () const override;
		virtual int FindMinElement () const override;
		virtual DWORD GetBasicDatatype () const override { return IDatatype::ARRAY; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeArray; }
		virtual int GetCount (void) const override { return m_Array.GetCount(); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY_STRING); }
		virtual int GetDimensions () const override { return 1; }
		virtual CDatum GetElement (int iIndex) const override { return ((iIndex >= 0 && iIndex < m_Array.GetCount()) ? m_Array[iIndex] : CDatum()); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElementAt (int iIndex) const override;
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override { return TArrayImpl<CAEONVectorString, CDatum>::GetElementAt(this, m_Array, dIndex); }
		virtual CDatum GetMethod (const CString &sMethod) const override { if (m_pMethodsExt) return m_pMethodsExt->GetMethod(sMethod); else return CDatum(); }
		virtual const CString &GetTypename (void) const override;
		virtual void GrowToFit (int iCount) override { m_Array.GrowToFit(iCount); }
		virtual void InsertElementAt (CDatum dIndex, CDatum dDatum) override;
		virtual void InsertEmpty (int iCount) override { m_Array.InsertEmpty(iCount); }
		virtual bool IsArray (void) const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil (void) const override { return (GetCount() == 0); }
		virtual CDatum IteratorGetKey (CDatum dIterator) const override { return dIterator; };
		virtual CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const override { return TArrayImpl<CAEONVectorString, CDatum>::GetElementAt(this, m_Array, dIterator); }
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool RemoveAll () override { m_Array.DeleteAll(); return true; }
		virtual bool RemoveElementAt (CDatum dIndex) override;
		virtual CDatum MathAbs () const override;
		virtual void MathAccumulateStats (CDatum::SStatsCtx& Stats) const override;
		virtual CDatum MathAverage () const override;
		virtual CDatum MathCeil () const override;
		virtual CDatum MathFloor () const override;
		virtual CDatum MathMax () const override;
		virtual CDatum MathMin () const override;
		virtual CDatum MathRound () const override;
		virtual CDatum MathSign () const override;
		virtual CDatum MathSum () const override;
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::CompareArray(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::CompareArrayExact(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual CDatum OpConcatenated (IInvokeCtx& Ctx, CDatum dSrc, int iAxis = 0) const override { return CAEONOp::ExecConcatenateArray_Array(Ctx, CDatum::raw_AsComplex(this), dSrc, iAxis); }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::IsArrayEqual(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::IsArrayIdentical(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { m_Array.Sort(Order); }
		virtual void SetElement (int iIndex, CDatum dDatum) override { m_Array[iIndex] = (dDatum.GetBasicType() == CDatum::typeString ? dDatum : CDatum(dDatum.AsString())); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override { TArrayImpl<CAEONVectorString, CDatum>::SetElementAt(this, m_Array, dIndex, dDatum); }

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static CDatum DeserializeAEON_v1 (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		CDatum MakeNullElement () const { return CDatum(); }
		static void SetMethodsExt (TDatumMethodHandler<IComplexDatum> &MethodsExt) { m_pMethodsExt = &MethodsExt; }
		CDatum ToDatum (CDatum dValue) const { return dValue; }

	private:

		CDatum AsNumberArray () const;
		bool FindElement (CDatum dValue, int *retiIndex = NULL) const;
		void Insert (CStringView sElement, int iIndex = -1) { m_Array.Insert(sElement, iIndex); }
		void Insert (CDatum dElement, int iIndex = -1) { m_Array.Insert((dElement.GetBasicType() == CDatum::typeString ? dElement : CDatum(dElement.AsString())), iIndex); }
		void InsertEmpty (int iCount = 1, int iIndex = -1) { m_Array.InsertEmpty(iCount, iIndex); }

		virtual void OnMarked (void) override;

		TArray<CDatum> m_Array;

		static TDatumPropertyHandler<CAEONVectorString> m_Properties;
		static TDatumMethodHandler<IComplexDatum> *m_pMethodsExt;
	};

class CAEONVectorTyped : public IComplexDatum
	{
	public:
		CAEONVectorTyped (CDatum dDatatype) : m_dDatatype(dDatatype), m_dElementType(CalcElementType(dDatatype)) { }

		virtual void Append (CDatum dDatum) override { m_Array.Insert(FromDatum(dDatum)); }
		virtual CString AsString (void) const override { return Format(CStringFormat()); }
		virtual size_t CalcMemorySize (void) const override;
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual void DeleteElement (int iIndex) override { if (iIndex >= 0 && iIndex < m_Array.GetCount()) m_Array.Delete(iIndex); }
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const override;
		virtual CDatum FindAll (CDatum dValue) const override;
		virtual CDatum FindAllExact (CDatum dValue) const override;
		virtual bool FindExact (CDatum dValue, int *retiIndex = NULL) const override;
		virtual int FindMaxElement () const override { return CComplexArray::FindMaxElementInArray(m_Array); }
		virtual int FindMinElement () const override { return CComplexArray::FindMinElementInArray(m_Array); }
		virtual CString Format (const CStringFormat &Format) const override;
		virtual CDatum GetArrayElementUnchecked (int iIndex) const override { return m_Array[iIndex]; }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::ARRAY; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeArray; }
		virtual CDatum GetDatatype () const override { return m_dDatatype; }
		virtual int GetDimensions () const override { return 1; }
		virtual int GetCount (void) const override { return m_Array.GetCount(); }
		virtual CDatum GetElement (int iIndex) const override { return ((iIndex >= 0 && iIndex < m_Array.GetCount()) ? m_Array[iIndex] : MakeNullElement()); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElementAt (int iIndex) const override;
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override { return TArrayImpl<CAEONVectorTyped, CDatum>::GetElementAt(this, m_Array, dIndex); }
		virtual CDatum GetMethod (const CString &sMethod) const override { if (m_pMethodsExt) return m_pMethodsExt->GetMethod(sMethod); else return CDatum(); }
		virtual const CString &GetTypename (void) const override;
		virtual void GrowToFit (int iCount) override { m_Array.GrowToFit(iCount); }
		virtual void InsertElementAt (CDatum dIndex, CDatum dDatum) override;
		virtual void InsertEmpty (int iCount) override { m_Array.InsertEmpty(iCount); }
		virtual bool IsArray (void) const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil (void) const override { return (GetCount() == 0); }
		virtual CDatum IteratorGetKey (CDatum dIterator) const override { return dIterator; };
		virtual CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const override { return TArrayImpl<CAEONVectorTyped, CDatum>::GetElementAt(this, m_Array, dIterator); }
		virtual CDatum MathAbs () const override;
		virtual void MathAccumulateStats (CDatum::SStatsCtx& Stats) const override;
		virtual CDatum MathCeil () const override;
		virtual CDatum MathFloor () const override;
		virtual CDatum MathMax () const override { CRecursionGuard Guard(*this); if (Guard.InRecursion()) return CDatum(); return CComplexArray::CalcMax(m_Array); }
		virtual CDatum MathMedian () const override { CRecursionGuard Guard(*this); if (Guard.InRecursion()) return CDatum(); return CComplexArray::CalcMedian(m_Array); }
		virtual CDatum MathMin () const override { CRecursionGuard Guard(*this); if (Guard.InRecursion()) return CDatum(); return CComplexArray::CalcMin(m_Array); }
		virtual CDatum MathRound () const override;
		virtual CDatum MathSign () const override;
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::CompareArray(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::CompareArrayExact(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual CDatum OpConcatenated (IInvokeCtx& Ctx, CDatum dSrc, int iAxis = 0) const override { return CAEONOp::ExecConcatenateArray_Array(Ctx, CDatum::raw_AsComplex(this), dSrc, iAxis); }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::IsArrayEqual(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override { return CComplexArray::IsArrayIdentical(CDatum::raw_AsComplex(this), iValueType, dValue); }
		virtual bool RemoveAll () override;
		virtual bool RemoveElementAt (CDatum dIndex) override;
		virtual void ResolveDatatypes (const CAEONTypeSystem &TypeSystem) override;
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void SetArrayElementUnchecked (int iIndex, CDatum dValue) override { m_Array[iIndex] = dValue; }
		virtual void SetElement (int iIndex, CDatum dDatum) override { m_Array[iIndex] = FromDatum(dDatum); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override { TArrayImpl<CAEONVectorTyped, CDatum>::SetElementAt(this, m_Array, dIndex, dDatum); }
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { m_Array.Sort(pCtx, CDatum::DefaultCompare, Order); }

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		CDatum FromDatum (CDatum dValue) const;
		CDatum MakeNullElement () const;
		static void SetMethodsExt (TDatumMethodHandler<IComplexDatum> &MethodsExt) { m_pMethodsExt = &MethodsExt; }
		CDatum ToDatum (CDatum dValue) const { return dValue; }

	private:

		CAEONVectorTyped () { }
		CAEONVectorTyped (CDatum dDatatype, const TArray<CDatum> &Src) : m_Array(Src), m_dDatatype(dDatatype), m_dElementType(CalcElementType(dDatatype)) { }

		static CDatum CalcElementType (CDatum dType);

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const;
		virtual void OnMarked (void) override;

		TArray<CDatum> m_Array;
		CDatum m_dDatatype;
		CDatum m_dElementType;

		static TDatumPropertyHandler<CAEONVectorTyped> m_Properties;
		static TDatumMethodHandler<IComplexDatum> *m_pMethodsExt;
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
		virtual IComplexDatum* Clone (CDatum::EClone iMode) const override { return new CAEONVector2D(*this); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::VECTOR_2D_F64; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeVector2D; }
		virtual int GetCount (void) const override { return 2; }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::VECTOR_2D_F64); }
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual CDatum GetProperty (const CString& sProperty) const override { return m_Properties.GetProperty(*this, sProperty); }
		virtual bool IsArray (void) const override { return true; }
		virtual bool IsContainer (void) const override { return true; }
		virtual void SetElement (int iIndex, CDatum dDatum) override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static TArray<IDatatype::SMemberDesc> GetMembers (void);

	protected:

		virtual CString AsString () const override;
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		CVector2D m_vVector;

		static TDatumPropertyHandler<CAEONVector2D> m_Properties;
		static TDatumMethodHandler<CAEONVector2D> m_Methods;
	};

class CAEONVector3D : public TExternalDatum<CAEONVector3D>
	{
	public:

		CAEONVector3D () { }
		CAEONVector3D (const CVector3D& vVector);

		const CVector3D& GetVector (void) const { return m_vVector; }

		static const CString& StaticGetTypename (void);

		//	IComplexDatum

		virtual const CVector3D& CastCVector3D () const override { return m_vVector; }
		virtual IComplexDatum* Clone (CDatum::EClone iMode) const override { return new CAEONVector3D(*this); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::VECTOR_3D_F64; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeVector3D; }
		virtual int GetCount (void) const override { return 3; }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::VECTOR_3D_F64); }
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual CDatum GetProperty (const CString& sProperty) const override { return m_Properties.GetProperty(*this, sProperty); }
		virtual bool IsArray (void) const override { return true; }
		virtual bool IsContainer (void) const override { return true; }
		virtual void SetElement (int iIndex, CDatum dDatum) override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static TArray<IDatatype::SMemberDesc> GetMembers (void);

	protected:

		virtual CString AsString () const override;
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		CVector3D m_vVector;

		static TDatumPropertyHandler<CAEONVector3D> m_Properties;
		static TDatumMethodHandler<CAEONVector3D> m_Methods;
	};

