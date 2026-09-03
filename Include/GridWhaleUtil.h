//	GridWhaleUtil.h
//
//	GridWhale Classes and Functions
//	Copyright (c) 2020 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "AEON.h"
#include "HTTPUtil.h"

struct SHexarcMsg
	{
	CString sAddr;
	CString sMsg;
	CDatum dPayload;
	};

class CProgramExecIPC
	{
	public:
		void Close ();
		bool Create (const CString &sID, DWORD dwMaxSize, CString *retsError = NULL);
		CString DebugBlockChain ();
		CString DequeueAsString ();
		bool DequeueCmd (CString &sCmd, CDatum &dData);
		bool Enqueue (const void *pPos, DWORD dwLength, CString *retsError = NULL);
		bool EnqueueCmd (const CString &sCmd, CDatum dData, CString *retsError = NULL);
		CManualEvent &GetEvent () { return m_HasNewData; }
		bool HasMore () const;
		bool Open (const CString &sID, CString *retsError = NULL);

		static void DebugTest ();

	private:
		static constexpr DWORD FLAG_BLOCK_FREE =		0x00000001;
		static constexpr DWORD FLAG_BLOCK_END =			0x00000002;

		struct SBlock
			{
			DWORD dwAllocSize = 0;			//	Size of block, including SBlock header
			DWORD dwLength = 0;				//	Size of content
			DWORD dwFlags = 0;				//	Flags for the block
			DWORD dwNext = 0;				//	Offset to next block (0 = none)
			DWORD dwPrev = NULL;			//	Offset to previous block (0 = none)
			};

		struct SHeader
			{
			DWORD dwMaxSize = 0;			//	Max size of the block
			DWORD dwFirst = 0;				//	Offset to first block (0 = none)
			DWORD dwLast = 0;				//	Offset to last block (0 = none)
			DWORD dwFirstFree = 0;			//	Offset to first free block.
			};

		void AddBlockToAllocChain (DWORD dwOffset);
		void AddBlockToFreeChain (DWORD dwBlock);
		SBlock *AllocBlock (DWORD dwContentSize, CString *retsError = NULL);
		void DequeueBlock (DWORD dwBlock);
		void FreeBlock (DWORD dwBlock);
		SBlock *GetBlock (DWORD dwOffset) { return (dwOffset == 0 ? NULL : (SBlock *)(((char *)m_pHeader) + dwOffset)); }
		void *GetBlockContent (DWORD dwBlock) { return &GetBlock(dwBlock)[1]; }
		void *GetBlockContent (SBlock *pBlock) { return &pBlock[1]; }
		CString GetBufferName () const { return strPattern("%s-Buffer", m_sID); }
		CString GetEventName () const { return strPattern("%s-HasNewData", m_sID); }
		DWORD GetNextAdjacentBlockIfFree (DWORD dwBlock);
		DWORD GetPrevAdjacentBlockIfFree (DWORD dwBlock);
		CString GetSemaphoreName () const { return strPattern("%s-Semaphore", m_sID); }
		void RemoveBlockFromAllocChain (DWORD dwBlock);
		void RemoveBlockFromFreeChain (DWORD dwBlock);

		static bool DebugTestDatum (CProgramExecIPC &IPC, CDatum dValue);

		CString m_sID;						//	Unique ID for the buffer
		CSharedMemoryBuffer m_Buffer;
		CSemaphore m_Lock;					//	Controls access to the shared buffer
		CManualEvent m_HasNewData;			//	Set when we have new data

		//	Valid when open

		SHeader *m_pHeader = NULL;
	};

#include "GridWhaleFS.h"
#include "GridWhaleCX.h"
#include "GridWhaleData.h"
#include "GridWhaleSearch.h"
#include "GridWhaleUser.h"
#include "GridWhalePermissions.h"
#include "GridWhaleLicensing.h"
#include "GridWhaleNotifications.h"

#include "AEON3DSObject.h"
#include "AEONGridConnection.h"
#include "ProgramDef.h"

