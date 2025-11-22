//	CDatatypeNumber.cpp
//
//	CDatatypeNumber class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_INT_32,					"Int32");

bool CDatatypeNumber::OnCanBeConstructedFrom (CDatum dType) const
	{
	const IDatatype& SrcType = dType;

	//	We always support construction from Any. This allows us to have type
	//	annotations be optional.

	if (SrcType.IsAny())
		return true;

	//	Is-a

	if (SrcType.IsA(*this))
		return true;

	//	We implicitly convert from Number

	if (SrcType.GetCoreType() == IDatatype::NUMBER)
		return true;

	//	If we're an integer, then we implicity convert from Integer

	if (!m_bFloat && SrcType.GetCoreType() == IDatatype::INTEGER)
		return true;

	//	If we're infinite precision and the source is an integer,
	//	then we can convert.

	if (m_iBits == 0 && SrcType.IsA(IDatatype::INTEGER))
		return true;

	//	If we're a float, then we accept any number

	if (m_bFloat && SrcType.IsA(IDatatype::NUMBER))
		return true;

	//	Otherwise, we don't know

	return false;
	}

bool CDatatypeNumber::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion)
	{
	SetCoreType(Stream.ReadDWORD());

	if (!CDatatypeList::Deserialize(iFormat, Stream, m_Implements))
		return false;

	Stream.Read(&m_iBits, sizeof(DWORD));

	DWORD dwFlags;
	Stream.Read(&dwFlags, sizeof(DWORD));
	m_bFloat =		((dwFlags & 0x00000001) ? true : false);
	m_bUnsigned =	((dwFlags & 0x00000002) ? true : false);

	return true;
	}

bool CDatatypeNumber::OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized)
	{
	if (!CDatatypeList::DeserializeAEON(Stream, Serialized, m_Implements))
		return false;

	m_iBits = (int)Stream.ReadDWORD();

	DWORD dwFlags = Stream.ReadDWORD();
	m_bFloat =		((dwFlags & 0x00000001) ? true : false);
	m_bUnsigned =	((dwFlags & 0x00000002) ? true : false);
	m_bSubRange =	((dwFlags & 0x00000004) ? true : false);
	m_bAbstract =	((dwFlags & 0x00000008) ? true : false);
	m_bCanBeNull =	((dwFlags & 0x00000010) ? true : false);

	if (m_bSubRange)
		{
		m_iSubRangeMin = Stream.ReadInt();
		m_iSubRangeMax = Stream.ReadInt();
		}

	return true;
	}

bool CDatatypeNumber::OnEquals (const IDatatype &Src) const
	{
	auto &Other = (const CDatatypeNumber &)Src;

	if (GetCoreType() != Other.GetCoreType())
		return false;

	if (m_Implements != Other.m_Implements)
		return false;

	if (m_iBits != Other.m_iBits)
		return false;

	if (m_bFloat != Other.m_bFloat)
		return false;

	if (m_bUnsigned != Other.m_bUnsigned)
		return false;

	if (m_bSubRange != Other.m_bSubRange)
		return false;

	if (m_bSubRange)
		{
		if (m_iSubRangeMin != Other.m_iSubRangeMin)
			return false;

		if (m_iSubRangeMax != Other.m_iSubRangeMax)
			return false;
		}

	return true;
	}

CString CDatatypeNumber::OnGetName () const
	{
	if (IsAnonymous() && m_bSubRange)
		return strPattern("%d...%d", m_iSubRangeMin, m_iSubRangeMax);
	else
		return DefaultGetName();
	}

IDatatype::SNumberDesc CDatatypeNumber::OnGetNumberDesc () const
	{
	SNumberDesc Desc;
	Desc.bNumber = true;
	Desc.iBits = m_iBits;
	Desc.bFloat = m_bFloat;
	Desc.bUnsigned = m_bUnsigned;
	Desc.bSubRange = m_bSubRange;
	Desc.iSubRangeMin = m_iSubRangeMin;
	Desc.iSubRangeMax = m_iSubRangeMax;

	return Desc;
	}

bool CDatatypeNumber::OnIsA (const IDatatype &Type) const
	{
	if (m_Implements.IsA(Type))
		return true;

	//	See if we're a subrange of the given type.

	SNumberDesc OtherDesc = Type.GetNumberDesc();
	if (OtherDesc.bNumber)
		{
	/*
		//	If we're both floats, then we must have matching bits

		if (m_bFloat && OtherDesc.bFloat)
			{
			if (m_iBits == OtherDesc.iBits)
				return true;
			else
				return false;
			}

		//	If they are a float and we're integer, then we fit as long
		//	as we are not IntIP.
		
		else if (OtherDesc.bFloat)
			{
			if (m_iBits != 0)
				return true;
			else
				return false;
			}

		//	If we're a float, then we don't fit.

		else if (m_bFloat)
			return false;

		else
	*/
		//	If we're both subranges, then we need to check the bounds.

		if (m_bSubRange && OtherDesc.bSubRange)
			{
			if (m_iSubRangeMin >= OtherDesc.iSubRangeMin
					&& m_iSubRangeMax <= OtherDesc.iSubRangeMax)
				return true;
			else
				return false;
			}

		//	If they are a subrange bit we are not, then we don't fit.
		//	(Because the smallest non-subrange integer is Int32).
		
		else if (OtherDesc.bSubRange)
			return false;

		//	If we are a subrange then we fit in any integer type.

		else if (m_bSubRange)
			return true;

		//	All integers fit in IntIP.

	/*
		else if (OtherDesc.iBits == 0)
			return true;

		//	But if we're IntIP, then we don't fit.

		else if (m_iBits == 0)
			return false;

		//	If we're the same sign, then we need to have at least as many bits.

		else if (m_bUnsigned == OtherDesc.bUnsigned)
			{
			if (m_iBits >= OtherDesc.iBits)
				return true;
			else
				return false;
			}

		//	If we're different signs, then we need to have at least one more bit.

		else if (m_bUnsigned)
			{
			if (OtherDesc.iBits > m_iBits)
				return true;
			else
				return false;
			}
		else if (OtherDesc.bUnsigned)
			{
			if (m_iBits > OtherDesc.iBits)
				return true;
			else
				return false;
			}
	*/
		}

	//	Otherwise, not a match.

	return false;
	}

void CDatatypeNumber::OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	m_Implements.SerializeAEON(Stream, Serialized);

	Stream.Write(m_iBits);

	DWORD dwFlags = 0;
	dwFlags |= (m_bFloat ?		0x00000001 : 0);
	dwFlags |= (m_bUnsigned ?	0x00000002 : 0);
	dwFlags |= (m_bSubRange ?	0x00000004 : 0);
	dwFlags |= (m_bAbstract ?	0x00000008 : 0);
	dwFlags |= (m_bCanBeNull ?	0x00000010 : 0);
	Stream.Write(dwFlags);

	if (m_bSubRange)
		{
		Stream.Write(m_iSubRangeMin);
		Stream.Write(m_iSubRangeMax);
		}
	}
