//	CConsolePort.cpp
//
//	CConsolePort class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

CDatum CConsolePort::OnGetView (SequenceNumber Seq) const

//	OnGetView
//
//	Returns new lines after the given sequence.

	{
	CDatum dResult(CDatum::typeArray);

	for (int i = 0; i < m_Buffer.GetCount(); i++)
		if (m_Buffer[i].Seq > Seq)
			dResult.Append(m_Buffer[i].dValue);

	if (dResult.GetCount() == 0)
		return CDatum();
	else
		return dResult;
	}

void CConsolePort::OnMark ()

//	Mark
//
//	Mark data in use.

	{
	for (int i = 0; i < m_Buffer.GetCount(); i++)
		m_Buffer[i].dValue.Mark();
	}

void CConsolePort::OnOutput (CDatum dValue)

//	OnOutput
//
//	Outputs a new line.

	{
	m_Buffer.EnqueueAndOverwrite({ m_PortSet.AllocSeq(), dValue });
	}
