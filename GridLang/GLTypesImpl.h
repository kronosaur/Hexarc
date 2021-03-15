//	GLTypesImpl.h
//
//	IGLType Implementations
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CGLFunctionFamilyType : public IGLType
	{
	public:
		virtual GLTypeClass GetClass () const override { return GLTypeClass::FunctionFamily; }

	private:
		CGLTypeNamespace m_Functions;
	};

class CGLFunctionType : public IGLType
	{
	public:
		CGLFunctionType (const IGLType *pParent, const IGLType *pScope, const CString &sName);

		virtual GLTypeClass GetClass () const override { return GLTypeClass::Function; }

	private:
		struct SArgDef
			{
			CString sName;				//	Propercase name
			const IGLType *pType = NULL;
			};

		virtual bool OnDefine (CGLNamespaceCtx &Ctx, CString *retsError = NULL) override { return true; }
		
		TArray<SArgDef> m_Args;
		const IGLType *m_pReturnType = NULL;
		bool m_bGlobal = false;			//	Does not need object instance
		bool m_bConstant = false;		//	No side-effects
		bool m_bPublic = false;			//	Accessible outside object
	};

class CGLNumberType : public IGLType
	{
	public:
		CGLNumberType (const IGLType *pParent, const IGLType *pScope, const CString &sName, GLCoreType iType);

		virtual GLTypeClass GetClass () const override { return GLTypeClass::Number; }

	private:
		virtual bool OnDefine (CGLNamespaceCtx &Ctx, CString *retsError = NULL) override { return true; }

		int m_iBits = 0;
		bool m_bFloat = false;
		bool m_bUnsigned = false;
	};

class CGLObjectType : public IGLType
	{
	public:
		CGLObjectType (const CString &sName, const CString &sCanonical) : IGLType(sName, sCanonical)
			{ }

		virtual GLTypeClass GetClass () const override { return GLTypeClass::Object; }
		
	private:
		virtual bool OnDeclare (CGLNamespaceCtx &Ctx, const IASTNode &Def, CString *retsError = NULL) override;
		virtual bool OnDefine (CGLNamespaceCtx &Ctx, CString *retsError = NULL) override;
		virtual const IGLType *OnResolveSymbol (const CString &sSymbol) const override { return m_Types.ResolveSymbol(sSymbol); }

		CGLTypeNamespace m_Types;
	};

class CGLOrdinalType : public IGLType
	{
	public:
		CGLOrdinalType (const IGLType *pParent, const IGLType *pScope, const CString &sName, const TArray<CString> &Ordinals) : IGLType(pParent, pScope, sName),
				m_Ordinals(Ordinals)
			{ }

		virtual GLTypeClass GetClass () const override { return GLTypeClass::Ordinal; }

	private:
		virtual bool OnDefine (CGLNamespaceCtx &Ctx, CString *retsError = NULL) override { return true; }

		TArray<CString> m_Ordinals;
	};

class CGLPropertyType : public IGLType
	{
	public:
		CGLPropertyType (const IGLType *pParent, const IGLType *pScope, const CString &sName, const IGLType &Type, const IASTNode &Node);

		virtual GLTypeClass GetClass () const override { return GLTypeClass::Property; }

	private:
		virtual bool OnDefine (CGLNamespaceCtx &Ctx, CString *retsError = NULL) override { return true; }

		const IGLType &m_Type;			//	Property datatype
		bool m_bGlobal = false;			//	Single instance for all objects
		bool m_bConstant = false;		//	Cannot be modified after init
		bool m_bPublic = false;			//	Accessible outside type
		bool m_bReadOnly = false;		//	Cannot be modified outside type

		TSharedPtr<IASTNode> m_pDef;
	};

class CGLSimpleType : public IGLType
	{
	public:
		CGLSimpleType (const IGLType *pParent, const IGLType *pScope, const CString &sName) : IGLType(pParent, pScope, sName)
			{ }

		void SetConcrete (bool bValue = true) { m_bConcrete = bValue; }

		virtual GLTypeClass GetClass () const override { return GLTypeClass::Simple; }

	private:
		virtual bool OnDefine (CGLNamespaceCtx &Ctx, CString *retsError = NULL) override { return true; }

		bool m_bConcrete = false;		//	Concrete type
	};

class CGLTypeTree
	{
	public:
		void AddType (const IGLType &Type);
		void Dump () const { for (int i = 0; i < m_Root.Children.GetCount(); i++) DumpNode(*m_Root.Children[i], NULL_STR); }

	private:
		struct SNode
			{
			const IGLType *pType = NULL;
			TArray<TUniquePtr<SNode>> Children;
			};

		void DumpNode (const SNode &Node, const CString &sIndent) const;
		SNode *SetAt (const IGLType *pType);

		SNode m_Root;
	};
