//	CXMLElement.cpp
//
//	CXMLElement class
//	Copyright (c) 2014 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_1,							"1")
DECLARE_CONST_STRING(STR_TRUE,						"true")

CXMLElement::CXMLElement (const CXMLElement &Obj) : 
		m_Store(Obj.m_Store)

//	CXMLElement constructor

	{
	m_Tag = Obj.m_Tag;
	m_pParent = Obj.m_pParent;
	m_Attributes = Obj.m_Attributes;
	m_ContentText = Obj.m_ContentText;

	m_ContentElements.InsertEmpty(Obj.m_ContentElements.GetCount());
	for (int i = 0; i < m_ContentElements.GetCount(); i++)
		m_ContentElements[i] = new CXMLElement(*Obj.m_ContentElements[i]);
	}

CXMLElement::CXMLElement (CXMLStore& Store, SXMLNameID TagID, CXMLElement* pParent) :
		m_Store(Store),
		m_Tag(TagID),
		m_pParent(pParent)

//	CXMLElement constructor

	{
	}

CXMLElement::CXMLElement (CXMLStore& Store, const CString &sTag, CXMLElement *pParent) :
		m_Store(Store),
		m_Tag(Atomize(sTag)),
		m_pParent(pParent)

//	CXMLElement constructor

	{
	}

CXMLElement &CXMLElement::operator= (const CXMLElement &Obj)

//	CXMLElement operator=

	{
	CleanUp();

	m_Tag = Obj.m_Tag;
	m_pParent = Obj.m_pParent;
	m_Attributes = Obj.m_Attributes;
	m_ContentText = Obj.m_ContentText;

	m_ContentElements.InsertEmpty(Obj.m_ContentElements.GetCount());
	for (int i = 0; i < m_ContentElements.GetCount(); i++)
		m_ContentElements[i] = new CXMLElement(*Obj.m_ContentElements[i]);

	return *this;
	}

void CXMLElement::AppendContent (const CString &sContent)

//	AppendContent
//
//	Appends some content

	{
	//	Always append to the last content element

	int iCount = m_ContentText.GetCount();
	if (iCount == 0)
		{
		//	If we have sub elements, but no content array then it means we've
		//	deferred creating the content array until we actually have content.
		//	We need to add some empty content.

		if (m_ContentElements.GetCount() > 0)
			{
			m_ContentText.InsertEmpty(m_ContentElements.GetCount() + 1);
			m_ContentText[m_ContentElements.GetCount()] = sContent;
			}
		else
			{
			m_ContentText.Insert(sContent);
			}
		}
	else
		m_ContentText[iCount - 1] += sContent;
	}

void CXMLElement::AppendContent (CString&& sContent)

//	AppendContent
//
//	Appends some content

	{
	//	Always append to the last content element

	int iCount = m_ContentText.GetCount();
	if (iCount == 0)
		{
		//	If we have sub elements, but no content array then it means we've
		//	deferred creating the content array until we actually have content.
		//	We need to add some empty content.

		if (m_ContentElements.GetCount() > 0)
			{
			m_ContentText.InsertEmpty(m_ContentElements.GetCount() + 1);
			m_ContentText[m_ContentElements.GetCount()] = std::move(sContent);
			}
		else
			{
			m_ContentText.Insert(std::move(sContent));
			}
		}
	else
		m_ContentText[iCount - 1] += sContent;
	}

void CXMLElement::AppendSubElement (CXMLElement *pElement)

//	AppendSubElement
//
//	Append a sub element

	{
	//	Append the element

	m_ContentElements.Insert(pElement);

	//	If we have content text, then add content text entries.

	if (m_ContentText.GetCount() > 0)
		{
		m_ContentText.Insert(NULL_STR);
		}
	}

bool CXMLElement::AttributeExists (SXMLNameID ID) const

//	AttributeExists
//
//	Returns TRUE if the attribute exists in the element

	{
	return (FindAttributeIndex(ID) != -1);
	}

void CXMLElement::CleanUp (void)

//	CleanUp
//
//	Free

	{
	for (int i = 0; i < m_ContentElements.GetCount(); i++)
		delete m_ContentElements[i];

	m_ContentElements.DeleteAll();
	}

bool CXMLElement::FindAttribute (SXMLNameID ID, CString *retsValue) const

//	FindAttribute
//
//	If the attribute exists, returns TRUE and the attribute value.
//	Otherwise, returns FALSE

	{
	int iIndex = FindAttributeIndex(ID);
	if (iIndex == -1)
		return false;

	if (retsValue)
		*retsValue = m_Attributes[iIndex].sValue;

	return true;
	}

bool CXMLElement::FindAttributeBool (SXMLNameID ID, bool *retbValue) const

//	FindAttributeBool
//
//	If the attribute exists, returns TRUE and the value
//	Otherwise, returns FALSE

	{
	int iIndex = FindAttributeIndex(ID);
	if (iIndex == -1)
		return false;

	if (retbValue)
		*retbValue = IsBoolTrueValue(m_Attributes[iIndex].sValue);

	return true;
	}

bool CXMLElement::FindAttributeDouble (SXMLNameID ID, double *retrValue) const

//	FindAttributeDouble
//
//	Finds an attribute.

	{
	int iIndex = FindAttributeIndex(ID);
	if (iIndex == -1)
		return false;

	if (retrValue)
		*retrValue = strToDouble(m_Attributes[iIndex].sValue);

	return true;
	}

