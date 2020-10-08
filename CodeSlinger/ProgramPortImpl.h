//	ProgramPortImpl.h
//
//	CodeSlinger
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CConsolePort : public IProgramPort
	{
	public:
		CConsolePort (CPortSet &PortSet, const CString &sID) : IProgramPort(PortSet, sID),
				m_Buffer(MAX_LINES)
			{ }

	protected:
		virtual CDatum OnGetView (SequenceNumber Seq) const override;
		virtual void OnMark () override;
		virtual void OnOutput (CDatum dValue) override;

	private:
		static constexpr int MAX_LINES = 500;

		struct SEntry
			{
			SequenceNumber Seq = 0;
			CDatum dValue;
			};

		TQueue<SEntry> m_Buffer;
	};

class CNullPort : public IProgramPort
	{
	public:
		CNullPort (CPortSet &PortSet, const CString &sID) : IProgramPort(PortSet, sID)
			{ }
	};

class CValuePort : public IProgramPort
	{
	public:
		CValuePort (CPortSet &PortSet, const CString &sID) : IProgramPort(PortSet, sID)
			{ }

	protected:
		virtual CDatum OnGetView (SequenceNumber Seq) const override
			{ if (m_Seq <= Seq) return CDatum(); else return m_dValue; }

		virtual void OnMark () override { m_dValue.Mark(); }
		virtual void OnOutput (CDatum dValue) override { m_Seq = m_PortSet.AllocSeq(); m_dValue = dValue; }

	private:
		SequenceNumber m_Seq = 0;
		CDatum m_dValue;
	};
