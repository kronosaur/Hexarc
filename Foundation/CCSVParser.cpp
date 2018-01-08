//	CSVParser.cpp
//
//	CSVParser class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_END_OF_STREAM,					"End of stream.")

bool CCSVParser::ParseRow (TArray<CString> &Row, CString *retsError)

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
		};

	Row.DeleteAll();

	//	Keep reading until we hit the end of the line.

	EStates iState = stateStart;
	CBuffer Value;
	while (true)
		{
		switch (iState)
			{
			case stateStart:
				{
				switch (GetCurChar())
					{
					case '\0':
						return true;

					case ' ':
					case '\t':
						break;

					case ',':
						Row.Insert(NULL_STR);
						break;

					case '\r':
						iState = stateCR;
						break;

					case '\n':
						iState = stateLF;
						break;

					case '\'':
						iState = stateSingleQuote;
						break;

					case '\"':
						iState = stateDoubleQuote;
						break;

					default:
						Value.WriteChar(GetCurChar());
						iState = stateInPlainValue;
						break;
					}
				break;
				}

			case stateSingleQuote:
				{
				switch (GetCurChar())
					{
					case '\0':
						Row.Insert(CString(Value.GetPointer(), Value.GetLength()));
						return true;

					case '\'':
						Row.Insert(CString(Value.GetPointer(), Value.GetLength()));
						Value.SetLength(0);
						iState = stateEndOfValue;
						break;

					default:
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
						Row.Insert(CString(Value.GetPointer(), Value.GetLength()));
						return true;

					case '\"':
						Row.Insert(CString(Value.GetPointer(), Value.GetLength()));
						Value.SetLength(0);
						iState = stateEndOfValue;
						break;

					default:
						Value.WriteChar(GetCurChar());
						break;
					}
				break;
				}

			case stateEndOfValue:
				{
				switch (GetCurChar())
					{
					case '\0':
						return true;

					case ' ':
					case '\t':
						break;

					case ',':
						iState = stateStart;
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
				break;
				}

			case stateInPlainValue:
				{
				switch (GetCurChar())
					{
					case '\0':
						Row.Insert(CString(Value.GetPointer(), Value.GetLength()));
						return true;

					case ',':
						Row.Insert(CString(Value.GetPointer(), Value.GetLength()));
						Value.SetLength(0);
						iState = stateStart;
						break;

					case '\r':
						Row.Insert(CString(Value.GetPointer(), Value.GetLength()));
						Value.SetLength(0);
						iState = stateCR;
						break;

					case '\n':
						Row.Insert(CString(Value.GetPointer(), Value.GetLength()));
						Value.SetLength(0);
						iState = stateLF;
						break;

					default:
						Value.WriteChar(GetCurChar());
						break;
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
