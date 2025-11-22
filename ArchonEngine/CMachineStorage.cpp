//	CMachineStorage.cpp
//
//	CMachineStorage class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_LOCAL_PATH,					"localPath");
DECLARE_CONST_STRING(FIELD_STATUS,						"status");
DECLARE_CONST_STRING(FIELD_VOLUME_NAME,					"volumeName");

DECLARE_CONST_STRING(MNEMO_ARC_STORAGE,					"Arc.storage");

DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");

DECLARE_CONST_STRING(STATUS_ONLINE,						"online");

DECLARE_CONST_STRING(ERR_DUPLICATE_VOLUME_NAME,			"New volume conflicts with existing volume name: %s.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_STORAGE,		"Unable to create storage path: %s.");
DECLARE_CONST_STRING(ERR_BAD_PATH,						"Unable to open storage path: %s.");

CString CMachineStorage::CanonicalToMachine (const CString &sFilespec)

//	CanonicalToMachine
//
//	Converts from a canonical path to a machine-specific path

	{
	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < m_Volumes.GetCount(); i++)
		if (strStartsWith(sFilespec, m_Volumes[i].sCanonicalRoot))
			return fileAppend(m_Volumes[i].sMachineRoot, strSubString(sFilespec, m_Volumes[i].sCanonicalRoot.GetLength()));

	return NULL_STR;
	}

CString CMachineStorage::CanonicalRelativeToMachine (const CString &sVolume, const CString &sFilespec)

//	CanonicalRelativeToMachine
//
//	Converts from a canonical path with no volume or engine to a machine-specific path.

	{
	CSmartLock Lock(m_cs);

	int iIndex;
	if (!FindVolume(sVolume, &iIndex))
		return NULL_STR;

	return fileAppend(m_Volumes[iIndex].sMachineRoot, sFilespec);
	}

bool CMachineStorage::FindVolume (const CString &sVolume, int *retiIndex) const

//	FindVolume
//
//	Returns the index of the volume by name

	{
	int i;

	for (i = 0; i < m_Volumes.GetCount(); i++)
		if (strEquals(sVolume, m_Volumes[i].sVolumeName))
			{
			if (retiIndex)
				*retiIndex = i;

			return true;
			}
	
	return false;
	}

const CString &CMachineStorage::GetPath (const CString &sVolume) const

//	GetPath
//
//	Returns the root machine path for the given volume.

	{
	CSmartLock Lock(m_cs);
	int iIndex;

	if (!FindVolume(sVolume, &iIndex))
		return NULL_STR;

	return GetPath(iIndex);
	}

const CString &CMachineStorage::GetRedundantVolume (const CString &sVolume) const

//	GetRedundantVolume
//
//	Returns a volume that is different from the given volume, suitable for
//	creating a backup.
//
//	If no backup is available we return NULL_STR.
//
//	NOTE: If sVolume is not a current volume then we return an arbitrary
//	existing volume.

	{
	CSmartLock Lock(m_cs);

	int iIndex;
	if (FindVolume(sVolume, &iIndex))
		{
		if (m_Volumes.GetCount() == 1)
			return NULL_STR;

		return GetVolume((iIndex + 1) % m_Volumes.GetCount());
		}
	else
		return GetVolume(mathRandom(0, m_Volumes.GetCount() - 1));
	}

CString CMachineStorage::GetVolumePath (const CString &sFilespec)

//	GetVolumePath
//
//	Returns the machine root path of the filespec

	{
	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < m_Volumes.GetCount(); i++)
		if (strStartsWith(sFilespec, m_Volumes[i].sMachineRoot))
			return m_Volumes[i].sMachineRoot;

	return NULL_STR;
	}

bool CMachineStorage::Init (IArchonProcessCtx *pProcess, const CString &sEngineDirectory, CString *retsError)

//	Init
//
//	Initializes from Mnemosynth

	{
	CSmartLock Lock(m_cs);

	m_sEngineDirectory = sEngineDirectory;

	//	Load

	if (!Reinit(pProcess, NULL, NULL, retsError))
		return false;

	//	Done

	return true;
	}

bool CMachineStorage::InitLocal (const CString &sStoragePath, const CString &sEngineDirectory, CString *retsError)

//	InitLocal
//
//	Initializes with a single volume path. This is used for console mode.

	{
	m_sEngineDirectory = sEngineDirectory;
	CString sVolumeName = CString("Vol01");

	//	Add a path for the engine directory

	CString sCanonicalRoot = fileAppend(sVolumeName, m_sEngineDirectory);
	CString sMachineRoot = fileAppend(sStoragePath, m_sEngineDirectory);

	//	Make sure the path exists

	if (!fileExists(sMachineRoot))
		{
		//	Try to create it

		if (!filePathCreate(sMachineRoot))
			{
			if (retsError) *retsError = strPattern(ERR_BAD_PATH, sMachineRoot);
			return false;
			}
		}

	//	Add a new entry

	SVolumeDesc *pEntry = m_Volumes.Insert();

	//	Initialize

	pEntry->sResourceName = sVolumeName;
	pEntry->sVolumeName = sVolumeName;
	pEntry->sPath = sStoragePath;
	pEntry->sCanonicalRoot = sCanonicalRoot;
	pEntry->sMachineRoot = sMachineRoot;
	pEntry->bMarked = true;

	//	Done

	return true;
	}

