//	CAEONArrayAlgorithm.cpp
//
//	CAEONArrayAlgorithm class
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_EXACT,						"exact");

template <typename FUNCTION>
static void ForEachArrayElement (CDatum dArray, FUNCTION fn)

//	ForEachArrayElement
//
//	Enumerates tensor elements in logical order. Other arrays retain their
//	ordinary top-level iteration semantics.

	{
	if (dArray.GetBasicType() == CDatum::typeTensor)
		{
		CBuffer Pos = dArray.raw_IteratorStart();
		while (dArray.raw_IteratorHasMore(Pos))
			{
			fn(dArray.raw_IteratorGetElement(Pos));
			dArray.raw_IteratorNext(Pos);
			}
		}
	else
		{
		for (int i = 0; i < dArray.GetCount(); i++)
			fn(dArray.GetElement(i));
		}
	}

static bool ArrayContains (CDatum dArray, CDatum dValue, bool bExact)

//	ArrayContains
//
//	Looks for a value using the array's native equality semantics. Tensor Find
//	methods search their flattened scalar elements.

	{
	return (bExact ? dArray.FindExact(dValue) : dArray.Find(dValue));
	}

static CDatum CreateResult (CAEONTypeSystem& TypeSystem, CDatum dSource, CDatum dValues)

//	CreateResult
//
//	Tensor set operations return a zero-based rank-1 tensor. Other arrays are
//	already constructed with the appropriate result type.

	{
	if (dSource.GetBasicType() != CDatum::typeTensor)
		return dValues;

	const IDatatype& SourceType = dSource.GetDatatype();
	CDatum dElementType = SourceType.GetElementType();

	TArray<CDatum> Dimensions;
	Dimensions.Insert(CAEONTypes::CreateInt32SubRange(NULL_STR, 0, dValues.GetCount() - 1));
	CDatum dResultType = TypeSystem.AddAnonymousTensor(dElementType, std::move(Dimensions));
	return CDatum::CreateTensorAsType(dResultType, dValues);
	}

CDatum CAEONArrayAlgorithm::Except (CAEONTypeSystem& TypeSystem, CDatum dArray, CDatum dExclude, CDatum dOptions)

//	Except
//
//	Returns the equivalent of dArray.filter(fn(row)->!dExclude.find(row)).

	{
	ASSERT(dArray.IsArray());

	CDatum dResult = (dArray.GetBasicType() == CDatum::typeTextLines ? CDatum::CreateAsType(dArray.GetDatatype()) : CDatum::CreateArrayAsTypeOfElement(((const IDatatype&)dArray.GetDatatype()).GetElementType()));
	bool bExact = dOptions.GetElement(FIELD_EXACT).AsBool();

	ForEachArrayElement(dArray,
		[&](CDatum dItem)
			{
			bool bExcluded = (dExclude.IsArray()
					? ArrayContains(dExclude, dItem, bExact)
					: (bExact ? dItem.OpIsIdentical(dExclude) : dItem.OpIsEqual(dExclude)));

			if (!bExcluded)
				dResult.Append(dItem);
			});

	return CreateResult(TypeSystem, dArray, dResult);
	}

CDatum CAEONArrayAlgorithm::Intersect (CAEONTypeSystem& TypeSystem, CDatum dArray, CDatum dIntersect, CDatum dOptions)

//	Intersect
//
//	Return the equivalent of dArray.filter(fn(row)->dIntersect.find(row)).

	{
	ASSERT(dArray.IsArray());

	CDatum dResult = (dArray.GetBasicType() == CDatum::typeTextLines ? CDatum::CreateAsType(dArray.GetDatatype()) : CDatum::CreateArrayAsTypeOfElement(((const IDatatype&)dArray.GetDatatype()).GetElementType()));
	bool bExact = dOptions.GetElement(FIELD_EXACT).AsBool();

	ForEachArrayElement(dArray,
		[&](CDatum dItem)
			{
			bool bIncluded = (dIntersect.IsArray()
					? ArrayContains(dIntersect, dItem, bExact)
					: (bExact ? dItem.OpIsIdentical(dIntersect) : dItem.OpIsEqual(dIntersect)));

			if (bIncluded)
				dResult.Append(dItem);
			});

	return CreateResult(TypeSystem, dArray, dResult);
	}

CDatum CAEONArrayAlgorithm::Union (CAEONTypeSystem& TypeSystem, CDatum dArray, CDatum dUnion, CDatum dOptions)

//	Union
//
//	Returns dArray followed by elements of dUnion not present in dArray.

	{
	ASSERT(dArray.IsArray());

	CDatum dResult = (dArray.GetBasicType() == CDatum::typeTextLines ? CDatum::CreateAsType(dArray.GetDatatype()) : CDatum::CreateArrayAsTypeOfElement(((const IDatatype&)dArray.GetDatatype()).GetElementType()));
	bool bExact = dOptions.GetElement(FIELD_EXACT).AsBool();

	ForEachArrayElement(dArray,
		[&](CDatum dItem)
			{
			dResult.Append(dItem);
			});

	if (dUnion.IsArray())
		{
		ForEachArrayElement(dUnion,
			[&](CDatum dItem)
				{
				if (!ArrayContains(dArray, dItem, bExact))
					dResult.Append(dItem);
				});
		}
	else if (!ArrayContains(dArray, dUnion, bExact))
		dResult.Append(dUnion);

	return CreateResult(TypeSystem, dArray, dResult);
	}
