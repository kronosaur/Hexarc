//	Hexe.h
//
//	Hexe EXecution Environment
//	Copyright (c) 2011 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	1. Requires Foundation
//	2. Requires AEON
//	3. Include Hexe.h
//	4. Link with Hexe.lib
//	5. Call CHexe::Boot at the beginning of the process

#pragma once

#include "AEON.h"
#include "HexeVM.h"

class CHexeCode;
class CHexeGlobalEnvironment;
class CHexeLocalEnvironment;
class CHexeProcess;

//	CHexeDocument --------------------------------------------------------------

class CHexeDocument
	{
	public:
		struct SEntryPoint
			{
			CString sFunction;
			CDatum dDesc;
			CDatum dCode;
			};

		~CHexeDocument (void);

		static void CreateFunctionCall (const CString &sFunction, const TArray<CDatum> &Args, CDatum *retdEntryPoint);
		static bool IsValidIdentifier (const CString &sIdentifier);
		static bool ParseData (IByteStream &Stream, CDatum *retdData);
		static bool ParseLispExpression (const CString &sExpression, CDatum *retdExpression, CString *retsError);

		CDatum AsDatum (void) const;
		bool FindEntry (const CString &sName, CString *retsType = NULL, CDatum *retdData = NULL) const;
		int GetCount (void) const { return m_Doc.GetCount(); }
		CDatum GetData (int iIndex) const { return m_Doc[iIndex].dData; }
		void GetEntryPoints (TArray<SEntryPoint> *retList) const;
		void GetHexeDefinitions (TArray<CDatum> *retList) const;
		const CString &GetName (int iIndex) const { return m_Doc[iIndex].sName; }
		const CString &GetType (int iIndex) const { return m_Doc[iIndex].sType; }
		int GetTypeIndexCount (int iTypeIndex) const { return (iTypeIndex == -1 ? 0 : m_TypeIndex[iTypeIndex].GetCount()); }
		CDatum GetTypeIndexData (int iTypeIndex, int iIndex) const { return (iTypeIndex == -1 ? CDatum() : m_Doc[m_TypeIndex[iTypeIndex].GetAt(iIndex)].dData); }
		const CString &GetTypeIndexName (int iTypeIndex, int iIndex) const { return (iTypeIndex == -1 ? NULL_STR : m_Doc[m_TypeIndex[iTypeIndex].GetAt(iIndex)].sName); }
		int GetTypeIndex (const CString &sType) const;
		bool InitFromData (CDatum dData, CHexeProcess &Process, CString *retsError);
		bool InitFromStream (IByteStream &Stream, CHexeProcess &Process, CString *retsError);
		void Mark (void);
		void Merge (CHexeDocument *pDoc);

	private:
		struct SEntry
			{
			CString sName;
			CString sType;
			CDatum dData;
			};

		bool AddEntry (const CString &sName, const CString &sType, CDatum dData);
		void InitTypeIndex (void) const;
		void InvalidateTypeIndex (void) { m_TypeIndex.DeleteAll(); }
		bool ParseAEONDef (CCharStream *pStream, CHexeProcess &Process, CString *retsName, CString *retsType, CDatum *retdDatum, CString *retsError);
		bool ParseComments (CCharStream *pStream, CString *retsError);
		bool ParseDefine (CAEONScriptParser &Parser, CHexeProcess &Process, const CString &sType, CString *retsName, CDatum *retdDatum, CString *retsError);

		static bool IsAnonymous (const CString &sName, const CString &sType);
		static bool ParseHexeLispDef (CCharStream *pStream, CString *retsName, CString *retsType, CDatum *retdDatum, CString *retsError);

		TSortMap<CString, SEntry> m_Doc;
		mutable TSortMap<CString, TArray<int>> m_TypeIndex;
	};

//	CHexeLibrarian -------------------------------------------------------------

typedef bool (* FHexeLibraryFunc)(IInvokeCtx *pCtx, DWORD dwData, CDatum dLocalEnv, CDatum dContinueContext, CDatum *retdResult);

struct SLibraryFuncDef
	{
	const CString &sName;
	FHexeLibraryFunc pfFunc;
	DWORD dwData;
	const CString &sArgList;
	const CString &sHelpLine;
	DWORD dwExecutionRights;
	};

