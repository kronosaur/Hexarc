//	CAEONXMLElement.cpp
//
//	CAEONXMLElement class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(FIELD_ERROR,						"error");
DECLARE_CONST_STRING(FIELD_TAG,							"tag");

DECLARE_CONST_STRING(TAG_ERROR,							"Error");

DECLARE_CONST_STRING(TYPENAME_XML_ELEMENT,				"xmlElement");

DECLARE_CONST_STRING(XML_PROLOGUE,						"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

TDatumPropertyHandler<CAEONXMLElement> CAEONXMLElement::m_Properties = {
	{
		"attributes",
		"x",
		"Returns the set of attributes as a struct.",
		[](const CAEONXMLElement &Obj, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeStruct);
			for (int i = 0; i < Obj.m_Attributes.GetCount(); i++)
				dResult.SetElement(Obj.m_Attributes.GetKey(i), Obj.m_Attributes[i]);

			return dResult;
			},
		NULL,
		},
	{
		"children",
		"a",
		"Returns an array of child elements.",
		[](const CAEONXMLElement &Obj, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			for (int i = 0; i < Obj.m_Children.GetCount(); i++)
				dResult.Append(Obj.m_Children[i]);

			return dResult;
			},
		NULL,
		},
	{
		"tag",
		"s",
		"Returns the element tag.",
		[](const CAEONXMLElement &Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_sTag);
			},
		NULL,
		},
	{
		"text",
		"?",
		"Returns either the content text or an array of text.",
		[](const CAEONXMLElement &Obj, const CString &sProperty)
			{
			if (Obj.m_Text.GetCount() == 0)
				return CDatum();
			else if (Obj.m_Text.GetCount() == 1)
				return CDatum(Obj.m_Text[0]);
			else
				{
				CDatum dResult(CDatum::typeArray);
				for (int i = 0; i < Obj.m_Text.GetCount(); i++)
					dResult.Append(Obj.m_Text[i]);

				return dResult;
				}
			},
		NULL,
		},
	};

