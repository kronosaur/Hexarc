//	CHexeFunction.cpp
//
//	CHexeFunction class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_ATTRIBS,						"attributes");
DECLARE_CONST_STRING(FIELD_GLOBAL_ENV,					"globalEnv");
DECLARE_CONST_STRING(FIELD_HEXE_CODE,					"hexeCode");
DECLARE_CONST_STRING(FIELD_LOCAL_ENV,					"localEnv");
DECLARE_CONST_STRING(FIELD_OFFSETX,						"offset");	//	NOTE: Unable to use FIELD_OFFSET as name

DECLARE_CONST_STRING(TYPENAME_HEXE_FUNCTION,			"hexeFunction");
const CString &CHexeFunction::StaticGetTypename (void) { return TYPENAME_HEXE_FUNCTION; }

const int COMPRISING_ATTRIBS =							0;
const int COMPRISING_GLOBAL_ENV =						1;
const int COMPRISING_HEXE_CODE =						2;
const int COMPRISING_LOCAL_ENV =						3;
const int COMPRISING_OFFSET =							4;
const int COMPRISING_COUNT =							5;

bool CHexeFunction::Contains (CDatum dValue, TArray<IComplexDatum *> &retChecked) const

//	Contains
//
//	Returns TRUE if we contain the given value.

	{
	if (m_dLocalEnv.Contains(dValue, retChecked))
		return true;

	return false;
	}

CDatum CHexeFunction::Create (CDatum dCodeBank, int iCodeOffset, CDatum dGlobalEnv, CDatum dLocalEnv, CDatum dAttribs)

//	Create
//
//	Creates the datum

	{
	CHexeFunction *pFunc = new CHexeFunction;

	pFunc->m_dHexeCode = dCodeBank;
	pFunc->m_iOffset = iCodeOffset;
	pFunc->m_dAttribs = dAttribs;

	pFunc->m_pGlobalEnv = CHexeGlobalEnvironment::Upconvert(dGlobalEnv);
	pFunc->m_dGlobalEnv = (pFunc->m_pGlobalEnv ? dGlobalEnv : CDatum());

	pFunc->m_dLocalEnv = dLocalEnv;

	return CDatum(pFunc);
	}

CDatum::ECallType CHexeFunction::GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const

//	GetCallInfo
//
//	Returns info required to call this function through the
//	compute process.
//
//	If the function returns FALSE, then this function does not
//	change the instruction pointer.

	{
	CHexeCode *pCodeBank = CHexeCode::Upconvert(m_dHexeCode);
	if (pCodeBank == NULL)
		return CDatum::ECallType::None;

	if (retdCodeBank)
		*retdCodeBank = m_dHexeCode;

	if (retpIP)
		*retpIP = pCodeBank->GetCode(m_iOffset);

	return CDatum::ECallType::Call;
	}

DWORD *CHexeFunction::GetCode (CDatum *retdCodeBank)

//	GetCode
//
//	Returns the a pointer to the code for this expression

	{
	if (retdCodeBank)
		*retdCodeBank = m_dHexeCode;

	CHexeCode *pCodeBank = CHexeCode::Upconvert(m_dHexeCode);
	if (pCodeBank == NULL)
		return NULL;

	return pCodeBank->GetCode(m_iOffset);
	}

int CHexeFunction::GetCount (void) const

//	GetCount
//
//	Returns number comprising elements

	{
	return COMPRISING_COUNT;
	}

CDatum CHexeFunction::GetElement (int iIndex) const

//	GetElement
//
//	Returns comprising elements

	{
	switch (iIndex)
		{
		case COMPRISING_ATTRIBS:
			return m_dAttribs;

		case COMPRISING_GLOBAL_ENV:
			return m_dGlobalEnv;

		case COMPRISING_HEXE_CODE:
			return m_dHexeCode;

		case COMPRISING_OFFSET:
			return CDatum(m_iOffset);

		case COMPRISING_LOCAL_ENV:
			return m_dLocalEnv;

		default:
			return CDatum();
		}
	}

