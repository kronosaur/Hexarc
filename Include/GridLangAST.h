//	GridLangAST.h
//
//	GridLang Internals
//	Copyright (c) 2020 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "GridLangParser.h"

class CGridLangLibraries;

enum class EASTType
	{
	Unknown,

	AnonEnum,
	AnonSchema,
	ArgDef,
	ArrayDef,
	ArrayDynamicDef,
	Attributes,
	CastDef,
	ClassDef,
	ColumnRef,
	ConstDef,
	ConstructorDef,
	DictionaryDef,
	DoForEachLoop,
	DoForLoop,
	DoIf,
	DoWhileLoop,
	EnumDef,
	EvalDo,
	EvalIf,
	FunctionCall,
	FunctionDef,
	FunctionEventPlaceholder,
	FunctionPlaceholder,
	GlobalDef,
	LibraryConstDef,
	LibraryFunctionDef,
	LibraryObjectCreator,
	LibraryDef,
	LibraryRef,
	LibraryTypeDef,
	LiteralAll,
	LiteralArray,
	LiteralNaN,
	LiteralNull,
	LiteralFalse,
	LiteralFloat,
	LiteralInteger,
	LiteralIPInteger,
	LiteralString,
	LiteralStruct,
	LiteralStructHeader,
	LiteralTrue,
	LoopScope,
	InstanceEventMethodDef,
	InstanceKeyMemberDef,
	InstanceMemberDef,
	InstanceMethodDef,
	InstanceValueDef,
	MacroCall,
	ModuleRef,
	NullableDef,
	OpArithmeticAnd,
	OpArithmeticOr,
	OpArray,
	OpAssignment,
	OpConcatenate,
	OpDivide,
	OpEquals,
	OpFunctionCall,
	OpGreaterThan,
	OpGreaterThanEquals,
	OpIdentical,
	OpIn,
	OpLessThan,
	OpLessThanEquals,
	OpLiteralMemberAssignment,
	OpLogicalAnd,
	OpLogicalOr,
	OpMemberAccess,
	OpMinus,
	OpMutateConcat,
	OpMutateDivide,
	OpMutateMinus,
	OpMutateMod,
	OpMutatePlus,
	OpMutatePower,
	OpMutateTimes,
	OpNonNull,
	OpNot,
	OpNotEquals,
	OpNotIdentical,
	OpNotIn,
	OpNullable,
	OpObjectInit,
	OpOf,
	OpPlus,
	OpTensorRef,
	OpTimes,
	OpPower,
	OpModulo,
	OpRange,
	OpReturn,
	OpSpread,
	OpUsing,
	OrdinalDef,
	PropertyDef,
	SchemaDef,
	Sequence,
	SubRangeDef,
	TableDef,
	TypeRef,
	TypeSequence,
	ValueDef,
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
		enum class EMemberAccess
			{
			ObjProperty,				//	Call GetProperty on the object
			EnumValue,					//	retdValue is an enum value
			ValueDef,					//	Invoke a method to get value.
			Expression,					//	The result is a column expression.
			SchemaColumn,				//	retdValue is the column ordinal

			Error,						//	retdValue is an error message.
			};

		struct SResolveTypeCtx
			{
			bool bAllowSchemaStub = false;
			TArray<CDatum> NewTypes;
			};

		IASTNode (const CGridLangParser::SPos& Pos) :
				m_SourcePos(Pos)
			{ }

		virtual ~IASTNode () { }

		virtual bool Accumulate (IASTNode& Src, CString* retsError = NULL) { throw CException(errFail); }
		void AccumulateModules (TSortMap<CString, CString>& retModules) const;
		virtual void AccumulateTypes (TSortMap<CString, CDatum>& retTypes) const;
		EMemberAccess CalcMemberAccess (CDatum& retdResult) const;
		virtual bool CanBeCalledWith (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, CDatum& retdReturnType, CString* retsError = NULL) const { return false; }
		bool CheckColExprSchema (CDatum dType, CString* retsError = NULL) const;
		bool CheckLiteralSchema (CDatum dType, CString* retsError = NULL) const;
		bool ComposeError (const CString &sError, CString *retsError) const;
		virtual TSharedPtr<IASTNode> Clone () const = 0;
		static TSharedPtr<IASTNode> CreateVarRef (const CGridLangParser::SPos& Pos, const CString& sIdentifier1, const CString& sIdentifier2 = NULL_STR);
		virtual void DebugDump (const CString &sIndent = NULL_STR) const { }
		virtual void DeleteAll () { }
		virtual bool EmbedsType (const IASTNode& Type) const { return false; }
		virtual bool EvalConstExpression (CDatum& retdValue) const { return false; }
		bool CalcNarrowingPath (CString& retsPath) const;
		IASTNode* FindChildByName (const CString& sName);
		IASTNode* FindClassDefinition (const CString& sName);
		virtual const IASTNode *FindDefinition (const CString &sID) const { return NULL; }
		virtual CDatum FindNarrowedType (const CString& sPath, const IASTNode* pPos = NULL) const;
		IASTNode* FindTypeDefinition (const CString& sName);
		const IASTNode* FindTypeDefinition (const CString& sName) const { return const_cast<IASTNode*>(this)->FindTypeDefinition(sName); }
		IASTNode* FindReference (const CString &sName, IASTNode *pPos = NULL, int iLevel = 0, int *retiLevelFound = NULL);
		const IASTNode* FindReference (const CString &sName, const IASTNode *pPos = NULL, int iLevel = 0, int *retiLevelFound = NULL) const { return const_cast<IASTNode*>(this)->FindReference(sName, const_cast<IASTNode*>(pPos), iLevel, retiLevelFound); }
		virtual IASTNode* FindChildSymbol (const CString &sName) { return NULL; }
		const IASTNode* FindChildSymbol (const CString &sName) const { return const_cast<IASTNode*>(this)->FindChildSymbol(sName); }
		virtual const IASTNode* FindTypeRef () const { return &GetTypeRef(); }
		virtual CDatum GetAttributes () const { throw CException(errFail); }
		virtual const CString &GetBaseName () const { return NULL_STR; }
		virtual IASTNode &GetChild (int iIndex) { throw CException(errFail); }
		const IASTNode &GetChild (int iIndex) const { return const_cast<IASTNode *>(this)->GetChild(iIndex); }
		virtual int GetChildCount () const { return 0; }
		virtual TSharedPtr<IASTNode> GetChildReference (int iIndex) const { throw CException(errFail); }
		virtual int GetCodeID () const { return 0; }
		virtual const IASTNode &GetDefinition (int iIndex) const { throw CException(errFail); }
		virtual int GetDefinitionCount () const { return 0; }
		virtual const CString &GetDefinitionString (int iIndex) const { throw CException(errFail); }
		virtual CDatum GetElementType () const { return CAEONTypes::Get(IDatatype::ANY); }
		virtual const CString &GetFullyQualifiedName () const { return NULL_STR; }
		TArray<TSharedPtr<IASTNode>> GetLibraryRefs () const;
		virtual const CString &GetName () const { return GetTypeName(); }
		virtual int GetOrdinal () const { return -1; }
		const IASTNode &GetParent () const { if (m_pParent) return *m_pParent; else throw CException(errFail); }
		IASTNode &GetParent () { if (m_pParent) return *m_pParent; else throw CException(errFail); }
		virtual IASTNode* GetReference (int *retiLevel = NULL) { return NULL; }
		const IASTNode* GetReference (int *retiLevel = NULL) const { return const_cast<IASTNode*>(this)->GetReference(retiLevel); }
		const IASTNode &GetRoot () const { return const_cast<IASTNode*>(this)->GetRoot(); }
		virtual IASTNode &GetRoot () { return *this; }
		const CGridLangParser::SPos &GetSourcePos () const { return m_SourcePos; }
		virtual IASTNode &GetStatement (int iIndex) { throw CException(errFail); }
		virtual int GetStatementCount () const { return 0; }
		virtual CDatum GetStaticType () const { return GetTypeDef(); }
		virtual IASTNode* GetStaticTypeDef () { return NULL; }
		virtual CString GetStringValue () const { return NULL_STR; }
		virtual EASTType GetType () const = 0;
		virtual CDatum GetTypeDef () const { return CAEONTypes::Get(IDatatype::ANY); }
		const CString& GetTypeName () const { return GetTypeName(GetType()); }
		virtual const IASTNode &GetTypeRef () const { throw CException(errFail); }
		virtual CDatum GetValue () const { return CDatum(); }
		virtual IASTNode &GetVarDef (int iIndex) { throw CException(errFail); }
		const IASTNode &GetVarDef (int iIndex) const { return const_cast<IASTNode*>(this)->GetVarDef(iIndex); }
		virtual int GetVarDefCount () const { return 0; }
		virtual bool HasClosure () const;
		bool HasArgsOfType (DWORD dwTypeID) const;
		bool HasParent () const { return m_pParent != NULL; }
		virtual bool HasScope () const { return false;}
		virtual void AccumulateStaticNamespaces (TArray<CString>& retNamespaces) const { }
		virtual bool HasStaticNamespace (CStringView sSymbol) const { return false; }
		virtual bool InsertChild (TSharedPtr<IASTNode> &Node, int iPos = -1, CString *retsError = NULL) { throw CException(errFail); }
		virtual bool InsertChildren (const TArray<TSharedPtr<IASTNode>> &Nodes, int iPos = -1, CString *retsError = NULL) { throw CException(errFail); }
		virtual bool IsColumnExpression (CAEONExpression::EOp* retiOp = NULL) const { return false; }
		virtual bool IsConstExpression () const { return false; }
		bool IsEnumDef () const;
		virtual bool IsExpression () const { return false; }
		virtual bool IsFunctionDefinition () const { return false; }
		virtual bool IsImmutable () const { return false; }
		virtual bool IsInferredType () const { return false; }
		virtual bool IsHoisted () const { return false; }
		bool IsMapColExpression () const;
		bool IsSameFunctionType (const IASTNode& Src, CString* retsError = NULL) const;
		bool IsAssignmentTarget () const;
		virtual bool IsStatement () const { return false; }
		virtual bool IsTypeDefinition () const { return false; }
		virtual bool IsUndefined () const { return false; }
		virtual bool IsVarDefinition () const { return false; }
		virtual bool IsVoidReturn () const { return false; }
		bool LoadTypes (TSortMap<CString, CDatum>& retTypes, CString* retsError = NULL);
		virtual void Mark () { }
		virtual void ReplaceChild (int iIndex, TSharedPtr<IASTNode> pNode) { throw CException(errFail); }
		virtual void ReplaceRoot (TSharedPtr<IASTNode> pNode) { throw CException(errFail); }
		virtual bool ResolveReductions (TSharedPtr<IASTNode>& retReplace, CString* retsError = NULL) { retReplace = NULL; return true; }
		virtual bool ResolveLibraryReference (IASTNode& Ref, TSharedPtr<IASTNode> pLibrary, CString* retsError = NULL) { throw CException(errFail); }
		virtual bool ResolveReferences (CString* retsError = NULL) { return true; }
		virtual bool ResolveTypes (SResolveTypeCtx& Ctx, CDatum* retdStaticType = NULL, CString* retsError = NULL);
		virtual bool ResolveTypesForMember (SResolveTypeCtx& Ctx, const CString& sName, CDatum* retdStaticType = NULL, CString* retsError = NULL);
		virtual void SetCodeID (int iID) { EASTType iType = GetType(); throw CException(errFail); }
		virtual void SetOrdinal (int iOrdinal) { throw CException(errFail); }
		void SetParent (IASTNode &Parent) { m_pParent = &Parent; }
		virtual void SetReference (IASTNode& Reference, int iLevel = 0) { }
		virtual void SetReferenceOwned (TSharedPtr<IASTNode> pNode) { }
		virtual void SetTypeRef (TSharedPtr<IASTNode> pNode) { }

		IASTNode *AddRef (void) { m_dwRefCount++; return this; }
		void Delete (void) { if (--m_dwRefCount == 0) delete this; }

		static CDatum CalcInferredTypeForLoopVar (SResolveTypeCtx& Ctx, CDatum dType, CString* retsError = NULL);
		static CDatum CalcInferredTypeForVar (SResolveTypeCtx& Ctx, CDatum dType);
		static const CString& GetTypeName (EASTType iType);
		static const IASTNode& GetTypeRefAny ();
		static bool IsStructHeaderTable (const TArray<TSharedPtr<IASTNode>> &Nodes);
		static bool IsArrayIndexSupported (const IDatatype& ArrayType, const IDatatype& IndexType, bool bMutable, CString* retsError = NULL);
		static bool IsAssignmentSupported (const IASTNode& Left, const IASTNode& Value, CString* retsError = NULL);

	protected:

		void AddChild (TSharedPtr<IASTNode> &pPointer, TSharedPtr<IASTNode> pChild)
			{
			pPointer = pChild;
			if (pChild)
				pChild->SetParent(*this);
			}

		void AddChildClone (TSharedPtr<IASTNode>& pPointer, const IASTNode *pChild)
			{
			if (pChild)
				{
				pPointer = pChild->Clone();
				pPointer->SetParent(*this);
				}
			}

		static void AccumulateTypes (CDatum dType, TSortMap<CString, CDatum>& retTypes);
		static bool FindArrayType (const TArray<CDatum>& Types, CDatum dElementType, CDatum* retdType = NULL);
		static bool FindDictionaryType (const TArray<CDatum>& Types, CDatum dKeyType, CDatum dElementType, CDatum* retdType = NULL);
		static bool FindMatrixType (const TArray<CDatum>& Types, CDatum dElementType, const TArray<CDatum>& DimTypes, CDatum* retdType = NULL);
		static bool FindNullableType (const TArray<CDatum>& Types, CDatum dElementType, CDatum* retdType = NULL);
		static bool FindSubRangeType (const TArray<CDatum>& Types, int iMin, int iMax, CDatum* retdType = NULL);
		static bool FindTableType (const TArray<CDatum>& Types, CDatum dVariantType, CDatum* retdType = NULL);
		static CDatum FindType (TSortMap<CString, CDatum>& Types, const CString& sName, IASTNode& Scope);
		bool ResolveReductionsForChildren (TSharedPtr<IASTNode>& retReplace, CString* retsError = NULL);
		bool ResolveReductionsForChildrenAndRoot (TSharedPtr<IASTNode>& retReplace, CString* retsError = NULL);
		bool ResolveReferencesForChildren (CString* retsError = NULL);
		bool ResolveReferencesForChildrenAndRoot (CString* retsError = NULL);

		CRecursionState m_rs;

	private:

		IASTNode *m_pParent = NULL;			//	Lexical parent
		CGridLangParser::SPos m_SourcePos;	//	Position in the source
		int m_dwRefCount = 1;

		struct STypeDesc
			{
			EASTType iType = EASTType::Unknown;
			CString sName;
			};

		static const STypeDesc m_TypeDesc[(int)EASTType::LastType];
		static TSharedPtr<IASTNode> m_pTypeRefAny;
	};

