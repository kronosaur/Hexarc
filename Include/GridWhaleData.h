//	GridWhaleData.h
//
//	GridWhale Classes and Functions
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#pragma once

#ifdef DEBUG
//#define DEBUG_PROPERTIES
//#define DEBUG_DELETE_FILE
//#define DEBUG_UPLOAD_FILE
#endif

class CGridName;
class CGridUser;
class CProgramRunInfo;

//	GridID
//
//	A GridID is a unique identifier to a resource ID. A GridID is sufficient to
//	locate the resource at the appropriate machine and module.

class CGridID
	{
	public:
		enum class EType
			{
			Unknown,
			Error,

			File,						//	/file/ABCD1234
			NewFile,					//	/newfile/
			Memory,						//	/memo/ABCD1234:XYZ
			Name,						//	/name/george moromisato
										//	/name/*
										//	/name/{george moromisato}/*
										//	/name/{george moromisato}/{my group}
			Node,						//	/node/mazian09A1
			Path,						//	/path/~/foo
			Instance,					//	/proc/GHIJ7890/00001234

			FileTypes,					//	/grid/fileTypes
			Grid,						//	/grid/cores (or some other built-in Grid dataset).
										//		LATER: Other /grid/... datasets should move
										//		to this type.
			GridFolder,					//	/grid/gridFolder
			GridSettings,				//	/grid/gridSettings
			GridStatus,					//	/grid/gridStatus
			RootFolder,					//	/grid/rootFolder
			UserFolder,					//	/grid/userFolder

			KeyCodes,					//	/keycodes/gridwhale.offering123
										//	/keycodes/{george moromisato}/offering123

			UserProcesses,				//	/uproc/george moromisato
			UserProgramData,			//	/udata/george moromisato
			UserPrograms,				//	/uprog/george moromisato
			UserRoot,					//	/uroot/george moromisato
			UserTransactions,			//	/ubill/george moromisato
			UserNotify,					//	/unotify/george moromisato
			UserNotifyIcon,				//	/unotifyicon/george moromisato
			};

		enum class ELocation
			{
			Unknown,
			Error,

			Memory,
			
			ProgramDataFolder,
			UserDataFolder,

			AeonDB,
			PostgreSQL,
			};

		enum class EFileEncoding
			{
			None,							//	/file/ABCD1234
			Error,

			ProgramData,					//	/file/PROGRMID_XYZ.dat
			ProgramDataDir,					//	/file/PROGRMID.dir
			UserData,						//	/file/USERNAME~PROGRMID_XYZ.dat
			UserDataDir,					//	/file/USERNAME~PROGRMID.dir
			UserDir,						//	/file/USERNAME~HOME.dir
			};

		enum class EUserFolder
			{
			Unknown				= -1,

			AgentStudio			= 0,
			Fonts				= 1,
			Home				= 2,
			Images				= 3,
			Libraries			= 4,
			Limbo				= 5,
			Shelf				= 6,
			Start				= 7,

			Count				= 8,
			};

		struct SFileRef
			{
			CString sFileRef;				//	File ref after /file/ (currently same as local file token).
			CString sLocalFileToken;			//	File token with no enclave information.
			CString sEnclaveID;				//	Reserved for future enclave support.
			};

		CGridID () { }
		explicit CGridID (CDatum dValue) : m_sGridID((CStringView)dValue) { }
		explicit CGridID (CStringView Src) : m_sGridID(Src) { }
		explicit CGridID (const CString& Src) : m_sGridID(Src) { }
		explicit CGridID (CString&& Src) noexcept : m_sGridID(Src) { }

		bool operator== (const CGridID& Src) const { return strEquals(m_sGridID, Src.m_sGridID); }
		bool operator!= (const CGridID& Src) const { return !(*this == Src); };

		bool operator== (const CString& Src) const { return strEquals(m_sGridID, Src); }
		bool operator!= (const CString& Src) const { return !(*this == Src); };

		explicit operator bool () const { return !IsEmpty(); }

		CDatum AsDatum () const { return CDatum(m_sGridID); }
		const CString& AsString () const { return m_sGridID; }
		CString GetError () const;
		CString GetFileID () const;
		CString GetFileKey () const;
		CString GetInstanceID () const;
		CGridName GetName () const;
		CString GetOfferingID () const;
		bool GetProgramData (CString *retsProgramID = NULL, CString *retsSymbol = NULL) const;
		CString GetProgramIDFromFileID () const;
		CString GetUsernameID () const;
		EType GetType () const { return GetType(m_sGridID); }
		bool IsEmpty () const { return m_sGridID.IsEmpty(); }
		bool IsError (CString *retsError = NULL) const;
		bool IsFileID (CString *retsFileKey = NULL) const;
		bool IsPathID (CString *retsFilePath = NULL) const;
		bool IsValid () const { return IsValid(m_sGridID); }
		EFileEncoding ParseFileID (CString* retsProgramID = NULL, CString* retsUsernameID = NULL, CString* retsSymbol = NULL) const { return ParseFileID(m_sGridID, retsProgramID, retsUsernameID, retsSymbol); }
		static ELocation ParseLocation (const CString& sID);
		bool ParseLocationAndName (CGridID* retLocationID = NULL, CString* retsName = NULL) const;

		static CGridID ComposeFile (const CString& sFileKey);
		static CGridID ComposeFile (const CString& sProgramID, const CString& sSymbolName);
		static CGridID ComposeFile (const CString& sProgramID, const CString& sUsernameID, const CString& sSymbolName);
		static CGridID ComposeFileOrPath (CStringView sFileID);
		static CGridID ComposeNewFile ();
		static CGridID ComposeResolvableFile (CStringView sValue);

		static CGridID ComposeFolder (const CString& sProgramID);
		static CGridID ComposeFolder (const CString& sProgramID, const CString& sUsernameID);

		static CGridID ComposeInstance (const CString& sInstanceID);

		static CGridID ComposeMemory (const CString& sProgramID, const CString& sSymbolName);
		static CGridID ComposeMemory (const CString& sProgramID, const CString& sUsernameID, const CString& sSymbolName);

		static CGridID ComposeError (const CString &sError);
		static CGridID ComposeFilePath (const CString& sProgramID, const CString& sUsernameID, const CString& sValue);
		static CGridID ComposeGroupList (const CString& sUsernameID);
		static CGridID ComposeName (const CGridName& Name);
		static CGridID ComposeSpecial (const CString& sGridID);
		static CGridID ComposeUser (const CString& sUsernameID);
		static CGridID ComposeUserFolder (EType iType, const CString& sUsernameID);
		static CGridID ComposeUserFolder (EUserFolder iFolder, CStringView sUsernameID);
		static CGridID ComposeUserNamespace (const CString& sUsernameID, const CString& sPath);

		static EUserFolder FindUserFolderID (CStringView sID);
		static void InitFolderDesc ();
		static bool IsUserFolderPath (CStringView sPath);
		static bool IsValid (const CString& sGridID);
		static CString GetID (EFileEncoding iEncoding);
		static CString GetID (ELocation iLocation);
		static EType GetType (const CString& sGridID);
		static CGridID GetUserFolderIcon (EUserFolder iFolder);
		static CString GetUserFolderID (EUserFolder iFolder);
		static CString GetUserFolderName (EUserFolder iFolder);
		static bool ParseFileGridID (CStringView sGridID, SFileRef* retFileRef = NULL);
		static bool ParseFileGridIDOrStoragePath (CStringView sValue, SFileRef* retFileRef = NULL);
		static EFileEncoding ParseFileID (const CString& sGridID, CString* retsProgramID = NULL, CString* retsUsernameID = NULL, CString* retsSymbol = NULL);
		static bool ParseFileRef (CStringView sFileRef, SFileRef* retFileRef = NULL);
		static bool ParseFileStoragePath (CStringView sStoragePath, SFileRef* retFileRef = NULL);

	private:

		struct SFolderDesc
			{
			EUserFolder iFolder = EUserFolder::Unknown;
			CString sFolderID;
			CString sFolderName;
			CString sFolderIcon;
			};

		static constexpr int RANDOM_SYMBOL_SIZE = 20;

		CString m_sGridID;

		static TArray<SFolderDesc> m_FOLDER_DESC;
	};

