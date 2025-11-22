//	CComplexBinary.cpp
//
//	CComplexBinary class
//	Copyright (c) 2012 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_BINARY,				"binary");

DECLARE_CONST_STRING(ERR_INVALID_OPTIONS,			"Invalid options");

TDatumPropertyHandler<CComplexBinary> CComplexBinary::m_Properties = {
	{
		"length",
		"i",
		"Returns the length of the binary value in bytes.",
		[](const CComplexBinary& Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetLength());
			},
		NULL,
		},
	};

TDatumMethodHandler<CComplexBinary> CComplexBinary::m_Methods = {
	{
		"getAs",
		"%1:type=%|type=%,offset=n|type=%,offset=n,options=?",
		".getAs(type, offset, options) -> value.",
		0,
		[](CComplexBinary& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CDatum dType = LocalEnv.GetArgument(1);
			int iOffset = (int)LocalEnv.GetArgument(2);
			CDatum dOptions = LocalEnv.GetArgument(3);
			CDatumInterpret::SOptions Options;
			if (!CDatumInterpret::ParseOptions(dOptions, Options))
				{
				retResult.dResult = CDatum::CreateError(ERR_INVALID_OPTIONS);
				return false;
				}

			const BYTE* pPos = (const BYTE*)Obj.m_pData;
			const BYTE* pEndPos = (iOffset < 0 ? NULL : pPos + Obj.GetLength());
			CString sError;
			if (!CDatumInterpret::InterpretAs(pPos + Max(0, iOffset), pEndPos, dType, Options, retResult.dResult, &pPos, &sError))
				{
				retResult.dResult = CDatum::CreateError(sError);
				return false;
				}

			return true;
			},
		},
	{
		"getStringAt",
		"s:offset=n|offset=n,length=n",
		".getStringAt(offset, options) -> value.",
		0,
		[](CComplexBinary& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			int iOffset = (int)LocalEnv.GetArgument(1);
			CDatum dLength = LocalEnv.GetArgument(2);
			int iLength = (dLength.IsNil() ? Obj.GetLength() - iOffset : (int)dLength);

			retResult.dResult = CDatum(Obj.ReadAt(iOffset, iLength));
			return true;
			},
		},
	{
		"hashed",
		"v:|options=?",
		".hashed() -> BLAKE2 hash.",
		0,
		[](CComplexBinary& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CBuffer Buffer(Obj.m_pData, Obj.GetLength(), false);
			retResult.dResult = CDatum::CreateBinary(cryptoBLAKE2(Buffer));
			return true;
			},
		},
	{
		"hex",
		"v:|options=?",
		".hex() -> hex string.",
		0,
		[](CComplexBinary& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(CString::EncodeHex((const BYTE*)Obj.m_pData, Obj.GetLength()));
			return true;
			},
		},
	{
		"setAs",
		"?:type=%,offset=n,value=?|type=%,offset=n,value=?,options=?",
		".setAs(type, offset, value, options) -> true/error",
		0,
		[](CComplexBinary& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CDatum dType = LocalEnv.GetArgument(1);
			int iOffset = (int)LocalEnv.GetArgument(2);
			CDatum dValue = LocalEnv.GetArgument(3);
			CDatum dOptions = LocalEnv.GetArgument(4);
			CDatumInterpret::SOptions Options;
			if (!CDatumInterpret::ParseOptions(dOptions, Options))
				{
				retResult.dResult = CDatum::CreateError(ERR_INVALID_OPTIONS);
				return false;
				}

			BYTE* pPos = (BYTE*)Obj.m_pData;
			BYTE* pEndPos = (iOffset < 0 ? NULL : pPos + Obj.GetLength());
			CString sError;
			if (!CDatumInterpret::EncodeAs(pPos + Max(0, iOffset), pEndPos, dType, dValue, Options, &pPos, &sError))
				{
				retResult.dResult = CDatum::CreateError(sError);
				return false;
				}

			return true;
			},
		},
	{
		"setStringAt",
		"?:offset=n,value=?",
		".setStringAt(offset, value) -> length-written/error",
		0,
		[](CComplexBinary& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			int iOffset = (int)LocalEnv.GetArgument(1);
			CDatum dValue = LocalEnv.GetArgument(2);

			int iWritten;
			if (dValue.GetBasicType() == CDatum::typeString || dValue.GetBasicType() == CDatum::typeBinary)
				{
				CStringView sValue = dValue;
				iWritten = Obj.WriteAt(iOffset, (const BYTE*)sValue.GetPointer(), sValue.GetLength());
				}
			else
				{
				CString sValue = dValue.AsString();
				iWritten = Obj.WriteAt(iOffset, (const BYTE*)sValue.GetPointer(), sValue.GetLength());
				}

			if (iWritten < 0)
				{
				retResult.dResult = CDatum::CreateError(strPattern("Unable to write at offset %d", iOffset));
				return false;
				}

			retResult.dResult = iWritten;
			return true;
			},
		},
	/*
	{
		"toString",
		"s:|format=?",
		".toString(format) -> string.",
		0,
		[](CComplexBinary& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringFormat Format(LocalEnv.GetArgument(1).AsStringView());
			CString sResult;
			if (!Format.FormatBinary((const BYTE*)Obj.m_pData, Obj.GetLength(), sResult))
				{
				retResult.dResult = CDatum::CreateError(sResult);
				return false;
				}

			retResult.dResult = CDatum(std::move(sResult));
			return true;
			},
		},
	*/
	};

