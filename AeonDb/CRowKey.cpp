//	CRowKey.cpp
//
//	CRowKey class
//	Copyright (c) 2012 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_INVALID_PATH,					"Invalid rowPath: %s");

CRowKey::CRowKey (void)

//	CRowKey constructor

	{
	}

CRowKey::CRowKey (const CRowKey &Src)

//	CRowKey copy constructor

	{
	m_iCount = Src.m_iCount;
	m_psKey = new CString(Src.AsEncodedString());
	m_bFree = true;
	}

CRowKey::CRowKey (const CTableDimensions &Dims, const CString &sKey) :
		m_iCount(Dims.GetCount()),
		m_psKey(&sKey),
		m_bFree(false)

//	CRowKey constructor

	{
	}

CRowKey::~CRowKey (void)

//	CRowKey destructor

	{
	CleanUp();
	}

CRowKey &CRowKey::operator= (const CRowKey &Src)

//	CRowKey operator =

	{
	CleanUp();

	m_iCount = Src.m_iCount;
	m_psKey = new CString(Src.AsEncodedString());
	m_bFree = true;

	return *this;
	}

CDatum CRowKey::AsDatum (const CTableDimensions &Dims) const

//	AsDatum
//
//	Converts the key to a datum. This is the reverse of ParseKey

	{
	if (m_iCount == 1)
		return AsDatum(m_psKey->GetParsePointer(), Dims[0].iKeyType);
	else
		{
		CComplexArray *pArray = new CComplexArray;

		char *pPos = m_psKey->GetParsePointer();
		char *pPosEnd = pPos + m_psKey->GetLength();
		int i = 0;
		while (pPos < pPosEnd && i < Dims.GetCount())
			{
			pArray->Insert(AsDatum(pPos, Dims[i].iKeyType, &pPos));
			i++;
			}

		return CDatum(pArray);
		}
	}

CDatum CRowKey::AsDatum (char *pPos, EKeyTypes iKeyType, char **retpPos) const

//	AsDatum
//
//	Converts the key part to a datum. Optionally returns the new parse position.

	{
	CDatum dResult;

	switch (iKeyType)
		{
		case keyDatetime:
			{
			CString sDate(pPos);
			CComplexDateTime::CreateFromString(sDate, &dResult);

			//	Next

			if (retpPos)
				*retpPos = pPos + sDate.GetLength() + 1;
			break;
			}

		case keyInt32:
			{
			dResult = CDatum(*(int *)pPos);

			//	Next

			if (retpPos)
				*retpPos = pPos + sizeof(int) + 1;
			break;
			}

		case keyInt64:
			{
			dResult = CDatum(*(DWORDLONG *)pPos);

			//	Next

			if (retpPos)
				*retpPos = pPos + sizeof(DWORDLONG) + 1;
			break;
			}

		case keyListUTF8:
		case keyUTF8:
			{
			CString sValue(pPos);
			dResult = CString(sValue);

			//	Next

			if (retpPos)
				*retpPos = pPos + sValue.GetLength() + 1;
			break;
			}
		}

	return dResult;
	}

void CRowKey::CleanUp (void)

//	CleanUp
//
//	Clean up

	{
	if (m_bFree && m_psKey)
		delete m_psKey;

	m_psKey = NULL;
	m_iCount = 0;
	m_bFree = false;
	}

int CRowKey::Compare (const CTableDimensions &Dims, const CRowKey &Key1, const CRowKey &Key2)

