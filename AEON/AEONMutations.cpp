//	AEONMutations.cpp
//
//	CDatum class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

void CDatum::MutateAdd (int iInc)

//	MutateAdd
//
//	Add the given value to the datum

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			*this = CAEONOp::Add(*this, CDatum(iInc));
			break;

		case TYPE_CONSTANTS:
			//	Cannot increment in place. Nothing happens.
			break;

		case TYPE_INT32:
			{
			LONGLONG iResult = (LONGLONG)DecodeInt32(m_dwData) + (LONGLONG)iInc;
			if (iResult >= INT_MIN && iResult <= INT_MAX)
				m_dwData = EncodeInt32((int)iResult);
			else
				{
				bool bNegative = false;
				if (iResult < 0)
					{
					iResult = -iResult;
					bNegative = true;
					}

				*this = CDatum(CIPInteger(iResult, bNegative));
				}
			break;
			}

		case TYPE_ENUM:
			{
			LONGLONG iResult = (LONGLONG)DecodeEnumValue(m_dwData) + (LONGLONG)iInc;

			//	LATER: Limit based on enum range

			if (iResult >= INT_MIN && iResult <= INT_MAX)
				m_dwData = EncodeInt32((int)iResult);
			else
				{
				bool bNegative = false;
				if (iResult < 0)
					{
					iResult = -iResult;
					bNegative = true;
					}

				*this = CDatum(CIPInteger(iResult, bNegative));
				}
			break;
			}

		case TYPE_STRING:
			*this = CAEONOp::Add(*this, CDatum(iInc));
			break;

		case TYPE_COMPLEX:
			*this = CAEONOp::Add(*this, CDatum(iInc));
			break;

		case TYPE_ROW_REF:
			break;

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
			break;

		default:
			*this = CDatum((double)(DecodeDouble(m_dwData) + iInc));
			break;
		}
	}