CDatum CHexeFunction::GetElement (const CString &sKey) const

//	GetElement
//
//	Returns comprising elements

	{
	if (strEquals(sKey, FIELD_ATTRIBS))
		return m_dAttribs;
	else if (strEquals(sKey, FIELD_GLOBAL_ENV))
		return m_dGlobalEnv;
	else if (strEquals(sKey, FIELD_HEXE_CODE))
		return m_dHexeCode;
	else if (strEquals(sKey, FIELD_OFFSETX))
		return CDatum(m_iOffset);
	else if (strEquals(sKey, FIELD_LOCAL_ENV))
		return m_dLocalEnv;
	else
		return CDatum();
	}

CString CHexeFunction::GetKey (int iIndex) const

//	GetKey
//
//	Returns the key of a comprising element.

	{
	switch (iIndex)
		{
		case COMPRISING_ATTRIBS:
			return FIELD_ATTRIBS;

		case COMPRISING_GLOBAL_ENV:
			return FIELD_GLOBAL_ENV;

		case COMPRISING_HEXE_CODE:
			return FIELD_HEXE_CODE;

		case COMPRISING_OFFSET:
			return FIELD_OFFSETX;

		case COMPRISING_LOCAL_ENV:
			return FIELD_LOCAL_ENV;

		default:
			return NULL_STR;
		}
	}

void CHexeFunction::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize the function.

	{
	//	Serialize code

	pStruct->SetElement(FIELD_HEXE_CODE, m_dHexeCode);
	pStruct->SetElement(FIELD_OFFSETX, m_iOffset);
	pStruct->SetElement(FIELD_ATTRIBS, m_dAttribs);

	//	Serialize local environment.
	//	NOTE: We only serialize a single level. In the future, we should be 
	//	smarter about only saving variables used by the function (in a closure).

	if (CHexeLocalEnvironment *pCurrentLocalEnv = CHexeLocalEnvironment::Upconvert(m_dLocalEnv))
		{
		CHexeLocalEnvironment *pLocalEnv = new CHexeLocalEnvironment;
		pLocalEnv->SetNextArg(pCurrentLocalEnv->GetNextArg());

		CDatum dSelf(CDatum::raw_AsComplex(this));

		for (int i = 0; i < m_dLocalEnv.GetCount(); i++)
			{
			pLocalEnv->SetArgumentKey(0, i, m_dLocalEnv.GetKey(i));
			CDatum dValue = m_dLocalEnv.GetElement(i);

			CString sKey = m_dLocalEnv.GetKey(i);

			//	Make sure we don't try to save ourselves.

			TArray<IComplexDatum *> Checked;
			if (dValue.Contains(dSelf, Checked))
				continue;

			//	Add

			pLocalEnv->SetArgumentValue(0, i, dValue);
			}

		pStruct->SetElement(FIELD_LOCAL_ENV, CDatum(pLocalEnv));
		}

	//	NOTE: We do not serialize the global environment because we don't know
	//	how. Instead, we should attached it on deserialize.
	}

void CHexeFunction::SetElement (const CString &sKey, CDatum dDatum)

//	SetElement
//
//	Sets a comprising element

	{
	if (strEquals(sKey, FIELD_ATTRIBS))
		m_dAttribs = dDatum;
	else if (strEquals(sKey, FIELD_GLOBAL_ENV))
		{
		m_pGlobalEnv = CHexeGlobalEnvironment::Upconvert(dDatum);
		m_dGlobalEnv = (m_pGlobalEnv ? dDatum : CDatum());
		}
	else if (strEquals(sKey, FIELD_HEXE_CODE))
		m_dHexeCode = dDatum;
	else if (strEquals(sKey, FIELD_OFFSETX))
		m_iOffset = dDatum;
	else if (strEquals(sKey, FIELD_LOCAL_ENV))
		m_dLocalEnv = dDatum;
	}
