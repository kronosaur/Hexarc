//	CAEONMapColumnExpression.cpp
//
//	CAEONMapColumnExpression class
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_MAP_COLUMN_EXPRESSION,"mapColumnExpression");

TDatumPropertyHandler<CAEONMapColumnExpression> CAEONMapColumnExpression::m_Properties = {
	};

TDatumMethodHandler<CAEONMapColumnExpression> CAEONMapColumnExpression::m_Methods = {
	};

const CString& CAEONMapColumnExpression::StaticGetTypename (void)
	{
	return TYPENAME_MAP_COLUMN_EXPRESSION;
	}

size_t CAEONMapColumnExpression::CalcMemorySize () const
	{
	size_t iSize = m_dType.CalcMemorySize();

	iSize += sizeof(DWORD); // Flags for future expansion
	iSize += sizeof(int); // Count of expressions

	for (int i = 0; i < m_dExpressions.GetCount(); i++)
		{
		iSize += m_dExpressions.GetElement(i).CalcMemorySize();
		}

	return iSize;
	}

CDatum CAEONMapColumnExpression::CreateFromStruct (CDatum dStruct)

//	CreateFromStruct
//
//	Creates from a structure in which some elements are column expressions.

	{
	TArray<IDatatype::SMemberDesc> Members;
	CDatum dExprs(CDatum::typeArray);

	for (int i = 0; i < dStruct.GetCount(); i++)
		{
		Members.Insert({ IDatatype::EMemberType::InstanceVar, dStruct.GetKey(i), CAEONTypes::Get(IDatatype::ANY) });
		CDatum dColExpr = dStruct.GetElement(i);
		if (dColExpr.GetBasicType() == CDatum::typeExpression)
			{
			dExprs.Append(dColExpr);
			}
		else
			{
			dExprs.Append(CAEONExpression::CreateLiteral(dColExpr));
			}
		}

	CDatum dNewType = CAEONTypeSystem::CreateAnonymousSchema(Members);
	return CAEONMapColumnExpression::Create(dNewType, dExprs);
	}

CDatum CAEONMapColumnExpression::CreateIdentity (CDatum dTable)

//	CreateIdentity
//
//	Creates a map column expression that has the same schema as the table, and
//	accounts for grouping (defining key columns for group keys).

	{
	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		throw CException(errFail);

	const CAEONTableGroupDefinition& GroupDef = pTable->GetGroups();

	//	Now add the columns that are not excluded.

	CDatum dExprs(CDatum::typeArray);
	CDatum dTableSchema = pTable->GetSchema();
	const IDatatype& TableSchema = dTableSchema;
	for (int i = 0; i < TableSchema.GetMemberCount(); i++)
		{
		const IDatatype::SMemberDesc& Member = TableSchema.GetMember(i);
		CDatum dColExpr = CAEONExpression::CreateColumnRef(Member.sID);
		dExprs.Append(dColExpr);
		}

	CDatum dResultExpr = CAEONMapColumnExpression::Create(dTableSchema, dExprs);

	//	Now combine with groups.

	return CreateWithGroups(dTableSchema, dResultExpr, dTable);
	}

CDatum CAEONMapColumnExpression::CreateSummary (CDatum dTable, CStringView sColName, CDatum dColExpr)

//	CreateSummary
//
//	Creates a summary table definition with a single column, and accounts for
//	grouping if the table is grouped.

	{
	//	First we create a map column expression for the result column.

	TArray<IDatatype::SMemberDesc> Members;
	Members.Insert({ IDatatype::EMemberType::InstanceVar, CString(sColName), CAEONTypes::Get(IDatatype::ANY) });

	CDatum dNewType = CAEONTypeSystem::CreateAnonymousSchema(Members);
	CDatum dColExprs = CDatum(CDatum::typeArray);
	dColExprs.Append(dColExpr);
	CDatum dResultExpr = CAEONMapColumnExpression::Create(dNewType, dColExprs);

	//	Now combine with groups, if any.

	return CreateWithGroups(dNewType, dResultExpr, dTable);
	}

CDatum CAEONMapColumnExpression::CreateWithGroups (CDatum dType, CDatum dExpressions, CDatum dTable)

