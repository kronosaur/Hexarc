//	CAeonTable.cpp
//
//	CAeonTable class
//	Copyright (c) 2024 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_STORAGE_PATH,				"storagePath");

bool CAeonTable::Diagnostics (DWORD dwFlags, TArray<CString>& retLog, CString* retsError)
	{
	CSmartLock Lock(m_cs);

	retLog.Insert(strPattern("[%s]: Diagnostics", m_sName));

	for (int i = 0; i < m_Views.GetCount(); i++)
		{
		retLog.Insert(strPattern("[%s/%s]: Examining view", m_sName, m_Views[i].GetName()));

		if (!m_Views[i].Diagnostics(dwFlags, retLog, retsError))
			return false;
		}

	//	If this is a file table then check all files.

	if (GetType() == typeFile)
		{
		CTableDimensions Dims = m_Views[DEFAULT_VIEW].GetDimensions();

		CDatum dAllRows;
		if (!GetRows(DEFAULT_VIEW, CDatum(), -1, TArray<int>(), dwFlags, &dAllRows, retsError))
			return false;

		for (int i = 0; i < dAllRows.GetCount(); i += 2)
			{
			CRowKey Key;
			if (!CRowKey::ParseKey(Dims, dAllRows.GetElement(i), &Key, retsError))
				return false;

			CDatum dSrcData = dAllRows.GetElement(i + 1);
			CString sSrcStorage = dSrcData.GetElement(FIELD_STORAGE_PATH).AsString();
			CString sFilespec = m_pStorage->CanonicalRelativeToMachine(m_sPrimaryVolume, sSrcStorage);

			if (!fileExists(sFilespec))
				{
				retLog.Insert(strPattern("ERROR: File does not exist: %s", sFilespec));
				}
			}

		retLog.Insert(strPattern("[%s]: All files found.", m_sName));
		}

	retLog.Insert(strPattern("[%s] Diagnostics complete", m_sName));

	return true;
	}

bool CAeonView::Diagnostics (DWORD dwFlags, TArray<CString>& retLog, CString* retsError)
	{
	for (int i = 0; i < m_Segments.GetCount(); i++)
		{
		CAeonSegment& Segment = *m_Segments[i];

		retLog.Insert(strPattern("[%s]: Examining segment", ::fileGetFilename(Segment.GetFilespec())));

		if (!Segment.Diagnostics(dwFlags, retLog, retsError))
			return false;
		}

	return true;
	}

