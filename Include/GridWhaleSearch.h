//	GridWhaleSearch.h
//
//	GridWhale Classes and Functions
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "Foundation.h"
#include "GridWhaleData.h"

struct SGridSearchFileEntry
	{
	CGridID FileID;					//	ID of the file
	CString sName;					//	Name of the file
	CGridID IconID;					//	Icon of the file
	CString sFileType;				//	File type
	CDateTime ModifiedOn;			//	The last time the file was modified
	};

struct SGridSearchIndexDefinition
	{
	CString sIndexType;
	CString sBackend;
	CString sTokenizer;
	int iRemoveDiacritics = 2;
	CString sSourceErrorPolicy;
	CDatum dNormalized;
	};

class CGridSearchIndex
	{
	public:
		static CDatum CreateTextDefinition ();
		static bool InitDefinition (CDatum dValue, SGridSearchIndexDefinition& retDefinition, CString* retsError = NULL);
	};

class CGridSearchResults
	{
	public:

		enum class EType
			{
			Unknown,

			Result,
			Error,
			FatalError,
			};

		struct SEntry
			{
			EType iType = EType::Unknown;	//	Type of entry
			int iFileIndex = -1;			//	Index into the file array
			int iRelevance = 0;				//	An integer value representing relevance, higher is better
			CString sField;					//	The field in the dataset
			int iPosition = 0;				//	The position in the result (for text, a line index, 1-based)
			int iCharacter = 0;				//	The character offset within the position (1-based)
			int iLength = 0;				//	The length of the word (in bytes)
			CString sExtract;				//	An extract of the document.
			};

		CGridSearchResults ();
		static CGridSearchResults CreateError (const SGridSearchFileEntry& FileEntry, CStringView sError);
		static CGridSearchResults Deserialize (IByteStream& Stream);

		CDatum AsGridSearchResultsType () const;
		CString AsString () const;
		CDatum AsTable () const;
		void AddEntry (const SGridSearchFileEntry& FileEntry, const SEntry& Entry);
		void AddLineEntry (const SGridSearchFileEntry& FileEntry, int iRelevance, int iLine, int iChar, int iLength, CStringView sExtract);
		int Compare (const CGridSearchResults& Src) const;
		bool IsFatalError (CString* retsError = NULL) const;
		void Serialize (IByteStream& Stream) const;

	private:

		int AddFileEntry (const SGridSearchFileEntry& FileEntry);
		const SGridSearchFileEntry& GetFileEntry (int iIndex) const { return m_Files[iIndex]; }

		int m_iFilesSearched = 0;
		int m_iFoldersSearched = 0;
		TArray<SGridSearchFileEntry> m_Files;		//	Files that we found
		TSortMap<CGridID, int> m_FileIndex;			//	Index of files by GridID
		TSortMap<int, SEntry> m_Results;			//	Results (sorted by relevance)
	};

class CGridSearch
	{
	public:

		struct SOptions
			{
			};

		static bool CanSearchType (CStringView sType);
		static CGridSearchResults Search (CStringView sQuery, const SGridSearchFileEntry& FileEntry, CDatum dData, const SOptions& Options);

	private:

		static TArray<int> ComputeLPS (CStringView sPattern);
		static CGridSearchResults SearchString (CStringView sQuery, const SGridSearchFileEntry& FileEntry, CDatum dData, const SOptions& Options);
		static CGridSearchResults SearchTextLines (CStringView sQuery, const SGridSearchFileEntry& FileEntry, CDatum dData, const SOptions& Options);
	};
