//	CAEONNilImpl.cpp
//
//	CAEONNilImpl class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

TDatumPropertyHandler<int> CAEONNilImpl::m_Properties = {
	{
		"bytes",
		"a",
		"Returns an array of bytes.",
		[](const int& iDummy, const CString &sProperty)
			{
			return CDatum(CDatum::typeArray);
			},
		NULL,
		},
	{
		"chars",
		"a",
		"Returns an array of Unicode characters.",
		[](const int& iDummy, const CString &sProperty)
			{
			return CDatum(CDatum::typeArray);
			},
		NULL,
		},
	{
		"dimensions",
		"I",
		"Returns number of dimensions in the tensor.",
		[](const int& iDummy, const CString &sProperty)
			{
			return 0;
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the length of the string in bytes.",
		[](const int& iDummy, const CString &sProperty)
			{
			return CDatum(0);
			},
		NULL,
		},
	{
		"shape",
		"$ArrayOfInt32",
		"Returns an array of sizes for each dimension.",
		[](const int& iDummy, const CString &sProperty)
			{
			return CDatum(CDatum::typeArray);
			},
		NULL,
		},
	{
		"size",
		"I",
		"Returns the number of elements in the collection.",
		[](const int& iDummy, const CString &sProperty)
			{
			return CDatum(0);
			},
		NULL,
		},
	};

TDatumMethodHandler<int> CAEONNilImpl::m_Methods = {
	{
		"at",
		"?:key=?",
		".at(key) -> value",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"average",
		"*",
		".average() -> numeric average.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"cleaned",
		"s:",
		".cleaned() -> string",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"col",
		"a:col=?",
		".col(colname) -> column",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"concatenated",
		"?:src=?|src=?,axis=n",
		".concatenated(src, axis) -> array",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"edited",
		"s:pos=?,replace=s|start=?,end=?|start=?,end=?,replace=s",
		".edited(start, end, replace) -> string.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"endsWith",
		"*",
		".endsWith(string) -> true/false (case insensitive)",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = LocalEnv.GetArgument(0).IsNil();
			return true;
			},
		},
	{
		"endsWithExact",
		"*",
		".endsWithExact(string) -> true/false (case sensitive)",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = LocalEnv.GetArgument(0).IsNil();
			return true;
			},
		},
	{
		"filter",
		"*",
		".filter(fn(element)) -> array",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(CDatum::typeArray);
			return true;
			},
		},
	{
		"find",
		"*",
		".find(x) -> Finds offset of x in string (case insensitive).",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"findAll",
		"*",
		".findAll(x) -> Finds all x in array.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(CDatum::typeArray);
			return true;
			},
		},
	{
		"findAllExact",
		"*",
		".findAllExact(x) -> Finds all x in array.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(CDatum::typeArray);
			return true;
			},
		},
	{
		"findExact",
		"*",
		".findExact(x) -> Finds offset of x in string (case sensitive).",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"findMax",
		"*",
		".findMax() -> index or null",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"findMin",
		"*",
		".findMin() -> index or null",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"getAt",
		"?:key=?|key=?,column=?",
		".getAt(key) -> row\n"
		".getAt(key, colname) -> value",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"getColumn",
		"a:column=?",
		".getColumn(colname) -> column",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"grouped",
		"a:column=?|fn=?",
		".grouped(colname) -> array\n"
		".grouped(fn(row)) -> array",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(CDatum::typeArray);
			return true;
			},
		},
	{
		"hashed",
		"v:|options=?",
		".hashed() -> BLAKE2 hash.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CBuffer Buffer(NULL_STR);
			retResult.dResult = CDatum::CreateBinary(cryptoBLAKE2(Buffer));
			return true;
			},
		},
	{
		"indexed",
		"y:column=?|fn=?",
		".indexed(colname) -> dictionary\n"
		".indexed(fn(row)) -> dictionary",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum::CreateDictionary(CAEONTypes::Get(IDatatype::DICTIONARY));
			return true;
			},
		},
	{
		"joined",
		"*",
		".joined([separator, [lastSeparator]]) -> string.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(NULL_STR);
			return true;
			},
		},
	{
		"left",
		"*",
		".left(n) -> left n bytes.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"lowercased",
		"*",
		".lowercased() -> string in lowercase.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"map",
		"*",
		".map() -> Mapped array.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(CDatum::typeArray);
			return true;
			},
		},
	{
		"max",
		"?:",
		".max() -> value",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"min",
		"?:",
		".min() -> value",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"pos",
		"*",
		".pos(index) -> value.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"random",
		"*",
		".random() -> random element.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"reduce",
		"*",
		".reduce(fn, [[initial-value], [options], [progressFn]]) -> Result",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"repeated",
		"*",
		".repeated(count) -> Result",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"right",
		"*",
		".right(n) -> right n bytes.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"shuffled",
		"*",
		".shuffled() -> array.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(CDatum::typeArray);
			return true;
			},
		},
	{
		"sliced",
		"*",
		".sliced(start, [end]) -> slice",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"sorted",
		"*",
		".sort() -> sorted array, ascending\n"
		".sort(-1) -> sorted array, descending\n",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(CDatum::typeArray);
			return true;
			},
		},
	{
		"split",
		"*",
		".split() -> array",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(CDatum::typeArray);
			return true;
			},
		},
	{
		"startsWith",
		"*",
		".startsWith(string) -> true/false (case insensitive)",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = LocalEnv.GetArgument(0).IsNil();
			return true;
			},
		},
	{
		"startsWithExact",
		"*",
		".startsWithExact(string) -> true/false (case sensitive)",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = LocalEnv.GetArgument(0).IsNil();
			return true;
			},
		},
	{
		"stats",
		"*",
		".stats() -> Return statistics calculations.\n",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(CDatum::typeArray).MathStats();
			return true;
			},
		},
	{
		"stdDev",
		"*",
		".stdDev() -> Return standard deviation.\n",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"stdError",
		"*",
		".stdError() -> Return standard error.\n",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"sum",
		"*",
		".sum() -> Arithmetic sum.\n",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	{
		"ungrouped",
		"?:",
		".ungrouped() -> array",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(CDatum::typeArray);
			return true;
			},
		},
	{
		"unique",
		"0:|options=?",
		".unique() -> array of unique values.\n"
		".unique([ allowNulls=true, exact=true ])\n",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(CDatum::typeArray);
			return true;
			},
		},
	{
		"uppercased",
		"*",
		".uppercased() -> string in uppercase.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum();
			return true;
			},
		},
	};

