//	AEONLines.h
//
//	AEON Line Editor Implementation
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CAEONLines : public IComplexDatum, public IAEONTextLines
	{
	public:

		CAEONLines () { }

		static CDatum Create (CDatum dValue) { return CDatum(new CAEONLines(dValue)); }

		//	IAEONTextLines

		virtual void ApplyDiff (const CAEONTextLinesDiff& Diff) override;
		virtual const CString& GetLine (int iLine) const { return (iLine >= 0 && iLine < m_Lines.GetCount() ? m_Lines[iLine] : NULL_STR); }
		virtual int GetLineCount () const { return m_Lines.GetCount(); }
		virtual SequenceNumber GetSeq () const override { return m_Seq; }
		virtual void SetSeq (SequenceNumber Seq) override { m_Seq = Seq; }

		//	IComplexDatum

		virtual void Append (CDatum dDatum) override;
		virtual CString AsString (void) const override;
		virtual size_t CalcMemorySize (void) const override;
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual void DeleteElement (int iIndex) override;
		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const override { throw CException(errFail); }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeTextLines; }
		virtual int GetCount (void) const override { return m_Lines.GetCount(); }
		virtual CDatum GetDatatype () const override { return CAEONTypes::Get(IDatatype::TEXT_LINES); }
		virtual CDatum GetElement (int iIndex) const override { if (iIndex >= 0 && iIndex < m_Lines.GetCount()) return m_Lines[iIndex]; else return CDatum(); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const override;
		virtual IAEONTextLines *GetTextLinesInterface () override { return this; }
		virtual const CString &GetTypename (void) const override;
		virtual void GrowToFit (int iCount) override;
		virtual bool IsArray (void) const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil (void) const override { return (GetCount() == 0) || (GetCount() == 1 && m_Lines[0].IsEmpty()); }
		virtual void ResolveDatatypes (const CAEONTypeSystem &TypeSystem) override { }
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { throw CException(errFail); }
		virtual void SetElement (int iIndex, CDatum dDatum) override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }
		virtual void SetElementAt (CDatum dIndex, CDatum dDatum) override;

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
	};
