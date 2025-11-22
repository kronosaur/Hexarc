//	CAEONTableRowRef.cpp
//
//	CAEONTableRowRef classes
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DATATYPE_SPECIAL,			"__datatype__");

DECLARE_CONST_STRING(TYPENAME_OBJECT,					"object");
DECLARE_CONST_STRING(TYPENAME_TABLE_ROW_REF,			"tableRowRef");

TDatumPropertyHandler<CAEONTableRowRef> CAEONTableRowRef::m_Properties = {
	{
		"columns",
		"$ArrayOfString",
		"Returns an array of keys.",
		[](const CAEONTableRowRef& Obj, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			dResult.GrowToFit(Obj.GetCount());
			for (int i = 0; i < Obj.GetCount(); i++)
				dResult.Append(Obj.GetKey(i));

			return dResult;
			},
		NULL,
		},
	{
		"datatype",
		"%",
		"Returns the type of the struct.",
		[](const CAEONTableRowRef &Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"keys",
		"a",
		"Returns an array of keys.",
		[](const CAEONTableRowRef& Obj, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			dResult.GrowToFit(Obj.GetCount());
			for (int i = 0; i < Obj.GetCount(); i++)
				dResult.Append(Obj.GetKey(i));

			return dResult;
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the number of entries in the struct.",
		[](const CAEONTableRowRef& Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	};

TDatumMethodHandler<CAEONTableRowRef> CAEONTableRowRef::m_Methods = {
	{
		"deleteAt",
		"*",
		".deleteAt(x) -> true/false.",
		0,
		[](CAEONTableRowRef& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(false);
			return true;
			},
		},
	};

CString CAEONTableRowRef::AsString () const

//	AsString
//
//	Returns a string representation.

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return AsAddress();

	CStringBuffer Output;

	Output.Write("{", 1);

	for (int i = 0; i < GetCount(); i++)
		{
		if (i != 0)
			Output.Write(" ", 1);

		Output.Write(GetKey(i));
		Output.Write(":", 1);

		Output.Write(GetElement(i).AsString());
		}

	Output.Write("}", 1);

	CString sOutput;
	sOutput.TakeHandoff(Output);
	return sOutput;
	}

size_t CAEONTableRowRef::CalcMemorySize () const

//	CalcMemorySize
//
//	Calc memory used.

	{
	return sizeof(*this);
	}

IComplexDatum* CAEONTableRowRef::Clone (CDatum::EClone iMode) const

//	Clone
//
//	Clone a copy.

	{
	const IAEONTable* pTable = ResolveTable();
	if (!pTable)
		return new CComplexStruct;

	CDatum dSchema = GetDatatype();
	CAEONObject* pObj = new CAEONObject(dSchema);

	const IDatatype &Schema = dSchema;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);
		pObj->SetElement(ColumnDesc.sID, pTable->GetFieldValue(m_iRow, i));
		}

	return pObj;
	}

bool CAEONTableRowRef::Contains (CDatum dValue) const

//	Contains
//
//	Returns TRUE if we contain the given datum.

	{
	for (int i = 0; i < GetCount(); i++)
		if (GetElement(i).Contains(dValue))
			return true;

	return false;
	}

bool CAEONTableRowRef::FindElement (const CString &sKey, CDatum* retpValue) const

//	FindElement
//
//	Returns TRUE if we have the given element.

	{
	const IAEONTable* pTable = ResolveTable();
	if (!pTable)
		return false;

	int iCol;
	if (!pTable->FindCol(sKey, &iCol))
		return false;

	if (retpValue)
		*retpValue = pTable->GetFieldValue(m_iRow, iCol);

	return true;
	}

int CAEONTableRowRef::GetCount () const

//	GetCount
//
//	Returns the number of elements.

	{
	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return 0;

	return pTable->GetColCount();
	}

CDatum CAEONTableRowRef::GetDatatype () const

//	GetDatatype
//
//	Returns the datatype of the value.

	{
	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return CAEONTypes::Get(IDatatype::NULL_T);

	return pTable->GetSchema();
	}

CDatum CAEONTableRowRef::GetElement (int iIndex) const

//	GetElement
//
//	Returns the given element.

	{
	const IAEONTable* pTable = ResolveTable();
	if (!pTable)
		return CDatum();

	return pTable->GetFieldValue(m_iRow, iIndex);
	}

CDatum CAEONTableRowRef::GetElement (const CString &sKey) const

//	GetElement
//
//	Returns the given element.

	{
	const IAEONTable* pTable = ResolveTable();
	if (!pTable)
		return CDatum();

	int iCol;
	if (!pTable->FindCol(sKey, &iCol))
		return CDatum();

	return pTable->GetFieldValue(m_iRow, iCol);
	}

CDatum CAEONTableRowRef::GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const

//	GetElementAt
//
//	Returns the given element.

	{
	int iIndex;

	if (dIndex.IsNil())
		return CDatum();

	else if (dIndex.IsNumberInt32(&iIndex))
		return GetElement(iIndex);

	else if (dIndex.IsContainer())
		{
		CDatum dResult(CDatum::typeStruct);

		for (int i = 0; i < dIndex.GetCount(); i++)
			{
			CDatum dEntry = dIndex.GetElement(i);
			if (dEntry.IsNil())
				{ }
			else if (dEntry.IsNumberInt32(&iIndex))
				{
				if (iIndex >= 0 && iIndex < GetCount())
					dResult.SetElement(GetKey(iIndex), GetElement(iIndex));
				}
			else
				{
				CString sKey = dEntry.AsString();
				CDatum dValue;
				if (FindElement(sKey, &dValue))
					dResult.SetElement(sKey, dValue);
				}
			}

		return dResult;
		}
	else
		{
		return GetElement(dIndex.AsString());
		}
	}

CString CAEONTableRowRef::GetKey (int iIndex) const

//	GetKey
//
//	Returns the key at the given index.

	{
	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return NULL_STR;

	IDatatype::SMemberDesc ColDesc = pTable->GetColDesc(m_dTable, iIndex);
	return ColDesc.sID;
	}

CDatum CAEONTableRowRef::GetMethod (const CString &sMethod) const

//	GetMethod
//
//	Returns the given method.

	{
	CDatum dValue = GetElement(sMethod);
	if (dValue.CanInvoke())
		return dValue;

	return m_Methods.GetMethod(sMethod);
	}

CDatum CAEONTableRowRef::GetProperty (const CString& sProperty) const

//	GetProperty
//
//	Returns the given property.

	{
	CDatum dValue;
	if (FindElement(sProperty, &dValue))
		return dValue;

	return m_Properties.GetProperty(*this, sProperty);
	}

const CString& CAEONTableRowRef::GetTypename () const

//	GetTypename
//
//	Returns the typename of the value.

	{
	return TYPENAME_TABLE_ROW_REF;
	}

bool CAEONTableRowRef::IsNil (void) const

//	IsNil
//
//	Returns TRUE if the value is nil.

	{
	const IAEONTable* pTable = ResolveTable();
	if (!pTable)
		return true;

	return false;
	}

size_t CAEONTableRowRef::OnCalcSerializeSizeAEONScript(CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns the size of the serialized object.

	{
	return 0;
	}

int CAEONTableRowRef::OpCompare (CDatum::Types iValueType, CDatum dValue) const

//	OpCompare
//
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2

	{
	if (!dValue.IsStruct())
		return KeyCompareNoCase(AsString(), dValue.AsString());

	int iCount = Min(GetCount(), dValue.GetCount());
	for (int i = 0; i < iCount; i++)
		{
		int iCompare = KeyCompareNoCase(GetKey(i), dValue.GetKey(i));
		if (iCompare != 0)
			return iCompare;

		iCompare = GetElement(i).OpCompare(dValue.GetElement(i));
		if (iCompare != 0)
			return iCompare;
		}

	return KeyCompare(GetCount(), dValue.GetCount());
	}

int CAEONTableRowRef::OpCompareExact (CDatum::Types iValueType, CDatum dValue) const

//	OpCompareExact
//
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2

	{
	if (!dValue.IsStruct())
		return KeyCompare(AsString(), dValue.AsString());

	int iCount = Min(GetCount(), dValue.GetCount());
	for (int i = 0; i < iCount; i++)
		{
		int iCompare = KeyCompare(GetKey(i), dValue.GetKey(i));
		if (iCompare != 0)
			return iCompare;

		iCompare = GetElement(i).OpCompareExact(dValue.GetElement(i));
		if (iCompare != 0)
			return iCompare;
		}

	return KeyCompare(GetCount(), dValue.GetCount());
	}

bool CAEONTableRowRef::OpIsEqual (CDatum::Types iValueType, CDatum dValue) const

//	OpIsEqual
//
//	Returns TRUE if we are equal to the given value.

	{
	if (!dValue.IsStruct())
		return false;

	if (GetCount() != dValue.GetCount())
		return false;

	for (int i = 0; i < GetCount(); i++)
		if (!strEqualsNoCase(GetKey(i), dValue.GetKey(i))
				|| !GetElement(i).OpIsEqual(dValue.GetElement(i)))
			return false;

	return true;
	}

bool CAEONTableRowRef::OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const

//	OpIsIdentical
//
//	Returns TRUE if we are identical to the given value.

	{
	if ((const IDatatype&)GetDatatype() != (const IDatatype&)dValue.GetDatatype())
		return false;

	if (GetCount() != dValue.GetCount())
		return false;

	for (int i = 0; i < GetCount(); i++)
		if (!strEqualsNoCase(GetKey(i), dValue.GetKey(i))
				|| !GetElement(i).OpIsIdentical(dValue.GetElement(i)))
			return false;

	return true;
	}

IAEONTable* CAEONTableRowRef::ResolveTable ()

//	ResolveTable
//
//	Returns a table pointer if we have a valid table and row. Otherwise, we 
//	return NULL.

	{
	IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return NULL;

	if (m_iRow < 0 || m_iRow >= pTable->GetRowCount())
		return NULL;

	return pTable;
	}

const IAEONTable* CAEONTableRowRef::ResolveTable () const

//	ResolveTable
//
//	Returns a table pointer if we have a valid table and row. Otherwise, we 
//	return NULL.

	{
	const IAEONTable* pTable = m_dTable.GetTableInterface();
	if (!pTable)
		return NULL;

	if (m_iRow < 0 || m_iRow >= pTable->GetRowCount())
		return NULL;

	return pTable;
	}

void CAEONTableRowRef::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serialize the object to a stream.

	{
	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			Stream.Write("[", 1);
			Stream.Write(TYPENAME_OBJECT);
			Stream.Write(":", 1);

			//	Write out the datatype

			GetDatatype().Serialize(iFormat, Stream);
			Stream.Write(":", 1);

			//	Now write out each member variable as a structure.

			Stream.Write("{", 1);

			for (int i = 0; i < GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(" ", 1);

				//	Write the key

				CDatum Key(GetKey(i));
				Key.Serialize(iFormat, Stream);

				//	Separator

				Stream.Write(":", 1);

				//	Write the value

				GetElement(i).Serialize(iFormat, Stream);
				}

			Stream.Write("}]", 2);
			break;
			}

		case CDatum::EFormat::GridLang:
			SerializeAsStruct(iFormat, Stream);
			break;

		//	For JSON format we just serialize as a plain Struct but write out 
		//	the datatype in the special __datatype__ field.

		case CDatum::EFormat::JSON:
			{
			Stream.Write("{\"", 2);

			FIELD_DATATYPE_SPECIAL.SerializeJSON(Stream);
			Stream.Write("\": ", 3);
			GetDatatype().Serialize(iFormat, Stream);


			for (int i = 0; i < GetCount(); i++)
				{
				Stream.Write(", ", 2);

				//	Write the key

				CDatum Key(GetKey(i));
				Key.Serialize(iFormat, Stream);

				//	Separator

				Stream.Write(": ", 2);

				//	Write the value

				GetElement(i).Serialize(iFormat, Stream);
				}

			Stream.Write("}", 1);
			break;
			}

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

void CAEONTableRowRef::SetElement (const CString &sKey, CDatum dDatum)

//	SetElement
//
//	Sets the given element.

	{
	IAEONTable* pTable = ResolveTable();
	if (!pTable)
		return;

	int iCol;
	if (!pTable->FindCol(sKey, &iCol))
		return;

	pTable->SetFieldValue(m_iRow, iCol, dDatum);
	}

void CAEONTableRowRef::SetElementAt (CDatum dIndex, CDatum dDatum)

//	SetElementAt
//
//	Sets the given element.

	{
	IAEONTable* pTable = ResolveTable();
	if (!pTable)
		return;

	if (dIndex.IsNil())
		{ }
	else if (dIndex.IsNumberInt32())
		{
		int iIndex = dIndex;
		pTable->SetFieldValue(m_iRow, iIndex, dDatum);
		}
	else
		{
		SetElement(dIndex.AsString(), dDatum);
		}
	}
