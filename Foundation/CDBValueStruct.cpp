//	CDBValueStruct.cpp
//
//	CDBValueStruct class
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"
#include "DBValueObjectImpl.h"

CString CDBValueStruct::AsString (void) const

//	AsString
//
//	Return string representation.

	{
	CStringBuffer Buffer;

	Buffer.WriteChar('{');
	for (int i = 0; i < m_Table.GetCount(); i++)
		{
		if (i != 0)
			Buffer.WriteChar(' ');

		Buffer.Write(m_Table.GetKey(i));
		Buffer.WriteChar(':');
		Buffer.Write(m_Table[i].AsString());
		}

	Buffer.WriteChar('}');

	return CString::CreateFromHandoff(Buffer);
	}