class CGridUtil
	{
	public:

		static bool Boot ();
		static CDatum CreateMarkdown (CStringView sMarkdown);
		static constexpr DWORD FLAG_DISPOSABLE = 0x00000001;
		static CDatum CreateUserDataset (CGridName Username, CGridName OrgName, const CGridNameList& Groups, const CGridAccountList& Accounts, const CGridKeyCodeList& KeyCodes, const CGridUserPreferences& Preferences, const CGridUserProfile& Profile, const CGridGroupHierarchy& Hierarchy, const CDateTime& CreatedOn, const CDateTime& ModifiedOn, const CDateTime& LastLogin, DWORD dwFlags);
		static CDatum CreateUserDataset (const CGridUser& User);

		struct SUserListTableOptions
			{
			CDateTime MaxCreatedOn;				//	Maximum createdOn date
			CDateTime MinCreatedOn;				//	Minimum createdOn date
			CDateTime MaxLastLoginOn;			//	Maximum lastLogin date
			CDateTime MinLastLoginOn;			//	Minimum lastLogin date

			bool bIncludeActive = true;			//	TRUE if we include active users
			bool bIncludeInactive = false;		//	TRUE if we include inactive users
			bool bIncludeNonDisposable = true;	//	TRUE if we include non-disposable users
			bool bIncludeDisposable = false;	//	TRUE if we include disposable users
			};

		static CDatum CreateUserListTable (CDatum dUserList, const SUserListTableOptions& Options);
		static bool ParseUserListTableOptions (CDatum dOptions, SUserListTableOptions& retOptions, CString* retsError = NULL);

		static CString DecodeHexarcAddress (CStringView sValue);
		static CString DecodeHexarcMsg (CStringView sValue);
		static CString Desymbolize (const CString &sValue);
		static CString EncodeHexarcAddressAndMsg (CStringView sAddr, CStringView sMsg);
		static CString Symbolize (const CString &sValue);

		static DWORD ACL_TYPE;
		static DWORD A3DS_OBJECT_TYPE;
		static DWORD A3DS_RESOURCE_SCHEMA;
		static DWORD ACL_USER_PERMISSION_SCHEMA;
		static DWORD ACL_PROGRAM_PERMISSION_SCHEMA;
		static DWORD FOLDER_SCHEMA;
		static DWORD VERSION_SCHEMA;
		static DWORD GRID_COMPUTE_CORE_SCHEMA;
		static DWORD GRID_COMPUTE_NODE_SCHEMA;
		static DWORD GRID_CONNECTION_API_SCHEMA;
		static DWORD GRID_CONNECTION_LIST_SCHEMA;		//	Table of connections
		static DWORD GRID_CONNECTION_TYPE;
		static DWORD GRID_FILE_TYPE_SCHEMA;
		static DWORD GRID_GIT_COMMIT_SCHEMA;			//	Table of Git commits for a program repo
		static DWORD GRID_GIT_FILE_SCHEMA;				//	Table of Git-tracked files for a program repo
		static DWORD GRID_GIT_REPO_SCHEMA;				//	Table of Git repo bindings
		static DWORD GRID_GROUP_LIST_SCHEMA;			//	Table of groups
		static DWORD GRID_GROUP_MEMBER_SCHEMA;			//	Table of members in a group
		static DWORD GRID_IMAGE_SOURCE_TYPE;
		static DWORD GRID_KEY_CODE_SCHEMA;				//	Table of key codes owned by user
		static DWORD GRID_KEY_CODE_MANAGER_SCHEMA;		//	Table of key codes as viewed by offering manager
		static DWORD GRID_LICENSE_SCHEMA;
		static DWORD MARKDOWN_TYPE;
		static DWORD GRID_MEMBERSHIP_SCHEMA;			//	Table of groups a member belongs to
		static DWORD GRID_MODIFIED_TIMESTAMP_SCHEMA;	//	Table of ids and modifiedOn
		static DWORD GRID_NOTIFICATION_SCHEMA;			//	Table of notifications
		static DWORD GRID_NOTIFICATION_ICON_SCHEMA;		//	Table of coalesced notifications
		static DWORD GRID_OFFERING_SCHEMA;
		static DWORD GRID_PROCESS_LIST_SCHEMA;
		static DWORD GRID_PROGRAM_RUN_SCHEMA;
		static DWORD GRID_PROGRAM_USAGE_SCHEMA;
		static DWORD GRID_SEARCH_RESULTS_SCHEMA;
		static DWORD GRID_SEARCH_RESULTS_TYPE;
		static DWORD GRID_SEARCH_INDEX_STATUS_SCHEMA;
		static DWORD GRID_SECRET_METADATA_SCHEMA;		//	Table of Director-managed Arcology secret metadata
		static DWORD GRID_SPRITE_SHEET_TYPE;
		static DWORD GRID_STORED_TABLE_SCHEMA;			//	Table of physical stored tables
		static DWORD GRID_TRANSACTION_SCHEMA;
		static DWORD GRID_USER_CONFIG_SCHEMA;			//	GridUser config record
		static DWORD GRID_USER_LIST_SCHEMA;				//	Table of users
		static DWORD GRID_USER_PROGRAM_PREFERENCES_SCHEMA;	//	Schema for user program preferences

		static CDateTime GRID_FOUNDING;

	private:

		static bool m_bAEONRegistered;
	};
