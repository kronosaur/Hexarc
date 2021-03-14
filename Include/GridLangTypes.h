//	GridLangTypes.h
//
//	GridLang Classes
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "GridLangAST.h"

class CGLNamespaceCtx;

enum class GLTypeClass
	{
	Simple,

	Number,
	Ordinal,
	Function,
	Property,
	Object,

	FunctionFamily,
	};

enum class GLCoreType
	{
	Type = 0,							//	Type: Any type

	NullType,							//	NullType
	Bool,								//	Bool: A true/false value

	Number,								//	Number: Any number
	Real,								//	Real: A real number
	Complex,							//	Complex: A complex number
	Integer,							//	Integer: An integer real
	Float,								//	Float: A floating point real
	Signed,								//	Signed: A signed integer
	Unsigned,							//	Unsigned: An unsigned integer
	Ordinal,							//	Ordinal: An unsigned integer

	Int32,								//	Int32: 32-bit integer
	Int64,								//	Int64: 64-bit integer
	IntIP,								//	IntIP: Infinite precision integer
	UInt32,								//	UInt32: unsigned 32-bit integer
	UInt64,								//	UInt64: unsigned 64-bit integer

	Float64,							//	Float64: 64-bit float

	String,								//	String: UTF-8 string
	Object,								//	Object: an abstract object type
	Property,							//	Property: a property definition
	Function,							//	Function: a function definition

	enumCount,							//	Total count of core types
	};

class IGLType
	{
	public:
		static TSharedPtr<IGLType> CreateAbstract (const IGLType &IsA, const IGLType *pScope, const CString &sName);
		static TSharedPtr<IGLType> CreateConcrete (const IGLType &IsA, const IGLType *pScope, const CString &sName);
		static TSharedPtr<IGLType> CreateConcreteNumber (const IGLType &IsA, const IGLType *pScope, const CString &sName, GLCoreType iType);
		static TSharedPtr<IGLType> CreateFunction (const IGLType &IsA, const IGLType *pScope, const CString &sName);
		static TSharedPtr<IGLType> CreateObject (const IGLType &IsA, const IGLType *pScope, const CString &sName, const IASTNode &Def);
		static TSharedPtr<IGLType> CreateOrdinals (const IGLType &IsA, const IGLType *pScope, const CString &sName, const TArray<CString> &Ordinals);
		static TSharedPtr<IGLType> CreateProperty (const IGLType &IsA, const IGLType *pScope, const IGLType &Type, const CString &sName, const IASTNode &Def);
		static TSharedPtr<IGLType> CreateRoot ();

		virtual ~IGLType () { }

		virtual GLTypeClass GetClass () const = 0;
		void AccumulateSymbols (TSortMap<CString, const IGLType *> &retSymbols) const { retSymbols.Insert(m_sSymbol, this); OnAccumulateSymbols(retSymbols); }
		bool Declare (CGLNamespaceCtx &Ctx, const IASTNode &Def, CString *retsError);
		bool Define (CGLNamespaceCtx &Ctx, CString *retsError = NULL);
		const CString &GetName () const { return m_sName; }
		const IGLType *GetParent () const { return m_pParent; }
		const CString &GetSymbol () const { return m_sSymbol; }
		bool IsA (const IGLType &Type) const;
		bool IsDefined () const { return m_bDefined; }
		const IGLType *ResolveSymbol (const CString &sSymbol) const { return OnResolveSymbol(sSymbol); }

		IGLType *AddRef (void) { m_dwRefCount++; return this; }
		void Delete (void) { if (--m_dwRefCount == 0) delete this; }

	protected:
		IGLType (const IGLType *pParent, const IGLType *pScope, const CString &sName) :
				m_pParent(pParent),
				m_pScope(pScope),
				m_sName(sName),
				m_sSymbol(!pScope ? sName : NULL_STR)
			{ }

		void GenerateSymbol ();

	private:
		virtual void OnAccumulateSymbols (TSortMap<CString, const IGLType *> &retSymbols) const { }
		virtual bool OnDeclare (CGLNamespaceCtx &Ctx, const IASTNode &Def, CString *retsError = NULL) { return true; }
		virtual bool OnDefine (CGLNamespaceCtx &Ctx, CString *retsError = NULL) = 0;
		virtual CString OnGenerateLocalSymbol () const { return ::strToLower(m_sName); }
		virtual const IGLType *OnResolveSymbol (const CString &sSymbol) const { return NULL; }

		CString m_sName;					//	Propercase name of type.
		CString m_sSymbol;					//	Globally unique symbol.
		const IGLType *m_pParent = NULL;	//	Inheritance parent of type.
		const IGLType *m_pScope = NULL;		//	Scope in which type is defined (this is the definition's
											//		lexical scope).
		int m_dwRefCount = 1;

		bool m_bDefined = false;
	};

