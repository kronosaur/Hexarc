//	CDatatypeGenericFunction.cpp
//
//	CDatatypeGenericFunction class
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_ANY,	"Any");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_SCHEMA,	"Schema");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_FUNCTION,	"Function");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_EXPRESSION,	"Expression");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_DATATYPE,	"Datatype");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_WHERE,	"where");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_THIS,	"this");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_IS_ASSIGNABLE_TO,	"isAssignableTo");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_ISA,	"isa");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_ARRAY_OF,	"array of ");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_TABLE_OF,	"table of ");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_DICTIONARY,	"dictionary[");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_OF,	"of ");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_DICTIONARY_OF,	"dictionary of ");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_FUNCTION_9EF93D81,	"function");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_NULLABLE,	"nullable");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_NONNULLABLE,	"nonnullable");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_ELEMENTTYPE,	"elementtype");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_KEYTYPE,	"keytype");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_COMMON,	"common");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_EVALTYPE,	"evaltype");
DECLARE_CONST_STRING(STR_CDATATYPE_GENERIC_FUNCTION_ERROR,	"error");

namespace
	{
	DECLARE_CONST_STRING(ERR_GENERIC_PARSE,				"Unable to parse generic function signature.");
	DECLARE_CONST_STRING(ERR_NO_MATCHING_SIGNATURES,	"No matching signatures for argument types.");
	DECLARE_CONST_STRING(ERR_UNKNOWN_VALUE_REF,			"Unknown value reference in generic function signature: %s.");

	struct SMatchCtx
		{
		TSortMap<CString, CDatum> Bindings;
		TSortMap<CString, CDatum> LiteralBindings;
		const TArray<CDatum>* pArgLiteralTypes = NULL;
		bool bCoerce = false;
		};

	CString Trim (const CString& sValue)
		{
		const char* pStart = sValue.GetParsePointer();
		const char* pEnd = pStart + sValue.GetLength();

		while (pStart < pEnd && strIsWhitespace(pStart))
			pStart++;

		while (pEnd > pStart && strIsWhitespace(pEnd - 1))
			pEnd--;

		return CString(pStart, pEnd - pStart);
		}

	bool StartsWithNoCase (const CString& sValue, const CString& sPrefix)
		{
		if (sValue.GetLength() < sPrefix.GetLength())
			return false;

		return strEqualsNoCase(CString(sValue.GetParsePointer(), sPrefix.GetLength()), sPrefix);
		}

	bool IsIdentChar (char chChar)
		{
		return (isalnum((unsigned char)chChar) || chChar == '_' || chChar == '.');
		}

	bool IsKeywordBoundary (const char* pStart, const char* pPos, const char* pEnd, int iLen)
		{
		return ((pPos == pStart || !IsIdentChar(pPos[-1]))
				&& (pPos + iLen == pEnd || !IsIdentChar(pPos[iLen])));
		}

	bool IsValueRefName (const CString& sValue)
		{
		CString sTrimmed = Trim(sValue);
		const char* pPos = sTrimmed.GetParsePointer();
		const char* pEnd = pPos + sTrimmed.GetLength();
		if (pPos == pEnd || !IsIdentChar(*pPos))
			return false;

		while (pPos < pEnd)
			{
			if (!IsIdentChar(*pPos))
				return false;

			pPos++;
			}

		return true;
		}

	bool IsTypeVar (const CString& sValue, CString* retsVar = NULL)
		{
		CString sTrimmed = Trim(sValue);
		const char* pPos = sTrimmed.GetParsePointer();
		int iLen = sTrimmed.GetLength();
		if (iLen < 3 || pPos[0] != '<' || pPos[iLen - 1] != '>')
			return false;

		CString sVar(pPos + 1, iLen - 2);
		if (sVar.IsEmpty())
			return false;

		if (retsVar)
			*retsVar = sVar;

		return true;
		}

	int FindMatching (const CString& sValue, int iOpen, char chOpen, char chClose)
		{
		const char* pStart = sValue.GetParsePointer();
		const char* pPos = pStart + iOpen;
		const char* pEnd = pStart + sValue.GetLength();
		int iDepth = 0;
		bool bInString = false;

		while (pPos < pEnd)
			{
			char chChar = *pPos;
			if (bInString)
				{
				if (chChar == '\\' && pPos + 1 < pEnd)
					pPos++;
				else if (chChar == '"')
					bInString = false;
				}
			else if (chChar == '"')
				bInString = true;
			else if (chChar == chOpen)
				iDepth++;
			else if (chChar == chClose)
				{
				iDepth--;
				if (iDepth == 0)
					return (int)(pPos - pStart);
				}

			pPos++;
			}

		return -1;
		}

	TArray<CString> SplitTopLevel (const CString& sValue, char chDelimiter)
		{
		TArray<CString> Result;
		const char* pStart = sValue.GetParsePointer();
		const char* pPos = pStart;
		const char* pEnd = pStart + sValue.GetLength();
		const char* pPartStart = pStart;
		int iParenDepth = 0;
		int iBracketDepth = 0;
		int iAngleDepth = 0;
		bool bInString = false;

		while (pPos < pEnd)
			{
			char chChar = *pPos;
			if (bInString)
				{
				if (chChar == '\\' && pPos + 1 < pEnd)
					pPos++;
				else if (chChar == '"')
					bInString = false;
				}
			else if (chChar == '"')
				bInString = true;
			else if (chChar == '(')
				iParenDepth++;
			else if (chChar == ')')
				iParenDepth--;
			else if (chChar == '[')
				iBracketDepth++;
			else if (chChar == ']')
				iBracketDepth--;
			else if (chChar == '<')
				iAngleDepth++;
			else if (chChar == '>')
				iAngleDepth--;
			else if (chChar == chDelimiter && iParenDepth == 0 && iBracketDepth == 0 && iAngleDepth == 0)
				{
				Result.Insert(Trim(CString(pPartStart, pPos - pPartStart)));
				pPartStart = pPos + 1;
				}

			pPos++;
			}

		Result.Insert(Trim(CString(pPartStart, pEnd - pPartStart)));
		return Result;
		}

	bool SplitFirstTopLevel (const CString& sValue, char chDelimiter, CString* retsLeft, CString* retsRight)
		{
		const char* pStart = sValue.GetParsePointer();
		const char* pPos = pStart;
		const char* pEnd = pStart + sValue.GetLength();
		int iParenDepth = 0;
		int iBracketDepth = 0;
		int iAngleDepth = 0;
		bool bInString = false;

		while (pPos < pEnd)
			{
			char chChar = *pPos;
			if (bInString)
				{
				if (chChar == '\\' && pPos + 1 < pEnd)
					pPos++;
				else if (chChar == '"')
					bInString = false;
				}
			else if (chChar == '"')
				bInString = true;
			else if (chChar == '(')
				iParenDepth++;
			else if (chChar == ')')
				iParenDepth--;
			else if (chChar == '[')
				iBracketDepth++;
			else if (chChar == ']')
				iBracketDepth--;
			else if (chChar == '<')
				iAngleDepth++;
			else if (chChar == '>')
				iAngleDepth--;
			else if (chChar == chDelimiter && iParenDepth == 0 && iBracketDepth == 0 && iAngleDepth == 0)
				{
				*retsLeft = Trim(CString(pStart, pPos - pStart));
				*retsRight = Trim(CString(pPos + 1, pEnd - pPos - 1));
				return true;
				}

			pPos++;
			}

		return false;
		}

	CString ParseStringLiteral (const CString& sValue)
		{
		CString sTrimmed = Trim(sValue);
		const char* pStart = sTrimmed.GetParsePointer();
		const char* pEnd = pStart + sTrimmed.GetLength();
		if (pEnd - pStart < 2 || *pStart != '"' || pEnd[-1] != '"')
			return sTrimmed;

		CStringBuffer Buffer;
		const char* pPos = pStart + 1;
		while (pPos < pEnd - 1)
			{
			if (*pPos == '\\' && pPos + 1 < pEnd - 1)
				{
				pPos++;
				switch (*pPos)
					{
					case '"':
						Buffer.Write("\"", 1);
						break;

					case '\\':
						Buffer.Write("\\", 1);
						break;

					case 'n':
						Buffer.Write("\n", 1);
						break;

					default:
						Buffer.Write(pPos, 1);
						break;
					}
				}
			else
				Buffer.Write(pPos, 1);

			pPos++;
			}

		return CString(Buffer.GetPointer(), Buffer.GetLength());
		}

	CDatum ResolveTypeName (const CString& sName)
		{
		CString sTrimmed = Trim(sName);
		if (sTrimmed.IsEmpty())
			return CDatum();

		if (strEqualsNoCase(sTrimmed, STR_CDATATYPE_GENERIC_FUNCTION_ANY))
			return CAEONTypes::Get(IDatatype::ANY);
		else if (strEqualsNoCase(sTrimmed, STR_CDATATYPE_GENERIC_FUNCTION_SCHEMA))
			return CAEONTypes::Get(IDatatype::SCHEMA);
		else if (strEqualsNoCase(sTrimmed, STR_CDATATYPE_GENERIC_FUNCTION_FUNCTION))
			return CAEONTypes::Get(IDatatype::FUNCTION);
		else if (strEqualsNoCase(sTrimmed, STR_CDATATYPE_GENERIC_FUNCTION_EXPRESSION))
			return CAEONTypes::Get(IDatatype::EXPRESSION);
		else if (strEqualsNoCase(sTrimmed, STR_CDATATYPE_GENERIC_FUNCTION_DATATYPE))
			return CAEONTypes::Get(IDatatype::DATATYPE);

		CDatum dType = CAEONTypes::FindBuiltInType(CAEONTypes::MakeFullyQualifiedName(NULL_STR, sTrimmed));
		if (!dType.IsNil())
			return dType;

		return CAEONTypes::FindBuiltInType(sTrimmed);
		}

	void AccumulateExprTypes (const CDatatypeGenericFunction::STypeExpr& Expr, TSortMap<CString, CDatum>& retTypes)
		{
		if (!Expr.dType.IsNil())
			{
			const IDatatype& Type = Expr.dType;
			if (!Type.IsCoreType())
				{
				retTypes.SetAt(Type.GetFullyQualifiedName(), Expr.dType);
				Type.AccumulateTypesUsed(retTypes);
				}
			}

		for (int i = 0; i < Expr.Args.GetCount(); i++)
			AccumulateExprTypes(Expr.Args[i], retTypes);
		}

	void MarkExpr (CDatatypeGenericFunction::STypeExpr& Expr)
		{
		Expr.dType.Mark();
		for (int i = 0; i < Expr.Args.GetCount(); i++)
			MarkExpr(Expr.Args[i]);
		}

	bool SameType (CDatum dLeft, CDatum dRight)
		{
		if (dLeft.IsNil() || dRight.IsNil())
			return false;

		return ((const IDatatype&)dLeft == (const IDatatype&)dRight);
		}

	bool BindTypeVar (const CString& sVar, CDatum dType, SMatchCtx& Ctx)
		{
		if (sVar.IsEmpty() || dType.IsNil())
			return false;

		CDatum* pExisting = Ctx.Bindings.GetAt(sVar);
		if (pExisting)
			return SameType(*pExisting, dType);

		Ctx.Bindings.SetAt(sVar, dType);
		return true;
		}

	bool EvalLiteralExpr (const CDatatypeGenericFunction::STypeExpr& Expr, const SMatchCtx& Ctx, CDatum* retdValue);
	bool EvalTypeExpr (const CDatatypeGenericFunction::STypeExpr& Expr, const SMatchCtx& Ctx, CDatum* retdType, CString* retsError);
	bool MatchConstraint (const CDatatypeGenericFunction::SConstraintDesc& Constraint, SMatchCtx& Ctx);

	bool MatchTypeExpr (const CDatatypeGenericFunction::STypeExpr& Pattern, CDatum dActualType, SMatchCtx& Ctx)
		{
		if (dActualType.IsNil() || dActualType.GetBasicType() != CDatum::typeDatatype)
			return false;

		const IDatatype& ActualType = dActualType;

		switch (Pattern.iType)
			{
			case CDatatypeGenericFunction::ETypeExpr::Concrete:
				{
				if (Pattern.dType.IsNil())
					return false;

				const IDatatype& RequiredType = Pattern.dType;
				return (Ctx.bCoerce ? RequiredType.CanBeConstructedFrom(dActualType) : ActualType.IsA(RequiredType));
				}

			case CDatatypeGenericFunction::ETypeExpr::TypeVar:
				return BindTypeVar(Pattern.sValue, dActualType, Ctx);

			case CDatatypeGenericFunction::ETypeExpr::ValueRef:
				{
				CDatum dRequiredType;
				if (!EvalTypeExpr(Pattern, Ctx, &dRequiredType, NULL))
					return false;

				const IDatatype& RequiredType = dRequiredType;
				return (Ctx.bCoerce ? RequiredType.CanBeConstructedFrom(dActualType) : ActualType.IsA(RequiredType));
				}

			case CDatatypeGenericFunction::ETypeExpr::ArrayOf:
				if (ActualType.GetClass() != IDatatype::ECategory::Array)
					return false;
				return MatchTypeExpr(Pattern.Args[0], ActualType.GetElementType(), Ctx);

			case CDatatypeGenericFunction::ETypeExpr::TableOf:
				if (ActualType.GetClass() != IDatatype::ECategory::Table)
					return false;
				return MatchTypeExpr(Pattern.Args[0], ActualType.GetElementType(), Ctx);

			case CDatatypeGenericFunction::ETypeExpr::DictionaryOf:
				if (ActualType.GetClass() != IDatatype::ECategory::Dictionary)
					return false;
				return (MatchTypeExpr(Pattern.Args[0], ActualType.GetKeyType(), Ctx)
						&& MatchTypeExpr(Pattern.Args[1], ActualType.GetElementType(), Ctx));

			case CDatatypeGenericFunction::ETypeExpr::FunctionOf:
				{
				if (Pattern.Args.GetCount() == 0)
					return false;

				if (!ActualType.IsA(CAEONTypes::Get(IDatatype::FUNCTION)))
					return false;

				TArray<CDatum> ArgTypes;
				for (int i = 0; i + 1 < Pattern.Args.GetCount(); i++)
					{
					CDatum dArgType;
					if (!EvalTypeExpr(Pattern.Args[i], Ctx, &dArgType, NULL))
						return false;

					ArgTypes.Insert(dArgType);
					}

				CDatum dReturnType;
				if (!ActualType.CanBeCalledWith(CDatum(), ArgTypes, TArray<CDatum>(), &dReturnType))
					return false;

				return MatchTypeExpr(Pattern.Args[Pattern.Args.GetCount() - 1], dReturnType, Ctx);
				}

			default:
				{
				CDatum dRequiredType;
				if (!EvalTypeExpr(Pattern, Ctx, &dRequiredType, NULL))
					return false;

				const IDatatype& RequiredType = dRequiredType;
				return (Ctx.bCoerce ? RequiredType.CanBeConstructedFrom(dActualType) : ActualType.IsA(RequiredType));
				}
			}
		}

	bool EvalLiteralExpr (const CDatatypeGenericFunction::STypeExpr& Expr, const SMatchCtx& Ctx, CDatum* retdValue)
		{
		switch (Expr.iType)
			{
			case CDatatypeGenericFunction::ETypeExpr::ValueRef:
				{
				CDatum* pValue = Ctx.LiteralBindings.GetAt(Expr.sValue);
				if (!pValue)
					return false;

				*retdValue = *pValue;
				return true;
				}

			case CDatatypeGenericFunction::ETypeExpr::Concrete:
				*retdValue = Expr.dType;
				return !Expr.dType.IsNil();

			case CDatatypeGenericFunction::ETypeExpr::TypeVar:
				{
				CDatum* pType = Ctx.Bindings.GetAt(Expr.sValue);
				if (!pType)
					return false;

				*retdValue = *pType;
				return true;
				}

			default:
				return EvalTypeExpr(Expr, Ctx, retdValue, NULL);
			}
		}

	bool EvalTypeExpr (const CDatatypeGenericFunction::STypeExpr& Expr, const SMatchCtx& Ctx, CDatum* retdType, CString* retsError)
		{
		switch (Expr.iType)
			{
			case CDatatypeGenericFunction::ETypeExpr::Concrete:
				*retdType = (Expr.dType.IsNil() ? CAEONTypes::Get(IDatatype::ANY) : Expr.dType);
				return true;

			case CDatatypeGenericFunction::ETypeExpr::TypeVar:
				{
				CDatum* pType = Ctx.Bindings.GetAt(Expr.sValue);
				*retdType = (pType ? *pType : CAEONTypes::Get(IDatatype::ANY));
				return true;
				}

			case CDatatypeGenericFunction::ETypeExpr::ValueRef:
				{
				CDatum* pValue = Ctx.LiteralBindings.GetAt(Expr.sValue);
				if (pValue && pValue->GetBasicType() == CDatum::typeDatatype)
					*retdType = *pValue;
				else
					*retdType = CAEONTypes::Get(IDatatype::ANY);

				return true;
				}

			case CDatatypeGenericFunction::ETypeExpr::ArrayOf:
				{
				CDatum dElementType;
				if (!EvalTypeExpr(Expr.Args[0], Ctx, &dElementType, retsError))
					return false;

				*retdType = CAEONTypes::CreateArray(NULL_STR, dElementType);
				return true;
				}

			case CDatatypeGenericFunction::ETypeExpr::TableOf:
				{
				CDatum dSchema;
				if (!EvalTypeExpr(Expr.Args[0], Ctx, &dSchema, retsError))
					return false;

				if (((const IDatatype&)dSchema).GetClass() != IDatatype::ECategory::Schema)
					*retdType = CAEONTypes::Get(IDatatype::ANY);
				else
					*retdType = CAEONTypes::FindTableOrAdd(dSchema);

				return true;
				}

			case CDatatypeGenericFunction::ETypeExpr::DictionaryOf:
				{
				CDatum dKeyType;
				CDatum dElementType;
				if (!EvalTypeExpr(Expr.Args[0], Ctx, &dKeyType, retsError)
						|| !EvalTypeExpr(Expr.Args[1], Ctx, &dElementType, retsError))
					return false;

				*retdType = CAEONTypes::CreateDictionary(NULL_STR, dKeyType, dElementType);
				return true;
				}

			case CDatatypeGenericFunction::ETypeExpr::Nullable:
				{
				CDatum dType;
				if (!EvalTypeExpr(Expr.Args[0], Ctx, &dType, retsError))
					return false;

				*retdType = CAEONTypes::CreateNullableType(NULL_STR, dType);
				return true;
				}

			case CDatatypeGenericFunction::ETypeExpr::NonNullable:
				{
				CDatum dType;
				if (!EvalTypeExpr(Expr.Args[0], Ctx, &dType, retsError))
					return false;

				const IDatatype& Type = dType;
				*retdType = (Type.IsNullable() ? Type.GetVariantType() : dType);
				return true;
				}

			case CDatatypeGenericFunction::ETypeExpr::ElementType:
				{
				CDatum dType;
				if (!EvalTypeExpr(Expr.Args[0], Ctx, &dType, retsError))
					return false;

				*retdType = ((const IDatatype&)dType).GetElementType();
				if (retdType->IsNil())
					*retdType = CAEONTypes::Get(IDatatype::ANY);
				return true;
				}

			case CDatatypeGenericFunction::ETypeExpr::KeyType:
				{
				CDatum dType;
				if (!EvalTypeExpr(Expr.Args[0], Ctx, &dType, retsError))
					return false;

				*retdType = ((const IDatatype&)dType).GetKeyType();
				if (retdType->IsNil())
					*retdType = CAEONTypes::Get(IDatatype::ANY);
				return true;
				}

			case CDatatypeGenericFunction::ETypeExpr::Common:
				{
				TArray<CDatum> Types;
				for (int i = 0; i < Expr.Args.GetCount(); i++)
					{
					CDatum dType;
					if (!EvalTypeExpr(Expr.Args[i], Ctx, &dType, retsError))
						return false;

					Types.Insert(dType);
					}

				*retdType = CAEONTypes::GetCompatibleType(Types);
				return true;
				}

			case CDatatypeGenericFunction::ETypeExpr::EvalType:
				{
				CDatum dExprValue;
				CDatum dSchema;
				if (!EvalLiteralExpr(Expr.Args[0], Ctx, &dExprValue)
						|| !EvalTypeExpr(Expr.Args[1], Ctx, &dSchema, retsError))
					{
					*retdType = CAEONTypes::Get(IDatatype::ANY);
					return true;
					}

				const CAEONExpression* pExpr = dExprValue.GetQueryInterface();
				if (!pExpr)
					{
					*retdType = CAEONTypes::Get(IDatatype::ANY);
					return true;
					}

				*retdType = CAEONExpression::CalcEvalType(*pExpr, dSchema);
				return true;
				}

			case CDatatypeGenericFunction::ETypeExpr::FunctionOf:
				{
				if (Expr.Args.GetCount() == 0)
					{
					*retdType = CAEONTypes::Get(IDatatype::FUNCTION);
					return true;
					}

				TArray<IDatatype::SArgDesc> Args;
				for (int i = 0; i + 1 < Expr.Args.GetCount(); i++)
					{
					CDatum dArgType;
					if (!EvalTypeExpr(Expr.Args[i], Ctx, &dArgType, retsError))
						return false;

					IDatatype::SArgDesc Arg;
					Arg.sID = strPattern("arg%d", i + 1);
					Arg.dType = dArgType;
					Args.Insert(Arg);
					}

				CDatum dReturnType;
				if (!EvalTypeExpr(Expr.Args[Expr.Args.GetCount() - 1], Ctx, &dReturnType, retsError))
					return false;

				IDatatype::SReturnTypeDesc Return;
				Return.iType = IDatatype::EReturnDescType::Type;
				Return.dType = dReturnType;

				*retdType = CAEONTypes::CreateFunction(NULL_STR, Return, std::move(Args));
				return true;
				}

			case CDatatypeGenericFunction::ETypeExpr::Error:
				if (retsError)
					*retsError = Expr.sValue;
				return false;

			default:
				*retdType = CAEONTypes::Get(IDatatype::ANY);
				return true;
			}
		}

	bool MatchConstraint (const CDatatypeGenericFunction::SConstraintDesc& Constraint, SMatchCtx& Ctx)
		{
		CDatum dLeft;
		if (!EvalLiteralExpr(Constraint.Left, Ctx, &dLeft))
			return false;

		switch (Constraint.iOp)
			{
			case CDatatypeGenericFunction::EConstraintOp::Equal:
				{
				CDatum dRight;
				if (!EvalLiteralExpr(Constraint.Right, Ctx, &dRight))
					return false;

				if (dLeft.GetBasicType() == CDatum::typeDatatype && dRight.GetBasicType() == CDatum::typeDatatype)
					return SameType(dLeft, dRight);
				else
					return dLeft.OpIsEqual(dRight);
				}

			case CDatatypeGenericFunction::EConstraintOp::IsA:
				if (dLeft.GetBasicType() != CDatum::typeDatatype)
					return false;

				return MatchTypeExpr(Constraint.Right, dLeft, Ctx);

			case CDatatypeGenericFunction::EConstraintOp::IsAssignableTo:
				{
				if (dLeft.GetBasicType() != CDatum::typeDatatype)
					return false;

				CDatum dRight;
				if (!EvalTypeExpr(Constraint.Right, Ctx, &dRight, NULL))
					return false;

				const IDatatype& Right = dRight;
				return Right.CanBeConstructedFrom(dLeft);
				}

			default:
				return false;
			}
		}

	class CGenericFunctionParser
		{
		public:
			bool Parse (CStringView sSource, TArray<CDatatypeGenericFunction::SSignatureDesc>& retSignatures, CString* retsError = NULL)
				{
				m_sSource = CString(sSource);

				const char* pStart = m_sSource.GetParsePointer();
				const char* pPos = pStart;
				const char* pEnd = pStart + m_sSource.GetLength();

				while (pPos < pEnd)
					{
					SkipWhitespace(pPos, pEnd);
					if (pPos == pEnd)
						break;

					if (*pPos != '(')
						return Error(retsError);

					int iOpen = (int)(pPos - pStart);
					int iClose = FindMatching(m_sSource, iOpen, '(', ')');
					if (iClose == -1)
						return Error(retsError);

					CString sParams(pStart + iOpen + 1, iClose - iOpen - 1);
					pPos = pStart + iClose + 1;
					SkipWhitespace(pPos, pEnd);

					if (pPos + 1 >= pEnd || pPos[0] != '-' || pPos[1] != '>')
						return Error(retsError);

					pPos += 2;
					const char* pReturnStart = pPos;
					if (!FindOverloadEnd(pPos, pEnd))
						return Error(retsError);

					CString sReturn(pReturnStart, pPos - pReturnStart);

					CDatatypeGenericFunction::SSignatureDesc Signature;
					if (!ParseSignatureBody(sParams, Signature, retsError)
							|| !ParseTypeExpr(sReturn, Signature.Return, retsError)
							|| !ValidateValueRefs(Signature, retsError))
						return false;

					retSignatures.Insert(Signature);

					SkipWhitespace(pPos, pEnd);
					if (pPos < pEnd)
						{
						if (*pPos != '|')
							return Error(retsError);
						pPos++;
						}
					}

				return (retSignatures.GetCount() > 0);
				}

		private:
			static void SkipWhitespace (const char*& pPos, const char* pEnd)
				{
				while (pPos < pEnd && strIsWhitespace(pPos))
					pPos++;
				}

			bool Error (CString* retsError = NULL) const
				{
				if (retsError)
					*retsError = ERR_GENERIC_PARSE;
				return false;
				}

			bool ErrorUnknownValueRef (const CString& sName, CString* retsError = NULL) const
				{
				if (retsError)
					*retsError = strPattern(ERR_UNKNOWN_VALUE_REF, sName);
				return false;
				}

			bool FindOverloadEnd (const char*& pPos, const char* pEnd) const
				{
				int iParenDepth = 0;
				int iBracketDepth = 0;
				int iAngleDepth = 0;
				bool bInString = false;

				while (pPos < pEnd)
					{
					char chChar = *pPos;
					if (bInString)
						{
						if (chChar == '\\' && pPos + 1 < pEnd)
							pPos++;
						else if (chChar == '"')
							bInString = false;
						}
					else if (chChar == '"')
						bInString = true;
					else if (chChar == '(')
						iParenDepth++;
					else if (chChar == ')')
						{
						if (iParenDepth > 0)
							iParenDepth--;
						}
					else if (chChar == '[')
						iBracketDepth++;
					else if (chChar == ']')
						iBracketDepth--;
					else if (chChar == '<')
						iAngleDepth++;
					else if (chChar == '>')
						iAngleDepth--;
					else if (chChar == '|' && iParenDepth == 0 && iBracketDepth == 0 && iAngleDepth == 0)
						return true;

					pPos++;
					}

				return true;
				}

			bool ParseSignatureBody (const CString& sBody, CDatatypeGenericFunction::SSignatureDesc& retSignature, CString* retsError)
				{
				CString sParams;
				TArray<CString> Constraints;

				const char* pStart = sBody.GetParsePointer();
				const char* pPos = pStart;
				const char* pEnd = pStart + sBody.GetLength();
				const char* pConstraintStart = NULL;
				int iParenDepth = 0;
				int iBracketDepth = 0;
				int iAngleDepth = 0;
				bool bInString = false;

				while (pPos < pEnd)
					{
					char chChar = *pPos;
					if (bInString)
						{
						if (chChar == '\\' && pPos + 1 < pEnd)
							pPos++;
						else if (chChar == '"')
							bInString = false;
						}
					else if (chChar == '"')
						bInString = true;
					else if (chChar == '(')
						iParenDepth++;
					else if (chChar == ')')
						iParenDepth--;
					else if (chChar == '[')
						iBracketDepth++;
					else if (chChar == ']')
						iBracketDepth--;
					else if (chChar == '<')
						iAngleDepth++;
					else if (chChar == '>')
						iAngleDepth--;
					else if (iParenDepth == 0 && iBracketDepth == 0 && iAngleDepth == 0
							&& pPos + 5 <= pEnd
							&& strEqualsNoCase(CString(pPos, 5), STR_CDATATYPE_GENERIC_FUNCTION_WHERE)
							&& IsKeywordBoundary(pStart, pPos, pEnd, 5))
						{
						if (pConstraintStart)
							Constraints.Insert(Trim(CString(pConstraintStart, pPos - pConstraintStart)));
						else
							sParams = Trim(CString(pStart, pPos - pStart));

						pPos += 5;
						SkipWhitespace(pPos, pEnd);
						pConstraintStart = pPos;
						continue;
						}

					pPos++;
					}

				if (pConstraintStart)
					Constraints.Insert(Trim(CString(pConstraintStart, pEnd - pConstraintStart)));
				else
					sParams = Trim(sBody);

				if (!ParseParams(sParams, retSignature.Params, retsError))
					return false;

				retSignature.Constraints.InsertEmpty(Constraints.GetCount());
				for (int i = 0; i < Constraints.GetCount(); i++)
					if (!ParseConstraint(Constraints[i], retSignature.Constraints[i], retsError))
						return false;

				return true;
				}

			bool ParseParams (const CString& sParams, TArray<CDatatypeGenericFunction::SParamDesc>& retParams, CString* retsError)
				{
				CString sTrimmed = Trim(sParams);
				if (sTrimmed.IsEmpty())
					return true;

				TArray<CString> Parts = SplitTopLevel(sTrimmed, ',');
				for (int i = 0; i < Parts.GetCount(); i++)
					{
					CDatatypeGenericFunction::SParamDesc Param;
					if (!ParseParam(Parts[i], Param, retsError))
						return false;

					if (strEqualsNoCase(Param.sName, STR_CDATATYPE_GENERIC_FUNCTION_THIS) && i != 0)
						return Error(retsError);

					retParams.Insert(Param);
					}

				return true;
				}

			bool ParseParam (const CString& sParam, CDatatypeGenericFunction::SParamDesc& retParam, CString* retsError)
				{
				CString sLeft;
				CString sRight;
				if (!SplitFirstTopLevel(sParam, ':', &sLeft, &sRight))
					return Error(retsError);

				CString sVar;
				if (IsTypeVar(sLeft, &sVar))
					{
					retParam.sCapture = sVar;
					}
				else
					{
					retParam.sName = Trim(sLeft);
					if (retParam.sName.IsEmpty())
						return Error(retsError);
					}

				return ParseTypeExpr(sRight, retParam.Type, retsError);
				}

			bool ParseConstraint (const CString& sConstraint, CDatatypeGenericFunction::SConstraintDesc& retConstraint, CString* retsError)
				{
				CString sTrimmed = Trim(sConstraint);
				const char* pStart = sTrimmed.GetParsePointer();
				const char* pPos = pStart;
				const char* pEnd = pStart + sTrimmed.GetLength();
				int iParenDepth = 0;
				int iBracketDepth = 0;
				int iAngleDepth = 0;
				bool bInString = false;

				while (pPos < pEnd)
					{
					char chChar = *pPos;
					if (bInString)
						{
						if (chChar == '\\' && pPos + 1 < pEnd)
							pPos++;
						else if (chChar == '"')
							bInString = false;
						}
					else if (chChar == '"')
						bInString = true;
					else if (chChar == '(')
						iParenDepth++;
					else if (chChar == ')')
						iParenDepth--;
					else if (chChar == '[')
						iBracketDepth++;
					else if (chChar == ']')
						iBracketDepth--;
					else if (chChar == '<')
						iAngleDepth++;
					else if (chChar == '>')
						iAngleDepth--;
					else if (iParenDepth == 0 && iBracketDepth == 0 && iAngleDepth == 0)
						{
						int iOpLen = 0;
						if (pPos + 2 <= pEnd && pPos[0] == '=' && pPos[1] == '=')
							{
							retConstraint.iOp = CDatatypeGenericFunction::EConstraintOp::Equal;
							iOpLen = 2;
							}
						else if (pPos + 14 <= pEnd
								&& strEqualsNoCase(CString(pPos, 14), STR_CDATATYPE_GENERIC_FUNCTION_IS_ASSIGNABLE_TO)
								&& IsKeywordBoundary(pStart, pPos, pEnd, 14))
							{
							retConstraint.iOp = CDatatypeGenericFunction::EConstraintOp::IsAssignableTo;
							iOpLen = 14;
							}
						else if (pPos + 3 <= pEnd
								&& strEqualsNoCase(CString(pPos, 3), STR_CDATATYPE_GENERIC_FUNCTION_ISA)
								&& IsKeywordBoundary(pStart, pPos, pEnd, 3))
							{
							retConstraint.iOp = CDatatypeGenericFunction::EConstraintOp::IsA;
							iOpLen = 3;
							}

						if (iOpLen)
							{
							return (ParseTypeExpr(CString(pStart, pPos - pStart), retConstraint.Left, retsError)
									&& ParseTypeExpr(CString(pPos + iOpLen, pEnd - (pPos + iOpLen)), retConstraint.Right, retsError));
							}
						}

					pPos++;
					}

				return Error(retsError);
				}

			bool ParseTypeExpr (const CString& sExpr, CDatatypeGenericFunction::STypeExpr& retExpr, CString* retsError)
				{
				CString sTrimmed = Trim(sExpr);
				if (IsValueRefName(sTrimmed) && islower((unsigned char)*sTrimmed.GetParsePointer()))
					{
					retExpr.iType = CDatatypeGenericFunction::ETypeExpr::ValueRef;
					retExpr.sValue = sTrimmed;
					return true;
					}

				CString sVar;
				if (IsTypeVar(sTrimmed, &sVar))
					{
					retExpr.iType = CDatatypeGenericFunction::ETypeExpr::TypeVar;
					retExpr.sValue = sVar;
					return true;
					}

				if (StartsWithNoCase(sTrimmed, STR_CDATATYPE_GENERIC_FUNCTION_ARRAY_OF))
					{
					retExpr.iType = CDatatypeGenericFunction::ETypeExpr::ArrayOf;
					retExpr.Args.Insert();
					return ParseTypeExpr(strSubString(sTrimmed, 9), retExpr.Args[0], retsError);
					}
				else if (StartsWithNoCase(sTrimmed, STR_CDATATYPE_GENERIC_FUNCTION_TABLE_OF))
					{
					retExpr.iType = CDatatypeGenericFunction::ETypeExpr::TableOf;
					retExpr.Args.Insert();
					return ParseTypeExpr(strSubString(sTrimmed, 9), retExpr.Args[0], retsError);
					}
				else if (StartsWithNoCase(sTrimmed, STR_CDATATYPE_GENERIC_FUNCTION_DICTIONARY))
					{
					int iClose = FindMatching(sTrimmed, 10, '[', ']');
					if (iClose == -1)
						return Error(retsError);

					CString sRest = Trim(strSubString(sTrimmed, iClose + 1));
					if (!StartsWithNoCase(sRest, STR_CDATATYPE_GENERIC_FUNCTION_OF))
						return Error(retsError);

					retExpr.iType = CDatatypeGenericFunction::ETypeExpr::DictionaryOf;
					retExpr.Args.InsertEmpty(2);
					return (ParseTypeExpr(strSubString(sTrimmed, 11, iClose - 11), retExpr.Args[0], retsError)
							&& ParseTypeExpr(strSubString(sRest, 3), retExpr.Args[1], retsError));
					}
				else if (StartsWithNoCase(sTrimmed, STR_CDATATYPE_GENERIC_FUNCTION_DICTIONARY_OF))
					{
					retExpr.iType = CDatatypeGenericFunction::ETypeExpr::DictionaryOf;
					retExpr.Args.InsertEmpty(2);
					retExpr.Args[0].iType = CDatatypeGenericFunction::ETypeExpr::Concrete;
					retExpr.Args[0].dType = CAEONTypes::Get(IDatatype::ANY);
					return ParseTypeExpr(strSubString(sTrimmed, 14), retExpr.Args[1], retsError);
					}
				else if (sTrimmed.GetLength() > 1 && sTrimmed.GetParsePointer()[sTrimmed.GetLength() - 1] == '?')
					{
					retExpr.iType = CDatatypeGenericFunction::ETypeExpr::Nullable;
					retExpr.Args.Insert();
					return ParseTypeExpr(strSubString(sTrimmed, 0, sTrimmed.GetLength() - 1), retExpr.Args[0], retsError);
					}
				else if (ParseFunctionTypeExpr(sTrimmed, retExpr, retsError))
					return true;
				else if (ParseCallExpr(sTrimmed, retExpr, retsError))
					return true;
				else
					{
					retExpr.iType = CDatatypeGenericFunction::ETypeExpr::Concrete;
					retExpr.dType = ResolveTypeName(sTrimmed);
					if (!retExpr.dType.IsNil())
						return true;

					if (IsValueRefName(sTrimmed))
						{
						retExpr.iType = CDatatypeGenericFunction::ETypeExpr::ValueRef;
						retExpr.sValue = sTrimmed;
						return true;
						}

					return Error(retsError);
					}
				}

			bool ParseFunctionTypeExpr (const CString& sExpr, CDatatypeGenericFunction::STypeExpr& retExpr, CString* retsError)
				{
				const char* pStart = sExpr.GetParsePointer();
				const char* pEnd = pStart + sExpr.GetLength();
				const char* pPos = pStart;
				const int iKeywordLen = 8;

				if (pEnd - pStart < iKeywordLen
						|| !strEqualsNoCase(CString(pStart, iKeywordLen), STR_CDATATYPE_GENERIC_FUNCTION_FUNCTION_9EF93D81)
						|| !IsKeywordBoundary(pStart, pStart, pEnd, iKeywordLen))
					return false;

				pPos += iKeywordLen;
				SkipWhitespace(pPos, pEnd);
				if (pPos == pEnd || *pPos != '(')
					return false;

				int iOpen = (int)(pPos - pStart);
				int iClose = FindMatching(sExpr, iOpen, '(', ')');
				if (iClose == -1)
					return Error(retsError);

				pPos = pStart + iClose + 1;
				SkipWhitespace(pPos, pEnd);
				if (pPos == pEnd || *pPos != ':')
					return Error(retsError);

				CString sParams(pStart + iOpen + 1, iClose - iOpen - 1);
				CString sReturn(pPos + 1, pEnd - pPos - 1);

				CDatatypeGenericFunction::SSignatureDesc Signature;
				if (!ParseSignatureBody(sParams, Signature, retsError))
					return false;

				if (Signature.Constraints.GetCount() != 0)
					return Error(retsError);

				retExpr.iType = CDatatypeGenericFunction::ETypeExpr::FunctionOf;
				retExpr.Args.InsertEmpty(Signature.Params.GetCount() + 1);
				for (int i = 0; i < Signature.Params.GetCount(); i++)
					retExpr.Args[i] = Signature.Params[i].Type;

				return ParseTypeExpr(sReturn, retExpr.Args[Signature.Params.GetCount()], retsError);
				}

			bool ParseCallExpr (const CString& sExpr, CDatatypeGenericFunction::STypeExpr& retExpr, CString* retsError)
				{
				const char* pStart = sExpr.GetParsePointer();
				const char* pEnd = pStart + sExpr.GetLength();
				const char* pOpen = pStart;
				while (pOpen < pEnd && IsIdentChar(*pOpen))
					pOpen++;

				if (pOpen == pStart || pOpen == pEnd || *pOpen != '(')
					return false;

				int iOpen = (int)(pOpen - pStart);
				int iClose = FindMatching(sExpr, iOpen, '(', ')');
				if (iClose != sExpr.GetLength() - 1)
					return false;

				CString sName(pStart, pOpen - pStart);
				CString sArgs(pOpen + 1, iClose - iOpen - 1);

				if (strEqualsNoCase(sName, STR_CDATATYPE_GENERIC_FUNCTION_NULLABLE))
					retExpr.iType = CDatatypeGenericFunction::ETypeExpr::Nullable;
				else if (strEqualsNoCase(sName, STR_CDATATYPE_GENERIC_FUNCTION_NONNULLABLE))
					retExpr.iType = CDatatypeGenericFunction::ETypeExpr::NonNullable;
				else if (strEqualsNoCase(sName, STR_CDATATYPE_GENERIC_FUNCTION_ELEMENTTYPE))
					retExpr.iType = CDatatypeGenericFunction::ETypeExpr::ElementType;
				else if (strEqualsNoCase(sName, STR_CDATATYPE_GENERIC_FUNCTION_KEYTYPE))
					retExpr.iType = CDatatypeGenericFunction::ETypeExpr::KeyType;
				else if (strEqualsNoCase(sName, STR_CDATATYPE_GENERIC_FUNCTION_COMMON))
					retExpr.iType = CDatatypeGenericFunction::ETypeExpr::Common;
				else if (strEqualsNoCase(sName, STR_CDATATYPE_GENERIC_FUNCTION_EVALTYPE))
					retExpr.iType = CDatatypeGenericFunction::ETypeExpr::EvalType;
				else if (strEqualsNoCase(sName, STR_CDATATYPE_GENERIC_FUNCTION_ERROR))
					{
					retExpr.iType = CDatatypeGenericFunction::ETypeExpr::Error;
					retExpr.sValue = ParseStringLiteral(sArgs);
					return true;
					}
				else
					return false;

				TArray<CString> Args = SplitTopLevel(sArgs, ',');
				if ((retExpr.iType == CDatatypeGenericFunction::ETypeExpr::Common && Args.GetCount() < 2)
						|| (retExpr.iType != CDatatypeGenericFunction::ETypeExpr::Common && Args.GetCount() != 1 && retExpr.iType != CDatatypeGenericFunction::ETypeExpr::EvalType)
						|| (retExpr.iType == CDatatypeGenericFunction::ETypeExpr::EvalType && Args.GetCount() != 2))
					return Error(retsError);

				retExpr.Args.InsertEmpty(Args.GetCount());
				for (int i = 0; i < Args.GetCount(); i++)
					if (!ParseTypeExpr(Args[i], retExpr.Args[i], retsError))
						return false;

				return true;
				}

			bool ValidateTypeExprValueRefs (const CDatatypeGenericFunction::STypeExpr& Expr, const CDatatypeGenericFunction::SSignatureDesc& Signature, CString* retsError) const
				{
				if (Expr.iType == CDatatypeGenericFunction::ETypeExpr::ValueRef
						&& !IsParamName(Signature, Expr.sValue))
					return ErrorUnknownValueRef(Expr.sValue, retsError);

				for (int i = 0; i < Expr.Args.GetCount(); i++)
					if (!ValidateTypeExprValueRefs(Expr.Args[i], Signature, retsError))
						return false;

				return true;
				}

			bool ValidateValueRefs (const CDatatypeGenericFunction::SSignatureDesc& Signature, CString* retsError) const
				{
				for (int i = 0; i < Signature.Constraints.GetCount(); i++)
					{
					if (!ValidateTypeExprValueRefs(Signature.Constraints[i].Left, Signature, retsError)
							|| !ValidateTypeExprValueRefs(Signature.Constraints[i].Right, Signature, retsError))
						return false;
					}

				return ValidateTypeExprValueRefs(Signature.Return, Signature, retsError);
				}

			static bool IsParamName (const CDatatypeGenericFunction::SSignatureDesc& Signature, const CString& sName)
				{
				for (int i = 0; i < Signature.Params.GetCount(); i++)
					if (strEqualsNoCase(Signature.Params[i].sName, sName))
						return true;

				return false;
				}

			CString m_sSource;
		};

	bool MatchSignature (const CDatatypeGenericFunction::SSignatureDesc& Signature, CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, bool bCoerce, CDatum* retdReturnType, CString* retsError)
		{
		SMatchCtx Ctx;
		Ctx.pArgLiteralTypes = &ArgLiteralTypes;
		Ctx.bCoerce = bCoerce;

		int iArg = 0;
		for (int i = 0; i < Signature.Params.GetCount(); i++)
			{
			const auto& Param = Signature.Params[i];
			if (strEqualsNoCase(Param.sName, STR_CDATATYPE_GENERIC_FUNCTION_THIS))
				{
				if (i != 0 || dThisType.IsNil())
					return false;

				if (!Param.sCapture.IsEmpty() && !BindTypeVar(Param.sCapture, dThisType, Ctx))
					return false;

				if (!MatchTypeExpr(Param.Type, dThisType, Ctx))
					return false;
				}
			else
				{
				if (iArg >= ArgTypes.GetCount())
					return false;

				if (!Param.sName.IsEmpty() && iArg < ArgLiteralTypes.GetCount())
					Ctx.LiteralBindings.SetAt(Param.sName, ArgLiteralTypes[iArg]);

				if (!Param.sCapture.IsEmpty() && !BindTypeVar(Param.sCapture, ArgTypes[iArg], Ctx))
					return false;

				if (!MatchTypeExpr(Param.Type, ArgTypes[iArg], Ctx))
					return false;

				iArg++;
				}
			}

		if (iArg != ArgTypes.GetCount())
			return false;

		for (int i = 0; i < Signature.Constraints.GetCount(); i++)
			if (!MatchConstraint(Signature.Constraints[i], Ctx))
				return false;

		if (retdReturnType)
			return EvalTypeExpr(Signature.Return, Ctx, retdReturnType, retsError);
		else
			return true;
		}
	}

