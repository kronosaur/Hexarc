//	GridLangAST.h
//
//	GridLang Internals
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "GridLangParser.h"

enum class EASTType
	{
	Unknown,

	ArgDef,
	ClassDef,
	ConstDef,
	DoIf,
	FunctionCall,
	FunctionDef,
	GlobalDef,
	LiteralNull,
	LiteralFloat,
	LiteralInteger,
	LiteralString,
	LiteralTrue,
	OpArithmeticAnd,
	OpArithmeticOr,
	OpAssignment,
	OpDivide,
	OpEquals,
	OpFunctionCall,
	OpGreaterThan,
	OpGreaterThanEquals,
	OpLessThan,
	OpLessThanEquals,
	OpLogicalAnd,
	OpLogicalOr,
	OpMemberAccess,
	OpMinus,
	OpNot,
	OpNotEquals,
	OpPlus,
	OpTimes,
	OpReturn,
	OrdinalDef,
	PropertyDef,
	Sequence,
	TypeRef,
	VarDef,
	VarRef,
	};

class IASTNode
	{
	public:
		virtual ~IASTNode () { }

		bool ComposeError (const CString &sError, CString *retsError = NULL) const;
		virtual void DebugDump (const CString &sIndent) const { }
		virtual const IASTNode *FindDefinition (const CString &sID) const { return NULL; }
		virtual const CString &GetBaseName () const { return NULL_STR; }
		virtual IASTNode &GetChild (int iIndex) const { throw CException(errFail); }
		virtual int GetChildCount () const { return 0; }
		virtual const IASTNode &GetDefinition (int iIndex) const { throw CException(errFail); }
		virtual int GetDefinitionCount () const { return 0; }
		virtual const CString &GetDefinitionString (int iIndex) const { throw CException(errFail); }
		virtual const CString &GetName () const { return NULL_STR; }
		virtual EASTType GetType () const = 0;
		virtual const IASTNode &GetTypeRef () const { throw CException(errFail); }

		IASTNode *AddRef (void) { m_dwRefCount++; return this; }
		void Delete (void) { if (--m_dwRefCount == 0) delete this; }

	private:
		int m_dwRefCount = 1;
	};

class CGridLangAST
	{
	public:
		void DebugDump () const { if (m_pRoot) m_pRoot->DebugDump(NULL_STR); }
		const IASTNode &GetNode (int iIndex) const { if (m_pRoot) return m_pRoot->GetChild(iIndex); else throw CException(errFail); }
		int GetNodeCount () const { return (m_pRoot ? m_pRoot->GetChildCount() : 0); }
		const IASTNode &GetRoot () const { if (m_pRoot) return *m_pRoot; else throw CException(errFail); }
		bool IsEmpty () const { return !m_pRoot; }
		bool Load (IMemoryBlock &Stream, CString *retsError = NULL);
		
	private:
		static EASTType GetOperator (EGridLangToken iToken);
		static int GetOperatorPrecedence (EASTType iOperator);
		static bool IsGreaterPrecedence (EASTType iOperator, EASTType iTest);
		static bool IsOperator (EGridLangToken iToken);

		IASTNode &GetNode (int iIndex) { if (m_pRoot) return m_pRoot->GetChild(iIndex); else throw CException(errFail); }
		bool ParseArgDef (CGridLangParser &Parser, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseClassDef (CGridLangParser &Parser, const CString &sBaseType, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseExpression (CGridLangParser &Parser, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL) { return ParseExpression(Parser, EASTType::Unknown, retpNode, retsError); }
		bool ParseExpression (CGridLangParser &Parser, EASTType iLeftOperator, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseFunctionCall (CGridLangParser &Parser, TSharedPtr<IASTNode> pFunction, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseFunctionDef (CGridLangParser &Parser, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseIf (CGridLangParser &Parser, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseOrdinalDef (CGridLangParser &Parser, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseSequence (CGridLangParser &Parser, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseTerm (CGridLangParser &Parser, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseTypeRef (CGridLangParser &Parser, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseVarDef (CGridLangParser &Parser, EASTType iVarType, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);

		TSharedPtr<IASTNode> m_pRoot;
	};
