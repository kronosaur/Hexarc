//	ProgramDef.h
//
//	GridWhale program definition
//	Copyright (c) 2020 GridWhale Corporation. All Rights Reserved.

#pragma once

class CProgramDef
	{
	public:
		static constexpr const char* PROGRAM_DESC_FILENAME = ".programDesc";
		static constexpr const char* PROGRAM_DESC_FORMAT = "gridwhale.programDesc";
		static constexpr DWORD PROGRAM_DESC_VERSION = 1;

		struct SSourceDesc
			{
			CString sName;
			CString sFilePath;
			CString sFolder;
			CString sType;
			CString sRepoPath;
			};

		CProgramDef () { }
		explicit CProgramDef (CDatum dDef) { InitFromDatum(dDef); }
		explicit CProgramDef (const IMemoryBlock &Data);

		void AddSourceFile (const CString &sFilePath, const CString &sName, const CString &sType, const CString &sFolder = NULL_STR);
		CDatum AsDatum (CString *retsError = NULL) const;
		CDatum AsRepoDescDatum () const;
		bool FindSourceByName (CStringView sName, SSourceDesc *retSource = NULL) const;
		bool FindSourceByRepoPath (CStringView sRepoPath, SSourceDesc *retSource = NULL) const;
		int FindSourceIndexByName (CStringView sName) const;
		int FindSourceIndexByRepoPath (CStringView sRepoPath) const;
		const CString &GetEnvironment () const { return m_sEnvironment; }
		const CString &GetName () const { return m_sProgramName; }
		int GetSourceCount () const { return m_Sources.GetCount(); }
		SSourceDesc GetSource (int iIndex) const;
		CGridID GetSourceDatasetID (int iIndex) const { return CGridID::ComposeFile(m_Sources[iIndex].sFilePath); }
		CDatum GetSourceDatum (int iIndex) const;
		const CString &GetSourceFilename (int iIndex) const { return m_Sources[iIndex].sName; }
		const CString &GetSourceFilePath (int iIndex) const { return m_Sources[iIndex].sFilePath; }
		CString GetSourceRepoPath (int iIndex) const;
		const CString &GetSourceType (int iIndex) const { return m_Sources[iIndex].sType; }
		const CString &GetUIType () const { return m_sUIType; }
		bool HasGridFSBindings () const;
		bool HasLogicalSources () const { return true; }
		void InitFromDatum (CDatum dDef);
		bool InitFromRepoDescDatum (CDatum dDef, CString *retsError = NULL);
		void MergeFilePathsFrom (const CProgramDef &Src);
		void SetEnvironment (const CString &sValue) { m_sEnvironment = sValue; }
		void SetName (const CString &sValue) { m_sProgramName = sValue; }
		void SetUIType (const CString &sValue) { m_sUIType = sValue; }
		CStringBuffer Serialize (CString *retsError = NULL) const;
		CStringBuffer SerializeRepoDescJSON () const;

		static bool DeserializeRepoDescJSON (const IMemoryBlock &Data, CProgramDef *retProgramDef, CString *retsError = NULL);
		static bool DeserializeRepoDescJSON (CStringView sJSON, CProgramDef *retProgramDef, CString *retsError = NULL);
		static CString ComposeProgramSourceName (CStringView sEnvironment, CStringView sRepoPath, CStringView sType);
		static CString ComposeRepoSourceName (CStringView sEnvironment, CStringView sName, CStringView sType);
		static bool IsGridLangSource (CStringView sEnvironment, CStringView sType);
		static bool IsProgramDescPath (CStringView sRepoPath);

	private:
		struct SSourceEntry
			{
			//	NOTE: For GridFS program definitions this can be either a fileKey
			//	(/Grid.files/ABCD1234) or a GridID (/file/ABCD1234). For repo
			//	.programDesc values this is intentionally empty.

			CString sFilePath;
			CString sName;
			CString sType;

			CString sFolder;		//	NULL = root; otherwise, filePath of container
			};

		const SSourceEntry *FindSourceEntryByName (const CString &sName) const;
		void RebuildIndex ();

		CString m_sProgramName;
		CString m_sEnvironment;
		CString m_sUIType;

		//	Sources indexed by filePath for GridFS program defs and by name for
		//	repo program descs.

		TArray<SSourceEntry> m_Sources;
		TSortMap<CString, int> m_ByName;
	};