inline int KeyCompare (const CGridID& pKey1, const CGridID& pKey2) { return ::KeyCompare(pKey1.AsString(), pKey2.AsString()); }

class CGridDataset
	{
	public:
		struct SProgramURLOptions
			{
			CString sDomain;
			bool bRelative = false;
			bool bSSR = false;
			bool bViewSwitch = false;
			CString sFragment;
			};

		struct SProgramURL
			{
			CString sCanonicalURL;
			CString sProgramID;
			CString sFileID;
			CString sSession;
			CString sView;
			bool bViewSwitch = false;
			};

		static CDatum ComposeAddInvitationKeyMutationValue (const CGridID& DatasetID, const CGridName& UserToChange, const CGridName& InvitedBy);
		static CDatum ComposeDatasetPropertiesError (CStringView sError);
		static CDatum ComposeDatasetPropertiesFromFileDesc (CGridID DatasetID, CDatum dFileDesc);
		static CString ComposeDownloadURL (const CGridID& DatasetID, CStringView sFormat = NULL_STR);
		static CString ComposeResourceURL (const CGridID& DatasetID);
		static CDatum ComposeInviteMutationValue (const CGridID& DatasetID, const CGridName& Invitee, const CGridName& InvitedBy, const CEmailAddress& Email = CEmailAddress(), const CString& sDisplayName = NULL_STR);
		static CString ComposeInviteURL (const CString& sKey, const CString& sDomain = NULL_STR);
		static CString ComposeProgramURL (const CString& sProgramID, CDatum dParams, const SProgramURLOptions& Options = SProgramURLOptions());
		static bool ComposeProgramURL (const CString& sProgramID, CDatum dParams, const SProgramURLOptions& Options, CString* retsURL, CString* retsError = NULL);
		static CDatum ComposeSetUserStatusMutationValue (const CGridID& DatasetID, const CGridName& UserToChange, const CString& sNewStatus);
		static bool ParseProgramURL (CStringView sURL, SProgramURL* retURL, CString* retsError = NULL);
	};

