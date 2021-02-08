//	CGridLangCodeBank.cpp
//
//	CGridLangCodeBank Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

TArray<CHexeDocument::SEntryPoint> CGridLangCodeBank::GetCodeAsEntryPointTable () const

//	GetCodeAsEntryPointTable
//
//	Returns an array of all symbols and their code.

	{
	TArray<CHexeDocument::SEntryPoint> Result;
	Result.InsertEmpty(m_Map.GetCount());

	for (int i = 0; i < m_Map.GetCount(); i++)
		{
		Result[i].sFunction = m_Map.GetKey(i);
		Result[i].dCode = GetCode(i);
		}

	return Result;
	}

void CGridLangCodeBank::Mark ()

//	Mark
//
//	Mark data in use.

	{
	for (int i = 0; i < m_Map.GetCount(); i++)
		m_Map[i].dCode.Mark();
	}
