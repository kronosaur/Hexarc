//	HexeDocument.h
//
//	Hexe EXecution Environment
//	Copyright (c) 2011 GridWhale Corporation. All Rights Reserved.

#pragma once

class CHexeProcess;

//	CHexeDocument --------------------------------------------------------------

class CHexeDocument
	{
	public:
		struct SEntryPoint
			{
			CString sFunction;
			CDatum dDesc;
			CDatum dCode;
			};

		~CHexeDocument (void);

		static void CreateFunctionCall (const CString &sFunction, const TArray<CDatum> &Args, CDatum *retdEntryPoint);
		static bool IsValidIdentifier (const CString &sIdentifier);
		static bool ParseData (IByteStream &Stream, CDatum *retdData);
		static bool ParseLispExpression (const CString &sExpression, CDatum *retdExpression, CString *retsError);

		CDatum AsDatum (void) const;
		bool FindEntry (const CString &sName, CString *retsType = NULL, CDatum *retdData = NULL) const;
		int GetCount (void) const { return m_Doc.GetCount(); }
		CDatum GetData (int iIndex) const { return m_Doc[iIndex].dData; }
		void GetEntryPoints (TArray<SEntryPoint> *retList) const;
		void GetHexeDefinitions (TArray<CDatum> *retList) const;
		const CString &GetName (int iIndex) const { return m_Doc[iIndex].sName; }
		const CString &GetType (int iIndex) const { return m_Doc[iIndex].sType; }
		int GetTypeIndexCount (int iTypeIndex) const { return (iTypeIndex == -1 ? 0 : m_TypeIndex[iTypeIndex].GetCount()); }
		CDatum GetTypeIndexData (int iTypeIndex, int iIndex) const { return (iTypeIndex == -1 ? CDatum() : m_Doc[m_TypeIndex[iTypeIndex].GetAt(iIndex)].dData); }
		const CString &GetTypeIndexName (int iTypeIndex, int iIndex) const { return (iTypeIndex == -1 ? NULL_STR : m_Doc[m_TypeIndex[iTypeIndex].GetAt(iIndex)].sName); }
		int GetTypeIndex (const CString &sType) const;
		bool InitFromData (CDatum dData, CHexeProcess &Process, CString *retsError);
		bool InitFromStream (IByteStream &Stream, CHexeProcess &Process, CString *retsError);
		void Mark (void);
		void Merge (CHexeDocument *pDoc);

	private:
		struct SEntry
			{
			CString sName;
			CString sType;
			CDatum dData;
			};

		bool AddEntry (const CString &sName, const CString &sType, CDatum dData);
		void InitTypeIndex (void) const;
		void InvalidateTypeIndex (void) { m_TypeIndex.DeleteAll(); }
		bool ParseAEONDef (CCharStream *pStream, CHexeProcess &Process, CString *retsName, CString *retsType, CDatum *retdDatum, CString *retsError);
		bool ParseComments (CCharStream *pStream, CString *retsError);
		bool ParseDefine (CAEONScriptParser &Parser, CHexeProcess &Process, const CString &sType, CString *retsName, CDatum *retdDatum, CString *retsError);

		static bool IsAnonymous (const CString &sName, const CString &sType);
		static bool ParseHexeLispDef (CCharStream *pStream, CString *retsName, CString *retsType, CDatum *retdDatum, CString *retsError);

		TSortMap<CString, SEntry> m_Doc;
		mutable TSortMap<CString, TArray<int>> m_TypeIndex;
	};