class CGLTypeSystem;

class CGLTypeNamespace
	{
	public:
		void AccumulateSymbols (TSortMap<CString, const IGLType *> &retSymbols) const;
		bool DeclareTypes (CGLNamespaceCtx &Ctx, IGLType *pScope, const IASTNode &AST, CString *retsError = NULL);
		bool DefineTypes (CGLNamespaceCtx &Ctx, CString *retsError = NULL);
		void DeletAll () { m_Types.DeleteAll(); }
		void Dump () const;
		const IGLType *GetAt (const CString &sID) const;
		int GetCount () const { return m_Types.GetCount(); }
		const IGLType &GetDefinition (int iIndex) const { return *m_Types[iIndex]; }
		bool Insert (TSharedPtr<IGLType> pType);
		bool IsDeclared (const CString &sName) const { return GetAt(strToLower(sName)) != NULL; }
		const IGLType *ResolveSymbol (const CString &sSymbol) const;

	private:
		bool DeclareType (CGLNamespaceCtx &Ctx, IGLType *pScope, const IASTNode &AST, const IASTNode &Def, CString *retsError = NULL);

		bool DeclareClass (CGLNamespaceCtx &Ctx, IGLType *pScope, const IASTNode &AST, const IASTNode &Def, CString *retsError = NULL);
		bool DeclareFunction (CGLNamespaceCtx &Ctx, IGLType *pScope, const IASTNode &AST, const IASTNode &Def, CString *retsError = NULL);
		bool DeclareIfNeeded (CGLNamespaceCtx &Ctx, IGLType *pScope, const IASTNode &AST, const CString &sTypeID, CString *retsError = NULL);
		bool DeclareProperty (CGLNamespaceCtx &Ctx, IGLType *pScope, const IASTNode &AST, const IASTNode &Def, CString *retsError = NULL);
		bool DeclareOrdinal (CGLNamespaceCtx &Ctx, IGLType *pScope, const IASTNode &AST, const IASTNode &Def, CString *retsError = NULL);
		bool InterpretTypeRef (CGLNamespaceCtx &Ctx, IGLType *pScope, const IASTNode &AST, const IASTNode &TypeRef, const IGLType *&retpType, CString *retsError = NULL);

		TSortMap<CString, TSharedPtr<IGLType>> m_Types;
	};

class CGLTypeSystem
	{
	public:
		void DebugDump () const;
		const IGLType &GetCoreType (GLCoreType iType) const { if ((int)iType < 0 || iType >= GLCoreType::enumCount) throw CException(errFail); return *m_Core[(int)iType]; }
		const CGLTypeNamespace &GetDefinitions () const { return m_Types; }
		CGLTypeNamespace &GetDefinitions () { return m_Types; }
		bool InitFromAST (const CGridLangAST &AST, CString *retsError = NULL);
		bool Insert (TSharedPtr<IGLType> pType) { return m_Types.Insert(pType); }
		bool IsDefined (const CString &sName) const;

	private:
		void AddCoreTypes ();
		IGLType *InsertOrThrow (TSharedPtr<IGLType> pType) { if (!Insert(pType)) throw CException(errFail); return pType; }

		CGLTypeNamespace m_Types;
		IGLType *m_Core[(int)GLCoreType::enumCount] = { NULL };
	};

class CGLNamespaceCtx
	{
	public:
		CGLNamespaceCtx (const CGLTypeSystem &Types);

		const IGLType *Find (const CString &sName) const;
		const IGLType &GetCoreType (GLCoreType iType) const { return m_Types.GetCoreType(iType); }
		void Pop () { if (m_Scopes.GetCount() > 0) m_Scopes.Delete(m_Scopes.GetCount() - 1); else throw CException(errFail); }
		void Push (const CGLTypeNamespace &Namespace) { m_Scopes.Insert(&Namespace); }

	private:
		const CGLTypeSystem &m_Types;
		TArray<const CGLTypeNamespace *> m_Scopes;
	};

class CGLPushNamespace
	{
	public:
		CGLPushNamespace (CGLNamespaceCtx &Ctx, const CGLTypeNamespace &Namespace) :
				m_Ctx(Ctx)
			{
			Ctx.Push(Namespace);
			}

		~CGLPushNamespace ()
			{
			m_Ctx.Pop();
			}

	private:
		CGLNamespaceCtx &m_Ctx;
	};