bool CAeonSegment::Diagnostics (DWORD dwFlags, TArray<CString>& retLog, CString* retsError)
	{
	DWORD dwIndexRowCount = 0;
	DWORD dwBlockRowCount = 0;
	bool bHasErrors = false;

	retLog.Insert(strPattern("[%s]: Rows = %d", ::fileGetFilename(m_sFilespec), m_pHeader->dwRowCount));

	for (int i = 0; i < GetIndexCount(); i++)
		{
		SIndexEntry *pIndex = GetIndexEntry(i);
		CString sIndexKey = IndexGetKey(pIndex);
		retLog.Insert(strPattern("Block: %d Key: %s %d rows at %d", i, CAeonTable::GetDimensionPath(m_Dims, sIndexKey).AsString(), pIndex->dwRowCount, pIndex->dwBlockOffset));

		dwIndexRowCount += pIndex->dwRowCount;

		CString sNextKey;
		if (i < GetIndexCount() - 1)
			sNextKey = IndexGetKey(GetIndexEntry(i + 1));

		if (pIndex->dwBlockSize > 0)
			{
			DWORD dwErrorRows = 0;
			DWORD dwExtraRows = 0;
			DWORD dwMisplacedRows = 0;

			SBlockHeader *pBlock = NULL;
			try
				{
				m_Blocks.LoadBlock(pIndex->dwBlockOffset, pIndex->dwBlockSize, (void **)&pBlock);
				if (!pBlock)
					return false;
				}
			catch (...)
				{
				if (retsError)
					*retsError = strPattern("Unable to load block.");
				return false;
				}

			dwBlockRowCount += pBlock->dwRowCount;

			for (int j = 0; j < (int)pIndex->dwRowCount; j++)
				{
				CString sKey = BlockGetKey(pBlock, j);

				//	Make sure the key is in order (belongs in this block)

				if (CRowKey::Compare(m_Dims, CRowKey(m_Dims, sKey), CRowKey(m_Dims, sIndexKey)) > 0
						|| !sNextKey.IsEmpty() && CRowKey::Compare(m_Dims, CRowKey(m_Dims, sKey), CRowKey(m_Dims, sNextKey)) <= 0 && i != GetIndexCount() - 2)
					{
					dwErrorRows++;

					if (FindKey(CRowKey(m_Dims, sKey), NULL))
						{
						dwExtraRows++;
						}
					else
						{
						dwMisplacedRows++;
						}
					}
				}

			m_Blocks.UnloadBlock(pIndex->dwBlockOffset);

			//	If this is an invalid key, then see if we can find the key elsewhere.

			if (dwErrorRows > 0)
				{
				retLog.Insert(strPattern("ERROR: %d rows (out of %d) have key errors", dwErrorRows, pBlock->dwRowCount));
				retLog.Insert(strPattern("ERROR: %d rows were extra", dwExtraRows));
				retLog.Insert(strPattern("ERROR: %d rows were misplaced", dwMisplacedRows));
				bHasErrors = true;
				}
			}
		}

	//	Validate row counts

	if (dwIndexRowCount != m_pHeader->dwRowCount)
		{
		retLog.Insert(strPattern("ERROR: Index row count (%d) does not match segment row count (%d)", dwIndexRowCount, m_pHeader->dwRowCount));
		bHasErrors = true;
		}

	if (dwBlockRowCount != m_pHeader->dwRowCount)
		{
		retLog.Insert(strPattern("ERROR: Block row count (%d) does not match segment row count (%d)", dwBlockRowCount, m_pHeader->dwRowCount));
		bHasErrors = true;
		}

	//	If we have errors we will try to build a new segment with only the valid rows.

	if (bHasErrors)
		{
		retLog.Insert(strPattern("[%s]: Repairing", ::fileGetFilename(m_sFilespec)));

		DWORD dwRows = 0;
		DWORD dwRowsSkipped = 0;
		CRowKey PrevKey;

		CAeonRowArray Rows;
		Rows.Init(m_Dims);
		for (int i = 0; i < GetCount(); i++)
			{
			CRowKey Key;
			CDatum dData;
			SEQUENCENUMBER RowID = 0;
			SEQUENCENUMBER *pRowID = (HasRowID() ? &RowID : NULL);
			if (!GetRow(i, &Key, &dData, pRowID))
				{
				if (retsError) *retsError = strPattern("Unable to get row %d", i);
				return false;
				}

			//	If this key is out of order, then skip it

			if (PrevKey.IsEmpty() || CRowKey::Compare(m_Dims, Key, PrevKey) <= 0)
				{
				PrevKey = Key;
				dwRows++;
				}
			else
				{
				retLog.Insert(strPattern("Skipping row %d: %s", i, Key.AsEncodedString()));
				dwRowsSkipped++;
				continue;
				}

			Rows.Insert(Key, dData, RowID);
			}

		CRowIterator Iterator;
		Iterator.Init(m_Dims);
		Iterator.AddSegment(&Rows);

		CString sNewFilespec = CString("c:\\ArcologyConsole\\NewSegment.aseg");

		DWORD dwSegFlags = 0;
		dwSegFlags |= (HasRowID() ? CAeonSegment::FLAG_HAS_ROW_ID : 0);
		dwSegFlags |= (IsSecondaryView() ? CAeonSegment::FLAG_SECONDARY_VIEW : 0);

		CAeonSegment *pNewSeg = new CAeonSegment;
		if (!pNewSeg->Create(m_pHeader->dwViewID, m_Dims, GetSequence(), Iterator, sNewFilespec, dwSegFlags, retsError))
			{
			pNewSeg->Release();
			return false;
			}

		retLog.Insert(strPattern("[%s]: New segment created %d rows (%d rows skipped)", ::fileGetFilename(sNewFilespec), dwRows, dwRowsSkipped));
		}

	return true;
	}

