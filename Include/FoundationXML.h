//	FoundationXML.h
//
//	Foundation header file
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.
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

class CXMLElement
	{
	public:
		CXMLElement (void);
		CXMLElement (const CXMLElement &Obj);
		CXMLElement (const CString &sTag, CXMLElement *pParent);
		~CXMLElement (void) { CleanUp(); }

		CXMLElement &operator= (const CXMLElement &Obj);

		static bool ParseXML (IMemoryBlock &Stream, 
								 CXMLElement **retpElement, 
								 CString *retsError,
								 CExternalEntityTable *retEntityTable = NULL);
		static bool ParseXML (IMemoryBlock &Stream, 
								 IXMLParserController *pController,
								 CXMLElement **retpElement, 
								 CString *retsError,
								 CExternalEntityTable *retEntityTable = NULL);
		static bool ParseEntityTable (IMemoryBlock &Stream, CExternalEntityTable *retEntityTable, CString *retsError);
		static bool ParseRootElement (IMemoryBlock &Stream, CXMLElement **retpRoot, CExternalEntityTable *retEntityTable, CString *retsError);
		static bool ParseRootTag (IMemoryBlock &Stream, CString *retsTag);

		void AddAttribute (const CString &sAttribute, const CString &sValue);
		void AppendContent (const CString &sContent);
		void AppendSubElement (CXMLElement *pElement);
		bool AttributeExists (const CString &sName);
		bool FindAttribute (const CString &sName, CString *retsValue = NULL);
		bool FindAttributeBool (const CString &sName, bool *retbValue = NULL);
		bool FindAttributeDouble (const CString &sName, double *retrValue = NULL);
		bool FindAttributeInteger (const CString &sName, int *retiValue = NULL);
		CString GetAttribute (const CString &sName);
		inline CString GetAttribute (int iIndex) { return m_Attributes[iIndex]; }
		bool GetAttributeBool (const CString &sName);
		inline int GetAttributeCount (void) { return m_Attributes.GetCount(); }
		double GetAttributeDouble (const CString &sName);
		double GetAttributeDoubleBounded (const CString &sName, double rMin, double rMax = -1.0, double rNull = 0.0);
		int GetAttributeInteger (const CString &sName);
		int GetAttributeIntegerBounded (const CString &sName, int iMin, int iMax = -1, int iNull = 0);
		void GetAttributeIntegerList (const CString &sName, TArray<int> *retList);
		float GetAttributeFloat (const CString &sName);
		inline CString GetAttributeName (int iIndex) { return m_Attributes.GetKey(iIndex); }
		inline int GetContentElementCount (void) const { return m_ContentElements.GetCount(); }
		inline CXMLElement *GetContentElement (int iOrdinal) const { return m_ContentElements[iOrdinal]; }
		CXMLElement *GetContentElementByTag (const CString &sTag) const;
		inline const CString &GetContentText (int iOrdinal) { return (iOrdinal < m_ContentText.GetCount() ? m_ContentText[iOrdinal] : NULL_STR); }
		inline CXMLElement *GetParentElement (void) const { return m_pParent; }
		inline const CString &GetTag (void) const { return m_sTag; }
		CXMLElement *OrphanCopy (void);
		void SetAttribute (const CString &sName, const CString &sValue);

		static CString MakeAttribute (const CString &sText);
		static bool IsBoolTrueValue (const CString &sValue);

	private:
		void CleanUp (void);

		CString m_sTag;							//	Element tag
		CXMLElement *m_pParent;					//	Parent of this element
		TSortMap<CString, CString> m_Attributes;//	Table of CStrings
		TArray<CXMLElement *> m_ContentElements;//	Array of sub elements
		TArray<CString> m_ContentText;			//	Interleaved content
	};

class CExternalEntityTable : public IXMLParserController
	{
	public:
		CExternalEntityTable (void);

		void AddTable (const TSortMap<CString, CString> &Table);
		inline int GetCount (void) { return m_Entities.GetCount(); }
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

		inline void AddResolver (IXMLParserController *pResolver) { m_Resolvers.Insert(pResolver); }

		//	IXMLParserController virtuals
		virtual CString ResolveExternalEntity (const CString &sName, bool *retbFound = NULL);

	private:
		TArray<IXMLParserController *> m_Resolvers;
	};

//	Some utilities

void CreateXMLElementFromCommandLine (int argc, char *argv[], CXMLElement **retpElement);
void ParseAttributeIntegerList (const CString &sValue, TArray<int> *retList);
CString strToXMLText (const CString &sText);