class CGridDataExport
	{
	public:
		enum class EConvertResult
			{
			OK,
			Unsupported,
			Error,
			};

		struct SOptions
			{
			TArray<CString> StyleSheets;
			};

		static CString MapFileTypeToExportType (const CString& sMediaType);
		static EConvertResult ToMediaType (CDatum dValue, const CString& sSourceMediaType, const CString& sDestMediaType, CDatum& retdData, const SOptions& Options = SOptions());

		static CStringBuffer ToHTML (CDatum dValue);
		static CStringBuffer ToHTMLDiv (CDatum dValue);
		static CStringBuffer MarkdownToHTML (CDatum dValue, const SOptions& Options = SOptions());
		static CStringBuffer MarkdownToHTMLDiv (CDatum dValue, const SOptions& Options = SOptions());

		static CStringBuffer ToCSV (CDatum dValue);
		static CStringBuffer ToBMP (CDatum dValue);
		static CStringBuffer ToJPEG (CDatum dValue);
		static CStringBuffer ToJSON (CDatum dValue);
		static CStringBuffer ToPNG (CDatum dValue);
		static CStringBuffer ToText (CDatum dValue);
		static CStringBuffer ToZip (CDatum dValue);

	private:

		static CStringBuffer ArrayToCSV (CDatum dValue);
		static CStringBuffer TableToCSV (CDatum dValue);
		static void WriteMarkdown (IByteStream& Stream, const CString& sValue, const SOptions& Options);
		static void WriteHTMLBodyStart (IByteStream& Stream);
		static void WriteHTMLBodyEnd (IByteStream& Stream);
	};

class CGridOData
	{
	public:

		static CDatum AsEntitySetJSON (CDatum dValue, CStringView sRootURL, CStringView sName, CStringView sUpdate);
		static CDatum AsEntitySetXML (CDatum dValue, CStringView sRootURL, CStringView sName, CStringView sUpdate);
		static CDatum AsMetadata (CDatum dValue);
		static CDatum AsServiceDoc (CDatum dValue, CStringView sRootURL);

		static CString GetEDMType (const IDatatype& Type);

	private:

		static CDatum GetEntityFieldValueAsJSON (CDatum dValue);
		static void WriteEntityFieldValueAsXMLText (IByteStream& Output, CDatum dValue);
	};

