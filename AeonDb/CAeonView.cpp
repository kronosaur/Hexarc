//	CAeonView.cpp
//
//	CAeonView class
//	Copyright (c) 2012 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

const int MIN_SEGMENTS_TO_MERGE =						3;		//	Merge if we have at least this many segments
const DWORDLONG SMALL_MERGE_SIZE =						10000000;	//	10 MB is considered a small merge
const double MIN_MERGE_RATIO =							0.8;
const double MAX_MERGE_RATIO =							1.25;

DECLARE_CONST_STRING(STR_ERROR_KEY,						"(Cannot evaluate key function)");
DECLARE_CONST_STRING(STR_EMPTY_KEY,						"(nil)");
DECLARE_CONST_STRING(STR_ALL_COLUMNS,					"*");

DECLARE_CONST_STRING(FIELD_COLUMNS,						"columns");
DECLARE_CONST_STRING(FIELD_COMPUTED_COLUMNS,			"computedColumns");
DECLARE_CONST_STRING(FIELD_ERROR,						"error");
DECLARE_CONST_STRING(FIELD_EXCLUDE_NIL_KEYS,			"excludeNilKeys");
DECLARE_CONST_STRING(FIELD_GLOBAL_ENV,					"globalEnv");
DECLARE_CONST_STRING(FIELD_ID,							"id");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_PRIMARY_KEY,					"primaryKey");
DECLARE_CONST_STRING(FIELD_PRIMARY_KEY_PREFIX,			"primaryKey.");
DECLARE_CONST_STRING(FIELD_RECOVERY_FILESPEC,			"recoveryFilespec");
DECLARE_CONST_STRING(FIELD_SEGMENTS,					"segments");
DECLARE_CONST_STRING(FIELD_UPDATE_NEEDED,				"updateNeeded");
DECLARE_CONST_STRING(FIELD_X,							"x");
DECLARE_CONST_STRING(FIELD_Y,							"y");
DECLARE_CONST_STRING(FIELD_Z,							"z");

DECLARE_CONST_STRING(TYPENAME_HEXE_FUNCTION,			"hexeFunction");

DECLARE_CONST_STRING(ERR_COMPUTED_COLUMN,				"Cannot evaluate computed column function.");
DECLARE_CONST_STRING(ERR_DIMENSIONS_REQUIRED,			"Cannot create a table without dimensions.");
DECLARE_CONST_STRING(ERR_PRIMARY_KEY_CANT_BE_LIST,		"List-type keys not supported for primary views.");
DECLARE_CONST_STRING(ERR_INVALID_PATH,					"Path does not have the correct number of dimensions.");

CAeonView::CAeonView (void) : 
		m_Segments(DescendingSort)

//	CAeonView constructor

	{
	}

CAeonView::~CAeonView (void)

//	CAeonView destructor

	{
	CleanUp();
	}

bool CAeonView::CanInsert (const CRowKey &Path, CDatum dData, CString *retsError)

//	CanInsert
//
//	Returns TRUE if we can insert the row

	{
	//	Secondary views are different

	if (IsSecondaryView())
		{
		//	Can't think of any way that this can fail for secondary views...
		}
	
	//	Otherwise, check the dimensions of the primary key

	else
		{
		//	If dimensions don't match, then we have an error

		if (!Path.MatchesDimensions(m_Dims) || Path.HasNullDimensions(m_Dims))
			{
			*retsError = ERR_INVALID_PATH;
			return false;
			}
		}

	return true;
	}

void CAeonView::CleanUp (void)

//	CleanUp
//
//	Clean up the view

	{
	//	Delete all segment

	CloseSegments();

	//	Delete rows

	if (m_pRows)
		{
		m_pRows->Release();
		m_pRows = NULL;
		}
	}

void CAeonView::CloseRecovery (void)

//	CloseRecovery
//
//	Close the recovery file

	{
	m_Recovery.Close();
	}

void CAeonView::CloseSegments (bool bMarkForDelete)

//	CloseSegments
//
//	Close all segments

	{
	int i;

	//	Queue all current segments for deletion.

	for (i = 0; i < m_Segments.GetCount(); i++)
		{
		if (bMarkForDelete)
			m_Segments[i]->MarkForDelete();

		m_Segments[i]->Release();
		}

	m_Segments.DeleteAll();
	}

CDatum CAeonView::ComputeColumns (CHexeProcess &Process, CDatum dRowData)

