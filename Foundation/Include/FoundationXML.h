//	FoundationXML.h
//
//	Foundation header file
//	Copyright (c) 2014 by GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class CExternalEntityTable;
class CXMLElement;

class IXMLParserController
	{
	public:
		virtual ~IXMLParserController (void) { }

		virtual bool OnOpenTag (CXMLElement *pElement, CString *retsError) { return true; }
		virtual CString ResolveExternalEntity (const CString &sName, bool *retbFound = NULL) = 0;
	};

struct SXMLNameID
	{
	DWORD dwNSID = 0;
	DWORD dwLocalID = 0;

	auto operator<=>(const SXMLNameID&) const = default; // defines == and <=> 
	};

class CXMLStore
	{
	public:

		void AddNamespaceMap (CStringView sFromNamespace, CStringView sToNamespace) { m_NamespaceMappings.SetAt(m_Symbols.Atomize(sFromNamespace), m_Symbols.Atomize(sToNamespace)); }
		SXMLNameID Atomize (CStringSlice sLocalName) { return { 0, m_Symbols.Atomize(sLocalName) }; }
		SXMLNameID Atomize (CStringSlice sNamespace, CStringSlice sLocalName) { return { (sNamespace.IsEmpty() ? 0 : MapNamespace(m_Symbols.Atomize(sNamespace))), m_Symbols.Atomize(sLocalName) }; }
		const CSymbolTable& GetSymbols () const { return m_Symbols; }
		CSymbolTable& GetSymbols () { return m_Symbols; }

	private:

		DWORD MapNamespace (DWORD dwFrom) const { DWORD* pTo = m_NamespaceMappings.GetAt(dwFrom); return (pTo ? *pTo : dwFrom); }

		CSymbolTable m_Symbols;
		TSortMap<DWORD, DWORD> m_NamespaceMappings;
	};