CComplexBinary::CComplexBinary (CString&& sData)

//	CComplexBinary constructor

	{
	if (sData.IsEmpty())
		m_pData = NULL;
	else
		{
		if (sData.IsLiteral())
			{
			CString sCopy(sData.GetParsePointer(), sData.GetLength());
			m_pData = sCopy.Handoff();
			}
		else
			m_pData = sData.Handoff();
		}
	}

CComplexBinary::CComplexBinary (const CBuffer64& Buffer)

//	CComplexBinary constructor

	{
	if (Buffer.GetLength() >= (DWORDLONG)MAXINT32)
		throw CException(errFail);

	int iLength = (int)Buffer.GetLength();

	//	Sadly we need to make a copy because CComplexBinary uses a buffer that
	//	is compatible with CString.

	if (iLength == 0)
		m_pData = NULL;
	else
		{
		char *pPos = new char [sizeof(DWORD) + iLength + 1];

		*(DWORD *)pPos = iLength;
		pPos += sizeof(DWORD);

		m_pData = pPos;

		utlMemCopy(Buffer.GetPointer(), pPos, iLength);
		pPos += iLength;

		*pPos = '\0';
		}
	}

CComplexBinary::CComplexBinary (IByteStream &Stream, int iLength)

//	CComplexBinary constructor

	{
	ASSERT(iLength >= 0);

	if (iLength == 0)
		m_pData = NULL;
	else
		{
		char *pPos = new char [sizeof(DWORD) + iLength + 1];

		*(DWORD *)pPos = iLength;
		pPos += sizeof(DWORD);

		m_pData = pPos;

		Stream.Read(pPos, iLength);
		pPos += iLength;

		*pPos = '\0';
		}
	}

CComplexBinary::CComplexBinary (int iLength)

//	CComplexBinary constructor

	{
	ASSERT(iLength >= 0);

	if (iLength == 0)
		m_pData = NULL;
	else
		{
		char *pPos = new char [sizeof(DWORD) + iLength + 1];

		*(DWORD *)pPos = iLength;
		pPos += sizeof(DWORD);

		m_pData = pPos;
		utlMemSet(pPos, iLength, 0);
		pPos += iLength;

		*pPos = '\0';
		}
	}

CComplexBinary::~CComplexBinary (void)

//	CComplexBinary destructor

	{
	if (m_pData)
		delete [] GetBuffer();
	}

TArray<IDatatype::SMemberDesc> CComplexBinary::GetMembers (void)

//	GetMembers
//
//	Returns a list of members.

	{
	TArray<IDatatype::SMemberDesc> Members;

	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);

	return Members;
	}

const CString &CComplexBinary::GetTypename (void) const { return TYPENAME_BINARY; }

void CComplexBinary::Append (CDatum dDatum)

//	Append
//
//	Appends data

	{
	CStringView sNewData = dDatum;
	if (sNewData.GetLength() == 0)
		return;

	//	Compute the new length

	int iOldLen = GetLength();
	int iNewLen = iOldLen + sNewData.GetLength();

	//	Allocate a new buffer

	char *pNewBuffer = new char [sizeof(DWORD) + iNewLen + 1];
	char *pPos = pNewBuffer;
	*(DWORD *)pPos = iNewLen;
	pPos += sizeof(DWORD);

	//	Copy the original data

	if (iOldLen)
		{
		utlMemCopy(m_pData, pPos, iOldLen);
		pPos += iOldLen;
		}

	//	Copy the new data

	utlMemCopy(sNewData.GetParsePointer(), pPos, sNewData.GetLength());
	pPos += sNewData.GetLength();

	//	NULL-terminator

	*pPos++ = '\0';

	//	Free our original buffer and swap

	if (m_pData)
		delete [] GetBuffer();

	m_pData = pNewBuffer + sizeof(DWORD);
	}

