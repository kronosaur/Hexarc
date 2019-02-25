//	Hexe.h
//
//	Hexe EXecution Environment
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
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
		inline int GetCount (void) const { return m_Doc.GetCount(); }
		inline CDatum GetData (int iIndex) const { return m_Doc[iIndex].dData; }
		void GetEntryPoints (TArray<SEntryPoint> *retList) const;
		void GetHexeDefinitions (TArray<CDatum> *retList) const;
		inline const CString &GetName (int iIndex) const { return m_Doc[iIndex].sName; }
		inline const CString &GetType (int iIndex) const { return m_Doc[iIndex].sType; }
		inline int GetTypeIndexCount (int iTypeIndex) const { return (iTypeIndex == -1 ? 0 : m_TypeIndex[iTypeIndex].GetCount()); }
		inline CDatum GetTypeIndexData (int iTypeIndex, int iIndex) const { return (iTypeIndex == -1 ? CDatum() : m_Doc[m_TypeIndex[iTypeIndex].GetAt(iIndex)].dData); }
		inline const CString &GetTypeIndexName (int iTypeIndex, int iIndex) const { return (iTypeIndex == -1 ? NULL_STR : m_Doc[m_TypeIndex[iTypeIndex].GetAt(iIndex)].sName); }
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

		void AddEntry (const CString &sName, const CString &sType, CDatum dData);
		void InitTypeIndex (void) const;
		inline void InvalidateTypeIndex (void) { m_TypeIndex.DeleteAll(); }
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
		bool FindLibrary (const CString &sName, DWORD *retdwLibraryID);
		const CString &GetEntry (DWORD dwLibrary, int iIndex, CDatum *retdFunction);
		int GetEntryCount (DWORD dwLibrary);
		inline int GetLibraryCount (void) { return m_Catalog.GetCount(); }
		void Mark (void);
		void RegisterCoreLibraries (void);
		void RegisterLibrary (const CString &sName, int iCount, SLibraryFuncDef *pNewLibrary);

	private:
		struct SLibrary
			{
			CString sName;
			TSortMap<CString, CDatum> Functions;
			};

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
		virtual void Serialize (CDatum::ESerializationFormats iFormat, IByteStream &Stream) const override;

	protected:
		//	IComplexDatum
		virtual bool OnDeserialize (CDatum::ESerializationFormats iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnSerialize (CDatum::ESerializationFormats iFormat, IByteStream &Stream) const override;

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
		inline DWORD GetExecutionRights (void) const { return m_dwExecutionRights; }
		inline const CString &GetSandbox (void) const { return m_sSandbox; }
		inline CString GetSandboxName (void) const { return strSubString(m_sSandbox, 0, m_sSandbox.GetLength() - 1); }
		inline const CString &GetUsername (void) const { return m_sUsername; }
		inline void GetServiceSecurity (CHexeSecurityCtx *retCtx) { retCtx->SetServiceSecurity(*this); }
		inline const CAttributeList &GetUserRights (void) const { return m_UserRights; }
		inline void GetUserSecurity (CHexeSecurityCtx *retCtx) { retCtx->SetUserSecurity(*this); }
		bool HasServiceRightArcAdmin (void) const;
		inline bool HasNoServiceRights (void) const { return m_ServiceRights.IsEmpty(); }
		inline bool HasServiceRight (const CString &sRight) const { return m_ServiceRights.HasAttribute(sRight); }
		inline bool HasUserRight (const CString &sRight) const { return m_UserRights.HasAttribute(sRight); }
		bool HasUserRights (const CAttributeList &Rights) const;
		void Init (CDatum dDatum);
		inline void InsertServiceRight (const CString &sRight) { m_ServiceRights.Insert(sRight); }
		inline void InsertUserRight (const CString &sRight) { m_UserRights.Insert(sRight); }
		inline bool IsEmpty (void) const { return m_sSandbox.IsEmpty() && m_sUsername.IsEmpty(); }
		inline bool IsNamespaceAccessible (const CString &sName) const { return m_sSandbox.IsEmpty() || strStartsWith(sName, m_sSandbox); }
		inline bool IsAnonymous (void) const { return m_sUsername.IsEmpty(); }
		inline void SetAnonymous (void) { m_sUsername = NULL_STR; m_UserRights.DeleteAll(); }
		inline void SetExecutionRights (DWORD dwRights) { m_dwExecutionRights = dwRights; }
		inline void SetSandbox (const CString &sSandbox) { m_sSandbox = sSandbox; }
		inline void SetServiceRights (CDatum dDatum) { dDatum.AsAttributeList(&m_ServiceRights); }
		void SetServiceSecurity (const CHexeSecurityCtx &SecurityCtx);
		inline void SetUsername (const CString &sUsername) { m_sUsername = sUsername; }
		inline void SetUserRights (CDatum dDatum) { dDatum.AsAttributeList(&m_UserRights); }
		inline void SetUserRights (const CAttributeList &Rights) { m_UserRights = Rights; }
		void SetUserSecurity (const CHexeSecurityCtx &SecurityCtx);

	private:
		CAttributeList m_ServiceRights;		//	Rights granted to the service
		CString m_sSandbox;					//	Sandbox prefix (e.g., "Trans.") Only set if not admin.

		CString m_sUsername;				//	Authenticated user (may be NULL, in which case on behalf of service).
		CAttributeList m_UserRights;		//	Rights granted to user
		DWORD m_dwExecutionRights = EXEC_RIGHTS_ALL;
	};