//	ComputeColumns
//
//	Returns a new row struct containing any computed columns.

	{
	if (m_ComputedColumns.IsNil() || !m_ComputedColumns.CanInvoke())
		return dRowData;

	//	Compute columns. We should get back a struct of all new columns.

	TArray<CDatum> Args;
	Args.Insert(dRowData);

	CDatum dResult;
	CHexeProcess::ERun iRun = Process.Run(m_ComputedColumns, Args, &dResult);

	switch (iRun)
		{
		case CHexeProcess::ERun::OK:
			{
			//	dResult is a struct containing zero or more columns

			CDatum dNewRowData(new CComplexStruct(dRowData));
			dNewRowData.Append(dResult);
			return dNewRowData;
			}

		case CHexeProcess::ERun::Error:
		case CHexeProcess::ERun::ForcedTerminate:
			{
			CDatum dNewRowData(new CComplexStruct(dRowData));
			dNewRowData.SetElement(FIELD_ERROR, strPattern("ERROR: %s", dResult.AsString()));
			return dNewRowData;
			}

		default:
			{
			CDatum dNewRowData(new CComplexStruct(dRowData));
			dNewRowData.SetElement(FIELD_ERROR, ERR_COMPUTED_COLUMN);
			return dNewRowData;
			}
		}
	}

void CAeonView::Copy (const CAeonView &Src) noexcept

//	Copy
//
//	Make a copy

	{
	m_dwID = Src.m_dwID;
	m_sName = Src.m_sName;
	m_Dims = Src.m_Dims;
	m_pRows = Src.m_pRows;
	if (m_pRows)
		m_pRows->AddRef();

	m_Segments = Src.m_Segments;
	for (int i = 0; i < m_Segments.GetCount(); i++)
		m_Segments[i]->AddRef();

	m_Keys = Src.m_Keys;
	m_Columns = Src.m_Columns;
	m_ComputedColumns = Src.m_ComputedColumns;
	m_bInvalid = Src.m_bInvalid;
	m_bExcludeNil = Src.m_bExcludeNil;
	m_bUsesListKeys = Src.m_bUsesListKeys;
	m_bUpdateNeeded = Src.m_bUpdateNeeded;
	}

void CAeonView::CreatePermutedKeys (const TArray<CDatum> &KeyData, int iDim, const TArray<CDatum> &PrevKey, SEQUENCENUMBER RowID, TArray<CRowKey> *retKeys)

//	CreatePermutedKeys
//
//	Adds keys to retKeys by permuting any list values.

	{
	int i;

	//	If we're done, then add the key

	if (iDim == KeyData.GetCount())
		{
		CRowKey *pNewKey = retKeys->Insert();
		CRowKey::CreateFromDatumAndRowID(m_Dims, PrevKey, RowID, pNewKey);
		}

	//	Otherwise, we generate the keys just for the current dimension and 
	//	recurse.

	else
		{
		switch (m_Dims[iDim].iKeyType)
			{
			//	For list keys, add all the values of the list as separate keys

			case keyListUTF8:
				{
				CDatum dList = KeyData[iDim];

				//	If nil, just add as a single nil key

				if (dList.IsNil())
					{
					TArray<CDatum> NewKey(PrevKey);
					NewKey.Insert(dList);
					CreatePermutedKeys(KeyData, iDim + 1, NewKey, RowID, retKeys);
					}

				//	Otherwise, add all values

				else
					{
					TArray<CDatum> NewKey(PrevKey);
					NewKey.Insert(CDatum());

					for (i = 0; i < dList.GetCount(); i++)
						{
						NewKey[iDim] = dList.GetElement(i);
						CreatePermutedKeys(KeyData, iDim + 1, NewKey, RowID, retKeys);
						}
					}

				break;
				}

			//	For non-list keys we just continue adding them

			default:
				{
				TArray<CDatum> NewKey(PrevKey);
				NewKey.Insert(KeyData[iDim]);
				CreatePermutedKeys(KeyData, iDim + 1, NewKey, RowID, retKeys);
				}
			}
		}
	}

void CAeonView::CreateSecondaryData (const CTableDimensions &PrimaryDims, const CRowKey &PrimaryKey, CDatum dFullData, SEQUENCENUMBER RowID, CDatum *retdData)

