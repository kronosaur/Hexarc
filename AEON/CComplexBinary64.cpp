//	CComplexBinary64.cpp
//
//	CComplexBinary64 class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_BINARY64,				"binary64")
const CString &CComplexBinary64::GetTypename (void) const { return TYPENAME_BINARY64; }

TDatumPropertyHandler<CComplexBinary64> CComplexBinary64::m_Properties = {
	{
		"length",
		"i",
		"Returns the length of the binary value in bytes.",
		[](const CComplexBinary64& Obj, const CString &sProperty)
			{
			return CDatum(CIPInteger(Obj.GetLength()));
			},
		NULL,
		},
	};

TDatumMethodHandler<CComplexBinary64> CComplexBinary64::m_Methods = {
	{
		"hashed",
		"v:|options=?",
		".hashed() -> BLAKE2 hash.",
		0,
		[](CComplexBinary64& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			//	LATER
			retResult.dResult = CDatum();
			return true;
			},
		},
	};

CComplexBinary64::CComplexBinary64 (IByteStream64& Stream, DWORDLONG dwLength)

//	CComplexBinary64 constructor

	{
	m_Data.SetLength(dwLength);
	Stream.Read(m_Data.GetPointer(), dwLength);
	}

CComplexBinary64::~CComplexBinary64 (void)

//	CComplexBinary64 destructor

	{
	}

TArray<IDatatype::SMemberDesc> CComplexBinary64::GetMembers (void)

//	GetMembers
//
//	Returns a list of members.

	{
	TArray<IDatatype::SMemberDesc> Members;

	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);

	return Members;
	}

void CComplexBinary64::Append (CDatum dDatum)

//	Append
//
//	Appends data

	{
	CStringView sNewData = dDatum;
	if (sNewData.GetLength() == 0)
		return;

	//	Append to the end

	DWORDLONG dwOriginalPos = m_Data.GetPos();
	m_Data.Seek(0, true);
	m_Data.Write(sNewData.GetParsePointer(), sNewData.GetLength());
	m_Data.Seek(dwOriginalPos, false);
	}

CString CComplexBinary64::AsString (void) const

//	AsString
//
//	Returns the datum as a string

	{
	CStringBuffer Output;
	CBase64Encoder Encoder(&Output);
	Encoder.Write(m_Data.GetPointer(), GetBinarySize());
	Encoder.Close();

	return Output;
	}

IComplexDatum *CComplexBinary64::Clone (CDatum::EClone iMode) const

//	Clone
//
//	Creates a clone

	{
	CComplexBinary64 *pDest = new CComplexBinary64;
	pDest->m_Data = m_Data;
	return pDest;
	}

size_t CComplexBinary64::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns an approximation of serialization size.

	{
	return GetLength();
	}

bool CComplexBinary64::OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Deserialize

	{
	//	LATER
	return false;
	}

void CComplexBinary64::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize

	{
	//	LATER	
	}

CDatum CComplexBinary64::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	//	LATER
	return CDatum();
	}

void CComplexBinary64::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	}

void CComplexBinary64::TakeHandoff (CBuffer64& Buffer)

//	TakeHandoff
//
//	Takes ownership

	{
	m_Data = std::move(Buffer);
	}
