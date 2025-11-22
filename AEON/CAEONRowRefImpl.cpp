//	CAEONRowRefImpl.cpp
//
//	CAEONRowRefImpl class
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"
#include <string_view>

DECLARE_CONST_STRING(TYPENAME_ROW_REF,					"rowRef");

TDatumPropertyHandler<DWORDLONG> CAEONRowRefImpl::m_Properties = {
	{
		"columns",
		"$ArrayOfString",
		"Returns an array of keys.",
		[](const DWORDLONG& dwData, const CString &sProperty)
			{
			return CAEONRowRefImpl::GetKeys(dwData);
			},
		NULL,
		},
	{
		"datatype",
		"%",
		"Returns the type of the struct.",
		[](const DWORDLONG& dwData, const CString &sProperty)
			{
			return CAEONRowRefImpl::GetDatatype(dwData);
			},
		NULL,
		},
	{
		"keys",
		"a",
		"Returns an array of keys.",
		[](const DWORDLONG& dwData, const CString &sProperty)
			{
			return CAEONRowRefImpl::GetKeys(dwData);
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the number of entries in the struct.",
		[](const DWORDLONG& dwData, const CString &sProperty)
			{
			return CDatum(GetCount(dwData));
			},
		NULL,
		},
	};

TDatumMethodHandler<DWORDLONG> CAEONRowRefImpl::m_Methods = {
	{
		"deleteAt",
		"*",
		".deleteAt(x) -> true/false.",
		0,
		[](DWORDLONG& dwData, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(false);
			return true;
			},
		},
	};

CString CAEONRowRefImpl::AsString (DWORDLONG dwData)
	{
	const IAEONTable* pTable = GetTable(dwData).GetTableInterface();
	if (!pTable)
		return strPattern("%s %08x%08x", (LPCSTR)GetTypename(dwData), (DWORD)(dwData >> 32), (DWORD)dwData);

	int iRow = DecodeRowIndex(dwData);

	CStringBuffer Output;

	Output.Write("{", 1);

	for (int i = 0; i < pTable->GetColCount(); i++)
		{
		if (i != 0)
			Output.Write(" ", 1);

		Output.Write(pTable->GetColName(i));
		Output.Write(":", 1);

		Output.Write(pTable->GetFieldValue(iRow, i).AsString());
		}

	Output.Write("}", 1);

	CString sOutput;
	sOutput.TakeHandoff(Output);
	return sOutput;
	}

CDatum CAEONRowRefImpl::AsStruct (DWORDLONG dwData)
	{
	const IAEONTable* pTable = GetTable(dwData).GetTableInterface();
	if (!pTable)
		return strPattern("%s %08x%08x", (LPCSTR)GetTypename(dwData), (DWORD)(dwData >> 32), (DWORD)dwData);

	int iRow = DecodeRowIndex(dwData);

	CDatum dResult(CDatum::typeStruct);
	for (int i = 0; i < pTable->GetColCount(); i++)
		dResult.SetElement(pTable->GetColName(i), pTable->GetFieldValue(iRow, i));

	return dResult;
	}

size_t CAEONRowRefImpl::CalcMemorySize (DWORDLONG dwData)
	{
	return 0;
	}

size_t CAEONRowRefImpl::CalcSerializeSizeAEONScript (DWORDLONG dwData, CDatum::EFormat iFormat)
	{
	return 0;
	}

CDatum CAEONRowRefImpl::Clone (DWORDLONG dwData, CDatum::EClone iMode)
	{
	const IAEONTable* pTable = GetTable(dwData).GetTableInterface();
	if (!pTable)
		return CDatum(CDatum::typeStruct);

	int iRow = DecodeRowIndex(dwData);

	CDatum dSchema = pTable->GetSchema();
	CAEONObject* pObj = new CAEONObject(dSchema);

	const IDatatype &Schema = dSchema;
	for (int i = 0; i < Schema.GetMemberCount(); i++)
		{
		auto ColumnDesc = Schema.GetMember(i);
		pObj->SetElement(ColumnDesc.sID, pTable->GetFieldValue(iRow, i));
		}

	return CDatum(pObj);
	}

bool CAEONRowRefImpl::Contains (DWORDLONG dwData, CDatum dValue)
	{
	const IAEONTable* pTable = GetTable(dwData).GetTableInterface();
	if (!pTable)
		return false;

	int iRow = DecodeRowIndex(dwData);

	for (int i = 0; i < pTable->GetColCount(); i++)
		{
		if (pTable->GetFieldValue(iRow, i).Contains(dValue))
			return true;
		}

	return false;
	}

bool CAEONRowRefImpl::EnumElements (DWORDLONG dwData, DWORD dwFlags, std::function<bool(CDatum)> fn)
	{
	return false;
	}

bool CAEONRowRefImpl::Find (DWORDLONG dwData, CDatum dValue, int* retiIndex)
	{
	const IAEONTable* pTable = GetTable(dwData).GetTableInterface();
	if (!pTable)
		return false;

	int iRow = DecodeRowIndex(dwData);

	for (int i = 0; i < pTable->GetColCount(); i++)
		{
		if (pTable->GetFieldValue(iRow, i).OpIsEqual(dValue))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}
		}

	return false;
	}

