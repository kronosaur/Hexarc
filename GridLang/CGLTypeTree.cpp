//	CGLTypeTree.cpp
//
//	CGLTypeTree Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CGLTypeTree::AddType (IGLType &Type)
	{
	SetAt(&Type);
	}

void CGLTypeTree::DumpNode (const SNode &Node, const CString &sIndent) const

//	DumpNode
//
//	Dumps the node and its children.

	{
	if (Node.pType)
		printf("%s%s\n", (LPSTR)sIndent, (LPSTR)Node.pType->GetName());
	else
		printf("%s(root)\n", (LPSTR)sIndent);

	CString sNewIndent = strPattern("%s\t", sIndent);
	for (int i = 0; i < Node.Children.GetCount(); i++)
		DumpNode(*Node.Children[i], sNewIndent);
	}

CGLTypeTree::SNode *CGLTypeTree::SetAt (IGLType *pType)

//	SetAt
//
//	Returns the node for the given type.

	{
	//	If NULL, then this is the root.

	if (!pType)
		return &m_Root;

	//	Get the parent node of this type.

	SNode *pParentNode = SetAt(pType->GetParent());

	//	Are we there already?

	for (int i = 0; i < pParentNode->Children.GetCount(); i++)
		if (pParentNode->Children[i]->pType == pType)
			{
			return pParentNode->Children[i];
			}

	//	Otherwise, add a node

	SNode *pNewNode = new SNode;
	pNewNode->pType = pType;

	TUniquePtr<SNode> *pSlot = pParentNode->Children.Insert();
	pSlot->Set(pNewNode);

	return pNewNode;
	}
