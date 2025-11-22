//	CAEONExpression.cpp
//
//	CAEONExpression classes
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_LEFT,						"left");
DECLARE_CONST_STRING(FIELD_OP,							"op");
DECLARE_CONST_STRING(FIELD_RIGHT,						"right");

DECLARE_CONST_STRING(OP_ABS,							"abs");
DECLARE_CONST_STRING(OP_ADD,							"+");
DECLARE_CONST_STRING(OP_ALL,							"all");
DECLARE_CONST_STRING(OP_AND,							"&&");
DECLARE_CONST_STRING(OP_ANY,							"any");
DECLARE_CONST_STRING(OP_ARRAY,							"Array");
DECLARE_CONST_STRING(OP_AVERAGE,						"average");
DECLARE_CONST_STRING(OP_CEIL,							"ceil");
DECLARE_CONST_STRING(OP_CLEAN,							"clean");
DECLARE_CONST_STRING(OP_COUNT,							"count");
DECLARE_CONST_STRING(OP_DATE_TIME,						"DateTime");
DECLARE_CONST_STRING(OP_DEREF,							"[]");
DECLARE_CONST_STRING(OP_DIVIDE,							"/");
DECLARE_CONST_STRING(OP_EQUAL_TO,						"==");
DECLARE_CONST_STRING(OP_ERROR_LITERAL,					"error");
DECLARE_CONST_STRING(OP_FALSE,							"false");
DECLARE_CONST_STRING(OP_FIELD,							"columnRef");
DECLARE_CONST_STRING(OP_FIND,							"find");
DECLARE_CONST_STRING(OP_FIRST,							"first");
DECLARE_CONST_STRING(OP_FLOOR,							"floor");
DECLARE_CONST_STRING(OP_GREATER_THAN,					">");
DECLARE_CONST_STRING(OP_GREATER_THAN_OR_EQUAL_TO,		">=");
DECLARE_CONST_STRING(OP_IF,								"if");
DECLARE_CONST_STRING(OP_IN,								"in");
DECLARE_CONST_STRING(OP_INTEGER,						"Integer");
DECLARE_CONST_STRING(OP_LEFT,							"left");
DECLARE_CONST_STRING(OP_LENGTH,							"length");
DECLARE_CONST_STRING(OP_LESS_THAN,						"<");
DECLARE_CONST_STRING(OP_LESS_THAN_OR_EQUAL_TO,			"<=");
DECLARE_CONST_STRING(OP_LITERAL,						"literal");
DECLARE_CONST_STRING(OP_LOWERCASE,						"lowercase");
DECLARE_CONST_STRING(OP_MAX,							"max");
DECLARE_CONST_STRING(OP_MEDIAN,							"median");
DECLARE_CONST_STRING(OP_MEMBER,							".");
DECLARE_CONST_STRING(OP_MIN,							"min");
DECLARE_CONST_STRING(OP_MOD,							"%");
DECLARE_CONST_STRING(OP_MULTIPLY,						"*");
DECLARE_CONST_STRING(OP_NEGATE,							"unary-");
DECLARE_CONST_STRING(OP_NUMBER,							"Number");
DECLARE_CONST_STRING(OP_NOT,							"!");
DECLARE_CONST_STRING(OP_NOT_EQUAL_TO,					"!=");
DECLARE_CONST_STRING(OP_OR,								"||");
DECLARE_CONST_STRING(OP_POWER,							"^");
DECLARE_CONST_STRING(OP_REAL,							"Real");
DECLARE_CONST_STRING(OP_RIGHT,							"right");
DECLARE_CONST_STRING(OP_ROUND,							"round");
DECLARE_CONST_STRING(OP_SIGN,							"sign");
DECLARE_CONST_STRING(OP_SLICE,							"slice");
DECLARE_CONST_STRING(OP_STD_DEV,						"stdDev");
DECLARE_CONST_STRING(OP_STD_ERROR,						"stdError");
DECLARE_CONST_STRING(OP_STRING,							"String");
DECLARE_CONST_STRING(OP_SUBTRACT,						"-");
DECLARE_CONST_STRING(OP_SUM,							"sum");
DECLARE_CONST_STRING(OP_TIME_SPAN,						"TimeSpan");
DECLARE_CONST_STRING(OP_TRUE,							"true");
DECLARE_CONST_STRING(OP_UNIQUE_ARRAY,					"uniqueArray");
DECLARE_CONST_STRING(OP_UNIQUE_COUNT,					"uniqueCount");
DECLARE_CONST_STRING(OP_UPPERCASE,						"uppercase");

DECLARE_CONST_STRING(TYPENAME_EXPRESSION,				"aeonExpression");

const CAEONExpression CAEONExpression::Null;
const CAEONExpression::SNode CAEONExpression::m_NullNode;

const CString& CAEONExpression::GetTypename (void) const { return TYPENAME_EXPRESSION; }
	
//	Constructors ---------------------------------------------------------------

CDatum CAEONExpression::CreateBinaryOp (EOp iOp, const CAEONExpression& Left, const CAEONExpression& Right)
	{
	CAEONExpression* pExp = new CAEONExpression;
	pExp->m_Columns.GrowToFit(Left.GetColumnCount() + Right.GetColumnCount());
	pExp->m_Literals.GrowToFit(Left.GetLiteralCount() + Right.GetLiteralCount());

	pExp->m_Nodes.InsertEmpty(1 + Left.GetNodeCount() + Right.GetNodeCount());
	pExp->m_Nodes[0].iOp = iOp;					// Set the operation
	pExp->m_Nodes[0].iLeft = 1;					// Left node index
	pExp->m_Nodes[0].iRight = 1 + Left.GetNodeCount(); // Right node index
	pExp->m_iRoot = 0;							// Root node index

	//	Set the left and right nodes

	for (int i = 0; i < Left.GetNodeCount(); i++)
		{
		pExp->CopyNode(1 + i, Left, i, 1);
		}

	for (int i = 0; i < Right.GetNodeCount(); i++)
		{
		pExp->CopyNode(1 + Left.GetNodeCount() + i, Right, i, 1 + Left.GetNodeCount());
		}

	return CDatum(pExp);
	}