class CASTSymbolTable
	{
	public:
		bool AddSymbol (TSharedPtr<IASTNode> pNode);
		void EnterScope (const CString& sName);
		void ExitScope ();
		IASTNode* Find (const CString& sName) const;
		int GetScopeCount () const { return m_Symbols.GetCount(); }

	private:
		struct SScope
			{
			int iLevel = 0;

			CString sName;
			TSortMap<CString, TSharedPtr<IASTNode>> Symbols;
			};

		IASTNode* FindInScope (const SScope& Scope, const CString& sSymbol) const;
		const SScope& GetScope () const;
		SScope& GetScope ();

		TArray<SScope> m_Symbols;
	};

enum class EGridLangStatus
	{
	Unknown,

	OK,					//	Compiled OK
	Error,				//	1 or more compile errors
	};

class CGridLangResult
	{
	public:

		enum class ELibrarySource
			{
			Unknown,

			Core,					//	Built-in library
			Inline,					//	Library defined in source code
			Public,					//	/archive/libraries
			Private,				//	~/home/libraries
			};

		struct SLibraryInfo
			{
			ELibrarySource iSource = ELibrarySource::Unknown;
			CString sName;			//	E.g., "UI"
			CString sPath;			//	E.g., "/file/ABCD1234"
			};

		struct SParamInfo
			{
			CString sName;
			CString sTypename;
			CString sDesc;
			CDatum dJSONSchema;		//	The type as a JSON schema (optional)
			bool bOptional = false;
			};

		struct SEntryPoint
			{
			CString sName;
			CString sDesc;
			TArray<CString> Tags;

			TArray<SParamInfo> Params;
			CString sReturnTypename;
			CString sReturnDesc;
			CDatum dReturnJSONSchema;	//	The return type as a JSON schema (optional)
			};

		void AddEntryPoint (CStringView sName, CDatum dFuncType, CDatum dFuncAttribs);
		void AddError (const CGridLangParser::SPos& Pos, CStringView sError, DWORD dwErrorID = 0);
		void AddSourceFile (CStringView sFilename, int iLines) { m_Files.Insert(sFilename); m_iLines += iLines; }
		CDatum AsDatum () const;
		const TArray<CString>& GetErrors () const { return m_Errors; }
		CStringView GetFirstError () const { return (m_Errors.GetCount() > 0 ? m_Errors[0] : NULL_STR); }
		const TArray<CString>& GetFiles () const { return m_Files; }
		const TArray<SLibraryInfo>& GetLibraries () const { return m_Libraries; }
		EGridLangStatus GetStatus () const { return (m_Errors.GetCount() > 0 ? EGridLangStatus::Error : EGridLangStatus::OK); }
		int GetTotalLines () const { return m_iLines; }
		void Mark ();
		void SetCompiledOn (const CDateTime& On = CDateTime(CDateTime::Now)) { m_CompiledOn = On; }
		void SetLibraries (const TArray<SLibraryInfo>& Libraries) { m_Libraries = Libraries; }
		void SetProgramID (CStringView sID) { m_sProgramID = sID; }

	private:

		static CString AsType (ELibrarySource iSource);

		CDateTime m_CompiledOn;
		CString m_sProgramID;
		int m_iLines = 0;
		TArray<CString> m_Files;
		TArray<CString> m_Errors;

		TArray<SLibraryInfo> m_Libraries;
		TArray<SEntryPoint> m_EntryPoints;
	};

