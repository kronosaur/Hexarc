//	GridLangTypes.h
//
//	GridLang Classes
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "GridLangAST.h"

enum class GLTypeClass
	{
	Simple,

	Number,
	Ordinal,
	Function,
	Property,
	Object,
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
		static TSharedPtr<IGLType> CreateAbstract (IGLType &Parent, const CString &sName);
		static TSharedPtr<IGLType> CreateConcrete (IGLType &Parent, const CString &sName);
		static TSharedPtr<IGLType> CreateConcreteNumber (IGLType &Parent, const CString &sName, GLCoreType iType);
		static TSharedPtr<IGLType> CreateObject (IGLType &Parent, const CString &sName, const IASTNode &Def);
		static TSharedPtr<IGLType> CreateOrdinals (IGLType &Parent, const CString &sName, const TArray<CString> &Ordinals);
		static TSharedPtr<IGLType> CreateProperty (IGLType &PropertyType, IGLType &Type, const CString &sName, const IASTNode &Def);
		static TSharedPtr<IGLType> CreateRoot ();

		virtual ~IGLType () { }

		virtual GLTypeClass GetClass () const = 0;
		const CString &GetName () const { return m_sName; }
		IGLType *GetParent () const { return m_pParent; }
		bool IsA (const IGLType &Type) const;

		IGLType *AddRef (void) { m_dwRefCount++; return this; }
		void Delete (void) { if (--m_dwRefCount == 0) delete this; }

	protected:
		IGLType (IGLType *pParent, const CString &sName) :
				m_pParent(pParent),
				m_sName(sName)
			{ }

	private:
		CString m_sName;				//	Propercase name of type.
		IGLType *m_pParent = NULL;		//	Parent of type.
		int m_dwRefCount = 1;
	};

class CGLNamespaceCtx;
class CGLTypeSystem;

class CGLTypeNamespace
	{
	public:
		bool DeclareTypes (CGLNamespaceCtx &Ctx, const IASTNode &AST, CString *retsError = NULL);
		void DeletAll () { m_Types.DeleteAll(); }
		void Dump () const;
		IGLType *GetAt (const CString &sID) const;
		bool Insert (TSharedPtr<IGLType> pType);
		bool IsDeclared (const CString &sName) const { return GetAt(strToLower(sName)) != NULL; }

	private:
		bool DeclareType (CGLNamespaceCtx &Ctx, const IASTNode &AST, const IASTNode &Def, CString *retsError = NULL);

		bool DeclareClass (CGLNamespaceCtx &Ctx, const IASTNode &AST, const IASTNode &Def, CString *retsError = NULL);
		bool DeclareFunction (CGLNamespaceCtx &Ctx, const IASTNode &AST, const IASTNode &Def, CString *retsError = NULL);
		bool DeclareIfNeeded (CGLNamespaceCtx &Ctx, const IASTNode &AST, const CString &sTypeID, CString *retsError = NULL);
		bool DeclareProperty (CGLNamespaceCtx &Ctx, const IASTNode &AST, const IASTNode &Def, CString *retsError = NULL);
		bool DeclareOrdinal (CGLNamespaceCtx &Ctx, const IASTNode &AST, const IASTNode &Def, CString *retsError = NULL);
		bool InterpretTypeRef (CGLNamespaceCtx &Ctx, const IASTNode &AST, const IASTNode &TypeRef, IGLType *&retpType, CString *retsError = NULL);

		TSortMap<CString, TSharedPtr<IGLType>> m_Types;
	};

class CGLTypeSystem
	{
	public:
		IGLType &GetCoreType (GLCoreType iType) const { if ((int)iType < 0 || iType >= GLCoreType::enumCount) throw CException(errFail); return *m_Core[(int)iType]; }
		const CGLTypeNamespace &GetGlobals () const { return m_Types; }
		CGLTypeNamespace &GetGlobals () { return m_Types; }
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
		CGLNamespaceCtx (CGLTypeSystem &Types);

		IGLType *Find (const CString &sName) const;
		IGLType &GetCoreType (GLCoreType iType) const { return m_Types.GetCoreType(iType); }
		void Pop () { if (m_Scopes.GetCount() > 0) m_Scopes.Delete(m_Scopes.GetCount() - 1); else throw CException(errFail); }
		void Push (CGLTypeNamespace &Namespace) { m_Scopes.Insert(&Namespace); }

	private:
		CGLTypeSystem &m_Types;
		TArray<CGLTypeNamespace *> m_Scopes;
	};