CDatum CAEONExpression::CreateBinaryOp (EOp iOp, const CAEONExpression& Left, CDatum dLiteral)
	{
	CAEONExpression* pExp = new CAEONExpression;
	pExp->m_Columns.GrowToFit(Left.GetColumnCount());
	pExp->m_Literals.GrowToFit(Left.GetLiteralCount() + 1);

	pExp->m_Nodes.InsertEmpty(1 + Left.GetNodeCount() + 1);
	pExp->m_Nodes[0].iOp = iOp;					// Set the operation
	pExp->m_Nodes[0].iLeft = 1;					// Left node index
	pExp->m_Nodes[0].iRight = 1 + Left.GetNodeCount(); // Right node index
	pExp->m_iRoot = 0;							// Root node index

	//	Set the left and right nodes

	for (int i = 0; i < Left.GetNodeCount(); i++)
		{
		pExp->CopyNode(1 + i, Left, i, 1);
		}

	SNode& RightNode = pExp->m_Nodes[1 + Left.GetNodeCount()];
	RightNode.iOp = EOp::Literal;				// Set the operation to Literal
	RightNode.iDataID = pExp->m_Literals.GetCount(); // Index of the literal in m_Literals
	pExp->m_Literals.Insert(dLiteral);			// Insert the literal

	return CDatum(pExp);
	}

CDatum CAEONExpression::CreateBinaryOp (EOp iOp, CDatum dLiteral, const CAEONExpression& Right)
	{
	//	For certain comparison operators we make sure the expression is always
	//	on the left.

	EOp iFlipOp = FlipOp(iOp);
	if (iFlipOp != EOp::None)
		return CreateBinaryOp(iFlipOp, Right, dLiteral);

	//	Otherwise, normal

	else
		{
		CAEONExpression* pExp = new CAEONExpression;
		pExp->m_Columns.GrowToFit(Right.GetColumnCount());
		pExp->m_Literals.GrowToFit(Right.GetLiteralCount() + 1);

		pExp->m_Nodes.InsertEmpty(1 + 1 + Right.GetNodeCount());
		pExp->m_Nodes[0].iOp = iOp;					// Set the operation
		pExp->m_Nodes[0].iLeft = 1;					// Left node index
		pExp->m_Nodes[0].iRight = 1 + 1;			// Right node index
		pExp->m_iRoot = 0;							// Root node index

		//	Set the left and right nodes

		SNode& LeftNode = pExp->m_Nodes[1];
		LeftNode.iOp = EOp::Literal;				// Set the operation to Literal
		LeftNode.iDataID = pExp->m_Literals.GetCount(); // Index of the literal in m_Literals
		pExp->m_Literals.Insert(dLiteral);			// Insert the literal


		for (int i = 0; i < Right.GetNodeCount(); i++)
			{
			pExp->CopyNode(1 + 1 + i, Right, i, 1 + 1);
			}

		return CDatum(pExp);
		}
	}

CDatum CAEONExpression::CreateIf (const CAEONExpression& Condition, const CAEONExpression& TrueExpr, const CAEONExpression& FalseExpr)
	{
	CAEONExpression* pExp = new CAEONExpression;
	pExp->m_Columns.GrowToFit(Condition.GetColumnCount() + TrueExpr.GetColumnCount() + FalseExpr.GetColumnCount());
	pExp->m_Literals.GrowToFit(Condition.GetLiteralCount() + TrueExpr.GetLiteralCount() + FalseExpr.GetLiteralCount());

	pExp->m_Nodes.InsertEmpty(1 + Condition.GetNodeCount() + TrueExpr.GetNodeCount() + FalseExpr.GetNodeCount());
	pExp->m_Nodes[0].iOp = EOp::If;				// Set the operation
	pExp->m_Nodes[0].iDataID = 1;				// Condition node index
	pExp->m_Nodes[0].iLeft = 1 + Condition.GetNodeCount(); // True node index
	pExp->m_Nodes[0].iRight = 1 + Condition.GetNodeCount() + TrueExpr.GetNodeCount(); // False node index
	pExp->m_iRoot = 0;							// Root node index

	//	Set the condition, true, and false nodes

	for (int i = 0; i < Condition.GetNodeCount(); i++)
		{
		pExp->CopyNode(1 + i, Condition, i, 1);
		}

	for (int i = 0; i < TrueExpr.GetNodeCount(); i++)
		{
		pExp->CopyNode(1 + Condition.GetNodeCount() + i, TrueExpr, i, 1 + Condition.GetNodeCount());
		}

	for (int i = 0; i < FalseExpr.GetNodeCount(); i++)
		{
		pExp->CopyNode(1 + Condition.GetNodeCount() + TrueExpr.GetNodeCount() + i, FalseExpr, i, 1 + Condition.GetNodeCount() + TrueExpr.GetNodeCount());
		}

	return CDatum(pExp);
	}

CDatum CAEONExpression::CreateTernaryOp (EOp iOp, const CAEONExpression& Arg1, const CAEONExpression& Arg2, const CAEONExpression& Arg3)
	{
	CAEONExpression* pExp = new CAEONExpression;
	pExp->m_Columns.GrowToFit(Arg1.GetColumnCount() + Arg2.GetColumnCount() + Arg3.GetColumnCount());
	pExp->m_Literals.GrowToFit(Arg1.GetLiteralCount() + Arg2.GetLiteralCount() + Arg3.GetLiteralCount());

	pExp->m_Nodes.InsertEmpty(1 + Arg1.GetNodeCount() + Arg2.GetNodeCount() + Arg3.GetNodeCount());
	pExp->m_Nodes[0].iOp = iOp;				// Set the operation
	pExp->m_Nodes[0].iLeft = 1;				// param1
	pExp->m_Nodes[0].iRight = 1 + Arg1.GetNodeCount(); // param2
	pExp->m_Nodes[0].iDataID = 1 + Arg1.GetNodeCount() + Arg2.GetNodeCount(); // param3
	pExp->m_iRoot = 0;							// Root node index

	//	Set the parameters

	for (int i = 0; i < Arg1.GetNodeCount(); i++)
		{
		pExp->CopyNode(1 + i, Arg1, i, 1);
		}

	for (int i = 0; i < Arg2.GetNodeCount(); i++)
		{
		pExp->CopyNode(1 + Arg1.GetNodeCount() + i, Arg2, i, 1 + Arg1.GetNodeCount());
		}

	for (int i = 0; i < Arg3.GetNodeCount(); i++)
		{
		pExp->CopyNode(1 + Arg1.GetNodeCount() + Arg2.GetNodeCount() + i, Arg3, i, 1 + Arg1.GetNodeCount() + Arg2.GetNodeCount());
		}

	return CDatum(pExp);
	}

