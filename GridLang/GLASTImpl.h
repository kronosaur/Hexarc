//	GLASTImpl.h
//
//	IASTNode Implementations
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CASTArgDef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (IASTNode *pParent, const CString &sArg, TSharedPtr<IASTNode> pTypeRef)
			{
			CASTArgDef *pNode = new CASTArgDef(pParent);
			pNode->m_sArg = sArg;
			pNode->m_pTypeRef = pTypeRef;
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTArgDef (IASTNode *pParent) : IASTNode(pParent)
			{ }

		virtual void DebugDump (const CString &sIndent) const override;
		virtual const CString &GetName () const override { return m_sArg; }
		virtual EASTType GetType () const override { return EASTType::ArgDef; }
		virtual const IASTNode &GetTypeRef () const override { return *m_pTypeRef; }

	private:
		CString m_sArg;
		TSharedPtr<IASTNode> m_pTypeRef;
	};

class CASTBinaryOp : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (IASTNode *pParent, EASTType iOp, TSharedPtr<IASTNode> pLeft, TSharedPtr<IASTNode> pRight)
			{
			CASTBinaryOp *pNode = new CASTBinaryOp(pParent);
			pNode->m_iOp = iOp;
			pNode->m_pLeft = pLeft;
			pNode->m_pRight = pRight;
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTBinaryOp (IASTNode *pParent) : IASTNode(pParent)
			{ }
		
		virtual void DebugDump (const CString &sIndent) const override;
		virtual IASTNode &GetChild (int iIndex) override
			{
			switch (iIndex)
				{
				case 0:
					return *m_pLeft;

				case 1:
					return *m_pRight;

				default:
					throw CException(errFail);
				}
			}

		virtual int GetChildCount () const override { return 2; }
		virtual EASTType GetType () const override { return m_iOp; }

	private:
		EASTType m_iOp = EASTType::Unknown;
		TSharedPtr<IASTNode> m_pLeft;
		TSharedPtr<IASTNode> m_pRight;
	};

class CASTClassDef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (IASTNode *pParent, const CString &sID, const CString &sBaseID, TSharedPtr<IASTNode> pBody)
			{
			CASTClassDef *pNode = new CASTClassDef(pParent);
			pNode->m_sID = sID;
			pNode->m_sBaseID = sBaseID;
			pNode->m_pBody = pBody;
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTClassDef (IASTNode *pParent) : IASTNode(pParent)
			{ }

		virtual void DebugDump (const CString &sIndent) const override;
		virtual const CString &GetBaseName () const override { return m_sBaseID; }
		virtual IASTNode &GetChild (int iIndex) override { if (iIndex == 0) return *m_pBody; else throw CException(errFail); }
		virtual int GetChildCount () const override { return 1; }
		virtual const CString &GetName () const override { return m_sID; }
		virtual EASTType GetType () const override { return EASTType::ClassDef; }
		virtual bool IsTypeDefinition () const override { return true; }
		virtual void SetTypeDef (IGLType &Type) override { m_pTypeDef = &Type; }

	private:
		CString m_sID;
		CString m_sBaseID;
		TSharedPtr<IASTNode> m_pBody;

		IGLType *m_pTypeDef = NULL;
	};

class CASTFunctionCall : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (IASTNode *pParent, TSharedPtr<IASTNode> pFunction, TArray<TSharedPtr<IASTNode>> Args)
			{
			CASTFunctionCall *pNode = new CASTFunctionCall(pParent);
			pNode->m_pFunction = pFunction;
			pNode->m_Args = std::move(Args);
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTFunctionCall (IASTNode *pParent) : IASTNode(pParent)
			{ }

		virtual void DebugDump (const CString &sIndent) const override;
		const IASTNode &GetArg (int iIndex) const { return *m_Args[iIndex]; }
		int GetArgCount () const { return m_Args.GetCount(); }
		virtual IASTNode &GetChild (int iIndex) override { return const_cast<IASTNode &>(GetArg(iIndex)); }
		virtual int GetChildCount () const override { return GetArgCount(); }
		virtual const CString &GetName () const override { return m_pFunction->GetName(); }
		virtual const IASTNode &GetRoot () const override { return *m_pFunction; }
		virtual EASTType GetType () const override { return EASTType::FunctionCall; }
		virtual bool IsStatement () const override { return true; }

	private:
		TSharedPtr<IASTNode> m_pFunction;
		TArray<TSharedPtr<IASTNode>> m_Args;
	};