AEONERR CAeonTable::Merge (CAeonTable& SrcTable, CString *retsError)
	{
	//	Lock both tables

	CSmartLock SrcLock(SrcTable.m_cs);
	CSmartLock Lock(m_cs);

	if (SrcTable.GetType() != GetType())
		return AEONERR_FAIL;

	//	Get the primary view.

	CAeonView* pDestView = m_Views.GetAt(DEFAULT_VIEW);
	if (!pDestView)
		return AEONERR_FAIL;

	CAeonView* pSrcView = m_Views.GetAt(DEFAULT_VIEW);
	if (!pSrcView)
		return AEONERR_FAIL;

	//	Make sure the dimensions match

	CTableDimensions Dims = pDestView->GetDimensions();
	CTableDimensions SrcDims = pSrcView->GetDimensions();
	if (Dims.GetCount() != SrcDims.GetCount())
		return AEONERR_FAIL;

	for (int i = 0; i < Dims.GetCount(); i++)
		{
		if (Dims[i].iKeyType != SrcDims[i].iKeyType)
			return AEONERR_FAIL;

		if (Dims[i].iSort != SrcDims[i].iSort)
			return AEONERR_FAIL;
		}

	//	Loop over all rows in the source table and add them to the destination table

	DWORD dwFlags = 0;

	CDatum dResult;
	if (!SrcTable.GetRows(DEFAULT_VIEW, CDatum(), -1, TArray<int>(), dwFlags, &dResult, retsError))
		return AEONERR_FAIL;

	DWORD dwRowsAdded = 0;
	for (int i = 0; i < dResult.GetCount(); i += 2)
		{
		CRowKey Key;
		if (!CRowKey::ParseKey(Dims, dResult.GetElement(i), &Key, retsError))
			return AEONERR_FAIL;

		CDatum dSrcData = dResult.GetElement(i + 1);

		CDatum dExisting;
		if (!GetData(DEFAULT_VIEW, Key, &dExisting, NULL, retsError))
			return AEONERR_FAIL;

		if (!dExisting.IsNil())
			continue;

		printf("%s: %s\n", (LPCSTR)Key.AsEncodedString(), (LPCSTR)dSrcData.GetElement(FIELD_NAME).AsString());

		//	Add the row

		if (Insert(Key, dSrcData, true, retsError) != AEONERR_OK)
			return AEONERR_FAIL;

		//	If this is a file view then we need to copy the file too.

		if (GetType() == typeFile)
			{
			CString sSrcStorage = dSrcData.GetElement(FIELD_STORAGE_PATH).AsString();
			const char* pPos = sSrcStorage.GetParsePointer();
			if (*pPos == '\\')
				pPos++;

			while (*pPos != '\0' && *pPos != '\\')
				pPos++;

			if (*pPos == '\\')
				{
				pPos++;
				sSrcStorage = CString("\\") + SrcTable.GetName() + CString("\\") + strSubString(sSrcStorage, (int)(pPos - sSrcStorage.GetParsePointer()));
				}

			CString sSrcFilespec = SrcTable.m_pStorage->CanonicalRelativeToMachine(SrcTable.m_sPrimaryVolume, sSrcStorage);
			CString sDestFilespec = m_pStorage->CanonicalRelativeToMachine(m_sPrimaryVolume, dSrcData.GetElement(FIELD_STORAGE_PATH).AsStringView());

			if (!fileExists(sDestFilespec))
				{
				if (!fileCopy(sSrcFilespec, sDestFilespec, retsError))
					return AEONERR_FAIL;
				}
			else
				{
				printf("Skipping file because it already exists: %s\n", (LPCSTR)sDestFilespec);
				}
			}

		dwRowsAdded++;
		}

	if (!Save(retsError))
		return AEONERR_FAIL;

	printf("Added %d rows\n", dwRowsAdded);

	return AEONERR_OK;
	}