//	CreateSecondaryData
//
//	Creates the data for a secondary view row.

	{
	int i, j;
	CComplexStruct *pData = new CComplexStruct;

	//	If the list of columns is empty then we just add the primary key

	if (m_Columns.GetCount() == 0)
		pData->SetElement(FIELD_PRIMARY_KEY, PrimaryKey.AsDatum(PrimaryDims));

	//	Otherwise we add all the fields listed in the columns array

	else
		{
		for (i = 0; i < m_Columns.GetCount(); i++)
			{
			//	The special string "primaryKey" means that we insert the 
			//	primary key as a special field.

			if (strEquals(m_Columns[i], FIELD_PRIMARY_KEY))
				pData->SetElement(FIELD_PRIMARY_KEY, PrimaryKey.AsDatum(PrimaryDims));

			//	The special string "*" means that we insert all existing
			//	fields.

			else if (strEquals(m_Columns[i], STR_ALL_COLUMNS))
				{
				for (j = 0; j < dFullData.GetCount(); j++)
					{
					CDatum dKey = dFullData.GetKey(j);
					CDatum dValue = dFullData.GetElement(j);

					if (!dValue.IsNil())
						pData->SetElement(dKey.AsStringView(), dValue);
					}
				}

			//	Add the field by name.

			else
				{
				CDatum dColData = dFullData.GetElement(m_Columns[i]);
				if (!dColData.IsNil())
					pData->SetElement(m_Columns[i], dColData);
				}
			}
		}

	//	Done

	*retdData = CDatum(pData);
	}

bool CAeonView::CreateSecondaryKeys (CHexeProcess &Process, const CTableDimensions &PrimaryDims, const CRowKey &PrimaryKey, CDatum dData, SEQUENCENUMBER RowID, TArray<CRowKey> *retKeys)

//	CreateSecondaryKeys
//
//	Creates a secondary key from the data and rowID. We return TRUE if all of 
//	the key values are non-nil. FALSE if one or more values are nil.

	{
	int i;
	bool bAllValid = true;

	//	Pull the dimensions from the data

	TArray<CDatum> KeyData;
	for (i = 0; i < m_Keys.GetCount(); i++)
		{
		CDatum dValue;
		CDatum dKeyDesc = m_Keys[i];

		//	If this is a function then we need to evaluate it.

		if (dKeyDesc.CanInvoke())
			{
			TArray<CDatum> Args;
			Args.Insert(dData);

			CHexeProcess::ERun iRun = Process.Run(dKeyDesc, Args, &dValue);

			switch (iRun)
				{
				case CHexeProcess::ERun::OK:
					//	dValue is a valid value for a key
					break;

				case CHexeProcess::ERun::Error:
				case CHexeProcess::ERun::ForcedTerminate:
					dValue = CDatum(strPattern("(%s)", dValue.AsString()));
					break;

				default:
					dValue = CDatum(STR_ERROR_KEY);
				}
			}

		//	Otherwise this specifies a field in the data to use as a key

		else
			{
			CStringView sKeyDesc = dKeyDesc;
			if (strStartsWith(sKeyDesc, FIELD_PRIMARY_KEY_PREFIX))
				{
				CString sKeyPart = strSubString(sKeyDesc, FIELD_PRIMARY_KEY_PREFIX.GetLength());
				int iPart;
				if (strEquals(sKeyPart, FIELD_X))
					iPart = 0;
				else if (strEquals(sKeyPart, FIELD_Y))
					iPart = 1;
				else if (strEquals(sKeyPart, FIELD_Z))
					iPart = 2;
				else
					iPart = -1;

				if (iPart == -1 || iPart >= PrimaryDims.GetCount())
					dValue = CDatum(strPattern("(Unknown field: %s)", sKeyPart));
				else
					{
					CDatum dKey = PrimaryKey.AsDatum(PrimaryDims);
					dValue = dKey.GetElement(iPart);
					}
				}
			else
				dValue = dData.GetElement(dKeyDesc.AsStringView());
			}

		//	We don't support Nil keys, so we have to replace these with a
		//	a special value.

		if (dValue.IsNil())
			{
			dValue = CDatum(STR_EMPTY_KEY);

			//	If we're not valid if we're excluding nil keys

			if (m_bExcludeNil)
				bAllValid = false;
			}

		//	Add to list

		KeyData.Insert(dValue);
		}


	//	Generate the keys.
	//	If we use list keys then we need to create permuted keys

	retKeys->DeleteAll();
	if (m_bUsesListKeys)
		CreatePermutedKeys(KeyData, 0, TArray<CDatum>(), RowID, retKeys);

	//	Otherwise we just create the key normally

	else
		{
		CRowKey *pNewKey = retKeys->Insert();
		CRowKey::CreateFromDatumAndRowID(m_Dims, KeyData, RowID, pNewKey);
		}

	//	Done

	return bAllValid;
	}