CDatatypeGenericFunction::CDatatypeGenericFunction (CStringView sSource) : IDatatype(false, CAEONTypes::MakeFullyQualifiedFunctionName())
	{
	CString sError;
	if (!InitFromSource(sSource, &sError))
		throw CException(errFail);
	}

bool CDatatypeGenericFunction::InitFromSource (CStringView sSource, CString* retsError)
	{
	m_sSource = CString(sSource);
	m_Signatures.DeleteAll();

	CGenericFunctionParser Parser;
	return Parser.Parse(sSource, m_Signatures, retsError);
	}

void CDatatypeGenericFunction::OnAccumulateTypesUsed (TSortMap<CString, CDatum>& retTypes) const
	{
		for (int i = 0; i < m_Signatures.GetCount(); i++)
			{
			for (int j = 0; j < m_Signatures[i].Params.GetCount(); j++)
				AccumulateExprTypes(m_Signatures[i].Params[j].Type, retTypes);

			for (int j = 0; j < m_Signatures[i].Constraints.GetCount(); j++)
				{
				AccumulateExprTypes(m_Signatures[i].Constraints[j].Left, retTypes);
				AccumulateExprTypes(m_Signatures[i].Constraints[j].Right, retTypes);
				}

			AccumulateExprTypes(m_Signatures[i].Return, retTypes);
			}
	}