TDatumMethodHandler<CAEONXMLElement> CAEONXMLElement::m_Methods = {
	{
		"copy",
		"0:",
		".copy() -> xmlElement",
		0,
		[](CAEONXMLElement& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CDatum dXML = CDatum::raw_AsComplex(&Obj);
			retResult.dResult = dXML.Clone();
			return true;
			},
		},
	{
		"getChildByTag",
		"0:tag=?",
		".getChildByTag(tag) -> xmlElement",
		0,
		[](CAEONXMLElement& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sTag = LocalEnv.GetArgument(1);

			for (int i = 0; i < Obj.m_Children.GetCount(); i++)
				{
				if (strEqualsNoCase(Obj.m_Children[i].GetElement(FIELD_TAG).AsStringView(), sTag))
					{
					retResult.dResult = Obj.m_Children[i];
					return true;
					}
				}

			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"getChildrenByTag",
		"a:tag=?",
		".getChildrenByTag(tag) -> array",
		0,
		[](CAEONXMLElement& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sTag = LocalEnv.GetArgument(1);

			retResult.dResult = CDatum(CDatum::typeArray);
			for (int i = 0; i < Obj.m_Children.GetCount(); i++)
				{
				if (strEqualsNoCase(Obj.m_Children[i].GetElement(FIELD_TAG).AsStringView(), sTag))
					retResult.dResult.Append(Obj.m_Children[i]);
				}

			return true;
			},
		},
	};

const CString& CAEONXMLElement::StaticGetTypename (void)
	{
	return TYPENAME_XML_ELEMENT;
	}

CString CAEONXMLElement::AsString () const
	{
	CStringBuffer Output;
	Output.Write(XML_PROLOGUE);

	WriteAsXML(Output);

	return CString(std::move(Output));
	}

size_t CAEONXMLElement::CalcMemorySize () const
	{
	size_t dwSize = m_sTag.GetLength() + sizeof(DWORD) + 1;
	for (int i = 0; i < m_Attributes.GetCount(); i++)
		dwSize += m_Attributes.GetKey(i).GetLength() + m_Attributes[i].GetLength() + 2 * (sizeof(DWORD) + 1);

	for (int i = 0; i < m_Children.GetCount(); i++)
		dwSize += m_Children[i].CalcMemorySize();

	return dwSize;
	}

CDatum CAEONXMLElement::Create (const IMemoryBlock& FileData)

//	Create
//
//	Creates a new element from an XML file.

	{
	CXMLStore XMLStore;

	CXMLElement* pElement;
	CString sError;
	if (!CXMLElement::ParseXML(FileData, XMLStore, &pElement, &sError))
		return CreateError(sError);

	CDatum dResult = Create(*pElement);
	delete pElement;

	return dResult;
	}

CDatum CAEONXMLElement::Create (const CXMLElement& Element)

//	Create
//
//	Create from an XML element.

	{
	auto* pXML = new CAEONXMLElement;
	pXML->m_sTag = Element.GetTag();
	pXML->m_Attributes.GrowToFit(Element.GetAttributeCount());
	for (int i = 0; i < Element.GetAttributeCount(); i++)
		pXML->m_Attributes.SetAt(Element.GetAttributeName(i), Element.GetAttribute(i));

	pXML->m_Children.GrowToFit(Element.GetContentElementCount());
	pXML->m_Text.GrowToFit(Element.GetContentElementCount() + 1);
	for (int i = 0; i < Element.GetContentElementCount(); i++)
		{
		pXML->m_Children.Insert(Create(*Element.GetContentElement(i)));
		pXML->m_Text.Insert(Element.GetContentText(i));
		}

	pXML->m_Text.Insert(Element.GetContentText(Element.GetContentElementCount()));

	return CDatum(pXML);
	}

CDatum CAEONXMLElement::CreateError (CStringView sError)

//	CreateError
//
//	Create an error XML object.

	{
	auto* pXML = new CAEONXMLElement;
	pXML->m_sTag = TAG_ERROR;
	pXML->m_Attributes.SetAt(FIELD_ERROR, sError);

	return CDatum(pXML);
	}

void CAEONXMLElement::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized)
	{
	m_sTag = CString::Deserialize(Stream);

	int iAttribCount = Stream.ReadInt();
	m_Attributes.GrowToFit(iAttribCount);
	for (int i = 0; i < iAttribCount; i++)
		{
		CString sKey = CString::Deserialize(Stream);
		CString sValue = CString::Deserialize(Stream);
		m_Attributes.SetAt(sKey, std::move(sValue));
		}

	int iChildCount = Stream.ReadInt();
	m_Children.GrowToFit(iChildCount);
	for (int i = 0; i < iChildCount; i++)
		{
		m_Children.Insert(CDatum::DeserializeAEON(Stream, Serialized));
		}

	int iTextCount = Stream.ReadInt();
	m_Text.GrowToFit(iTextCount);
	for (int i = 0; i < iTextCount; i++)
		{
		m_Text.Insert(CString::Deserialize(Stream));
		}
	}

CDatum CAEONXMLElement::GetDatatype () const
	{
	return CAEONTypes::Get(CHTTPUtil::XML_ELEMENT_TYPE);
	}

TArray<IDatatype::SMemberDesc> CAEONXMLElement::GetMembers (void)

//	GetMembers
//
//	Returns a list of members.

	{
	TArray<IDatatype::SMemberDesc> Members;

	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);

	return Members;
	}