//	Compare
//
//	-1:	Key2 followed by Key1
//	 0: Key1 and Key2 are equal
//	 1: Key1 followed by Key2

	{
	int iDim = 0;
	char *pKey1 = Key1.m_psKey->GetParsePointer();
	char *pKey1End = pKey1 + Key1.m_psKey->GetLength();
	char *pKey2 = Key2.m_psKey->GetParsePointer();
	char *pKey2End = pKey2 + Key2.m_psKey->GetLength();

	while (iDim < Dims.GetCount())
		{
		ESortOptions iSort = Dims[iDim].iSort;

		switch (Dims[iDim].iKeyType)
			{
			case keyInt32:
				{
				//	Handle edge case where we don't have both keys

				bool bHasKey1 = (pKey1 + sizeof(int) <= pKey1End);
				bool bHasKey2 = (pKey2 + sizeof(int) <= pKey2End);
				if (!bHasKey1 && !bHasKey2)
					return 0;
				else if (!bHasKey1)
					return 1;
				else if (!bHasKey2)
					return -1;

				//	We've got both keys

				int *pIntKey1 = (int *)pKey1;
				int *pIntKey2 = (int *)pKey2;
				if (*pIntKey1 > *pIntKey2)
					return iSort;
				else if (*pIntKey1 < *pIntKey2)
					return -iSort;
				else
					{
					//	Skip integer plus null-terminator

					pKey1 += sizeof(int) + 1;
					pKey2 += sizeof(int) + 1;

					//	Continue to next dimension
					break;
					}
				}

			case keyInt64:
				{
				//	Handle edge case where we don't have both keys

				bool bHasKey1 = (pKey1 + sizeof(DWORDLONG) <= pKey1End);
				bool bHasKey2 = (pKey2 + sizeof(DWORDLONG) <= pKey2End);
				if (!bHasKey1 && !bHasKey2)
					return 0;
				else if (!bHasKey1)
					return 1;
				else if (!bHasKey2)
					return -1;

				//	We've got both keys

				DWORDLONG *pIntKey1 = (DWORDLONG *)pKey1;
				DWORDLONG *pIntKey2 = (DWORDLONG *)pKey2;
				if (*pIntKey1 > *pIntKey2)
					return iSort;
				else if (*pIntKey1 < *pIntKey2)
					return -iSort;
				else
					{
					//	Skip integer plus null-terminator

					pKey1 += sizeof(DWORDLONG) + 1;
					pKey2 += sizeof(DWORDLONG) + 1;

					//	Continue to next dimension
					break;
					}
				}

			case keyDatetime:
			case keyListUTF8:
			case keyUTF8:
				{
				//	Handle edge case where we don't have both keys

				bool bHasKey1 = (pKey1 < pKey1End && *pKey1 != '\0');
				bool bHasKey2 = (pKey2 < pKey2End && *pKey2 != '\0');
				if (!bHasKey1 && !bHasKey2)
					return 0;
				else if (!bHasKey1)
					return 1;
				else if (!bHasKey2)
					return -1;

				//	We've got both keys

				int iCompare = iSort * KeyCompare(pKey1, pKey2);
				if (iCompare != 0)
					return iCompare;

				//	If equal then we continue

				while (*pKey1 != '\0' && pKey1 < pKey1End)
					pKey1++;

				while (*pKey2 != '\0' && pKey2 < pKey2End)
					pKey2++;

				pKey1++;
				pKey2++;
				break;
				}
			}

		//	Next dimension

		iDim++;
		}

	//	If we get this far then they are equal

	return 0;
	}

bool CRowKey::ComparePartial (const CTableDimensions &Dims, const CRowKey &PartialKey, const CRowKey &Key)

//	ComparePartial
//
//	Returns TRUE if PartialKey matched Key (for the dimensions that it contains).

	{
	int iDim = 0;
	char *pKeyPartial = PartialKey.m_psKey->GetParsePointer();
	char *pKeyPartialEnd = pKeyPartial + PartialKey.m_psKey->GetLength();
	char *pKey = Key.m_psKey->GetParsePointer();
	char *pKeyEnd = pKey + Key.m_psKey->GetLength();

	while (iDim < Dims.GetCount())
		{
		ESortOptions iSort = Dims[iDim].iSort;

		switch (Dims[iDim].iKeyType)
			{
			case keyInt32:
				{
				//	Handle edge case where we don't have both keys

				bool bHasKeyPartial = (pKeyPartial + sizeof(int) <= pKeyPartialEnd);
				bool bHasKey = (pKey + sizeof(int) <= pKeyEnd);
				if (!bHasKeyPartial && !bHasKey)
					return false;
				else if (!bHasKeyPartial)
					return true;
				else if (!bHasKey)
					return false;

				//	We've got both keys

				int *pIntKey1 = (int *)pKeyPartial;
				int *pIntKey2 = (int *)pKey;
				if (*pIntKey1 != *pIntKey2)
					return false;
				else
					{
					//	Skip integer plus null-terminator

					pKeyPartial += sizeof(int) + 1;
					pKey += sizeof(int) + 1;

					//	Continue to next dimension
					break;
					}
				}

			case keyInt64:
				{
				//	Handle edge case where we don't have both keys

				bool bHasKeyPartial = (pKeyPartial + sizeof(DWORDLONG) <= pKeyPartialEnd);
				bool bHasKey = (pKey + sizeof(DWORDLONG) <= pKeyEnd);
				if (!bHasKeyPartial && !bHasKey)
					return false;
				else if (!bHasKeyPartial)
					return true;
				else if (!bHasKey)
					return false;

				//	We've got both keys

				DWORDLONG *pIntKey1 = (DWORDLONG *)pKeyPartial;
				DWORDLONG *pIntKey2 = (DWORDLONG *)pKey;
				if (*pIntKey1 != *pIntKey2)
					return false;
				else
					{
					//	Skip integer plus null-terminator

					pKeyPartial += sizeof(DWORDLONG) + 1;
					pKey += sizeof(DWORDLONG) + 1;

					//	Continue to next dimension
					break;
					}
				}

			case keyDatetime:
			case keyListUTF8:
			case keyUTF8:
				{
				//	Handle edge case where we don't have both keys

				bool bHasKeyPartial = (pKeyPartial < pKeyPartialEnd && *pKeyPartial != '\0');
				bool bHasKey = (pKey < pKeyEnd && *pKey != '\0');
				if (!bHasKeyPartial && !bHasKey)
					return false;
				else if (!bHasKeyPartial)
					return true;
				else if (!bHasKey)
					return false;

				//	We've got both keys

				int iCompare = iSort * KeyCompare(pKeyPartial, pKey);
				if (iCompare != 0)
					return false;

				//	If equal then we continue

				while (*pKeyPartial != '\0' && pKeyPartial < pKeyPartialEnd)
					pKeyPartial++;

				while (*pKey != '\0' && pKey < pKeyEnd)
					pKey++;

				pKeyPartial++;
				pKey++;
				break;
				}
			}

		//	Next dimension

		iDim++;
		}

	//	If we get this far then they match

	return true;
	}