bool CDatatypeGenericFunction::OnCanBeCalledWith (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, CDatum* retdReturnType, CString* retsError) const
	{
	for (int i = 0; i < m_Signatures.GetCount(); i++)
		if (MatchSignature(m_Signatures[i], dThisType, ArgTypes, ArgLiteralTypes, false, retdReturnType, retsError))
			return true;

	for (int i = 0; i < m_Signatures.GetCount(); i++)
		if (MatchSignature(m_Signatures[i], dThisType, ArgTypes, ArgLiteralTypes, true, retdReturnType, retsError))
			return true;

	if (retsError && retsError->IsEmpty())
		*retsError = ERR_NO_MATCHING_SIGNATURES;

	return false;
	}

bool CDatatypeGenericFunction::OnCanBeCalledWithArgCount (CDatum dThisType, int iArgCount, CDatum* retdReturnType, CString* retsError) const
	{
	for (int i = 0; i < m_Signatures.GetCount(); i++)
		{
		int iCount = 0;
		bool bValid = true;
		for (int j = 0; j < m_Signatures[i].Params.GetCount(); j++)
			{
			const auto& Param = m_Signatures[i].Params[j];
			if (strEqualsNoCase(Param.sName, STR_CDATATYPE_GENERIC_FUNCTION_THIS))
				{
				if (j != 0 || dThisType.IsNil())
					{
					bValid = false;
					break;
					}
				}
			else
				iCount++;
			}

		if (bValid && iCount == iArgCount)
			{
			if (retdReturnType)
				{
				SMatchCtx Ctx;
				CDatum dReturnType;
				if (EvalTypeExpr(m_Signatures[i].Return, Ctx, &dReturnType, retsError))
					*retdReturnType = dReturnType;
				else
					*retdReturnType = CAEONTypes::Get(IDatatype::ANY);
				}

			return true;
			}
		}

	if (retsError)
		*retsError = ERR_NO_MATCHING_SIGNATURES;

	return false;
	}

