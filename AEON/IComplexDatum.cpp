//	IComplexDatum.cpp
//
//	IComplexDatum class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

size_t IComplexDatum::CalcSerializeAsStructSize (CDatum::EFormat iFormat) const

//	CalcSerializeAsStructSize
//
//	Returns the approximate serialization size if we serialize as a struct (in
//	bytes).

	{
	size_t TotalSize = 2;

	for (int i = 0; i < GetCount(); i++)
		TotalSize += (size_t)GetKey(i).GetLength() + 2 + GetElement(i).CalcSerializeSize(iFormat);

	return TotalSize;
	}

size_t IComplexDatum::CalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	CalcSerializeSizeAEONScript
//
//	Computes an approximate size.

	{
	size_t TotalSize = 0;
	DWORD dwFlags = OnGetSerializeFlags();

	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			TotalSize = GetTypename().GetLength() + 3;

			//	If this is object is serializable as a struct, then we do that.

			if (dwFlags & FLAG_SERIALIZE_AS_STRUCT)
				TotalSize += OnCalcSerializeSizeAEONScript(iFormat);

			//	Otherwise, serialize as base64 encoding, so we increase the total size by
			//	20%.

			else
				{
				size_t RawSize = OnCalcSerializeSizeAEONScript(iFormat);
				TotalSize += RawSize + (RawSize / 5);
				}
			break;
			}

		default:
			ASSERT(false);
		}

	return TotalSize;
	}

const IDatatype &IComplexDatum::CastIDatatype (void) const

//	CastIDatatype
//
//	Cast to IDatatype (default implementation)

	{
	return (const IDatatype &)CAEONTypeSystem::GetCoreType(IDatatype::ANY);
	}

bool IComplexDatum::OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct)

//	OnDeserialize
//
//	Descendants can override this to deserialize from a struct.

	{
	for (int i = 0; i < dStruct.GetCount(); i++)
		SetElement(dStruct.GetKey(i), dStruct.GetElement(i));

	return true;
	}

bool IComplexDatum::DeserializeAEONScript (CDatum::EFormat iFormat, const CString &sTypename, CCharStream *pStream)

//	DeserializeAEONScript
//
//	Deserialize AEONScript

	{
	DWORD dwFlags = OnGetSerializeFlags();

	//	If we have an open brace then we've stored everything as a structure.

	if (pStream->GetChar() == '{')
		{
		//	Object must support this

		if (!(dwFlags & FLAG_SERIALIZE_AS_STRUCT))
			return false;

		//	Parse the structure

		CAEONScriptParser Parser(pStream);
		CDatum dData;
		CAEONScriptParser::ETokens iToken = Parser.ParseToken(&dData);
		if (iToken != CAEONScriptParser::tkDatum)
			return false;

		//	Take all the fields in the structure and apply them to our object
		//	(our descendants will do the right thing).

		if (!OnDeserialize(iFormat, dData))
			return false;
		}

	//	Otherwise we expect base64 encoded data

	else
		{
		//	Backup one character because we want the OnDeserialize call to read it.

		pStream->UnreadChar();

		//	Deserialize

		CBase64Decoder Decoder(pStream->GetByteStream());
		if (!OnDeserialize(iFormat, sTypename, Decoder))
			return false;

		//	Read the next character into the stream 

		pStream->RefreshStream();
		pStream->ReadChar();
		}

	return true;
	}

bool IComplexDatum::DeserializeJSON (const CString &sTypename, const TArray<CDatum> &Data)

//	DeserializeJSON
//
//	Deserialize from JSON

	{
	if (Data.GetCount() == 0 || Data[0].GetBasicType() != CDatum::typeString)
		return false;

	//	LATER: Handle serialization/deserialization of struct-based objects

	//	Default deserialization

	CStringBuffer Buffer(Data[0]);
	CBase64Decoder Decoder(&Buffer);
	if (!OnDeserialize(CDatum::EFormat::JSON, sTypename, Decoder))
		return false;

	return true;
	}

CDatum IComplexDatum::GetDatatype () const

//	GetDatatype
//
//	Default implementation. This should only be used for internal datum types.

	{
	return CAEONTypeSystem::GetCoreType(IDatatype::ANY);
	}

CDatum IComplexDatum::GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const

//	GetElement
//
//	Returns the element.

	{
	int iIndex;

	if (dIndex.IsNil())
		return CDatum();
	else if (dIndex.IsNumberInt32(&iIndex))
		return GetElement(iIndex);
	else
		return GetElement(dIndex.AsString());
	}

void IComplexDatum::OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const

//	OnSerialize
//
//	Serialize all elements for obects that are serialized as structures.

	{
	for (int i = 0; i < GetCount(); i++)
		pStruct->SetElement(GetKey(i), GetElement(i));
	}

