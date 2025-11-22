//	CAEONArrayAlgorithm.cpp
//
//	CAEONArrayAlgorithm class
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_EXACT,						"exact");

CDatum CAEONArrayAlgorithm::Except (CDatum dArray, CDatum dExclude, CDatum dOptions)

//	Except
//
//	Returns the equivalent of dArray.filter(fn(row)->!dExclude.find(row)).

	{
	ASSERT(dArray.IsArray());

	//	NOTE: We create a new array with the same element type. If dArray is a
	//	tensor, then it will decay to a normal array, which is what we want.

	CDatum dResult = CDatum::CreateArrayAsTypeOfElement(((const IDatatype&)dArray.GetDatatype()).GetElementType());

	if (dOptions.GetElement(FIELD_EXACT).AsBool())
		{
		if (dExclude.IsArray())
			{
			for (int i = 0; i < dArray.GetCount(); i++)
				{
				CDatum dItem = dArray.GetElement(i);
				if (!dExclude.FindExact(dItem))
					dResult.Append(dItem);
				}
			}
		else
			{
			for (int i = 0; i < dArray.GetCount(); i++)
				{
				CDatum dItem = dArray.GetElement(i);
				if (!dItem.OpIsIdentical(dExclude))
					dResult.Append(dItem);
				}
			}
		}
	else
		{
		if (dExclude.IsArray())
			{
			for (int i = 0; i < dArray.GetCount(); i++)
				{
				CDatum dItem = dArray.GetElement(i);
				if (!dExclude.Find(dItem))
					dResult.Append(dItem);
				}
			}
		else
			{
			for (int i = 0; i < dArray.GetCount(); i++)
				{
				CDatum dItem = dArray.GetElement(i);
				if (!dItem.OpIsEqual(dExclude))
					dResult.Append(dItem);
				}
			}
		}

	return dResult;
	}

CDatum CAEONArrayAlgorithm::Intersect (CDatum dArray, CDatum dIntersect, CDatum dOptions)

//	Intersect
//
//	Return the equivalent of dArray.filter(fn(row)->dIntersect.find(row)).

	{
	ASSERT(dArray.IsArray());

	CDatum dResult = CDatum::CreateArrayAsTypeOfElement(((const IDatatype&)dArray.GetDatatype()).GetElementType());

	if (dOptions.GetElement(FIELD_EXACT).AsBool())
		{
		if (dIntersect.IsArray())
			{
			for (int i = 0; i < dArray.GetCount(); i++)
				{
				CDatum dItem = dArray.GetElement(i);
				if (dIntersect.FindExact(dItem))
					dResult.Append(dItem);
				}
			}
		else
			{
			for (int i = 0; i < dArray.GetCount(); i++)
				{
				CDatum dItem = dArray.GetElement(i);
				if (dItem.OpIsIdentical(dIntersect))
					dResult.Append(dItem);
				}
			}
		}
	else
		{
		if (dIntersect.IsArray())
			{
			for (int i = 0; i < dArray.GetCount(); i++)
				{
				CDatum dItem = dArray.GetElement(i);
				if (dIntersect.Find(dItem))
					dResult.Append(dItem);
				}
			}
		else
			{
			for (int i = 0; i < dArray.GetCount(); i++)
				{
				CDatum dItem = dArray.GetElement(i);
				if (dItem.OpIsEqual(dIntersect))
					dResult.Append(dItem);
				}
			}
		}

	return dResult;
	}

CDatum CAEONArrayAlgorithm::Union (CDatum dArray, CDatum dUnion, CDatum dOptions)

//	Union
//
//	Return the equivalent of dArray.filter(fn(row)->!dUnion.find(row)).

	{
	ASSERT(dArray.IsArray());

	CDatum dResult = CDatum::CreateArrayAsTypeOfElement(((const IDatatype&)dArray.GetDatatype()).GetElementType(), dArray);

	if (dOptions.GetElement(FIELD_EXACT).AsBool())
		{
		if (dUnion.IsArray())
			{
			for (int i = 0; i < dUnion.GetCount(); i++)
				{
				CDatum dItem = dUnion.GetElement(i);
				if (!dArray.FindExact(dItem))
					dResult.Append(dItem);
				}
			}
		else
			{
			if (!dArray.FindExact(dUnion))
				dResult.Append(dUnion);
			}
		}
	else
		{
		if (dUnion.IsArray())
			{
			for (int i = 0; i < dUnion.GetCount(); i++)
				{
				CDatum dItem = dUnion.GetElement(i);
				if (!dArray.Find(dItem))
					dResult.Append(dItem);
				}
			}
		else
			{
			if (!dArray.Find(dUnion))
				dResult.Append(dUnion);
			}
		}

	return dResult;
	}