class CGridDBDesc
	{
	public:

		enum class EClass
			{
			Unknown,
			Table,								//	A single table
			Database,							//	A database with multiple tables
			};

		struct STableDesc
			{
			CString sID;
			CDatum dSchema;
			};

		CGridDBDesc () { }
		CGridDBDesc (CDatum dValue);

		static CDatum Create (CGridID::ELocation iLocation, CStringView sProgramID, CStringView sFileID, CDatum dSchema, bool bDebugSim = false);
		static CDatum CreateAeonDBRef (CStringView sAeonTable, CStringView sAeonSubTable);
		static CDatum CreateAeonDBStore (CGridID::ELocation iLocation, CStringView sProgramID, CStringView sFileID, bool bDebugSim = false);
		static CDatum MakeTableList (const TArray<STableDesc>& Tables);

		CDatum AsDatum () const;
		int FindAeonSubTable (CStringView sTable) const;
		CDatum GetAeonRowsMsg (int iIndex, CDatum dOptions) const;
		CString GetAeonSubTable (int iIndex) const;
		int GetAeonSubTableCount () const { return m_Tables.GetCount(); }
		CString GetAeonTable () const;
		EClass GetClass () const { return m_iClass; }
		CGridID::ELocation GetLocation () const { return m_iLocation; }
		CString GetPostgreSQLSchema () const;
		CString GetPostgreSQLTable () const;
		CDatum GetSchema (int iIndex) const { if (iIndex >= 0 && iIndex < m_Tables.GetCount()) return m_Tables[iIndex].dSchema; else return CDatum(); }
		bool IsAeonDedicatedTable () const;
		bool IsValid () const { return m_iLocation != CGridID::ELocation::Unknown && m_iClass != EClass::Unknown; }
		void Mark ();

	private:

		static CString GenerateIDFromProgramID (CStringView sProgramID);
		static CString GenerateUniqueAeonID ();
		static CString GetID (EClass iClass);
		static EClass ParseClass (CStringView sValue);

		CGridID::ELocation m_iLocation = CGridID::ELocation::Unknown;
		EClass m_iClass = EClass::Unknown;
		CString m_sProgramID;					//	Program ID that created the table.
		CString m_sFileID;						//	File ID where we store table descriptor.

		TArray<STableDesc> m_Tables;			//	Sub-tables in database. NOTE: In general we store multiple tables
												//	in a single AeonDB table.

		CString m_sExtensionMutateMsg;			//	Message to send to extension on mutation (msg@addr)

		//	Aeon-specific

		CString m_sAeonTable;					//	Grid.tables (or other Aeon table).
	};

class CGridImageSource
	{
	public:

		CGridImageSource () { }
		explicit CGridImageSource (CDatum dSrc);

		static CDatum AsView (CDatum dValue);
		static CGridImageSource CreateCropped (CDatum dSource, int xCrop, int yCrop, int cxCrop, int cyCrop);
		static CDatum CreateSpriteSheet (CDatum dSource, int cxCell, int cyCell, CDatum dOptions);
		static const CGridImageSource* GetGridImageSource (CDatum dValue);

		bool operator== (const CGridImageSource& Src) const { return IsEqual(Src); }
		bool operator!= (const CGridImageSource& Src) const { return !IsEqual(Src); }

		CDatum AsDatum () const;
		CDatum AsView () const;
		CGridImageSource Crop (int xCrop, int yCrop, int cxCrop, int cyCrop) const;
		static CGridImageSource Deserialize (IByteStream& Stream);
		int GetCropX () const { return m_xCrop; }
		int GetCropY () const { return m_yCrop; }
		int GetCropWidth () const { return m_cxCrop; }
		int GetCropHeight () const { return m_cyCrop; }
		CStringView GetSourcePath () const { return m_sSourcePath; }
		bool IsCropped () const { return (!IsEmpty() && m_cxCrop > 0 && m_cyCrop > 0); }
		bool IsEmpty () const { return m_sSourcePath.IsEmpty(); }
		void Serialize (IByteStream& Stream) const;

	private:

		void InitOptions (CDatum dOptions);
		bool IsEqual (const CGridImageSource& Src) const;

		CString m_sSourcePath;					//	GridID or URL

		int m_xCrop = 0;						//	X offset of crop area
		int m_yCrop = 0;						//	Y offset of crop area
		int m_cxCrop = 0;						//	Width of crop area (0 = no crop)
		int m_cyCrop = 0;						//	Height of crop area
	};
