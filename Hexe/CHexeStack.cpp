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
	int i;

	for (i = 0; i < m_iTop + 1; i++)
		m_Stack[i].Mark();
	}

void CHexeStack::Push (CDatum dData)

//	Push
//
//	Push the datum

	{
	m_iTop++;

	if (m_iTop >= m_Stack.GetCount())
		m_Stack.InsertEmpty(100);

	m_Stack[m_iTop] = dData;
	}
