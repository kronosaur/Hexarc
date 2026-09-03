//	CSVParser.cpp
//
//	CSVParser class
//	Copyright (c) 2018 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_END_OF_STREAM,					"End of stream.")

bool CCSVParser::AppendValue (CBuffer &Value, TArray<CString> *retRow, CString *retsError)

//	AppendValue
//
//	Adds the current field to the row, converting it to UTF-8 if necessary.

	{
	if (!retRow)
		{
		Value.SetLength(0);
		return true;
		}

	CString sRawValue(Value.GetPointer(), Value.GetLength());
	CString sValue;
	if (m_iCharSet == ECharSetType::UTF8)
		sValue = std::move(sRawValue);
	else if (!strDecodeToString(sRawValue.GetParsePointer(), sRawValue.GetLength(), m_iCharSet, &sValue, retsError))
		return false;

	retRow->Insert(std::move(sValue));
	Value.SetLength(0);
	return true;
	}

ECharSetType CCSVParser::DetectCharSet (IByteStream64 &Stream)

//	DetectCharSet
//
//	Returns UTF-8 if the entire stream is valid UTF-8. Otherwise, we assume
//	Windows-1252. We preserve the stream position.

	{
	constexpr int BUFFER_SIZE = 64 * 1024;
	char Buffer[BUFFER_SIZE];

	DWORDLONG dwOriginalPos = Stream.GetPos();
	DWORDLONG dwRemaining = Stream.GetStreamLength() - dwOriginalPos;

	int iContinuationCount = 0;
	UTF32 dwCodePoint = 0;
	UTF32 dwMinCodePoint = 0;
	bool bValid = true;

	while (bValid && dwRemaining > 0)
		{
		DWORDLONG dwToRead = Min((DWORDLONG)BUFFER_SIZE, dwRemaining);
		DWORDLONG dwRead = Stream.ReadTry(Buffer, dwToRead);
		if (dwRead == 0)
			{
			bValid = false;
			break;
			}

		const BYTE *pPos = (const BYTE *)Buffer;
		const BYTE *pEndPos = pPos + dwRead;
		while (bValid && pPos < pEndPos)
			{
			BYTE byValue = *pPos++;
			if (iContinuationCount == 0)
				{
				if ((byValue & 0x80) == 0)
					continue;
				else if ((byValue & 0xe0) == 0xc0)
					{
					iContinuationCount = 1;
					dwCodePoint = byValue & 0x1f;
					dwMinCodePoint = 0x80;
					}
				else if ((byValue & 0xf0) == 0xe0)
					{
					iContinuationCount = 2;
					dwCodePoint = byValue & 0x0f;
					dwMinCodePoint = 0x800;
					}
				else if ((byValue & 0xf8) == 0xf0)
					{
					iContinuationCount = 3;
					dwCodePoint = byValue & 0x07;
					dwMinCodePoint = 0x10000;
					}
				else
					bValid = false;
				}
			else if ((byValue & 0xc0) != 0x80)
				bValid = false;
			else
				{
				dwCodePoint = (dwCodePoint << 6) | (byValue & 0x3f);
				iContinuationCount--;
				if (iContinuationCount == 0
						&& (dwCodePoint < dwMinCodePoint
							|| (dwCodePoint >= 0xd800 && dwCodePoint <= 0xdfff)
							|| dwCodePoint > 0x10ffff))
					bValid = false;
				}
			}

		dwRemaining -= dwRead;
		}

	if (iContinuationCount != 0)
		bValid = false;

	Stream.Seek(dwOriginalPos);
	return (bValid ? ECharSetType::UTF8 : ECharSetType::Windows1252);
	}

void CCSVParser::ParseBOM (void)

//	ParseBOM
//
//	Skips a UTF-8 BOM at the start of the stream. If the leading bytes are not
//	a complete BOM, we restore the stream so the bytes are parsed as data.

	{
	if ((BYTE)GetCurChar() != 0xef)
		return;

	DWORDLONG dwStart = m_Stream.GetPos() - 1;
	if ((BYTE)GetNextChar() == 0xbb
			&& (BYTE)GetNextChar() == 0xbf)
		{
		GetNextChar();
		return;
		}

	m_Stream.Seek(dwStart);
	m_chCur = m_Stream.ReadChar();
	}

bool CCSVParser::ParseRow (TArray<CString> *retRow, CString *retsError)

