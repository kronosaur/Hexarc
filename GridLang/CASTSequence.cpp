//	CASTSequence.cpp
//
//	CASTSequence Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(ERR_DUPLICATE_DEFINITION,			"Duplicate definition: %s");

bool CASTSequence::AddChild (const TArray<TSharedPtr<IASTNode>> &Nodes, CString *retsError)

//	AddChild
//
//	Adds all the nodes as children.

	{
	for (int i = 0; i < Nodes.GetCount(); i++)
		{
		m_Node.Insert(Nodes[i]);

		if (!AddToIndex(*Nodes[i], retsError))
			return false;
		}

	return true;
	}

bool CASTSequence::AddToIndex (IASTNode &Node, CString *retsError)

//	AddToIndex
//
//	Adds the node to the index. We assume that the node is already in our list
//	of nodes.

	{
	//	If this is a definition, then add to the map.

	if (Node.IsFunctionDefinition())
		{
		if (!AddSymbol(Node, retsError))
			return false;

		m_Functions.SetAt(strToLower(Node.GetName()), &Node);
		}

	else if (Node.IsTypeDefinition())
		{
		if (!AddSymbol(Node, retsError))
			return false;

		m_Types.SetAt(strToLower(Node.GetName()), &Node);
		}

	//	Otherwise, if this is a variable, then add to map.

	else if (Node.IsVarDefinition())
		{
		if (!AddSymbol(Node, retsError))
			return false;

		m_Vars.SetAt(strToLower(Node.GetName()), &Node);
		}

	//	If this is a statement, then add to the ordered list of statements.
	//	NOTE: It is possible to be in both (e.g., var s = 1).

	if (Node.IsStatement())
		{
		m_Statements.Insert(&Node);
		}

	return true;
	}

bool CASTSequence::AddSymbol (IASTNode &Node, CString *retsError)

//	AddSymbol
//
//	Adds a new symbol

	{
	bool bNew;
	m_Symbols.SetAt(strToLower(Node.GetName()), &Node, &bNew);
	if (!bNew)
		{
		if (retsError) *retsError = strPattern(ERR_DUPLICATE_DEFINITION, Node.GetName());
		return false;
		}

	return true;
	}

TSharedPtr<IASTNode> CASTSequence::Create (IASTNode *pParent, TArray<TSharedPtr<IASTNode>> Nodes, CString *retsError)

//	Create
//
//	Creates the sequence.

	{
	CASTSequence *pSeq = new CASTSequence(pParent);
	TSharedPtr<IASTNode> pResult(pSeq);

	pSeq->m_Node = std::move(Nodes);

	//	Keep an index of definitions

	for (int i = 0; i < pSeq->m_Node.GetCount(); i++)
		{
		IASTNode &Node = *pSeq->m_Node[i];

		if (!pSeq->AddToIndex(Node, retsError))
			return NULL;
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