bool CDatatypeGenericFunction::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion)
	{
	return InitFromSource(CString::Deserialize(Stream));
	}

bool CDatatypeGenericFunction::OnDeserializeAEON (IByteStream& Stream, DWORD dwVersion, CAEONSerializedMap &Serialized)
	{
	return InitFromSource(CString::Deserialize(Stream));
	}

bool CDatatypeGenericFunction::OnEquals (const IDatatype &Src) const
	{
	const auto& Other = (const CDatatypeGenericFunction&)Src;
	return strEquals(m_sSource, Other.m_sSource);
	}

bool CDatatypeGenericFunction::OnIsA (const IDatatype &Type) const
	{
	return (Type.GetCoreType() == IDatatype::FUNCTION);
	}

void CDatatypeGenericFunction::OnMark ()
	{
		for (int i = 0; i < m_Signatures.GetCount(); i++)
			{
			for (int j = 0; j < m_Signatures[i].Params.GetCount(); j++)
				MarkExpr(m_Signatures[i].Params[j].Type);

			for (int j = 0; j < m_Signatures[i].Constraints.GetCount(); j++)
				{
				MarkExpr(m_Signatures[i].Constraints[j].Left);
				MarkExpr(m_Signatures[i].Constraints[j].Right);
				}

			MarkExpr(m_Signatures[i].Return);
			}
	}

void CDatatypeGenericFunction::OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	m_sSource.Serialize(Stream);
	}