class CASTFunctionDef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (IASTNode *pParent, const CString &sFunction, TSharedPtr<IASTNode> pTypeDef, TArray<TSharedPtr<IASTNode>> ArgDefs, TSharedPtr<IASTNode> pBody)
			{
			CASTFunctionDef *pNode = new CASTFunctionDef(pParent);
			pNode->m_sFunction = sFunction;
			pNode->m_ArgDefs = std::move(ArgDefs);
			pNode->m_pBody = pBody;
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTFunctionDef (IASTNode *pParent) : IASTNode(pParent)
			{ }

		virtual void DebugDump (const CString &sIndent) const override;
		virtual const CString &GetName () const override { return m_sFunction; }
		virtual EASTType GetType () const override { return EASTType::FunctionDef; }
		virtual const IASTNode &GetTypeRef () const override { return *m_pTypeDef; }
		virtual bool IsFunctionDefinition () const override { return true; }

	private:
		CString m_sFunction;
		TSharedPtr<IASTNode> m_pTypeDef;
		TArray<TSharedPtr<IASTNode>> m_ArgDefs;
		TSharedPtr<IASTNode> m_pBody;
	};

class CASTIf : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (IASTNode *pParent, TSharedPtr<IASTNode> pCondition, TSharedPtr<IASTNode> pThen, TSharedPtr<IASTNode> pElse)
			{
			CASTIf *pNode = new CASTIf(pParent);
			pNode->m_pCondition = pCondition;
			pNode->m_pThen = pThen;
			pNode->m_pElse = pElse;
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTIf (IASTNode *pParent) : IASTNode(pParent)
			{ }

		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return EASTType::DoIf; }
		virtual bool IsStatement () const override { return true; }

	private:
		TSharedPtr<IASTNode> m_pCondition;
		TSharedPtr<IASTNode> m_pThen;
		TSharedPtr<IASTNode> m_pElse;
	};

class CASTLibraryFunctionDef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (const CString &sFunctionName)
			{
			CASTLibraryFunctionDef *pNode = new CASTLibraryFunctionDef(sFunctionName);
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTLibraryFunctionDef (const CString &sFunctionName) : IASTNode(NULL),
				m_sFunction(sFunctionName)
			{ }

		virtual void DebugDump (const CString &sIndent) const override;
		virtual const CString &GetName () const override { return m_sFunction; }
		virtual EASTType GetType () const override { return EASTType::LibraryFunctionDef; }
		virtual bool IsFunctionDefinition () const override { return true; }

	private:
		CString m_sFunction;
	};

class CASTLiteralFloat : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (IASTNode *pParent, double rValue)
			{
			CASTLiteralFloat *pNode = new CASTLiteralFloat(pParent);
			pNode->m_rValue = rValue;
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTLiteralFloat (IASTNode *pParent) : IASTNode(pParent)
			{ }

		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return EASTType::LiteralFloat; }
		virtual CDatum GetValue () const { return CDatum(m_rValue); }

	private:
		double m_rValue = 0.0;
	};

class CASTLiteralIdentifier : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (IASTNode *pParent, EASTType iType)
			{
			CASTLiteralIdentifier *pNode = new CASTLiteralIdentifier(pParent);
			pNode->m_iType = iType;
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTLiteralIdentifier (IASTNode *pParent) : IASTNode(pParent)
			{ }

		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return m_iType; }

	private:
		EASTType m_iType = EASTType::Unknown;
	};

