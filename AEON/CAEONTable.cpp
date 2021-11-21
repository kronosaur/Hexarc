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
		m_Cols[i].Append(dValue);
		}

	m_iRows++;
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

IAEONTable::EResult CAEONTable::DeleteAllRows ()

//	DeleteAllRows
//
//	Deletes all rows (but leaves the schema intact).

	{
	//	Recreate the columns.

	const IDatatype &Schema = m_dSchema;
	if (Schema.GetClass() != IDatatype::ECategory::Schema)
		return EResult::NotATable;

	SetSchema(m_dSchema);
	return EResult::OK;
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
	throw CException(errFail);
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
