//	CDatatypeNever.cpp
//
//	CDatatypeNever class
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_NEVER,					"Never");

CDatatypeNever::CDatatypeNever () : IDatatype(true, CAEONTypes::MakeFullyQualifiedName(NULL_STR, TYPENAME_NEVER), IDatatype::NEVER)

//	CDatatypeNever constructor

	{
	}
