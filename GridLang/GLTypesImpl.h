//	GLTypesImpl.h
//
//	IGLType Implementations
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CGLFunctionType : public IGLType
	{
	public:
		virtual GLTypeClass GetClass () const override { return GLTypeClass::Function; }

	private:
		struct SArgDef
			{
			CString sName;				//	Propercase name
			IGLType *pType = NULL;
			};
		
		TArray<SArgDef> m_Args;
		const IGLType *m_pReturnType = NULL;
		bool m_bGlobal = false;			//	Does not need object instance
		bool m_bConstant = false;		//	No side-effects
		bool m_bPublic = false;			//	Accessible outside object
	};

class CGLNumberType : public IGLType
	{
	public:
		CGLNumberType (IGLType *pParent, const CString &sName, GLCoreType iType);

		virtual GLTypeClass GetClass () const override { return GLTypeClass::Number; }

	private:
		int m_iBits = 0;
		bool m_bFloat = false;
		bool m_bUnsigned = false;
	};

class CGLObjectType : public IGLType
	{
	public:
		CGLObjectType (IGLType *pParent, const CString &sName, const IASTNode &Node) : IGLType(pParent, sName),
				m_pDef(const_cast<IASTNode &>(Node).AddRef())
			{ }

		virtual GLTypeClass GetClass () const override { return GLTypeClass::Object; }
		
	private:
		CGLTypeNamespace m_Types;

		TSharedPtr<IASTNode> m_pDef;
	};

class CGLOrdinalType : public IGLType
	{
	public:
		CGLOrdinalType (IGLType *pParent, const CString &sName, const TArray<CString> &Ordinals) : IGLType(pParent, sName),
				m_Ordinals(Ordinals)
			{ }

		virtual GLTypeClass GetClass () const override { return GLTypeClass::Ordinal; }

	private:
		TArray<CString> m_Ordinals;
	};

class CGLPropertyType : public IGLType
	{
	public:
		CGLPropertyType (IGLType *pParent, const CString &sName, const IGLType &Type, const IASTNode &Node);

		virtual GLTypeClass GetClass () const override { return GLTypeClass::Property; }

	private:
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
		CGLSimpleType (IGLType *pParent, const CString &sName) : IGLType(pParent, sName)
			{ }

		void SetConcrete (bool bValue = true) { m_bConcrete = bValue; }

		virtual GLTypeClass GetClass () const override { return GLTypeClass::Simple; }

	private:
		bool m_bConcrete = false;		//	Concrete type
	};

class CGLTypeTree
	{
	public:
		void AddType (IGLType &Type);
		void Dump () const { for (int i = 0; i < m_Root.Children.GetCount(); i++) DumpNode(*m_Root.Children[i], NULL_STR); }

	private:
		struct SNode
			{
			IGLType *pType = NULL;
			TArray<TUniquePtr<SNode>> Children;
			};

		void DumpNode (const SNode &Node, const CString &sIndent) const;
		SNode *SetAt (IGLType *pType);

		SNode m_Root;
	};
