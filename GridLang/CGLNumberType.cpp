//	CGLNumberType.cpp
//
//	CGLNumberType Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

CGLNumberType::CGLNumberType (const IGLType *pParent, const IGLType *pScope, const CString &sName, GLCoreType iType) : IGLType(pParent, pScope, sName)

//	CGLNumberType constructor

	{
	switch (iType)
		{
		case GLCoreType::Int32:
			m_iBits = 32;
			m_bFloat = false;
			m_bUnsigned = false;
			break;

		case GLCoreType::Int64:
			m_iBits = 64;
			m_bFloat = false;
			m_bUnsigned = false;
			break;

		case GLCoreType::IntIP:
			m_iBits = -1;
			m_bFloat = false;
			m_bUnsigned = false;
			break;

		case GLCoreType::UInt32:
			m_iBits = 32;
			m_bFloat = false;
			m_bUnsigned = true;
			break;

		case GLCoreType::UInt64:
			m_iBits = 64;
			m_bFloat = false;
			m_bUnsigned = true;
			break;

		case GLCoreType::Float64:
			m_iBits = 64;
			m_bFloat = true;
			m_bUnsigned = false;
			break;

		default:
			throw CException(errFail);
		}
	}
