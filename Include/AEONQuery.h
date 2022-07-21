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

			EqualTo,
			GreaterThan,
			GreaterThanOrEqualTo,
			In,
			LessThan,
			LessThanOrEqualTo,

			And,
			Or,
			};

		static CDatum And (CDatum dLeft, CDatum dRight);
		static CDatum EqualTo (const CString& sField, CDatum dValue);
		static CDatum GreaterThan (const CString& sField, CDatum dValue);
		static CDatum GreaterThanOrEqualTo (const CString& sField, CDatum dValue);
		static CDatum In (const CString& sField, CDatum dValue);
		static CDatum LessThan (const CString& sField, CDatum dValue);
		static CDatum LessThanOrEqualTo (const CString& sField, CDatum dValue);
		static CDatum Or (CDatum dLeft, CDatum dRight);

		EOp GetOp () const { return m_iOp; }
		const CString& GetField () const { return m_dLeft; }
		CDatum GetLeft () const { return m_dLeft; }
		CDatum GetRight () const { return m_dRight; }
		CDatum GetValue () const { return m_dRight; }
		void Mark () { m_dLeft.Mark(); m_dRight.Mark(); }

	private:

		EOp m_iOp = EOp::None;
		CDatum m_dLeft;
		CDatum m_dRight;
	};

