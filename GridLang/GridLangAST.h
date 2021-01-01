//	GridLangAST.h
//
//	GridLang Internals
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

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

		virtual void DebugDump (const CString &sIndent) const { }
		virtual EASTType GetType () const = 0;

		IASTNode *AddRef (void) { m_dwRefCount++; return this; }
		void Delete (void) { if (--m_dwRefCount == 0) delete this; }

	private:
		int m_dwRefCount = 1;
	};

class CASTArgDef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (const CString &sArg, TSharedPtr<IASTNode> pTypeRef)
			{
			CASTArgDef *pNode = new CASTArgDef;
			pNode->m_sArg = sArg;
			pNode->m_pTypeRef = pTypeRef;
			return TSharedPtr<IASTNode>(pNode);
			}

		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return EASTType::ArgDef; }

	private:
		CString m_sArg;
		TSharedPtr<IASTNode> m_pTypeRef;
	};

class CASTBinaryOp : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (EASTType iOp, TSharedPtr<IASTNode> pLeft, TSharedPtr<IASTNode> pRight)
			{
			CASTBinaryOp *pNode = new CASTBinaryOp;
			pNode->m_iOp = iOp;
			pNode->m_pLeft = pLeft;
			pNode->m_pRight = pRight;
			return TSharedPtr<IASTNode>(pNode);
			}
		
		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return m_iOp; }

	private:
		EASTType m_iOp = EASTType::Unknown;
		TSharedPtr<IASTNode> m_pLeft;
		TSharedPtr<IASTNode> m_pRight;
	};

class CASTClassDef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (const CString &sID, const CString &sBaseID, TSharedPtr<IASTNode> pBody)
			{
			CASTClassDef *pNode = new CASTClassDef;
			pNode->m_sID = sID;
			pNode->m_sBaseID = sBaseID;
			pNode->m_pBody = pBody;
			return TSharedPtr<IASTNode>(pNode);
			}

		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return EASTType::ClassDef; }

	private:
		CString m_sID;
		CString m_sBaseID;
		TSharedPtr<IASTNode> m_pBody;
	};

class CASTFunctionCall : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (TSharedPtr<IASTNode> pFunction, TArray<TSharedPtr<IASTNode>> Args)
			{
			CASTFunctionCall *pNode = new CASTFunctionCall;
			pNode->m_pFunction = pFunction;
			pNode->m_Args = std::move(Args);
			return TSharedPtr<IASTNode>(pNode);
			}

		virtual void DebugDump (const CString &sIndent) const override;
		const IASTNode &GetArg (int iIndex) const { return *m_Args[iIndex]; }
		int GetArgCount () const { return m_Args.GetCount(); }
		virtual EASTType GetType () const override { return EASTType::FunctionCall; }

	private:
		TSharedPtr<IASTNode> m_pFunction;
		TArray<TSharedPtr<IASTNode>> m_Args;
	};

class CASTFunctionDef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (const CString &sFunction, TSharedPtr<IASTNode> pTypeDef, TArray<TSharedPtr<IASTNode>> ArgDefs, TSharedPtr<IASTNode> pBody)
			{
			CASTFunctionDef *pNode = new CASTFunctionDef;
			pNode->m_sFunction = sFunction;
			pNode->m_ArgDefs = std::move(ArgDefs);
			pNode->m_pBody = pBody;
			return TSharedPtr<IASTNode>(pNode);
			}

		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return EASTType::FunctionDef; }

	private:
		CString m_sFunction;
		TSharedPtr<IASTNode> m_pTypeDef;
		TArray<TSharedPtr<IASTNode>> m_ArgDefs;
		TSharedPtr<IASTNode> m_pBody;
	};

class CASTIf : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (TSharedPtr<IASTNode> pCondition, TSharedPtr<IASTNode> pThen, TSharedPtr<IASTNode> pElse)
			{
			CASTIf *pNode = new CASTIf;
			pNode->m_pCondition = pCondition;
			pNode->m_pThen = pThen;
			pNode->m_pElse = pElse;
			return TSharedPtr<IASTNode>(pNode);
			}

		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return EASTType::DoIf; }

	private:
		TSharedPtr<IASTNode> m_pCondition;
		TSharedPtr<IASTNode> m_pThen;
		TSharedPtr<IASTNode> m_pElse;
	};

class CASTLiteralFloat : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (double rValue)
			{
			CASTLiteralFloat *pNode = new CASTLiteralFloat;
			pNode->m_rValue = rValue;
			return TSharedPtr<IASTNode>(pNode);
			}

		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return EASTType::LiteralFloat; }

	private:
		double m_rValue = 0.0;
	};

