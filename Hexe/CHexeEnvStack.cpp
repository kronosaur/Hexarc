//	CHexeEnvStack.cpp
//
//	CHexeEnvStack class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CHexeLocalEnvPointer CHexeEnvStack::AllocEnv (int iArgCount)

//	AllocEnv
//
//	Allocate a new environment.

	{
	if (!m_Free.IsEmpty())
		{
		CHexeLocalEnvPointer NewEnv = m_Free.Pop();
		NewEnv.GetEnv()->Init(iArgCount);
		return NewEnv;
		}
	else
		{
		return CHexeLocalEnvPointer(iArgCount);
		}
	}

void CHexeEnvStack::FreeEnv (CHexeLocalEnvPointer&& pEnv)

//	FreeEnv
//
//	Free an environment for use later.

	{
	if (!pEnv.TrackedByGC())
		m_Free.Push(std::move(pEnv));
	}

void CHexeEnvStack::DeleteAll (void)

//	DeleteAll
//
//	Delete all frames

	{
	for (int i = 0; i < m_Stack.GetCount(); i++)
		{
		SEnvCtx *pFrame = &m_Stack[i];
		SetLocalEnv(*pFrame, CHexeLocalEnvPointer());
		}

	m_Stack.DeleteAll();
	m_dCurGlobalEnv = CDatum();
	m_pCurGlobalEnv = NULL;
	m_pCurLocalEnv.DeleteAll();
	}

CHexeLocalEnvironment& CHexeEnvStack::GetLocalEnvAt (int iLevel) const

//	GetLocalEnvAt
//
//	Returns the local environment at the given level.

	{
	return m_pCurLocalEnv.AsEnv().GetEnvAtLevel(iLevel);
	}

CDatum CHexeEnvStack::GetLocalEnvClosure () const

//	GetLocalEnvClosure
//
//	Returns a closure of all local environments.

	{
	//	If this is already a GC datum, then we're done.

	if (m_pCurLocalEnv.TrackedByGC() || m_pCurLocalEnv.IsEmpty())
		return m_pCurLocalEnv.GetDatum();

	//	We convert the current environment frame to a datum. And we need to make
	//	sure that all its parent frames are also datums, and that we've fixed up
	//	any weak pointers in our stack.

	CDatum dEnv = m_pCurLocalEnv.GetClosure();

	CDatum dParentEnv = m_pCurLocalEnv.AsEnv().GetParentEnvClosure();
	CHexeLocalEnvironment* pParentEnv = m_pCurLocalEnv.AsEnv().GetParentEnv();
	while (pParentEnv)
		{
		//	See if we have any references to this parent. If so, we convert to
		//	a datum.

		for (int i = m_Stack.GetCount() - 1; i >= 0; i--)
			{
			SEnvCtx *pFrame = &m_Stack[i];
			if (pFrame->pLocalEnv == pParentEnv)
				{
				pFrame->dLocalEnv = dParentEnv;
				break;
				}
			}

		//	Next

		dParentEnv = pParentEnv->GetParentEnvClosure();
		pParentEnv = pParentEnv->GetParentEnv();
		}

	//	Done

	return dEnv;
	}

void CHexeEnvStack::Init (CDatum dGlobalEnv)

//	Init
//
//	Initialize with new global environment.

	{
	DeleteAll();

	m_dCurGlobalEnv = dGlobalEnv;
	m_pCurGlobalEnv = CHexeGlobalEnvironment::Upconvert(dGlobalEnv);
	if (m_pCurGlobalEnv == NULL)
		throw CException(errFail);
	}

void CHexeEnvStack::Mark (void)

//	Mark
//
//	Mark data in use

	{
	for (int i = 0; i < m_Stack.GetCount(); i++)
		{
		m_Stack[i].dGlobalEnv.Mark();

		if (m_Stack[i].dLocalEnv.IsIdenticalToTrue())
			{
			//	Since we're not part of the CDatum GC, we need to mark; we clear 
			//	the mark first because otherwise no one will clear it.

			m_Stack[i].pLocalEnv->ClearMark();
			m_Stack[i].pLocalEnv->Mark();
			}
		else
			m_Stack[i].dLocalEnv.Mark();
		}

	for (int i = 0; i < m_Free.GetCount(); i++)
		m_Free[i].Mark();

	m_dCurGlobalEnv.Mark();
	m_pCurLocalEnv.Mark();
	}

void CHexeEnvStack::PopFrame (void)

//	PopFrame
//
//	Restore the previous environment.

	{
	int iFrame = m_Stack.GetCount() - 1;
	if (iFrame < 0)
		throw CException(errFail);
	
	SEnvCtx *pFrame = &m_Stack[iFrame];
	
	m_dCurGlobalEnv = pFrame->dGlobalEnv;
	m_pCurGlobalEnv = pFrame->pGlobalEnv;

	//	If nil, then it means this is a weak pointer. In this case, we expect
	//	m_pCurLocalEnv to be the immediate child frame, which owns its parent,
	//	so we can just transfer ownership.

	if (pFrame->dLocalEnv.IsIdenticalToNil())
		{
		if (pFrame->pLocalEnv == NULL)
			{
			FreeEnv(std::move(m_pCurLocalEnv));
			m_pCurLocalEnv = CHexeLocalEnvPointer();
			}
		else if (m_pCurLocalEnv.AsEnv().GetParentEnv() == pFrame->pLocalEnv)
			{
			CHexeLocalEnvPointer pParent(m_pCurLocalEnv.AsEnv().GetParentEnvHandoff());
			FreeEnv(std::move(m_pCurLocalEnv));
			m_pCurLocalEnv = std::move(pParent);
			}
		else
			throw CException(errFail);
		}

	//	If true, the it means that this is a pointer and we own it.

	else if (pFrame->dLocalEnv.IsIdenticalToTrue())
		{
		FreeEnv(std::move(m_pCurLocalEnv));
		m_pCurLocalEnv = CHexeLocalEnvPointer(pFrame->pLocalEnv);
		}

	//	Otherwise, this is in the GC

	else
		{
		FreeEnv(std::move(m_pCurLocalEnv));
		m_pCurLocalEnv = CHexeLocalEnvPointer(pFrame->dLocalEnv);
		}

	m_Stack.Delete(iFrame);
	}

