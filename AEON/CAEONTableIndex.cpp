//	CAEONTableIndex.cpp
//
//	CAEONTableIndex class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

const CAEONTableIndex CAEONTableIndex::Null;

void CAEONTableIndex::Add (CDatum dTable, int iRow)

//	Add
//
//	Add an entry to the index.

	{
	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		return;

	m_Index.SetAt(GetIndexKey(*pTable, iRow), iRow);
	}

void CAEONTableIndex::AddRow (CStringView sIndexKey, int iRow)
	{
	m_Index.SetAt(sIndexKey, iRow);
	}

CString CAEONTableIndex::AsIndexKeyFromRow (CDatum dTable, CDatum dValue) const

//	AsIndexKeyFromRow
//
//	Returns an index key depending on the value. The value is either a row 
//	struct or an array of values or a single value.

	{
	if (m_Cols.GetCount() == 1)
		{
		return AsIndexKeyFromValue(dValue);
		}
	else if (m_Cols.GetCount() > 1)
		{
		if (dValue.IsArray())
			{
			CStringBuffer Result;
			Result.Write(AsIndexKeyFromValue(dValue.GetElement(0)));
			for (int i = 1; i < m_Cols.GetCount(); i++)
				{
				Result.WriteChar('/');
				Result.Write(AsIndexKeyFromValue(dValue.GetElement(i)));
				}

			return CString(std::move(Result));
			}
		else
			{
			CDatum dDatatype = dTable.GetDatatype();
			const IDatatype& Schema = ((const IDatatype&)dDatatype).GetElementType();

			CStringBuffer Result;
			for (int i = 0; i < m_Cols.GetCount(); i++)
				{
				if (m_Cols[i] < 0 || m_Cols[i] >= Schema.GetMemberCount())
					return NULL_STR;

				if (i != 0)
					Result.WriteChar('/');

				Result.Write(AsIndexKeyFromValue(dValue.GetElement(Schema.GetMember(m_Cols[i]).sID)));
				}

			return CString(std::move(Result));
			}
		}
	else
		return NULL_STR;
	}

CString CAEONTableIndex::AsIndexKeyFrom2 (CDatum dTable, CDatum dKey1, CDatum dKey2) const
	{
	if (m_Cols.GetCount() != 2)
		return NULL_STR;

	CStringBuffer Result;
	Result.Write(AsIndexKeyFromValue(dKey1));
	Result.WriteChar('/');
	Result.Write(AsIndexKeyFromValue(dKey2));

	return CString(std::move(Result));
	}

CString CAEONTableIndex::AsIndexKeyFrom3 (CDatum dTable, CDatum dKey1, CDatum dKey2, CDatum dKey3) const
	{
	if (m_Cols.GetCount() != 3)
		return NULL_STR;

	CStringBuffer Result;
	Result.Write(AsIndexKeyFromValue(dKey1));
	Result.WriteChar('/');
	Result.Write(AsIndexKeyFromValue(dKey2));
	Result.WriteChar('/');
	Result.Write(AsIndexKeyFromValue(dKey3));

	return CString(std::move(Result));
	}

CString CAEONTableIndex::AsIndexKeyFromArray (CDatum dStruct, const TArray<int>& Cols)

//	AsIndexKeyFromArray
//
//	Returns an index key from an array.

	{
	CStringBuffer Result;
	for (int i = 0; i < Cols.GetCount(); i++)
		{
		if (i != 0)
			Result.WriteChar('/');

		Result.Write(AsIndexKeyFromValue(dStruct.GetElement(Cols[i])));
		}

	return CString(std::move(Result));
	}

CString CAEONTableIndex::AsIndexKeyFromStruct (CDatum dStruct, const TArray<CString>& Cols)

//	AsIndexKeyFromStruct
//
//	Returns an index key from a struct.

	{
	CStringBuffer Result;
	for (int i = 0; i < Cols.GetCount(); i++)
		{
		if (i != 0)
			Result.WriteChar('/');

		Result.Write(AsIndexKeyFromValue(dStruct.GetElement(Cols[i])));
		}

	return CString(std::move(Result));
	}

CString CAEONTableIndex::AsIndexKeyFromValue (CDatum dValue)