CDatum CAEONRowRefImpl::FindAll (DWORDLONG dwData, CDatum dValue)
	{
	return CDatum();
	}

CDatum CAEONRowRefImpl::FindAllExact (DWORDLONG dwData, CDatum dValue)
	{
	return CDatum();
	}

bool CAEONRowRefImpl::FindElement (DWORDLONG dwData, CStringView sKey, CDatum* retpValue)
	{
	const IAEONTable* pTable = GetTable(dwData).GetTableInterface();
	if (!pTable)
		return false;

	int iRow = DecodeRowIndex(dwData);
	int iCol;
	if (!pTable->FindCol(sKey, &iCol))
		return false;

	if (retpValue)
		*retpValue = pTable->GetFieldValue(iRow, iCol);

	return true;
	}

bool CAEONRowRefImpl::FindExact (DWORDLONG dwData, CDatum dValue, int* retiIndex)
	{
	return false;
	}

CDatum CAEONRowRefImpl::GetDatatype (DWORDLONG dwData)
	{
	const IAEONTable* pTable = GetTable(dwData).GetTableInterface();
	if (!pTable)
		return CAEONTypes::Get(IDatatype::NULL_T);

	return pTable->GetSchema();
	}

int CAEONRowRefImpl::GetCount (DWORDLONG dwData)
	{
	const IAEONTable* pTable = GetTable(dwData).GetTableInterface();
	if (!pTable)
		return 0;

	return pTable->GetColCount();
	}

CDatum CAEONRowRefImpl::GetElement (DWORDLONG dwData, IInvokeCtx* pCtx, int iIndex)
	{
	IAEONTable* pTable = GetTable(dwData).GetTableInterface();
	if (!pTable)
		return NULL;

	return pTable->GetFieldValue(DecodeRowIndex(dwData), iIndex);
	}

CDatum CAEONRowRefImpl::GetElement (DWORDLONG dwData, IInvokeCtx* pCtx, CStringView sKey)
	{
	IAEONTable* pTable = GetTable(dwData).GetTableInterface();
	if (!pTable)
		return CDatum();

	int iRow = DecodeRowIndex(dwData);
	int iCol;
	if (!pTable->FindCol(sKey, &iCol))
		return CDatum();

	return pTable->GetFieldValue(iRow, iCol);
	}

CDatum CAEONRowRefImpl::GetElementAt (DWORDLONG dwData, int iIndex)
	{
	return GetElement(dwData, NULL, iIndex);
	}

CDatum CAEONRowRefImpl::GetElementAt (DWORDLONG dwData, CAEONTypeSystem& TypeSystem, CDatum dIndex)
	{
	IAEONTable* pTable = GetTable(dwData).GetTableInterface();
	if (!pTable)
		return CDatum();

	int iRow = DecodeRowIndex(dwData);
	int iIndex;

	if (dIndex.IsNil())
		return CDatum();

	else if (dIndex.IsNumberInt32(&iIndex))
		return pTable->GetFieldValue(iRow, iIndex);

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
				if (iIndex >= 0 && iIndex < pTable->GetColCount())
					dResult.SetElement(pTable->GetColName(i), pTable->GetFieldValue(iRow, iIndex));
				}
			else
				{
				CString sKey = dEntry.AsString();
				int iCol;
				if (pTable->FindCol(sKey, &iCol))
					dResult.SetElement(sKey, pTable->GetFieldValue(iRow, iCol));
				}
			}

		return dResult;
		}
	else
		{
		int iCol;
		if (!pTable->FindCol(dIndex.AsString(), &iCol))
			return CDatum();

		return pTable->GetFieldValue(iRow, iCol);
		}
	}

CString CAEONRowRefImpl::GetKey (DWORDLONG dwData, int iIndex)
	{
	CDatum dTable = GetTable(dwData);
	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		return NULL_STR;

	IDatatype::SMemberDesc ColDesc = pTable->GetColDesc(dTable, iIndex);
	return ColDesc.sID;
	}