int CAEONXMLElement::OpCompare (CDatum::Types iValueType, CDatum dValue) const
	{
	const CAEONXMLElement* pSrc = CAEONXMLElement::Upconvert(dValue);
	if (!pSrc)
		return -2;

	//	Tags are case-sensitive.

	int iResult = KeyCompare(m_sTag, pSrc->m_sTag);
	if (iResult != 0)
		return iResult;

	if (m_Attributes.GetCount() < pSrc->m_Attributes.GetCount())
		return -1;
	else if (m_Attributes.GetCount() > pSrc->m_Attributes.GetCount())
		return 1;

	for (int i = 0; i < m_Attributes.GetCount(); i++)
		{
		//	Attribute keys are case-sensitive.

		iResult = KeyCompare(m_Attributes.GetKey(i), pSrc->m_Attributes.GetKey(i));
		if (iResult != 0)
			return iResult;

		iResult = KeyCompareNoCase(m_Attributes[i], pSrc->m_Attributes[i]);
		if (iResult != 0)
			return iResult;
		}

	if (m_Children.GetCount() < pSrc->m_Children.GetCount())
		return -1;
	else if (m_Children.GetCount() > pSrc->m_Children.GetCount())
		return 1;

	for (int i = 0; i < m_Children.GetCount(); i++)
		{
		iResult = m_Children[i].OpCompare(pSrc->m_Children[i]);
		if (iResult != 0)
			return iResult;
		}

	if (m_Text.GetCount() < pSrc->m_Text.GetCount())
		return -1;
	else if (m_Text.GetCount() > pSrc->m_Text.GetCount())
		return 1;

	for (int i = 0; i < m_Text.GetCount(); i++)
		{
		iResult = KeyCompareNoCase(m_Text[i], pSrc->m_Text[i]);
		if (iResult != 0)
			return iResult;
		}

	return 0;
	}

int CAEONXMLElement::OpCompareExact (CDatum::Types iValueType, CDatum dValue) const
	{
	const CAEONXMLElement* pSrc = CAEONXMLElement::Upconvert(dValue);
	if (!pSrc)
		return -2;

	//	Tags are case-sensitive.

	int iResult = KeyCompare(m_sTag, pSrc->m_sTag);
	if (iResult != 0)
		return iResult;

	if (m_Attributes.GetCount() < pSrc->m_Attributes.GetCount())
		return -1;
	else if (m_Attributes.GetCount() > pSrc->m_Attributes.GetCount())
		return 1;

	for (int i = 0; i < m_Attributes.GetCount(); i++)
		{
		//	Attribute keys are case-sensitive.

		iResult = KeyCompare(m_Attributes.GetKey(i), pSrc->m_Attributes.GetKey(i));
		if (iResult != 0)
			return iResult;

		iResult = KeyCompare(m_Attributes[i], pSrc->m_Attributes[i]);
		if (iResult != 0)
			return iResult;
		}

	if (m_Children.GetCount() < pSrc->m_Children.GetCount())
		return -1;
	else if (m_Children.GetCount() > pSrc->m_Children.GetCount())
		return 1;

	for (int i = 0; i < m_Children.GetCount(); i++)
		{
		iResult = m_Children[i].OpCompareExact(pSrc->m_Children[i]);
		if (iResult != 0)
			return iResult;
		}

	if (m_Text.GetCount() < pSrc->m_Text.GetCount())
		return -1;
	else if (m_Text.GetCount() > pSrc->m_Text.GetCount())
		return 1;

	for (int i = 0; i < m_Text.GetCount(); i++)
		{
		iResult = KeyCompare(m_Text[i], pSrc->m_Text[i]);
		if (iResult != 0)
			return iResult;
		}

	return 0;
	}

bool CAEONXMLElement::OpIsEqual (CDatum::Types iValueType, CDatum dValue) const
	{
	const CAEONXMLElement* pSrc = CAEONXMLElement::Upconvert(dValue);
	if (!pSrc)
		return false;

	if (!strEqualsNoCase(m_sTag, pSrc->m_sTag))
		return false;

	if (m_Attributes.GetCount() != pSrc->m_Attributes.GetCount())
		return false;

	for (int i = 0; i < m_Attributes.GetCount(); i++)
		if (!strEqualsNoCase(m_Attributes.GetKey(i), pSrc->m_Attributes.GetKey(i))
				|| !strEqualsNoCase(m_Attributes[i], pSrc->m_Attributes[i]))
			return false;

	if (m_Children.GetCount() != pSrc->m_Children.GetCount())
		return false;

	for (int i = 0; i < m_Children.GetCount(); i++)
		if (!m_Children[i].OpIsEqual(pSrc->m_Children[i]))
			return false;

	if (m_Text.GetCount() != pSrc->m_Text.GetCount())
		return false;

	for (int i = 0; i < m_Text.GetCount(); i++)
		if (!strEqualsNoCase(m_Text[i], pSrc->m_Text[i]))
			return false;

	return true;
	}

