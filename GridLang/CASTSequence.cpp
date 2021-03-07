//	CASTSequence.cpp
//
//	CASTSequence Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(ERR_DUPLICATE_DEFINITION,			"Duplicate definition: %s");

TSharedPtr<IASTNode> CASTSequence::Create (TArray<TSharedPtr<IASTNode>> Nodes, CString *retsError)

//	Create
//
//	Creates the sequence.

	{
	CASTSequence *pSeq = new CASTSequence;
	TSharedPtr<IASTNode> pResult(pSeq);

	pSeq->m_Node = std::move(Nodes);

	//	Keep an index of definitions

	for (int i = 0; i < pSeq->m_Node.GetCount(); i++)
		{
		IASTNode &Node = *pSeq->m_Node[i];

		//	If this is a definition, then add to the map.

		if (Node.IsDefinition())
			{
			bool bNew;
			pSeq->m_Types.SetAt(strToLower(Node.GetName()), &Node, &bNew);
			if (!bNew)
				{
				if (retsError) *retsError = strPattern(ERR_DUPLICATE_DEFINITION, Node.GetName());
				return NULL;
				}
			}

		//	If this is a statement, then add to the ordered list of statements.
		//	NOTE: It is possible to be in both (e.g., var s = 1).

		if (Node.IsStatement())
			{
			pSeq->m_Statements.Insert(&Node);
			}
		}

	return pResult;
	}

void CASTSequence::DebugDump (const CString &sIndent) const 
	{
	for (int i = 0; i < m_Node.GetCount(); i++)
		{
		m_Node[i]->DebugDump(sIndent);
		}
	}

const IASTNode *CASTSequence::FindDefinition (const CString &sID) const

//	FindDefinition
//
//	Finds the given definition by ID. Returns NULL if not found.

	{
	auto pSlot = m_Types.GetAt(sID);
	if (!pSlot)
		return NULL;

	return *pSlot;
	}