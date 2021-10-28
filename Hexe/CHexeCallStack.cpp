//	CHexeCallStack.cpp
//
//	CHexeCallStack class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

void CHexeCallStack::Mark (void)

//	Mark
//
//	Mark data in use

	{
	int i;

	for (i = 0; i < m_Stack.GetCount(); i++)
		{
		m_Stack[i].dCodeBank.Mark();
		m_Stack[i].dExpression.Mark();
		}
	}

void CHexeCallStack::Restore (CDatum *retdExpression, 
							  CDatum *retdCodeBank, 
							  DWORD **retpIP, 
							  CHexeCode **retpCodeBank)

//	Restore
//
//	Restore the position

	{
	int iFrame = m_Stack.GetCount() - 1;
	if (iFrame < 0)
		throw CException(errFail);

	SExecuteCtx *pFrame = &m_Stack[iFrame];

	*retdExpression = pFrame->dExpression;
	*retdCodeBank = pFrame->dCodeBank;
	*retpIP = pFrame->pIP;
	*retpCodeBank = CHexeCode::Upconvert(pFrame->dCodeBank);

	m_Stack.Delete(iFrame);
	}

void CHexeCallStack::Save (CDatum dExpression, CDatum dCodeBank, DWORD *pIP)

//	Save
//
//	Save the position

	{
	SExecuteCtx *pFrame = m_Stack.Insert();
	pFrame->dExpression = dExpression;
	pFrame->dCodeBank = dCodeBank;
	pFrame->pIP = pIP;
	}

//	CHexeEnvStack --------------------------------------------------------------

void CHexeEnvStack::Mark (void)

//	Mark
//
//	Mark data in use

	{
	int i;

	for (i = 0; i < m_Stack.GetCount(); i++)
		{
		m_Stack[i].dGlobalEnv.Mark();
		m_Stack[i].dLocalEnv.Mark();
		}
	}

void CHexeEnvStack::Restore (CDatum *retdGlobalEnv, CHexeGlobalEnvironment **retpGlobalEnv, CDatum *retdLocalEnv, CHexeLocalEnvironment **retpLocalEnv)

//	Restore
//
//	Restore

	{
	int iFrame = m_Stack.GetCount() - 1;
	if (iFrame < 0)
		throw CException(errFail);

	SEnvCtx *pFrame = &m_Stack[iFrame];

	*retdGlobalEnv = pFrame->dGlobalEnv;
	*retpGlobalEnv = pFrame->pGlobalEnv;
	*retdLocalEnv = pFrame->dLocalEnv;
	*retpLocalEnv = pFrame->pLocalEnv;

	m_Stack.Delete(iFrame);
	}

void CHexeEnvStack::Save (CDatum dGlobalEnv, CHexeGlobalEnvironment *pGlobalEnv, CDatum dLocalEnv, CHexeLocalEnvironment *pLocalEnv)

//	Save
//
//	Save

	{
	SEnvCtx *pFrame = m_Stack.Insert();
	pFrame->dGlobalEnv = dGlobalEnv;
	pFrame->pGlobalEnv = pGlobalEnv;
	pFrame->dLocalEnv = dLocalEnv;
	pFrame->pLocalEnv = pLocalEnv;
	}