bool CAeonView::CreateSecondaryRows (const CTableDimensions &PrimaryDims, CHexeProcess &Process, const CRowKey &PrimaryKey, CDatum dFullData, SEQUENCENUMBER RowID, CAeonRowArray *Rows)

//	CreateSecondaryRow
//
//	Returns the key and data for a secondary view row. If the secondary key has
//	all non-nil values then we return TRUE. FALSE otherwise.

	{
	//	Compute columns, if necessary

	dFullData = ComputeColumns(Process, dFullData);

	//	Create keys

	TArray<CRowKey> Keys;
	if (!CreateSecondaryKeys(Process, PrimaryDims, PrimaryKey, dFullData, RowID, &Keys))
		return false;

	//	Create row data

	CDatum dRowData;
	CreateSecondaryData(PrimaryDims, PrimaryKey, dFullData, RowID, &dRowData);

	for (int i = 0; i < Keys.GetCount(); i++)
		Rows->Insert(Keys[i], dRowData, RowID);

	return true;
	}

bool CAeonView::CreateSegment (const CString &sFilespec, SEQUENCENUMBER Seq, IOrderedRowSet *pRows, CAeonSegment **retpNewSeg, CString *retsError)

//	CreateSegment
//
//	Creates a segment out of the current rows.

	{
	//	Create a row iterator containing the rows

	CRowIterator Rows;
	Rows.Init(m_Dims);
	if (pRows)
		Rows.AddSegment(pRows);
	else
		Rows.AddSegment(m_pRows);

	//	Create the segment

	DWORD dwSegFlags = 0;
	dwSegFlags |= (HasRowID() ? CAeonSegment::FLAG_HAS_ROW_ID : 0);
	dwSegFlags |= (IsSecondaryView() ? CAeonSegment::FLAG_SECONDARY_VIEW : 0);

	CAeonSegment *pNewSeg = new CAeonSegment;
	if (!pNewSeg->Create(GetID(), m_Dims, Seq, Rows, sFilespec, dwSegFlags, retsError))
		{
		pNewSeg->Release();
		return false;
		}

	//	Done

	*retpNewSeg = pNewSeg;

	return true;
	}

CDatum CAeonView::DebugDump (void) const

//	DebugDump
//
//	Returns data about the view.

	{
	int i;

	CComplexStruct *pData = new CComplexStruct;

	pData->SetElement(FIELD_RECOVERY_FILESPEC, m_Recovery.GetFilespec());

	CComplexArray *pSegments = new CComplexArray;
	for (i = 0; i < m_Segments.GetCount(); i++)
		pSegments->Append(m_Segments[i]->DebugDump());

	pData->SetElement(FIELD_SEGMENTS, CDatum(pSegments));

	return CDatum(pData);
	}

bool CAeonView::GetData (const CRowKey &Path, CDatum *retData, SEQUENCENUMBER *retRowID, CString *retsError)

//	GetData
//
//	Returns data for the given key

	{
	int i;

	//	If dimensions don't match, then we have an error

	if (!Path.MatchesDimensions(m_Dims))
		{
		*retsError = ERR_INVALID_PATH;
		return false;
		}

	//	First look for the value in the in-memory rows

	if (m_pRows->FindData(Path, retData, retRowID))
		return true;

	//	Otherwise, look in the segments

	for (i = 0; i < m_Segments.GetCount(); i++)
		if (m_Segments[i]->FindData(Path, retData, retRowID))
			return true;

	//	Otherwise, we succeed, but the value is not found

	*retData = CDatum();
	if (retRowID)
		*retRowID = 0;

	return true;
	}

bool CAeonView::GetSegmentsToMerge (CAeonSegment **retpSeg1, CAeonSegment **retpSeg2)

