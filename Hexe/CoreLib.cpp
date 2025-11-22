//	CoreLib.cpp
//
//	Core Library
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

bool coreDateTime (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult);

const DWORD DATETIME =									0;
DECLARE_CONST_STRING(DATETIME_NAME,						"dateTime")
DECLARE_CONST_STRING(DATETIME_ARGS,						"*")
DECLARE_CONST_STRING(DATETIME_HELP,						"(dateTime 'now) -> current date/time")

const DWORD DATETIME_FORMAT =							1;
DECLARE_CONST_STRING(DATETIME_FORMAT_NAME,				"dateTimeFormat")
DECLARE_CONST_STRING(DATETIME_FORMAT_ARGS,				"*")
DECLARE_CONST_STRING(DATETIME_FORMAT_HELP,				"(dateTimeFormat dateTime [options]) -> formatted string")

const DWORD DATETIME_SPAN =								2;
DECLARE_CONST_STRING(DATETIME_SPAN_NAME,				"dateTimeSpan")
DECLARE_CONST_STRING(DATETIME_SPAN_ARGS,				"*")
DECLARE_CONST_STRING(DATETIME_SPAN_HELP,				"(dateTimeSpan dateTime1 dateTime2) -> seconds from dt1 to dt2")

bool coreLists (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult);

const DWORD LIST_ITEM =									0;
DECLARE_CONST_STRING(LIST_ITEM_NAME,					"@")
DECLARE_CONST_STRING(LIST_ITEM_ARGS,					"*")
DECLARE_CONST_STRING(LIST_ITEM_HELP,					"(@ list n) -> nth value in list")

const DWORD LIST_APPEND =								1;
DECLARE_CONST_STRING(LIST_APPEND_NAME,					"append")
DECLARE_CONST_STRING(LIST_APPEND_ARGS,					"*")
DECLARE_CONST_STRING(LIST_APPEND_HELP,					"(append list ...) -> list")

const DWORD LIST_COUNT =								2;
DECLARE_CONST_STRING(LIST_COUNT_NAME,					"count")
DECLARE_CONST_STRING(LIST_COUNT_ARGS,					"*")
DECLARE_CONST_STRING(LIST_COUNT_HELP,					"(count list) -> elements in list")

const DWORD LIST_GROUP =								3;
DECLARE_CONST_STRING(LIST_GROUP_NAME,					"group")
DECLARE_CONST_STRING(LIST_GROUP_ARGS,					"*")
DECLARE_CONST_STRING(LIST_GROUP_HELP,					"(group list count) -> list of lists")

const DWORD LIST_ITEM2 =								4;
DECLARE_CONST_STRING(LIST_ITEM2_NAME,					"item")
DECLARE_CONST_STRING(LIST_ITEM2_ARGS,					"*")
DECLARE_CONST_STRING(LIST_ITEM2_HELP,					"Same as @")

const DWORD LIST_LIST =									5;
DECLARE_CONST_STRING(LIST_LIST_NAME,					"list")
DECLARE_CONST_STRING(LIST_LIST_ARGS,					"*")
DECLARE_CONST_STRING(LIST_LIST_HELP,					"(list ...) -> list")

const DWORD LIST_MAKE =									6;
DECLARE_CONST_STRING(LIST_MAKE_NAME,					"make")
DECLARE_CONST_STRING(LIST_MAKE_ARGS,					"*")
DECLARE_CONST_STRING(LIST_MAKE_HELP,					"(make makeType ...) -> value")

const DWORD LIST_SLICE =								7;
DECLARE_CONST_STRING(LIST_SLICE_NAME,					"slice")
DECLARE_CONST_STRING(LIST_SLICE_ARGS,					"*")
DECLARE_CONST_STRING(LIST_SLICE_HELP,					"(slice list start [count]) -> list")

const DWORD LIST_SORT =									8;
DECLARE_CONST_STRING(LIST_SORT_NAME,					"sort!")
DECLARE_CONST_STRING(LIST_SORT_ARGS,					"*")
DECLARE_CONST_STRING(LIST_SORT_HELP,					"(sort! list [options]) -> struct")

const DWORD LIST_STRUCT =								9;
DECLARE_CONST_STRING(LIST_STRUCT_NAME,					"struct")
DECLARE_CONST_STRING(LIST_STRUCT_ARGS,					"*")
DECLARE_CONST_STRING(LIST_STRUCT_HELP,					"(struct ...) -> struct")

bool coreMath (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult);

const DWORD MATH_ABS =									0;
DECLARE_CONST_STRING(MATH_ABS_NAME,						"abs")
DECLARE_CONST_STRING(MATH_ABS_ARGS,						"*")
DECLARE_CONST_STRING(MATH_ABS_HELP,						"(abs x) -> result")

const DWORD MATH_ACOS =									1;
DECLARE_CONST_STRING(MATH_ACOS_NAME,					"acos")
DECLARE_CONST_STRING(MATH_ACOS_ARGS,					"*")
DECLARE_CONST_STRING(MATH_ACOS_HELP,					"(acos x) -> result")

const DWORD MATH_ASIN =									2;
DECLARE_CONST_STRING(MATH_ASIN_NAME,					"asin")
DECLARE_CONST_STRING(MATH_ASIN_ARGS,					"*")
DECLARE_CONST_STRING(MATH_ASIN_HELP,					"(asin x) -> result")

const DWORD MATH_ATAN =									3;
DECLARE_CONST_STRING(MATH_ATAN_NAME,					"atan")
DECLARE_CONST_STRING(MATH_ATAN_ARGS,					"*")
DECLARE_CONST_STRING(MATH_ATAN_HELP,					"(atan x) -> result")

const DWORD MATH_ATAN2 =								4;
DECLARE_CONST_STRING(MATH_ATAN2_NAME,					"atan2")
DECLARE_CONST_STRING(MATH_ATAN2_ARGS,					"*")
DECLARE_CONST_STRING(MATH_ATAN2_HELP,					"(atan2 y x) -> result")

const DWORD MATH_CEIL =									5;
DECLARE_CONST_STRING(MATH_CEIL_NAME,					"ceil")
DECLARE_CONST_STRING(MATH_CEIL_ARGS,					"*")
DECLARE_CONST_STRING(MATH_CEIL_HELP,					"(ceil x) -> result")

const DWORD MATH_COS =									6;
DECLARE_CONST_STRING(MATH_COS_NAME,						"cos")
DECLARE_CONST_STRING(MATH_COS_ARGS,						"*")
DECLARE_CONST_STRING(MATH_COS_HELP,						"(cos x) -> result")

const DWORD MATH_EXP =									7;
DECLARE_CONST_STRING(MATH_EXP_NAME,						"exp")
DECLARE_CONST_STRING(MATH_EXP_ARGS,						"*")
DECLARE_CONST_STRING(MATH_EXP_HELP,						"(exp x) -> result")

const DWORD MATH_FLOOR =								8;
DECLARE_CONST_STRING(MATH_FLOOR_NAME,					"floor")
DECLARE_CONST_STRING(MATH_FLOOR_ARGS,					"*")
DECLARE_CONST_STRING(MATH_FLOOR_HELP,					"(floor x) -> result")

const DWORD MATH_DIGEST =								9;
DECLARE_CONST_STRING(MATH_DIGEST_NAME,					"mathDigest")
DECLARE_CONST_STRING(MATH_DIGEST_ARGS,					"*")
DECLARE_CONST_STRING(MATH_DIGEST_HELP,					"(mathDigest value) -> SHA1 digest")

const DWORD MATH_INTIP_TO_INT32 =						10;
DECLARE_CONST_STRING(MATH_INTIP_TO_INT32_NAME,			"mathIntIPToByteList")
DECLARE_CONST_STRING(MATH_INTIP_TO_INT32_ARGS,			"*")
DECLARE_CONST_STRING(MATH_INTIP_TO_INT32_HELP,			"(mathIntIPToByteList intIP) -> list of 8-bit ints (big-endian)")

const DWORD MATH_LOG =									11;
DECLARE_CONST_STRING(MATH_LOG_NAME,						"log")
DECLARE_CONST_STRING(MATH_LOG_ARGS,						"*")
DECLARE_CONST_STRING(MATH_LOG_HELP,						"(log x) -> result")

const DWORD MATH_MAX =									12;
DECLARE_CONST_STRING(MATH_MAX_NAME,						"max")
DECLARE_CONST_STRING(MATH_MAX_ARGS,						"*")
DECLARE_CONST_STRING(MATH_MAX_HELP,						"(max x ...) -> result")

const DWORD MATH_MIN =									13;
DECLARE_CONST_STRING(MATH_MIN_NAME,						"min")
DECLARE_CONST_STRING(MATH_MIN_ARGS,						"*")
DECLARE_CONST_STRING(MATH_MIN_HELP,						"(min x ...) -> result")

