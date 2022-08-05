//	AEONQuery.h
//
//	AEON Query
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CAEONQuery : public IComplexDatum
	{
	public:

		enum class EOp
			{
			None,

			True,
			False,
			Literal,
			Field,

			EqualTo,
			GreaterThan,
			GreaterThanOrEqualTo,
			In,
			LessThan,
			LessThanOrEqualTo,

			And,
			Or,
			};

		static CDatum BinaryOp (EOp iOp, CDatum dLeft, CDatum dRight);
		static CDatum False () { return CDatum(new CAEONQuery(EOp::False)); }
		static CDatum Field (const CString& sField) { return CDatum(new CAEONQuery(EOp::Field, CDatum(sField))); }
		static CDatum Literal (CDatum dValue) { return CDatum(new CAEONQuery(EOp::Literal, dValue)); }
		static CDatum True () { return CDatum(new CAEONQuery(EOp::True)); }

		EOp GetOp () const { return m_iOp; }
		CDatum GetLeft () const { return m_dLeft; }
		CDatum GetRight () const { return m_dRight; }

		//	IComplexDatum

		virtual CString AsString (void) const override;
		virtual size_t CalcMemorySize (void) const override;
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeQuery; }
		virtual int GetCount (void) const override { return 0; }
		virtual CDatum GetElement (int iIndex) const override { return CDatum(); }
		virtual const CAEONQuery* GetQueryInterface () const override { return this; }
		virtual const CString &GetTypename (void) const override;
		virtual bool IsArray (void) const override { return false; }
		virtual bool IsNil (void) const override { return (m_iOp == EOp::None); }

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override { m_dLeft.Mark(); m_dRight.Mark(); }
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		CAEONQuery (EOp iOp = EOp::None, CDatum dLeft = CDatum(), CDatum dRight = CDatum()) :
				m_iOp(iOp),
				m_dLeft(dLeft),
				m_dRight(dRight)
			{ }

		static CString AsID (EOp iOp);
		static EOp AsOp (const CString& sValue);

		EOp m_iOp = EOp::None;
		CDatum m_dLeft;
		CDatum m_dRight;
	};
