//	CDBValueArray.cpp
//
//	CDBValueArray class
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"
#include "DBValueObjectImpl.h"

CString CDBValueArray::AsString (void) const

//	AsString
//
//	Return string representation.

	{
	CStringBuffer Buffer;

	Buffer.WriteChar('[');
	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		if (i != 0)
			Buffer.WriteChar(',');

		Buffer.Write(m_Array[i].AsString());
		}

	Buffer.WriteChar(']');

	return CString::CreateFromHandoff(Buffer);
	}
