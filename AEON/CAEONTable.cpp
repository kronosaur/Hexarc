//	CAEONTable.cpp
//
//	CAEONTable classes
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_TABLE,					"table");

void CAEONTable::Append (CDatum dDatum)

//	Append
//
//	Append a row, column, or table.

	{
	switch (dDatum.GetBasicType())
		{
		case CDatum::typeArray:
			AppendColumn(dDatum);
			break;

		case CDatum::typeStruct:
			AppendRow(dDatum);
			break;

		case CDatum::typeTable:
			AppendTable(dDatum);
			break;

		default:
			break;
		}
	}

IAEONTable::EResult CAEONTable::AppendColumn (CDatum dColumn)

//	AppendColumn
//
//	Appends a column.

	{
	return EResult::NotImplemented;
	}

IAEONTable::EResult CAEONTable::AppendRow (CDatum dRow)

//	AppendRow
//
//	Appends a row.

	{
	const IDatatype &Schema = m_dSchema;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		CDatum dValue = dRow.GetElement(ColumnDesc.sName);

		//	Make sure we're not trying to add ourselves.

		TArray<IComplexDatum *> Checked1;
		TArray<IComplexDatum *> Checked2;
		if (dValue.Contains(CDatum::raw_AsComplex(this), Checked1) || dValue.Contains(m_Cols[i], Checked2))
			dValue = CDatum();

		//	Add it

		m_Cols[i].Append(dValue);
		}

	m_iRows++;
	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::AppendSlice (CDatum dSlice)

//	AppendSlice
//
//	Appends a slice of data using the current schema.

	{
	if (m_Cols.GetCount() == 0 || dSlice.GetCount() == 0)
		return EResult::OK;

	int iRows = dSlice.GetCount() / m_Cols.GetCount();
	for (int iCol = 0; iCol < m_Cols.GetCount(); iCol++)
		m_Cols[iCol].GrowToFit(iRows);

	int i = 0;
	for (int iRow = 0; iRow < iRows; iRow++)
		{
		for (int iCol = 0; iCol < m_Cols.GetCount(); iCol++)
			{
			m_Cols[iCol].Append(dSlice.GetElement(i));

			i++;
			}
		}

	m_iRows += iRows;
	return EResult::OK;
	}

IAEONTable::EResult CAEONTable::AppendTable (CDatum dTable)

//	AppendTable
//
//	Appends a table.

	{
	GrowToFit(dTable.GetCount());

	for (int i = 0; i < dTable.GetCount(); i++)
		{
		AppendRow(dTable.GetElement(i));
		}

	return EResult::OK;
	}

CString CAEONTable::AsString (void) const

//	AsString
//
//	Represent as a string

	{
	CStringBuffer Buffer;

	//	Output header

	const IDatatype &Schema = m_dSchema;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		if (i != 0)
			Buffer.WriteChar('\t');

		Buffer.Write(ColumnDesc.sName);
		}

	Buffer.WriteChar('\n');

	//	Output each value

	for (int iRow = 0; iRow < m_iRows; iRow++)
		{
		for (int iCol = 0; iCol < m_Cols.GetCount(); iCol++)
			{
			if (iCol != 0)
				Buffer.WriteChar('\t');

			Buffer.Write(m_Cols[iCol].GetElement(iRow).AsString());
			}

		Buffer.WriteChar('\n');
		}

	return CString(Buffer);
	}

CDatum CAEONTable::CalcColumnDatatype (CDatum dValue)

//	CalcColumnDatatype
//
//	Computes the column datatype based on a value.

	{
	if (dValue.GetBasicType() == CDatum::typeArray)
		{
		return dValue.GetElement(0).GetDatatype();
		}
	else
		{
		return dValue.GetDatatype();
		}
	}

size_t CAEONTable::CalcMemorySize (void) const

//	CalcMemorySize
//
//	Returns number of bytes used.

	{
	size_t iSize = 0;

	for (int i = 0; i < m_Cols.GetCount(); i++)
		iSize += m_Cols[i].CalcMemorySize();

	return iSize;
	}

CDatum CAEONTable::CreateTableFromArray (CAEONTypeSystem &TypeSystem, CDatum dValue)

//	CreateTableFromArray
//
//	Creates a table from an array, creating a schema as necessary.

	{
	return CDatum();
	}

CDatum CAEONTable::CreateTableFromDatatype (CAEONTypeSystem &TypeSystem, CDatum dType)

//	CreateTableFromDatatype
//
//	Creates an empty table from a datatype, creating a schema if necessary.

	{
	const IDatatype &Type = dType;

	//	If this is a schema datatype, then we're done.

	if (Type.GetClass() == IDatatype::ECategory::Schema)
		return CDatum::CreateTable(dType);

	//	Otherwise, nothing.

	else
		return CDatum();
	}

CDatum CAEONTable::CreateTableFromStruct (CAEONTypeSystem &TypeSystem, CDatum dValue)

