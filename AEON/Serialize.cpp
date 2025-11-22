//	Serialize.cpp
//
//	CDatum class
//	Copyright (c) 2023 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

void CDatum::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const

//	SerializeAEON
//
//	Writes the datum to a stream in AEON format. For each datum we write out an 
//	ID. If the high bit is set, then the ID is a reference to a previously
//	serialized datum. Otherwise, the ID is followed by the datum itself.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			Stream.Write(SERIALIZE_TYPE_NULL);
			break;

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
					Stream.Write(SERIALIZE_TYPE_FALSE);
					break;

				case VALUE_TRUE:
					Stream.Write(SERIALIZE_TYPE_TRUE);
					break;

				default:
					ASSERT(false);
					break;
				}
			break;
			}

		case TYPE_INT32:
			{
			DWORD dwValue = (DWORD)DecodeInt32(m_dwData);
			if (dwValue & 0xff000000)
				{
				Stream.Write(SERIALIZE_TYPE_INT32);
				Stream.Write(dwValue);
				}
			else
				{
				Stream.Write(SERIALIZE_TYPE_INT24 | (dwValue & 0xffffff));
				}
			break;
			}

		case TYPE_ENUM:
			{
			DWORD dwDatatypeID = DecodeEnumType(m_dwData);
			CDatum dType = CAEONTypes::Get(dwDatatypeID);
			int iOrdinal = DecodeEnumValue(m_dwData);
			CString sID;

			const IDatatype& EnumType = dType;
			int iIndex = EnumType.FindMemberByOrdinal(iOrdinal);
			if (iIndex != -1)
				{
				IDatatype::SMemberDesc MemberDesc = EnumType.GetMember(iIndex);
				sID = MemberDesc.sID;
				}
			else
				sID = strFromInt(iOrdinal);

			DWORD dwType = SERIALIZE_TYPE_ENUM | sID.GetLength();
			Stream.Write(dwType);
			Stream.Write(sID);
			Stream.Write("    ", AlignUp(sID.GetLength(), (int)sizeof(DWORD)) - sID.GetLength());

			dType.SerializeAEON(Stream, Serialized);
			break;
			}

		case TYPE_STRING:
			{
			CStringView sString = DecodeString(m_dwData);
			DWORD dwLength = sString.GetLength();
			if (dwLength & 0xff000000)
				{
				Stream.Write(SERIALIZE_TYPE_LARGE_STRING);
				Stream.Write(dwLength);
				}
			else
				{
				DWORD dwType = SERIALIZE_TYPE_STRING | sString.GetLength();
				Stream.Write(dwType);
				}

			Stream.Write(sString);
			Stream.Write("    ", AlignUp(sString.GetLength(), (int)sizeof(DWORD)) - sString.GetLength());
			break;
			}

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).SerializeAEON(Stream, Serialized);

		case TYPE_ROW_REF:
			return CAEONRowRefImpl::SerializeAEON(m_dwData, Stream, Serialized);

		case TYPE_NAN:
			Stream.Write(SERIALIZE_TYPE_NAN);
			break;

		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
			Stream.Write(SERIALIZE_TYPE_NAN);
			break;

		default:
			{
			Stream.Write(SERIALIZE_TYPE_DOUBLE);
			Stream.Write(DecodeDouble(m_dwData));
			break;
			}
		}
	}

CDatum CDatum::Deserialize (IByteStream& Stream, EFormat iFormat, CAEONSerializedMap& Serialized)

//	Deserialize
//
//	Deserializes from a stream.

	{
	switch (iFormat)
		{
		case EFormat::AEONScript:
		case EFormat::AEONLocal:
			return DeserializeAEON(Stream, Serialized);
		
		default:
			throw CException(errFail);
		}
	}

