//	CHexeFunction.cpp
//
//	CHexeFunction class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_ATTRIBS,						"attributes");
DECLARE_CONST_STRING(FIELD_CACHED,						"cached");
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

void CHexeFunction::CacheInvokeResult (CHexeLocalEnvironment& LocalEnv, CDatum dResult)

//	CacheInvokeResult
//
//	Caches a result.

	{
	if (m_bCached)
		m_Cache.SetAt(LocalEnv.MakeCacheKey(), dResult);
	}

bool CHexeFunction::Contains (CDatum dValue) const

//	Contains
//
//	Returns TRUE if we contain the given value.

	{
	if (m_dLocalEnv.Contains(dValue))
		return true;

	return false;
	}

CDatum CHexeFunction::Create (CDatum dCodeBank, int iCodeOffset, CDatum dGlobalEnv, CDatum dLocalEnv, CDatum dAttribs, CDatum dDatatype)

//	Create
//
//	Creates the datum

	{
	CHexeFunction *pFunc = new CHexeFunction;

	pFunc->m_dHexeCode = dCodeBank;
	pFunc->m_iOffset = iCodeOffset;
	pFunc->m_dDatatype = dDatatype;
	pFunc->m_dAttribs = dAttribs;

	if (pFunc->m_dDatatype.IsNil())
		pFunc->m_dDatatype = CAEONTypes::Get(IDatatype::FUNCTION);

	pFunc->m_pGlobalEnv = CHexeGlobalEnvironment::Upconvert(dGlobalEnv);
	pFunc->m_dGlobalEnv = (pFunc->m_pGlobalEnv ? dGlobalEnv : CDatum());

	pFunc->m_dLocalEnv = dLocalEnv;

	pFunc->m_bCached = dAttribs.GetElement(FIELD_CACHED).AsBool();

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

	return (m_bCached ? CDatum::ECallType::CachedCall : CDatum::ECallType::Call);
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

CDatum::InvokeResult CHexeFunction::Invoke (IInvokeCtx *pCtx, CHexeLocalEnvironment& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult)

//	Invoke
//
//	This call is used to get a cached result.

	{
	CDatum* pResult = m_Cache.GetAt(LocalEnv.MakeCacheKey());
	if (!pResult)
		return CDatum::InvokeResult::unknown;

	retResult.iResult = CDatum::InvokeResult::ok;
	retResult.dResult = pResult->Clone(CDatum::EClone::CopyOnWrite);
	return retResult.iResult;
	}

void CHexeFunction::OnMarked (void)

//	OnMarked
//
//	Mark data in use.

	{
	m_dHexeCode.Mark();
	m_dGlobalEnv.Mark();
	m_dLocalEnv.Mark();
	m_dDatatype.Mark();
	m_dAttribs.Mark();

	for (int i = 0; i < m_Cache.GetCount(); i++)
		m_Cache[i].Mark();
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

			if (dValue.Contains(dSelf))
				continue;

			//	Add

			pLocalEnv->SetArgumentValue(0, i, dValue);
			}

		pStruct->SetElement(FIELD_LOCAL_ENV, CDatum(pLocalEnv));
		}

	//	NOTE: We do not serialize the global environment because we don't know
	//	how. Instead, we should attached it on deserialize.
	}

void CHexeFunction::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized)
	{
	m_dHexeCode = CDatum::DeserializeAEON(Stream, Serialized);

	//	We add a flag on the offset to indicate that we're a new version.

	DWORD dwLoad = Stream.ReadDWORD();
	DWORD dwFlags = 0;
	if (dwLoad & 0x80000000)
		{
		m_iOffset = dwLoad & 0x7fffffff;
		dwFlags = Stream.ReadDWORD();

		m_dDatatype = CDatum::DeserializeAEON(Stream, Serialized);
		}
	else
		{
		m_iOffset = dwLoad;
		m_dDatatype = CAEONTypes::Get(IDatatype::FUNCTION);
		}

	m_dAttribs = CDatum::DeserializeAEON(Stream, Serialized);
	m_dLocalEnv = CDatum::DeserializeAEON(Stream, Serialized);

	m_bCached = m_dAttribs.GetElement(FIELD_CACHED).AsBool();
	}

void CHexeFunction::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const
	{
	m_dHexeCode.SerializeAEON(Stream, Serialized);

	DWORD dwSave = m_iOffset | 0x80000000;	//	Set a flag so we distinguish versions.
	Stream.Write(dwSave);

	DWORD dwFlags = 0;
	Stream.Write(dwFlags);

	m_dDatatype.SerializeAEON(Stream, Serialized);
	m_dAttribs.SerializeAEON(Stream, Serialized);
	m_dLocalEnv.SerializeAEON(Stream, Serialized);

	//	NOTE: We do not serialize the global environment because we don't know
	//	how. Instead, we should attached it on deserialize.
	}

void CHexeFunction::SetElement (const CString &sKey, CDatum dDatum)

//	SetElement
//
//	Sets a comprising element

	{
	if (strEquals(sKey, FIELD_ATTRIBS))
		{
		m_dAttribs = dDatum;
		m_bCached = dDatum.GetElement(FIELD_CACHED).AsBool();
		if (!m_bCached)
			m_Cache.DeleteAll();
		}
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
