//	CDatatypeLiteralStruct.cpp
//
//	CDatatypeLiteralStruct class
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_LITERAL_STRUCT,			"LiteralStruct");

CDatatypeLiteralStruct::CDatatypeLiteralStruct (CDatum dSchema) : IDatatype(false, NULL_STR),
		m_dSchema(dSchema)

//	CDatatypeLiteralStruct constructor

	{
	}

bool CDatatypeLiteralStruct::OnCanBeCalledWith (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, CDatum* retdReturnType, CString* retsError) const
	{
	return false;
	}

bool CDatatypeLiteralStruct::OnCanBeConstructedFrom (CDatum dType) const
	{
	const IDatatype& Type = dType;
	if (Type.IsAny() || Type.IsA(*this))
		return true;

	//	We can construct from a schema.

	if (Type.IsA(IDatatype::SCHEMA))
		return true;

	return false;
	}

bool CDatatypeLiteralStruct::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion)
	{
	SetCoreType(Stream.ReadDWORD());

	if (!CComplexDatatype::CreateFromStream(Stream, m_dSchema))
		return false;

	return true;
	}

bool CDatatypeLiteralStruct::OnDeserializeAEON (IByteStream& Stream, DWORD dwVersion, CAEONSerializedMap &Serialized)
	{
	DWORD dwFlags = Stream.ReadDWORD();

	m_dSchema = CDatum::DeserializeAEON(Stream, Serialized);
	return true;
	}

bool CDatatypeLiteralStruct::OnEquals (const IDatatype &Src) const
	{
	auto& Other = (const CDatatypeLiteralStruct&)Src;

	return ((const IDatatype&)m_dSchema).IsEqualEx((const IDatatype&)Other.m_dSchema);
	}

int CDatatypeLiteralStruct::OnFindMember (CStringView sName) const
	{
	const IDatatype& Schema = m_dSchema;
	int iIndex = Schema.FindMember(sName);
	if (iIndex != -1)
		return iIndex;

	iIndex = CComplexStruct::FindPropertyByKey(sName);
	if (iIndex != -1)
		return Schema.GetMemberCount() + iIndex;

	iIndex = CComplexStruct::FindMethodByKey(sName);
	if (iIndex != -1)
		return Schema.GetMemberCount() + CComplexStruct::GetPropertyCount() + iIndex;

	return -1;
	}

IDatatype::SMemberDesc CDatatypeLiteralStruct::OnGetMember (int iIndex) const
	{
	if (iIndex < 0)
		throw CException(errFail);

	const IDatatype& Schema = m_dSchema;
	
	if (iIndex < Schema.GetMemberCount())
		return Schema.GetMember(iIndex);

	iIndex -= Schema.GetMemberCount();
	if (iIndex < CComplexStruct::GetPropertyCount())
		return SMemberDesc({ EMemberType::InstanceProperty, CComplexStruct::GetPropertyKey(iIndex), CComplexStruct::GetPropertyType(iIndex) });

	iIndex -= CComplexStruct::GetPropertyCount();
	if (iIndex < CComplexStruct::GetMethodCount())
		return SMemberDesc({ EMemberType::InstanceMethod, CComplexStruct::GetMethodKey(iIndex), CComplexStruct::GetMethodType(iIndex) });

	throw CException(errFail);
	}

int CDatatypeLiteralStruct::OnGetMemberCount () const
	{
	const IDatatype& Schema = m_dSchema;
	return Schema.GetMemberCount() + CComplexStruct::GetPropertyCount() + CComplexStruct::GetMethodCount();
	}

CString CDatatypeLiteralStruct::OnGetName () const
	{
	return TYPENAME_LITERAL_STRUCT;
	}

IDatatype::EMemberType CDatatypeLiteralStruct::OnHasMember (CStringView sName, CDatum* retdType, int* retiOrdinal) const
	{
	int iIndex = FindMember(sName);
	if (iIndex == -1)
		return EMemberType::None;

	SMemberDesc Member = GetMember(iIndex);
	if (retdType)
		*retdType = Member.dType;

	if (retiOrdinal)
		*retiOrdinal = Member.iOrdinal;

	return Member.iType;
	}

bool CDatatypeLiteralStruct::OnIsA (const IDatatype &Type) const
	{
	if (Type.GetCoreType() == IDatatype::STRUCT)
		return true;

	if (Type.GetCoreType() == IDatatype::ABSTRACT_DICTIONARY)
		return true;

	return false;
	}

void CDatatypeLiteralStruct::OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	DWORD dwFlags = 0;
	Stream.Write(dwFlags);

	m_dSchema.SerializeAEON(Stream, Serialized);
	}