void CRowKey::CreateFromDatumAndRowID (const CTableDimensions &Dims, const TArray<CDatum> &Data, SEQUENCENUMBER RowID, CRowKey *retKey)

//	CreateFromDatumAndRowID
//
//	Creates a secondary key. The last dimension is always the rowID.

	{
	int i;

	ASSERT(Dims[Dims.GetCount() - 1].iKeyType == keyInt64);

	retKey->CleanUp();
	retKey->m_iCount = Dims.GetCount();

	//	Load the key parts based on its type

	CStringBuffer Buffer;
	for (i = 0; i < Dims.GetCount() - 1; i++)
		{
		//	Key parts are separated by NULL.

		if (i != 0)
			Buffer.Write("\0", 1);

		//	Write out the key part based on type

		if (i < Data.GetCount())
			WriteKeyPart(Buffer, Dims[i].iKeyType, Data[i]);
		else
			WriteKeyPart(Buffer, Dims[i].iKeyType, CDatum());
		}

	//	Now write the rowID

	Buffer.Write("\0", 1);
	Buffer.Write(&RowID, sizeof(RowID));

	//	Set the key

	CString *pResult = new CString;
	pResult->TakeHandoff(Buffer);

	retKey->m_psKey = pResult;
	retKey->m_bFree = true;
	}

void CRowKey::CreateFromEncodedKey (const CTableDimensions &Dims, const CString &sKey, CRowKey *retKey, bool bMakeCopy)

//	CreateFromEncodedKey
//
//	Creates a key from a string

	{
	retKey->CleanUp();

	retKey->m_iCount = Dims.GetCount();
	if (bMakeCopy)
		{
		retKey->m_psKey = new CString(sKey);
		retKey->m_bFree = true;
		}
	else
		{
		retKey->m_psKey = &sKey;
		retKey->m_bFree = false;
		}
	}

void CRowKey::CreateFromEncodedKeyPartial (const CTableDimensions &Dims, int iPartialDims, const CString &sKey, CRowKey *retKey)

//	CreateFromEncodedKeyPartial
//
//	Creates a partial key of the given dimensions.

	{
	ASSERT(iPartialDims <= Dims.GetCount());

	int iPart = 0;
	char *pPos = sKey.GetParsePointer();
	char *pPosEnd = pPos + sKey.GetLength();

	char *pStart = pPos;
	while (iPart < iPartialDims && pPos < pPosEnd)
		{
		switch (Dims[iPart].iKeyType)
			{
			//	Int keys are never NULL (if parsing succeeded).

			case keyInt32:
				pPos += sizeof(int) + 1;
				break;

			case keyInt64:
				pPos += sizeof(DWORDLONG) + 1;
				break;

			//	These are encoded as strings.

			case keyDatetime:
			case keyListUTF8:
			case keyUTF8:
				//	Skip to next part

				while (*pPos != '\0' && pPos < pPosEnd)
					pPos++;

				//	Skip null-terminator

				pPos++;
				break;

			default:
				ASSERT(false);
			}

		//	Next part

		iPart++;
		}

	//	Done

	retKey->CleanUp();
	retKey->m_iCount = iPartialDims;
	retKey->m_psKey = new CString(pStart, pPos - pStart);
	retKey->m_bFree = true;
	}