CDatum CAEONExpression::CreateUnaryOp (EOp iOp, const CAEONExpression& Expr)
	{
	CAEONExpression* pExp = new CAEONExpression;
	pExp->m_Columns.GrowToFit(Expr.GetColumnCount());
	pExp->m_Literals.GrowToFit(Expr.GetLiteralCount());

	pExp->m_Nodes.InsertEmpty(1 + Expr.GetNodeCount());
	pExp->m_Nodes[0].iOp = iOp;					// Set the operation
	pExp->m_Nodes[0].iLeft = 1;					// Left node index
	pExp->m_Nodes[0].iRight = -1;				// No right node
	pExp->m_iRoot = 0;							// Root node index

	//	Set the left node

	for (int i = 0; i < Expr.GetNodeCount(); i++)
		{
		pExp->CopyNode(1 + i, Expr, i, 1);
		}

	return CDatum(pExp);
	}

CDatum CAEONExpression::CreateZeroaryOp (EOp iOp)
	{
	CAEONExpression* pExp = new CAEONExpression;
	pExp->m_Nodes.InsertEmpty(1);
	pExp->m_Nodes[0].iOp = iOp;
	pExp->m_iRoot = 0;							// Root node index

	return CDatum(pExp);
	}

void CAEONExpression::CopyNode (int iDestID, const CAEONExpression& Src, int iSrcID, int iOffset)
	{
	SNode& DestNode = m_Nodes[iDestID];
	const SNode& SrcNode = Src.GetNode(iSrcID);

	DestNode.iOp = SrcNode.iOp;				// Copy the operation
	switch (SrcNode.iOp)
		{
		case EOp::Column:
			DestNode.iDataID = m_Columns.GetCount();
			m_Columns.Insert(Src.GetColumnID(SrcNode.iDataID));
			break;

		case EOp::ErrorLiteral:
		case EOp::Literal:
			DestNode.iDataID = m_Literals.GetCount();
			m_Literals.Insert(Src.GetLiteral(SrcNode.iDataID));
			break;

		default:
			if (SrcNode.iLeft != -1)
				DestNode.iLeft = SrcNode.iLeft + iOffset; // Adjust left index
			else
				DestNode.iLeft = -1; // No left child

			if (SrcNode.iRight != -1)
				DestNode.iRight = SrcNode.iRight + iOffset; // Adjust right index
			else
				DestNode.iRight = -1; // No right child

			if (SrcNode.iDataID != -1)
				DestNode.iDataID = SrcNode.iDataID + iOffset; // Adjust data ID
			else
				DestNode.iDataID = -1; // No data ID
			break;
		}
	}

CDatum CAEONExpression::CreateError (CStringView sError)
	{
	CAEONExpression* pExp = new CAEONExpression;
	pExp->m_Literals.Insert(CDatum(sError));

	pExp->m_Nodes.InsertEmpty(1);
	pExp->m_Nodes[0].iOp = EOp::ErrorLiteral;
	pExp->m_Nodes[0].iDataID = 0;
	pExp->m_iRoot = 0;

	return CDatum(pExp);
	}

CDatum CAEONExpression::CreateFalse ()
	{
	CAEONExpression* pExp = new CAEONExpression;
	pExp->m_Nodes.InsertEmpty(1);
	pExp->m_Nodes[0].iOp = EOp::False;			// Set the operation to False
	pExp->m_iRoot = 0;							// Root node index

	return CDatum(pExp);
	}

CDatum CAEONExpression::CreateColumnRef (const CString& sField)
	{
	CAEONExpression* pExp = new CAEONExpression;
	pExp->m_Columns.Insert(sField);

	pExp->m_Nodes.InsertEmpty(1);
	pExp->m_Nodes[0].iOp = EOp::Column;			// Set the operation to Column
	pExp->m_Nodes[0].iDataID = 0;				// Index of the column in m_Columns
	pExp->m_iRoot = 0;							// Root node index

	return CDatum(pExp);
	}

CDatum CAEONExpression::CreateLiteral (CDatum dValue)
	{
	CAEONExpression* pExp = new CAEONExpression;
	pExp->m_Literals.Insert(dValue);

	pExp->m_Nodes.InsertEmpty(1);
	pExp->m_Nodes[0].iOp = EOp::Literal;		// Set the operation to Literal
	pExp->m_Nodes[0].iDataID = 0;				// Index of the literal in m_Literals
	pExp->m_iRoot = 0;							// Root node index

	return CDatum(pExp);
	}

CDatum CAEONExpression::CreateTrue ()
	{
	CAEONExpression* pExp = new CAEONExpression;
	pExp->m_Nodes.InsertEmpty(1);
	pExp->m_Nodes[0].iOp = EOp::True;			// Set the operation to True
	pExp->m_iRoot = 0;							// Root node index

	return CDatum(pExp);
	}

//	Implementation -------------------------------------------------------------

CString CAEONExpression::AsID (EOp iOp)