CDatum CDatum::DeserializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized)
	{
	DWORD dwType = Stream.ReadDWORD();

	//	Otherwise, read the type.

	switch (dwType & SERIALIZE_TYPE_MASK)
		{
		case SERIALIZE_TYPE_NULL:
			return CDatum();

		case SERIALIZE_TYPE_FALSE:
			return CDatum(false);

		case SERIALIZE_TYPE_TRUE:
			return CDatum(true);

		case SERIALIZE_TYPE_NAN:
			return CDatum::CreateNaN();

		case SERIALIZE_TYPE_STRING:
			{
			int iLen = dwType & ~SERIALIZE_TYPE_MASK;
			CString sString(iLen);
			Stream.Read(sString.GetPointer(), iLen);
			Stream.Read(NULL, AlignUp(iLen, (int)sizeof(DWORD)) - iLen);
			return CDatum(std::move(sString));
			}

		case SERIALIZE_TYPE_LARGE_STRING:
			{
			DWORD dwLen = Stream.ReadDWORD();
			CString sString(dwLen);
			Stream.Read(sString.GetPointer(), dwLen);
			Stream.Read(NULL, AlignUp(dwLen, (DWORD)sizeof(DWORD)) - dwLen);
			return CDatum(std::move(sString));
			}

		case SERIALIZE_TYPE_INT24:
			{
			DWORD dwValue = dwType & ~SERIALIZE_TYPE_MASK;
			return CDatum((int)dwValue);
			}

		case SERIALIZE_TYPE_INT32:
			{
			DWORD dwValue = Stream.ReadDWORD();
			return CDatum((int)dwValue);
			}

		case SERIALIZE_TYPE_DOUBLE:
			{
			double rValue = Stream.ReadDouble();
			return CDatum(rValue);
			}

		case SERIALIZE_TYPE_ENUM:
			{
			int iLen = dwType & ~SERIALIZE_TYPE_MASK;
			CString sID(iLen);
			Stream.Read(sID.GetPointer(), iLen);
			Stream.Read(NULL, AlignUp(iLen, (int)sizeof(DWORD)) - iLen);

			CDatum dType = DeserializeAEON(Stream, Serialized);
			const IDatatype& EnumType = dType;

			int iIndex = EnumType.FindMember(sID);
			if (iIndex == -1)
				return CDatum();

			IDatatype::SMemberDesc MemberDesc = EnumType.GetMember(iIndex);
			return CDatum::CreateEnum(MemberDesc.iOrdinal, dType);
			}

		case SERIALIZE_TYPE_REF:
			{
			DWORD dwID = dwType & ~SERIALIZE_TYPE_MASK;
			return Serialized.Get(dwID);
			}

		case SERIALIZE_TYPE_ARRAY:
			return CComplexArray::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_INTIP:
			return CComplexInteger::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_ANNOTATED:
			return CAEONAnnotated::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_BINARY:
			return CComplexBinary::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_BINARY_FILE:
			return CComplexBinaryFile::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_DATATYPE:
			return CComplexDatatype::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_DICTIONARY:
			return CAEONDictionary::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_VECTOR_TYPED:
			return CAEONVectorTyped::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_ERROR:
			return CAEONError::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_EXPRESSION:
			return CAEONExpression::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_IMAGE32:
			return CComplexImage32::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_OBJECT:
			return CAEONObject::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_TABLE:
			return CAEONTable::DeserializeAEON_v1(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_TABLE_V2:
			return CAEONTable::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_RANGE:
			return CAEONRange::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_STRUCT:
			return CComplexStruct::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_TENSOR:
			return CAEONTensor::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_VECTOR_FLOAT64:
			return CAEONVectorFloat64::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_VECTOR_INT32:
			return CAEONVectorInt32::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_VECTOR_INTIP:
			return CAEONVectorIntIP::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_VECTOR_NUMBER:
			return CAEONVectorNumber::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_VECTOR_STRING:
			return CAEONVectorString::DeserializeAEON_v1(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_VECTOR_STRING_V2:
			return CAEONVectorString::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_DATE_TIME:
			return CComplexDateTime::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_TIME_SPAN:
			return CAEONTimeSpan::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_VECTOR_2D:
			return CAEONVector2D::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_VECTOR_3D:
			return CAEONVector3D::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_TEXT_LINES:
			return CAEONLines::DeserializeAEON(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		case SERIALIZE_TYPE_EXTERNAL:
			return IComplexDatum::DeserializeAEONAsExternal(Stream, dwType & ~SERIALIZE_TYPE_MASK, Serialized);

		default:
			throw CException(errFail);
		}
	}
