//	AEONInlines.h
//
//	AEON Inline implementations
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#pragma once

//	CDatum

inline CDatum CDatum::GetElementOrDefault (const CString &sKey, CDatum dDefault) const
	{
	CDatum dResult = GetElement(sKey);
	if (dResult.IsNil())
		return dDefault;
	else
		return dResult;
	}

inline CDatum CDatum::raw_GetArrayElement (int iIndex) const
	{
	return DecodeComplex(m_dwData).GetArrayElementUnchecked(iIndex);
	}

inline void CDatum::raw_SetArrayElement (int iIndex, CDatum dValue)
	{
	DecodeComplex(m_dwData).SetArrayElementUnchecked(iIndex, dValue);
	}

inline DWORD CDatum::GetBasicDatatype () const

//	GetBasicDatatype
//
//	Returns a concrete core type that best represents this value. This is meant
//	to be a replacement for ::GetBasicType that works better for GridLang 
//	values. We only return the following values:
//
//	ARRAY					Any array, including multi-dimensional (but not a table)
//	BINARY
//	BOOL					For true/false
//	DATATYPE
//	DATE_TIME
//	ENUM
//	ERROR_T
//	FLOAT_64				Encoded as a double
//	FUNCTION
//	INT_32					Encoded as Int32
//	INT_64
//	INT_IP
//	NAN_CONST				Pseudo-constant NaN
//	NULL_T					For explicit nil (but [] is ARRAY)
//	OBJECT
//	STRING					Encoded as a string
//	STRUCT
//	TABLE
//	TIME_SPAN
//	VECTOR_2D_F64

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return IDatatype::NULL_T;

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
				case VALUE_TRUE:
					return IDatatype::BOOL;

				default:
					ASSERT(false);
					return IDatatype::ANY;
				}
			}

		case TYPE_INT32:
			return IDatatype::INT_32;

		case TYPE_ENUM:
			return IDatatype::ENUM;

		case TYPE_STRING:
			return IDatatype::STRING;

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).GetBasicDatatype();

		case TYPE_ROW_REF:
			return IDatatype::SCHEMA;

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
			//	NOTE: It is OK to interpret these as doubles.
			return IDatatype::FLOAT_64;

		default:
			return IDatatype::FLOAT_64;
		}
	}

//	CHexeLocalEnvironment

inline CDatum CHexeLocalEnvironment::OpAdd (int iIndex, CDatum dValue)
	{
	ASSERT(iIndex < GetAllocSize());
	return (m_pArray[iIndex].dValue = CAEONOp::Add(m_pArray[iIndex].dValue, dValue));
	}