class CASTLiteralInteger : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (IASTNode *pParent, int iValue)
			{
			CASTLiteralInteger *pNode = new CASTLiteralInteger(pParent);
			pNode->m_iValue = iValue;
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTLiteralInteger (IASTNode *pParent) : IASTNode(pParent)
			{ }

		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return EASTType::LiteralInteger; }
		virtual CDatum GetValue () const { return CDatum(m_iValue); }

	private:
		int m_iValue = 0;
	};

class CASTLiteralString : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (IASTNode *pParent, const CString &sValue)
			{
			CASTLiteralString *pNode = new CASTLiteralString(pParent);
			pNode->m_sValue = sValue;
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTLiteralString (IASTNode *pParent) : IASTNode(pParent)
			{ }

		virtual void DebugDump (const CString &sIndent) const override;
		virtual EASTType GetType () const override { return EASTType::LiteralString; }
		virtual CDatum GetValue () const { return CDatum(m_sValue); }

	private:
		CString m_sValue;
	};

class CASTOrdinalDef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (IASTNode *pParent, const CString &sName, TArray<CString> Ordinals)
			{
			CASTOrdinalDef *pNode = new CASTOrdinalDef(pParent);
			pNode->m_sName = sName;
			pNode->m_Ordinals = Ordinals;
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTOrdinalDef (IASTNode *pParent) : IASTNode(pParent)
			{ }

		virtual void DebugDump (const CString &sIndent) const override;
		virtual int GetDefinitionCount () const override { return m_Ordinals.GetCount(); }
		virtual const CString &GetDefinitionString (int iIndex) const override { if (iIndex >= 0 && iIndex < GetDefinitionCount()) return m_Ordinals[iIndex]; else throw CException(errFail); }
		virtual const CString &GetName () const override { return m_sName; }
		virtual EASTType GetType () const override { return EASTType::OrdinalDef; }
		virtual bool IsTypeDefinition () const override { return true; }

	private:
		CString m_sName;
		TArray<CString> m_Ordinals;
	};

class CASTSequence : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (IASTNode *pParent, TArray<TSharedPtr<IASTNode>> Nodes, CString *retsError);

		CASTSequence (IASTNode *pParent) : IASTNode(pParent)
			{ }

		virtual bool AddChild (const TArray<TSharedPtr<IASTNode>> &Nodes, CString *retsError = NULL) override;
		void AddNode (TSharedPtr<IASTNode> pChild) { m_Node.Insert(pChild); }
		virtual void DebugDump (const CString &sIndent) const override;
		virtual const IASTNode *FindDefinition (const CString &sID) const override;
		virtual IASTNode &GetChild (int iIndex) override { if (iIndex >= 0 && iIndex < GetChildCount()) return *m_Node[iIndex]; else throw CException(errFail); }
		virtual int GetChildCount () const override { return m_Node.GetCount(); }
		virtual const IASTNode &GetDefinition (int iIndex) const override { if (iIndex >= 0 && iIndex < GetDefinitionCount()) return *m_Types[iIndex]; else throw CException(errFail); }
		virtual int GetDefinitionCount () const override { return m_Types.GetCount(); }
		virtual const IASTNode &GetStatement (int iIndex) const { if (iIndex >= 0 && iIndex < GetStatementCount()) return *m_Statements[iIndex]; else throw CException(errFail); }
		virtual int GetStatementCount () const { return m_Statements.GetCount(); }
		virtual EASTType GetType () const override { return EASTType::Sequence; }
		virtual IASTNode &GetVarDef (int iIndex) override { if (iIndex >= 0 && iIndex < GetVarDefCount()) return *m_Vars[iIndex]; else throw CException(errFail); }
		virtual int GetVarDefCount () const override { return m_Vars.GetCount(); }

	private:
		bool AddToIndex (IASTNode &Node, CString *retsError);
		bool AddSymbol (IASTNode &Node, CString *retsError);

		TArray<TSharedPtr<IASTNode>> m_Node;
		TSortMap<CString, IASTNode *> m_Functions;
		TSortMap<CString, IASTNode *> m_Symbols;
		TSortMap<CString, IASTNode *> m_Types;
		TSortMap<CString, IASTNode *> m_Vars;
		TArray<IASTNode *> m_Statements;
	};

