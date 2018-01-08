//	CXMLElement.cpp
//
//	CXMLElement class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_1,							"1")
DECLARE_CONST_STRING(STR_TRUE,						"true")

CXMLElement::CXMLElement (void) :
		m_pParent(NULL)

//	CXMLElement constructor

	{
	}

CXMLElement::CXMLElement (const CXMLElement &Obj)

//	CXMLElement constructor

	{
	int i;

	m_sTag = Obj.m_sTag;
	m_pParent = Obj.m_pParent;
	m_Attributes = Obj.m_Attributes;
	m_ContentText = Obj.m_ContentText;

	m_ContentElements.InsertEmpty(Obj.m_ContentElements.GetCount());
	for (i = 0; i < m_ContentElements.GetCount(); i++)
		m_ContentElements[i] = new CXMLElement(*Obj.m_ContentElements[i]);
	}

CXMLElement::CXMLElement (const CString &sTag, CXMLElement *pParent) :
		m_sTag(sTag),
		m_pParent(pParent)

//	CXMLElement constructor

	{
	}

CXMLElement &CXMLElement::operator= (const CXMLElement &Obj)

//	CXMLElement operator=

	{
	int i;

	CleanUp();

	m_sTag = Obj.m_sTag;
	m_pParent = Obj.m_pParent;
	m_Attributes = Obj.m_Attributes;
	m_ContentText = Obj.m_ContentText;

	m_ContentElements.InsertEmpty(Obj.m_ContentElements.GetCount());
	for (i = 0; i < m_ContentElements.GetCount(); i++)
		m_ContentElements[i] = new CXMLElement(*Obj.m_ContentElements[i]);

	return *this;
	}

void CXMLElement::AddAttribute (const CString &sAttribute, const CString &sValue)

//	AddAttribute
//
//	Add the given attribute to our table

	{
	m_Attributes.Insert(sAttribute, sValue);
	}

void CXMLElement::AppendContent (const CString &sContent)

//	AppendContent
//
//	Appends some content

	{
	//	Always append to the last content element

	int iCount = m_ContentText.GetCount();
	if (iCount == 0)
		m_ContentText.Insert(sContent);
	else
		m_ContentText[iCount - 1] += sContent;
	}

void CXMLElement::AppendSubElement (CXMLElement *pElement)

//	AppendSubElement
//
//	Append a sub element

	{
	//	If this element was previously empty, we have to add
	//	a content text item. [Because we optimize the case where
	//	there are no children.]

	if (m_ContentText.GetCount() == 0)
		m_ContentText.Insert(NULL_STR);

	//	Append the element

	m_ContentElements.Insert(pElement);

	//	We always add a new content text value at the end

	m_ContentText.Insert(NULL_STR);
	}

bool CXMLElement::AttributeExists (const CString &sName)

//	AttributeExists
//
//	Returns TRUE if the attribute exists in the element

	{
	return (m_Attributes.GetAt(sName) != NULL);
	}

void CXMLElement::CleanUp (void)

//	CleanUp
//
//	Free

	{
	int i;

	for (i = 0; i < m_ContentElements.GetCount(); i++)
		delete m_ContentElements[i];

	m_ContentElements.DeleteAll();
	}

bool CXMLElement::FindAttribute (const CString &sName, CString *retsValue)

//	FindAttribute
//
//	If the attribute exists, returns TRUE and the attribute value.
//	Otherwise, returns FALSE

	{
	CString *pValue = m_Attributes.GetAt(sName);
	if (pValue == NULL)
		return false;

	if (retsValue)
		*retsValue = *pValue;

	return true;
	}

bool CXMLElement::FindAttributeBool (const CString &sName, bool *retbValue)

//	FindAttributeBool
//
//	If the attribute exists, returns TRUE and the value
//	Otherwise, returns FALSE

	{
	CString *pValue = m_Attributes.GetAt(sName);
	if (pValue == NULL)
		return false;

	if (retbValue)
		*retbValue = IsBoolTrueValue(*pValue);

	return true;
	}

bool CXMLElement::FindAttributeDouble (const CString &sName, double *retrValue)

//	FindAttributeDouble
//
//	Finds an attribute.

	{
	CString *pValue = m_Attributes.GetAt(sName);
	if (pValue == NULL)
		return false;

	if (retrValue)
		*retrValue = strToDouble(*pValue);

	return true;
	}

bool CXMLElement::FindAttributeInteger (const CString &sName, int *retiValue)

//	FindAttributeInteger
//
//	If the attribute exists, returns TRUE and the attribute value.
//	Otherwise, returns FALSE

	{
	CString *pValue = m_Attributes.GetAt(sName);
	if (pValue == NULL)
		return false;

	if (retiValue)
		*retiValue = strToInt(*pValue, 0, NULL);

	return true;
	}

CString CXMLElement::GetAttribute (const CString &sName)

//	GetAttribute
//
//	Returns the attribute

	{
	CString *pValue = m_Attributes.GetAt(sName);
	if (pValue == NULL)
		return NULL_STR;

	return *pValue;
	}

bool CXMLElement::GetAttributeBool (const CString &sName)

//	GetAttributeBool
//
//	Returns TRUE or FALSE for the attribute

	{
	CString *pValue = m_Attributes.GetAt(sName);
	if (pValue == NULL)
		return false;

	return IsBoolTrueValue(*pValue);
	}

double CXMLElement::GetAttributeDouble (const CString &sName)

//	GetAttributeDouble
//
//	Returns a double attribute.

	{
	CString *pValue = m_Attributes.GetAt(sName);
	if (pValue == NULL)
		return 0.0;

	return strToDouble(*pValue);
	}

double CXMLElement::GetAttributeDoubleBounded (const CString &sName, double rMin, double rMax, double rNull)

//	GetAttributeDoubleBounded
//
//	Returns a double, insuring that it is in range

	{
	CString sValue;
	if (FindAttribute(sName, &sValue))
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

float CXMLElement::GetAttributeFloat (const CString &sName)

//	GetAttributeFloat
//
//	Returns a floating point attribute

	{
	CString *pValue = m_Attributes.GetAt(sName);
	if (pValue == NULL)
		return 0.0;

	return (float)strToDouble(*pValue);
	}

int CXMLElement::GetAttributeInteger (const CString &sName)

//	GetAttributeInteger
//
//	Returns an integer attribute

	{
	return strToInt(GetAttribute(sName), 0, NULL);
	}

int CXMLElement::GetAttributeIntegerBounded (const CString &sName, int iMin, int iMax, int iNull)

//	GetAttributeIntegerBounded
//
//	Returns an integer, insuring that it is in range

	{
	CString sValue;
	if (FindAttribute(sName, &sValue))
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

void CXMLElement::GetAttributeIntegerList (const CString &sName, TArray<int> *retList)

//	GetAttributeIntegerList
//
//	Appends a list of integers separated by commas

	{
	ParseAttributeIntegerList(GetAttribute(sName), retList);
	}

CXMLElement *CXMLElement::GetContentElementByTag (const CString &sTag) const

//	GetContentElementByTag
//
//	Returns a sub element of the given tag

	{
	for (int i = 0; i < GetContentElementCount(); i++)
		{
		CXMLElement *pElement = GetContentElement(i);

		if (strEquals(sTag, pElement->GetTag()))
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

void CXMLElement::SetAttribute (const CString &sName, const CString &sValue)

//	SetAttribute
//
//	Sets an attribute on the element.

	{
	m_Attributes.SetAt(sName, sValue);
	}