#define DECLARE_DEF_LIBRARY_FUNC(name,func,rights)	{ name##_NAME, func, name, name##_ARGS, name##_HELP, (rights) }

class CHexeLibrarian
	{
	public:
		CDatum FindFunction (const CString &sLibrary, const CString &sFunction) const;
		bool FindLibrary (const CString &sName, DWORD *retdwLibraryID) const;
		const CString &GetEntry (DWORD dwLibrary, int iIndex, CDatum *retdFunction);
		int GetEntryCount (DWORD dwLibrary);
		int GetLibraryCount (void) { return m_Catalog.GetCount(); }
		void Mark (void);
		void RegisterCoreLibraries (void);
		void RegisterLibrary (const CString &sName, int iCount, SLibraryFuncDef *pNewLibrary);

	private:
		struct SLibrary
			{
			CString sName;
			TSortMap<CString, CDatum> Functions;
			};

		const SLibrary *GetLibrary (const CString &sName) const;
		SLibrary *GetLibrary (const CString &sName);

		TSortMap<CString, SLibrary> m_Catalog;
	};

extern CHexeLibrarian g_HexeLibrarian;

//	CHexeError -----------------------------------------------------------------

class CHexeError : public TExternalDatum<CHexeError>
	{
	public:
		static void Create (const CString &sErrorCode, const CString &sErrorDesc, CDatum *retdDatum);
		static const CString &StaticGetTypename (void);

		//	IComplexDatum
		virtual CString AsString (void) const override { return m_sDescription; }
		virtual const CString &CastCString (void) const override { return m_sDescription; }
		virtual bool IsError (void) const override { return true; }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	protected:
		//	IComplexDatum
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:
		CString m_sError;
		CString m_sDescription;
	};

//	CHexeProcess ---------------------------------------------------------------
//
//	This object represents a running instantiation of a program. It contains
//	global variables, a stack, and an instruction pointer.
//
//	A process is created from a CHexeDocument object. If necessary, the process
//	will compile definitions in the HexeDocument (but it will also accept 
//	compiled definitions).
//
//	There is a method for compiling a HexeDoc into another HexeDoc.

#include "HexeProcessImpl.h"

class CHexeSecurityCtx
	{
	public:
		static constexpr DWORD EXEC_RIGHT_SIDE_EFFECTS =		0x00000001;
		static constexpr DWORD EXEC_RIGHT_INVOKE =				0x00000002;

		static constexpr DWORD EXEC_RIGHTS_ALL =				0xffffffff;
		static constexpr DWORD EXEC_RIGHTS_MINIMAL =			0x00000000;

		CDatum AsDatum (void) const;
		DWORD GetExecutionRights (void) const { return m_dwExecutionRights; }
		const CString &GetSandbox (void) const { return m_sSandbox; }
		CString GetSandboxName (void) const { return strSubString(m_sSandbox, 0, m_sSandbox.GetLength() - 1); }
		const CString &GetUsername (void) const { return m_sUsername; }
		void GetServiceSecurity (CHexeSecurityCtx *retCtx) { retCtx->SetServiceSecurity(*this); }
		const CAttributeList &GetUserRights (void) const { return m_UserRights; }
		void GetUserSecurity (CHexeSecurityCtx *retCtx) { retCtx->SetUserSecurity(*this); }
		bool HasServiceRightArcAdmin (void) const;
		bool HasNoServiceRights (void) const { return m_ServiceRights.IsEmpty(); }
		bool HasServiceRight (const CString &sRight) const { return m_ServiceRights.HasAttribute(sRight); }
		bool HasUserRight (const CString &sRight) const { return m_UserRights.HasAttribute(sRight); }
		bool HasUserRights (const CAttributeList &Rights) const;
		void Init (CDatum dDatum);
		void InsertServiceRight (const CString &sRight) { m_ServiceRights.Insert(sRight); }
		void InsertUserRight (const CString &sRight) { m_UserRights.Insert(sRight); }
		bool IsEmpty (void) const { return m_sSandbox.IsEmpty() && m_sUsername.IsEmpty(); }
		bool IsNamespaceAccessible (const CString &sName) const { return m_sSandbox.IsEmpty() || strStartsWith(sName, m_sSandbox); }
		bool IsAnonymous (void) const { return m_sUsername.IsEmpty(); }
		void SetAnonymous (void) { m_sUsername = NULL_STR; m_UserRights.DeleteAll(); }
		void SetExecutionRights (DWORD dwRights) { m_dwExecutionRights = dwRights; }
		void SetSandbox (const CString &sSandbox) { m_sSandbox = sSandbox; }
		void SetServiceRights (CDatum dDatum) { dDatum.AsAttributeList(&m_ServiceRights); }
		void SetServiceRightsArcAdmin ();
		void SetServiceSecurity (const CHexeSecurityCtx &SecurityCtx);
		void SetUsername (const CString &sUsername) { m_sUsername = sUsername; }
		void SetUserRights (CDatum dDatum) { dDatum.AsAttributeList(&m_UserRights); }
		void SetUserRights (const CAttributeList &Rights) { m_UserRights = Rights; }
		void SetUserSecurity (const CHexeSecurityCtx &SecurityCtx);

	private:
		CAttributeList m_ServiceRights;		//	Rights granted to the service
		CString m_sSandbox;					//	Sandbox prefix (e.g., "Trans.") Only set if not admin.

		CString m_sUsername;				//	Authenticated user (may be NULL, in which case on behalf of service).
		CAttributeList m_UserRights;		//	Rights granted to user
		DWORD m_dwExecutionRights = EXEC_RIGHTS_ALL;
	};

