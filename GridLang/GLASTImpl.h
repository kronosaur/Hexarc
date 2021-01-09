//	GLASTImpl.h
//
//	IASTNode Implementations
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

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
		static TSharedPtr<IASTNode> Create (EASTType iOp, TSharedPtr<IASTNode> pLeft, TSharedPtr<IASTNode> pRight)
			{
			CASTBinaryOp *pNode = new CASTBinaryOp;
			pNode->m_iOp = iOp;
			pNode->m_pLeft = pLeft;
			pNode->m_pRight = pRight;
			return TSharedPtr<IASTNode>(pNode);
			}
		
		virtual void DebugDump (const CString &sIndent) const override;
		virtual IASTNode &GetChild (int iIndex) const override
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
		static TSharedPtr<IASTNode> Create (const CString &sID, const CString &sBaseID, TSharedPtr<IASTNode> pBody)
			{
			CASTClassDef *pNode = new CASTClassDef;
			pNode->m_sID = sID;
			pNode->m_sBaseID = sBaseID;
			pNode->m_pBody = pBody;
			return TSharedPtr<IASTNode>(pNode);
			}

		virtual void DebugDump (const CString &sIndent) const override;
		virtual const CString &GetBaseName () const override { return m_sBaseID; }
		virtual IASTNode &GetChild (int iIndex) const override { if (iIndex == 0) return *m_pBody; else throw CException(errFail); }
		virtual int GetChildCount () const override { return 1; }
		virtual const CString &GetName () const override { return m_sID; }
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
		virtual const CString &GetName () const override { return m_pFunction->GetName(); }
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
		virtual const CString &GetName () const override { return m_sFunction; }
		virtual EASTType GetType () const override { return EASTType::FunctionDef; }
		virtual const IASTNode &GetTypeRef () const override { return *m_pTypeDef; }

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
		virtual int GetDefinitionCount () const override { return m_Ordinals.GetCount(); }
		virtual const CString &GetDefinitionString (int iIndex) const override { if (iIndex >= 0 && iIndex < GetDefinitionCount()) return m_Ordinals[iIndex]; else throw CException(errFail); }
		virtual const CString &GetName () const override { return m_sName; }
		virtual EASTType GetType () const override { return EASTType::OrdinalDef; }

	private:
		CString m_sName;
		TArray<CString> m_Ordinals;
	};

class CASTSequence : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (TArray<TSharedPtr<IASTNode>> Nodes, CString *retsError);

		void AddNode (TSharedPtr<IASTNode> pChild) { m_Node.Insert(pChild); }
		virtual void DebugDump (const CString &sIndent) const override;
		virtual const IASTNode *FindDefinition (const CString &sID) const override;
		virtual IASTNode &GetChild (int iIndex) const override { if (iIndex >= 0 && iIndex < GetChildCount()) return *m_Node[iIndex]; else throw CException(errFail); }
		virtual int GetChildCount () const override { return m_Node.GetCount(); }
		virtual const IASTNode &GetDefinition (int iIndex) const override { if (iIndex >= 0 && iIndex < GetDefinitionCount()) return *m_Types[iIndex]; else throw CException(errFail); }
		virtual int GetDefinitionCount () const override { return m_Types.GetCount(); }
		virtual EASTType GetType () const override { return EASTType::Sequence; }

	private:
		TArray<TSharedPtr<IASTNode>> m_Node;
		TSortMap<CString, IASTNode *> m_Types;
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
		virtual const CString &GetName () const override { return m_sType; }
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
		virtual IASTNode &GetChild (int iIndex) const override { if (iIndex == 0) return *m_pOperand; else throw CException(errFail); }
		virtual int GetChildCount () const override { return 1; }
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
		virtual const CString &GetName () const override { return m_sName; }
		virtual EASTType GetType () const override { return m_iVarType; }
		virtual const IASTNode &GetTypeRef () const override { return *m_pTypeDef; }

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
		virtual const CString &GetName () const override { return m_sVar; }
		virtual EASTType GetType () const override { return EASTType::VarRef; }

	private:
		CString m_sVar;
	};