void CRowKey::CreateFromFilePath (const CString &sFilePath, CRowKey *retpKey, bool bIsKey)

//	CreateFromFilePath
//
//	Creates a key to a file-type table.

	{
	retpKey->CleanUp();

	retpKey->m_iCount = 1;
	if (bIsKey)
		retpKey->m_psKey = new CString(sFilePath);
	else
		retpKey->m_psKey = new CString(FilePathToKey(sFilePath));
	retpKey->m_bFree = true;
	}

CString CRowKey::FilePathToKey (const CString &sFilePath)

//	FilePathToKey
//
//	Converts a filepath (excluding the table name) to a key. To implement
//	efficient directory operations we surround all directory names with
//	brackets []. This insures that all directories sort at the end.
//
//	NOTE: A filepath always starts with a /

	{
	char *pPos = sFilePath.GetParsePointer();
	char *pEndPos = pPos + sFilePath.GetLength();
	CStringBuffer Output;

	char *pStart = NULL;
	while (pPos < pEndPos)
		{
		//	If we have a slash and we have a component, then output the 
		//	component with brackets.

		if (*pPos == '/')
			{
			if (pStart)
				{
				Output.Write("[", 1);
				Output.Write(pStart, pPos - pStart);
				Output.Write("]/", 2);
				}

			//	If we don't have a component then this is the leading slash.

			else
				Output.Write("/", 1);

			pStart = pPos + 1;
			}

		pPos++;
		}

	//	The last component is the filename and needs no brackets

	if (pStart)
		Output.Write(pStart, pPos - pStart);

	//	Done

	return CString::CreateFromHandoff(Output);
	}

bool CRowKey::HasNullDimensions (const CTableDimensions &Dims) const

//	HasNullDimensions
//
//	Returns TRUE if any of the dimensions are NULL.

	{
	int iPart = 0;
	char *pPos = m_psKey->GetParsePointer();
	char *pPosEnd = pPos + m_psKey->GetLength();

	while (iPart < Dims.GetCount())
		{
		switch (Dims[iPart].iKeyType)
			{
			//	Int keys are never NULL (if parsing succeeded).

			case keyInt32:
				//	Skip integer plus null-terminator
				pPos += sizeof(int) + 1;
				break;

			case keyInt64:
				//	Skip integer plus null-terminator
				pPos += sizeof(DWORDLONG) + 1;
				break;

			//	These are encoded as strings.

			case keyDatetime:
			case keyListUTF8:
			case keyUTF8:
				if (*pPos == '\0')
					return true;

				//	Skip to next part

				while (*pPos != '\0' && pPos < pPosEnd)
					pPos++;

				//	Skip null-terminator

				pPos++;
				break;

			default:
				ASSERT(false);
			}

		//	Next part

		iPart++;
		}

	//	OK

	return false;
	}

CString CRowKey::KeyToFilePath (const CString &sKey)

//	KeyToFilePath
//
//	Converts a table key to a filepath. This is the reverse of FilePathToKey.

	{
	char *pPos = sKey.GetParsePointer();
	char *pEndPos = pPos + sKey.GetLength();
	CStringBuffer Output;

	char *pStart = NULL;
	while (pPos < pEndPos)
		{
		//	If we have a slash and we have a component, then output the 
		//	component without brackets.

		if (*pPos == '/')
			{
			if (pStart)
				{
				int iComponentLen = (int)(pPos - pStart);

				//	If we have a component in brackets the remove the brackets.

				if (iComponentLen > 2 && pStart[0] == '[' && pPos[-1] == ']')
					Output.Write(pStart + 1, iComponentLen - 2);

				//	Otherwise we write it out. [This should not happen, but we
				//	include the code to make sure we don't crash.]

				else
					Output.Write(pStart, iComponentLen);

				Output.Write("/", 1);
				}

			//	If we don't have a component then this is the leading slash.

			else
				Output.Write("/", 1);

			pStart = pPos + 1;
			}

		pPos++;
		}

	//	The last component is the filename and has no brackets

	if (pStart)
		Output.Write(pStart, pPos - pStart);

	//	Done

	return CString::CreateFromHandoff(Output);
	}

