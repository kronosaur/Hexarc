//	CComplexDatatype.cpp
//
//	CComplexDatatype class
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_DATATYPE,					"datatype")

DECLARE_CONST_STRING(ERR_NOT_A_TYPE,					"Not a type: %s.");

TDatumPropertyHandler<CComplexDatatype> CComplexDatatype::m_Properties = {
	{
		"binarySize",
		"I",
		"The size of this type (in bytes) when write to a binary object with .setAt.",
		[](const CComplexDatatype &Obj, const CString &sProperty)
			{
			return CDatumInterpret::CalcSizeOf(*Obj.m_pType);
			},
		NULL,
		},
	{
		"columns",
		"$ArrayOfString",
		"Returns an array of keys.",
		[](const CComplexDatatype& Obj, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			for (int i = 0; i < Obj.m_pType->GetMemberCount(); i++)
				{
				auto MemberDesc = Obj.m_pType->GetMember(i);
				if (MemberDesc.iType == IDatatype::EMemberType::InstanceKeyVar || MemberDesc.iType == IDatatype::EMemberType::InstanceVar)
					dResult.Append(MemberDesc.sID);
				}

			return dResult;
			},
		NULL,
		},
	{
		"elementtype",
		"%",
		"For array and table types, this is the type of the element.",
		[](const CComplexDatatype &Obj, const CString &sProperty)
			{
			return Obj.m_pType->GetElementType();
			},
		NULL,
		},
	{
		"fields",
		"t",
		"A table representing the fields of the type.",
		[](const CComplexDatatype &Obj, const CString &sProperty)
			{
			return Obj.m_pType->GetFieldsAsTable();
			},
		NULL,
		},
	{
		"keytype",
		"%",
		"For array and table types, this is the type of the key.",
		[](const CComplexDatatype &Obj, const CString &sProperty)
			{
			return Obj.m_pType->GetKeyType();
			},
		NULL,
		},
	{
		"members",
		"t",
		"A table representing the members of the type.",
		[](const CComplexDatatype &Obj, const CString &sProperty)
			{
			return Obj.m_pType->GetMembersAsTable();
			},
		NULL,
		},
	{
		"name",
		"s",
		"Name of the datatype.",
		[](const CComplexDatatype &Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_pType->GetName());
			},
		NULL,
		},
	{
		"symbol",
		"s",
		"Fully-qualified name of the datatype.",
		[](const CComplexDatatype &Obj, const CString &sProperty)
			{
			return CDatum(Obj.m_pType->GetFullyQualifiedName());
			},
		NULL,
		},
	};

TDatumMethodHandler<CComplexDatatype> CComplexDatatype::m_Methods = {
	{
		"isa",
		"b:t=%",
		".isa(type) -> true/false",
		0,
		[](CComplexDatatype& Obj, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CDatum dType = LocalEnv.GetArgument(1);
			if (dType.GetBasicType() != CDatum::typeDatatype)
				{
				retResult.dResult = CDatum::CreateError(strPattern(ERR_NOT_A_TYPE, dType.AsString()));
				return false;
				}

			retResult.dResult = Obj.m_pType->IsA(dType);
			return true;
			},
		},
	};

bool CComplexDatatype::CreateFromStream (CCharStream& Stream, CDatum& retdDatum)

//	CreateFromStream
//
//	Creates a datatype from a stream.

	{
	//	Backup one character because we want the OnDeserialize call to read it.

	Stream.UnreadChar();

	CBase64Decoder Decoder(Stream.GetByteStream());

	if (!CreateFromStream(Decoder, retdDatum))
		return false;

	//	Done

	Stream.RefreshStream();
	Stream.ReadChar();
	return true;
	}

bool CComplexDatatype::CreateFromStream (IByteStream& Stream, CDatum& retdDatum)

//	CreateFromStream
//
//	Creates a datatype from a stream.

	{
	//	Read the type code

	DWORD dwType = Stream.ReadDWORD();

	//	If this is a core type, then we just need to look it up.

	if (dwType == IMPL_CORE_TYPE_ID)
		{
		CString sFullyQualifiedName = CString::Deserialize(Stream);

		retdDatum = CAEONTypeSystem::FindCoreType(sFullyQualifiedName);
		if (retdDatum.IsNil())
			{
			//	If not found then we create a datatype reference. This means
			//	that we cannot use it, but at least we can transmit it.

			retdDatum = CDatum(new CComplexDatatype(new CDatatypeUnknownCoreType(sFullyQualifiedName)));
			return true;
			}
		}

	//	Otherwise, we need to create a new datatype

	else
		{
		CComplexDatatype* pDatum;

		if (dwType == IMPL_SERIALIZATION_V2)
			{
			dwType = Stream.ReadDWORD();

			CAEONSerializedMap Serialized;
			pDatum = new CComplexDatatype(IDatatype::DeserializeAEON(Stream, dwType, Serialized));
			}
		else
			{
			pDatum = new CComplexDatatype(IDatatype::Deserialize(CDatum::EFormat::AEONScript, dwType, Stream));
			}

		if (!pDatum->m_pType)
			{
			delete pDatum;
			return false;
			}

		//	If this is a core type, then it is a previously stored core type, 
		//	so we need to look it up.

		if (pDatum->m_pType->GetCoreType())
			{
			retdDatum = CAEONTypeSystem::FindCoreType(pDatum->m_pType->GetFullyQualifiedName());
			if (!retdDatum.IsNil())
				{
				delete pDatum;
				return true;
				}
			}

		retdDatum = CDatum(pDatum);
		}

	return true;
	}

TArray<IDatatype::SMemberDesc> CComplexDatatype::GetMembers (void)
	{
	TArray<IDatatype::SMemberDesc> Members;

	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);

	return Members;
	}

