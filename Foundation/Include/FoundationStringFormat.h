//	FoundationStringFormat.h
//
//	Foundation header file
//	Copyright (c) 2025 by GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class CStringFormat
	{
	public:

		struct SNumberFormat
			{
			CString sLeading;
			CString sTrailing;

			int iZeroPadding = 0;
			int iMinSignificantDigits = 0;
			int iMaxSignificantDigits = 0;
			bool bHasDigits = false;
			bool bCommaSeparators = false;
			bool bPercent = false;
			bool bNoNegativeSign = false;
			};

		enum class ECondition
			{
			Default,

			Zero,
			One,
			NegativeOne,
			Positive,
			Negative,
			NotZero,
			NotOne,
			Null,
			GreaterThanOne,
			LessThanNegativeOne,
			};

		struct SConditionalClause
			{
			ECondition iCondition = ECondition::Default;
			SNumberFormat Format;
			};

		CStringFormat () { }
		CStringFormat (CStringView sFormat) : m_sFormat(sFormat)
			{ }

		int GetClauseCount () const { InitNumericFormats(); return m_Clauses.GetCount(); }
		const SConditionalClause& GetClause (int iIndex) const
			{
			InitNumericFormats();

			if (iIndex < 0 || iIndex >= m_Clauses.GetCount())
				throw CException(errFail);

			return m_Clauses[iIndex];
			}

		CStringView GetFormatString () const { return m_sFormat; }
		bool IsEmpty () const { return m_sFormat.IsEmpty(); }

		bool FormatBinary (const BYTE* pData, int iLength, CString& retsResult) const;
		CString FormatDateTime (const CDateTime& Value) const;
		CString FormatDouble (double rValue) const;
		CString FormatInteger (int iValue) const;
		CString FormatIPInteger (const CIPInteger& Value) const;
		CString FormatNull () const;
		CString FormatTimeSpan (const CTimeSpan& Value) const;
		CString FormatTimeSpanHumanized (const CTimeSpan& Value, int iParts = 1) const;

		static const CStringFormat Null;

	private:

		static constexpr int MAX_FORMAT_STRING = 10'000;

		struct SDecimalDigits
			{
			const char* pDigits = NULL;
			int iLeft = 0;					//	Digits before decimal point
			int iRight = 0;					//	Digits after decimal point
			bool bDecimal = false;			//	TRUE if we've seen a decimal point
			bool bNegative = false;			//	TRUE if original number is negative
			};

		SNumberFormat CalcConditionForDouble (double rValue) const;
		SNumberFormat CalcConditionForInteger (int iValue) const;
		SNumberFormat CalcConditionForIPInteger (const CIPInteger& Value) const;
		bool InitNumericFormats () const { if (m_Clauses.GetCount() == 0) m_Clauses = Parse(m_sFormat, &m_bFormatError); return !m_bFormatError; }
		static CString FormatIntegerDigits (CStringView sDigits, bool bNegative, const SNumberFormat& Format);

		static void CalcDecimalDigits (const char* pSrc, bool bNegative, SDecimalDigits& retDigits);
		static int CalcFormatDoubleLength (const SDecimalDigits& Digits, const SNumberFormat& Format, int& retiOffset);
		static CBuffer FormatDoubleIntermediate (double rValue, const SNumberFormat& Format);
		static CString FormatDoubleOutput (const SDecimalDigits& Digits, const SNumberFormat& Format);
		static TArray<SConditionalClause> Parse (CStringView sFormat, bool* retbFormatError = NULL);
		static void ParseClause (CStringView sFormat, SConditionalClause& retClause);
		static SNumberFormat ParseNumberFormat (CStringView sFormat);
		static bool IsPluralizationFormat (const TArray<CString>& Clauses);
		static bool IsSpecialPluralizationFormat (const TArray<CString>& Clauses);
		static double SafeRound (double rValue, int iDigits);

		CString m_sFormat;
		mutable TArray<SConditionalClause> m_Clauses;	//	Cached parsed format for numbers
		mutable bool m_bFormatError = false;			//	TRUE if we had an error parsing the format string
	};