//	AsIndexKey
//
//	Returns as a key.

	{
	switch (dValue.GetBasicType())
		{
		case CDatum::typeString:
			return dValue.AsString();

		case CDatum::typeDateTime:
			return ((const CDateTime&)dValue).Format(CDateTime::formatISO8601);

		default:
			return dValue.AsString();
		}
	}

TArray<bool> CAEONTableIndex::CalcColumnIsKey (CDatum dSchema)

//	CalcColumnIsKey
//
//	Returns an array with a bool for each column specifying whether the column
//	is a key or not.

	{
	const IDatatype& Schema = dSchema;
	if (Schema.GetClass() != IDatatype::ECategory::Schema)
		throw CException(errFail);

	TArray<bool> IsKeyCol;
	IsKeyCol.InsertEmpty(Schema.GetMemberCount());

	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);
		IsKeyCol[i] = ColumnDesc.iType == IDatatype::EMemberType::InstanceKeyVar;
		}

	return IsKeyCol;
	}

void CAEONTableIndex::DeleteRow (CDatum dTable, int iRow)

//	DeleteRow
//
//	Deletes the given row from the index. Note that this is different from 
//	Remove because we need to adjust the row indices of all rows after this row.

	{
	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		return;

	for (int i = 0; i < m_Index.GetCount(); i++)
		{
		if (m_Index[i] == iRow)
			{
			m_Index.Delete(i);
			i--;
			}
		else if (m_Index[i] > iRow)
			{
			m_Index[i]--;
			}
		}
	}

int CAEONTableIndex::Find (CDatum dTable, CDatum dValue) const

//	Find
//
//	Find an entry.

	{
	int* pRow = m_Index.GetAt(AsIndexKeyFromRow(dTable, dValue));
	if (!pRow)
		return -1;

	return *pRow;
	}

int CAEONTableIndex::Find2 (CDatum dTable, CDatum dKey1, CDatum dKey2) const
	{
	int* pRow = m_Index.GetAt(AsIndexKeyFrom2(dTable, dKey1, dKey2));
	if (!pRow)
		return -1;

	return *pRow;
	}

int CAEONTableIndex::Find3 (CDatum dTable, CDatum dKey1, CDatum dKey2, CDatum dKey3) const
	{
	int* pRow = m_Index.GetAt(AsIndexKeyFrom3(dTable, dKey1, dKey2, dKey3));
	if (!pRow)
		return -1;

	return *pRow;
	}

CAEONTableIndex::EFindResult CAEONTableIndex::FindOrAdd (CDatum dTable, CDatum dKey, int iNewRow, int *retiRow)

//	FindOrAdd
//
//	Looks for the given key. If it exists, we return ::Found and retiRow is
//	initialized with the row index. Otherwise, we try to add a new entry using
//	iNewRow and return ::Added.

	{
	EFindResult iResult;

	bool bAdded;
	int* pRow = m_Index.SetAt(AsIndexKeyFromRow(dTable, dKey), &bAdded);
	if (bAdded)
		{
		*pRow = iNewRow;
		iResult = EFindResult::Added;
		}
	else
		{
		iResult = EFindResult::Found;
		}

	if (retiRow)
		*retiRow = *pRow;

	return iResult;
	}

CString CAEONTableIndex::GetIndexKey (const IAEONTable& Table, int iRow) const

//	GetIndexKey
//
//	Returns the index key.

	{
	if (m_Cols.GetCount() == 1)
		return AsIndexKeyFromValue(Table.GetFieldValue(iRow, m_Cols[0]));
	else if (m_Cols.GetCount() > 1)
		{
		CStringBuffer Result;
		Result.Write(AsIndexKeyFromValue(Table.GetFieldValue(iRow, m_Cols[0])));
		for (int i = 1; i < m_Cols.GetCount(); i++)
			{
			Result.WriteChar('/');
			Result.Write(AsIndexKeyFromValue(Table.GetFieldValue(iRow, m_Cols[i])));
			}

		return CString(std::move(Result));
		}
	else
		return NULL_STR;
	}

CDatum CAEONTableIndex::GetKeyArray (CDatum dTable) const