//	GetSegmentsToMerge
//
//	Returns two segments to merge (or FALSE if no merge is needed).

	{
	int i;

	//	Must have at least 2 segments to merge.

	int iSegCount = m_Segments.GetCount();
	if (iSegCount < 2)
		return false;

	//	Pick two segments to merge. Find two segments of about the same,
	//	starting with the last two segments (i.e., oldest).

	CAeonSegment *pSeg1 = NULL;
	CAeonSegment *pSeg2 = NULL;

	for (i = iSegCount - 2; i >= 0; i--)
		{
		DWORDLONG dwSeg1Size = m_Segments[i]->GetFileSize();
		DWORDLONG dwSeg2Size = m_Segments[i + 1]->GetFileSize();

		//	If the combined file if greater than a gigabyte, then skip because
		//	we don't currently handle files that size.

		if (dwSeg1Size + dwSeg2Size > GIGABYTE_DISK)
			continue;

		//	Compute the ratio of segment 1 to segment 2

		double rRatio = (double)dwSeg1Size / (double)dwSeg2Size;

		//	If the final segment size is pretty small or if both are about the
		//	same size, then pick this pair.

		if ((dwSeg1Size + dwSeg2Size <= SMALL_MERGE_SIZE)
				|| (rRatio >= MIN_MERGE_RATIO && rRatio <= MAX_MERGE_RATIO))
			{
			pSeg1 = m_Segments[i];
			pSeg2 = m_Segments[i + 1];
			break;
			}
		}

	//	If we did not find segments to merge and if we've got more than the
	//	maximum number of segments, merge the first two.

	if (pSeg1 == NULL || pSeg2 == NULL)
		{
		if (iSegCount >= MIN_SEGMENTS_TO_MERGE)
			{
			pSeg1 = m_Segments[0];
			pSeg2 = m_Segments[1];
			}
		else
			return false;
		}

	//	Done

	*retpSeg1 = pSeg1;
	*retpSeg2 = pSeg2;

	return true;
	}

bool CAeonView::InitAsFileView (const CString &sRecoveryFilespec, int *retiRowsRecovered, CString *retsError)

//	InitAsFileView
//
//	Initializes a standard file table view.

	{
	ASSERT(m_Dims.GetCount() == 0);

	SDimensionDesc *pDimDesc = m_Dims.Insert();
	pDimDesc->iKeyType = keyUTF8;
	pDimDesc->iSort = AscendingSort;

	if (!InitRows(sRecoveryFilespec, retiRowsRecovered, retsError))
		return false;

	return true;
	}

bool CAeonView::InitAsPrimaryView (CDatum dDesc, const CString &sRecoveryFilespec, int *retiRowsRecovered, CString *retsError)

//	InitAsPrimaryView
//
//	Initialize from a descriptor

	{
	int i;

	ASSERT(m_Dims.GetCount() == 0);

	//	Parse the x dimension

	CDatum dimDesc = dDesc.GetElement(FIELD_X);
	if (!dimDesc.IsNil())
		{
		SDimensionDesc *pDimDesc = m_Dims.Insert();
		if (!CAeonTable::ParseDimensionDesc(dimDesc, pDimDesc, retsError))
			return false;

		//	Parse the y dimension

		dimDesc = dDesc.GetElement(FIELD_Y);
		if (!dimDesc.IsNil())
			{
			pDimDesc = m_Dims.Insert();
			if (!CAeonTable::ParseDimensionDesc(dimDesc, pDimDesc, retsError))
				return false;

			//	Parse z dimension

			dimDesc = dDesc.GetElement(FIELD_Z);
			if (!dimDesc.IsNil())
				{
				pDimDesc = m_Dims.Insert();
				if (!CAeonTable::ParseDimensionDesc(dimDesc, pDimDesc, retsError))
					return false;
				}
			}
		}

	//	If we don't have at least one dimension then this is an error

	else
		{
		*retsError = ERR_DIMENSIONS_REQUIRED;
		return false;
		}

	//	We do not support list types for primary views (only secondary views)

	for (i = 0; i < m_Dims.GetCount(); i++)
		if (m_Dims[i].iKeyType == keyListUTF8)
			{
			*retsError = ERR_PRIMARY_KEY_CANT_BE_LIST;
			return false;
			}

	//	Initialize rows

	if (!InitRows(sRecoveryFilespec, retiRowsRecovered, retsError))
		return false;

	//	Done

	return true;
	}

bool CAeonView::InitAsSecondaryView (CDatum dDesc, CHexeProcess &Process, const CString &sRecoveryFilespec, bool bForceUpdate, CString *retsError)

