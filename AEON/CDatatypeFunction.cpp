//	CDatatypeFunction.cpp
//
//	CDatatypeFunction class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_NO_MATCHING_SIGNATURES,		"No matching signatures for argument types: %s");
DECLARE_CONST_STRING(ERR_TOO_FEW_ARGS,					"Must be called with at least %d arg%p, but called with %d.");
DECLARE_CONST_STRING(ERR_TOO_MANY_ARGS,					"Must be called with no more than %d arg%p, but called with %d.");
DECLARE_CONST_STRING(ERR_TOO_MANY_ARGS_0,				"Must be called with no arguments, but called with %d.");

CDatatypeFunction::CDatatypeFunction (const SCreate& Create) : IDatatype(Create.sFullyQualifiedName)

//	CDatatypeFunction constructor

	{
	//	Set the return type. We can only have one return type, regardless of
	//	the number of signatures.

	if (Create.Return.iType == EReturnDescType::Type && Create.Return.dType.GetBasicType() != CDatum::typeDatatype)
		throw CException(errFail);

	//	Parse all signatures.

	for (int i = 0; i < Create.Args.GetCount(); i++)
		{
		const auto& Arg = Create.Args[i];

		//	We expect signatures to be in order and contiguous.

		if (Arg.iSignature < 0 || Arg.iSignature > m_Signatures.GetCount())
			{
			//	If the first signature is 1, then it means that we have a null signature.

			if (Arg.iSignature == 1 && m_Signatures.GetCount() == 0)
				{
				//	Insert a signature with no arguments.
				m_Signatures.Insert();
				m_Signatures[0].Return = Create.Return;
				}
			else
				throw CException(errFail);
			}

		SSigDesc* pSignature = NULL;
		if (Arg.iSignature == m_Signatures.GetCount())
			{
			pSignature = m_Signatures.Insert();
			pSignature->Return = Create.Return;
			}
		else
			pSignature = &m_Signatures[Arg.iSignature];

		//	Add the argument

		if (Arg.dType.GetBasicType() != CDatum::typeDatatype)
			throw CException(errFail);

		//	If the argument name is blank then we expect this to be the return 
		//	variable.

		if (Arg.sID.IsEmpty())
			{
			if (pSignature->Args.GetCount() > 0)
				throw CException(errFail);

			pSignature->Return.iType = EReturnDescType::Type;
			pSignature->Return.dType = Arg.dType;
			}
		else
			{
			//	Make sure the argument is not already defined

			for (int j = 0; j < pSignature->Args.GetCount(); j++)
				if (strEqualsNoCase(Arg.sID, pSignature->Args[j].sID))
					throw CException(errFail);

			//	Var args must be last.

			if (pSignature->Args.GetCount() > 0 && pSignature->Args[pSignature->Args.GetCount() - 1].bVarArg)
				throw CException(errFail);

			//	Add it.

			pSignature->Args.Insert(Arg);
			}
		}

	//	If no signatures, then we insert one with no arguments.

	if (m_Signatures.GetCount() == 0)
		m_Signatures.Insert({ Create.Return, TArray<SArgDesc>() });
	}

CDatum CDatatypeFunction::CalcReturnType (const SSigDesc& Signature, CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes)
	{
	switch (Signature.Return.iType)
		{
		case EReturnDescType::Type:
			return Signature.Return.dType;

		case EReturnDescType::ArgType:
			if (Signature.Return.iFromArg == 0)
				return dThisType;
			else if (Signature.Return.iFromArg >= 1 && Signature.Return.iFromArg <= ArgTypes.GetCount())
				return ArgTypes[Signature.Return.iFromArg - 1];
			else
				throw CException(errFail);
			break;

		case EReturnDescType::ArgLiteral:
			if (Signature.Return.iFromArg == 0)
				throw CException(errFail);
			else if (Signature.Return.iFromArg >= 1 && Signature.Return.iFromArg <= ArgLiteralTypes.GetCount())
				{
				CDatum dValue = ArgLiteralTypes[Signature.Return.iFromArg - 1];
				if (dValue.GetBasicType() != CDatum::typeDatatype)
					return CAEONTypes::Get(IDatatype::ANY);
				else
					return dValue;
				}
			else
				throw CException(errFail);
			break;

		default:
			throw CException(errFail);
		}
	}