//	AsID
//
//	Represent an op as an ID.

	{
	switch (iOp)
		{
		case EOp::None:
			return NULL_STR;

		case EOp::Abs:
			return OP_ABS;

		case EOp::Add:
			return OP_ADD;

		case EOp::All:
			return OP_ALL;

		case EOp::And:
			return OP_AND;

		case EOp::Any:
			return OP_ANY;

		case EOp::Array:
			return OP_ARRAY;

		case EOp::Average:
			return OP_AVERAGE;

		case EOp::Ceil:
			return OP_CEIL;

		case EOp::Clean:
			return OP_CLEAN;

		case EOp::Column:
			return OP_FIELD;

		case EOp::Count:
			return OP_COUNT;

		case EOp::DateTime:
			return OP_DATE_TIME;

		case EOp::Deref:
			return OP_DEREF;

		case EOp::Divide:
			return OP_DIVIDE;

		case EOp::EqualTo:
			return OP_EQUAL_TO;

		case EOp::ErrorLiteral:
			return OP_ERROR_LITERAL;

		case EOp::False:
			return OP_FALSE;

		case EOp::Find:
			return OP_FIND;

		case EOp::First:
			return OP_FIRST;

		case EOp::Floor:
			return OP_FLOOR;

		case EOp::GreaterThan:
			return OP_GREATER_THAN;

		case EOp::GreaterThanOrEqualTo:
			return OP_GREATER_THAN_OR_EQUAL_TO;

		case EOp::If:
			return OP_IF;

		case EOp::In:
			return OP_IN;

		case EOp::Integer:
			return OP_INTEGER;

		case EOp::Left:
			return OP_LEFT;

		case EOp::Length:
			return OP_LENGTH;

		case EOp::LessThan:
			return OP_LESS_THAN;

		case EOp::LessThanOrEqualTo:
			return OP_LESS_THAN_OR_EQUAL_TO;

		case EOp::Literal:
			return OP_LITERAL;

		case EOp::Lowercase:
			return OP_LOWERCASE;

		case EOp::Max:
			return OP_MAX;

		case EOp::Median:
			return OP_MEDIAN;

		case EOp::Member:
			return OP_MEMBER;

		case EOp::Min:
			return OP_MIN;

		case EOp::Mod:
			return OP_MOD;

		case EOp::Multiply:
			return OP_MULTIPLY;

		case EOp::Negate:
			return OP_NEGATE;

		case EOp::Not:
			return OP_NOT;

		case EOp::NotEqualTo:
			return OP_NOT_EQUAL_TO;

		case EOp::Number:
			return OP_NUMBER;

		case EOp::Or:
			return OP_OR;

		case EOp::Power:
			return OP_POWER;

		case EOp::Real:
			return OP_REAL;

		case EOp::Right:
			return OP_RIGHT;

		case EOp::Round:
			return OP_ROUND;

		case EOp::Sign:
			return OP_SIGN;

		case EOp::Slice:
			return OP_SLICE;

		case EOp::StdDev:
			return OP_STD_DEV;

		case EOp::StdError:
			return OP_STD_ERROR;

		case EOp::String:
			return OP_STRING;

		case EOp::Subtract:
			return OP_SUBTRACT;

		case EOp::Sum:
			return OP_SUM;

		case EOp::TimeSpan:
			return OP_TIME_SPAN;

		case EOp::True:
			return OP_TRUE;

		case EOp::UniqueArray:
			return OP_UNIQUE_ARRAY;

		case EOp::UniqueCount:
			return OP_UNIQUE_COUNT;

		case EOp::Uppercase:
			return OP_UPPERCASE;

		default:
			throw CException(errFail);
		}
	}

CAEONExpression::EOp CAEONExpression::AsOp (const CString& sValue)

//	AsOp
//
//	Parse an op code

	{
	if (strEqualsNoCase(sValue, OP_ABS))
		return EOp::Abs;
	else if (strEqualsNoCase(sValue, OP_ADD))
		return EOp::Add;
	else if (strEqualsNoCase(sValue, OP_ALL))
		return EOp::All;
	else if (strEqualsNoCase(sValue, OP_ANY))
		return EOp::Any;
	else if (strEqualsNoCase(sValue, OP_AND))
		return EOp::And;
	else if (strEqualsNoCase(sValue, OP_ARRAY))
		return EOp::Array;
	else if (strEqualsNoCase(sValue, OP_AVERAGE))
		return EOp::Average;
	else if (strEqualsNoCase(sValue, OP_CEIL))
		return EOp::Ceil;
	else if (strEqualsNoCase(sValue, OP_CLEAN))
		return EOp::Clean;
	else if (strEqualsNoCase(sValue, OP_COUNT))
		return EOp::Count;
	else if (strEqualsNoCase(sValue, OP_DATE_TIME))
		return EOp::DateTime;
	else if (strEqualsNoCase(sValue, OP_DEREF))
		return EOp::Deref;
	else if (strEqualsNoCase(sValue, OP_ERROR_LITERAL))
		return EOp::ErrorLiteral;
	else if (strEqualsNoCase(sValue, OP_FIELD))
		return EOp::Column;
	else if (strEqualsNoCase(sValue, OP_FIND))
		return EOp::Find;
	else if (strEqualsNoCase(sValue, OP_FLOOR))
		return EOp::Floor;
	else if (strEqualsNoCase(sValue, OP_DIVIDE))
		return EOp::Divide;
	else if (strEqualsNoCase(sValue, OP_EQUAL_TO))
		return EOp::EqualTo;
	else if (strEqualsNoCase(sValue, OP_FALSE))
		return EOp::False;
	else if (strEqualsNoCase(sValue, OP_FIRST))
		return EOp::First;
	else if (strEqualsNoCase(sValue, OP_GREATER_THAN))
		return EOp::GreaterThan;
	else if (strEqualsNoCase(sValue, OP_GREATER_THAN_OR_EQUAL_TO))
		return EOp::GreaterThanOrEqualTo;
	else if (strEqualsNoCase(sValue, OP_IF))
		return EOp::If;
	else if (strEqualsNoCase(sValue, OP_IN))
		return EOp::In;
	else if (strEqualsNoCase(sValue, OP_INTEGER))
		return EOp::Integer;
	else if (strEqualsNoCase(sValue, OP_LEFT))
		return EOp::Left;
	else if (strEqualsNoCase(sValue, OP_LENGTH))
		return EOp::Length;
	else if (strEqualsNoCase(sValue, OP_LESS_THAN))
		return EOp::LessThan;
	else if (strEqualsNoCase(sValue, OP_LESS_THAN_OR_EQUAL_TO))
		return EOp::LessThanOrEqualTo;
	else if (strEqualsNoCase(sValue, OP_LITERAL))
		return EOp::Literal;
	else if (strEqualsNoCase(sValue, OP_LOWERCASE))
		return EOp::Lowercase;
	else if (strEqualsNoCase(sValue, OP_MAX))
		return EOp::Max;
	else if (strEqualsNoCase(sValue, OP_MEDIAN))
		return EOp::Median;
	else if (strEqualsNoCase(sValue, OP_MEMBER))
		return EOp::Member;
	else if (strEqualsNoCase(sValue, OP_MIN))
		return EOp::Min;
	else if (strEqualsNoCase(sValue, OP_MOD))
		return EOp::Mod;
	else if (strEqualsNoCase(sValue, OP_MULTIPLY))
		return EOp::Multiply;
	else if (strEqualsNoCase(sValue, OP_NEGATE))
		return EOp::Negate;
	else if (strEqualsNoCase(sValue, OP_NOT))
		return EOp::Not;
	else if (strEqualsNoCase(sValue, OP_NOT_EQUAL_TO))
		return EOp::NotEqualTo;
	else if (strEqualsNoCase(sValue, OP_NUMBER))
		return EOp::Number;
	else if (strEqualsNoCase(sValue, OP_OR))
		return EOp::Or;
	else if (strEqualsNoCase(sValue, OP_POWER))
		return EOp::Power;
	else if (strEqualsNoCase(sValue, OP_REAL))
		return EOp::Real;
	else if (strEqualsNoCase(sValue, OP_RIGHT))
		return EOp::Right;
	else if (strEqualsNoCase(sValue, OP_ROUND))
		return EOp::Round;
	else if (strEqualsNoCase(sValue, OP_SIGN))
		return EOp::Sign;
	else if (strEqualsNoCase(sValue, OP_SLICE))
		return EOp::Slice;
	else if (strEqualsNoCase(sValue, OP_STD_DEV))
		return EOp::StdDev;
	else if (strEqualsNoCase(sValue, OP_STD_ERROR))
		return EOp::StdError;
	else if (strEqualsNoCase(sValue, OP_STRING))
		return EOp::String;
	else if (strEqualsNoCase(sValue, OP_SUBTRACT))
		return EOp::Subtract;
	else if (strEqualsNoCase(sValue, OP_SUM))
		return EOp::Sum;
	else if (strEqualsNoCase(sValue, OP_TIME_SPAN))
		return EOp::TimeSpan;
	else if (strEqualsNoCase(sValue, OP_TRUE))
		return EOp::True;
	else if (strEqualsNoCase(sValue, OP_UNIQUE_ARRAY))
		return EOp::UniqueArray;
	else if (strEqualsNoCase(sValue, OP_UNIQUE_COUNT))
		return EOp::UniqueCount;
	else if (strEqualsNoCase(sValue, OP_UPPERCASE))
		return EOp::Uppercase;
	else
		return EOp::None;
	}