const DWORD MATH_MOD =									14;
DECLARE_CONST_STRING(MATH_MOD_NAME,						"mod")
DECLARE_CONST_STRING(MATH_MOD_ARGS,						"*")
DECLARE_CONST_STRING(MATH_MOD_HELP,						"(mod x y) -> result")

const DWORD MATH_POW =									15;
DECLARE_CONST_STRING(MATH_POW_NAME,						"pow")
DECLARE_CONST_STRING(MATH_POW_ARGS,						"*")
DECLARE_CONST_STRING(MATH_POW_HELP,						"(pow x y) -> result")

const DWORD MATH_RANDOM =								16;
DECLARE_CONST_STRING(MATH_RANDOM_NAME,					"random")
DECLARE_CONST_STRING(MATH_RANDOM_ARGS,					"*")
DECLARE_CONST_STRING(MATH_RANDOM_HELP,					"(random) -> 0 to 1.0")

const DWORD MATH_ROUND =								17;
DECLARE_CONST_STRING(MATH_ROUND_NAME,					"round")
DECLARE_CONST_STRING(MATH_ROUND_ARGS,					"*")
DECLARE_CONST_STRING(MATH_ROUND_HELP,					"(round x) -> result")

const DWORD MATH_SIN =									18;
DECLARE_CONST_STRING(MATH_SIN_NAME,						"sin")
DECLARE_CONST_STRING(MATH_SIN_ARGS,						"*")
DECLARE_CONST_STRING(MATH_SIN_HELP,						"(sin x) -> result")

const DWORD MATH_SQRT =									19;
DECLARE_CONST_STRING(MATH_SQRT_NAME,					"sqrt")
DECLARE_CONST_STRING(MATH_SQRT_ARGS,					"*")
DECLARE_CONST_STRING(MATH_SQRT_HELP,					"(sqrt x) -> result")

const DWORD MATH_TAN =									20;
DECLARE_CONST_STRING(MATH_TAN_NAME,						"tan")
DECLARE_CONST_STRING(MATH_TAN_ARGS,						"*")
DECLARE_CONST_STRING(MATH_TAN_HELP,						"(tan x) -> result")

bool coreStrings (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult);

const DWORD STR_ASCII =									0;
DECLARE_CONST_STRING(STR_ASCII_NAME,					"ascii")
DECLARE_CONST_STRING(STR_ASCII_ARGS,					"*")
DECLARE_CONST_STRING(STR_ASCII_HELP,					"(ascii char) -> ascii code")

const DWORD STR_CAT =									1;
DECLARE_CONST_STRING(STR_CAT_NAME,						"cat")
DECLARE_CONST_STRING(STR_CAT_ARGS,						"*")
DECLARE_CONST_STRING(STR_CAT_HELP,						"(cat ...) -> string")

const DWORD STR_CLEAN =									2;
DECLARE_CONST_STRING(STR_CLEAN_NAME,					"clean")
DECLARE_CONST_STRING(STR_CLEAN_ARGS,					"*")
DECLARE_CONST_STRING(STR_CLEAN_HELP,					"(clean string [options]) -> string")

const DWORD STR_CONVERT_TO =							3;
DECLARE_CONST_STRING(STR_CONVERT_TO_NAME,				"convertTo")
DECLARE_CONST_STRING(STR_CONVERT_TO_ARGS,				"*")
DECLARE_CONST_STRING(STR_CONVERT_TO_HELP,				"(convertTo type value) -> result")

const DWORD STR_FIND =									4;
DECLARE_CONST_STRING(STR_FIND_NAME,						"find")
DECLARE_CONST_STRING(STR_FIND_ARGS,						"*")
DECLARE_CONST_STRING(STR_FIND_HELP,						"(find string stringToFind) -> index")

const DWORD STR_FORMAT =								5;
DECLARE_CONST_STRING(STR_FORMAT_NAME,					"format")
DECLARE_CONST_STRING(STR_FORMAT_ARGS,					"*")
DECLARE_CONST_STRING(STR_FORMAT_HELP,					"(format formatDesc value) -> string")

const DWORD STR_FROM_ARS =								6;
DECLARE_CONST_STRING(STR_FROM_ARS_NAME,					"fromARS")
DECLARE_CONST_STRING(STR_FROM_ARS_ARGS,					"*")
DECLARE_CONST_STRING(STR_FROM_ARS_HELP,					"(fromARS ARS-string) -> value")

const DWORD STR_HEX =									7;
DECLARE_CONST_STRING(STR_HEX_NAME,						"hex")
DECLARE_CONST_STRING(STR_HEX_ARGS,						"*")
DECLARE_CONST_STRING(STR_HEX_HELP,						"(hex decimal) -> hex string")

const DWORD STR_HTML =									8;
DECLARE_CONST_STRING(STR_HTML_NAME,						"html")
DECLARE_CONST_STRING(STR_HTML_ARGS,						"*")
DECLARE_CONST_STRING(STR_HTML_HELP,						"(html template struct) -> string")

const DWORD STR_JOIN =									9;
DECLARE_CONST_STRING(STR_JOIN_NAME,						"join")
DECLARE_CONST_STRING(STR_JOIN_ARGS,						"*")
DECLARE_CONST_STRING(STR_JOIN_HELP,						"(join list [separator-string]) -> string")

const DWORD STR_LENGTH =								10;
DECLARE_CONST_STRING(STR_LENGTH_NAME,					"length")
DECLARE_CONST_STRING(STR_LENGTH_ARGS,					"*")
DECLARE_CONST_STRING(STR_LENGTH_HELP,					"(length string) -> number of characters")

const DWORD STR_LOWERCASE =								11;
DECLARE_CONST_STRING(STR_LOWERCASE_NAME,				"lowercase")
DECLARE_CONST_STRING(STR_LOWERCASE_ARGS,				"*")
DECLARE_CONST_STRING(STR_LOWERCASE_HELP,				"(lowercase string) -> string")

const DWORD STR_SPLIT =									12;
DECLARE_CONST_STRING(STR_SPLIT_NAME,					"split")
DECLARE_CONST_STRING(STR_SPLIT_ARGS,					"*")
DECLARE_CONST_STRING(STR_SPLIT_HELP,					"(split string [separators] [max] [options]) -> string")

const DWORD STR_SUBSTRING =								13;
DECLARE_CONST_STRING(STR_SUBSTRING_NAME,				"substring")
DECLARE_CONST_STRING(STR_SUBSTRING_ARGS,				"*")
DECLARE_CONST_STRING(STR_SUBSTRING_HELP,				"(substring string start [count]) -> string")

const DWORD STR_TO_JSON =								14;
DECLARE_CONST_STRING(STR_TO_JSON_NAME,					"toJSON")
DECLARE_CONST_STRING(STR_TO_JSON_ARGS,					"*")
DECLARE_CONST_STRING(STR_TO_JSON_HELP,					"(toJSON value) -> JSON string")

const DWORD STR_TYPE_OF =								15;
DECLARE_CONST_STRING(STR_TYPE_OF_NAME,					"typeof")
DECLARE_CONST_STRING(STR_TYPE_OF_ARGS,					"*")
DECLARE_CONST_STRING(STR_TYPE_OF_HELP,					"(typeof value) -> type")

const DWORD STR_UPPERCASE =								16;
DECLARE_CONST_STRING(STR_UPPERCASE_NAME,				"uppercase")
DECLARE_CONST_STRING(STR_UPPERCASE_ARGS,				"*")
DECLARE_CONST_STRING(STR_UPPERCASE_HELP,				"(uppercase string) -> string")

const DWORD STR_URL_DECODE =							17;
DECLARE_CONST_STRING(STR_URL_DECODE_NAME,				"urlDecode")
DECLARE_CONST_STRING(STR_URL_DECODE_ARGS,				"*")
DECLARE_CONST_STRING(STR_URL_DECODE_HELP,				"(urlDecode string) -> string")

const DWORD STR_URL_PARAM =								18;
DECLARE_CONST_STRING(STR_URL_PARAM_NAME,				"urlParam")
DECLARE_CONST_STRING(STR_URL_PARAM_ARGS,				"*")
DECLARE_CONST_STRING(STR_URL_PARAM_HELP,				"(urlParam string) -> string")

bool coreSystem (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult);

const DWORD SYS_TICKS =									0;
DECLARE_CONST_STRING(SYS_TICKS_NAME,					"sysTicks")
DECLARE_CONST_STRING(SYS_TICKS_ARGS,					"*")
DECLARE_CONST_STRING(SYS_TICKS_HELP,					"(sysTicks) -> milliseconds")

DECLARE_CONST_STRING(DATE_TIME_NOW,						"now")