//	InitAsSecondaryView
//
//	Initializes a secondary view.

	{
	int i;

	ASSERT(m_Dims.GetCount() == 0);

	//	Get the name

	m_sName = dDesc.GetElement(FIELD_NAME).AsStringView();

	m_bUsesListKeys = false;

	//	Parse the x dimension

	CDatum dimDesc = dDesc.GetElement(FIELD_X);
	if (!dimDesc.IsNil())
		{
		SDimensionDesc *pDimDesc = m_Dims.Insert();
		CDatum *pKey = m_Keys.Insert();
		if (!CAeonTable::ParseDimensionDescForSecondaryView(dimDesc, Process, pDimDesc, pKey, retsError))
			{
			m_bInvalid = true;
			return false;
			}

		if (pDimDesc->iKeyType == keyListUTF8)
			m_bUsesListKeys = true;

		//	Parse the y dimension

		dimDesc = dDesc.GetElement(FIELD_Y);
		if (!dimDesc.IsNil())
			{
			pDimDesc = m_Dims.Insert();
			pKey = m_Keys.Insert();
			if (!CAeonTable::ParseDimensionDescForSecondaryView(dimDesc, Process, pDimDesc, pKey, retsError))
				{
				m_bInvalid = true;
				return false;
				}

			if (pDimDesc->iKeyType == keyListUTF8)
				m_bUsesListKeys = true;

			//	Parse z dimension

			dimDesc = dDesc.GetElement(FIELD_Z);
			if (!dimDesc.IsNil())
				{
				pDimDesc = m_Dims.Insert();
				pKey = m_Keys.Insert();
				if (!CAeonTable::ParseDimensionDescForSecondaryView(dimDesc, Process, pDimDesc, pKey, retsError))
					{
					m_bInvalid = true;
					return false;
					}

				if (pDimDesc->iKeyType == keyListUTF8)
					m_bUsesListKeys = true;
				}
			}
		}

	//	If we don't have at least one dimension then this is an error

	else
		{
		*retsError = ERR_DIMENSIONS_REQUIRED;
		m_bInvalid = true;
		return false;
		}

	//	Secondary views always have an extra dimension. We use the rowID as a
	//	way to break ties in the other parts of the key (since secondary keys
	//	need not be unique).

	SDimensionDesc *pDimDesc = m_Dims.Insert();
	pDimDesc->iKeyType = keyInt64;
	pDimDesc->iSort = AscendingSort;

	//	Parse columns

	CDatum dColumns = dDesc.GetElement(FIELD_COLUMNS);
	for (i = 0; i < dColumns.GetCount(); i++)
		{
		CStringView sCol = dColumns.GetElement(i);
		if (!sCol.IsEmpty())
			m_Columns.Insert(sCol);
		}

	//	Computed columns

	m_ComputedColumns = dDesc.GetElement(FIELD_COMPUTED_COLUMNS);

	//	We need to set the global environment because it got loaded under a 
	//	different process.

	if (strEquals(m_ComputedColumns.GetTypename(), TYPENAME_HEXE_FUNCTION))
		m_ComputedColumns.SetElement(FIELD_GLOBAL_ENV, Process.GetGlobalEnv());

	//	Exclude nil?

	m_bExcludeNil = !dDesc.GetElement(FIELD_EXCLUDE_NIL_KEYS).IsNil();

	//	Initialize rows

	if (!InitRows(sRecoveryFilespec, NULL, retsError))
		{
		m_bInvalid = true;
		return false;
		}

	//	Set up update. If we have stored an update sequence number then it means
	//	that we are loading an old view that has not yet been fully updated.
	//
	//	Otherwise we take the updating number passed in.

	m_bUpdateNeeded = (bForceUpdate ? true : !dDesc.GetElement(FIELD_UPDATE_NEEDED).IsNil());

	//	Done

	return true;
	}

bool CAeonView::InitIterator (CRowIterator *retIterator, DWORD dwFlags)

//	InitIterator
//
//	Initializes an iterator

	{
	int i;

	if (!retIterator->Init(m_Dims))
		return false;

	//	Add the rows first because they are the latest

	if (m_pRows->GetCount() > 0 && !(dwFlags & FLAG_EXCLUDE_MEMORY_ROWS))
		retIterator->AddSegment(m_pRows);

	//	Next add all the segments (which are ordered from most
	//	recent to least recent).

	if (!(dwFlags & FLAG_EXCLUDE_SEGMENTS))
		{
		for (i = 0; i < m_Segments.GetCount(); i++)
			retIterator->AddSegment(m_Segments[i]);
		}

	//	Done

	retIterator->Reset();
	return true;
	}