CString CAEONExpression::AsString (void) const

//	AsString
//
//	Represent the query as a string.

	{
	if (m_iRoot == -1)
		return NULL_STR;
	else
		{
		switch (m_Nodes[m_iRoot].iOp)
			{
			case EOp::ErrorLiteral:
			case EOp::Literal:
				return GetLiteral(m_Nodes[m_iRoot].iDataID).AsString();

			default:
				//	LATER
				return CString("AEON Expression");
			}
		}
	}

size_t CAEONExpression::CalcMemorySize (void) const

//	CalcMemorySize
//
//	Returns the memory size.

	{
	//	LATER:
	size_t iSize = 0;
	return iSize;
	}

CDatum CAEONExpression::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	CAEONExpression *pExp = new CAEONExpression;
	CDatum dValue(pExp);
	Serialized.Add(dwID, dValue);

	//	Read the flags (not used yet)

	DWORD dwFlags = Stream.ReadDWORD();

	//	Read the root node

	pExp->m_iRoot = Stream.ReadDWORD();

	//	Read the nodes

	int iNodeCount = Stream.ReadDWORD();
	pExp->m_Nodes.InsertEmpty(iNodeCount);
	for (int i = 0; i < iNodeCount; i++)
		{
		SNode& Node = pExp->m_Nodes[i];

		//	Read the type

		CString sType = CString::Deserialize(Stream);
		Node.iOp = AsOp(sType);

		//	Read the node info

		Node.iLeft = Stream.ReadDWORD();
		Node.iRight = Stream.ReadDWORD();
		Node.iDataID = Stream.ReadDWORD();
		Node.dwFlags = Stream.ReadDWORD();
		}

	//	Read the column references.

	int iColCount = Stream.ReadDWORD();
	pExp->m_Columns.InsertEmpty(iColCount);
	for (int i = 0; i < iColCount; i++)
		{
		pExp->m_Columns[i] = CString::Deserialize(Stream);
		}

	//	Read the literals

	int iLiteralCount = Stream.ReadDWORD();
	pExp->m_Literals.InsertEmpty(iLiteralCount);
	for (int i = 0; i < iLiteralCount; i++)
		{
		pExp->m_Literals[i] = CDatum::DeserializeAEON(Stream, Serialized);
		}

	return dValue;
	}

CAEONExpression::EOp CAEONExpression::FlipOp (EOp iOp)

//	FlipOp
//
//	For certain symmetric comparison operations, we can flip the arguments but
//	we might need to change the operator.

	{
	switch (iOp)
		{
		case EOp::LessThan:
			return EOp::GreaterThan;

		case EOp::LessThanOrEqualTo:
			return EOp::GreaterThanOrEqualTo;

		case EOp::GreaterThan:
			return EOp::LessThan;

		case EOp::GreaterThanOrEqualTo:
			return EOp::LessThanOrEqualTo;

		case EOp::EqualTo:
		case EOp::NotEqualTo:
			return iOp;

		default:
			return EOp::None;
		}
	}

bool CAEONExpression::IsError (CString* retsError) const
	{
	if (m_iRoot == -1)
		return false;
	else
		{
		switch (m_Nodes[m_iRoot].iOp)
			{
			case EOp::ErrorLiteral:
				{
				if (retsError)
					*retsError = GetLiteral(m_Nodes[m_iRoot].iDataID).AsString();

				return true;
				}

			default:
				return false;
			}
		}
	}