DECLARE_CONST_STRING(FLAG_ALPHA_NUMERIC,				"alphaNumeric")
DECLARE_CONST_STRING(FLAG_CLOCK,						"clock")
DECLARE_CONST_STRING(FLAG_CONVERT_TO_LOWERCASE,			"convertToLowercase")
DECLARE_CONST_STRING(FLAG_DEGREES,						"degrees")
DECLARE_CONST_STRING(FLAG_INCLUDE_BLANKS,				"includeBlanks")
DECLARE_CONST_STRING(FLAG_LINE,							"lines")
DECLARE_CONST_STRING(FLAG_RADIANS,						"radians")
DECLARE_CONST_STRING(FLAG_WHITESPACE,					"whitespace")

DECLARE_CONST_STRING(FORMAT_HEXE_TEXT,					"hexetext")
DECLARE_CONST_STRING(FORMAT_DATE_ONLY,					"dateonly")
DECLARE_CONST_STRING(FORMAT_DATE_TIME,					"datetime")
DECLARE_CONST_STRING(FORMAT_INTERNET,					"internet")
DECLARE_CONST_STRING(FORMAT_RELATIVE,					"relative")
DECLARE_CONST_STRING(FORMAT_SHORT_DATE_ONLY,			"shortdateonly")
DECLARE_CONST_STRING(FORMAT_SHORT_DATE_TIME,			"shortdatetime")
DECLARE_CONST_STRING(FORMAT_SHORT_DATE_TIME24,			"shortdatetime24")

DECLARE_CONST_STRING(MAKE_TYPE_CODE32,					"code32")
DECLARE_CONST_STRING(MAKE_TYPE_CODE_BLOCK32,			"codeBlock32")
DECLARE_CONST_STRING(MAKE_TYPE_SEQUENCE,				"sequence")

DECLARE_CONST_STRING(TYPE_BINARY,						"binary")
DECLARE_CONST_STRING(TYPE_DATE_TIME,					"dateTime")
DECLARE_CONST_STRING(TYPE_ERROR,						"error")
DECLARE_CONST_STRING(TYPE_FALSE,						"false")
DECLARE_CONST_STRING(TYPE_FUNCTION,						"function")
DECLARE_CONST_STRING(TYPE_IMAGE32,						"image32")
DECLARE_CONST_STRING(TYPE_INT32,						"int32")
DECLARE_CONST_STRING(TYPE_INT64,						"int64")
DECLARE_CONST_STRING(TYPE_INT_IP,						"intIP")
DECLARE_CONST_STRING(TYPE_LIST,							"list")
DECLARE_CONST_STRING(TYPE_NIL,							"nil")
DECLARE_CONST_STRING(TYPE_REAL,							"real")
DECLARE_CONST_STRING(TYPE_STRING,						"string")
DECLARE_CONST_STRING(TYPE_STRUCT,						"struct")
DECLARE_CONST_STRING(TYPE_TRUE,							"true")

DECLARE_CONST_STRING(TYPENAME_HEXE_FUNCTION,			"hexeFunction")

DECLARE_CONST_STRING(ERR_DIVISION_BY_ZERO,				"Divide by zero error.")
DECLARE_CONST_STRING(ERR_INSUFFICIENT_PARAMS,			"Insufficient parameters: %s.")
DECLARE_CONST_STRING(ERR_INVALID_DATETIME_FORMAT,		"Invalid dateTime format: %s.")
DECLARE_CONST_STRING(ERR_INVALID_PARAMETER,				"Invalid parameter: %s.")
DECLARE_CONST_STRING(ERR_INVALID_ASCII,					"Invalid ASCII number: %s.")
DECLARE_CONST_STRING(ERR_INVALID_CONTINUE_CTX,			"Invalid continue context.")
DECLARE_CONST_STRING(ERR_INVALID_MOD_OPTION,			"Invalid mod option: %s.")
DECLARE_CONST_STRING(ERR_KEY_EXPECTED,					"Key expected: %s.")
DECLARE_CONST_STRING(ERR_KEY_VALUE_EXPECTED,			"Key/value pair expected: %s.")
DECLARE_CONST_STRING(ERR_CANT_HEXIFY,					"Unable to convert value to hexadecimal: %s.")
DECLARE_CONST_STRING(ERR_CANT_DEHEXIFY,					"Unable to parse hexadecimal value: %s.")
DECLARE_CONST_STRING(ERR_UNEXPECTED_END_OF_TEMPLATE,	"Unexpected end of template.")
DECLARE_CONST_STRING(ERR_UNKNOWN_MAKE_TYPE,				"Unknown make type: %s.")
DECLARE_CONST_STRING(ERR_LIST_EXPECTED,					"List expected: %s.")
DECLARE_CONST_STRING(ERR_START_CANNOT_BE_NEGATIVE,		"Splice start must be non-negative.")

bool GetStrSplitFlags (CDatum dArg, DWORD *retdwFlags);
CDatum MinMaxOfList (CHexeStackEnv& LocalEnv, int iMinMax);
CDatum MinMaxOfList (CDatum dList, int iMinMax);
void WriteDatumToCat (CStringBuffer &Output, CDatum dDatum);
void WriteHTMLContent (CStringBuffer &Output, CDatum dDatum);

//	Library --------------------------------------------------------------------

SLibraryFuncDef g_CoreLibraryDef[] =
	{
	DECLARE_DEF_LIBRARY_FUNC(DATETIME, coreDateTime, 0),
	DECLARE_DEF_LIBRARY_FUNC(DATETIME_FORMAT, coreDateTime, 0),
	DECLARE_DEF_LIBRARY_FUNC(DATETIME_SPAN, coreDateTime, 0),

	DECLARE_DEF_LIBRARY_FUNC(LIST_APPEND, coreLists, 0),
	DECLARE_DEF_LIBRARY_FUNC(LIST_COUNT, coreLists, 0),
	DECLARE_DEF_LIBRARY_FUNC(LIST_GROUP, coreLists, 0),
	DECLARE_DEF_LIBRARY_FUNC(LIST_ITEM, coreLists, 0),
	DECLARE_DEF_LIBRARY_FUNC(LIST_ITEM2, coreLists, 0),
	DECLARE_DEF_LIBRARY_FUNC(LIST_LIST, coreLists, 0),
	DECLARE_DEF_LIBRARY_FUNC(LIST_MAKE, coreLists, 0),
	DECLARE_DEF_LIBRARY_FUNC(LIST_SLICE, coreLists, 0),
	DECLARE_DEF_LIBRARY_FUNC(LIST_SORT, coreLists, 0),
	DECLARE_DEF_LIBRARY_FUNC(LIST_STRUCT, coreLists, 0),

	DECLARE_DEF_LIBRARY_FUNC(MATH_ABS, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_ACOS, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_ASIN, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_ATAN, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_ATAN2, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_CEIL, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_COS, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_DIGEST, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_EXP, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_FLOOR, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_INTIP_TO_INT32, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_LOG, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_MAX, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_MIN, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_MOD, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_POW, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_RANDOM, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_ROUND, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_SIN, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_SQRT, coreMath, 0),
	DECLARE_DEF_LIBRARY_FUNC(MATH_TAN, coreMath, 0),

	DECLARE_DEF_LIBRARY_FUNC(STR_ASCII, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_CAT, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_CLEAN, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_CONVERT_TO, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_FIND, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_FORMAT, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_FROM_ARS, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_HEX, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_HTML, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_JOIN, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_LENGTH, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_LOWERCASE, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_SPLIT, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_SUBSTRING, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_TO_JSON, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_TYPE_OF, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_UPPERCASE, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_URL_DECODE, coreStrings, 0),
	DECLARE_DEF_LIBRARY_FUNC(STR_URL_PARAM, coreStrings, 0),

	DECLARE_DEF_LIBRARY_FUNC(SYS_TICKS, coreSystem, 0),
	};

const int g_iCoreLibraryDefCount = SIZEOF_STATIC_ARRAY(g_CoreLibraryDef);

//	Implementations ------------------------------------------------------------