//	CreateTableFromStruct
//
//	Creates a table from a structure, generating a schema, if necessary.

	{
	int iRows = 0;

	//	We treat each field of the struct as a column.

	TArray<IDatatype::SMemberDesc> Columns;
	Columns.GrowToFit(dValue.GetCount());
	for (int i = 0; i < dValue.GetCount(); i++)
		{
		CDatum dColumnValues = dValue.GetElement(i);

		auto pNewColumn = Columns.Insert();
		pNewColumn->iType = IDatatype::EMemberType::InstanceVar;
		pNewColumn->sName = dValue.GetKey(i);
		pNewColumn->dType = CalcColumnDatatype(dColumnValues);

		if (dColumnValues.GetBasicType() == CDatum::typeArray)
			iRows = Max(iRows, dColumnValues.GetCount());
		else
			iRows = Max(iRows, 1);
		}

	//	Create a new schema.

	CDatum dSchema = TypeSystem.AddAnonymousSchema(Columns);
	if (dSchema.IsNil())
		return CDatum();

	//	Create the table.

	CAEONTable *pNewTable = new CAEONTable(dSchema);
	pNewTable->GrowToFit(iRows);
	pNewTable->m_iRows = iRows;

	for (int i = 0; i < dValue.GetCount(); i++)
		{
		CDatum dColumnValues = dValue.GetElement(i);
		int iValueCount = dColumnValues.GetCount();
		if (iValueCount == 0)
			{
			for (int iRow = 0; iRow < iRows; iRow++)
				{
				pNewTable->m_Cols[i].Append(CDatum());
				}
			}
		else
			{
			for (int iRow = 0; iRow < iRows; iRow++)
				{
				pNewTable->m_Cols[i].Append(dColumnValues.GetElement(iRow % iValueCount));
				}
			}
		}

	return CDatum(pNewTable);
	}

IAEONTable::EResult CAEONTable::DeleteAllRows ()

//	DeleteAllRows
//
//	Deletes all rows (but leaves the schema intact).

	{
	//	Recreate the columns.

	SetSchema(m_dSchema);
	return EResult::OK;
	}

bool CAEONTable::FindCol (const CString &sName, int *retiCol) const

//	FindCol
//
//	Find column.

	{
	const IDatatype &Schema = m_dSchema;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		if (strEqualsNoCase(sName, Schema.GetMember(i).sName))
			{
			if (retiCol)
				*retiCol = i;

			return true;
			}
		}

	return false;
	}

int CAEONTable::GetColCount () const

//	GetColCount
//
//	Returns the number of column.

	{
	return m_Cols.GetCount();
	}

CString CAEONTable::GetColName (int iCol) const

//	GetColName
//
//	Returns the name of the column.

	{
	const IDatatype &Schema = m_dSchema;
	if (iCol < 0 || iCol >= Schema.GetMemberCount())
		throw CException(errFail);

	return Schema.GetMember(iCol).sName;
	}

CDatum CAEONTable::GetDataSlice (int iFirstRow, int iRowCount) const

//	GetDataSlice
//
//	Returns a 2D array of values.

	{
	if (iFirstRow < 0 || iFirstRow >= m_iRows || GetColCount() == 0)
		return CDatum();

	int iLastRow = Min(iFirstRow + iRowCount, m_iRows);

	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(GetColCount() * (iLastRow - iFirstRow));

	for (int iRow = iFirstRow; iRow < iLastRow; iRow++)
		{
		for (int iCol = 0; iCol < GetColCount(); iCol++)
			{
			dResult.Append(m_Cols[iCol].GetElement(iRow));
			}
		}

	return dResult;
	}

CDatum CAEONTable::GetFieldValue (int iRow, int iCol) const

//	GetFieldValue
//
//	Returns the given field.

	{
	if (iRow < 0 || iRow >= m_iRows)
		return CDatum();

	if (iCol < 0 || iCol >= m_Cols.GetCount())
		return CDatum();

	return m_Cols[iCol].GetElement(iRow);
	}

int CAEONTable::GetRowCount () const

//	GetRowCount
//
//	Returns the number of rows.

	{
	return m_iRows;
	}

CDatum CAEONTable::GetElement (int iIndex) const

//	GetElement
//
//	Return the nth row of the table (as a struct).

	{
	if (iIndex < 0 || iIndex >= m_iRows)
		return CDatum();

	CDatum dRow(CDatum::typeStruct);

	const IDatatype &Schema = m_dSchema;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		dRow.SetElement(ColumnDesc.sName, m_Cols[i].GetElement(iIndex));
		}

	return dRow;
	}

const CString &CAEONTable::GetTypename (void) const

//	GetTypename
//
//	Returns the typename.

	{
	return TYPENAME_TABLE;
	}

void CAEONTable::GrowToFit (int iCount)

//	GrowToFit
//
//	Allocate to fit an additional n rows.

	{
	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i].GrowToFit(iCount);
	}

bool CAEONTable::IsSameSchema (CDatum dSchema) const