bool CAEONExpression::IsNil (void) const

//	IsNil
//
//	Returns TRUE if this is a nil expression.

	{
	if (m_iRoot == -1)
		return true;
	else
		{
		switch (m_Nodes[m_iRoot].iOp)
			{
			case EOp::ErrorLiteral:
				return true;

			default:
				return false;
			}
		}
	}

bool CAEONExpression::NeedsParens (const SNode& Node)
	{
	switch (Node.iOp)
		{
		case EOp::None:
		case EOp::Abs:
		case EOp::All:
		case EOp::Any:
		case EOp::Array:
		case EOp::Average:
		case EOp::Ceil:
		case EOp::Clean:
		case EOp::Column:
		case EOp::Count:
		case EOp::DateTime:
		case EOp::Deref:
		case EOp::ErrorLiteral:
		case EOp::False:
		case EOp::Find:
		case EOp::First:
		case EOp::Floor:
		case EOp::Integer:
		case EOp::Left:
		case EOp::Length:
		case EOp::Literal:
		case EOp::Lowercase:
		case EOp::Max:
		case EOp::Median:
		case EOp::Member:
		case EOp::Min:
		case EOp::Negate:
		case EOp::Not:
		case EOp::Number:
		case EOp::Real:
		case EOp::Right:
		case EOp::Round:
		case EOp::Sign:
		case EOp::Slice:
		case EOp::StdDev:
		case EOp::StdError:
		case EOp::String:
		case EOp::Sum:
		case EOp::TimeSpan:
		case EOp::True:
		case EOp::UniqueArray:
		case EOp::UniqueCount:
		case EOp::Uppercase:
			return false;

		case EOp::Add:
		case EOp::And:
		case EOp::Divide:
		case EOp::EqualTo:
		case EOp::GreaterThan:
		case EOp::GreaterThanOrEqualTo:
		case EOp::If:
		case EOp::In:
		case EOp::LessThan:
		case EOp::LessThanOrEqualTo:
		case EOp::Mod:
		case EOp::Multiply:
		case EOp::NotEqualTo:
		case EOp::Or:
		case EOp::Power:
		case EOp::Subtract:
			return true;

		default:
			throw CException(errFail);
		}
	}

size_t CAEONExpression::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Compute a rough size.

	{
	return CalcMemorySize();
	}

void CAEONExpression::OnMarked (void)
	{
	for (int i = 0; i < m_Literals.GetCount(); i++)
		m_Literals[i].Mark();
	}

CAEONExpression::EOp CAEONExpression::ParseFunctionName (CStringView sSymbol)
	{
	if (strEqualsNoCase(sSymbol, OP_ABS))
		return EOp::Abs;
	else if (strEqualsNoCase(sSymbol, OP_ALL))
		return EOp::All;
	else if (strEqualsNoCase(sSymbol, OP_ANY))
		return EOp::Any;
	else if (strEqualsNoCase(sSymbol, OP_ARRAY))
		return EOp::Array;
	else if (strEqualsNoCase(sSymbol, OP_AVERAGE))
		return EOp::Average;
	else if (strEqualsNoCase(sSymbol, OP_CEIL))
		return EOp::Ceil;
	else if (strEqualsNoCase(sSymbol, OP_CLEAN))
		return EOp::Clean;
	else if (strEqualsNoCase(sSymbol, OP_COUNT))
		return EOp::Count;
	else if (strEqualsNoCase(sSymbol, OP_DATE_TIME))
		return EOp::DateTime;
	else if (strEqualsNoCase(sSymbol, OP_FIND))
		return EOp::Find;
	else if (strEqualsNoCase(sSymbol, OP_FIRST))
		return EOp::First;
	else if (strEqualsNoCase(sSymbol, OP_FLOOR))
		return EOp::Floor;
	else if (strEqualsNoCase(sSymbol, OP_INTEGER))
		return EOp::Integer;
	else if (strEqualsNoCase(sSymbol, OP_LEFT))
		return EOp::Left;
	else if (strEqualsNoCase(sSymbol, OP_LENGTH))
		return EOp::Length;
	else if (strEqualsNoCase(sSymbol, OP_LOWERCASE))
		return EOp::Lowercase;
	else if (strEqualsNoCase(sSymbol, OP_MAX))
		return EOp::Max;
	else if (strEqualsNoCase(sSymbol, OP_MEDIAN))
		return EOp::Median;
	else if (strEqualsNoCase(sSymbol, OP_MIN))
		return EOp::Min;
	else if (strEqualsNoCase(sSymbol, OP_NUMBER))
		return EOp::Number;
	else if (strEqualsNoCase(sSymbol, OP_REAL))
		return EOp::Real;
	else if (strEqualsNoCase(sSymbol, OP_RIGHT))
		return EOp::Right;
	else if (strEqualsNoCase(sSymbol, OP_ROUND))
		return EOp::Round;
	else if (strEqualsNoCase(sSymbol, OP_SIGN))
		return EOp::Sign;
	else if (strEqualsNoCase(sSymbol, OP_SLICE))
		return EOp::Slice;
	else if (strEqualsNoCase(sSymbol, OP_STD_DEV))
		return EOp::StdDev;
	else if (strEqualsNoCase(sSymbol, OP_STD_ERROR))
		return EOp::StdError;
	else if (strEqualsNoCase(sSymbol, OP_STRING))
		return EOp::String;
	else if (strEqualsNoCase(sSymbol, OP_SUM))
		return EOp::Sum;
	else if (strEqualsNoCase(sSymbol, OP_TIME_SPAN))
		return EOp::TimeSpan;
	else if (strEqualsNoCase(sSymbol, OP_UNIQUE_ARRAY))
		return EOp::UniqueArray;
	else if (strEqualsNoCase(sSymbol, OP_UNIQUE_COUNT))
		return EOp::UniqueCount;
	else if (strEqualsNoCase(sSymbol, OP_UPPERCASE))
		return EOp::Uppercase;
	else
		return EOp::None; // Not a recognized function
	}