//	GetKeyArray
//
//	Returns an array of keys.

	{
	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		return CDatum();

	if (m_Cols.GetCount() == 0)
		return CDatum();
	else if (m_Cols.GetCount() == 1)
		{
		CDatum dResult(CDatum::typeArray);
		dResult.GrowToFit(m_Index.GetCount());

		for (int i = 0; i < m_Index.GetCount(); i++)
			{
			int iRow = m_Index[i];
			dResult.Append(pTable->GetFieldValue(iRow, m_Cols[0]));
			}

		return dResult;
		}
	else
		{
		CDatum dResult(CDatum::typeArray);
		dResult.GrowToFit(m_Index.GetCount());

		for (int i = 0; i < m_Index.GetCount(); i++)
			{
			int iRow = m_Index[i];

			CDatum dKey(CDatum::typeArray);
			dKey.GrowToFit(m_Cols.GetCount());
			for (int j = 0; j < m_Cols.GetCount(); j++)
				dKey.Append(pTable->GetFieldValue(iRow, m_Cols[j]));

			dResult.Append(dKey);
			}

		return dResult;
		}
	}

CDatum CAEONTableIndex::GetKeyFromColumnStruct (CDatum dTable, CDatum dCols, int iIndex) const

//	GetKeyFromColumnStruct
//
//	Returns the key from the given row in the struct of columns.

	{
	CDatum dDatatype = dTable.GetDatatype();
	const IDatatype& Schema = ((const IDatatype&)dDatatype).GetElementType();

	if (m_Cols.GetCount() == 1)
		{
		if (m_Cols[0] < 0)
			return CDatum();

		auto ColDesc = Schema.GetMember(m_Cols[0]);
		return dCols.GetElement(ColDesc.sID).GetElement(iIndex);
		}
	else if (m_Cols.GetCount() > 1)
		{
		CDatum dResult(CDatum::typeArray);
		for (int i = 0; i < m_Cols.GetCount(); i++)
			{
			if (m_Cols[i] < 0)
				dResult.Append(CDatum());
			else
				{
				auto ColDesc = Schema.GetMember(m_Cols[i]);
				dResult.Append(dCols.GetElement(ColDesc.sID).GetElement(iIndex));
				}
			}
		return dResult;
		}
	else
		return CDatum();
	}

CDatum CAEONTableIndex::GetKeyFromRow (CDatum dTable, CDatum dRow) const

//	GetKeyFromRow
//
//	Returns the key from the given row.

	{
	CDatum dDatatype = dTable.GetDatatype();
	const IDatatype& Schema = ((const IDatatype&)dDatatype).GetElementType();

	if (m_Cols.GetCount() == 1)
		{
		if (m_Cols[0] < 0)
			return CDatum();
		else
			{
			auto ColDesc = Schema.GetMember(m_Cols[0]);
			return dRow.GetElement(ColDesc.sID);
			}
		}
	else if (m_Cols.GetCount() > 1)
		{
		CDatum dResult(CDatum::typeArray);
		for (int i = 0; i < m_Cols.GetCount(); i++)
			{
			if (m_Cols[i] < 0)
				dResult.Append(CDatum());
			else
				{
				auto ColDesc = Schema.GetMember(m_Cols[i]);
				dResult.Append(dRow.GetElement(ColDesc.sID));
				}
			}
		return dResult;
		}
	else
		return CDatum();
	}

CDatum CAEONTableIndex::GetKeyFromRow (CDatum dTable, int iRow) const

//	GetKeyFromRow
//
//	Returns the key from the given row.

	{
	const IAEONTable *pTable = dTable.GetTableInterface();
	if (!pTable)
		return CDatum();

	if (iRow < 0 || iRow >= pTable->GetRowCount())
		return CDatum();

	if (m_Cols.GetCount() == 1)
		{
		return pTable->GetFieldValue(iRow, m_Cols[0]);
		}
	else if (m_Cols.GetCount() > 1)
		{
		CDatum dResult(CDatum::typeArray);
		for (int i = 0; i < m_Cols.GetCount(); i++)
			{
			dResult.Append(pTable->GetFieldValue(iRow, m_Cols[i]));
			}
		return dResult;
		}
	else
		return CDatum();
	}

CDatum CAEONTableIndex::GetKeyFromRowArray (CDatum dTable, CDatum dRow) const

//	GetKeyFromRowArray
//
//	Returns the key from a given row expressed as an array of values.

	{
	if (m_Cols.GetCount() == 1)
		{
		return dRow.GetElement(m_Cols[0]);
		}
	else if (m_Cols.GetCount() > 1)
		{
		CDatum dResult(CDatum::typeArray);
		for (int i = 0; i < m_Cols.GetCount(); i++)
			{
			dResult.Append(dRow.GetElement(m_Cols[i]));
			}
		return dResult;
		}
	else
		return CDatum();
	}

