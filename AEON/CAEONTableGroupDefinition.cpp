//	CAEONTableGroupDefinition.cpp
//
//	CAEONTableGroupDefinition class
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CAEONTableGroupDefinition CAEONTableGroupDefinition::DeserializeAEON (IByteStream& Stream, CAEONSerializedMap &Serialized)
	{
	CAEONTableGroupDefinition GroupDef;

	int iCount = Stream.ReadInt();
	for (int i = 0; i < iCount; i++)
		{
		CDatum dExpr = CDatum::DeserializeAEON(Stream, Serialized);
		GroupDef.AddExpression(dExpr.AsExpression());
		}

	return GroupDef;
	}

void CAEONTableGroupDefinition::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	Stream.Write(GetCount());
	for (int i = 0; i < m_Expressions.GetCount(); i++)
		{
		m_Expressions[i].SerializeAEON(Stream, Serialized);
		}
	}