void CAEONExpression::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const
	{
	switch (iFormat)
		{
		case CDatum::EFormat::GridLang:
			SerializeGridLang(Stream, GetRootNode());
			break;

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

void CAEONExpression::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_EXPRESSION))
		return;

	//	Flags

	DWORD dwFlags = 0;
	Stream.Write(dwFlags);

	//	Root node

	Stream.Write(m_iRoot);

	//	Nodes

	Stream.Write(m_Nodes.GetCount());
	for (int i = 0; i < m_Nodes.GetCount(); i++)
		{
		const SNode& Node = m_Nodes[i];

		//	Write the type

		CString sType = AsID(Node.iOp);
		sType.Serialize(Stream);

		//	Write the node info

		Stream.Write(Node.iLeft);
		Stream.Write(Node.iRight);
		Stream.Write(Node.iDataID);
		Stream.Write(Node.dwFlags);
		}

	//	Column references.

	Stream.Write(m_Columns.GetCount());
	for (int i = 0; i < m_Columns.GetCount(); i++)
		{
		m_Columns[i].Serialize(Stream);
		}

	//	Literals

	Stream.Write(m_Literals.GetCount());
	for (int i = 0; i < m_Literals.GetCount(); i++)
		{
		m_Literals[i].SerializeAEON(Stream, Serialized);
		}
	}

