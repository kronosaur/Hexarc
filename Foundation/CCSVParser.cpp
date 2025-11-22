//	CSVParser.cpp
//
//	CSVParser class
//	Copyright (c) 2018 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_END_OF_STREAM,					"End of stream.")

CCSVParser::EFormat CCSVParser::ParseBOM (void)

//	ParseBOM
//
//	Parses any Byte Order Mark

	{
	switch (GetCurChar())
		{
		case '\xEF':
			{
			switch (GetNextChar())
				{
				case '\xBB':
					{
					switch (GetNextChar())
						{
						case '\xBF':
							{
							GetNextChar();
							return formatUTF8;
							}

						default:
							{
							ParseToOpenQuote();
							return formatError;
							}
						}
					break;
					}

				default:
					{
					ParseToOpenQuote();
					return formatError;
					}
				}
			break;
			}

		case '\xFE':
			{
			switch (GetNextChar())
				{
				case '\xFF':
					{
					ASSERT(false);	//	Not yet implemented
					GetNextChar();
					return formatUTF16_BigEndian;
					}

				default:
					{
					ParseToOpenQuote();
					return formatError;
					}
				}
			break;
			}

		case '\xFF':
			{
			switch (GetNextChar())
				{
				case '\xFE':
					{
					ASSERT(false);	//	Not yet implemented
					GetNextChar();
					return formatUTF16_LittleEndian;
					}

				default:
					{
					ParseToOpenQuote();
					return formatError;
					}
				}
			}

		default:
			return formatNone;
		}
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

	//	Parse the BOM, if any

	EFormat iBOMFormat = ParseBOM();
	if (iBOMFormat != formatError && iBOMFormat != formatNone)
		m_iFormat = iBOMFormat;

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
						if (retRow)
							retRow->Insert(CString(Value.GetPointer(), Value.GetLength()));
						return true;

					case '\'':
						if (retRow)
							{
							retRow->Insert(CString(Value.GetPointer(), Value.GetLength()));
							Value.SetLength(0);
							}
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
						if (retRow)
							retRow->Insert(CString(Value.GetPointer(), Value.GetLength()));
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
					if (retRow)
						{
						retRow->Insert(CString(Value.GetPointer(), Value.GetLength()));
						Value.SetLength(0);
						}
					iState = stateStart;
					}
				else
					{
					switch (GetCurChar())
						{
						case '\0':
							if (retRow)
								retRow->Insert(CString(Value.GetPointer(), Value.GetLength()));
							return true;

						//	Two double-quotes in a row is an escape for an embedded
						//	double-quote.

						case '"':
							Value.WriteChar('"');
							iState = stateDoubleQuote;
							break;

						case '\r':
							if (retRow)
								{
								retRow->Insert(CString(Value.GetPointer(), Value.GetLength()));
								Value.SetLength(0);
								}
							iState = stateCR;
							break;

						case '\n':
							if (retRow)
								{
								retRow->Insert(CString(Value.GetPointer(), Value.GetLength()));
								Value.SetLength(0);
								}
							iState = stateLF;
							break;

						case ' ':
						case '\t':
						default:
							if (retRow)
								{
								retRow->Insert(CString(Value.GetPointer(), Value.GetLength()));
								Value.SetLength(0);
								}
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
					if (retRow)
						{
						retRow->Insert(CString(Value.GetPointer(), Value.GetLength()));
						Value.SetLength(0);
						}
					iState = stateStart;
					}
				else
					{
					switch (GetCurChar())
						{
						case '\0':
							if (retRow)
								retRow->Insert(CString(Value.GetPointer(), Value.GetLength()));
							return true;

						case '\r':
							if (retRow)
								{
								retRow->Insert(CString(Value.GetPointer(), Value.GetLength()));
								Value.SetLength(0);
								}
							iState = stateCR;
							break;

						case '\n':
							if (retRow)
								{
								retRow->Insert(CString(Value.GetPointer(), Value.GetLength()));
								Value.SetLength(0);
								}
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

void CCSVParser::ParseToOpenQuote (void)

//	ParseToOpenQuote
//
//	Reads characters until we find an open quote.

	{
	while (GetCurChar() != '"')
		GetNextChar();
	}