//	ParseRow
//
//	Parses a row

	{
	enum EStates
		{
		stateStart,
		stateSingleQuote,
		stateDoubleQuote,
		stateInPlainValue,
		stateEndOfValue,
		stateCR,
		stateLF,
		stateDoubleQuoteEnd,
		};

	if (retRow)
		retRow->DeleteAll();

	//	Parse the BOM once, at the start of the stream.

	if (m_bAtStart)
		{
		ParseBOM();
		m_bAtStart = false;
		}

	//	Keep reading until we hit the end of the line.

	EStates iState = stateStart;
	CBuffer Value;
	while (true)
		{
		switch (iState)
			{
			case stateStart:
				{
				if (GetCurChar() == m_chDelimiter)
					{
					if (retRow)
						retRow->Insert(NULL_STR);
					}
				else
					{
					switch (GetCurChar())
						{
						case '\0':
							//	If we get here then it means that we ended a line with a comma.
							//	In that case we add an empty value to the row.

							if (retRow && retRow->GetCount() > 0)
								retRow->Insert(NULL_STR);
							return true;

						case ' ':
						case '\t':
							break;

						case '\r':
							//	If we get here then it means that we ended a line with a comma.
							//	In that case we add an empty value to the row.

							if (retRow && retRow->GetCount() > 0)
								retRow->Insert(NULL_STR);

							iState = stateCR;
							break;

						case '\n':
							//	If we get here then it means that we ended a line with a comma.
							//	In that case we add an empty value to the row.

							if (retRow && retRow->GetCount() > 0)
								retRow->Insert(NULL_STR);

							iState = stateLF;
							break;

						case '\'':
							iState = stateSingleQuote;
							break;

						case '"':
							iState = stateDoubleQuote;
							break;

						default:
							if (retRow)
								Value.WriteChar(GetCurChar());
							iState = stateInPlainValue;
							break;
						}
					}
				break;
				}

			case stateSingleQuote:
				{
				switch (GetCurChar())
					{
					case '\0':
						if (!AppendValue(Value, retRow, retsError))
							return false;
						return true;

					case '\'':
						if (!AppendValue(Value, retRow, retsError))
							return false;
						iState = stateEndOfValue;
						break;

					default:
						if (retRow)
							Value.WriteChar(GetCurChar());
						break;
					}
				break;
				}

			case stateDoubleQuote:
				{
				switch (GetCurChar())
					{
					case '\0':
						if (!AppendValue(Value, retRow, retsError))
							return false;
						return true;

					case '"':
						//if (retRow)
						//	{
						//	retRow->Insert(CString(Value.GetPointer(), Value.GetLength()));
						//	Value.SetLength(0);
						//	}
						iState = stateDoubleQuoteEnd;
						break;

					default:
						if (retRow)
							Value.WriteChar(GetCurChar());
						break;
					}
				break;
				}

			case stateDoubleQuoteEnd:
				{
				if (GetCurChar() == m_chDelimiter)
					{
					if (!AppendValue(Value, retRow, retsError))
						return false;
					iState = stateStart;
					}
				else
					{
					switch (GetCurChar())
						{
						case '\0':
							if (!AppendValue(Value, retRow, retsError))
								return false;
							return true;

						//	Two double-quotes in a row is an escape for an embedded
						//	double-quote.

						case '"':
							Value.WriteChar('"');
							iState = stateDoubleQuote;
							break;

						case '\r':
							if (!AppendValue(Value, retRow, retsError))
								return false;
							iState = stateCR;
							break;

						case '\n':
							if (!AppendValue(Value, retRow, retsError))
								return false;
							iState = stateLF;
							break;

						case ' ':
						case '\t':
						default:
							if (!AppendValue(Value, retRow, retsError))
								return false;
							iState = stateEndOfValue;
							break;
						}
					}
				break;
				}

			case stateEndOfValue:
				{
				if (GetCurChar() == m_chDelimiter)
					{
					iState = stateStart;
					}
				else
					{
					switch (GetCurChar())
						{
						case '\0':
							return true;

						case ' ':
						case '\t':
							break;

						case '\r':
							iState = stateCR;
							break;

						case '\n':
							iState = stateLF;
							break;

						default:
							break;
						}
					}
				break;
				}

			case stateInPlainValue:
				{
				if (GetCurChar() == m_chDelimiter)
					{
					if (!AppendValue(Value, retRow, retsError))
						return false;
					iState = stateStart;
					}
				else
					{
					switch (GetCurChar())
						{
						case '\0':
							if (!AppendValue(Value, retRow, retsError))
								return false;
							return true;

						case '\r':
							if (!AppendValue(Value, retRow, retsError))
								return false;
							iState = stateCR;
							break;

						case '\n':
							if (!AppendValue(Value, retRow, retsError))
								return false;
							iState = stateLF;
							break;

						default:
							if (retRow)
								Value.WriteChar(GetCurChar());
							break;
						}
					}
				break;
				}

			case stateCR:
				{
				switch (GetCurChar())
					{
					case '\0':
						return true;

					case '\n':
						GetNextChar();
						return true;

					default:
						break;
					}
				break;
				}

			case stateLF:
				{
				switch (GetCurChar())
					{
					case '\0':
						return true;

					case '\r':
						GetNextChar();
						return true;

					default:
						return true;
					}
				break;
				}
			}

		GetNextChar();
		}
	}