void CAEONExpression::SerializeGridLang (IByteStream &Stream, const SNode& Node) const
	{
	switch (Node.iOp)
		{
		case EOp::None:
			CDatum().Serialize(CDatum::EFormat::GridLang, Stream);
			break;

		case EOp::Abs:
			SerializeGridLangFunction(Stream, Node, OP_ABS);
			break;

		case EOp::Add:
			SerializeGridLangBinaryOp(Stream, Node, OP_ADD);
			break;

		case EOp::All:
			SerializeGridLangFunction(Stream, Node, OP_ALL);
			break;

		case EOp::And:
			SerializeGridLangBinaryOp(Stream, Node, OP_AND);
			break;

		case EOp::Any:
			SerializeGridLangFunction(Stream, Node, OP_ANY);
			break;

		case EOp::Array:
			SerializeGridLangFunction(Stream, Node, OP_ARRAY);
			break;

		case EOp::Average:
			SerializeGridLangFunction(Stream, Node, OP_AVERAGE);
			break;

		case EOp::Ceil:
			SerializeGridLangFunction(Stream, Node, OP_CEIL);
			break;

		case EOp::Clean:
			SerializeGridLangFunction(Stream, Node, OP_CLEAN);
			break;

		case EOp::Column:
			Stream.WriteChar('.');
			CDatum::WriteGridLangIdentifier(Stream, GetColumnID(Node.iDataID));
			break;

		case EOp::Count:
			{
			const SNode& ArgNode = GetNode(Node.iLeft);
			if (ArgNode.iOp == EOp::Literal)
				Stream.Write("count(*)", 8);
			else
				SerializeGridLangFunction(Stream, Node, OP_COUNT);
			break;
			}

		case EOp::DateTime:
			SerializeGridLangFunction(Stream, Node, OP_DATE_TIME);
			break;

		case EOp::Deref:
			{
			const SNode& ArrayNode = GetNode(Node.iLeft);
			const SNode& IndexNode = GetNode(Node.iRight);
			SerializeGridLang(Stream, ArrayNode);
			Stream.WriteChar('[');
			SerializeGridLang(Stream, IndexNode);
			Stream.WriteChar(']');
			break;
			}

		case EOp::Divide:
			SerializeGridLangBinaryOp(Stream, Node, OP_DIVIDE);
			break;

		case EOp::EqualTo:
			SerializeGridLangBinaryOp(Stream, Node, OP_EQUAL_TO);
			break;

		case EOp::ErrorLiteral:
			CDatum(GetLiteral(Node.iDataID)).Serialize(CDatum::EFormat::GridLang, Stream);
			break;

		case EOp::False:
			Stream.Write(OP_FALSE, 5);
			break;

		case EOp::Find:
			SerializeGridLangFunction(Stream, Node, OP_FIND);
			break;

		case EOp::First:
			SerializeGridLangFunction(Stream, Node, OP_FIRST);
			break;

		case EOp::Floor:
			SerializeGridLangFunction(Stream, Node, OP_FLOOR);
			break;

		case EOp::GreaterThan:
			SerializeGridLangBinaryOp(Stream, Node, OP_GREATER_THAN);
			break;

		case EOp::GreaterThanOrEqualTo:
			SerializeGridLangBinaryOp(Stream, Node, OP_GREATER_THAN_OR_EQUAL_TO);
			break;

		case EOp::If:
			{
			const SNode& ConditionNode = GetNode(Node.iDataID);
			const SNode& TrueNode = GetNode(Node.iLeft);
			const SNode& FalseNode = GetNode(Node.iRight);
			Stream.Write("if ", 3);
			SerializeGridLang(Stream, ConditionNode);
			Stream.Write(" -> ", 4);
			SerializeGridLang(Stream, TrueNode);

			if (FalseNode.iOp == EOp::Literal && GetLiteral(FalseNode.iDataID).IsIdenticalToNil())
				{ }
			else
				{
				Stream.Write(" else -> ", 9);
				SerializeGridLang(Stream, FalseNode);
				}
			break;
			}

		case EOp::In:
			SerializeGridLangBinaryOp(Stream, Node, OP_IN);
			break;

		case EOp::Integer:
			SerializeGridLangFunction(Stream, Node, OP_INTEGER);
			break;

		case EOp::Left:
			SerializeGridLangFunction(Stream, Node, OP_LEFT);
			break;

		case EOp::Length:
			SerializeGridLangFunction(Stream, Node, OP_LENGTH);
			break;

		case EOp::LessThan:
			SerializeGridLangBinaryOp(Stream, Node, OP_LESS_THAN);
			break;

		case EOp::LessThanOrEqualTo:
			SerializeGridLangBinaryOp(Stream, Node, OP_LESS_THAN_OR_EQUAL_TO);
			break;

		case EOp::Literal:
			CDatum(GetLiteral(Node.iDataID)).Serialize(CDatum::EFormat::GridLang, Stream);
			break;

		case EOp::Lowercase:
			SerializeGridLangFunction(Stream, Node, OP_LOWERCASE);
			break;

		case EOp::Max:
			SerializeGridLangFunction(Stream, Node, OP_MAX);
			break;

		case EOp::Median:
			SerializeGridLangFunction(Stream, Node, OP_MEDIAN);
			break;

		case EOp::Member:
			{
			const SNode& StructNode = GetNode(Node.iLeft);
			const SNode& FieldNode = GetNode(Node.iRight);
			SerializeGridLang(Stream, StructNode);
			Stream.WriteChar('.');
			if (FieldNode.iOp == EOp::Literal)
				{
				CDatum dField = GetLiteral(FieldNode.iDataID);
				CDatum::WriteGridLangIdentifier(Stream, dField.AsString());
				}
			else
				//	Should never happen
				SerializeGridLang(Stream, FieldNode);
			break;
			}

		case EOp::Min:
			SerializeGridLangFunction(Stream, Node, OP_MIN);
			break;

		case EOp::Mod:
			SerializeGridLangBinaryOp(Stream, Node, OP_MOD);
			break;

		case EOp::Multiply:
			SerializeGridLangBinaryOp(Stream, Node, OP_MULTIPLY);
			break;

		case EOp::Negate:
			{
			const SNode& Operand = GetNode(Node.iLeft);
			Stream.WriteChar('-');
			if (NeedsParens(Operand))
				{
				Stream.WriteChar('(');
				SerializeGridLang(Stream, Operand);
				Stream.WriteChar(')');
				}
			else
				SerializeGridLang(Stream, Operand);
			break;
			}

		case EOp::Not:
			{
			const SNode& Operand = GetNode(Node.iLeft);
			Stream.WriteChar('!');
			if (NeedsParens(Operand))
				{
				Stream.WriteChar('(');
				SerializeGridLang(Stream, Operand);
				Stream.WriteChar(')');
				}
			else
				SerializeGridLang(Stream, Operand);
			break;
			}

		case EOp::NotEqualTo:
			SerializeGridLangBinaryOp(Stream, Node, OP_NOT_EQUAL_TO);
			break;

		case EOp::Number:
			SerializeGridLangFunction(Stream, Node, OP_NUMBER);
			break;

		case EOp::Or:
			SerializeGridLangBinaryOp(Stream, Node, OP_OR);
			break;

		case EOp::Power:
			SerializeGridLangBinaryOp(Stream, Node, OP_POWER);
			break;

		case EOp::Real:
			SerializeGridLangFunction(Stream, Node, OP_REAL);
			break;

		case EOp::Right:
			SerializeGridLangFunction(Stream, Node, OP_RIGHT);
			break;

		case EOp::Round:
			SerializeGridLangFunction(Stream, Node, OP_ROUND);
			break;

		case EOp::Sign:
			SerializeGridLangFunction(Stream, Node, OP_SIGN);
			break;

		case EOp::Slice:
			{
			const SNode& StringNode = GetNode(Node.iLeft);

			Stream.Write("slice(", 6);
			SerializeGridLang(Stream, StringNode);

			if (Node.iRight != -1)
				{
				Stream.Write(", ", 2);
				SerializeGridLang(Stream, GetNode(Node.iRight));
				}

			if (Node.iDataID != -1)
				{
				Stream.Write(", ", 2);
				SerializeGridLang(Stream, GetNode(Node.iDataID));
				}

			Stream.Write(")", 1);
			break;
			}

		case EOp::StdDev:
			SerializeGridLangFunction(Stream, Node, OP_STD_DEV);
			break;

		case EOp::StdError:
			SerializeGridLangFunction(Stream, Node, OP_STD_ERROR);
			break;

		case EOp::String:
			SerializeGridLangFunction(Stream, Node, OP_STRING);
			break;

		case EOp::Subtract:
			SerializeGridLangBinaryOp(Stream, Node, OP_SUBTRACT);
			break;

		case EOp::Sum:
			SerializeGridLangFunction(Stream, Node, OP_SUM);
			break;

		case EOp::TimeSpan:
			SerializeGridLangFunction(Stream, Node, OP_TIME_SPAN);
			break;

		case EOp::True:
			Stream.Write(OP_TRUE);
			break;

		case EOp::UniqueArray:
			SerializeGridLangFunction(Stream, Node, OP_UNIQUE_ARRAY);
			break;

		case EOp::UniqueCount:
			SerializeGridLangFunction(Stream, Node, OP_UNIQUE_COUNT);
			break;

		case EOp::Uppercase:
			SerializeGridLangFunction(Stream, Node, OP_UPPERCASE);
			break;

		default:
			throw CException(errFail);
		}
	}

void CAEONExpression::SerializeGridLangBinaryOp (IByteStream &Stream, const SNode& Node, CStringView sOp) const
	{
	const SNode& Left = GetNode(Node.iLeft);
	const SNode& Right = GetNode(Node.iRight);

	if (NeedsParens(Left))
		{
		Stream.WriteChar('(');
		SerializeGridLang(Stream, Left);
		Stream.WriteChar(')');
		}
	else
		SerializeGridLang(Stream, Left);

	Stream.WriteChar(' ');
	Stream.Write(sOp);
	Stream.WriteChar(' ');

	if (NeedsParens(Right))
		{
		Stream.WriteChar('(');
		SerializeGridLang(Stream, Right);
		Stream.WriteChar(')');
		}
	else
		SerializeGridLang(Stream, Right);
	}

void CAEONExpression::SerializeGridLangFunction (IByteStream &Stream, const SNode& Node, CStringView sFunc) const
	{
	Stream.Write(sFunc);
	Stream.WriteChar('(');
	SerializeGridLang(Stream, GetNode(Node.iLeft));
	if (Node.iRight != -1)
		{
		Stream.Write(", ", 2);
		SerializeGridLang(Stream, GetNode(Node.iRight));
		}
	Stream.WriteChar(')');
	}