CString CComplexBinary::AsString (void) const

//	AsString
//
//	Returns the datum as a string

	{
	if (m_pData == NULL)
		return NULL_STR;

	CStringBuffer Output;
	CBase64Encoder Encoder(&Output);
	Encoder.Write(m_pData, GetLength());
	Encoder.Close();

	return Output;
	}

CStringView CComplexBinary::CastCString (void) const

//	CastCString
//
//	Casts to a string

	{
	if (m_pData == NULL)
		return CStringView();
	else
		return CStringView::FromCStringPtr(m_pData);
	}

IComplexDatum *CComplexBinary::Clone (CDatum::EClone iMode) const

//	Clone
//
//	Creates a clone

	{
	CComplexBinary *pDest = new CComplexBinary;
	if (m_pData)
		{
		char *pPos = new char [sizeof(DWORD) + GetLength() + 1];

		*(DWORD *)pPos = GetLength();
		pPos += sizeof(DWORD);

		pDest->m_pData = pPos;

		utlMemCopy(m_pData, pPos, GetLength());
		pPos += GetLength();

		*pPos = '\0';
		}

	return pDest;
	}

CString CComplexBinary::Format (const CStringFormat& Format) const

//	Format
//
//	Formats as a string.

	{
	CString sResult;
	if (!Format.FormatBinary((const BYTE*)m_pData, GetLength(), sResult))
		return sResult;
	else
		return sResult;
	}

size_t CComplexBinary::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns an approximation of serialization size.

	{
	return GetLength();
	}

bool CComplexBinary::OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Deserialize

	{
	if (m_pData)
		{
		delete [] GetBuffer();
		m_pData = NULL;
		}

	DWORD dwLength;
	Stream.Read(&dwLength, sizeof(DWORD));

	if (dwLength > 0)
		{
		CComplexBinary Temp(Stream, dwLength);
		m_pData = Temp.m_pData;
		Temp.m_pData = NULL;
		}

	return true;
	}

void CComplexBinary::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize

	{
	DWORD dwLength = GetLength();

	Stream.Write(&dwLength, sizeof(DWORD));
	if (dwLength)
		Stream.Write(m_pData, dwLength);
	}

CDatum CComplexBinary::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	DWORD dwLength = Stream.ReadDWORD();

	CDatum dValue = CDatum(new CComplexBinary(Stream, dwLength));
	Stream.Read(NULL, AlignUp((int)dwLength, (int)sizeof(DWORD)) - (int)dwLength);

	return dValue;
	}

CString CComplexBinary::ReadAt (int iOffset, int iLength) const

//	ReadAt
//
//	Reads as much as will fit.

	{
	if (iOffset < 0 || iLength < 0)
		return CString();

	//	Make sure we're in bounds.

	if (iOffset >= GetLength())
		return CString();

	//	Compute how much we can read

	int iRead = Min(iLength, GetLength() - iOffset);
	if (iRead == 0)
		return CString();

	//	Allocate a string

	CString sResult(iRead);

	//	Read

	utlMemCopy(m_pData + iOffset, sResult.GetPointer(), iRead);
	return sResult;
	}

void CComplexBinary::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	Stream.Write(CDatum::SERIALIZE_TYPE_BINARY);

	Stream.Write(GetLength());
	Stream.Write(m_pData, GetLength());
	Stream.Write("    ", AlignUp(GetLength(), (int)sizeof(DWORD)) - GetLength());
	}

void CComplexBinary::TakeHandoff (CStringBuffer &Buffer)

//	TakeHandoff
//
//	Takes ownership

	{
	if (m_pData)
		delete [] GetBuffer();

	m_pData = Buffer.Handoff();
	}

int CComplexBinary::WriteAt (int iOffset, const BYTE* pData, int iLength)

//	WriteAt
//
//	Writes as much as will fit (without resizing the buffer).

	{
	if (iOffset < 0 || (!pData && iLength > 0) || iLength < 0)
		return -1;

	//	Make sure we're in bounds.

	if (iOffset > GetLength())
		return 0;

	//	Compute how much we can write

	int iWrite = Min(iLength, GetLength() - iOffset);
	if (iWrite == 0)
		return 0;

	//	Write

	utlMemCopy(pData, m_pData + iOffset, iWrite);
	return iWrite;
	}