CString CMachineStorage::MachineToCanonical (const CString &sFilespec)

//	MachineToCanonical
//
//	Converts from a machine-specific path to a canonical path

	{
	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < m_Volumes.GetCount(); i++)
		if (strStartsWith(sFilespec, m_Volumes[i].sMachineRoot))
			return fileAppend(m_Volumes[i].sCanonicalRoot, strSubString(sFilespec, m_Volumes[i].sMachineRoot.GetLength()));

	return NULL_STR;
	}

CString CMachineStorage::MachineToCanonicalRelative (const CString &sFilespec, CString *retsVolume)

//	MachineToCanonicalRelative
//
//	Converts from a machine-specific path to a canonical path (the volume is 
//	returned separately).

	{
	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < m_Volumes.GetCount(); i++)
		if (strStartsWith(sFilespec, m_Volumes[i].sMachineRoot))
			{
			if (retsVolume)
				*retsVolume = m_Volumes[i].sVolumeName;

			return strSubString(sFilespec, m_Volumes[i].sMachineRoot.GetLength());
			}

	return NULL_STR;
	}

bool CMachineStorage::Reinit (IArchonProcessCtx *pProcess, TArray<CString> *retVolumesAdded, TArray<CString> *retVolumesDeleted, CString *retsError)

//	Reinit
//
//	Reinitialize from Mnemosynth

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Get the list of volumes in the entire arcology

	TArray<CString> VolumeKeys;
	pProcess->MnemosynthReadCollection(MNEMO_ARC_STORAGE, &VolumeKeys);

	//	Clear the mark on all our current volumes. Later we will mark any
	//	volumes that still exist.

	for (i = 0; i < m_Volumes.GetCount(); i++)
		m_Volumes[i].bMarked = false;

	//	Loop over all volumes; add the ones on this machine to the list

	CString sMachine = pProcess->GetMachineName();
	for (i = 0; i < VolumeKeys.GetCount(); i++)
		{
		if (strStartsWith(VolumeKeys[i], sMachine))
			{
			CDatum dVolumeDesc = pProcess->MnemosynthRead(MNEMO_ARC_STORAGE, VolumeKeys[i]);

			//	Make sure the volume is online

			CStringView sStatus = dVolumeDesc.GetElement(FIELD_STATUS);
			if (!strEquals(sStatus, STATUS_ONLINE))
				continue;

			//	Get volume data

			CStringView sVolumeName = dVolumeDesc.GetElement(FIELD_VOLUME_NAME);
			CStringView sPath = dVolumeDesc.GetElement(FIELD_LOCAL_PATH);

			//	Add a path for the engine directory

			CString sCanonicalRoot = fileAppend(sVolumeName, m_sEngineDirectory);
			CString sMachineRoot = fileAppend(sPath, m_sEngineDirectory);

			//	Make sure the path exists

			if (!fileExists(sMachineRoot))
				{
				//	Try to create it

				if (!filePathCreate(sMachineRoot))
					{
					pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_UNABLE_TO_CREATE_STORAGE, sMachineRoot));
					continue;
					}
				}

			//	Do we have this volume already? If not, insert a new entry.

			int iVol;
			SVolumeDesc *pEntry;
			if (FindVolume(sVolumeName, &iVol))
				{
				pEntry = &m_Volumes[iVol];

				//	Mark it

				pEntry->bMarked = true;

				//	We expect resource name and path to be the same (otherwise
				//	the caller should have created a new volume).

				if (!strEquals(pEntry->sResourceName, VolumeKeys[i]) || !strEquals(pEntry->sPath, sPath))
					{
					pProcess->Log(MSG_LOG_ERROR, strPattern(ERR_DUPLICATE_VOLUME_NAME, sVolumeName));
					continue;
					}
				}
			else
				{
				pEntry = m_Volumes.Insert();

				//	Initialize

				pEntry->sResourceName = VolumeKeys[i];
				pEntry->sVolumeName = sVolumeName;
				pEntry->sPath = sPath;
				pEntry->sCanonicalRoot = sCanonicalRoot;
				pEntry->sMachineRoot = sMachineRoot;
				pEntry->bMarked = true;

				//	Add this to the list of volumes inserted.

				if (retVolumesAdded)
					retVolumesAdded->Insert(sVolumeName);
				}
			}
		}

	//	Delete any unmarked volumes

	for (i = 0; i < m_Volumes.GetCount(); i++)
		if (!m_Volumes[i].bMarked)
			{
			if (retVolumesDeleted)
				retVolumesDeleted->Insert(m_Volumes[i].sVolumeName);

			m_Volumes.Delete(i);

			i--;
			}

	//	Done

	return true;
	}
