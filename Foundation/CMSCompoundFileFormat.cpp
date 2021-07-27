//	CMSCompoundFileFormat.cpp
//
//	CMSCompoundFileFormat class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.
//
//	REFERENCES
//
//	https://en.wikipedia.org/wiki/Compound_File_Binary_Format
//	https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-cfb/53989ce4-7b05-4f8d-829b-d08d6148375b

#include "stdafx.h"

void CMSCompoundFileFormat::AddStream (const CString &sName, IMemoryBlock &Data)

//	AddStream
//
//	Adds a new document stream. The stream lifetime must continue until the last
//	Write call to this object.

	{
	SStream *pDoc = m_Docs.Insert();
	pDoc->sName = sName;
	pDoc->pData = &Data;
	}

bool CMSCompoundFileFormat::Write (IByteStream &Stream)

//	Write
//
//	Write out the whole file.

	{
	CFF_HEADER Header;

	//	Skit the header. We'll go back and fix it up later.

	Stream.Seek(sizeof(CFF_HEADER));

	//	Write all the streams (and track their position, etc.)

	int iSector = 0;
	for (int i = 0; i < m_Docs.GetCount(); i++)
		{
		int iSize = m_Docs[i].pData->GetLength();
		Stream.Write(m_Docs[i].pData->GetPointer(), m_Docs[i].pData->GetLength());

		//	Make sure we align on sector boundaries

		int iAlignedSize = AlignUp(iSize, SECTOR_SIZE);
		Stream.WriteChar('\0', iAlignedSize - iSize);

		//	Remember the starting sector and size.

		m_Docs[i].iSector = iSector;
		m_Docs[i].iSizeInSectors = iAlignedSize / SECTOR_SIZE;

		//	Next stream.

		iSector += m_Docs[i].iSizeInSectors;
		}

	//	Now write the directory

	int iSectorsWritten;
	WriteDirectory(Stream, Header, iSector, &iSectorsWritten);

	iSector += iSectorsWritten;

	//	Now write the FAT sector

	WriteFAT(Stream, Header, iSector, iSectorsWritten);

	//	Now write the header.

	Stream.Seek(0);
	Stream.Write(&Header, sizeof(Header));

	return true;
	}

void CMSCompoundFileFormat::WriteDirectory (IByteStream &Stream, CFF_HEADER &Header, int iSector, int *retiSectorsWritten)
	{
	TArray<CFF_DIRECTORY> Directory;

	//	First entry is always the root entry

	CFF_DIRECTORY *pRoot = Directory.Insert();
	SetName(*pRoot, CString("Root Entry"));
	pRoot->byObjType = CFF_DIRECTORY::TYPE_ROOT;
	pRoot->dwChildID = 1;
	pRoot->dwStartSector = ENDOFCHAIN;

	//	Now add each entry

	DWORD dwPrevChild = 0xFFFFFFFF;
	CFF_DIRECTORY *pPrevChild = NULL;
	for (int i = 0; i < m_Docs.GetCount(); i++)
		{
		DWORD dwID = Directory.GetCount();

		CFF_DIRECTORY *pChild = Directory.Insert();
		SetName(*pChild, m_Docs[i].sName);
		pChild->byObjType = CFF_DIRECTORY::TYPE_STREAM;

		if (pPrevChild)
			{
			pChild->dwLeftSibling = dwPrevChild;
			pPrevChild->dwRightSibling = dwID;
			}

		pChild->dwStartSector = m_Docs[i].iSector;
		pChild->dwStreamSize = m_Docs[i].pData->GetLength();

		pPrevChild = pChild;
		dwPrevChild = dwID;
		}

	//	Write out.

	Stream.Write(&Directory[0], Directory.GetCount() * sizeof(CFF_DIRECTORY));

	//	Write out some blank entries

	int iFullCount = AlignUp(Directory.GetCount(), DIRECTORY_ENTRIES_PER_SECTOR);
	int iNeeded = iFullCount - Directory.GetCount();
	for (int i = 0; i < iNeeded; i++)
		{
		CFF_DIRECTORY Blank;
		utlMemSet(&Blank, sizeof(Blank));
		Blank.dwLeftSibling = 0xFFFFFFFF;
		Blank.dwRightSibling = 0xFFFFFFFF;
		Blank.dwChildID = 0xFFFFFFFF;

		Stream.Write(&Blank, sizeof(Blank));
		}

	*retiSectorsWritten = iFullCount / DIRECTORY_ENTRIES_PER_SECTOR;

	//	Update the header

	Header.dwSectDirStart = iSector;
	}

void CMSCompoundFileFormat::WriteFAT (IByteStream &Stream, CFF_HEADER &Header, int iSector, int iDirectorySectorSize)
	{
	TArray<DWORD> FAT;

	int iNextSector = 0;
	for (int i = 0; i < m_Docs.GetCount(); i++)
		{
		for (int j = 0; j < m_Docs[i].iSizeInSectors; j++)
			{
			iNextSector++;

			if (j + 1 < m_Docs[i].iSizeInSectors)
				FAT.Insert(iNextSector);
			else
				FAT.Insert(ENDOFCHAIN);
			}
		}

	//	Add the directory

	for (int i = 0; i < iDirectorySectorSize; i++)
		{
		iNextSector++;
		if (i + 1 < iDirectorySectorSize)
			FAT.Insert(iNextSector);
		else
			FAT.Insert(ENDOFCHAIN);
		}

	//	Count FAT sectors

	int iFATSectors = AlignUp(1 + FAT.GetCount(), FAT_ENTRIES_PER_SECTOR) / FAT_ENTRIES_PER_SECTOR;
	for (int i = 0; i < iFATSectors; i++)
		{
		FAT.Insert(FAT_SECTOR_ENTRY);
		}

	int iTotal = AlignUp(FAT.GetCount(), FAT_ENTRIES_PER_SECTOR);
	int iNeeded = iTotal - FAT.GetCount();
	for (int i = 0; i < iNeeded; i++)
		{
		FAT.Insert(FREESECT);
		}

	//	Write it out

	Stream.Write(&FAT[0], sizeof(DWORD) * FAT.GetCount());

	//	Update the header

	Header.dwSectFAT = FAT.GetCount() / FAT_ENTRIES_PER_SECTOR;
	for (int i = 0; i < Min((int)Header.dwSectFAT, DIFAT_IN_HEADER); i++)
		Header.DIFAT[i] = iSector + i;
	}

void CMSCompoundFileFormat::SetName (CFF_DIRECTORY &Entry, const CString &sName)
	{
	CString16 Name(sName);
	Entry.wNameLen = (WORD)(sizeof(TCHAR) * (Name.GetLength() + 1));
	utlMemSet(Entry.Name, sizeof(Entry.Name));
	utlMemCopy((LPTSTR)Name, Entry.Name, Entry.wNameLen);
	}