void CHexeEnvStack::PushNewFrame (int iArgCount)

//	PushNewFrame
//
//	Save the current environment and add a new empty local frame.

	{
	SEnvCtx *pFrame = m_Stack.Insert();
	pFrame->dGlobalEnv = m_dCurGlobalEnv;
	pFrame->pGlobalEnv = m_pCurGlobalEnv;
	SetLocalEnv(*pFrame, std::move(m_pCurLocalEnv));

	m_pCurLocalEnv = AllocEnv(iArgCount);
	}

void CHexeEnvStack::PushNewFrameAsChild (int iArgCount)

//	PushNewFrameAsChild
//
//	Save the current environment and add a new empty local frame.

	{
	//	If the current local environment is already in the GC, then we just
	//	add it.

	if (m_pCurLocalEnv.TrackedByGC())
		{
		CDatum dPrevFrame = m_pCurLocalEnv.GetClosure();

		PushNewFrame(iArgCount);

		//	Link the new frame to the previous one

		m_pCurLocalEnv.AsEnv().SetParentEnv(dPrevFrame);
		}

	//	Otherwise, we create a new local environment and transfer ownership
	//	to it (as its parent).

	else
		{
		CHexeLocalEnvPointer NewEnv = AllocEnv(iArgCount);
		NewEnv.AsEnv().SetParentEnv(std::move(m_pCurLocalEnv));

		SEnvCtx *pFrame = m_Stack.Insert();
		pFrame->dGlobalEnv = m_dCurGlobalEnv;
		pFrame->pGlobalEnv = m_pCurGlobalEnv;
		pFrame->dLocalEnv = CDatum();			//	nil because we don't own it
		pFrame->pLocalEnv = NewEnv.AsEnv().GetParentEnv();	//	Weak pointer.

		m_pCurLocalEnv = std::move(NewEnv);
		}
	}

void CHexeEnvStack::SetGlobalEnv (CDatum dGlobalEnv, CHexeGlobalEnvironment *pGlobalEnv)

//	SetGlobalEnv
//
//	Sets the global environment

	{
	m_dCurGlobalEnv = dGlobalEnv;
	if (pGlobalEnv == NULL)
		{
		m_pCurGlobalEnv = CHexeGlobalEnvironment::Upconvert(dGlobalEnv);
		if (m_pCurGlobalEnv == NULL)
			throw CException(errFail);
		}
	else
		m_pCurGlobalEnv = pGlobalEnv;
	}

void CHexeEnvStack::SetLocalEnv (SEnvCtx& Ctx, const CHexeLocalEnvPointer& pLocalEnv)

//	SetLocalEnv
//
//	Sets the local environment for the given frame.

	{
	if (Ctx.dLocalEnv.IsIdenticalToTrue())
		{
		//	We own the local environment, so we need to delete it.

		delete Ctx.pLocalEnv;
		}

	//	If the local environment is tracked by the GC, then we can just copy the
	//	values.

	if (pLocalEnv.TrackedByGC() || pLocalEnv.IsEmpty())
		{
		Ctx.dLocalEnv = pLocalEnv.GetDatum();
		Ctx.pLocalEnv = pLocalEnv.GetEnv();
		}

	//	Otherwise, this is an error because someone needs to own the local

	else
		{
		throw CException(errFail);
		}
	}

void CHexeEnvStack::SetLocalEnv (SEnvCtx& Ctx, CHexeLocalEnvPointer&& pLocalEnv)

//	SetLocalEnv
//
//	Sets the local environment for the given frame.

	{
	if (Ctx.dLocalEnv.IsIdenticalToTrue())
		{
		//	We own the local environment, so we need to delete it.

		delete Ctx.pLocalEnv;
		}

	//	If the local environment is tracked by the GC, then we can just copy the
	//	values.

	if (pLocalEnv.TrackedByGC() || pLocalEnv.IsEmpty())
		{
		Ctx.dLocalEnv = pLocalEnv.GetDatum();
		Ctx.pLocalEnv = pLocalEnv.GetEnv();
		}

	//	Otherwise, the frame takes ownership of the pointer.

	else
		{
		Ctx.dLocalEnv = CDatum(true);
		Ctx.pLocalEnv = pLocalEnv.GetHandoff();
		}
	}

void CHexeEnvStack::SetLocalEnvParent (CDatum dLocalEnv)

//	SetLocalEnvParent
//
//	Sets the parent of the current local environment

	{
	m_pCurLocalEnv.AsEnv().SetParentEnv(dLocalEnv);
	}