CDatum CAEONRowRefImpl::GetKeys (DWORDLONG dwData)
	{
	CDatum dTable = GetTable(dwData);
	const IAEONTable* pTable = dTable.GetTableInterface();
	if (!pTable)
		return CDatum();

	CDatum dResult(CDatum::typeArray);
	for (int i = 0; i < pTable->GetColCount(); i++)
		dResult.Append(pTable->GetColName(i));

	return dResult;
	}

CDatum CAEONRowRefImpl::GetMethod (DWORDLONG dwData, const CString &sMethod)
	{
	CDatum dValue = GetElement(dwData, NULL, sMethod);
	if (dValue.CanInvoke())
		return dValue;

	return m_Methods.GetMethod(sMethod);
	}

CDatum CAEONRowRefImpl::GetProperty (DWORDLONG dwData, const CString &sKey)
	{
	CDatum dValue;
	if (FindElement(dwData, sKey, &dValue))
		return dValue;

	return m_Properties.GetProperty(dwData, sKey);
	}

const CString& CAEONRowRefImpl::GetTypename (DWORDLONG dwData)
	{
	return TYPENAME_ROW_REF;
	}

bool CAEONRowRefImpl::IsNil (DWORDLONG dwData)
	{
	IAEONTable* pTable = GetTable(dwData).GetTableInterface();
	if (!pTable)
		return true;

	int iRow = DecodeRowIndex(dwData);
	if (iRow < 0 || iRow >= pTable->GetRowCount())
		return true;

	return false;
	}

size_t CAEONRowRefImpl::Hash (DWORDLONG dwData)
	{
	return std::hash<std::string_view>{}((LPCSTR)strToLower(AsString(dwData)));
	}

void CAEONRowRefImpl::Mark (DWORDLONG dwData)
	{
	GetTable(dwData).Mark();
	}

int CAEONRowRefImpl::OpCompare (DWORDLONG dwSrcData, DWORDLONG dwDestData)

//	OpCompare
//
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2

	{
	IAEONTable* pSrcTable = GetTable(dwSrcData).GetTableInterface();
	int iSrcRow = DecodeRowIndex(dwSrcData);

	IAEONTable* pDestTable = GetTable(dwDestData).GetTableInterface();
	int iDestRow = DecodeRowIndex(dwDestData);

	if (!pSrcTable && !pDestTable)
		return 0;
	else if (!pSrcTable)
		return -1;
	else if (!pDestTable)
		return 1;
	else
		{
		TArray<int> SrcOrder = pSrcTable->GetColsInSortedOrder();
		TArray<int> DestOrder = pDestTable->GetColsInSortedOrder();

		int iCount = Min(SrcOrder.GetCount(), DestOrder.GetCount());
		for (int i = 0; i < iCount; i++)
			{
			int iSrcCol = SrcOrder[i];
			int iDestCol = DestOrder[i];

			int iCompare = KeyCompareNoCase(pSrcTable->GetColName(iSrcCol), pDestTable->GetColName(iDestCol));
			if (iCompare != 0)
				return iCompare;

			iCompare = pSrcTable->GetFieldValue(iSrcRow, iSrcCol).OpCompare(pDestTable->GetFieldValue(iDestRow, iDestCol));
			if (iCompare != 0)
				return iCompare;
			}

		return KeyCompare(SrcOrder.GetCount(), DestOrder.GetCount());
		}
	}

int CAEONRowRefImpl::OpCompare (DWORDLONG dwSrcData, CDatum dDestStruct)
	{
	IAEONTable* pSrcTable = GetTable(dwSrcData).GetTableInterface();
	int iSrcRow = DecodeRowIndex(dwSrcData);

	if (!pSrcTable && !dDestStruct.IsStruct())
		return 0;
	else if (!pSrcTable)
		return -1;
	else if (!dDestStruct.IsStruct())
		return 1;
	else
		{
		TArray<int> SrcOrder = pSrcTable->GetColsInSortedOrder();
		TArray<int> DestOrder = dDestStruct.GetKeysInSortedOrder();

		int iCount = Min(SrcOrder.GetCount(), DestOrder.GetCount());
		for (int i = 0; i < iCount; i++)
			{
			int iSrcCol = SrcOrder[i];
			int iDestCol = DestOrder[i];

			int iCompare = KeyCompareNoCase(pSrcTable->GetColName(iSrcCol), dDestStruct.GetKey(iDestCol));
			if (iCompare != 0)
				return iCompare;

			iCompare = pSrcTable->GetFieldValue(iSrcRow, iSrcCol).OpCompare(dDestStruct.GetElement(iDestCol));
			if (iCompare != 0)
				return iCompare;
			}

		return KeyCompare(SrcOrder.GetCount(), DestOrder.GetCount());
		}
	}