bool coreDateTime (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
	{
	switch (dwData)
		{
		case DATETIME:
			{
			CStringView sArg = LocalEnv.GetArgument(0);

			//	If already a datetime, nothing to do

			if (LocalEnv.GetArgument(0).GetBasicType() == CDatum::typeDateTime)
				retResult.dResult = LocalEnv.GetArgument(0);

			//	Handle some special constants

			else if (strEquals(sArg, DATE_TIME_NOW))
				retResult.dResult = CDatum(CDateTime(CDateTime::Now));

			//	If a string, convert to date time

			else if (LocalEnv.GetArgument(0).GetBasicType() == CDatum::typeString)
				retResult.dResult = CDatum(CDatum::typeDateTime, sArg);

			else
				{
				CHexeError::Create(NULL_STR, strPattern(ERR_INVALID_PARAMETER, sArg), &retResult.dResult);
				return false;
				}

			return true;
			}

		case DATETIME_FORMAT:
			{
			//	Formats:
			//
			//	dateTime
			//	dateOnly
			//	internet
			//	relative
			//	shortDateOnly
			//	shortDateTime

			CDateTime DateTime = LocalEnv.GetArgument(0).AsDateTime();
			CString sFormat = strToLower(LocalEnv.GetArgument(1).AsStringView());

			if (!DateTime.IsValid())
				{
				retResult.dResult = CDatum();
				return true;
				}

			//	Format

			if (strEquals(sFormat, FORMAT_DATE_TIME))
				retResult.dResult = CDatum(DateTime.Format(CDateTime::dfLong, CDateTime::tfLong));
			else if (strEquals(sFormat, FORMAT_DATE_ONLY))
				retResult.dResult = CDatum(DateTime.Format(CDateTime::dfLong, CDateTime::tfNone));
			else if (strEquals(sFormat, FORMAT_INTERNET))
				retResult.dResult = CDatum(DateTime.FormatIMF());
			else if (strEquals(sFormat, FORMAT_RELATIVE))
				retResult.dResult = CDatum(DateTime.Format(CDateTime::dfRelative, CDateTime::tfNone));
			else if (strEquals(sFormat, FORMAT_SHORT_DATE_ONLY))
				retResult.dResult = CDatum(DateTime.Format(CDateTime::dfShort, CDateTime::tfNone));
			else if (strEquals(sFormat, FORMAT_SHORT_DATE_TIME))
				retResult.dResult = CDatum(DateTime.Format(CDateTime::dfShort, CDateTime::tfShort));
			else if (strEquals(sFormat, FORMAT_SHORT_DATE_TIME24))
				retResult.dResult = CDatum(DateTime.Format(CDateTime::dfShort, CDateTime::tfShort24));
			else
				{
				CHexeError::Create(NULL_STR, strPattern(ERR_INVALID_DATETIME_FORMAT, sFormat), &retResult.dResult);
				return false;
				}

			return true;
			}

		case DATETIME_SPAN:
			{
			//	If the second parameter is a number, then we generate a date
			//	from a date and offset.

			if (LocalEnv.GetArgument(1).IsNumber())
				{
				CDateTime DT1 = LocalEnv.GetArgument(0).AsDateTime();
				if (!DT1.IsValid())
					{
					retResult.dResult = CDatum();
					return true;
					}

				//	Generate a timespan of the appropriate length

				CTimeSpan Offset;
				CDatum dOffset = LocalEnv.GetArgument(1);
				bool bAdd;
				switch (dOffset.GetBasicType())
					{
					case CDatum::typeInteger32:
						{
						int iOffset = (int)dOffset;
						if (bAdd = (iOffset >= 0))
							Offset = CTimeSpan((DWORDLONG)iOffset * 1000);
						else
							Offset = CTimeSpan((DWORDLONG)-iOffset * 1000);

						break;
						}

					case CDatum::typeInteger64:
						Offset = CTimeSpan((DWORDLONG)dOffset * 1000);
						bAdd = true;
						break;

					case CDatum::typeIntegerIP:
						{
						const CIPInteger &Value = dOffset;
						bAdd = !Value.IsNegative();
						Offset = CTimeSpan(Value.AsInteger64Unsigned());
						break;
						}

					case CDatum::typeDouble:
						{
						double rValue = (double)dOffset;
						if (bAdd = (rValue >= 0.0))
							Offset = CTimeSpan((DWORDLONG)floor((rValue * 1000.0) + 0.5));
						else
							Offset = CTimeSpan((DWORDLONG)floor((-rValue * 1000.0) + 0.5));
						break;
						}

					default:
						{
						retResult.dResult = CDatum();
						return true;
						}
					}

				//	Add to timedate and return it

				if (bAdd)
					retResult.dResult = CDatum(timeAddTime(DT1, Offset));
				else
					retResult.dResult = CDatum(timeSubtractTime(DT1, Offset));

				return true;
				}

			//	Otherwise, we expect two dates and return the number of seconds
			//	between them.

			else
				{
				CDateTime DT1 = LocalEnv.GetArgument(0).AsDateTime();
				CDateTime DT2 = LocalEnv.GetArgument(1).AsDateTime();

				if (!DT1.IsValid() || !DT2.IsValid())
					{
					retResult.dResult = CDatum();
					return true;
					}

				bool bNegative = false;
				CTimeSpan Span;
				if (DT2 >= DT1)
					Span = timeSpan(DT1, DT2);
				else
					{
					Span = timeSpan(DT2, DT1);
					bNegative = true;
					}

				//	If more than 20,000 days, then we would overflow the number of
				//	seconds, so we use IPIntegers

				if (Span.Days() > 20000)
					{
					CIPInteger SpanIP(Span.Seconds64());
					if (bNegative)
						SpanIP = -SpanIP;

					CDatum::CreateIPInteger(SpanIP, &retResult.dResult);
					}
				else if (bNegative)
					retResult.dResult = CDatum(-Span.Seconds());
				else
					retResult.dResult = CDatum(Span.Seconds());

				return true;
				}
			}

		default:
			ASSERT(false);
			return false;
		}
	}

bool coreLists (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
	{
	int i, j;

	switch (dwData)
		{
		case LIST_APPEND:
			{
			if (LocalEnv.GetCount() == 0)
				{
				retResult.dResult = CDatum();
				return true;
				}

			CComplexArray *pResult = new CComplexArray(LocalEnv.GetArgument(0));
			for (i = 1; i < LocalEnv.GetCount(); i++)
				{
				CDatum dValue = LocalEnv.GetArgument(i);
				if (dValue.GetBasicType() == CDatum::typeArray)
					{
					for (j = 0; j < dValue.GetCount(); j++)
						pResult->Append(dValue.GetElement(j));
					}
				else if (!dValue.IsNil())
					pResult->Append(dValue);
				}

			if (pResult->GetCount() == 0)
				{
				delete pResult;
				retResult.dResult = CDatum();
				}
			else
				retResult.dResult = CDatum(pResult);

			return true;
			}

		case LIST_COUNT:
			{
			retResult.dResult = CDatum(LocalEnv.GetArgument(0).GetCount());
			return true;
			}

		case LIST_GROUP:
			{
			CDatum dList = LocalEnv.GetArgument(0);
			int iGroupSize = LocalEnv.GetArgument(1);

			if (iGroupSize <= 1)
				{
				retResult.dResult = dList;
				return true;
				}

			CComplexArray *pList = new CComplexArray;
			for (i = 0; i < dList.GetCount(); i += iGroupSize)
				{
				int j;

				CComplexArray *pGroup = new CComplexArray;

				for (j = 0; j < iGroupSize; j++)
					pGroup->Insert(dList.GetElement(i + j));

				pList->Insert(CDatum(pGroup));
				}

			retResult.dResult = CDatum(pList);
			return true;
			}

		case LIST_ITEM:
		case LIST_ITEM2:
			{
			CDatum dList = LocalEnv.GetArgument(0);
			CDatum dIndex = LocalEnv.GetArgument(1);

			//	LATER: If we have a structure with integers as keys, this code
			//	will fail (because it will try to treat the structure as an
			//	array). Unfortunately, I'm can't fix it until I make sure that
			//	existing code is not relying on that behavior.

			switch (dIndex.GetBasicType())
				{
				case CDatum::typeString:
					retResult.dResult = dList.GetElement(pCtx, dIndex.AsStringView());
					break;

				default:
					{
					int iIndex = (int)dIndex;
					if (iIndex < 0)
						{
						iIndex += dList.GetCount();
						retResult.dResult = (iIndex >= 0 ? dList.GetElement(pCtx, iIndex) : CDatum());
						}
					else
						retResult.dResult = dList.GetElement(pCtx, iIndex);
					}
				}

			return true;
			}

		case LIST_LIST:
			{
			CComplexArray *pList = new CComplexArray;
			for (i = 0; i < LocalEnv.GetCount(); i++)
				pList->Insert(LocalEnv.GetArgument(i));
			retResult.dResult = CDatum(pList);
			return true;
			}

		case LIST_MAKE:
			{
			CStringView sType = LocalEnv.GetArgument(0);

			if (strEquals(sType, MAKE_TYPE_CODE32))
				{
				int iDigits = LocalEnv.GetArgument(1);
				if (iDigits <= 0 || iDigits > 1000000)
					iDigits = 8;

				retResult.dResult = cryptoRandomCode(iDigits);
				}
			else if (strEquals(sType, MAKE_TYPE_CODE_BLOCK32))
				{
				int iDigits = LocalEnv.GetArgument(1);
				if (iDigits <= 0 || iDigits > 1000000)
					iDigits = 8;

				retResult.dResult = cryptoRandomCodeBlock(iDigits);
				}
			else if (strEquals(sType, MAKE_TYPE_SEQUENCE))
				{
				if (LocalEnv.GetCount() < 2)
					{
					retResult.dResult = CDatum();
					return true;
					}

				int iStart = LocalEnv.GetArgument(1);
				int iEnd;
				if (LocalEnv.GetCount() > 2)
					iEnd = LocalEnv.GetArgument(2);
				else
					{
					iEnd = iStart;
					iStart = 1;
					}

				//	Figure out increment

				int iInc;
				if (LocalEnv.GetCount() > 3)
					iInc = LocalEnv.GetArgument(3);
				else
					{
					if (iStart <= iEnd)
						iInc = 1;
					else
						iInc = -1;
					}

				if (iInc == 0)
					{
					retResult.dResult = CDatum();
					return true;
					}

				//	Make a list

				CComplexArray *pList = new CComplexArray;
				if (iInc > 0)
					{
					int iValue = iStart;
					while (iValue <= iEnd)
						{
						pList->Insert(iValue);
						iValue += iInc;
						}
					}
				else
					{
					int iValue = iStart;
					while (iValue >= iEnd)
						{
						pList->Insert(iValue);
						iValue += iInc;
						}
					}

				//	Done

				retResult.dResult = CDatum(pList);
				}
			else
				{
				CHexeError::Create(NULL_STR, strPattern(ERR_UNKNOWN_MAKE_TYPE, LocalEnv.GetArgument(0).AsString()), &retResult.dResult);
				return false;
				}

			return true;
			}

		case LIST_SLICE:
			{
			CDatum dList = LocalEnv.GetArgument(0);
			int iStart = LocalEnv.GetArgument(1);
			int iCount = (LocalEnv.GetArgument(2).IsNil() ? -1 : (int)LocalEnv.GetArgument(2));

			if (dList.IsNil())
				{
				retResult.dResult = CDatum();
				return true;
				}
			else if (dList.GetBasicType() != CDatum::typeArray)
				{
				CHexeError::Create(NULL_STR, strPattern(ERR_LIST_EXPECTED, dList.AsString()), &retResult.dResult);
				return false;
				}
			else if (iStart >= dList.GetCount() || iCount == 0)
				{
				retResult.dResult = CDatum();
				return true;
				}
			else if (iStart < 0)
				{
				CHexeError::Create(NULL_STR, ERR_START_CANNOT_BE_NEGATIVE, &retResult.dResult);
				return false;
				}
			else
				{
				if (iCount < 0)
					iCount = dList.GetCount() - iStart;

				retResult.dResult = CDatum(CDatum::typeArray);
				retResult.dResult.GrowToFit(iCount);

				int iEnd = Min(iStart + iCount, dList.GetCount());
				for (int i = iStart; i < iEnd; i++)
					retResult.dResult.Append(dList.GetElement(i));

				return true;
				}
			}

		case LIST_SORT:
			{
			//	If we have continue context then we reinvoke the function

			if (!dContinueCtx.IsNil())
				{
				CSortFunctionProcessor *pProcessor = CSortFunctionProcessor::Upconvert(dContinueCtx);
				if (pProcessor == NULL)
					{
					CHexeError::Create(NULL_STR, ERR_INVALID_CONTINUE_CTX, &retResult.dResult);
					return false;
					}

				return pProcessor->ProcessContinues(dContinueCtx, dContinueResult, &retResult.dResult);
				}

			//	Otherwise start the sorting

			else
				{
				//	Create a new processor to handle this

				CSortFunctionProcessor *pProcessor = new CSortFunctionProcessor(LocalEnv.GetArgument(0), LocalEnv.GetArgument(1));
				CDatum dProcessor(pProcessor);

				//	Invoke it.

				return pProcessor->Process(dProcessor, &retResult.dResult);
				}
			}

		case LIST_STRUCT:
			{
			CComplexStruct *pStruct = new CComplexStruct;

			i = 0;
			while (i < LocalEnv.GetCount())
				{
				CDatum dClause = LocalEnv.GetArgument(i);
				CStringView sKey = dClause;

				//	If the value is nil, then skip it. This allows us to have
				//	conditional key/value pairs.

				if (dClause.IsNil())
					i++;

				//	If this element is a string then assume that we have a key
				//	followed by a value

				else if (!sKey.IsEmpty())
					{
					i++;

					if (i < LocalEnv.GetCount())
						pStruct->SetElement(sKey, LocalEnv.GetArgument(i));

					i++;
					}

				//	If the clause is a structure then copy all the key/value
				//	pairs of the structure.

				else if (dClause.GetBasicType() == CDatum::typeStruct)
					{
					int j;

					for (j = 0; j < dClause.GetCount(); j++)
						pStruct->SetElement(dClause.GetKey(j), dClause.GetElement(j));

					i++;
					}

				//	Otherwise, we assume that the clause is a list with two
				//	elements (key and value)

				else if (dClause.GetCount() == 2)
					{
					CStringView sKey = dClause.GetElement(0);
					if (sKey.IsEmpty())
						{
						CHexeError::Create(NULL_STR, strPattern(ERR_KEY_EXPECTED, dClause.GetElement(0).AsString()), &retResult.dResult);
						return false;
						}

					pStruct->SetElement(sKey, dClause.GetElement(1));

					i++;
					}

				//	Otherwise an error

				else
					{
					CHexeError::Create(NULL_STR, strPattern(ERR_KEY_VALUE_EXPECTED, dClause.AsString()), &retResult.dResult);
					return false;
					}
				}

			//	Done

			retResult.dResult = CDatum(pStruct);
			return true;
			}

		default:
			ASSERT(false);
			return false;
		}
	}

bool coreMath (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
	{
	int i;

	switch (dwData)
		{
		case MATH_ABS:
			{
			int iValue;
			CDatum dValue = LocalEnv.GetArgument(0);
			switch (dValue.GetNumberType(&iValue))
				{
				case CDatum::typeInteger32:
					retResult.dResult = (iValue < 0 ? CDatum(-iValue) : dValue);
					return true;

				case CDatum::typeDouble:
					{
					double rValue = (double)dValue;
					retResult.dResult = (rValue < 0.0 ? CDatum(-rValue) : dValue);
					return true;
					}

				case CDatum::typeIntegerIP:
					{
					const CIPInteger &Value = (const CIPInteger &)dValue;
					if (Value.IsNegative())
						CDatum::CreateIPInteger(-Value, &retResult.dResult);
					else
						retResult.dResult = dValue;
					return true;
					}

				case CDatum::typeTimeSpan:
					{
					const CTimeSpan &Value = (const CTimeSpan &)dValue;
					if (Value.IsNegative())
						retResult.dResult = CDatum(CTimeSpan(Value, false));
					else
						retResult.dResult = dValue;
					return true;
					}

				default:
					retResult.dResult = dValue;
					return true;
				}
			}

		case MATH_ACOS:
			retResult.dResult = CDatum(acos((double)LocalEnv.GetArgument(0)));
			return true;

		case MATH_ASIN:
			retResult.dResult = CDatum(asin((double)LocalEnv.GetArgument(0)));
			return true;

		case MATH_ATAN:
			retResult.dResult = CDatum(atan((double)LocalEnv.GetArgument(0)));
			return true;

		case MATH_ATAN2:
			{
			double rY = LocalEnv.GetArgument(0);
			double rX = LocalEnv.GetArgument(1);

			retResult.dResult = CDatum(atan2(rY, rX));
			return true;
			}

		case MATH_CEIL:
			retResult.dResult = CDatum(ceil((double)LocalEnv.GetArgument(0)));
			return true;

		case MATH_COS:
			retResult.dResult = CDatum(cos((double)LocalEnv.GetArgument(0)));
			return true;

		case MATH_DIGEST:
			{
			CDatum dData = LocalEnv.GetArgument(0);
			int iDataSize;

			//	Digest

			if (dData.IsMemoryBlock())
				{
				//	Treat as a memory block

				CBuffer Buffer = dData.AsMemoryBlock();

				CIPInteger Hash;
				cryptoCreateDigest(Buffer, &Hash);
				CDatum::CreateIPIntegerFromHandoff(Hash, &retResult.dResult);
				return true;
				}

			//	If we have a binary blob (on disk) then we need to read this in
			//	pieces.

			else if ((iDataSize = dData.GetBinarySize()) > 0)
				{
				const int CHUNK_SIZE = 64 * 1024;
				CCryptoDigest SHA1;

				CBuffer Buffer(CHUNK_SIZE);
				int iPos = 0;
				while (iDataSize > 0)
					{
					int iSize = Min(iDataSize, CHUNK_SIZE);
					dData.WriteBinaryToStream(Buffer, iPos, iSize);
					SHA1.AddData(Buffer);

					Buffer.SetLength(0);
					iPos += iSize;
					iDataSize -= iSize;
					}

				CIPInteger Hash;
				SHA1.CalcDigest(&Hash);
				CDatum::CreateIPIntegerFromHandoff(Hash, &retResult.dResult);

				return true;
				}

			//	Otherwise, we don't have anything

			else
				{
				retResult.dResult = CDatum();
				return true;
				}
			}

		case MATH_EXP:
			retResult.dResult = CDatum(exp((double)LocalEnv.GetArgument(0)));
			return true;

		case MATH_FLOOR:
			retResult.dResult = CDatum(floor((double)LocalEnv.GetArgument(0)));
			return true;

		case MATH_INTIP_TO_INT32:
			{
			const CIPInteger &IntIP = LocalEnv.GetArgument(0);
			TArray<BYTE> Bytes;
			int iCount = IntIP.AsByteArray(&Bytes);
			if (iCount == 0)
				{
				retResult.dResult = CDatum();
				return true;
				}

			CComplexArray *pArray = new CComplexArray;
			pArray->InsertEmpty(iCount);

			for (i = 0; i < iCount; i++)
				pArray->SetElement(i, CDatum((int)Bytes[i]));

			retResult.dResult = CDatum(pArray);
			return true;
			}

		case MATH_LOG:
			retResult.dResult = CDatum(log((double)LocalEnv.GetArgument(0)));
			return true;

		case MATH_MAX:
			retResult.dResult = MinMaxOfList(LocalEnv, 1);
			return true;

		case MATH_MIN:
			retResult.dResult = MinMaxOfList(LocalEnv, -1);
			return true;

		case MATH_MOD:
			switch (LocalEnv.GetCount())
				{
				case 0:
				case 1:
					CHexeError::Create(NULL_STR, strPattern(ERR_INSUFFICIENT_PARAMS, MATH_MOD_NAME), &retResult.dResult);
					return false;

				case 2:
					{
					CNumberValue Dividend(LocalEnv.GetArgument(0));
					if (!Dividend.Mod(LocalEnv.GetArgument(1)))
						{
						CHexeError::Create(NULL_STR, ERR_DIVISION_BY_ZERO, &retResult.dResult);
						return false;
						}

					retResult.dResult = Dividend.GetDatum();
					break;
					}

				default:
					{
					if (strEquals(LocalEnv.GetArgument(0).AsStringView(), FLAG_DEGREES)
							|| strEquals(LocalEnv.GetArgument(0).AsStringView(), FLAG_RADIANS)
							|| strEquals(LocalEnv.GetArgument(0).AsStringView(), FLAG_CLOCK))
						{
						CNumberValue Dividend(LocalEnv.GetArgument(1));
						if (!Dividend.ModClock(LocalEnv.GetArgument(2)))
							{
							CHexeError::Create(NULL_STR, ERR_DIVISION_BY_ZERO, &retResult.dResult);
							return false;
							}

						retResult.dResult = Dividend.GetDatum();
						}
					else
						{
						CHexeError::Create(NULL_STR, strPattern(ERR_INVALID_MOD_OPTION, LocalEnv.GetArgument(0).AsString()), &retResult.dResult);
						return false;
						}
					break;
					}
				}

			return true;

		case MATH_POW:
			{
			double rX = LocalEnv.GetArgument(0);
			double rY = LocalEnv.GetArgument(1);

			retResult.dResult = CDatum(pow(rX, rY));
			return true;
			}

		case MATH_RANDOM:
			switch (LocalEnv.GetCount())
				{
				//	(random) -> 0 to 1.0 (real)

				case 0:
					retResult.dResult = CDatum(mathRandomDouble());
					break;

				//	(random x) -> 0 to x-1 (integer)
				//	(random list) -> random element in list

				case 1:
					{
					CDatum dArg = LocalEnv.GetArgument(0);
					if (dArg.IsNumber())
						retResult.dResult = CDatum(mathRandom(0, (int)dArg - 1));
					else
						retResult.dResult = dArg.GetElement(mathRandom(0, dArg.GetCount() - 1));
					break;
					}

				//	(random x y) -> x to y (integer)

				default:
					retResult.dResult = CDatum(mathRandom((int)LocalEnv.GetArgument(0), (int)LocalEnv.GetArgument(1)));
					break;
				}

			return true;

		case MATH_ROUND:
			{
			double rX = LocalEnv.GetArgument(0);

			if (rX >= 0.0)
				retResult.dResult = CDatum(floor(rX + 0.5));
			else
				retResult.dResult = CDatum(ceil(rX - 0.5));

			return true;
			}

		case MATH_SIN:
			retResult.dResult = CDatum(sin((double)LocalEnv.GetArgument(0)));
			return true;

		case MATH_SQRT:
			retResult.dResult = CDatum(sqrt((double)LocalEnv.GetArgument(0)));
			return true;

		case MATH_TAN:
			retResult.dResult = CDatum(tan((double)LocalEnv.GetArgument(0)));
			return true;

		default:
			ASSERT(false);
			return false;
		}
	}

bool coreStrings (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
	{
	int i;

	switch (dwData)
		{
		case STR_ASCII:
			{
			CDatum dInput = LocalEnv.GetArgument(0);
			
			//	If nil then we return nil

			if (dInput.IsNil())
				retResult.dResult = CDatum();

			//	If a single number then we return the character with that ASCII
			//	value.

			else if (dInput.IsNumber())
				{
				int iValue = (int)dInput;
				if (iValue < 0 || iValue > 255)
					{
					CHexeError::Create(NULL_STR, strPattern(ERR_INVALID_ASCII, dInput.AsString()), &retResult.dResult);
					return false;
					}

				char chChar = (char)(BYTE)iValue;
				retResult.dResult = CDatum(CString(&chChar, 1));
				}

			//	If a single string then convert to an array of ASCII values.

			else if (dInput.GetCount() == 1)
				{
				CString sChars = dInput.AsString();
				if (sChars.GetLength() == 1)
					retResult.dResult = CDatum((int)(BYTE)(*sChars.GetParsePointer()));
				else
					{
					char *pPos = sChars.GetParsePointer();
					char *pPosEnd = pPos + sChars.GetLength();

					CComplexArray *pResult = new CComplexArray;
					while (pPos < pPosEnd)
						pResult->Insert(CDatum((int)(BYTE)*pPos++));
					
					retResult.dResult = CDatum(pResult);
					}
				}

			//	If an array of numbers then convert to a single string.

			else if (dInput.GetCount() > 1 && dInput.GetElement(0).IsNumber())
				{
				CString sResult(dInput.GetCount());
				char *pResult = sResult.GetParsePointer();

				for (i = 0; i < dInput.GetCount(); i++)
					{
					int iValue = (int)dInput.GetElement(i);
					if (iValue >= 0 && iValue <= 255)
						*pResult++ = (char)(BYTE)iValue;
					else
						*pResult++ = '?';
					}

				retResult.dResult = CDatum(sResult);
				}

			//	Otherwise we expect an array of strings and we convert all the
			//	characters into ASCII.

			else
				{
				CComplexArray *pResult = new CComplexArray;

				for (i = 0; i < dInput.GetCount(); i++)
					{
					CStringView sChars = dInput.GetElement(i);
					const char *pPos = sChars.GetParsePointer();
					const char *pPosEnd = pPos + sChars.GetLength();

					while (pPos < pPosEnd)
						pResult->Insert(CDatum((int)(BYTE)*pPos++));
					}

				retResult.dResult = CDatum(pResult);
				}

			return true;
			}

		case STR_CAT:
			{
			CStringBuffer Buffer;

			for (i = 0; i < LocalEnv.GetCount(); i++)
				WriteDatumToCat(Buffer, LocalEnv.GetArgument(i));

			if (Buffer.GetLength() > 0)
				CDatum::CreateStringFromHandoff(Buffer, &retResult.dResult);
			else
				retResult.dResult = CDatum();
			return true;
			}

		case STR_CLEAN:
			{
			CStringView sString = LocalEnv.GetArgument(0);
			CString sClean = strClean(sString);

			if (!sClean.IsEmpty())
				retResult.dResult = CDatum(sClean);
			else
				retResult.dResult = CDatum();

			return true;
			}

		case STR_CONVERT_TO:
			{
			CStringView sType = LocalEnv.GetArgument(0);

			if (strEquals(sType, TYPE_INT32))
				retResult.dResult = (int)LocalEnv.GetArgument(1);
			else if (strEquals(sType, TYPE_INT64))
				retResult.dResult = (DWORDLONG)LocalEnv.GetArgument(1);
			else if (strEquals(sType, TYPE_REAL))
				retResult.dResult = (double)LocalEnv.GetArgument(1);
			else if (strEquals(sType, TYPE_STRING))
				retResult.dResult = LocalEnv.GetArgument(1).AsString();
			else
				retResult.dResult = CDatum();

			return true;
			}

		case STR_FIND:
			{
			CDatum dTarget = LocalEnv.GetArgument(0);
			CDatum dToFind = LocalEnv.GetArgument(1);

			//	If this is a string then do a string search

			int iIndex;
			if (dTarget.GetBasicType() == CDatum::typeString)
				iIndex = strFind(strToLower(dTarget.AsStringView()), strToLower(dToFind.AsString()));

			//	Otherwise, we do an array search

			else
				{
				iIndex = -1;

				for (i = 0; i < dTarget.GetCount(); i++)
					if (CHexeProcess::ExecuteIsEquivalent(dToFind, dTarget.GetElement(i)))
						{
						iIndex = i;
						break;
						}
				}

			//	Result

			if (iIndex != -1)
				retResult.dResult = CDatum(iIndex);
			else
				retResult.dResult = CDatum();

			return true;
			}

		case STR_FORMAT:
			{
			//	If we only have one parameter, then we try to format it as best 
			//	we can.

			if (LocalEnv.GetCount() == 1)
				{
				CDatum dValue = LocalEnv.GetArgument(0);

				switch (dValue.GetBasicType())
					{
					case CDatum::typeInteger32:
						retResult.dResult = CDatum(strFormatInteger(dValue, -1, FORMAT_THOUSAND_SEPARATOR));
						break;

					default:
						retResult.dResult = CDatum(dValue.AsString());
						break;
					}
				}

			//	Otherwise, we use the format

			else
				retResult.dResult = CDatum(CHexeTextMarkup::FormatString(LocalEnv));

			return true;
			}

		case STR_FROM_ARS:
			{
			CDatum dValue = LocalEnv.GetArgument(0);
			CString sError;

			//	Load as a HexeDocument. We create a brand new process to store
			//	any possible functions.

			CHexeProcess Process;
			if (!Process.LoadStandardLibraries(&sError))
				{
				retResult.dResult = sError;
				return false;
				}

			CBuffer Buffer(dValue.AsStringView());
			CHexeDocument ScenarioDoc;
			if (!ScenarioDoc.InitFromStream(Buffer, Process, &sError))
				{
				retResult.dResult = sError;
				return false;
				}

			//	Now convert the document into a datum

			retResult.dResult = ScenarioDoc.AsDatum();
			return true;
			}

		case STR_HEX:
			{
			CDatum dValue = LocalEnv.GetArgument(0);

			switch (dValue.GetBasicType())
				{
				case CDatum::typeInteger32:
					{
					int iSize = LocalEnv.GetArgument(1);

					if (iSize > 0 && iSize < 100)
						retResult.dResult = strPattern(strPattern("%%0%dx", iSize), (int)dValue);
					else
						retResult.dResult = strPattern("%x", (int)dValue);
					return true;
					}

				case CDatum::typeString:
					{
					CStringView sValue = dValue;

					bool bFailed;
					retResult.dResult = strParseIntOfBase(sValue.GetParsePointer(), 16, 0, NULL, &bFailed);
					if (bFailed)
						{
						CHexeError::Create(NULL_STR, strPattern(ERR_CANT_DEHEXIFY, dValue.AsString()), &retResult.dResult);
						return false;
						}
					
					return true;
					}

				default:
					CHexeError::Create(NULL_STR, strPattern(ERR_CANT_HEXIFY, dValue.AsString()), &retResult.dResult);
					return false;
				}
			}

		case STR_HTML:
			{
			//	If we have continue context then we reinvoke the function

			if (!dContinueCtx.IsNil())
				{
				CHexeTextFunctionProcessor *pProcessor = CHexeTextFunctionProcessor::Upconvert(dContinueCtx);
				if (pProcessor == NULL)
					{
					CHexeError::Create(NULL_STR, ERR_INVALID_CONTINUE_CTX, &retResult.dResult);
					return false;
					}

				return pProcessor->ProcessContinues(dContinueCtx, dContinueResult, &retResult.dResult);
				}

			//	One parameter means that we just escape the text so that it can
			//	be inside an HTML element.

			else if (LocalEnv.GetCount() == 1)
				{
				CStringBuffer Buffer;
				CHexeTextMarkup::WriteHTMLContent(LocalEnv.GetArgument(0), Buffer);
				CDatum::CreateStringFromHandoff(Buffer, &retResult.dResult);
				return true;
				}

			//	If we have two arguments and the second argument is a structure 
			//	then this is the old-style HTML escape, in which the second
			//	argument is a structure of replaceable text fragments.

			else if (LocalEnv.GetCount() == 2
						&& LocalEnv.GetArgument(1).GetBasicType() == CDatum::typeStruct)
				{
				CStringView sTemplate = LocalEnv.GetArgument(0);
				CDatum dStruct = LocalEnv.GetArgument(1);

				CStringBuffer Buffer;
				CString sError;
				if (!CHexeTextMarkup::EscapeHTML(CStringBuffer(sTemplate), dStruct, Buffer, &sError))
					{
					CHexeError::Create(NULL_STR, sError, &retResult.dResult);
					return false;
					}

				CDatum::CreateStringFromHandoff(Buffer, &retResult.dResult);
				return true;
				}

			//	Otherwise we expect the following:
			//
			//	1st arg: Some text
			//	2nd arg: Format
			//	3rd arg: Options

			else
				{
				//	Create a new processor to handle this

				CStringView sFormat = LocalEnv.GetArgument(1);
				CHexeTextFunctionProcessor *pProcessor = new CHexeTextFunctionProcessor(LocalEnv.GetArgument(0), sFormat, LocalEnv.GetArgument(2));
				CDatum dProcessor(pProcessor);

				//	Invoke it.

				return pProcessor->Process(dProcessor, &retResult.dResult);
				}
			}

		case STR_JOIN:
			{
			CStringBuffer Buffer;
			CDatum dList = LocalEnv.GetArgument(0);
			if (dList.GetCount() == 0)
				{
				retResult.dResult = CDatum();
				return true;
				}

			CString sSeparator = LocalEnv.GetArgument(1).AsString();

			for (i = 0; i < dList.GetCount(); i++)
				{
				CDatum dItem = dList.GetElement(i);

				if (i != 0 && !sSeparator.IsEmpty())
					Buffer.Write(sSeparator);

				Buffer.Write(dItem.AsString());
				}

			CDatum::CreateStringFromHandoff(Buffer, &retResult.dResult);
			return true;
			}

		case STR_LENGTH:
			{
			CDatum dValue = LocalEnv.GetArgument(0);

			switch (dValue.GetBasicType())
				{
				case CDatum::typeString:
				case CDatum::typeBinary:
					retResult.dResult = CDatum(dValue.AsStringView().GetLength());
					break;

				default:
					retResult.dResult = CDatum(dValue.GetCount());
				}

			return true;
			}

		case STR_LOWERCASE:
			{
			CString sString = LocalEnv.GetArgument(0).AsString();
			if (sString.IsEmpty())
				retResult.dResult = CDatum();
			else
				retResult.dResult = strToLower(sString);
			return true;
			}

		case STR_SPLIT:
			{
			//	The first argument is always the string to split.

			CString sString = LocalEnv.GetArgument(0).AsString();

			DWORD dwFlags = SSP_FLAG_NO_EMPTY_ITEMS;
			int iMaxCount = -1;
			CString sSeparators;

			//	Rest of the arguments depend on count

			switch (LocalEnv.GetCount())
				{
				case 1:
					//	Just take defaults
					break;

				case 2:
					{
					CDatum dArg2 = LocalEnv.GetArgument(1);

					//	If this is a number then it is max items

					if (dArg2.IsNumber())
						iMaxCount = Max(-1, (int)dArg2);

					//	If it is flags then use it as such

					else if (GetStrSplitFlags(dArg2, &dwFlags))
						;

					//	Otherwise, it is separators

					else
						sSeparators = dArg2.AsStringView();

					break;
					}

				case 3:
					{
					CDatum dArg2 = LocalEnv.GetArgument(1);
					CDatum dArg3 = LocalEnv.GetArgument(2);

					//	If this is a number then it is max items and the 
					//	following arg is flags.

					if (dArg2.IsNumber())
						{
						iMaxCount = Max(-1, (int)dArg2);
						GetStrSplitFlags(dArg3, &dwFlags);
						}

					//	Otherwise this must be separators (it can't be flags
					//	because there is no arg after flags).

					else
						{
						sSeparators = dArg2.AsStringView();

						//	If the third arg is a number then it is max items

						if (dArg3.IsNumber())
							iMaxCount = Max(-1, (int)dArg3);

						//	Otherwise, it is flags

						else
							GetStrSplitFlags(dArg3, &dwFlags);

						}

					break;
					}

				default:
					sSeparators = LocalEnv.GetArgument(1).AsStringView();
					iMaxCount = (LocalEnv.GetArgument(2).IsNil() ? -1 : Max(-1, (int)LocalEnv.GetArgument(2)));
					GetStrSplitFlags(LocalEnv.GetArgument(3), &dwFlags);
					break;
				}

			//	Now split the string

			TArray<CString> Result;
			strSplit(sString, sSeparators, &Result, iMaxCount, dwFlags);

			//	Convert to a datum

			if (Result.GetCount() == 0)
				retResult.dResult = CDatum();
			else
				{
				CComplexArray *pResult = new CComplexArray;
				for (i = 0; i < Result.GetCount(); i++)
					pResult->Insert(CDatum(Result[i]));
				retResult.dResult = CDatum(pResult);
				}

			return true;
			}

		case STR_SUBSTRING:
			{
			int iStart = Max(0, (int)LocalEnv.GetArgument(1));
			CDatum dCount = LocalEnv.GetArgument(2);
			int iCount = (dCount.IsNil() ? -1 : Max(0, (int)dCount));

			CString sResult = strSubString(LocalEnv.GetArgument(0).AsString(), iStart, iCount);
			if (sResult.IsEmpty())
				retResult.dResult = CDatum();
			else
				retResult.dResult = sResult;
			return true;
			}

		case STR_TO_JSON:
			retResult.dResult = LocalEnv.GetArgument(0).SerializeToString(CDatum::EFormat::JSON);
			return true;

		case STR_TYPE_OF:
			if (LocalEnv.GetArgument(0).IsError())
				retResult.dResult = TYPE_ERROR;
			else
				{
				switch (LocalEnv.GetArgument(0).GetBasicType())
					{
					case CDatum::typeNil:
						retResult.dResult = TYPE_NIL;
						break;

					case CDatum::typeFalse:
						retResult.dResult = TYPE_FALSE;
						break;

					case CDatum::typeTrue:
						retResult.dResult = TYPE_TRUE;
						break;

					case CDatum::typeInteger32:
						retResult.dResult = TYPE_INT32;
						break;

					case CDatum::typeString:
						retResult.dResult = TYPE_STRING;
						break;

					case CDatum::typeArray:
						retResult.dResult = TYPE_LIST;
						break;

					case CDatum::typeDouble:
						retResult.dResult = TYPE_REAL;
						break;

					case CDatum::typeStruct:
						retResult.dResult = TYPE_STRUCT;
						break;

					case CDatum::typeDateTime:
						retResult.dResult = TYPE_DATE_TIME;
						break;

					case CDatum::typeIntegerIP:
						retResult.dResult = TYPE_INT_IP;
						break;

					case CDatum::typeInteger64:
						retResult.dResult = TYPE_INT64;
						break;

					case CDatum::typeBinary:
						retResult.dResult = TYPE_BINARY;
						break;

					case CDatum::typeImage32:
						retResult.dResult = TYPE_IMAGE32;
						break;

					default:
						{
						CStringView sTypename = LocalEnv.GetArgument(0).GetTypename();
						if (strEquals(sTypename, TYPENAME_HEXE_FUNCTION))
							retResult.dResult = TYPE_FUNCTION;
						else
							retResult.dResult = CDatum();
						}
					}
				}
			return true;

		case STR_UPPERCASE:
			{
			CString sString = LocalEnv.GetArgument(0).AsString();
			if (sString.IsEmpty())
				retResult.dResult = CDatum();
			else
				retResult.dResult = strToUpper(sString);
			return true;
			}

		case STR_URL_DECODE:
			{
			retResult.dResult = urlDecode(LocalEnv.GetArgument(0).AsString());
			return true;
			}

		case STR_URL_PARAM:
			{
			retResult.dResult = urlEncodeParam(LocalEnv.GetArgument(0).AsString());
			return true;
			}

		default:
			ASSERT(false);
			return false;
		}
	}

bool coreSystem (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
	{
	switch (dwData)
		{
		case SYS_TICKS:
			retResult.dResult = CDatum((int)sysGetTickCount());
			return true;

		default:
			ASSERT(false);
			return false;
		}
	}

//	Helpers --------------------------------------------------------------------

bool GetStrSplitFlags (CDatum dArg, DWORD *retdwFlags)
	{
	int i;

	if (dArg.GetCount() == 0)
		return false;

	DWORD dwFlags = SSP_FLAG_NO_EMPTY_ITEMS;
	for (i = 0; i < dArg.GetCount(); i++)
		{
		CStringView sFlag = dArg.GetElement(i);
		if (strEquals(sFlag, FLAG_INCLUDE_BLANKS))
			dwFlags &= ~SSP_FLAG_NO_EMPTY_ITEMS;
		else if (strEquals(sFlag, FLAG_WHITESPACE))
			dwFlags |= SSP_FLAG_WHITESPACE_SEPARATOR;
		else if (strEquals(sFlag, FLAG_LINE))
			dwFlags |= SSP_FLAG_LINE_SEPARATOR;
		else if (strEquals(sFlag, FLAG_ALPHA_NUMERIC))
			dwFlags |= SSP_FLAG_ALPHANUMERIC_ONLY;
		else if (strEquals(sFlag, FLAG_CONVERT_TO_LOWERCASE))
			dwFlags |= SSP_FLAG_FORCE_LOWERCASE;
		else
			return false;
		}

	*retdwFlags = dwFlags;
	return true;
	}

CDatum MinMaxOfList (CHexeStackEnv& LocalEnv, int iMinMax)
	{
	if (LocalEnv.GetCount() == 0)
		return CDatum();
	else
		{
		CDatum dResult = LocalEnv.GetArgument(0);
		if (dResult.GetBasicType() == CDatum::typeArray)
			dResult = MinMaxOfList(dResult, iMinMax);

		for (int i = 1; i < LocalEnv.GetCount(); i++)
			{
			CDatum dElement = LocalEnv.GetArgument(i);
			if (dElement.GetBasicType() == CDatum::typeArray)
				dElement = MinMaxOfList(dElement, iMinMax);

			int iCompare = CDatum::Compare(dElement, dResult);
			if (iCompare == iMinMax)
				dResult = dElement;
			}

		return dResult;
		}
	}

CDatum MinMaxOfList (CDatum dList, int iMinMax)

//	MinMaxOfList
//
//	Returns either the min or the max element in a list.
//
//	iMinMax =  1: Max element in list
//	iMinMax = -1: Min element in list

	{
	int i;

	if (dList.GetCount() == 0)
		return CDatum();
	else
		{
		CDatum dResult = dList.GetElement(0);
		if (dResult.GetBasicType() == CDatum::typeArray)
			dResult = MinMaxOfList(dResult, iMinMax);

		for (i = 1; i < dList.GetCount(); i++)
			{
			CDatum dElement = dList.GetElement(i);
			if (dElement.GetBasicType() == CDatum::typeArray)
				dElement = MinMaxOfList(dElement, iMinMax);

			int iCompare = CDatum::Compare(dElement, dResult);
			if (iCompare == iMinMax)
				dResult = dElement;
			}

		return dResult;
		}
	}

CDatum MinOfList (CDatum dList)
	{
	int i;

	if (dList.GetCount() == 0)
		return CDatum();
	else if (dList.GetCount() == 1)
		return dList;
	else
		{
		CDatum dFirst = dList.GetElement(0);
		if (dFirst.GetCount() > 1)
			dFirst = MinOfList(dFirst);

		if (dFirst.IsNil())
			return CDatum();

		CNumberValue Result(dFirst);

		for (i = 1; i < dList.GetCount(); i++)
			{
			CDatum dElement = dList.GetElement(i);

			if (dElement.GetCount() == 0)
				return CDatum();
			else if (dElement.GetCount() == 1)
				Result.Min(dElement);
			else
				{
				CDatum dList = MinOfList(dElement);
				if (dList.IsNil())
					return CDatum();

				Result.Min(dList);
				}
			}

		return Result.GetDatum();
		}
	}

void WriteDatumToCat (CStringBuffer &Output, CDatum dDatum)
	{
	int i;

	if (dDatum.GetBasicType() == CDatum::typeArray)
		{
		for (i = 0; i < dDatum.GetCount(); i++)
			WriteDatumToCat(Output, dDatum.GetElement(i));
		}
	else
		{
		CString sString = dDatum.AsString();
		Output.Write((LPSTR)sString, sString.GetLength());
		}
	}

void WriteHTMLContent (CStringBuffer &Output, CDatum dDatum)
	{
	int i;

	if (dDatum.GetBasicType() == CDatum::typeArray)
		{
		for (i = 0; i < dDatum.GetCount(); i++)
			WriteHTMLContent(Output, dDatum.GetElement(i));
		}
	else
		htmlWriteText(dDatum.AsString(), Output);
	}