int CXMLElement::FindAttributeIndex (SXMLNameID ID) const

//	FindAttributeIndex
//
//	Returns the index of the given attribute (of -1 if not found).

	{
	for (int i = 0; i < m_Attributes.GetCount(); i++)
		if (m_Attributes[i].Attrib == ID)
			return i;

	return -1;
	}

bool CXMLElement::FindAttributeInteger (SXMLNameID ID, int *retiValue) const

//	FindAttributeInteger
//
//	If the attribute exists, returns TRUE and the attribute value.
//	Otherwise, returns FALSE

	{
	int iIndex = FindAttributeIndex(ID);
	if (iIndex == -1)
		return false;

	if (retiValue)
		*retiValue = strToInt(m_Attributes[iIndex].sValue, 0, NULL);

	return true;
	}

const CString& CXMLElement::GetAttribute (SXMLNameID ID) const

//	GetAttribute
//
//	Returns the attribute

	{
	int iIndex = FindAttributeIndex(ID);
	if (iIndex == -1)
		return NULL_STR;

	return m_Attributes[iIndex].sValue;
	}

bool CXMLElement::GetAttributeBool (SXMLNameID ID) const

//	GetAttributeBool
//
//	Returns TRUE or FALSE for the attribute

	{
	int iIndex = FindAttributeIndex(ID);
	if (iIndex == -1)
		return false;

	return IsBoolTrueValue(m_Attributes[iIndex].sValue);
	}

double CXMLElement::GetAttributeDouble (SXMLNameID ID) const

//	GetAttributeDouble
//
//	Returns a double attribute.

	{
	int iIndex = FindAttributeIndex(ID);
	if (iIndex == -1)
		return 0.0;

	return strToDouble(m_Attributes[iIndex].sValue);
	}

double CXMLElement::GetAttributeDoubleBounded (SXMLNameID ID, double rMin, double rMax, double rNull) const

//	GetAttributeDoubleBounded
//
//	Returns a double, insuring that it is in range

	{
	CString sValue;
	if (FindAttribute(ID, &sValue))
		{
		double rValue = strToDouble(sValue);

		//	The null value is always valid

		if (rValue == rNull)
			return rValue;

		//	If iMax is less than iMin, then there is no maximum

		else if (rMax < rMin)
			return Max(rValue, rMin);

		//	Bounded

		else
			return Max(Min(rValue, rMax), rMin);
		}
	else
		return rNull;
	}

float CXMLElement::GetAttributeFloat (SXMLNameID ID) const

//	GetAttributeFloat
//
//	Returns a floating point attribute

	{
	int iIndex = FindAttributeIndex(ID);
	if (iIndex == -1)
		return 0.0;

	return (float)strToDouble(m_Attributes[iIndex].sValue);
	}

int CXMLElement::GetAttributeInteger (SXMLNameID ID) const

//	GetAttributeInteger
//
//	Returns an integer attribute

	{
	return strToInt(GetAttribute(ID), 0, NULL);
	}

int CXMLElement::GetAttributeIntegerBounded (SXMLNameID ID, int iMin, int iMax, int iNull) const

//	GetAttributeIntegerBounded
//
//	Returns an integer, insuring that it is in range

	{
	CString sValue;
	if (FindAttribute(ID, &sValue))
		{
		bool bFailed;
		int iValue = strToInt(sValue, iNull, &bFailed);
		if (bFailed)
			return iNull;

		//	The null value is always valid

		if (iValue == iNull)
			return iValue;

		//	If iMax is less than iMin, then there is no maximum

		else if (iMax < iMin)
			return Max(iValue, iMin);

		//	Bounded

		else
			return Max(Min(iValue, iMax), iMin);
		}
	else
		return iNull;
	}

void CXMLElement::GetAttributeIntegerList (SXMLNameID ID, TArray<int> *retList) const

//	GetAttributeIntegerList
//
//	Appends a list of integers separated by commas

	{
	ParseAttributeIntegerList(GetAttribute(ID), retList);
	}

const CXMLElement *CXMLElement::GetContentElementByTag (SXMLNameID ID) const

//	GetContentElementByTag
//
//	Returns a sub element of the given tag

	{
	for (int i = 0; i < GetContentElementCount(); i++)
		{
		const CXMLElement *pElement = GetContentElement(i);

		if (pElement->GetTagID() == ID)
			return pElement;
		}

	return NULL;
	}

CString CXMLElement::MakeAttribute (const CString &sText)

//	MakeAttribute
//
//	Convert to attribute text.
	
	{
	return strToXMLText(sText); 
	}

bool CXMLElement::IsBoolTrueValue (const CString &sValue)

//	IsBoolTrueValue
//
//	Returns TRUE if this is a boolean value of TRUE

	{
	return (strEquals(sValue, STR_TRUE) || strEquals(sValue, STR_1)); 
	}

CXMLElement *CXMLElement::OrphanCopy (void)

//	OrphanCopy
//
//	Creates a copy of the element and makes it top-level

	{
	CXMLElement *pCopy = new CXMLElement(*this);
	pCopy->m_pParent = NULL;
	return pCopy;
	}

void CXMLElement::SetAttribute (SXMLNameID ID, const CString &sValue)

//	SetAttribute
//
//	Sets an attribute on the element.

	{
	int iIndex = FindAttributeIndex(ID);
	if (iIndex == -1)
		AddAttribute(ID, sValue);
	else
		m_Attributes[iIndex].sValue = sValue;
	}