int CAEONRowRefImpl::OpCompare (CDatum dSrcStruct, DWORDLONG dwDestData)
	{
	IAEONTable* pDestTable = GetTable(dwDestData).GetTableInterface();
	int iDestRow = DecodeRowIndex(dwDestData);

	if (!dSrcStruct.IsStruct() && !pDestTable)
		return 0;
	else if (!dSrcStruct.IsStruct())
		return -1;
	else if (!pDestTable)
		return 1;
	else
		{
		TArray<int> SrcOrder = dSrcStruct.GetKeysInSortedOrder();
		TArray<int> DestOrder = pDestTable->GetColsInSortedOrder();

		int iCount = Min(SrcOrder.GetCount(), DestOrder.GetCount());
		for (int i = 0; i < iCount; i++)
			{
			int iSrcCol = SrcOrder[i];
			int iDestCol = DestOrder[i];

			int iCompare = KeyCompareNoCase(dSrcStruct.GetKey(iSrcCol), pDestTable->GetColName(iDestCol));
			if (iCompare != 0)
				return iCompare;

			iCompare = dSrcStruct.GetElement(iSrcCol).OpCompare(pDestTable->GetFieldValue(iDestRow, iDestCol));
			if (iCompare != 0)
				return iCompare;
			}

		return KeyCompare(SrcOrder.GetCount(), DestOrder.GetCount());
		}
	}

int CAEONRowRefImpl::OpCompareExact (DWORDLONG dwSrcData, DWORDLONG dwDestData)
	{
	IAEONTable* pSrcTable = GetTable(dwSrcData).GetTableInterface();
	int iSrcRow = DecodeRowIndex(dwSrcData);

	IAEONTable* pDestTable = GetTable(dwDestData).GetTableInterface();
	int iDestRow = DecodeRowIndex(dwDestData);

	if (!pSrcTable && !pDestTable)
		return 0;
	else if (!pSrcTable)
		return -1;
	else if (!pDestTable)
		return 1;
	else
		{
		int iCount = Min(pSrcTable->GetColCount(), pDestTable->GetColCount());
		for (int i = 0; i < iCount; i++)
			{
			int iCompare = KeyCompare(pSrcTable->GetColName(i), pDestTable->GetColName(i));
			if (iCompare != 0)
				return iCompare;

			iCompare = pSrcTable->GetFieldValue(iSrcRow, i).OpCompareExact(pDestTable->GetFieldValue(iDestRow, i));
			if (iCompare != 0)
				return iCompare;
			}

		return KeyCompare(pSrcTable->GetColCount(), pDestTable->GetColCount());
		}
	}

int CAEONRowRefImpl::OpCompareExact (DWORDLONG dwSrcData, CDatum dDestStruct)
	{
	IAEONTable* pSrcTable = GetTable(dwSrcData).GetTableInterface();
	int iSrcRow = DecodeRowIndex(dwSrcData);

	if (!pSrcTable && !dDestStruct.IsStruct())
		return 0;
	else if (!pSrcTable)
		return -1;
	else if (!dDestStruct.IsStruct())
		return 1;
	else
		{
		int iCount = Min(pSrcTable->GetColCount(), dDestStruct.GetCount());
		for (int i = 0; i < iCount; i++)
			{
			int iCompare = KeyCompare(pSrcTable->GetColName(i), dDestStruct.GetKey(i));
			if (iCompare != 0)
				return iCompare;

			iCompare = pSrcTable->GetFieldValue(iSrcRow, i).OpCompareExact(dDestStruct.GetElement(i));
			if (iCompare != 0)
				return iCompare;
			}

		return KeyCompare(pSrcTable->GetColCount(), dDestStruct.GetCount());
		}
	}

int CAEONRowRefImpl::OpCompareExact (CDatum dSrcStruct, DWORDLONG dwDestData)
	{
	IAEONTable* pDestTable = GetTable(dwDestData).GetTableInterface();
	int iDestRow = DecodeRowIndex(dwDestData);

	if (dSrcStruct.IsNil() && !pDestTable)
		return 0;
	else if (dSrcStruct.IsNil())
		return -1;
	else if (!pDestTable)
		return 1;
	else
		{
		int iCount = Min(dSrcStruct.GetCount(), pDestTable->GetColCount());
		for (int i = 0; i < iCount; i++)
			{
			int iCompare = KeyCompare(dSrcStruct.GetKey(i), pDestTable->GetColName(i));
			if (iCompare != 0)
				return iCompare;

			iCompare = dSrcStruct.GetElement(i).OpCompareExact(pDestTable->GetFieldValue(iDestRow, i));
			if (iCompare != 0)
				return iCompare;
			}

		return KeyCompare(dSrcStruct.GetCount(), pDestTable->GetColCount());
		}
	}