class CASTLiteralIdentifier : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (EASTType iType)
			{
			CASTLiteralIdentifier *pNode = new CASTLiteralIdentifier;
			pNode->m_iType = iType;
			return TSharedPtr<IASTNode>(pNode);
			}

		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return m_iType; }

	private:
		EASTType m_iType = EASTType::Unknown;
	};

class CASTLiteralInteger : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (int iValue)
			{
			CASTLiteralInteger *pNode = new CASTLiteralInteger;
			pNode->m_iValue = iValue;
			return TSharedPtr<IASTNode>(pNode);
			}

		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return EASTType::LiteralInteger; }

	private:
		int m_iValue = 0;
	};

class CASTLiteralString : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (const CString &sValue)
			{
			CASTLiteralString *pNode = new CASTLiteralString;
			pNode->m_sValue = sValue;
			return TSharedPtr<IASTNode>(pNode);
			}

		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return EASTType::LiteralString; }

	private:
		CString m_sValue;
	};

class CASTOrdinalDef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (const CString &sName, TArray<CString> Ordinals)
			{
			CASTOrdinalDef *pNode = new CASTOrdinalDef;
			pNode->m_sName = sName;
			pNode->m_Ordinals = Ordinals;
			return TSharedPtr<IASTNode>(pNode);
			}

		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return EASTType::OrdinalDef; }

	private:
		CString m_sName;
		TArray<CString> m_Ordinals;
	};

class CASTSequence : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (TArray<TSharedPtr<IASTNode>> Nodes)
			{
			CASTSequence *pSeq = new CASTSequence;
			pSeq->m_Node = std::move(Nodes);
			return TSharedPtr<IASTNode>(pSeq);
			}

		void AddNode (TSharedPtr<IASTNode> pChild) { m_Node.Insert(pChild); }
		virtual void DebugDump (const CString &sIndent) const override;
		const IASTNode &GetNode (int iIndex) const { return *m_Node[iIndex]; }
		int GetNodeCount () const { return m_Node.GetCount(); }
		virtual EASTType GetType () const override { return EASTType::Sequence; }

	private:
		TArray<TSharedPtr<IASTNode>> m_Node;
	};

class CASTTypeRef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (const CString &sType)
			{
			CASTTypeRef *pNode = new CASTTypeRef;
			pNode->m_sType = sType;
			return TSharedPtr<IASTNode>(pNode);
			}

		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return EASTType::TypeRef; }

	private:
		CString m_sType;
	};

class CASTUnaryOp : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (EASTType iOp, TSharedPtr<IASTNode> pOperand)
			{
			CASTUnaryOp *pNode = new CASTUnaryOp;
			pNode->m_iOp = iOp;
			pNode->m_pOperand = pOperand;
			return TSharedPtr<IASTNode>(pNode);
			}
		
		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return m_iOp; }

	private:
		EASTType m_iOp = EASTType::Unknown;
		TSharedPtr<IASTNode> m_pOperand;
	};

class CASTVarDef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (EASTType iVarType, const CString &sName, TSharedPtr<IASTNode> pTypeDef, TSharedPtr<IASTNode> pBody)
			{
			CASTVarDef *pNode = new CASTVarDef;
			pNode->m_iVarType = iVarType;
			pNode->m_sName = sName;
			pNode->m_pTypeDef = pTypeDef;
			pNode->m_pBody = pBody;
			return TSharedPtr<IASTNode>(pNode);
			}

		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return m_iVarType; }

	private:
		EASTType m_iVarType = EASTType::Unknown;
		CString m_sName;
		TSharedPtr<IASTNode> m_pTypeDef;
		TSharedPtr<IASTNode> m_pBody;
	};

class CASTVarRef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (const CString &sVar)
			{
			CASTVarRef *pNode = new CASTVarRef;
			pNode->m_sVar = sVar;
			return TSharedPtr<IASTNode>(pNode);
			}
		
		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return EASTType::VarRef; }

	private:
		CString m_sVar;
	};

class CGridLangAST
	{
	public:
		void DebugDump () const { if (m_pRoot) m_pRoot->DebugDump(NULL_STR); }
		bool Load (IMemoryBlock &Stream, CString *retsError = NULL);
		
	private:
		static EASTType GetOperator (EGridLangToken iToken);
		static int GetOperatorPrecedence (EASTType iOperator);
		static bool IsGreaterPrecedence (EASTType iOperator, EASTType iTest);
		static bool IsOperator (EGridLangToken iToken);

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