bool CDatatypeFunction::CalcSignatureMatch (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, bool bCoerce, int* retiSignature) const

//	CalcSignatureMatch
//
//	Looks for a signature that matches the given set of arguments. If bCoerce is
//	TRUE, then we allow coercion of the argument types to match the signature.
//
//	If we find a match, then we return TRUE and set *retiSignature to the index of
//	the signature.

	{
	for (int i = 0; i < m_Signatures.GetCount(); i++)
		{
		const auto& Signature = m_Signatures[i];

		//	If this signature supports var args, then we do a different check.

		if (Signature.Args.GetCount() > 0 && Signature.Args[Signature.Args.GetCount() - 1].bVarArg)
			{
			//	Check the arguments before the var arg.

			int iRequired = Signature.Args.GetCount() - 1;
			if (ArgTypes.GetCount() < iRequired)
				continue;

			bool bMatch = true;
			for (int j = 0; j < iRequired; j++)
				{
				const IDatatype& RequiredType = (const IDatatype&)Signature.Args[j].dType;
				const IDatatype& ArgType = (const IDatatype&)ArgTypes[j];

				if (bCoerce)
					{
					if (!RequiredType.CanBeConstructedFrom(ArgTypes[j]))
						{
						bMatch = false;
						break;
						}
					}
				else if (!ArgType.IsA(RequiredType))
					{
					bMatch = false;
					break;
					}
				}

			//	If we don't match, then we continue checking.

			if (!bMatch)
				continue;

			//	Otherwise, check the var args.

			const IDatatype& RequiredType = (const IDatatype&)Signature.Args[iRequired].dType;
			for (int j = iRequired; j < ArgTypes.GetCount(); j++)
				{
				const IDatatype& ArgType = (const IDatatype&)ArgTypes[j];

				if (bCoerce)
					{
					if (!RequiredType.CanBeConstructedFrom(ArgTypes[j]))
						{
						bMatch = false;
						break;
						}
					}
				else if (!ArgType.IsA(RequiredType))
					{
					bMatch = false;
					break;
					}
				}

			if (!bMatch)
				continue;
			}

		//	Otherwise, we check all args.

		else
			{
			if (Signature.Args.GetCount() != ArgTypes.GetCount())
				continue;

			//	Make sure all the types match.

			bool bMatch = true;
			for (int j = 0; j < ArgTypes.GetCount(); j++)
				{
				const IDatatype& RequiredType = (const IDatatype&)Signature.Args[j].dType;
				const IDatatype& ArgType = (const IDatatype&)ArgTypes[j];

				if (bCoerce)
					{
					if (!RequiredType.CanBeConstructedFrom(ArgTypes[j]))
						{
						bMatch = false;
						break;
						}
					}
				else if (!ArgType.IsA(RequiredType))
					{
					bMatch = false;
					break;
					}
				}

			if (!bMatch)
				continue;
			}

		//	If we made it this far, then we match

		if (retiSignature)
			*retiSignature = i;

		return true;
		}

	//	Otherwise, no match.

	return false;
	}

bool CDatatypeFunction::OnCanBeCalledWith (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, CDatum* retdReturnType, CString* retsError) const