bool CAEONRowRefImpl::OpContains (DWORDLONG dwData, CDatum dValue)
	{
	return Find(dwData, dValue);
	}

bool CAEONRowRefImpl::OpIsEqual (DWORDLONG dwSrcData, DWORDLONG dwDestData)
	{
	return OpCompare(dwSrcData, dwDestData) == 0;
	}

bool CAEONRowRefImpl::OpIsIdentical (DWORDLONG dwSrcData, DWORDLONG dwDestData)
	{
	return OpCompareExact(dwSrcData, dwDestData) == 0;
	}

IAEONTable* CAEONRowRefImpl::ResolveTable (DWORDLONG dwData, int& retiRowIndex)
	{
	IAEONTable* pTable = GetTable(dwData).GetTableInterface();
	if (!pTable)
		return NULL;

	retiRowIndex = DecodeRowIndex(dwData);
	if (retiRowIndex < 0 || retiRowIndex >= pTable->GetRowCount())
		return NULL;

	return pTable;
	}

void CAEONRowRefImpl::Serialize (DWORDLONG dwData, CDatum::EFormat iFormat, IByteStream& Stream)
	{
	switch (iFormat)
		{
		case CDatum::EFormat::GridLang:
			{
			Stream.Write("[ ", 2);
			for (int i = 0; i < GetCount(dwData); i++)
				{
				if (i != 0)
					Stream.Write(", ", 2);

				CDatum::WriteGridLangIdentifier(Stream, GetKey(dwData, i));
				Stream.WriteChar('=');

				GetElement(dwData, NULL, i).Serialize(iFormat, Stream);
				}
			Stream.Write(" ]", 2);
			break;
			}

		case CDatum::EFormat::JSON:
			{
			Stream.Write("{", 1);
			for (int i = 0; i < GetCount(dwData); i++)
				{
				if (i != 0)
					Stream.Write(", ", 2);

				CDatum::WriteGridLangIdentifier(Stream, GetKey(dwData, i));
				Stream.WriteChar(':');

				GetElement(dwData, NULL, i).Serialize(iFormat, Stream);
				}
			Stream.Write("}", 1);
			break;
			}

		default:
			throw CException(errFail);
		}
	}

void CAEONRowRefImpl::SerializeAEON (DWORDLONG dwData, IByteStream& Stream, CAEONSerializedMap& Serialized)
	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, CDatum::raw_MakeDatum(dwData), CDatum::SERIALIZE_TYPE_STRUCT))
		return;

	//	Now write out each member variable

	Stream.Write(GetCount(dwData));
	for (int i = 0; i < GetCount(dwData); i++)
		{
		GetKey(dwData, i).Serialize(Stream);
		GetElement(dwData, NULL, i).SerializeAEON(Stream, Serialized);
		}
	}

void CAEONRowRefImpl::SetElement (DWORDLONG dwData, IInvokeCtx* pCtx, const CString& sKey, CDatum dDatum)
	{
	IAEONTable* pTable = GetTable(dwData).GetTableInterface();
	if (!pTable)
		return;

	int iRow = DecodeRowIndex(dwData);

	int iCol;
	if (!pTable->FindCol(sKey, &iCol))
		return;

	pTable->SetFieldValue(iRow, iCol, dDatum);
	}

void CAEONRowRefImpl::SetElement (DWORDLONG dwData, int iIndex, CDatum dDatum)
	{
	IAEONTable* pTable = GetTable(dwData).GetTableInterface();
	if (!pTable)
		return;

	int iRow = DecodeRowIndex(dwData);
	pTable->SetFieldValue(iRow, iIndex, dDatum);
	}

void CAEONRowRefImpl::SetElementAt (DWORDLONG dwData, CDatum dKey, CDatum dDatum)
	{
	IAEONTable* pTable = GetTable(dwData).GetTableInterface();
	if (!pTable)
		return;

	int iRow = DecodeRowIndex(dwData);

	if (dKey.IsNil())
		{ }
	else if (dKey.IsNumberInt32())
		{
		int iIndex = dKey;
		pTable->SetFieldValue(iRow, iIndex, dDatum);
		}
	else
		{
		int iCol;
		if (!pTable->FindCol(dKey.AsString(), &iCol))
			return;

		pTable->SetFieldValue(iRow, iCol, dDatum);
		}
	}
