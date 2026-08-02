//	CHexeCallStack.cpp
//
//	CHexeCallStack class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CHexeCallStack::CHexeCallStack ()

//	CHexeCallStack constructor

	{
	m_Stack.GrowToFit(DEFAULT_STACK_SIZE);
	}

void CHexeCallStack::Mark (void)

//	Mark
//
//	Mark data in use

	{
	DEBUG_TRY

	for (int i = 0; i < m_Stack.GetCount(); i++)
		{
		m_Stack[i].dCodeBank.Mark();
		m_Stack[i].dExpression.Mark();
		m_Stack[i].dPrimitive.Mark();
		m_Stack[i].dContext.Mark();
		}

	DEBUG_CATCH
	}

const CHexeCallStack::SFrame& CHexeCallStack::Top () const
	{
	int iFrame = m_Stack.GetCount() - 1;
	if (iFrame < 0)
		throw CException(errFail);

	return m_Stack[iFrame];
	}

void CHexeCallStack::PushFunCall (CDatum dExpression, CDatum dCodeBank, DWORD* pIP)
	{
	SFrame* pFrame = m_Stack.Insert();
	pFrame->dwType = FUNC_CALL;
	pFrame->dwFlags = 0;
	pFrame->pIP = pIP;
	pFrame->dExpression = dExpression;
	pFrame->dCodeBank = dCodeBank;
	}

void CHexeCallStack::PushStackFrameCall (CDatum dExpression, CDatum dCodeBank, DWORD* pIP, int iPrevFrameBase, int iPrevStackTop, int iFrameBase)
	{
	SFrame* pFrame = m_Stack.Insert();
	pFrame->dwType = STACK_FRAME_CALL;
	pFrame->dwFlags = 0;
	pFrame->pIP = pIP;
	pFrame->dExpression = dExpression;
	pFrame->dCodeBank = dCodeBank;
	pFrame->iPrevFrameBase = iPrevFrameBase;
	pFrame->iPrevStackTop = iPrevStackTop;
	pFrame->iFrameBase = iFrameBase;
	}

void CHexeCallStack::PushSysCall (CDatum dExpression, CDatum dCodeBank, DWORD* pIP, CDatum dPrimitive, CDatum dContext, DWORD dwFlags)
	{
	SFrame* pFrame = m_Stack.Insert();
	pFrame->dwType = SYSTEM_CALL;
	pFrame->dwFlags = dwFlags;
	pFrame->pIP = pIP;
	pFrame->dExpression = dExpression;
	pFrame->dCodeBank = dCodeBank;
	pFrame->dPrimitive = dPrimitive;
	pFrame->dContext = dContext;
	}

void CHexeCallStack::SetTopStackFrame (int iPrevFrameBase, int iPrevStackTop, int iFrameBase)
	{
	int iFrame = m_Stack.GetCount() - 1;
	if (iFrame < 0)
		throw CException(errFail);

	SFrame& Frame = m_Stack[iFrame];
	Frame.dwType = STACK_FRAME_CALL;
	Frame.dwFlags |= FLAG_STACK_FRAME_HAS_ENV;
	Frame.iPrevFrameBase = iPrevFrameBase;
	Frame.iPrevStackTop = iPrevStackTop;
	Frame.iFrameBase = iFrameBase;
	}

void CHexeCallStack::Pop ()
	{
	int iFrame = m_Stack.GetCount() - 1;
	if (iFrame < 0)
		throw CException(errFail);

	m_Stack.Delete(iFrame);
	}
