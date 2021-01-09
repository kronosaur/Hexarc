//	IASTNode.cpp
//
//	IASTNode Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

bool IASTNode::ComposeError (const CString &sError, CString *retsError) const

//	ComposeError
//
//	Composes an error for the given AST.

	{
	if (retsError)
		*retsError = sError;

	return false;
	}
