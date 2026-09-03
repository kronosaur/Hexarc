//	GridWhaleFS.h
//
//	GridWhale Classes and Functions
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "GridWhalePermissions.h"
#include "OLEDBUtil.h"

class CProgramRunInfo;

enum class EFileAccessType
	{
	unknown,

	read,
	contribute,
	write,
	own
	};

class CFilePermissions
	{
	public:
		CFilePermissions () { }

		CString AsString () const;
		bool CanAccess () const { return (m_dwFlags & (READ_ACCESS | CONTRIBUTE_ACCESS | OWNER_ACCESS | WRITE_ACCESS)); }
		bool CanContribute () const { return (m_dwFlags & CONTRIBUTE_ACCESS); }
		bool CanOwn () const { return (m_dwFlags & OWNER_ACCESS); }
		bool CanRead () const { return (m_dwFlags & READ_ACCESS); }
		bool CanWrite () const { return (m_dwFlags & WRITE_ACCESS); }

		bool CheckParent () const { return (m_dwFlags & CHECK_PARENT); }

		bool HasAccess (EFileAccessType iAccess) const;
		bool InitFromString (const CString &sValue);

		static EGridPermission AsPermission (EFileAccessType iAccess);

		static CFilePermissions AccessRead () { return CFilePermissions(READ_ACCESS); }
		static CFilePermissions AccessCheckParent () { return CFilePermissions(CHECK_PARENT); }
		static CFilePermissions AccessContribute () { return CFilePermissions(READ_ACCESS | CONTRIBUTE_ACCESS); }
		static CFilePermissions AccessNone () { return CFilePermissions(0); }
		static CFilePermissions AccessOwner () { return CFilePermissions(READ_ACCESS | CONTRIBUTE_ACCESS | OWNER_ACCESS | WRITE_ACCESS); }
		static CFilePermissions AccessWrite () { return CFilePermissions(READ_ACCESS | CONTRIBUTE_ACCESS | WRITE_ACCESS); }

		static EFileAccessType ParseAccess (const CString &sValue);

	private:
		CFilePermissions (DWORD dwFlags) : m_dwFlags(dwFlags) { }

		static constexpr DWORD READ_ACCESS =		0x00000001;
		static constexpr DWORD CONTRIBUTE_ACCESS =	0x00000002;
		static constexpr DWORD WRITE_ACCESS =		0x00000004;
		static constexpr DWORD OWNER_ACCESS =		0x00000008;

		static constexpr DWORD CHECK_PARENT =		0x80000000;

		DWORD m_dwFlags = 0;
	};

class CGridFSPath
	{
	public:
		enum class EOp
			{
			None,
			FindByName,					//	sFilePath is folderID; sData is filename
			GetUserHomeFolder,			//	sData is username
			};

		struct SOperation
			{
			EOp iOp;
			CString sFilePath;
			CString sData;
			};

		enum class EResult
			{
			OK,							//	Path has been resolved to a fileID
			Error,						//	Error resolving path
			Operation,					//	Operation required to resolve
			};

		CGridFSPath () { }
		CGridFSPath (const CString& sFilePath, const CProgramRunInfo& ProgramInfo);
		CGridFSPath (const CString& sFilePath, const CString &sUsername, const CString &sProgramID);

		const CString &GetFileID () const { return m_sFilePath; }
		bool GetNextOp (SOperation &retOp) const;
		bool GetNextOpMsg (CString& retsMsg, CDatum& retdPayload) const;
		const SOperation& GetOp (int iIndex) const { if (iIndex >= 0 && iIndex < m_Ops.GetCount()) return m_Ops[iIndex]; else throw CException(errFail); }
		CDatum GetOpAsDatum (int iIndex) const;
		int GetOpCount () const { return m_Ops.GetCount(); }
		EResult GetStatus (CString* retsFileID = NULL) const;
		bool Init (const CString& sFilePath, const CString &sUsername, const CString &sProgramID, CString *retsError = NULL);
		EResult SetOpResult (const CString& sMsg, CDatum dPayload);

		static CString FormatFileID (const CString &sValue);

	private:

		void InitDirectory (const CString& sDirectoryPath, const TArray<CString>& Component, int iPos);
		bool SetError (const CString &sError, CString *retsError = NULL) { m_sFilePath = sError; m_bError = true; if (retsError) *retsError = sError; return false; }
		static TArray<CString> SplitPath (CStringView sFilePath);

		TArray<SOperation> m_Ops;
		int m_iCurOp = -1;

		bool m_bError = false;
		CString m_sFilePath;
	};

