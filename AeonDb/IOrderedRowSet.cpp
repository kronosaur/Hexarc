//	IOrderedRowSet.cpp
//
//	IOrderedRowSet class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

void IOrderedRowSet::ShiftDimensions (const CTableDimensions &Source, CTableDimensions *retDest)

//	ShiftDimensions
//
//	Shifts down one dimension. This is used to convert from the dimensions of a table to a row value

	{
	int i;

	ASSERT(Source.GetCount() >= 1);

	retDest->DeleteAll();
	retDest->InsertEmpty(Source.GetCount() - 1);
	for (i = 1; i < Source.GetCount(); i++)
		retDest->GetAt(i - 1) = Source[i];
	}