//	IsSameSchema
//
//	Returns TRUE if the given schema has the exact same columns.

	{
	const IDatatype &Schema = m_dSchema;
	const IDatatype &TestSchema = dSchema;
	if (TestSchema.GetClass() != IDatatype::ECategory::Schema)
		return false;

	if (TestSchema.GetMemberCount() != Schema.GetMemberCount())
		return false;

	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		IDatatype::SMemberDesc Member = Schema.GetMember(i);

		CDatum dTestType;
		if (TestSchema.HasMember(Member.sName, &dTestType) != IDatatype::EMemberType::InstanceVar)
			return false;

		if (!Member.dType.IsEqual(dTestType))
			return false;
		}

	return true;
	}

size_t CAEONTable::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Estimate of how much space is required to serialize.

	{
	return CalcMemorySize();
	}

void CAEONTable::OnMarked (void)

//	OnMarked
//
//	Mark data in use

	{
	m_dSchema.Mark();

	for (int i = 0; i < m_Cols.GetCount(); i++)
		m_Cols[i].Mark();
	}

void CAEONTable::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serialize

	{
	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			Stream.Write("(", 1);

			for (int i = 0; i < m_iRows; i++)
				{
				if (i != 0)
					Stream.Write(" ", 1);

				SerializeRow(Stream, iFormat, i);
				}

			Stream.Write(")", 1);
			break;
			}

		case CDatum::EFormat::JSON:
			{
			Stream.Write("[", 1);

			for (int i = 0; i < m_iRows; i++)
				{
				if (i != 0)
					Stream.Write(", ", 2);

				SerializeRow(Stream, iFormat, i);
				}

			Stream.Write("]", 1);
			break;
			}

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

void CAEONTable::SerializeRow (IByteStream &Stream, CDatum::EFormat iFormat, int iRow) const

//	SerializeRow
//
//	Serializes the given row.

	{
	const IDatatype &Schema = m_dSchema;

	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			Stream.Write("{", 1);

			for (int i = 0; i < Schema.GetMemberCount(); i++)
				{
				auto ColumnDef = Schema.GetMember(i);

				if (i != 0)
					Stream.Write(" ", 1);

				//	Write the key

				CDatum Key(ColumnDef.sName);
				Key.Serialize(iFormat, Stream);

				//	Separator

				Stream.Write(":", 1);

				//	Write the value

				m_Cols[i].GetElement(iRow).Serialize(iFormat, Stream);
				}

			Stream.Write("}", 1);
			break;
			}

		case CDatum::EFormat::JSON:
			{
			Stream.Write("{", 1);

			for (int i = 0; i < GetCount(); i++)
				{
				auto ColumnDef = Schema.GetMember(i);

				if (i != 0)
					Stream.Write(", ", 2);

				//	Write the key

				CDatum Key(ColumnDef.sName);
				Key.Serialize(iFormat, Stream);

				//	Separator

				Stream.Write(": ", 2);

				//	Write the value

				m_Cols[i].GetElement(iRow).Serialize(iFormat, Stream);
				}

			Stream.Write("}", 1);
			break;
			}

		default:
			CDatum().Serialize(iFormat, Stream);
			break;
		}
	}

void CAEONTable::SetElement (int iIndex, CDatum dDatum)

//	SetElement
//
//	Sets the nth record with the given struct data.

	{
	if (iIndex < 0 || iIndex >= m_iRows)
		return;

	const IDatatype &Schema = m_dSchema;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		CDatum dValue = dDatum.GetElement(ColumnDesc.sName);
		m_Cols[i].SetElement(iIndex, dValue);
		}
	}

void CAEONTable::SetSchema (CDatum dSchema)

//	SetSchema
//
//	Sets the schema. This will delete all existing data in the table.

	{
	//	Make sure this is an actual schema.

	const IDatatype &Schema = dSchema;
	if (Schema.GetClass() != IDatatype::ECategory::Schema)
		throw CException(errFail);

	//	Reset

	m_iRows = 0;
	m_dSchema = dSchema;
	m_Cols.DeleteAll();
	m_Cols.InsertEmpty(Schema.GetMemberCount());

	//	Set the columns (OK if no members)

	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);

		if (ColumnDesc.iType != IDatatype::EMemberType::InstanceVar
				|| ColumnDesc.dType.GetBasicType() != CDatum::typeDatatype
				|| ColumnDesc.sName.IsEmpty())
			throw CException(errFail);

		//	Create columns based on the datatype

		const IDatatype &ColSchema = ColumnDesc.dType;
		switch (ColSchema.GetCoreType())
			{
			case IDatatype::INT_32:
				m_Cols[i] = CDatum::VectorOf(CDatum::typeInteger32);
				break;

			case IDatatype::FLOAT_64:
				m_Cols[i] = CDatum::VectorOf(CDatum::typeDouble);
				break;

			case IDatatype::STRING:
				m_Cols[i] = CDatum::VectorOf(CDatum::typeString);
				break;

			default:
				//	For anything else, we create a generic array.

				m_Cols[i] = CDatum::VectorOf(CDatum::typeUnknown);
				break;
			}
		}
	}