bool CRowKey::MatchesDimensions (const CTableDimensions &Dims) const

//	MatchesDimensions
//
//	Returns TRUE if the key matches the given dimensions.

	{
	return (Dims.GetCount() == m_iCount);
	}

bool CRowKey::ParseKey (const CTableDimensions &Dims, CDatum dKey, CRowKey *retpKey, CString *retsError)

//	ParseKey
//
//	Parses a key from a datum.

	{
	int i;

	//	Ignore any nil values at the end. It is valid to omit values at the end
	//	for finding partial matches. This makes nil values consistent with
	//	omitting them.

	int iKeyCount = dKey.GetArrayCount();
	while (iKeyCount > 0 && dKey.GetArrayElement(iKeyCount - 1).IsNil())
		iKeyCount--;

	//	Initialize

	retpKey->CleanUp();

	retpKey->m_iCount = iKeyCount;
	if (retpKey->m_iCount > Dims.GetCount())
		{
		*retsError = strPattern(ERR_INVALID_PATH, dKey.AsString());
		return false;
		}

	//	Load the key based on its type

	CStringBuffer Buffer;
	for (i = 0; i < iKeyCount; i++)
		{
		//	Key parts are separated by NULL.

		if (i != 0)
			Buffer.Write("\0", 1);

		//	Write out the key part based on type

		WriteKeyPart(Buffer, Dims[i].iKeyType, dKey.GetArrayElement(i));
		}

	//	Set the key

	CString *pResult = new CString;
	pResult->TakeHandoff(Buffer);

	retpKey->m_psKey = pResult;
	retpKey->m_bFree = true;

	return true;
	}

bool CRowKey::ParseKeyForCreate (const CTableDimensions &Dims, CDatum dKey, CRowKey *retpKey, CString *retsError)

//	ParseKeyForCreate
//
//	Parses a key from a datum and validates that the key can be used to insert
//	a new row.

	{
	if (!ParseKey(Dims, dKey, retpKey, retsError))
		return false;

	//	Make sure all the keys are non-null

	int iPart = 0;
	char *pPos = retpKey->m_psKey->GetParsePointer();
	char *pPosEnd = pPos + retpKey->m_psKey->GetLength();

	while (iPart < Dims.GetCount())
		{
		switch (Dims[iPart].iKeyType)
			{
			//	Int keys are never NULL (if parsing succeeded).

			case keyInt32:
				//	Skip integer plus null-terminator
				pPos += sizeof(int) + 1;
				break;

			case keyInt64:
				//	Skip integer plus null-terminator
				pPos += sizeof(DWORDLONG) + 1;
				break;

			//	These are encoded as strings.

			case keyDatetime:
			case keyListUTF8:
			case keyUTF8:
				if (*pPos == '\0')
					{
					*retsError = strPattern(ERR_INVALID_PATH, dKey.AsString());
					return false;
					}

				//	Skip to next part

				while (*pPos != '\0' && pPos < pPosEnd)
					pPos++;

				//	Skip null-terminator

				pPos++;
				break;

			default:
				ASSERT(false);
			}

		//	Next part

		iPart++;
		}

	//	OK

	return true;
	}

void CRowKey::WriteKeyPart (CStringBuffer &Buffer, EKeyTypes iKeyType, CDatum dKey)

//	WriteKeyPart
//
//	Writes out the keypart to the buffer.

	{
	//	Write out the key part based on type

	switch (iKeyType)
		{
		case keyDatetime:
			if (dKey.GetBasicType() == CDatum::typeDateTime)
				Buffer.Write(dKey.AsString());
			else if (dKey.GetBasicType() == CDatum::typeString)
				Buffer.Write(dKey.AsString());
			else
				Buffer.Write(NULL_STR);
			break;

		case keyInt32:
			{
			DWORD KeyPart = (DWORD)(int)dKey;
			Buffer.Write(&KeyPart, sizeof(DWORD));
			break;
			}

		case keyInt64:
			{
			DWORDLONG KeyPart = (DWORDLONG)dKey;
			Buffer.Write(&KeyPart, sizeof(DWORDLONG));
			break;
			}

		case keyListUTF8:
		case keyUTF8:
			Buffer.Write(dKey.AsString());
			break;

		default:
			ASSERT(false);
		}
	}