bool CAeonView::InitRows (const CString &sRecoveryFilespec, int *retiRowsRecovered, CString *retsError)

//	InitRows
//
//	Initializes the rows structure.

	{
	ASSERT(m_pRows == NULL);
	return LoadRecoveryFile(sRecoveryFilespec, &m_pRows, retiRowsRecovered, retsError);
	}

void CAeonView::Insert (const CTableDimensions &PrimaryDims, CHexeProcess &Process, const CRowKey &PrimaryKey, CDatum dData, CDatum dOldData, SEQUENCENUMBER RowID, bool *retbRecoveryFailed, CString *retsError)

//	Insert
//
//	Insert a row
//
//	NOTE: We cannot fail here because callers should have called CanInsert
//	(above).

	{
	int i;
	CString sError;
	*retbRecoveryFailed = false;

	//	If this is a primary view then all we have to do is insert the row

	if (!IsSecondaryView())
		{
		m_pRows->Insert(PrimaryKey, dData, RowID);
		if (!m_Recovery.Insert(PrimaryKey, dData, RowID, retsError))
			*retbRecoveryFailed = true;
		}

	//	Otherwise, it's more complicated.

	else
		{
		//	If this view is not yet up to date then we skip insertion (we will
		//	insert all rows later).

		if (!IsUpToDate())
			return;

		//	If we are updating an existing row, then we need to remove the old
		//	value. Note that it is OK if OldKey and NewKey end up being the
		//	same; we just end up overwriting it.

		if (!dOldData.IsNil())
			{
			//	Generate a key for the old value. If all key values are non-nil
			//	then we update the old value.

			TArray<CRowKey> OldKeys;
			if (CreateSecondaryKeys(Process, PrimaryDims, PrimaryKey, ComputeColumns(Process, dOldData), RowID, &OldKeys))
				{
				//	Now delete the old value (by writing out Nil)

				for (i = 0; i < OldKeys.GetCount(); i++)
					{
					m_pRows->Insert(OldKeys[i], CDatum(), RowID);
					if (!m_Recovery.Insert(OldKeys[i], CDatum(), RowID, retsError))
						*retbRecoveryFailed = true;
					}
				}
			}

		//	Save the new value (only if not Nil)

		if (!dData.IsNil())
			{
			//	Compute columns

			dData = ComputeColumns(Process, dData);

			//	Generate a key for the new value. If all key values are non-nil
			//	then we insert the row into the secondary view.

			TArray<CRowKey> NewKeys;
			if (CreateSecondaryKeys(Process, PrimaryDims, PrimaryKey, dData, RowID, &NewKeys))
				{
				//	Generate data for secondary key

				CDatum dViewData;
				CreateSecondaryData(PrimaryDims, PrimaryKey, dData, RowID, &dViewData);

				//	Insert it

				for (i = 0; i < NewKeys.GetCount(); i++)
					{
					m_pRows->Insert(NewKeys[i], dViewData, RowID);
					if (!m_Recovery.Insert(NewKeys[i], dViewData, RowID, retsError))
						*retbRecoveryFailed = true;
					}
				}
			}
		}
	}

bool CAeonView::LoadRecoveryFile (const CString &sRecoveryFilespec, CAeonRowArray **retpRows, int *retiRowsRecovered, CString *retsError)

//	LoadRecoveryFile
//
//	Loads the recovery file into the new array. This function does not alter the
//	state of the view.
//
//	Caller is responsible for freeing retpRows.

	{
	CAeonRowArray *pRows = new CAeonRowArray;
	pRows->Init(m_Dims);

	//	Open the recovery file (and recover rows, if any)

	if (!m_Recovery.Open(sRecoveryFilespec, pRows, retiRowsRecovered, retsError))
		{
		pRows->Release();
		return false;
		}

	//	Done

	if (retpRows)
		*retpRows = pRows;

	return true;
	}

void CAeonView::Mark (void)

//	Mark
//
//	Mark data in use

	{
	int i;

	for (i = 0; i < m_Keys.GetCount(); i++)
		m_Keys[i].Mark();

	m_ComputedColumns.Mark();
	}

