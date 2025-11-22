//	AEONLines.h
//
//	AEON Line Editor Implementation
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#pragma once

class CAEONLines : public IComplexDatum, public IAEONTextLines
	{
	public:

		CAEONLines () { }

		static CDatum Create (CDatum dValue) { return CDatum(new CAEONLines(dValue)); }

		//	IAEONTextLines

		virtual void ApplyDiff (const CAEONTextLinesDiff& Diff) override;
		virtual const CString& GetLine (int iLine) const override { return (iLine >= 0 && iLine < m_Lines.GetCount() ? m_Lines[iLine] : NULL_STR); }
		virtual int GetLineCount () const override { return m_Lines.GetCount(); }
		virtual SequenceNumber GetSeq () const override { return m_Seq; }
		virtual bool SetLine (int iLine, CStringView sLine) override;
		virtual void SetSeq (SequenceNumber Seq) override { m_Seq = Seq; }

		//	IComplexDatum

		virtual void Append (CDatum dDatum) override;
		virtual CString AsString (void) const override;
		virtual size_t CalcMemorySize (void) const override;
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual void DeleteElement (int iIndex) override;
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const override { throw CException(errFail); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeTextLines; }
		virtual int GetCount (void) const override { return m_Lines.GetCount(); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::TEXT_LINES); }
		virtual int GetDimensions () const override { return 1; }
		virtual CDatum GetElement (int iIndex) const override { if (iIndex >= 0 && iIndex < m_Lines.GetCount()) return m_Lines[iIndex]; else return CDatum(); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override;
		virtual CDatum GetMethod (const CString &sMethod) const override { if (m_pMethodsExt) return m_pMethodsExt->GetMethod(sMethod); else return CDatum(); }
		virtual IAEONTextLines *GetTextLinesInterface () override { return this; }
		virtual const CString &GetTypename (void) const override;
		virtual void GrowToFit (int iCount) override;
		virtual bool IsArray (void) const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil (void) const override { return (GetCount() == 0) || (GetCount() == 1 && m_Lines[0].IsEmpty()); }
		virtual void ResolveDatatypes (const CAEONTypeSystem &TypeSystem) override { }
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { throw CException(errFail); }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void SetElement (int iIndex, CDatum dDatum) override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override;

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static void SetMethodsExt (TDatumMethodHandler<IComplexDatum> &MethodsExt) { m_pMethodsExt = &MethodsExt; }

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnMarked (void) override { }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:

		CAEONLines (CDatum dValue);
		void Insert (TArray<CString>&& Lines, int iIndex = -1);
		void OnModify ();
		TArray<CString> SplitBuffer (const IMemoryBlock& Buffer);

		TArray<CString> m_Lines;
		SequenceNumber m_Seq = 0;

		static TDatumPropertyHandler<CAEONLines> m_Properties;
		static TDatumMethodHandler<IComplexDatum> *m_pMethodsExt;
	};