//	OnCanBeCalledWith
//
//	Returns TRUE if the given set of arguments is a valid signature.

	{
	//	Look for a signature that matches exactly first.

	int iSignature = -1;
	if (!CalcSignatureMatch(dThisType, ArgTypes, ArgLiteralTypes, false, &iSignature))
		{
		//	If we don't find a match, then we try coercion.

		if (!CalcSignatureMatch(dThisType, ArgTypes, ArgLiteralTypes, true, &iSignature))
			{
			//	If no match do a little bit more work to figure out a good error
			//	message.

			if (retsError)
				{
				//	Calculate min and max args.

				int iMinArgs = -1;
				int iMaxArgs = 0;

				for (int i = 0; i < m_Signatures.GetCount(); i++)
					{
					const auto& Signature = m_Signatures[i];

					if (Signature.Args.GetCount() > 0 && Signature.Args[Signature.Args.GetCount() - 1].bVarArg)
						{
						if (iMinArgs == -1 || Signature.Args.GetCount() - 1 < iMinArgs)
							iMinArgs = Signature.Args.GetCount() - 1;

						iMaxArgs = -1;
						}
					else
						{
						if (iMinArgs == -1 || Signature.Args.GetCount() < iMinArgs)
							iMinArgs = Signature.Args.GetCount();

						if (iMaxArgs != -1 && Signature.Args.GetCount() > iMaxArgs)
							iMaxArgs = Signature.Args.GetCount();
						}
					}

				//	If we have too few arguments, then say so.

				if (ArgTypes.GetCount() < iMinArgs)
					{
					*retsError = strPattern(ERR_TOO_FEW_ARGS, iMinArgs, ArgTypes.GetCount());
					}

				//	If we have too many arguments, then say so.

				else if (iMaxArgs != -1 && ArgTypes.GetCount() > iMaxArgs)
					{
					if (iMaxArgs == 0)
						*retsError = strPattern(ERR_TOO_MANY_ARGS_0, ArgTypes.GetCount());
					else
						*retsError = strPattern(ERR_TOO_MANY_ARGS, iMaxArgs, ArgTypes.GetCount());
					}

				//	Otherwise, types must match

				else
					{
					//	Compose a string of the input argument types.

					CString sArgTypes;
					for (int i = 0; i < ArgTypes.GetCount(); i++)
						{
						CString sType = ((const IDatatype&)ArgTypes[i]).GetName();
						if (i == 0)
							sArgTypes = sType;
						else
							sArgTypes = strPattern("%s, %s", sArgTypes, sType);
						}

					*retsError = strPattern(ERR_NO_MATCHING_SIGNATURES, sArgTypes);
					}
				}

			return false;
			}
		}

	//	If we get this far, we found a signature.

	if (retdReturnType)
		*retdReturnType = CalcReturnType(m_Signatures[iSignature], dThisType, ArgTypes, ArgLiteralTypes);

	return true;
	}

bool CDatatypeFunction::OnCanBeCalledWithArgCount (CDatum dThisType, int iArgCount, CDatum* retdReturnType, CString* retsError) const

//	OnCanBeCalledWith
//
//	Returns TRUE if the given set of arguments is a valid signature.

	{
	for (int i = 0; i < m_Signatures.GetCount(); i++)
		{
		const auto& Signature = m_Signatures[i];

		//	If this signature supports var args, then we do a different check.

		if (Signature.Args.GetCount() > 0 && Signature.Args[Signature.Args.GetCount() - 1].bVarArg)
			{
			//	Check the arguments before the var arg.

			int iRequired = Signature.Args.GetCount() - 1;
			if (iArgCount < iRequired)
				continue;
			}

		//	Otherwise, we check all args.

		else
			{
			if (Signature.Args.GetCount() != iArgCount)
				continue;
			}

		//	If we made it this far, then we match

		if (retdReturnType)
			{
			TArray<CDatum> ArgTypes;
			for (int j = 0; j < iArgCount; j++)
				ArgTypes.Insert(Signature.Args[j].dType);

			*retdReturnType = CalcReturnType(Signature, dThisType, ArgTypes, TArray<CDatum>());
			}

		return true;
		}

	if (retsError)
		*retsError = strPattern(ERR_NO_MATCHING_SIGNATURES, NULL_STR);

	return false;
	}

bool CDatatypeFunction::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion)

//	OnDeserialize
//
//	Deserialize.

	{
	//	Read the signatures

	m_Signatures.DeleteAll();
	DWORD dwCount = Stream.ReadDWORD();
	for (int i = 0; i < (int)dwCount; i++)
		{
		SSigDesc* pSignature = m_Signatures.Insert();

		pSignature->Return.iType = EReturnDescType::Type;
		if (!CComplexDatatype::CreateFromStream(Stream, pSignature->Return.dType))
			return false;

		DWORD dwArgCount = Stream.ReadDWORD();
		pSignature->Args.InsertEmpty(dwArgCount);

		DWORD dwFlags = Stream.ReadDWORD();
		bool bVarArg = ((dwFlags & 0x00000001) ? true : false);
		if (bVarArg && pSignature->Args.GetCount() > 0)
			pSignature->Args[pSignature->Args.GetCount() - 1].bVarArg = true;

		for (int j = 0; j < (int)dwArgCount; j++)
			{
			pSignature->Args[j].iSignature = i;
			pSignature->Args[j].sID = CString::Deserialize(Stream);
			pSignature->Args[j].sDesc = CString::Deserialize(Stream);

			if (!CComplexDatatype::CreateFromStream(Stream, pSignature->Args[j].dType))
				return false;
			}
		}

	return true;
	}