//	CreateWithGroups
//
//	dExpressions is CAEONMapColumnExpression object
//	dType is schema type for the map column expression.
//
//	We add columns to represent the group keys, if any.

	{
	struct SKeyColDef
		{
		CString sID;			//	ID of the column
		CDatum dType;			//	Type of the column
		CDatum dExpression;		//	Expression for the column

		bool bConflict = false;	//	Temporary flag to indicate if this column conflicts with an existing column
		bool bExclude = false;	//	In some cases, we exclude the key because the map column expression already has it.
		};

	const CAEONMapColumnExpression* pExprs = CAEONMapColumnExpression::Upconvert(dExpressions);
	if (!pExprs)
		return CDatum();

	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		return CDatum();

	const IDatatype& TableSchema = pTable->GetSchema();

	//	If no groups, then we just create the map column expression normally.

	auto& GroupDef = pTable->GetGroups();
	if (GroupDef.IsEmpty())
		return dExpressions;

	//	This is the output map schema

	const IDatatype& MapSchema = dType;

	//	Sometimes we turn a map column expression into a key column.

	TArray<bool> MakeKey;
	MakeKey.InsertEmpty(MapSchema.GetMemberCount());
	for (int i = 0; i < MakeKey.GetCount(); i++)
		MakeKey[i] = false;

	//	Generate columns for each group key.

	int iNextKey = 1;
	TArray<SKeyColDef> KeyCols;
	for (int i = 0; i < GroupDef.GetCount(); i++)
		{
		SKeyColDef KeyCol;

		const CAEONExpression& Expr = GroupDef.GetExpression(i);

		//	If the root node is a column, then we use that as the key.

		int iMember;
		if (Expr.GetRootNode().iOp == CAEONExpression::EOp::Column
				&& (iMember = TableSchema.FindMember(Expr.GetColumnID(Expr.GetRootNode().iDataID))) != -1)
			{
			auto Member = TableSchema.GetMember(iMember);
			KeyCol.sID = Member.sID;
			KeyCol.dType = Member.dType;
			KeyCol.dExpression = CAEONExpression::Create(Expr);

			//	Look for this column in the column expression. If it exists, and
			//	it is the same expression, then we exclude it from the key columns
			//	and make the column expression a key column.s

			int iMapSchemaIndex = MapSchema.FindMember(KeyCol.sID);
			if (iMapSchemaIndex != -1)
				{
				CDatum dColExpr = pExprs->GetColExpression(iMapSchemaIndex);
				const CAEONExpression* pColExpr = dColExpr.GetQueryInterface();
				if (pColExpr)
					{
					const CAEONExpression::SNode& RootNode = pColExpr->GetRootNode();
					if (RootNode.iOp == CAEONExpression::EOp::Column
							&& strEqualsNoCase(pColExpr->GetColumnID(RootNode.iDataID), KeyCol.sID))
						{
						KeyCol.bExclude = true; // We exclude this key column because it already exists in the map column expression.
						MakeKey[iMapSchemaIndex] = true; // We mark this column as a key column in the map column expression.
						}
					}
				}
			}
		else
			{
			KeyCol.sID = strPattern("key_%d", iNextKey++);
			KeyCol.dType = CAEONTypes::Get(IDatatype::ANY);
			KeyCol.dExpression = CAEONExpression::Create(Expr);
			}

		KeyCols.Insert(KeyCol);
		}

	//	Make sure the new key columns don't conflict with the map columns. If 
	//	they do, we rename them.

	while (true)
		{
		//	Check for conflicts.

		bool bConflict = false;
		for (int i = 0; i < KeyCols.GetCount(); i++)
			{
			if (KeyCols[i].bExclude)
				continue; // Skip excluded keys

			KeyCols[i].bConflict = (MapSchema.FindMember(KeyCols[i].sID) != -1);
			if (KeyCols[i].bConflict)
				bConflict = true;
			}

		//	If no conflicts, then we're done.

		if (!bConflict)
			break;

		//	Otherwise, we rename the columns.

		for (int i = 0; i < KeyCols.GetCount(); i++)
			{
			if (KeyCols[i].bConflict)
				KeyCols[i].sID = strPattern("key_%d", iNextKey++);
			}

		//	Loop
		}

	//	Create the new schema with the key columns added at the font.

	TArray<IDatatype::SMemberDesc> Members;
	for (int i = 0; i < KeyCols.GetCount(); i++)
		{
		if (KeyCols[i].bExclude)
			continue; // Skip excluded keys

		IDatatype::SMemberDesc Member;
		Member.iType = IDatatype::EMemberType::InstanceKeyVar;
		Member.sID = KeyCols[i].sID;
		Member.dType = KeyCols[i].dType;
		Members.Insert(Member);
		}

	for (int i = 0; i < MapSchema.GetMemberCount(); i++)
		{
		IDatatype::SMemberDesc Member = MapSchema.GetMember(i);

		//	If this is a key column, then we make it an instance variable.

		if (MakeKey[i])
			Member.iType = IDatatype::EMemberType::InstanceKeyVar;

		//	Otherwise, these must be normal columns

		else
			Member.iType = IDatatype::EMemberType::InstanceVar;

		Members.Insert(Member);
		}

	CDatum dNewType = CAEONTypeSystem::CreateAnonymousSchema(Members);

	//	Create a new map column expression with the new type and the existing expressions.

	CDatum dNewExpressions(CDatum::typeArray);
	for (int i = 0; i < KeyCols.GetCount(); i++)
		{
		if (KeyCols[i].bExclude)
			continue; // Skip excluded keys

		dNewExpressions.Append(KeyCols[i].dExpression);
		}

	for (int i = 0; i < pExprs->GetColCount(); i++)
		dNewExpressions.Append(pExprs->GetColExpression(i));

	//	Now we can finally create the map column expression.

	return Create(dNewType, dNewExpressions);
	}