void IComplexDatum::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serialize the datum

	{
	DWORD dwFlags = OnGetSerializeFlags();

	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			if (!(dwFlags & FLAG_SERIALIZE_NO_TYPENAME))
				{
				Stream.Write("[", 1);
				Stream.Write(GetTypename());
				Stream.Write(":", 1);
				}

			//	If this is object is serializable as a struct, then we do that.

			if (dwFlags & FLAG_SERIALIZE_AS_STRUCT)
				{
				CComplexStruct *pStruct = new CComplexStruct;

				OnSerialize(iFormat, pStruct);

				CDatum dDatum(pStruct);
				dDatum.Serialize(iFormat, Stream);
				}

			//	Otherwise, serialize as base64 encoding

			else
				{
				CBase64Encoder Encoder(&Stream);
				OnSerialize(iFormat, Encoder);
				Encoder.Close();
				}

			if (!(dwFlags & FLAG_SERIALIZE_NO_TYPENAME))
				Stream.Write("]", 1);
			break;
			}

		case CDatum::EFormat::JSON:
			{
			if (!(dwFlags & FLAG_SERIALIZE_NO_TYPENAME))
				{
				Stream.Write("[\"AEON2011:", 11);
				Stream.Write(GetTypename());
				Stream.Write(":v1\", ", 6);
				}

			if (dwFlags & FLAG_SERIALIZE_AS_STRUCT)
				{
				CComplexStruct *pStruct = new CComplexStruct;

				OnSerialize(iFormat, pStruct);

				CDatum dDatum(pStruct);
				dDatum.Serialize(iFormat, Stream);
				}
			else
				{
				Stream.Write("\"", 1);

				CBase64Encoder Encoder(&Stream);
				OnSerialize(iFormat, Encoder);
				Encoder.Close();

				Stream.Write("\"", 1);
				}

			if (!(dwFlags & FLAG_SERIALIZE_NO_TYPENAME))
				Stream.Write("]", 1);
			break;
			}

		default:
			ASSERT(false);
		}
	}

void IComplexDatum::SerializeAsStruct (CDatum::EFormat iFormat, IByteStream &Stream) const

//	SerializeAsStruct
//
//	Helper function to serialize the object as if it were a struct.

	{
	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			Stream.Write("{", 1);

			for (int i = 0; i < GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(" ", 1);

				//	Write the key

				CDatum Key(GetKey(i));
				Key.Serialize(iFormat, Stream);

				//	Separator

				Stream.Write(":", 1);

				//	Write the value

				GetElement(i).Serialize(iFormat, Stream);
				}

			Stream.Write("}", 1);
			break;
			}

		case CDatum::EFormat::JSON:
			{
			Stream.Write("{", 1);

			for (int i = 0; i < GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(", ", 2);

				//	Write the key

				CDatum Key(GetKey(i));
				Key.Serialize(iFormat, Stream);

				//	Separator

				Stream.Write(": ", 2);

				//	Write the value

				GetElement(i).Serialize(iFormat, Stream);
				}

			Stream.Write("}", 1);
			break;
			}

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

void IComplexDatum::SetElementAt (CDatum dIndex, CDatum dDatum)

//	SetElement
//
//	Sets the element.

	{
	int iIndex;

	if (dIndex.IsNil())
		{ }
	else if (dIndex.IsNumberInt32(&iIndex))
		SetElement(iIndex, dDatum);
	else
		SetElement(dIndex.AsString(), dDatum);
	}

CString IComplexDatum::StructAsString () const

//	StructAsString
//
//	Returns a struct-type object as a string.

	{
	CStringBuffer Output;

	Output.Write("{", 1);

	for (int i = 0; i < GetCount(); i++)
		{
		if (i != 0)
			Output.Write(" ", 1);

		Output.Write(GetKey(i));
		Output.Write(":", 1);

		Output.Write(GetElement(i).AsString());
		}

	Output.Write("}", 1);

	return CString(Output);
	}

void IComplexDatum::WriteBinaryToStream (IByteStream &Stream, int iPos, int iLength, IProgressEvents *pProgress) const

//	WriteBinaryToStream
	
	{
	const CString &sData = CastCString();
	if (iPos >= sData.GetLength())
		return;

	if (pProgress)
		pProgress->OnProgressStart();

	if (iLength == -1)
		iLength = Max(0, sData.GetLength() - iPos);
	else
		iLength = Min(iLength, sData.GetLength() - iPos);

	Stream.Write(sData.GetPointer() + iPos, iLength); 

	if (pProgress)
		pProgress->OnProgressDone();
	}

