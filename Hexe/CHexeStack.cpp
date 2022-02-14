//	CHexeStack.cpp
//
//	CHexeStack class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

void CHexeStack::Mark (void)

//	Mark
//
//	Mark data in use

	{
	for (int i = 0; i < m_iTop + 1; i++)
		m_Stack[i].Mark();
	}

CDatum CHexeStack::SafeGet (int iIndex) const

//	SafeGet
//
//	Returns the nth element of the stack.

	{
	if (iIndex < 0)
		return CDatum();

	int iEntry = m_iTop - iIndex;
	if (iEntry < 0)
		return CDatum();

	return m_Stack[iEntry];
	}

void CHexeStack::SafePush (CDatum dData)

//	SafePush
//
//	Push the datum

	{
	m_iTop++;

	if (m_iTop >= m_Stack.GetCount())
		m_Stack.InsertEmpty(1000);

	m_Stack[m_iTop] = dData;
	}