CDatum CAEONTableIndex::GetValueFromKey (CDatum dTable, CDatum dKey, const CString& sCol) const

//	GetValueFromKey
//
//	Returns the column value from a key.

	{
	if (m_Cols.GetCount() == 1)
		{
		return dKey;
		}
	else if (m_Cols.GetCount() > 1)
		{
		if (dKey.IsArray())
			{
			CDatum dDatatype = dTable.GetDatatype();
			const IDatatype& Schema = ((const IDatatype&)dDatatype).GetElementType();
			int iCol = Schema.FindMember(sCol);
			if (iCol == -1)
				return CDatum();

			int iPos;
			if (!m_Cols.Find(iCol, &iPos))
				return CDatum();

			return dKey.GetElement(iPos);
			}
		else
			{
			return dKey.GetElement(sCol);
			}
		}
	else
		return CDatum();
	}

void CAEONTableIndex::Init (CDatum dTable, const TArray<int>& Cols)

//	Init
//
//	Initialize the index from the given table.

	{
	m_Cols = Cols;
	m_Index.DeleteAll();

	if (m_Cols.GetCount() == 0)
		return;

	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		return;

	//	We're going to generate an array of keys for each row in the table.

	TArray<CString> Keys;
	Keys.GrowToFit(pTable->GetRowCount());
	for (int iRow = 0; iRow < pTable->GetRowCount(); iRow++)
		Keys.Insert(GetIndexKey(*pTable, iRow));

	//	Now we sort the keys

	m_Index = Keys.SortToMap();
	}

void CAEONTableIndex::Init (CDatum dTable, const TArray<CString>& Cols)

//	Init
//
//	Initializes the index from the given table. We handle the case where the 
//	table does not have 1 or more of the requested columns.

	{
	//	Look for the columns in the table schema.
	//
	//	NOTE: It is OK if we don't find a column because we handle it.
	//	We should always be able to handle -1 as a column index.

	TArray<int> ColIndex;
	const IDatatype& Schema = ((const IDatatype&)dTable.GetDatatype()).GetElementType();
	for (int i = 0; i < Cols.GetCount(); i++)
		ColIndex.Insert(Schema.FindMember(Cols[i]));

	//	Initialize

	Init(dTable, ColIndex);
	}

void CAEONTableIndex::InitColsOnly (CDatum dTable, const TArray<int>& Cols)

//	InitColsOnly
//
//	Initialize the columns, but not the actual index.

	{
	m_Cols = Cols;
	m_Index.DeleteAll();
	}

void CAEONTableIndex::InitFromArrayOfArrays (CDatum dArray, const TArray<int>& Cols)

//	InitFromArrayOfStructs
//
//	Initializes from an array of structs.

	{
	m_Cols = Cols;
	m_Index.DeleteAll();

	if (m_Cols.GetCount() == 0)
		return;

	//	We're going to generate an array of keys for each row in the table.

	TArray<CString> Keys;
	Keys.GrowToFit(dArray.GetCount());

	if (Cols.GetCount() == 1)
		{
		for (int iRow = 0; iRow < dArray.GetCount(); iRow++)
			Keys.Insert(AsIndexKeyFromValue(dArray.GetElement(iRow).GetElement(Cols[0])));
		}
	else
		{
		for (int iRow = 0; iRow < dArray.GetCount(); iRow++)
			Keys.Insert(AsIndexKeyFromArray(dArray.GetElement(iRow), Cols));
		}

	//	Now we sort the keys

	m_Index = Keys.SortToMap();
	}

void CAEONTableIndex::InitFromArrayOfStructs (CDatum dArray, const TArray<CString>& Cols)