CDatum CAEONMapColumnExpression::GetDatatype () const
	{
	return CAEONTypes::Get(IDatatype::MAP_COLUMN_EXPRESSION);
	}

TArray<IDatatype::SMemberDesc> CAEONMapColumnExpression::GetMembers (void)
	{
	TArray<IDatatype::SMemberDesc> Members;

	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);

	return Members;
	}

int CAEONMapColumnExpression::OpCompare (CDatum::Types iValueType, CDatum dValue) const

//	OpCompare
//
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2

	{
	//	If we're both of the same type, then we compare

	if (CAEONMapColumnExpression* pSrc = CAEONMapColumnExpression::Upconvert(dValue))
		return m_dType.OpCompare(pSrc->m_dType);

	//	Otherwise, not equal

	else
		return ::KeyCompare(AsString(), dValue.AsString());
	}

bool CAEONMapColumnExpression::OpIsEqual (CDatum::Types iValueType, CDatum dValue) const

//	OpIsEqual
//
//	Returns TRUE if we are equivalent.

	{
	//	If we're both of the same type, then we compare

	if (CAEONMapColumnExpression* pSrc = CAEONMapColumnExpression::Upconvert(dValue))
		return m_dType.OpIsEqual(pSrc->m_dType);

	//	Otherwise, not equal

	else
		return false;
	}

bool CAEONMapColumnExpression::OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const
	{
	//	If we're both of the same type, then we compare

	if (CAEONMapColumnExpression* pSrc = CAEONMapColumnExpression::Upconvert(dValue))
		return m_dType.OpIsIdentical(pSrc->m_dType);

	//	Otherwise, not equal

	else
		return false;
	}

bool CAEONMapColumnExpression::OnDeserialize (CDatum::EFormat iFormat, const CString& sTypename, IByteStream& Stream)
	{
	CAEONSerializedMap Serialized;
	DeserializeAEONExternal(Stream, Serialized);
	return true;
	}

void CAEONMapColumnExpression::OnSerialize (CDatum::EFormat iFormat, IByteStream& Stream) const
	{
	CAEONSerializedMap Serialized;
	SerializeAEONExternal(Stream, Serialized);
	}

void CAEONMapColumnExpression::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized)
	{
	m_dType = CDatum::DeserializeAEON(Stream, Serialized);

	DWORD dwFlags = Stream.ReadDWORD();

	int iCount = Stream.ReadInt();
	m_dExpressions.GrowToFit(iCount);
	for (int i = 0; i < iCount; i++)
		{
		m_dExpressions.Append(CDatum::DeserializeAEON(Stream, Serialized));
		}
	}

void CAEONMapColumnExpression::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	m_dType.SerializeAEON(Stream, Serialized);

	Stream.Write(0);	//	Flags for later expansion

	Stream.Write(m_dExpressions.GetCount());
	for (int i = 0; i < m_dExpressions.GetCount(); i++)
		{
		m_dExpressions.GetElement(i).SerializeAEON(Stream, Serialized);
		}
	}
