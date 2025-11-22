//	MsgGetStorageList.cpp
//
//	CExarchEngine class
//	Copyright (c) 2017 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_LOCAL_PATH,					"localPath");
DECLARE_CONST_STRING(FIELD_SPACE_FREE_GB,				"spaceFreeGB");

DECLARE_CONST_STRING(MNEMO_ARC_STORAGE,					"Arc.storage");

DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data");

void CExarchEngine::MsgGetStorageList (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetStorageList
//
//	Exarch.getStorageList

	{
	int i;

	//	No need to lock because we don't change state.

	//	Must be admin service

	if (!ValidateSandboxAdmin(Msg, pSecurityCtx))
		return;

	CString sMachine = GetMachineName();

	//	Prepare a list of structures

	CDatum List(CDatum::typeArray);

	//	Read Mnemosynth

	TArray<CString> VolumeKeys;
	MnemosynthReadCollection(MNEMO_ARC_STORAGE, &VolumeKeys);

	for (i = 0; i < VolumeKeys.GetCount(); i++)
		{
		//	Make a copy of the volume data (because we're going to change it).

		CDatum dVolume = MnemosynthRead(MNEMO_ARC_STORAGE, VolumeKeys[i]).Clone();

		//	If this volume is on our machine, then check free diskspace.
		//	LATER: Handle this for other machines.

		if (strStartsWith(VolumeKeys[i], sMachine))
			{
			CDatum dLocalPath = dVolume.GetElement(FIELD_LOCAL_PATH);
			DWORDLONG dwAvail;
			fileGetDriveSpace(dLocalPath.AsStringView(), &dwAvail);

			if (dwAvail > 0)
				{
				//	We report the free space in GB (10^9 bytes)

				dVolume.SetElement(FIELD_SPACE_FREE_GB, CDatum((double)dwAvail / 1000000000.0));
				}
			}

		//	Add to list

		List.Append(dVolume);
		}

	//	Done

	SendMessageReply(MSG_REPLY_DATA, List, Msg);
	}