const CString &CComplexDatatype::GetTypename (void) const 
	{
	return TYPENAME_DATATYPE;
	}

int CComplexDatatype::GetID (IDatatype::EImplementation iValue)
	{
	switch (iValue)
		{
		case IDatatype::EImplementation::Any:
			return IMPL_ANY_ID;

		case IDatatype::EImplementation::Array:
			return IMPL_ARRAY3_ID;

		case IDatatype::EImplementation::Class:
			return IMPL_CLASS3_ID;

		case IDatatype::EImplementation::Enum:
			return IMPL_ENUM2_ID;

		case IDatatype::EImplementation::Function:
			return IMPL_FUNCTION_ID;

		case IDatatype::EImplementation::Tensor:
			return IMPL_TENSOR_ID;

		case IDatatype::EImplementation::Nullable:
			return IMPL_NULLABLE_ID;

		case IDatatype::EImplementation::Number:
			return IMPL_NUMBER_ID;

		case IDatatype::EImplementation::Schema:
			return IMPL_SCHEMA2_ID;

		case IDatatype::EImplementation::Simple:
			return IMPL_SIMPLE_ID;

		default:
			throw CException(errFail);
		}
	}

IDatatype::EImplementation CComplexDatatype::GetImplementation (int iID, DWORD& retdwVersion)
	{
	switch (iID)
		{
		case IMPL_ANY_ID:
			retdwVersion = 1;
			return IDatatype::EImplementation::Any;

		case IMPL_ARRAY_ID:
			retdwVersion = 1;
			return IDatatype::EImplementation::Array;

		case IMPL_ARRAY2_ID:
			retdwVersion = 2;
			return IDatatype::EImplementation::Array;

		case IMPL_ARRAY3_ID:
			retdwVersion = 3;
			return IDatatype::EImplementation::Array;

		case IMPL_CLASS1_ID:
			retdwVersion = 1;
			return IDatatype::EImplementation::Class;

		case IMPL_CLASS2_ID:
			retdwVersion = 2;
			return IDatatype::EImplementation::Class;

		case IMPL_CLASS3_ID:
			retdwVersion = 3;
			return IDatatype::EImplementation::Class;

		case IMPL_ENUM1_ID:
			retdwVersion = 1;
			return IDatatype::EImplementation::Enum;

		case IMPL_ENUM2_ID:
			retdwVersion = 2;
			return IDatatype::EImplementation::Enum;

		case IMPL_FUNCTION_ID:
			retdwVersion = 1;
			return IDatatype::EImplementation::Function;

		case IMPL_MATRIX_ID:
			retdwVersion = 1;
			return IDatatype::EImplementation::Tensor;

		case IMPL_TENSOR_ID:
			retdwVersion = 2;
			return IDatatype::EImplementation::Tensor;

		case IMPL_NULLABLE_ID:
			retdwVersion = 1;
			return IDatatype::EImplementation::Nullable;

		case IMPL_NUMBER_ID:
			retdwVersion = 1;
			return IDatatype::EImplementation::Number;

		case IMPL_SCHEMA1_ID:
			retdwVersion = 1;
			return IDatatype::EImplementation::Schema;

		case IMPL_SCHEMA2_ID:
			retdwVersion = 2;
			return IDatatype::EImplementation::Schema;

		case IMPL_SIMPLE_ID:
			retdwVersion = 1;
			return IDatatype::EImplementation::Simple;

		default:
			retdwVersion = 0;
			return IDatatype::EImplementation::Unknown;
		}
	}

size_t CComplexDatatype::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns a heuristic estimate for serialization size (this helps us to 
//	allocate buffers during serialize.

	{
	return 0;
	}

bool CComplexDatatype::OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Deserialize from a stream.

	{
	DWORD dwType = Stream.ReadDWORD();

	m_pType = std::move(IDatatype::Deserialize(iFormat, dwType, Stream));
	if (!m_pType)
		return false;

	return true;
	}
		
void CComplexDatatype::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize to stream.

	{
	CAEONSerializedMap Serialized;
	Stream.Write(IMPL_SERIALIZATION_V2);
	m_pType->SerializeAEON(Stream, Serialized);
	}

CDatum CComplexDatatype::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	//	Read the type ID. If this is a core type, then we can handle it all 
	//	here.

	DWORD dwType = Stream.ReadDWORD();
	if (dwType == IMPL_CORE_TYPE_ID)
		{
		CString sFullyQualifiedName = CString::Deserialize(Stream);

		CDatum dValue = CAEONTypeSystem::FindCoreType(sFullyQualifiedName);
		if (dValue.IsNil())
			{
			//	If not found then we create a datatype reference. This means
			//	that we cannot use it, but at least we can transmit it.

			dValue = CDatum(new CComplexDatatype(new CDatatypeUnknownCoreType(sFullyQualifiedName)));
			}

		Serialized.Add(dwID, dValue);
		return dValue;
		}

	//	Otherwise, create a new object and add it to the map.

	else
		{
		CComplexDatatype *pValue = new CComplexDatatype(NULL);
		CDatum dValue(pValue);
		Serialized.Add(dwID, dValue);

		//	Read the actual type.

		pValue->m_pType = std::move(IDatatype::DeserializeAEON(Stream, dwType, Serialized));

		return dValue;
		}
	}

void CComplexDatatype::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const
	{
	switch (iFormat)
		{
		case CDatum::EFormat::GridLang:
			Stream.Write(m_pType->GetName());
			break;

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

void CComplexDatatype::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_DATATYPE))
		return;

	m_pType->SerializeAEON(Stream, Serialized);
	}