class CASTTypeRef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (IASTNode *pParent, const CString &sType)
			{
			CASTTypeRef *pNode = new CASTTypeRef(pParent);
			pNode->m_sType = sType;
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTTypeRef (IASTNode *pParent) : IASTNode(pParent)
			{ }

		virtual void DebugDump (const CString &sIndent) const override;
		virtual const CString &GetName () const override { return m_sType; }
		virtual EASTType GetType () const override { return EASTType::TypeRef; }

	private:
		CString m_sType;
	};

class CASTUnaryOp : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (IASTNode *pParent, EASTType iOp, TSharedPtr<IASTNode> pOperand)
			{
			CASTUnaryOp *pNode = new CASTUnaryOp(pParent);
			pNode->m_iOp = iOp;
			pNode->m_pOperand = pOperand;
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTUnaryOp (IASTNode *pParent) : IASTNode(pParent)
			{ }
		
		virtual void DebugDump (const CString &sIndent) const override;
		virtual IASTNode &GetChild (int iIndex) override { if (iIndex == 0) return *m_pOperand; else throw CException(errFail); }
		virtual int GetChildCount () const override { return 1; }
		virtual EASTType GetType () const override { return m_iOp; }

	private:
		EASTType m_iOp = EASTType::Unknown;
		TSharedPtr<IASTNode> m_pOperand;
	};

class CASTVarDef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (IASTNode *pParent, EASTType iVarType, int iOrdinal, const CString &sName, TSharedPtr<IASTNode> pTypeDef, TSharedPtr<IASTNode> pBody)
			{
			CASTVarDef *pNode = new CASTVarDef(pParent);
			pNode->m_iVarType = iVarType;
			pNode->m_sName = sName;
			pNode->m_pTypeDef = pTypeDef;
			pNode->m_pBody = pBody;
			pNode->m_iOrdinal = iOrdinal;
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTVarDef (IASTNode *pParent) : IASTNode(pParent)
			{ }

		virtual void DebugDump (const CString &sIndent) const override;
		virtual IASTNode &GetChild (int iIndex) { if (m_pBody && m_pBody->GetType() != EASTType::LiteralNull && iIndex == 0) return *m_pBody; else throw CException(errFail); }
		virtual int GetChildCount () const { return ((m_pBody && m_pBody->GetType() != EASTType::LiteralNull) ? 1 : 0); }
		virtual const CString &GetName () const override { return m_sName; }
		virtual int GetOrdinal () const override { return m_iOrdinal; }
		virtual EASTType GetType () const override { return m_iVarType; }
		virtual const IASTNode &GetTypeRef () const override { return *m_pTypeDef; }
		virtual bool IsStatement () const override;
		virtual bool IsVarDefinition () const override { return true; }

	private:
		EASTType m_iVarType = EASTType::Unknown;
		CString m_sName;
		TSharedPtr<IASTNode> m_pTypeDef;
		TSharedPtr<IASTNode> m_pBody;
		int m_iOrdinal = 0;
	};

class CASTVarRef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (IASTNode *pParent, const CString &sVar)
			{
			CASTVarRef *pNode = new CASTVarRef(pParent);
			pNode->m_sVar = sVar;
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTVarRef (IASTNode *pParent) : IASTNode(pParent)
			{ }
		
		virtual void DebugDump (const CString &sIndent) const override;
		virtual const CString &GetName () const override { return m_sVar; }
		virtual EASTType GetType () const override { return EASTType::VarRef; }

	private:
		CString m_sVar;
	};