class CHexeProcess : public IInvokeCtx
	{
	public:
		enum EFlags
			{
			FLAG_MAP_EXCLUDE_NIL =			0x00000001,
			FLAG_MAP_ORIGINAL =				0x00000002,
			};

		enum ERunCodes
			{
			runOK,					//	Run completed successfully
									//	Result is the function result.

			runAsyncRequest,		//	Run needs an async result.
									//	Result is a structure describing
									//	the required async operation
									//	(call RunContinues when ready).

			runStopCheck,			//	Check to see if user wants to
									//	abort execution.

			runError,				//	Runtime error.
									//	Result is the error.
			};

		CHexeProcess (void);

		static bool Boot (void);
		static void AddHexarcMsgPattern (const CString &sPrefix, const CString &sAddr);
		static void ComposeHexarcMessage (const CHexeSecurityCtx &SecurityCtx, const CString &sMsg, CDatum dPayload, CString *retsMsg, CDatum *retdPayload);
		static void CreateFunctionCall (int iArgCount, CDatum *retdExpression);
		static void CreateInvokeCall (const TArray<CDatum> &Args, CDatum *retdExpression);
		static bool IsHexarcMessage (const CString &sMsg, CString *retsAddress = NULL) { return FindHexarcMessage(sMsg, retsAddress); }
		static bool SendHexarcMessage (const CHexeSecurityCtx &SecurityCtx, const CString &sAddr, const CString &sMsg, CDatum dPayload, CDatum *retdResult);

		void DefineGlobal (const CString &sIdentifier, CDatum dValue);
		void DeleteAll (void);
		bool FindGlobalDef (const CString &sIdentifier, CDatum *retdValue = NULL);
		inline CDatum GetGlobalEnv (void) { return m_dGlobalEnv; }
		bool InitFrom (const CHexeProcess &Process, CString *retsError = NULL);
		bool LoadEntryPoints (const TArray<CHexeDocument::SEntryPoint> &EntryPoints, CString *retsError);
		bool LoadEntryPoints (const CHexeDocument &Program, CString *retsError);
		bool LoadHexeDefinitions (const TArray<CDatum> &Definitions, CString *retsError);
		bool LoadHexeDefinitions (const CHexeDocument &Program, CString *retsError);
		bool LoadLibrary (const CString &sName, CString *retsError = NULL);
		bool LoadStandardLibraries (CString *retsError);
		void Mark (void);
		ERunCodes Run (const CString &sExpression, CDatum *retdResult);
		ERunCodes Run (CDatum dFunc, const TArray<CDatum> &Args, CDatum *retResult);
		ERunCodes Run (CDatum dExpression, CDatum *retResult);
		ERunCodes Run (CDatum dFunc, CDatum dCallExpression, const TArray<CDatum> *pInitialStack, CDatum *retResult);
		ERunCodes RunContinues (CDatum dAsyncResult, CDatum *retResult);
		inline void SetMaxExecutionTime (DWORDLONG dwMilliseconds) { m_dwMaxExecutionTime = dwMilliseconds; }
		inline void SetLibraryCtx (const CString &sLibrary, void *pCtx) { m_LibraryCtx.SetAt(sLibrary, pCtx); }
		void SetSecurityCtx (const CHexeSecurityCtx &Ctx);

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

		ERunCodes Execute (CDatum *retResult);
		static bool FindHexarcMessage (const CString &sMsg, CString *retsAddr = NULL);
		void GetCurrentSecurityCtx (CHexeSecurityCtx *retCtx);
		CDatum GetCurrentSecurityCtx (void);
		void InitGlobalEnv (CHexeGlobalEnvironment **retpGlobalEnv = NULL);
		ERunCodes RunWithStack (CDatum dExpression, CDatum *retResult);
		inline bool SendHexarcMessage (const CString &sMsg, CDatum dPayload, CDatum *retdResult) { CHexeSecurityCtx Ctx; GetCurrentSecurityCtx(&Ctx); return SendHexarcMessage(Ctx, sMsg, dPayload, retdResult); }
		bool SendHexarcMessage (const CHexeSecurityCtx &SecurityCtx, const CString &sMsg, CDatum dPayload, CDatum *retdResult);
		static bool ValidateHexarcMessage (const CString &sMsg, CDatum dPayload, CString *retsAddr, CDatum *retdResult);

		//	Execution helpers
		bool ExecuteHandleInvokeResult (CDatum dExpression, CDatum dInvokeResult, CDatum *retResult);
		static bool ExecuteMakeFlagsFromArray (CDatum dOptions, CDatum dMap, CDatum *retdResult);
		static bool ExecuteSetAt (CDatum dOriginal, CDatum dKey, CDatum dValue, CDatum *retdResult);

		DWORDLONG m_dwMaxExecutionTime = 0;			//	Do not allow execution to exceed this amount of time (in ms)
		DWORDLONG m_dwAbortTime = 0;				//	Abort at this tick (0 = never abort)
		CHexeStack m_Stack;							//	Stack

		CDatum m_dExpression;						//	Current expression being executed
		DWORD *m_pIP = NULL;						//	Current instruction pointer
		CDatum m_dCodeBank;							//	Current code bank
		CHexeCode *m_pCodeBank = NULL;				//	Current code bank
		CHexeCallStack m_CallStack;					//	Call stack

		CDatum m_dGlobalEnv;						//	Process global environment

		CDatum m_dCurGlobalEnv;						//	Current global environment
		CHexeGlobalEnvironment *m_pCurGlobalEnv = NULL;	//	Current global environment

		CDatum m_dLocalEnv;							//	Local environment
		CHexeLocalEnvironment *m_pLocalEnv = NULL;	//	Local environment
		CHexeEnvStack m_LocalEnvStack;

		TSortMap<CString, void *> m_LibraryCtx;		//	Ctx for each library
		CHexeSecurityCtx m_UserSecurity;			//	User security context

		static SHexarcMsgInfo m_HexarcMsgInfo[];
		static int m_iHexarcMsgInfoCount;
		static TSortMap<CString, int> m_HexarcMsgIndex;
		static TArray<SHexarcMsgPattern> m_HexarcMsgPatterns;
	};

//	CHexe ----------------------------------------------------------------------

class CHexe
	{
	public:
		static bool Boot (void);

	private:
		static void Mark (void);

		static bool m_bInitialized;
	};

//	Utility Functions ----------------------------------------------------------

bool HexeGetPolygon2DArg (CDatum dArg, const CPolygon2D **retpPolygon, CPolygon2D *retTempStore, CDatum *retdResult);
bool HexeGetVector2DArg (CDatum dArgList, int *ioArg, CVector2D *retVector, CDatum *retdResult);