//	CGridLangAST
//
//	The AST can be loaded as follows:
//
//	1.	Call LoadSource 1 or more times to load program source files.
//	2.	Call GetLibraries to get a list of library references (the libraries used 
//			by the source code).
//	3.	Call ResolveLibrary for each library reference, with the AST for the
//			library.
//	4.	Call Resolve to resolve all references and types.

class CGridLangAST
	{
	public:

		void DebugDump () const { if (m_pRoot) m_pRoot->DebugDump(NULL_STR); }
		TArray<CString> GetLibraries () const;
		TSortMap<CString, TSharedPtr<IASTNode>> GetLibraryDefs ();
		TArray<TSharedPtr<IASTNode>> GetLibraryRefs ();
		TArray<CString> GetModules () const;
		const IASTNode &GetNode (int iIndex) const { if (m_pRoot) return m_pRoot->GetChild(iIndex); else throw CException(errFail); }
		int GetNodeCount () const { return (m_pRoot ? m_pRoot->GetChildCount() : 0); }
		IASTNode &GetRoot () { if (m_pRoot) return *m_pRoot; else throw CException(errFail); }
		const IASTNode &GetRoot () const { if (m_pRoot) return *m_pRoot; else throw CException(errFail); }
		TSharedPtr<IASTNode> GetRootReference () const { return m_pRoot; }
		CAEONTypeSystem GetTypes () const;
		bool IsEmpty () const { return !m_pRoot; }
		bool IsLibraryResolved (CStringView sLibrary) const;
		bool IsResolved () const { return m_iState == EState::Resolved; }
		void Mark ();
		bool Resolve (CGridLangResult& retResult);
		bool ResolveLibrary (const CString& sName, TSharedPtr<IASTNode> pDefinitions);
		bool ResolveLibraryReference (IASTNode& Ref, CGridLangResult::ELibrarySource iSource, CStringView sFilepath, TSharedPtr<IASTNode> pLibrary, TArray<TSharedPtr<IASTNode>>* retNewRefs = NULL, CString* retsError = NULL);
		bool ResolveLibraries (const CGridLangLibraries& Libraries, CGridLangResult& retResult);
		static bool ResolveRoot (TSharedPtr<IASTNode>& pRoot, TSortMap<CString, CDatum>& retTypes, CString* retsError = NULL);

		bool Load (const CString& sSourceFilename, const IMemoryBlock &Stream, const CGridLangLibraries &Libraries, CGridLangResult& retResult);
		bool LoadExpression (CStringView sSourceFilename, const IMemoryBlock& Stream, CGridLangResult& retResult);
		bool LoadSource (const CString& sSourceFilename, const IMemoryBlock& Stream, CGridLangResult& retResult);
		bool LoadSourceDone (CGridLangResult& retResult);

		static CDatum GetActualArrayType (CDatum dArrayType);

	private:

		enum class EState
			{
			Empty,							//	m_pRoot is NULL

			SourceLoaded,					//	m_pRoot is valid
			Resolved,						//	Resolve has been called.
			};

		enum class EBlock
			{
			Unknown,
			BeginEnd,
			Brackets,
			LiteralTable
			};

		enum class EFunctionType
			{
			Normal,							//	A normal function definition
			External,						//	A class member function defined outside the class
			ExternalEvent,					//	A class member function defined outside the class that is an event handler
			Event,							//	An event function

			Error
			};

		struct SScope
			{
			CString sClass;							//	If not blank then we're inside a class definition
			};

		static bool ComposeError (const IASTNode& Node, const CString& sError, CString* retsError);
		static TSharedPtr<IASTNode> CreateEventMethodAssignment (const CGridLangParser::SPos& Pos, TSharedPtr<IASTNode> pHandlerDef, CString* retsError = NULL);
		static EASTType GetOperator (const CGridLangParser& Parser);
		static int GetOperatorPrecedence (EASTType iOperator);
		static bool IsGreaterPrecedence (EASTType iOperator, EASTType iTest);
		static bool IsReserved (const CString& sKeyword) { DWORD* pFlags = m_KeywordFlags.GetAt(sKeyword); return ((pFlags && (*pFlags & KEYFLAG_RESERVED)) ? true : false); }
		static bool IsReservedFieldName (const CString& sKeyword) { DWORD* pFlags = m_KeywordFlags.GetAt(sKeyword); return ((pFlags && (*pFlags & KEYFLAG_RESERVED_FIELD)) ? true : false); }
		static bool IsReservedVarName (const CString& sKeyword) { DWORD* pFlags = m_KeywordFlags.GetAt(sKeyword); return ((pFlags && (*pFlags & KEYFLAG_RESERVED_VAR)) ? true : false); }

		static bool AddLibraryRef (const CString& sLibrary, TArray<TSharedPtr<IASTNode>> &retLibraryRefs, CString* retsError = NULL);
		bool AddSymbol (CGridLangParser &Parser, CStringView sScope, TSharedPtr<IASTNode> pNode, CString* retsError = NULL);
		IASTNode* FindClassBySymbol (const CString& sSymbol);
		const IASTNode &GetNode (int iIndex) { if (m_pRoot) return m_pRoot->GetChild(iIndex); else throw CException(errFail); }
		CString MakeScope (const CString &sParentScope);
		bool ParseArgDef (CGridLangParser &Parser, IASTNode *pParent, int iOrdinal, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseArgTuple (CGridLangParser &Parser, IASTNode *pParent, TArray<TSharedPtr<IASTNode>> &retArgs, CString *retsError = NULL);
		bool ParseArrayDef (CGridLangParser& Parser, const CString& sScope, const CString& sID, TSharedPtr<IASTNode>& retpNode, CString* retsError = NULL);
		bool ParseArrayDynamicDef (CGridLangParser& Parser, const CString& sScope, TSharedPtr<IASTNode>& retpNode, CString* retsError = NULL);
		bool ParseArrayElement (CGridLangParser &Parser, IASTNode *pParent, const CString& sScope, TSharedPtr<IASTNode> pArray, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseArrayLiteral (CGridLangParser &Parser, IASTNode *pParent, const CString& sScope, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseArrayOfRanges (CGridLangParser& Parser, TArray<TSharedPtr<IASTNode>>& retArgs, CString* retsError = NULL);
		bool ParseCastDef (CGridLangParser &Parser, IASTNode *pParent, const CString& sScope, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseClassDef (CGridLangParser& Parser, IASTNode* pParent, const CString& sScope, TSharedPtr<IASTNode>& retpNode, CString* retsError = NULL);
		bool ParseClassDefBody (CGridLangParser &Parser, IASTNode *pParent, const CString &sScope, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		EBlock ParseDefBlock (CGridLangParser &Parser);
		bool ParseDefinitionName (CGridLangParser& Parser, const CString& sTypeClass, EASTType iDefType, EASTType iAnonType, const CString& sID, EASTType& retiType, CString& retsID, CString* retsError = NULL);
		bool ParseDictionaryDef (CGridLangParser& Parser, const CString& sScope, const CString& sID, TSharedPtr<IASTNode>& retpNode, CString* retsError = NULL);
		bool ParseEndBlock (CGridLangParser &Parser, EBlock iBlock, CString* retsError = NULL);
		bool ParseEnum (CGridLangParser& Parser, const CString& sScope, const CString& sID, EASTType iEnumType, TSharedPtr<IASTNode>& retpNode, CString* retsError = NULL);
		bool ParseEnumDefBody (CGridLangParser& Parser, const CString& sScope, TSharedPtr<IASTNode>& retpNode, CString* retsError = NULL);
		bool ParseEnumDefBodyOld (CGridLangParser& Parser, const CString& sScope, TSharedPtr<IASTNode>& retpNode, CString* retsError = NULL);
		bool ParseEnumValueAttributes (CGridLangParser& Parser, const CString& sID, CDatum& retdAttributes, CString* retsError = NULL);
		bool ParseExpression (CGridLangParser &Parser, IASTNode *pParent, const CString& sScope, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL) { return ParseExpression(Parser, pParent, sScope, EASTType::Unknown, 0, retpNode, retsError); }

		static constexpr DWORD FLAG_ALLOW_SPREAD = 0x00000001;
		static constexpr DWORD FLAG_ALLOW_STATEMENTS = 0x00000002;
		static constexpr DWORD FLAG_MEMBER_VAR = 0x00000004;
		bool ParseExpression (CGridLangParser &Parser, IASTNode *pParent, const CString& sScope, EASTType iLeftOperator, DWORD dwFlags, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);

		bool ParseForLoop (CGridLangParser &Parser, IASTNode *pParent, const CString &sScope, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseFunctionAttributes (CGridLangParser& Parser, const CString& sID, const CString& sScope, const TArray<TSharedPtr<IASTNode>>& Args, CDatum& retdAttributes, CString* retsError = NULL);
		bool ParseFunctionAttributeBlock (CGridLangParser& Parser, const CString& sID, const CString& sScope, CDatum& retdAttributes, CString* retsError = NULL);
		bool ParseFunctionCall (CGridLangParser &Parser, IASTNode *pParent, const CString& sScope, TSharedPtr<IASTNode> pFunction, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseFunctionDef (CGridLangParser& Parser, const CString& sScope, EASTType iType, TSharedPtr<IASTNode>& retpNode, CString* retsError = NULL);
		bool ParseFunctionDefAnonymous (CGridLangParser& Parser, const CString& sScope, TSharedPtr<IASTNode>& retpNode, CString* retsError = NULL);
		bool ParseFunctionDefBody (CGridLangParser& Parser, const CString& sFunctionName, const CString& sScope, EASTType iType, TSharedPtr<IASTNode>& retpNode, CString* retsError = NULL);
		EFunctionType ParseFunctionName (CGridLangParser& Parser, CString& retFunctionName, CString& retExtra, CString* retsError = NULL);
		bool ParseIf (CGridLangParser &Parser, IASTNode *pParent, const CString &sScope, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseInstanceMemberDef (CGridLangParser &Parser, IASTNode *pParent, const CString &sScope, EASTType iType, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseInstanceMethodDef (CGridLangParser &Parser, IASTNode *pParent, const CString &sScope, EASTType iType, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseInstanceMethodDefBody (CGridLangParser &Parser, const CString& sFunctionName, const CString &sScope, EASTType iType, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseInt32Range (CGridLangParser& Parser, int* retiMin, int* retiMax, CString* retsError = NULL);
		bool ParseLibraryDef (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseNullableTypeRef (CGridLangParser &Parser, const CString& sScope, TSharedPtr<IASTNode> pLeft, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseObjectInit (CGridLangParser &Parser, IASTNode *pParent, const CString& sScope, TSharedPtr<IASTNode> pClass, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseOfTypeRef (CGridLangParser &Parser, const CString& sScope, TSharedPtr<IASTNode> pLeft, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseSchema (CGridLangParser& Parser, const CString& sScope, const CString& sID, EASTType iSchemaType, TSharedPtr<IASTNode>& retpNode, CString* retsError = NULL);
		bool ParseSchemaBody (CGridLangParser &Parser, IASTNode *pParent, const CString &sScope, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseSequence (CGridLangParser &Parser, const CString &sScope, EASTType iType, TSharedPtr<IASTNode> &retpNode, CString* retsEndToken = NULL, CString* retsError = NULL);
		bool ParseStructLiteral (CGridLangParser &Parser, IASTNode *pParent, const CString& sScope, EASTType iType, EGridLangToken iEndToken, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseStructLiteral (CGridLangParser &Parser, IASTNode *pParent, const CString& sScope, EASTType iType, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL)
			{ return ParseStructLiteral(Parser, pParent, sScope, iType, EGridLangToken::CloseBrace, retpNode, retsError); }

		bool ParseSubRangeDef (CGridLangParser &Parser, CStringView sScope, CStringView sID, CDatum dBaseType, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseTableDef (CGridLangParser& Parser, const CString& sScope, const CString& sID, TSharedPtr<IASTNode>& retpNode, CString* retsError = NULL);
		bool ParseTerm (CGridLangParser &Parser, IASTNode *pParent, const CString& sScope, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL) { return ParseTerm(Parser, pParent, sScope, 0, retpNode, retsError); }
		bool ParseTerm (CGridLangParser &Parser, IASTNode *pParent, const CString& sScope, DWORD dwFlags, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseTypeDef (CGridLangParser &Parser, IASTNode *pParent, const CString &sScope, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseTypeParams (CGridLangParser& Parser, const CString& sType, TArray<TSharedPtr<IASTNode>>& retArgs, CString* retsError = NULL);
		bool ParseTypeRef (CGridLangParser &Parser, IASTNode *pParent, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseVarDef (CGridLangParser &Parser, IASTNode *pParent, const CString &sScope, EASTType iVarType, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);
		bool ParseWhileLoop (CGridLangParser &Parser, IASTNode *pParent, const CString &sScope, TSharedPtr<IASTNode> &retpNode, CString *retsError = NULL);

		EState m_iState = EState::Empty;
		TSharedPtr<IASTNode> m_pRoot;
		CASTSymbolTable m_Symbols;
		TSortMap<CString, CDatum> m_Types;
		TSortMap<CString, CGridLangResult::SLibraryInfo> m_Libraries;

		int m_iNextScope = 1;

		static constexpr DWORD KEYFLAG_RESERVED =			0x00000001;			//	Cannot be used as a function or type definition
		static constexpr DWORD KEYFLAG_CONTEXT =			0x00000002;			//	Used in specific context (but not reserved)
		static constexpr DWORD KEYFLAG_FOR_VAR =			0x00000004;			//	Used as auto variables inside a for loop
		static constexpr DWORD KEYFLAG_DEPRECATED =			0x00000008;			//	Only used in older code.
		static constexpr DWORD KEYFLAG_CLASS_KEYWORD =		0x00000010;			//	Only used inside a class/schema definitions
		static constexpr DWORD KEYFLAG_RESERVED_FIELD =		0x00000020;			//	Cannot be used as a field name
		static constexpr DWORD KEYFLAG_RESERVED_VAR =		0x00000040;			//	Cannot be used as a variable definition

		static TSortMap<CString, DWORD> m_KeywordFlags;
	};

//	Public AST Classes ---------------------------------------------------------

class CASTLibraryFunctionDef : public IASTNode
	{
	public:
		static constexpr int DEF_INDEX_USAGE = 0;
		static constexpr int DEF_INDEX_TEXT = 1;

		static TSharedPtr<IASTNode> Create (const SLibraryFuncDef &Def, CDatum dFunctionType);

		CASTLibraryFunctionDef (const CString &sFunctionName, CDatum dFunctionType) : IASTNode(CGridLangParser::SPos()),
				m_iType(EASTType::LibraryFunctionDef),
				m_sFunction(sFunctionName),
				m_dFunctionType(dFunctionType)
			{ }

		virtual bool CanBeCalledWith (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, CDatum& retdReturnType, CString* retsError = NULL) const override;
		virtual TSharedPtr<IASTNode> Clone () const override { return TSharedPtr<IASTNode>(new CASTLibraryFunctionDef(*this)); }
		virtual void DebugDump (const CString &sIndent) const override;
		virtual int GetDefinitionCount () const override { return DEF_INDEX_COUNT; }
		virtual const CString &GetDefinitionString (int iIndex) const override;
		virtual const CString &GetName () const override { return m_sFunction; }
		virtual CDatum GetStaticType () const override;
		virtual EASTType GetType () const override { return m_iType; }
		virtual CDatum GetTypeDef () const override;
		virtual bool IsFunctionDefinition () const override { return true; }
		virtual bool IsImmutable () const override { return true; }
		virtual void Mark () override { m_dFunctionType.Mark(); }

	private:
		static constexpr int DEF_INDEX_COUNT = 2;

		EASTType m_iType = EASTType::Unknown;
		CString m_sFunction;
		CString m_sUsage;
		CString m_sText;
		DWORD m_dwExecFlags = 0;

		CDatum m_dFunctionType;
	};

class CASTLibraryConstDef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (const CString &sName, CDatum dValue, CDatum dType)
			{
			CASTLibraryConstDef *pNode = new CASTLibraryConstDef(sName, dValue, dType);
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTLibraryConstDef (const CString &sName, CDatum dValue, CDatum dType) : IASTNode(CGridLangParser::SPos()),
				m_sName(sName),
				m_dValue(dValue),
				m_dType(dType)
			{ }

		virtual TSharedPtr<IASTNode> Clone () const override { return TSharedPtr<IASTNode>(new CASTLibraryConstDef(*this)); }
		virtual void DebugDump (const CString &sIndent) const override;
		virtual const CString &GetName () const override { return m_sName; }
		virtual CDatum GetStaticType () const override { return m_dType; }
		virtual EASTType GetType () const override { return EASTType::LibraryConstDef; }
		virtual CDatum GetValue () const override { return m_dValue; }
		virtual bool IsConstExpression () const override { return true; }
		virtual bool IsImmutable () const override { return true; }
		virtual void Mark () override { m_dValue.Mark(); m_dType.Mark(); }

	private:
		CString m_sName;
		CDatum m_dValue;
		CDatum m_dType;
	};

class CASTLibraryTypeDef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (CDatum dDatatype)
			{
			CASTLibraryTypeDef *pNode = new CASTLibraryTypeDef(dDatatype);
			return TSharedPtr<IASTNode>(pNode);
			}

		CASTLibraryTypeDef (CDatum dDatatype) : IASTNode(CGridLangParser::SPos()),
				m_dDatatype(dDatatype)
			{
			const IDatatype& Datatype = dDatatype;
			m_sName = Datatype.GetName();
			m_sFullyQualifiedName = Datatype.GetFullyQualifiedName();
			}

		virtual void AccumulateTypes (TSortMap<CString, CDatum>& retTypes) const override;
		virtual TSharedPtr<IASTNode> Clone () const override { return TSharedPtr<IASTNode>(new CASTLibraryTypeDef(*this)); }
		virtual void DebugDump (const CString &sIndent) const override;
		virtual const CString &GetFullyQualifiedName () const override { return m_sFullyQualifiedName; }
		virtual const CString &GetName () const override { return m_sName; }
		virtual CDatum GetStaticType () const override { return m_dDatatype.GetDatatype(); }
		virtual EASTType GetType () const override { return EASTType::LibraryTypeDef; }
		virtual CDatum GetTypeDef () const override { return m_dDatatype; }
		virtual CDatum GetValue () const override { return m_dDatatype; }
		virtual bool IsConstExpression () const override { return true; }
		virtual bool IsImmutable () const override { return true; }
		virtual bool IsTypeDefinition () const override { return true; }
		virtual void Mark () override { m_dDatatype.Mark(); }

	private:
		CString m_sName;
		CString m_sFullyQualifiedName;
		CDatum m_dDatatype;
	};

class CASTLibraryDef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (const CString& sLibraryName, TSharedPtr<IASTNode> pBody);
		static TSharedPtr<IASTNode> Create (const CString& sLibraryName, const TArray<CString>& Namespaces, TArray<TSharedPtr<IASTNode>> &&Nodes);
		static TSharedPtr<IASTNode> Create (const CString& sLibraryName, 
				const CString& sModuleName, 
				const SLibraryFuncDef* pFuncDefs, 
				int iFuncDefs, 
				const std::initializer_list<CHexeLibrarian::SConstDoubleDef>& ConstDefs = {},
				const std::initializer_list<DWORD>& TypeDefs = {});

		CASTLibraryDef () : IASTNode(CGridLangParser::SPos())
			{ }

		void AddNode (TSharedPtr<IASTNode> pChild) { m_Node.Insert(pChild); }
		virtual TSharedPtr<IASTNode> Clone () const override { return TSharedPtr<IASTNode>(new CASTLibraryDef(*this)); }
		virtual void AccumulateStaticNamespaces (TArray<CString>& retNamespaces) const override;
		virtual void DebugDump (const CString &sIndent) const override;
		virtual IASTNode* FindChildSymbol (const CString &sName) override;
		virtual const CString &GetName () const override { return m_sName; }
		virtual IASTNode &GetChild (int iIndex) override { if (iIndex >= 0 && iIndex < GetChildCount()) return *m_Node[iIndex]; else throw CException(errFail); }
		virtual int GetChildCount () const override { return m_Node.GetCount(); }
		virtual TSharedPtr<IASTNode> GetChildReference (int iIndex) const { if (iIndex >= 0 && iIndex < GetChildCount()) return m_Node[iIndex]; else throw CException(errFail); }
		virtual EASTType GetType () const override { return EASTType::LibraryDef; }
		virtual bool HasStaticNamespace (CStringView sSymbol) const override;
		virtual bool IsImmutable () const override { return true; }
		virtual void Mark () override;

	private:
		CString m_sName;
		TArray<TSharedPtr<IASTNode>> m_Node;

		TArray<CString> m_Namespaces;
	};

class CASTModuleRef : public IASTNode
	{
	public:
		static TSharedPtr<IASTNode> Create (const CString &sModuleName, CString *retsError);

		CASTModuleRef () : IASTNode(CGridLangParser::SPos())
			{ }

		virtual TSharedPtr<IASTNode> Clone () const override { return TSharedPtr<IASTNode>(new CASTModuleRef(*this)); }
		virtual void DebugDump (const CString &sIndent) const override;
		virtual const CString &GetName () const override { return m_sName; }
		virtual EASTType GetType () const override { return EASTType::ModuleRef; }
		virtual bool IsImmutable () const override { return true; }

	private:
		CString m_sName;
	};

