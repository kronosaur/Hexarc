//	CAEONForeign.cpp
//
//	CAEONForeign class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CDatum CAEONForeign::Create (const CString& sTypename, CString&& sData)

//	Create
//
//	Creates a new foreign datum

	{
	return CDatum(new CAEONForeign(sTypename, CDatum(std::move(sData))));
	}

bool CAEONForeign::CreateFromStream (const CString& sTypename, CCharStream& Stream, CDatum& retdDatum)

//	CreateFromStream
//
//	Creates a foreign datum from the stream.

	{
	//	If we have an open brace then we've stored everything as a structure.
	//	We read the structure and then serialize it back.

	if (Stream.GetChar() == '{')
		{
		CAEONScriptParser Parser(&Stream);
		CDatum dData;
		CAEONScriptParser::ETokens iToken = Parser.ParseToken(&dData);
		if (iToken != CAEONScriptParser::tkDatum)
			return false;

		retdDatum = CDatum(new CAEONForeign(sTypename, dData));
		}

	//	Otherwise we expect base64 encoded data

	else
		{
		//	Keep reading until we find the close brace

		CBuffer Encoded;
		while (Stream.GetChar() != ']' && Stream.GetChar() != '\0')
			{
			Encoded.WriteChar(Stream.GetChar());
			Stream.ReadChar();
			}

		//	This was encoded in the stream, so we need to decode it. When we
		//	save it back, we encoded again. In previous versions we used to keep
		//	it encoded, but that won't work when we try to save to the new AEON
		//	binary format.

		Encoded.Seek(0);
		CBase64Decoder Decoder(&Encoded);
		CStringBuffer Output;
		Output.Write(Decoder, Decoder.GetStreamLength());

		retdDatum = CDatum(new CAEONForeign(sTypename, CDatum(std::move(Output))));
		}

	return true;
	}

void CAEONForeign::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serialize

	{
	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			Stream.Write("[", 1);
			Stream.Write(GetTypename());
			Stream.Write(":", 1);
			if (m_dData.GetBasicType() == CDatum::typeString)
				{
				//	Encode back to Base64

				CBase64Encoder Encoder(&Stream);
				CStringView sData = m_dData;
				Encoder.Write(sData);
				Encoder.Close();
				}
			else
				m_dData.Serialize(iFormat, Stream);

			Stream.Write("]", 1);
			break;

		case CDatum::EFormat::GridLang:
			CDatum::WriteGridLangString(Stream, strPattern("AEON: %s", GetTypename()));
			break;

		case CDatum::EFormat::JSON:
			Stream.Write("[\"AEON2011:", 11);
			Stream.Write(GetTypename());
			Stream.Write(":v1\", ", 6);
			if (m_dData.GetBasicType() == CDatum::typeString)
				{
				Stream.WriteChar('"');

				//	Always encode to Base64 because we might have null, etc.

				CBase64Encoder Encoder(&Stream);
				CStringView sData = m_dData;
				Encoder.Write(sData);
				Encoder.Close();

				Stream.WriteChar('"');
				}
			else
				m_dData.Serialize(iFormat, Stream);

			Stream.Write("]", 1);
			break;

		default:
			ASSERT(false);
		}
	}

void CAEONForeign::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_EXTERNAL))
		return;

	GetTypename().Serialize(Stream);

	CStringView sData = m_dData;
	Stream.Write(sData.GetLength());
	Stream.Write(sData);
	}