class CCSVFormat
	{
	public:
		
		static constexpr int PROGRESS_GRANULARITY = 1000;

		struct SOptions
			{
			CAEONTypeSystem *pTypeSystem = NULL;

			ECharSetType iCharSet = ECharSetType::Unknown;	//	Auto-detect UTF-8 or Windows-1252
			bool bAllowShortRows = false;
			char chDelimiter = '\0';		//	Detect delimiter if '\0'

			//	First value is percent complete; second value is rows read.
			std::function<void(int, int)> fnOnProgress = NULL;
			};

		static char DetectDelimiter (IByteStream64& Stream);
		static bool LoadAsTable (IByteStream64& Stream, const SOptions& Options, CDatum &retdResult);
		static bool Save (CDatum dTable, IByteStream64& Stream, CString* retsError = NULL);
		static bool Save (CDatum dTable, IByteStream& Stream, CString* retsError = NULL);

	private:

		static constexpr DWORDLONG AVERAGE_ROW_SIZE = 200;

		static CDatum CreateSchemaFromHeaders (const TArray<CString>& Headers, const SOptions& Options);
		static constexpr int EstimateRowCount (DWORDLONG dwStreamSize) { return (int)(dwStreamSize / AVERAGE_ROW_SIZE); }
		static void WriteValue (IByteStream64& Stream, const CString& sValue);
		static void WriteValue (IByteStream& Stream, const CString& sValue);
	};

enum class EFileClass
	{
	Unknown					= -1,

	UserFile				= 0,	//	A file in the ~/home directory
	ProgramFile				= 1,	//	A file in program global data directory
	ProgramUserFile			= 2,	//	A file in the per-user program directory

	Count					= 3,
	};

enum class EFileContentType
	{
	None,

	Binary,									//	Binary or unknown format.
	BMP,									//	Windows BMP
	CSV,									//	CSV text file.
	GIF,									//	GIF.
	HexeData,								//	AEONScript in UTF-8 text.
	JPEG,									//	JPEG image.
	Markdown,								//	Git-Flavored Markdown.
	PNG,									//	PNG image.
	SASDataset,								//	SAS dataset.
	Text,									//	UTF-8 text.
	XLSX,									//	Excel XLSX.
	XML,									//	XML.
	Zip,									//	Zip archive
	ThreeDS,								//	3D Studio model.
	};

