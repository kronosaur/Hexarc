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

				case VALUE_BLANK:
					return IDatatype::STRING;

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

	CDatum& dLocal = m_pArray[iIndex].dValue;

	if (dLocal.raw_IsInt32() && dValue.raw_IsInt32())
		{
		LONGLONG iResult = (LONGLONG)dLocal.raw_GetInt32() + (LONGLONG)dValue.raw_GetInt32();
		if (iResult >= INT_MIN && iResult <= INT_MAX)
			{
			dLocal = CDatum((int)iResult);
			return dLocal;
			}
		}

	return (dLocal = CAEONOp::Add(dLocal, dValue));
	}

inline void CHexeLocalEnvironment::OpInc (int iIndex, int iInc)
	{
	ASSERT(iIndex < GetAllocSize());

	CDatum& dLocal = m_pArray[iIndex].dValue;

	if (dLocal.raw_IsInt32())
		{
		int iValue = dLocal.raw_GetInt32();

		//	Keep this checked int32 fast path local instead of folding it into
		//	CDatum::MutateAdd. On the 600M GLDiagnostics loop, a shared helper
		//	regressed Release from ~5.8s to ~6.4s.
		if (iInc == 1)
			{
			if (iValue < INT_MAX)
				{
				dLocal.MutateAddInt32(1);
				return;
				}
			}
		else if (iInc == -1)
			{
			if (iValue > INT_MIN)
				{
				dLocal.MutateAddInt32(-1);
				return;
				}
			}
		else
			{
			LONGLONG iResult = (LONGLONG)iValue + (LONGLONG)iInc;
			if (iResult >= INT_MIN && iResult <= INT_MAX)
				{
				dLocal.MutateAddInt32(iInc);
				return;
				}
			}
		}

	dLocal.MutateAdd(iInc);
	}

inline void CHexeLocalEnvironment::OpInc1 (int iIndex)
	{
	ASSERT(iIndex < GetAllocSize());

	CDatum& dLocal = m_pArray[iIndex].dValue;

	if (dLocal.raw_IsInt32() && dLocal.raw_GetInt32() < INT_MAX)
		{
		dLocal.MutateAddInt32(1);
		return;
		}

	dLocal.MutateAdd(1);
	}

inline DWORD CDatum::GetBasicDatatypeEx () const

//	GetBasicDatatypeEx
//
//	Like GetBasicDatatype, but returns NAN_CONST for any IEEE-754 NaN payload.

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

				case VALUE_BLANK:
					return IDatatype::NULL_T;

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
			return IDatatype::NAN_CONST;

		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
			return IDatatype::FLOAT_64;

		default:
			return (IsIEEE754NaNBits(m_dwData) ? IDatatype::NAN_CONST : IDatatype::FLOAT_64);
		}
	}