bool CDatatypeFunction::OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized)
	{
	m_Signatures.DeleteAll();
	DWORD dwCount = Stream.ReadDWORD();

	for (int i = 0; i < (int)dwCount; i++)
		{
		SSigDesc* pSignature = m_Signatures.Insert();

		//	Return type

		pSignature->Return.dType = CDatum::DeserializeAEON(Stream, Serialized);

		//	Flags

		DWORD dwFlags = Stream.ReadDWORD();
		bool bVarArg =			((dwFlags & 0x00000001) ? true : false);
		bool bVarReturnType =	((dwFlags & 0x00000002) ? true : false);
		bool bVarReturnLit =	((dwFlags & 0x00000004) ? true : false);

		if (bVarReturnType)
			{
			pSignature->Return.iType = EReturnDescType::ArgType;
			pSignature->Return.iFromArg = Stream.ReadInt();
			}
		else if (bVarReturnLit)
			{
			pSignature->Return.iType = EReturnDescType::ArgLiteral;
			pSignature->Return.iFromArg = Stream.ReadInt();
			}
		else
			pSignature->Return.iType = EReturnDescType::Type;

		//	Arguments

		DWORD dwArgCount = Stream.ReadDWORD();
		pSignature->Args.InsertEmpty(dwArgCount);

		for (int j = 0; j < (int)dwArgCount; j++)
			{
			pSignature->Args[j].iSignature = i;
			pSignature->Args[j].sID = CString::Deserialize(Stream);
			pSignature->Args[j].sDesc = CString::Deserialize(Stream);
			pSignature->Args[j].dType = CDatum::DeserializeAEON(Stream, Serialized);
			}

		if (bVarArg && pSignature->Args.GetCount() > 0)
			pSignature->Args[pSignature->Args.GetCount() - 1].bVarArg = true;
		}

	return true;
	}

bool CDatatypeFunction::OnEquals (const IDatatype &Src) const

//	OnEquals
//
//	Returns TRUE if we are equal to the given type.

	{
	CRecursionSmartLock Lock(m_rs);
	if (Lock.InRecursion())
		return true;

	auto &Other = (const CDatatypeFunction&)Src;

	if (GetCoreType() != Other.GetCoreType())
		return false;

	if (m_Signatures.GetCount() != Other.m_Signatures.GetCount())
		return false;

	for (int i = 0; i < m_Signatures.GetCount(); i++)
		{
		if ((const IDatatype&)m_Signatures[i].Return.dType != (const IDatatype &)Other.m_Signatures[i].Return.dType)
			return false;

		if (m_Signatures[i].Return.iType != Other.m_Signatures[i].Return.iType)
			return false;

		if (m_Signatures[i].Return.iFromArg != Other.m_Signatures[i].Return.iFromArg)
			return false;

		if (m_Signatures[i].Args.GetCount() != Other.m_Signatures[i].Args.GetCount())
			return false;

		for (int j = 0; j < m_Signatures[i].Args.GetCount(); j++)
			{
			if (m_Signatures[i].Args[j].iSignature != Other.m_Signatures[i].Args[j].iSignature)
				return false;

			if (!strEqualsNoCase(m_Signatures[i].Args[j].sID, Other.m_Signatures[i].Args[j].sID))
				return false;

			if ((const IDatatype&)m_Signatures[i].Args[j].dType != (const IDatatype&)Other.m_Signatures[i].Args[j].dType)
				return false;

			if (m_Signatures[i].Args[j].bVarArg != Other.m_Signatures[i].Args[j].bVarArg)
				return false;
			}
		}

	return true;
	}

