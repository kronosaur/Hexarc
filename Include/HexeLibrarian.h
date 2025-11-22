//	HexeLibrarian.h
//
//	Hexe EXecution Environment
//	Copyright (c) 2011 GridWhale Corporation. All Rights Reserved.

#pragma once

//	CHexeLibrarian -------------------------------------------------------------

typedef bool (* FHexeLibraryFunc)(IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retdResult);

struct SLibraryFuncDef
	{
	const CString &sName;
	FHexeLibraryFunc pfFunc;
	DWORD dwData;
	const CString &sArgList;
	const CString &sHelpLine;
	DWORD dwExecFlags;
	};

#define DECLARE_DEF_LIBRARY_FUNC(name,func,rights)	{ name##_NAME, func, name, name##_ARGS, name##_HELP, (rights) }

class CHexeLibrarian
	{
	public:

		struct SFunctionDef
			{
			LPCSTR pName = "";
			LPCSTR pArgList = "";
			LPCSTR pHelp = "";

			FHexeLibraryFunc pfFunc = NULL;
			DWORD dwData = 0;
			DWORD dwExecFlags = 0;
			};

		struct SConstDoubleDef
			{
			LPCSTR pName = "";
			LPCSTR pHelp = "";
			double rValue = 0.0;
			};

		CDatum FindFunction (CStringView sLibrary, CStringView sFunction) const;
		bool FindLibrary (CStringView sName, DWORD* retdwLibraryID) const;
		CStringView GetEntry (DWORD dwLibrary, int iIndex, CDatum *retdFunction = NULL);
		int GetEntryCount (DWORD dwLibrary);
		int GetLibraryCount (void) { return m_Catalog.GetCount(); }
		void Mark (void);
		void RegisterCoreLibraries (void);
		void RegisterLibrary (CStringView sName, int iCount, const SLibraryFuncDef* pNewLibrary);

		static CDatum CreateFunction (const SFunctionDef& Def);

	private:

		struct SLibrary
			{
			CString sName;
			TSortMap<CString, int> Functions;
			};

		const SLibrary* GetLibrary (CStringView sName) const;
		SLibrary* GetLibrary (CStringView sName);

		TSortMap<CString, SLibrary> m_Catalog;
		TArray<CDatum> m_Functions;
	};

extern CHexeLibrarian g_HexeLibrarian;

