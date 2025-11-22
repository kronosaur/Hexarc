//	CHexeStackEnv.cpp
//
//	CHexeStackEnv class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CHexeStackEnv::CHexeStackEnv (CDatum* pStart, int iCount) :
		m_pStart(pStart),
		m_iCount(iCount)

//	CHexeStackEnv constructor

	{
	//	Check to see if any parameters need to be expanded.

	for (int i = 0; i < iCount; i++)
		if (m_pStart[i].GetAnnotation().fSpread)
			{
			Expand(pStart, iCount);
			break;
			}
	}

CHexeStackEnv::CHexeStackEnv (TArray<CDatum>&& Stack)

//	CHexeStackEnv constructor

	{
	m_Stack = std::move(Stack);
	m_pStart = (m_Stack.GetCount() > 0 ? &m_Stack[0] : NULL);
	m_iCount = m_Stack.GetCount();
	}

void CHexeStackEnv::AppendArgumentValue (CDatum dValue)

//	AppendArgumentValue
//
//	Adds an argument to the stack env.

	{
	if (m_Stack.GetCount() == 0 && m_pStart)
		throw CException(errFail);

	m_Stack.Insert(dValue);
	m_pStart = &m_Stack[0];
	m_iCount = m_Stack.GetCount();
	}

CDatum CHexeStackEnv::AsDatum () const
	{
	CDatum dResult(CDatum::typeArray);
	for (int i = 0; i < GetCount(); i++)
		dResult.Append(GetElement(i));

	return dResult;
	}

void CHexeStackEnv::Expand (CDatum *pStart, int iCount)
	{
	m_Stack.GrowToFit(iCount);
	for (int i = 0; i < iCount; i++)
		{
		CDatum dValue = pStart[i];
		if (dValue.GetAnnotation().fSpread)
			{
			dValue = dValue.GetElement(0);
			m_Stack.GrowToFit(dValue.GetCount());
			for (int j = 0; j < dValue.GetCount(); j++)
				m_Stack.Insert(dValue.GetElement(j));
			}
		else
			m_Stack.Insert(dValue);
		}

	m_iCount = m_Stack.GetCount();
	m_pStart = (m_iCount > 0 ? &m_Stack[0] : NULL);
	}
