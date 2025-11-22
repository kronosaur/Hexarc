//	CDatatypeNull.cpp
//
//	CDatatypeNull class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

bool CDatatypeNull::OnIsA (const IDatatype &Type) const
	{
	if (Type.CanBeNull())
		return true;

	if (Type.GetClass() == ECategory::Nullable)
		return true;

	return false;
	}