bool CAEONXMLElement::OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const
	{
	const CAEONXMLElement* pSrc = CAEONXMLElement::Upconvert(dValue);
	if (!pSrc)
		return false;

	if (!strEquals(m_sTag, pSrc->m_sTag))
		return false;

	if (m_Attributes.GetCount() != pSrc->m_Attributes.GetCount())
		return false;

	for (int i = 0; i < m_Attributes.GetCount(); i++)
		if (!strEquals(m_Attributes.GetKey(i), pSrc->m_Attributes.GetKey(i))
				|| !strEquals(m_Attributes[i], pSrc->m_Attributes[i]))
			return false;

	if (m_Children.GetCount() != pSrc->m_Children.GetCount())
		return false;

	for (int i = 0; i < m_Children.GetCount(); i++)
		if (!m_Children[i].OpIsIdentical(pSrc->m_Children[i]))
			return false;

	if (m_Text.GetCount() != pSrc->m_Text.GetCount())
		return false;

	for (int i = 0; i < m_Text.GetCount(); i++)
		if (!strEquals(m_Text[i], pSrc->m_Text[i]))
			return false;

	return true;
	}

void CAEONXMLElement::OnMarked ()

//	OnMarked
//
//	Mark data in use.

	{
	for (int i = 0; i < m_Children.GetCount(); i++)
		m_Children[i].Mark();
	}

bool CAEONXMLElement::OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream)
	{
	return false;
	}

void CAEONXMLElement::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const
	{
	throw CException(errFail);
	}

void CAEONXMLElement::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const
	{
	m_sTag.Serialize(Stream);
	Stream.Write(m_Attributes.GetCount());
	for (int i = 0; i < m_Attributes.GetCount(); i++)
		{
		m_Attributes.GetKey(i).Serialize(Stream);
		m_Attributes[i].Serialize(Stream);
		}

	Stream.Write(m_Children.GetCount());
	for (int i = 0; i < m_Children.GetCount(); i++)
		{
		m_Children[i].SerializeAEON(Stream, Serialized);
		}

	Stream.Write(m_Text.GetCount());
	for (int i = 0; i < m_Text.GetCount(); i++)
		{
		m_Text[i].Serialize(Stream);
		}
	}

void CAEONXMLElement::WriteAsXML (CStringBuffer& Output) const

//	WriteAsXML
//
//	Writes the element as XML.

	{
	Output.Write(strPattern("<%s", m_sTag));

	for (int i = 0; i < m_Attributes.GetCount(); i++)
		Output.Write(strPattern(" %s=\"%s\"", m_Attributes.GetKey(i), strToXMLTextUTF8(m_Attributes[i])));

	if (m_Children.GetCount() == 0 && m_Text.GetCount() == 0)
		{
		Output.Write("/>\n");
		return;
		}

	Output.Write(">");

	for (int i = 0; i < m_Children.GetCount(); i++)
		{
		if (i < m_Text.GetCount())
			Output.Write(strToXMLTextUTF8(m_Text[i]));

		const CAEONXMLElement* pChild = CAEONXMLElement::Upconvert(m_Children[i]);
		if (pChild)
			pChild->WriteAsXML(Output);
		}

	if (m_Children.GetCount() < m_Text.GetCount())
		Output.Write(strToXMLTextUTF8(m_Text[m_Text.GetCount() - 1]));

	Output.Write(strPattern("</%s>", m_sTag));
	}
