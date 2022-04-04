//	AEONMath.cpp
//
//	CDatum class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

CDatum CDatum::MathAbs () const

//	MathAbs
//
//	Return the absolute value.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			{
			if (m_dwData == 0)
				return CDatum();
			else
				{
				CNumberValue X(*this);
				X.Abs();
				return X.GetDatum();
				}
			}

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData)
						{
						case CONST_NAN:
						case CONST_TRUE:
							return *this;

						default:
							return CreateNaN();
						}
					}

				case AEON_NUMBER_28BIT:
					return CDatum(abs((int)(m_dwData & AEON_NUMBER_MASK) >> 4));

				case AEON_NUMBER_32BIT:
					return CDatum(abs(CAEONStore::GetInt(GetNumberIndex())));

				case AEON_NUMBER_DOUBLE:
					return CDatum(abs(CAEONStore::GetDouble(GetNumberIndex())));

				default:
					return CreateNaN();
				}

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->MathAbs();

		default:
			ASSERT(false);
			return 0;
		}
	}