int CDatatypeFunction::OnFindMember (CStringView sName) const

//	OnFindMember
//
//	Returns the index of the given member (or -1 if not found).

	{
	return -1;
	}

IDatatype::SMemberDesc CDatatypeFunction::OnGetMember (int iIndex) const

//	OnGetMember
//
//	Returns a member by index.

	{
	//	Find the signature that this index is in.

	for (int i = 0; i < m_Signatures.GetCount(); i++)
		{
		const auto& Signature = m_Signatures[i];

		//	Return type is always first.

		if (iIndex == 0)
			{
			SMemberDesc Member;
			Member.iType = IDatatype::EMemberType::ReturnType;
			Member.sID = NULL_STR;
			Member.dType = Signature.Return.dType;
			Member.iOrdinal = 0;
			Member.sLabel = NULL_STR;
			Member.sFormat = NULL_STR;

			return Member;
			}

		iIndex--;
		if (iIndex < Signature.Args.GetCount())
			{
			SMemberDesc Member;
			Member.iType = IDatatype::EMemberType::ArgType;
			Member.sID = Signature.Args[iIndex].sID;
			Member.dType = Signature.Args[iIndex].dType;
			Member.iOrdinal = iIndex;
			Member.sLabel = Signature.Args[iIndex].sDesc;
			Member.sFormat = NULL_STR;

			return Member;
			}

		iIndex -= Signature.Args.GetCount();
		}

	//	If we get this far, then it's a bug.

	throw CException(errFail);
	}

int CDatatypeFunction::OnGetMemberCount () const

//	OnGetMemberCount
//
//	Returns the number of members.

	{
	int iCount = 0;

	//	Add one member for each argument in all signatures.

	for (int i = 0; i < m_Signatures.GetCount(); i++)
		iCount += 1 + m_Signatures[i].Args.GetCount();

	return iCount;
	}

IDatatype::EMemberType CDatatypeFunction::OnHasMember (CStringView sName, CDatum* retdType, int* retiOrdinal) const

//	OnHasMember
//
//	Returns the type of the given member.

	{
	return IDatatype::EMemberType::DynamicMember;
	}

bool CDatatypeFunction::OnIsA (const IDatatype &Type) const

//	OnIsA
//
//	Returns TRUE if we are the given type or a subtype.

	{
	if (Type.GetCoreType() == IDatatype::FUNCTION)
		return true;

	return false;
	}

void CDatatypeFunction::OnMark ()

//	OnMark
//
//	Mark data in use.

	{
	for (int i = 0; i < m_Signatures.GetCount(); i++)
		{
		auto &Signature = m_Signatures[i];

		Signature.Return.dType.Mark();

		for (int j = 0; j < Signature.Args.GetCount(); j++)
			Signature.Args[j].dType.Mark();
		}
	}

void CDatatypeFunction::OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	Write out the signatures

	Stream.Write(m_Signatures.GetCount());
	for (int i = 0; i < m_Signatures.GetCount(); i++)
		{
		const auto& Signature = m_Signatures[i];
		
		//	Write out the return type

		Signature.Return.dType.SerializeAEON(Stream, Serialized);

		//	Flags
		
		DWORD dwFlags = 0;
		dwFlags |= (Signature.Args.GetCount() > 0 && Signature.Args[Signature.Args.GetCount() - 1].bVarArg ?	0x00000001 : 0);
		dwFlags |= (Signature.Return.iType == EReturnDescType::ArgType ?	0x00000002 : 0);
		dwFlags |= (Signature.Return.iType == EReturnDescType::ArgLiteral ?	0x00000004 : 0);
		Stream.Write(dwFlags);

		if (Signature.Return.iType != EReturnDescType::Type)
			Stream.Write(Signature.Return.iFromArg);
		
		//	Write out the arguments
		
		Stream.Write(Signature.Args.GetCount());
		for (int j = 0; j < Signature.Args.GetCount(); j++)
			{
			Signature.Args[j].sID.Serialize(Stream);
			Signature.Args[j].sDesc.Serialize(Stream);
		
			Signature.Args[j].dType.SerializeAEON(Stream, Serialized);
			}
		}
	}