void CAeonView::SegmentMergeComplete (CAeonSegment *pSeg1, CAeonSegment *pSeg2, CAeonSegment *pNewSeg)

//	SegmentMergeComplete
//
//	Replace two segments with the new one.

	{
	int i;

	//	Mark the original segments for deletion. (We cannot delete them here
	//	because an iterator might have pointers to them). They will be 
	//	deleted at the next garbage collection point.

	for (i = 0; i < m_Segments.GetCount(); i++)
		if (m_Segments[i] == pSeg1)
			{
			pSeg1->MarkForDelete();
			pSeg1->Release();
			m_Segments.Delete(i);
			i--;
			}
		else if (m_Segments[i] == pSeg2)
			{
			pSeg2->MarkForDelete();
			pSeg2->Release();
			m_Segments.Delete(i);
			i--;
			}

	//	Add the new segment

	m_Segments.Insert(pNewSeg->GetSequence(), pNewSeg);
	}

void CAeonView::SegmentSaveComplete (CAeonSegment *pSeg)

//	SegmentSaveComplete
//
//	The new segment replaces the in-memory rows.

	{
	//	Add the new segment

	m_Segments.Insert(pSeg->GetSequence(), pSeg);

	//	Since we've saved out all these rows, remove them from
	//	the in-memory structures.

	m_pRows->Release();
	m_pRows = new CAeonRowArray;
	m_pRows->Init(m_Dims);

	m_Recovery.Reset();
	}

void CAeonView::SetUnsavedRows (CAeonRowArray *pRows)

//	SetUnsavedRows
//
//	Replaces the row array with the given one (and takes ownership of pRows).
//	We assume that callers know what they are doing.

	{
	m_pRows->Release();
	m_pRows = pRows;
	}

void CAeonView::WriteDesc (CComplexStruct *pDesc)

//	WriteDesc
//
//	Write out a description of the view.

	{
	int i;

	//	ID

	if (m_dwID)
		pDesc->SetElement(FIELD_ID, CDatum((int)m_dwID));

	//	Name

	if (!m_sName.IsEmpty())
		pDesc->SetElement(FIELD_NAME, m_sName);

	//	Dimensions

	if (IsSecondaryView())
		{
		//	Secondary views always have an extra dimension (used for the disambiguating rowKey).

		int iDimsToSave = m_Dims.GetCount() - 1;

		//	Save the user-created dimensions.

		pDesc->SetElement(FIELD_X, CAeonTable::GetDimensionDescForSecondaryView(m_Dims[0], m_Keys[0]));
		if (iDimsToSave > 1)
			{
			pDesc->SetElement(FIELD_Y, CAeonTable::GetDimensionDescForSecondaryView(m_Dims[1], m_Keys[1]));
			if (iDimsToSave > 2)
				pDesc->SetElement(FIELD_Z, CAeonTable::GetDimensionDescForSecondaryView(m_Dims[2], m_Keys[2]));
			}
		}
	else
		{
		pDesc->SetElement(FIELD_X, CAeonTable::GetDimensionDesc(m_Dims[0]));
		if (m_Dims.GetCount() > 1)
			{
			pDesc->SetElement(FIELD_Y, CAeonTable::GetDimensionDesc(m_Dims[1]));
			if (m_Dims.GetCount() > 2)
				pDesc->SetElement(FIELD_Z, CAeonTable::GetDimensionDesc(m_Dims[2]));
			}
		}

	//	Write out columns

	if (m_Columns.GetCount() > 0)
		{
		CComplexArray *pColumns = new CComplexArray;
		for (i = 0; i < m_Columns.GetCount(); i++)
			pColumns->Insert(CDatum(m_Columns[i]));

		pDesc->SetElement(FIELD_COLUMNS, CDatum(pColumns));
		}

	//	Computed columns

	if (!m_ComputedColumns.IsNil())
		pDesc->SetElement(FIELD_COMPUTED_COLUMNS, m_ComputedColumns);

	//	Exclude nil

	if (m_bExcludeNil)
		pDesc->SetElement(FIELD_EXCLUDE_NIL_KEYS, CDatum(true));

	//	Write out update sequence number

	if (m_bUpdateNeeded)
		pDesc->SetElement(FIELD_UPDATE_NEEDED, CDatum(true));
	}