class IHexeComputeProgress
	{
	public:
		virtual ~IHexeComputeProgress () { }
		virtual bool OnAbortCheck () { return false; }
		virtual void OnCompute (DWORDLONG dwComputes, DWORDLONG dwLibraryTime) { }
		virtual void OnStart () { }
		virtual void OnStop () { }
	};

class CHexeProcess : public IInvokeCtx
	{
	public:
		static constexpr DWORD FLAG_MAP_EXCLUDE_NIL =		0x00000001;
		static constexpr DWORD FLAG_MAP_ORIGINAL =			0x00000002;

		enum class ERun
			{
			OK,						//	Run completed successfully
									//	Result is the function result.

			AsyncRequest,			//	Run needs an async result.
									//	Result is a structure describing
									//	the required async operation
									//	(call RunContinues when ready).
									//
									//	NOTE: This is actually a misnomer. The
									//	code blocks while waiting for the 
									//	result, so technically this should be a
									//	"blocking request". Should probably 
									//	rename to HexarcRequest.

			InputRequest,			//	Run needs input (usually from the user).
									//	Result is a struct describing the 
									//	request. Call RunContinues when the data
									//	is available.

			EventHandlerDone,		//	Run has finished running an event 
									//	handler.

			StopCheck,				//	Check to see if user wants to
									//	abort execution.

			Error,					//	Runtime error.
									//	Result is the error.
			};

		CHexeProcess (void);

		static bool Boot (void);
		static void AddHexarcMsgPattern (const CString &sPrefix, const CString &sAddr);
		static void ComposeHexarcMessage (const CHexeSecurityCtx &SecurityCtx, const CString &sMsg, CDatum dPayload, CString *retsMsg, CDatum *retdPayload);
		static void CreateFunctionCall (int iArgCount, CDatum *retdExpression);
		static void CreateInvokeCall (const TArray<CDatum> &Args, CDatum *retdExpression);
		static bool IsHexarcMessage (const CString &sMsg, CString *retsAddress = NULL) { return FindHexarcMessage(sMsg, retsAddress); }

		void DefineGlobal (const CString &sIdentifier, CDatum dValue);
		void DeleteAll (void);
		void DeleteLibraryCtx (const CString &sLibrary) { m_LibraryCtx.DeleteAt(sLibrary); }
		bool FindGlobalDef (const CString &sIdentifier, CDatum *retdValue = NULL);
		CDatum FindType (const CString &sFullyQualifiedName) const { if (m_pTypes) return m_pTypes->FindType(sFullyQualifiedName); else return CAEONTypeSystem::GetCoreType(IDatatype::ANY); }
		DWORDLONG GetComputes () const { return m_dwComputes; }
		CDatum GetGlobalEnv (void) { return m_dGlobalEnv; }
		DWORDLONG GetLibraryTime () const { return m_dwLibraryTime; }
		CHexeStack &GetStack () { return m_Stack; }
		bool InitFrom (const CHexeProcess &Process, CString *retsError = NULL);
		bool LoadEntryPoints (const TArray<CHexeDocument::SEntryPoint> &EntryPoints, CString *retsError);
		bool LoadEntryPoints (const CHexeDocument &Program, CString *retsError);
		bool LoadHexeDefinitions (const TArray<CDatum> &Definitions, CString *retsError);
		bool LoadHexeDefinitions (const CHexeDocument &Program, CString *retsError);
		bool LoadLibrary (const CString &sName, CString *retsError = NULL);
		bool LoadStandardLibraries (CString *retsError);
		void Mark (void);
		ERun Run (const CString &sExpression, CDatum *retdResult);
		ERun Run (CDatum dFunc, const TArray<CDatum> &Args, CDatum *retResult);
		ERun Run (CDatum dExpression, CDatum *retResult);
		ERun Run (CDatum dFunc, CDatum dCallExpression, const TArray<CDatum> *pInitialStack, CDatum *retResult);
		ERun RunContinues (CDatum dAsyncResult, CDatum *retResult);
		ERun RunEventHandler (CDatum dFunc, const TArray<CDatum> &Args, CDatum &retResult);
		void SetComputeProgress (IHexeComputeProgress &Progress) { m_pComputeProgress = &Progress; }
		void *SetLibraryCtx (const CString &sLibrary, void *pCtx);
		void SetMaxExecutionTime (DWORDLONG dwMilliseconds) { m_dwMaxExecutionTime = dwMilliseconds; }
		void SetOptionAddConcatenatesStrings (bool bValue = true) { m_bAddConcatenatesStrings = bValue; }
		void SetSecurityCtx (const CHexeSecurityCtx &Ctx);
		void SetTypeSystem (const CAEONTypeSystem &Types) { m_pTypes = &Types; }

		//	Execution helpers
		static int ExecuteCompare (CDatum dValue1, CDatum dValue2);
		static bool ExecuteIsEquivalent (CDatum dValue1, CDatum dValue2);

		//	IInvokeCtx virtuals
		virtual void *GetLibraryCtx (const CString &sLibrary) override { void **ppCtx = m_LibraryCtx.GetAt(sLibrary); return (ppCtx ? *ppCtx : NULL); }
		virtual void SetUserSecurity (const CString &sUsername, const CAttributeList &Rights) override;

	private:
		struct SHexarcMsgInfo
			{
			const CString &sMsg;
			const CString &sAddr;
			DWORD dwOptions;
			};

		struct SHexarcMsgPattern
			{
			CString sPrefix;
			CString sAddr;
			};

		ERun Execute (CDatum *retResult);
		static bool FindHexarcMessage (const CString &sMsg, CString *retsAddr = NULL);
		void GetCurrentSecurityCtx (CHexeSecurityCtx *retCtx);
		CDatum GetCurrentSecurityCtx (void);
		void InitGlobalEnv (CHexeGlobalEnvironment **retpGlobalEnv = NULL);
		static bool ParseHyperionServiceMessage (const CString &sMsg, CDatum dPayload, CString &sAddr, CString &sNewMsg, CDatum &dNewPayload);
		ERun RunWithStack (CDatum dExpression, CDatum *retResult);
		bool SendHexarcMessage (const CString &sMsg, CDatum dPayload, CDatum *retdResult);
		bool SendHexarcMessage (const CHexeSecurityCtx &SecurityCtx, const CString &sMsg, CDatum dPayload, CDatum *retdResult);
		static bool SendHexarcMessage (const CHexeSecurityCtx &SecurityCtx, const CString &sAddr, const CString &sMsg, CDatum dPayload, CDatum *retdResult);
		bool SendHexarcMessageSafe (const CString &sMsg, CDatum dPayload, CDatum *retdResult) { CHexeSecurityCtx Ctx; GetCurrentSecurityCtx(&Ctx); return SendHexarcMessage(Ctx, sMsg, dPayload, retdResult); }
		static bool ValidateHexarcMessage (const CString &sMsg, CDatum dPayload, CString *retsAddr, CDatum *retdResult);

		//	Execution helpers
		void ExecuteArrayMemberItem (CDatum dArray, const CString &sField);
		ERun ExecuteHandleInvokeResult (CDatum::InvokeResult iInvokeResult, CDatum dExpression, CDatum dInvokeResult, CDatum *retResult);
		static bool ExecuteMakeFlagsFromArray (CDatum dOptions, CDatum dMap, CDatum *retdResult);
		static bool ExecuteSetAt (CDatum dOriginal, CDatum dKey, CDatum dValue, CDatum *retdResult);

		//	Options

		IHexeComputeProgress *m_pProgress = NULL;	//	Callback to report progress
		DWORDLONG m_dwMaxExecutionTime = 0;			//	Do not allow execution to exceed this amount of time (in ms)
		DWORDLONG m_dwAbortTime = 0;				//	Abort at this tick (0 = never abort)
		bool m_bAddConcatenatesStrings = false;		//	If TRUE, then + will concatenate string (instead of converting 
													//		them to numbers).

		//	Execution State

		const CAEONTypeSystem *m_pTypes = NULL;		//	Static types
		CDatum m_dExpression;						//	Current expression being executed
		CHexeStack m_Stack;							//	Stack
		DWORD *m_pIP = NULL;						//	Current instruction pointer
		CDatum m_dCodeBank;							//	Current code bank
		CHexeCode *m_pCodeBank = NULL;				//	Current code bank
		CHexeCallStack m_CallStack;					//	Call stack
		bool m_bInEventHandler = false;				//	Inside event handler

		CDatum m_dGlobalEnv;						//	Process global environment

		CDatum m_dCurGlobalEnv;						//	Current global environment
		CHexeGlobalEnvironment *m_pCurGlobalEnv = NULL;	//	Current global environment

		CDatum m_dLocalEnv;							//	Local environment
		CHexeLocalEnvironment *m_pLocalEnv = NULL;	//	Local environment
		CHexeEnvStack m_LocalEnvStack;

		TSortMap<CString, void *> m_LibraryCtx;		//	Ctx for each library
		CHexeSecurityCtx m_UserSecurity;			//	User security context

		IHexeComputeProgress *m_pComputeProgress = NULL;
		DWORDLONG m_dwComputes = 0;					//	Total instructions processed so far.
		DWORDLONG m_dwLibraryTime = 0;				//	Total milliseconds spent executing
													//		native library functions.

		static SHexarcMsgInfo m_HexarcMsgInfo[];
		static int m_iHexarcMsgInfoCount;
		static TSortMap<CString, int> m_HexarcMsgIndex;
		static TArray<SHexarcMsgPattern> m_HexarcMsgPatterns;
	};

//	CHexeText ------------------------------------------------------------------

class CHexeTextMarkup
	{
	public:
		static bool ConvertToHTML (const IMemoryBlock &Input, const CString &sFormat, CDatum dParams, IByteStream &Output, CString *retsError);
		static bool EscapeHTML (const IMemoryBlock &Template, CDatum dStruct, IByteStream &Output, CString *retsError = NULL);
		static CString FormatString (CDatum dArgList);
		static void WriteHTMLContent (CDatum dValue, IByteStream &Output);
	};

//	CHexe ----------------------------------------------------------------------

class CHexe
	{
	public:
		static bool Boot (void);

		static bool InvokeHexarcMsg (const CString &sMsg, CDatum dPayload, CDatum &retdResult);

	private:
		static void Mark (void);

		static bool m_bInitialized;
	};

//	Utility Functions ----------------------------------------------------------

bool HexeGetPolygon2DArg (CDatum dArg, const CPolygon2D **retpPolygon, CPolygon2D *retTempStore, CDatum *retdResult);
bool HexeGetVector2DArg (CDatum dArgList, int *ioArg, CVector2D *retVector, CDatum *retdResult);

#include "HexeConsole.h"