class CXMLElement
	{
	public:

		CXMLElement (CXMLStore& Store) : m_Store(Store) { }
		CXMLElement (const CXMLElement& Obj);
		CXMLElement (CXMLStore& Store, SXMLNameID TagID, CXMLElement* pParent);
		CXMLElement (CXMLStore& Store, const CString& sTag, CXMLElement* pParent);
		~CXMLElement (void) { CleanUp(); }

		CXMLElement &operator= (const CXMLElement& Obj);

		static bool ParseXML (const IMemoryBlock& Stream, 
							  CXMLStore& Store,
							  CXMLElement **retpElement, 
							  CString *retsError,
							  CExternalEntityTable *retEntityTable = NULL);
		static bool ParseXML (const IMemoryBlock& Stream, 
							  CXMLStore& Store,
							  IXMLParserController *pController,
							  CXMLElement **retpElement, 
							  CString *retsError,
							  CExternalEntityTable *retEntityTable = NULL);
		static bool ParseEntityTable (IMemoryBlock &Stream, CXMLStore& Store, CExternalEntityTable *retEntityTable, CString *retsError);
		static bool ParseRootElement (IMemoryBlock &Stream, CXMLStore& Store, CXMLElement **retpRoot, CExternalEntityTable *retEntityTable, CString *retsError);
		static bool ParseRootTag (IMemoryBlock &Stream, CXMLStore& Store, CString *retsTag);

		SXMLNameID Atomize (CStringView sNamespace, CStringView sName) const { return m_Store.Atomize(sNamespace, sName); }
		SXMLNameID Atomize (const CString& sSymbol) const { return m_Store.Atomize(sSymbol); }

		void AddAttribute (SXMLNameID ID, const CString &sValue) { m_Attributes.Insert({ ID, sValue }); }
		void AddAttribute (SXMLNameID ID, CString&& sValue) { m_Attributes.Insert({ ID, std::move(sValue) }); }
		void AppendContent (const CString &sContent);
		void AppendContent (CString&& sContent);
		void AppendSubElement (CXMLElement *pElement);
		bool AttributeExists (SXMLNameID ID) const;
		bool FindAttribute (SXMLNameID ID, CString *retsValue = NULL) const;
		bool FindAttributeBool (SXMLNameID ID, bool *retbValue = NULL) const;
		bool FindAttributeDouble (SXMLNameID ID, double *retrValue = NULL) const;
		bool FindAttributeInteger (SXMLNameID ID, int *retiValue = NULL) const;
		const CString &GetAttribute (SXMLNameID ID) const;
		const CString &GetAttribute (int iIndex) const { return m_Attributes[iIndex].sValue; }
		bool GetAttributeBool (SXMLNameID ID) const;
		int GetAttributeCount (void) const { return m_Attributes.GetCount(); }
		double GetAttributeDouble (SXMLNameID ID) const;
		double GetAttributeDoubleBounded (SXMLNameID ID, double rMin, double rMax = -1.0, double rNull = 0.0) const;
		int GetAttributeInteger (SXMLNameID ID) const;
		int GetAttributeIntegerBounded (SXMLNameID ID, int iMin, int iMax = -1, int iNull = 0) const;
		void GetAttributeIntegerList (SXMLNameID ID, TArray<int> *retList) const;
		float GetAttributeFloat (SXMLNameID ID) const;
		SXMLNameID GetAttributeID (int iIndex) const { return m_Attributes[iIndex].Attrib; }
		const CString &GetAttributeName (int iIndex) const { return Symbol(GetAttributeID(iIndex).dwLocalID); }
		int GetContentElementCount (void) const { return m_ContentElements.GetCount(); }
		const CXMLElement *GetContentElement (int iOrdinal) const { return m_ContentElements[iOrdinal]; }
		const CXMLElement *GetContentElementByTag (SXMLNameID ID) const;
		const CString &GetContentText (int iOrdinal) const { return (iOrdinal < m_ContentText.GetCount() ? m_ContentText[iOrdinal] : NULL_STR); }
		const CXMLElement *GetParentElement (void) const { return m_pParent; }
		const CString &GetTag (void) const { return Symbol(m_Tag.dwLocalID); }
		SXMLNameID GetTagID () const { return m_Tag; }
		CXMLElement *OrphanCopy (void);
		void SetAttribute (SXMLNameID ID, const CString &sValue);

		void AddAttribute (const CString &sAttribute, const CString &sValue) { AddAttribute(Atomize(sAttribute), sValue); }
		bool AttributeExists (const CString &sName) const { return AttributeExists(FindSymbol(sName)); }
		bool FindAttribute (const CString &sName, CString *retsValue = NULL) const { return FindAttribute(FindSymbol(sName), retsValue); }
		bool FindAttributeBool (const CString &sName, bool *retbValue = NULL) const { return FindAttributeBool(FindSymbol(sName), retbValue); }
		bool FindAttributeDouble (const CString &sName, double *retrValue = NULL) const { return FindAttributeDouble(FindSymbol(sName), retrValue); }
		bool FindAttributeInteger (const CString &sName, int *retiValue = NULL) const { return FindAttributeInteger(FindSymbol(sName), retiValue); }
		const CString &GetAttribute (const CString &sName) const { return GetAttribute(FindSymbol(sName)); }
		bool GetAttributeBool (const CString &sName) const { return GetAttributeBool(FindSymbol(sName)); }
		double GetAttributeDouble (const CString &sName) const { return GetAttributeDouble(FindSymbol(sName)); }
		double GetAttributeDoubleBounded (const CString &sName, double rMin, double rMax = -1.0, double rNull = 0.0) const { return GetAttributeDoubleBounded(FindSymbol(sName), rMin, rMax, rNull); }
		int GetAttributeInteger (const CString &sName) const { return GetAttributeInteger(FindSymbol(sName)); }
		int GetAttributeIntegerBounded (const CString &sName, int iMin, int iMax = -1, int iNull = 0) const { return GetAttributeIntegerBounded(FindSymbol(sName), iMin, iMax, iNull); }
		void GetAttributeIntegerList (const CString &sName, TArray<int> *retList) const { return GetAttributeIntegerList(FindSymbol(sName), retList); }
		float GetAttributeFloat (const CString &sName) const { return GetAttributeFloat(FindSymbol(sName)); }
		const CXMLElement *GetContentElementByTag (const CString &sTag) const { return GetContentElementByTag(FindSymbol(sTag)); }
		void SetAttribute (const CString &sName, const CString &sValue) { SetAttribute(Atomize(sName), sValue); }

		static CString MakeAttribute (const CString &sText);
		static bool IsBoolTrueValue (const CString &sValue);

	private:

		struct SAttrib
			{
			SXMLNameID Attrib;
			CString sValue;
			};

		int FindAttributeIndex (SXMLNameID ID) const;
		SXMLNameID FindSymbol (const CString& sSymbol) const { return m_Store.Atomize(sSymbol); }
		void CleanUp (void);
		const CString& Symbol (DWORD dwAtom) const { return m_Store.GetSymbols().Symbol(dwAtom); }

		CXMLStore& m_Store;
		SXMLNameID m_Tag;							//	Element tag
		CXMLElement *m_pParent = NULL;				//	Parent of this element

		//	NOTE: In practice, an unsorted array of attributes is faster than
		//	a TSortMap for a small number of attributes. If this becomes a 
		//	bottleneck, we should optionally add an index when the number of
		//	attributes grows beyond a certain size.

		TArray<SAttrib> m_Attributes;				//	Array of attributes.

		TArray<CXMLElement *> m_ContentElements;	//	Array of sub elements
		TArray<CString> m_ContentText;				//	Interleaved content
	};

class CExternalEntityTable : public IXMLParserController
	{
	public:
		CExternalEntityTable (void);

		void AddTable (const TSortMap<CString, CString> &Table);
		int GetCount (void) { return m_Entities.GetCount(); }
		void GetEntity (int iIndex, CString *retsEntity, CString *retsValue);

		virtual CString ResolveExternalEntity (const CString &sName, bool *retbFound = NULL);
		virtual void SetParent (IXMLParserController *pParent) { m_pParent = pParent; }

	private:
		TSortMap<CString, CString> m_Entities;
		IXMLParserController *m_pParent;
	};

class CEntityResolverList : public IXMLParserController
	{
	public:
		CEntityResolverList (void) { }

		void AddResolver (IXMLParserController *pResolver) { m_Resolvers.Insert(pResolver); }

		//	IXMLParserController virtuals
		virtual CString ResolveExternalEntity (const CString &sName, bool *retbFound = NULL);

	private:
		TArray<IXMLParserController *> m_Resolvers;
	};

//	Some utilities

void CreateXMLElementFromCommandLine (int argc, char *argv[], CXMLElement **retpElement);
void ParseAttributeIntegerList (const CString &sValue, TArray<int> *retList);
CString strToXMLText (const CString &sText);
CString strToXMLTextUTF8 (CStringView sText);