//	InitFromArrayOfStructs
//
//	Initializes from an array of structs.

	{
	m_Cols.DeleteAll();
	for (int i = 0; i < Cols.GetCount(); i++)
		m_Cols.Insert(-1);

	m_Index.DeleteAll();

	if (m_Cols.GetCount() == 0)
		return;

	//	We're going to generate an array of keys for each row in the table.

	TArray<CString> Keys;
	Keys.GrowToFit(dArray.GetCount());

	if (Cols.GetCount() == 1)
		{
		for (int iRow = 0; iRow < dArray.GetCount(); iRow++)
			Keys.Insert(AsIndexKeyFromValue(dArray.GetElement(iRow).GetElement(Cols[0])));
		}
	else
		{
		for (int iRow = 0; iRow < dArray.GetCount(); iRow++)
			Keys.Insert(AsIndexKeyFromStruct(dArray.GetElement(iRow), Cols));
		}

	//	Now we sort the keys

	m_Index = Keys.SortToMap();
	}

void CAEONTableIndex::InitFromStructOfArrays (CDatum dStruct, const TArray<CString>& Cols)

//	InitFromStructOfArrays
//
//	Initializes from a struct of arrays.

	{
	m_Cols.DeleteAll();
	for (int i = 0; i < Cols.GetCount(); i++)
		m_Cols.Insert(-1);

	m_Index.DeleteAll();

	if (m_Cols.GetCount() == 0)
		return;

	//	We're going to generate an array of keys for each row in the table.

	TArray<CString> Keys;
	Keys.GrowToFit(dStruct.GetElement(Cols[0]).GetCount());

	if (Cols.GetCount() == 1)
		{
		CDatum dCol = dStruct.GetElement(Cols[0]);
		for (int iRow = 0; iRow < dCol.GetCount(); iRow++)
			Keys.Insert(AsIndexKeyFromValue(dCol.GetElement(iRow)));
		}
	else
		{
		TArray<CDatum> ColData;
		ColData.GrowToFit(Cols.GetCount());
		for (int i = 0; i < Cols.GetCount(); i++)
			ColData.Insert(dStruct.GetElement(Cols[i]));

		for (int iRow = 0; iRow < dStruct.GetElement(Cols[0]).GetCount(); iRow++)
			{
			CStringBuffer Result;
			for (int i = 0; i < ColData.GetCount(); i++)
				{
				if (i != 0)
					Result.WriteChar('/');

				Result.Write(AsIndexKeyFromValue(ColData[i].GetElement(iRow)));
				}

			Keys.Insert(std::move(Result));
			}
		}

	//	Now we sort the keys

	m_Index = Keys.SortToMap();
	}

TSortMap<CString, int> CAEONTableIndex::Merge (const CAEONTableIndex& Src) const

//	Merge
//
//	Merges the source into this index. The rowIDs in the source are converted to
//	negative numbers so that we can distinguish them.

	{
	//	Set up

	int iSrcPos = 0;
	int iDestPos = 0;

	TSortMap<CString, int> Result;

	//	Merge

	while (iSrcPos < Src.m_Index.GetCount() && iDestPos < m_Index.GetCount())
		{
		int iCompare = ::KeyCompare(Src.m_Index.GetKey(iSrcPos), m_Index.GetKey(iDestPos));

		//	If the same key, replace

		if (iCompare == 0)
			{
			Result.InsertSorted(Src.m_Index.GetKey(iSrcPos), -(Src.m_Index[iSrcPos] + 1));
			iSrcPos++;
			iDestPos++;
			}

		//	If the source key is less than the destination key, insert from source

		else if (iCompare < 0)
			{
			Result.InsertSorted(Src.m_Index.GetKey(iSrcPos), -(Src.m_Index[iSrcPos] + 1));
			iSrcPos++;
			}

		//	If the source key is greater than the destination key, insert from destination

		else
			{
			Result.InsertSorted(m_Index.GetKey(iDestPos), m_Index[iDestPos]);
			iDestPos++;
			}
		}

	//	If there are remaining elements in the source index, add them

	while (iSrcPos < Src.m_Index.GetCount())
		{
		Result.InsertSorted(Src.m_Index.GetKey(iSrcPos), -(Src.m_Index[iSrcPos] + 1));
		iSrcPos++;
		}

	//	If there are remaining elements in the destination index, add them

	while (iDestPos < m_Index.GetCount())
		{
		Result.InsertSorted(m_Index.GetKey(iDestPos), m_Index[iDestPos]);
		iDestPos++;
		}

	return Result;
	}

void CAEONTableIndex::Remove (CDatum dTable, int iRow)

//	Remove
//
//	Remove the given row from the index.

	{
	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		return;

	m_Index.DeleteAt(GetIndexKey(*pTable, iRow));
	}