class CGridFS
	{
	public:
		struct SFolderSchema
			{
			int FIELD_ID = 0;
			int FIELD_NAME = 0;
			int FIELD_ICON = 0;
			int FIELD_OWNER = 0;
			int FIELD_TYPE = 0;
			int FIELD_SIZE = 0;
			int FIELD_CREATED_BY = 0;
			int FIELD_CREATED_ON = 0;
			int FIELD_MODIFIED_BY = 0;
			int FIELD_MODIFIED_ON = 0;
			int FIELD_ACL = 0;
			int FIELD_DESC = 0;
			int FIELD_TAGS = 0;
			int FIELD_ACTIONS = 0;
			int FIELD_PURPOSE = 0;
			int FIELD_INFO = 0;
			};

		static CDatum ConvertToType (CDatum dValue, EFileContentType iType);
		static CDatum ComposeUpdateTablePlan (CDatum dDBDesc, CDatum dLog, CDatum dOptions = CDatum());
		static CDatum CreateTableFromFileList (CDatum dFileList);
		static CString DetectMediaType (CDatum dFileData);
		static CString DetectMediaType (const IMemoryBlock64& Data);
		static CString DetectMediaTypeOfStream (IByteStream& Data);
		static CString DetectTextFileType (const char* pBuffer, const char* pBufferEnd, bool bPartial);
		static EFileContentType GetContentType (const CString &sMediaType);
		static CString GetAeonIDFromFileKey (CStringView sFileKey);
		static CString GetFileIconFromGridID (const CGridID& IconID);
		static CString GetFileIDFromFileKey (const CString& sFileKey);
		static CString GetFileKeyFromGridID (const CString& sGridID);
		static CString GetFileKeyFromPrimaryKey (CStringView sPrimaryKey);
		static CDatum GetFolderRowID (CDatum dRow);
		static SFolderSchema GetFolderSchema (CDatum dSchema);
		static CGridName GetOwnerFromFileDesc (CDatum dFileDesc);
		static bool IsFilePath (const CString &sValue);
		static bool LoadFileData (CDatum dFileData, EFileContentType iType, CDatum& retdData);
		static CString MakeFileID (const CString &sProgramID);
		static CGridID MakeGridIDFromIcon (CStringView sIcon);
		static CString MakeGridIDFromFileKey (const CString &sFileKey);
		static CString MakeProgramDataFolderID (const CString &sProgramID);
		static CString MakeUserDataFolderID (const CString& sProgramID, const CString& sUserID);
		static CDatum SaveFileData (CDatum dData, EFileContentType iType);
		static bool SetFolderFromFileDesc (IAEONTable& Table, CDatum dFileDesc, int* retiRow = NULL);
		static bool ValidateFileKey (const CString& sFileKey, CString* retsCanonical = NULL);

	private:

		struct SMediaTypeInfo
			{
			CString sMediaType;
			CString sName;
			EFileContentType iType;
			};

		static CString DetectStdHeaders (const IMemoryBlock64& Data, bool bPartial = false);
		static CString DetectZipFile (const IMemoryBlock64& Data);
		static void SetRow (IAEONTable& Table, int iDestRow, const SFolderSchema& Schema, CDatum dEntry);

		static COLEDB m_OLEDB;
		static TSortMap<CString, SMediaTypeInfo> MEDIA_TYPE_TABLE;
		static constexpr int SAS_HEADER_SIZE = 32;
		static BYTE SAS_HEADER[SAS_HEADER_SIZE];
	};

class CGridSchemaFileTypes
	{
	public:
		static CGridSchemaFileTypes Create (CDatum dSchema);

		int GetField_ID () const { return m_FIELD_ID; }
		void SetRowFromGridTypesEntry (IAEONTable& Table, int iDestRow, CDatum dEntry);

	private:

		static CDatum ComposeExec (CDatum dValue);

		int m_FIELD_ID = 0;					//	ID (media type)
		int m_FIELD_NAME = 0;				//	Human-readable name
		int m_FIELD_OWNER = 0;				//	GridName of owner
		int m_FIELD_ICON = 0;				//	GridID of default icon
		int m_FIELD_EDIT_PROGRAM = 0;		//	GridID of default edit program
		int m_FIELD_VIEW_PROGRAM = 0;		//	GridID of default view program
		int m_FIELD_CREATED_ON = 0;
		int m_FIELD_CREATED_BY = 0;			//	GridName of creator
		int m_FIELD_MODIFIED_ON = 0;
		int m_FIELD_MODIFIED_BY = 0;		//	GridName of last modifier
	};

class CSASFileImporter
	{
	public:

		CDatum ImportSASFile (COLEDB& OLEDB, CDatum dFileData);

	private:

		CDatum ImportXPTFile (const CString& sFilespec);
		CString WriteToTempFile (CDatum dFileData);
	};

class CXMLFileImporter
	{
	public:

		CDatum ImportXMLFile (CDatum dFileData);

	private:
	};
