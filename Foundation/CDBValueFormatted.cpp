//	CDBValueFormatted.cpp
//
//	CDBValueFormatted class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"
#include "DBValueObjectImpl.h"

CDBValueFormatted::CDBValueFormatted (const CDBValue &Value, const CDBFormatDesc &Format) :
		m_Value(Value),
		m_Format(Format)

//	CDBValueFormatted constructor

	{
	switch (Value.GetType())
		{
		case CDBValue::typeNil:
		case CDBValue::typeInt32:
		case CDBValue::typeInt64:
		case CDBValue::typeDateTime:
		case CDBValue::typeDouble:
		case CDBValue::typeString:
			break;

		default:
			throw CException(errFail);
		}
	}
