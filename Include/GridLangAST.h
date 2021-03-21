//	GridLangAST.h
//
//	GridLang Internals
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "GridLangParser.h"

class IGLType;

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
	LibraryFunctionDef,
	LiteralArray,
	LiteralNull,
	LiteralFloat,
	LiteralInteger,
	LiteralString,
	LiteralTrue,
	OpArithmeticAnd,
	OpArithmeticOr,
	OpArray,
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

	//	NOTE:
	//
	//	When adding a new type, make sure to update IASTNode::m_TypeDesc.

	LastType,
	};

class IASTNode
	{
	public:
		IASTNode (IASTNode *pParent) :
				m_pParent(pParent)
			{ }

		virtual ~IASTNode () { }

		bool ComposeError (const CString &sError, CString *retsError = NULL) const;
		virtual bool AddChild (const TArray<TSharedPtr<IASTNode>> &Nodes, CString *retsError = NULL) { throw CException(errFail); }
		virtual void DebugDump (const CString &sIndent = NULL_STR) const { }
		virtual const IASTNode *FindDefinition (const CString &sID) const { return NULL; }
		virtual const CString &GetBaseName () const { return NULL_STR; }
		virtual IASTNode &GetChild (int iIndex) { throw CException(errFail); }
		const IASTNode &GetChild (int iIndex) const { return const_cast<IASTNode *>(this)->GetChild(iIndex); }
		virtual int GetChildCount () const { return 0; }
		virtual int GetCodeID () const { return 0; }
		virtual const IASTNode &GetDefinition (int iIndex) const { throw CException(errFail); }
		virtual int GetDefinitionCount () const { return 0; }
		virtual const CString &GetDefinitionString (int iIndex) const { throw CException(errFail); }
		virtual const CString &GetName () const { return NULL_STR; }
		virtual int GetOrdinal () const { return -1; }
		IASTNode &GetParent () { if (m_pParent) return *m_pParent; else throw CException(errFail); }
		virtual IASTNode &GetRoot () { return *this; }
		virtual IASTNode &GetStatement (int iIndex) { throw CException(errFail); }
		virtual int GetStatementCount () const { return 0; }
		virtual EASTType GetType () const = 0;
		CString GetTypeName () const { return GetTypeName(GetType()); }
		virtual const IASTNode &GetTypeRef () const { throw CException(errFail); }
		virtual CDatum GetValue () const { return CDatum(); }
		virtual IASTNode &GetVarDef (int iIndex) { throw CException(errFail); }
		virtual int GetVarDefCount () const { return 0; }
		bool HasParent () const { return m_pParent != NULL; }
		virtual bool HasScope () const { return false;}
		virtual bool IsFunctionDefinition () const { return false; }
		virtual bool IsStatement () const { return false; }
		virtual bool IsTypeDefinition () const { return false; }
		virtual bool IsVarDefinition () const { return false; }
		virtual void SetCodeID (int iID) { throw CException(errFail); }
		void SetParent (IASTNode &Parent) { m_pParent = &Parent; }
		virtual void SetTypeDef (IGLType &Type) { throw CException(errFail); }

		IASTNode *AddRef (void) { m_dwRefCount++; return this; }
		void Delete (void) { if (--m_dwRefCount == 0) delete this; }

		static CString GetTypeName (EASTType iType);

	protected:

		void AddChild (TSharedPtr<IASTNode> &pPointer, TSharedPtr<IASTNode> pChild)
			{
			pPointer = pChild;
			if (pChild)
				pChild->SetParent(*this);
			}

	private:
		IASTNode *m_pParent = NULL;			//	Lexical parent
		int m_dwRefCount = 1;

		struct STypeDesc
			{
			EASTType iType = EASTType::Unknown;
			CString sName;
			};

		static const STypeDesc m_TypeDesc[(int)EASTType::LastType];
	};

class CGridLangAST
	{
	public:
		void DebugDump () const { if (m_pRoot) m_pRoot->DebugDump(NULL_STR); }
		const IASTNode &GetNode (int iIndex) const { if (m_pRoot) return m_pRoot->GetChild(iIndex); else throw CException(errFail); }
		int GetNodeCount () const { return (m_pRoot ? m_pRoot->GetChildCount() : 0); }
		IASTNode &GetRoot () { if (m_pRoot) return *m_pRoot; else throw CException(errFail); }
		const IASTNode &GetRoot () const { if (m_pRoot) return *m_pRoot; else throw CException(errFail); }
		bool IsEmpty () const { return !m_pRoot; }
		bool Load (IMemoryBlock &Stream, CString *retsError = NULL);
		
	private:
		static EASTType GetOperator (EGridLangToken iToken);
		static int GetOperatorPrecedence (EASTType iOperator);
		static bool IsGreaterPrecedence (EASTType iOperator, EASTType iTest);
		static bool IsOperator (EGridLangToken iToken);

		const IASTNode &GetNode (int iIndex) { if (m_pRoot) return m_pRoot->GetChild(iIndex); else throw CException(errFail); }
		bool ParseArgDef (CGridLangParser &Parser, IASTNode *pParent, int iOrdinal, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseArrayElement (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> pArray, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseArrayLiteral (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseClassDef (CGridLangParser &Parser, IASTNode *pParent, const CString &sBaseType, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseExpression (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL) { return ParseExpression(Parser, pParent, EASTType::Unknown, retpNode, retsError); }
		bool ParseExpression (CGridLangParser &Parser, IASTNode *pParent, EASTType iLeftOperator, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseFunctionCall (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> pFunction, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseFunctionDef (CGridLangParser &Parser, IASTNode *pParent, int iOrdinal, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseIf (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseOrdinalDef (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseSequence (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseTerm (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseTypeRef (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseVarDef (CGridLangParser &Parser, IASTNode *pParent, EASTType iVarType, int iOrdinal, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);

		TSharedPtr<IASTNode> m_pRoot;
	};
