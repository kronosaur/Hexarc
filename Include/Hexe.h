//	Hexe.h
//
//	Hexe EXecution Environment
//	Copyright (c) 2011 GridWhale Corporation. All Rights Reserved.
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
#include "HexeDocument.h"
#include "HexeLibrarian.h"
#include "HexeProcessImpl.h"

//#define DEBUG_HISTOGRAM

//	Interfaces Used by CHexeProcess --------------------------------------------

class IHexeComputeProgress
	{
	public:
		virtual ~IHexeComputeProgress () { }
		virtual bool OnAbortCheck () { return false; }
		virtual void OnCompute (DWORDLONG dwComputes, DWORDLONG dwLibraryTime) { }
		virtual void OnStart () { }
		virtual void OnStop () { }
	};

//	NOTE: This IDs are not (and should not be) persisted. They are only
//	used to identify the entry point when we call VMLibraryInvoke. They
//	can be reordered or changed at any time.

static constexpr DWORD EPID_NONE =					0;
static constexpr DWORD EPID_ARRAY_MAP =				1;
static constexpr DWORD EPID_CODE_COMPILE =			2;
static constexpr DWORD EPID_CODE_GET_NODE_STATUS =	3;
static constexpr DWORD EPID_CODE_GET_UI_VIEW =		4;
static constexpr DWORD EPID_CODE_RUN =				5;
static constexpr DWORD EPID_CODE_TERMINATE =		6;
static constexpr DWORD EPID_CONSOLE_CLEAR =			7;
static constexpr DWORD EPID_DICTIONARY_MAP =		8;
static constexpr DWORD EPID_HTTP_GET =				9;
static constexpr DWORD EPID_HTTP_POST =				10;
static constexpr DWORD EPID_TABLE_MAP =				11;
static constexpr DWORD EPID_TENSOR_MAP =			12;

static constexpr DWORD EPID_HOST =					1000;

class IHexeVMHost
	{
	public:

		virtual ~IHexeVMHost () { }

		virtual bool GetInput (const IInvokeCtx::SInputOptions &Options, CDatum& retdResult) { return Impl_StdIn(Options, retdResult); }
		virtual CDatum GetProcessID () const { return CDatum(); }
		virtual CDatum GetProgramInfo () const { return CDatum(); }
		virtual CDatum GetSystemObject () const { return Impl_GetSystemObject(); }
		virtual CDatum GetUsername () const { return CDatum(); }
		virtual void Mark () { }
		virtual void Output (CDatum dValue) { Impl_StdOut(dValue); }
		virtual void SetAsyncProgressFunc (CDatum dFunc) { }
		virtual bool VMLibraryInvoke (IInvokeCtx& Ctx, DWORD dwEntryPoint, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult) { return Impl_DefaultVMLibraryInvoke(Ctx, dwEntryPoint, LocalEnv, dContinueCtx, dContinueResult, retResult); }

	protected:

		static bool Impl_ArrayMap (IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult);
		static CDatum Impl_CalcDictionaryMapType (IInvokeCtx &Ctx, CDatum dDictionary, CDatum dMapFunc, int& retiArgs);
		static bool Impl_DictionaryMap (IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult);
		static CDatum Impl_GetSystemObject ();
		static bool Impl_StdIn (const IInvokeCtx::SInputOptions &Options, CDatum& retdResult);
		static void Impl_StdOut (CDatum dValue);
		static bool Impl_TableMap (IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult);
		bool Impl_DefaultVMLibraryInvoke (IInvokeCtx& Ctx, DWORD dwEntryPoint, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult);
		bool Impl_UnsuportedFeature (CDatum &retdResult) const;
	};

//	CHexeProgram ---------------------------------------------------------------

class CHexeProgram
	{
	public:

		void AddEntryPoint (const CString& sName, CDatum dType) { m_EntryPoints.SetAt(strToLower(sName), { CDatum(), dType }); }
		CDatum GetEntryPoint (const CString& sName) const;
		TArray<CHexeDocument::SEntryPoint> GetMainCode () const;
		const TArray<CString>& GetModules () const { return m_LibraryModules; }
		const CAEONTypeSystem& GetTypes () const { return m_Types; }
		CAEONTypeSystem& GetTypes () { return m_Types; }
		void Mark ();
		void SetMain (const CString& sName, CDatum dCode) { m_sMain = sName; m_dMainCode = dCode; }
		void SetModules (const TArray<CString>& Modules) { m_LibraryModules = Modules; }
		void SetTypes (CAEONTypeSystem&& Types) { m_Types = std::move(Types); }

	private:

		struct SEntry
			{
			CDatum dCode;
			CDatum dType;
			};

		CString m_sMain;
		CDatum m_dMainCode;

		CAEONTypeSystem m_Types;
		TArray<CString> m_LibraryModules;
		TSortMap<CString, SEntry> m_EntryPoints;
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

class CHexeProcess : public IInvokeCtx
	{
	public:
		static constexpr DWORD FLAG_MAP_EXCLUDE_NIL =		0x00000001;
		static constexpr DWORD FLAG_MAP_ORIGINAL =			0x00000002;

		static constexpr int LOCAL_ENUM_VAR =				0;
		static constexpr int LOCAL_INDEX_VAR =				1;
		static constexpr int LOCAL_ARRAY_VAR =				2;
		static constexpr int LOCAL_KEY_VAR =				3;

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
			ForcedTerminate,		//	Forced termination.

			Continue,
			};

		CHexeProcess (IHexeVMHost& Host = DefaultHost);

		static bool Boot (void);
		static void AddHexarcMsgPattern (const CString &sPrefix, const CString &sAddr);
		static void ComposeHexarcMessage (const CHexeSecurityCtx &SecurityCtx, const CString &sMsg, CDatum dPayload, CString *retsMsg, CDatum *retdPayload);
		static void CreateFunctionCall (int iArgCount, CDatum *retdExpression);
		static void CreateInvokeCall (const TArray<CDatum> &Args, CDatum *retdExpression);
		static bool IsHexarcMessage (const CString &sMsg, CString *retsAddress = NULL) { return FindHexarcMessage(sMsg, retsAddress); }

		static CDatum AsDatum (const IInvokeCtx::SLimits& Limits);
		static IInvokeCtx::SLimits InitLimitsFromDatum (CDatum dData);

		void DefineGlobal (const CString &sIdentifier, CDatum dValue);
		void DeleteAll (void);
		void DeleteLibraryCtx (const CString &sLibrary) { m_LibraryCtx.DeleteAt(sLibrary); }
		bool FindGlobalDef (const CString &sIdentifier, CDatum *retdValue = NULL);
		CDatum FindType (const CString &sFullyQualifiedName) const { return m_Types.FindType(sFullyQualifiedName); }
		DWORDLONG GetComputes () const { return m_dwComputes; }
		CDatum GetGlobalEnv (void) { return m_dGlobalEnv; }
		DWORDLONG GetLibraryTime () const { return m_dwLibraryTime; }
		CHexeStack &GetStack () { return m_Stack; }
		CDatum GetTypeList () const { return m_Types.GetTypeList(); }
		const CAEONTypeSystem &GetTypeSystem () const { return m_Types; }
		bool InitFrom (const CHexeProcess &Process, CString *retsError = NULL);
		bool InitFrom (CDatum dSerialized, CString *retsError = NULL);
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
		ERun RunContinuesFromStopCheck (CDatum& retResult);
		ERun RunEntryPoint (CStringView sEntryPoint, const TArray<CDatum>& Args, CDatum& retResult);
		ERun RunEventHandler (CDatum dFunc, const TArray<CDatum> &Args, CDatum &retResult);
		CDatum Serialize () const;
		void SetComputeProgress (IHexeComputeProgress &Progress) { m_pComputeProgress = &Progress; }
		void *SetLibraryCtx (const CString &sLibrary, void *pCtx);
		void SetLimits (const IInvokeCtx::SLimits &Limits) { m_Limits = Limits; }
		void SetOptionAddConcatenatesStrings (bool bValue = true) { m_bAddConcatenatesStrings = bValue; }
		void SetSecurityCtx (const CHexeSecurityCtx &Ctx);
		void SetTypeSystem (const CAEONTypeSystem &Types) { m_Types = Types; }
		void SignalPause (bool bPause = true);

		//	Execution helpers
		static CDatum ExecuteBinaryOp (IInvokeCtx& Ctx, EOpCodes iOp, CDatum dLeft, CDatum dRight);
		static int ExecuteCompare (CDatum dValue1, CDatum dValue2) { return dValue1.OpCompare(dValue2); }
		static bool ExecuteIsEquivalent (CDatum dValue1, CDatum dValue2) { return dValue1.OpIsEqual(dValue2); }
		static bool ExecuteIsIdentical (CDatum dValue1, CDatum dValue2) { return dValue1.OpIsIdentical(dValue2); }
		static CDatum ExecuteUnaryOp (EOpCodes iOp, CDatum dValue);
		static bool ExecuteIsShortOperand (int iValue) { return iValue < 0x7f0000 && iValue > -0x800000; }
		static CDatum ExecuteOpDivideMod (CDatum dDividend, CDatum dDivisor);

		//	IInvokeCtx virtuals

		virtual CString GetErrorMsg (EErrorMsg iError) const override;
		virtual bool GetInput (const IInvokeCtx::SInputOptions &Options, CDatum& retdResult) override { return m_Host.GetInput(Options, retdResult); }
		virtual void *GetLibraryCtx (const CString &sLibrary) override { void **ppCtx = m_LibraryCtx.GetAt(sLibrary); return (ppCtx ? *ppCtx : NULL); }
		virtual const SLimits& GetLimits () const override { return m_Limits; }
		virtual CDatum GetProcessID () const override { return m_Host.GetProcessID(); }
		virtual CDatum GetProgramInfo () const override { return m_Host.GetProgramInfo(); }
		virtual CRandomModule& GetRandomModule () { return m_RNG; }
		virtual CDatum GetSystemObject () const override { return m_Host.GetSystemObject(); }
		virtual CAEONTypeSystem& GetTypeSystem () override { return m_Types; }
		virtual CDatum GetUsername () const override { return m_Host.GetUsername(); }
		virtual CDatum GetVMInfo () const override;
		virtual void Output (CDatum dValue) override { m_Host.Output(dValue); }
		virtual CDatum SerializeProcess () const override { return Serialize(); }
		virtual void SetAsyncProgressFunc (CDatum dFunc) { m_Host.SetAsyncProgressFunc(dFunc); }
		virtual void SetUserSecurity (const CString &sUsername, const CAttributeList &Rights) override;
		virtual bool VMLibraryInvoke (DWORD dwEntryPoint, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult) override { return m_Host.VMLibraryInvoke(*this, dwEntryPoint, LocalEnv, dContinueCtx, dContinueResult, retResult); }

		static IHexeVMHost DefaultHost;

	private:

		using OpCodeExec = ERun (CHexeProcess::*) (CDatum& retResult);

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
		ERun ExecuteWithHistogram (CDatum *retResult);
		static bool FindHexarcMessage (const CString &sMsg, CString *retsAddr = NULL);
		void GetCurrentSecurityCtx (CHexeSecurityCtx *retCtx);
		CDatum GetCurrentSecurityCtx (void);
		CDatum GetStringFromDataBlock (int iID);
		void InitGlobalEnv (CHexeGlobalEnvironment **retpGlobalEnv = NULL);
		void InitInstructionTable ();
		static bool ParseHyperionServiceMessage (const CString &sMsg, CDatum dPayload, CString &sAddr, CString &sNewMsg, CDatum &dNewPayload);
		ERun RunWithStack (CDatum dExpression, CDatum *retResult);
		bool SendHexarcMessage (const CString &sMsg, CDatum dPayload, CDatum *retdResult);
		bool SendHexarcMessage (const CHexeSecurityCtx &SecurityCtx, const CString &sMsg, CDatum dPayload, CDatum *retdResult);
		static bool SendHexarcMessage (const CHexeSecurityCtx &SecurityCtx, const CString &sAddr, const CString &sMsg, CDatum dPayload, CDatum *retdResult);
		bool SendHexarcMessageSafe (const CString &sMsg, CDatum dPayload, CDatum *retdResult) { CHexeSecurityCtx Ctx; GetCurrentSecurityCtx(&Ctx); return SendHexarcMessage(Ctx, sMsg, dPayload, retdResult); }
		bool SetCodeBank (CDatum dCodeBank);
		static bool ValidateHexarcMessage (const CString &sMsg, CDatum dPayload, CString *retsAddr, CDatum *retdResult);

		//	Execution helpers

		static ERun RuntimeError (const CString &sError, CDatum &retdResult);
		bool ExecuteCustomMemberItem (CDatum dObject, const CString &sField, CDatum &retdResult);

		static constexpr DWORD FLAG_NO_ADVANCE =	0x00000001;
		static constexpr DWORD FLAG_NO_ENV =		0x00000002;
		ERun ExecuteHandleInvokeResult (CDatum::InvokeResult iInvokeResult, CDatum dExpression, const SAEONInvokeResult& Result, CDatum *retResult, DWORD dwFlags = 0);

		void ExecuteMakeArrayFromStack (CDatum dArray, int iElements);
		ERun ExecuteMakeTensorTypeFromStack (CDatum& retResult);

		static bool ExecuteMakeFlagsFromArray (CDatum dOptions, CDatum dMap, CDatum *retdResult);
		bool ExecuteObjectMemberItem (CDatum dObject, const CString &sField, CDatum& retdResult, bool& retbNoIPInc);
		static CDatum ExecuteOpAddCompatible (CDatum dLeft, CDatum dRight, bool bConcatenateStrings);
		static CDatum ExecuteOpSubtractCompatible (CDatum dLeft, CDatum dRight);
		ERun ExecuteReturnFromLibrary (const CHexeCallStack::SFrame& Frame, CDatum& retResult);
		bool ExecuteSetAt (CDatum dOriginal, CDatum dKey, CDatum dValue, CDatum *retdResult);
		bool ExecuteSetCustomMemberItem (CDatum dObject, const CString &sField, CDatum dValue, CDatum &retdResult);

		//	Op Code Execution

		ERun ExecuteAdd (CDatum& retResult);
		ERun ExecuteAdd2 (CDatum& retResult);
		ERun ExecuteAddInt (CDatum& retResult);
		ERun ExecuteAppendToArray (CDatum& retResult);
		ERun ExecuteCall (CDatum& retResult);
		ERun ExecuteCallLib (CDatum& retResult);
		ERun ExecuteCompareForEach (CDatum& retResult);
		ERun ExecuteCompareStep (CDatum& retResult);
		ERun ExecuteConcat (CDatum& retResult);
		ERun ExecuteDebugBreak (CDatum& retResult);
		ERun ExecuteDefine (CDatum& retResult);
		ERun ExecuteDefineArg (CDatum& retResult);
		ERun ExecuteDefineArgFromCode (CDatum& retResult);
		ERun ExecuteDefineNextArg (CDatum& retResult);
		ERun ExecuteDivide (CDatum& retResult);
		ERun ExecuteDivide2 (CDatum& retResult);
		ERun ExecuteEnterEnv (CDatum& retResult);
		ERun ExecuteError (CDatum& retResult);
		ERun ExecuteExitEnv (CDatum& retResult);
		ERun ExecuteExitEnvAndJumpIfGreaterInt (CDatum& retResult);
		ERun ExecuteExitEnvAndJumpIfGreaterOrEqualInt (CDatum& retResult);
		ERun ExecuteExitEnvAndJumpIfNil (CDatum& retResult);
		ERun ExecuteExitEnvAndReturn (CDatum& retResult);
		ERun ExecuteHalt (CDatum& retResult);
		ERun ExecuteHexarcMsg (CDatum& retResult);
		ERun ExecuteInc (CDatum& retResult);
		ERun ExecuteIncForEach (CDatum& retResult);
		ERun ExecuteIncLocalInt (CDatum& retResult);
		ERun ExecuteIncLocalL0 (CDatum& retResult);
		ERun ExecuteIncStep (CDatum& retResult);
		ERun ExecuteInitForEach (CDatum& retResult);
		ERun ExecuteIsEqual (CDatum& retResult);
		ERun ExecuteIsEqualInt (CDatum& retResult);
		ERun ExecuteIsEqualMulti (CDatum& retResult);
		ERun ExecuteIsGreater (CDatum& retResult);
		ERun ExecuteIsGreaterInt (CDatum& retResult);
		ERun ExecuteIsGreaterMulti (CDatum& retResult);
		ERun ExecuteIsGreaterOrEqual (CDatum& retResult);
		ERun ExecuteIsGreaterOrEqualInt (CDatum& retResult);
		ERun ExecuteIsGreaterOrEqualMulti (CDatum& retResult);
		ERun ExecuteIsIdentical (CDatum& retResult);
		ERun ExecuteIsIn (CDatum& retResult);
		ERun ExecuteIsLess (CDatum& retResult);
		ERun ExecuteIsLessInt (CDatum& retResult);
		ERun ExecuteIsLessMulti (CDatum& retResult);
		ERun ExecuteIsLessOrEqual (CDatum& retResult);
		ERun ExecuteIsLessOrEqualInt (CDatum& retResult);
		ERun ExecuteIsLessOrEqualMulti (CDatum& retResult);
		ERun ExecuteIsNotEqual (CDatum& retResult);
		ERun ExecuteIsNotEqualInt (CDatum& retResult);
		ERun ExecuteIsNotEqualMulti (CDatum& retResult);
		ERun ExecuteIsNotIdentical (CDatum& retResult);
		ERun ExecuteJump (CDatum& retResult);
		ERun ExecuteJumpIfNil (CDatum& retResult);
		ERun ExecuteJumpIfNilNoPop (CDatum& retResult);
		ERun ExecuteJumpIfNotNilNoPop (CDatum& retResult);
		ERun ExecuteLoopIncAndJump (CDatum& retResult);
		ERun ExecuteMakeApplyEnv (CDatum& retResult);
		ERun ExecuteMakeArray (CDatum& retResult);
		ERun ExecuteMakeAsType (CDatum& retResult);
		ERun ExecuteMakeAsTypeCons (CDatum& retResult);
		ERun ExecuteMakeBlockEnv (CDatum& retResult);
		ERun ExecuteMakeDatatype (CDatum& retResult);
		ERun ExecuteMakeEmptyArray (CDatum& retResult);
		ERun ExecuteMakeEmptyArrayAsType (CDatum& retResult);
		ERun ExecuteMakeEmptyStruct (CDatum& retResult);
		ERun ExecuteMakeEnv (CDatum& retResult);
		ERun ExecuteMakeExpr (CDatum& retResult);
		ERun ExecuteMakeExprIf (CDatum& retResult);
		ERun ExecuteMakeFlagsFromArray (CDatum& retResult);
		ERun ExecuteMakeFunc (CDatum& retResult);
		ERun ExecuteMakeFunc2 (CDatum& retResult);
		ERun ExecuteMakeLocalEnv (CDatum& retResult);
		ERun ExecuteMakeMapColExpr (CDatum& retResult);
		ERun ExecuteMakeMethodEnv (CDatum& retResult);
		ERun ExecuteMakeObject (CDatum& retResult);
		ERun ExecuteMakePrimitive (CDatum& retResult);
		ERun ExecuteMakeRange (CDatum& retResult);
		ERun ExecuteMakeSpread (CDatum& retResult);
		ERun ExecuteMakeStruct (CDatum& retResult);
		ERun ExecuteMakeTensor (CDatum& retResult);
		ERun ExecuteMakeTensorType (CDatum& retResult);
		ERun ExecuteMapResult (CDatum& retResult);
		ERun ExecuteMod (CDatum& retResult);
		ERun ExecuteMultiply (CDatum& retResult);
		ERun ExecuteMultiply2 (CDatum& retResult);
		ERun ExecuteMutateArrayItemAdd (CDatum& retResult);
		ERun ExecuteMutateArrayItemConcat (CDatum& retResult);
		ERun ExecuteMutateArrayItemDivide (CDatum& retResult);
		ERun ExecuteMutateArrayItemMod (CDatum& retResult);
		ERun ExecuteMutateArrayItemMultiply (CDatum& retResult);
		ERun ExecuteMutateArrayItemPower (CDatum& retResult);
		ERun ExecuteMutateArrayItemSubtract (CDatum& retResult);
		ERun ExecuteMutateGlobalAdd (CDatum& retResult);
		ERun ExecuteMutateGlobalConcat (CDatum& retResult);
		ERun ExecuteMutateGlobalDivide (CDatum& retResult);
		ERun ExecuteMutateGlobalMod (CDatum& retResult);
		ERun ExecuteMutateGlobalMultiply (CDatum& retResult);
		ERun ExecuteMutateGlobalPower (CDatum& retResult);
		ERun ExecuteMutateGlobalSubtract (CDatum& retResult);
		ERun ExecuteMutateLocalAdd (CDatum& retResult);
		ERun ExecuteMutateLocalConcat (CDatum& retResult);
		ERun ExecuteMutateLocalDivide (CDatum& retResult);
		ERun ExecuteMutateLocalMod (CDatum& retResult);
		ERun ExecuteMutateLocalMultiply (CDatum& retResult);
		ERun ExecuteMutateLocalPower (CDatum& retResult);
		ERun ExecuteMutateLocalSubtract (CDatum& retResult);
		ERun ExecuteMutateObjectItemAdd (CDatum& retResult);
		ERun ExecuteMutateObjectItemConcat (CDatum& retResult);
		ERun ExecuteMutateObjectItemDivide (CDatum& retResult);
		ERun ExecuteMutateObjectItemMod (CDatum& retResult);
		ERun ExecuteMutateObjectItemMultiply (CDatum& retResult);
		ERun ExecuteMutateObjectItemPower (CDatum& retResult);
		ERun ExecuteMutateObjectItemSubtract (CDatum& retResult);
		ERun ExecuteMutateTensorItemAdd (CDatum& retResult);
		ERun ExecuteMutateTensorItemConcat (CDatum& retResult);
		ERun ExecuteMutateTensorItemDivide (CDatum& retResult);
		ERun ExecuteMutateTensorItemMod (CDatum& retResult);
		ERun ExecuteMutateTensorItemMultiply (CDatum& retResult);
		ERun ExecuteMutateTensorItemPower (CDatum& retResult);
		ERun ExecuteMutateTensorItemSubtract (CDatum& retResult);
		ERun ExecuteNegate (CDatum& retResult);
		ERun ExecuteNewObject (CDatum& retResult);
		ERun ExecuteNoOp (CDatum& retResult) { m_pIP++; return ERun::Continue; }
		ERun ExecuteNot (CDatum& retResult);
		ERun ExecutePop (CDatum& retResult);
		ERun ExecutePopDeep (CDatum& retResult);
		ERun ExecutePopLocal (CDatum& retResult);
		ERun ExecutePopLocalL0 (CDatum& retResult);
		ERun ExecutePower (CDatum& retResult);
		ERun ExecutePushArrayItem (CDatum& retResult);
		ERun ExecutePushArrayItemI (CDatum& retResult);
		ERun ExecutePushCoreType (CDatum& retResult);
		ERun ExecutePushDatum (CDatum& retResult);
		ERun ExecutePushFalse (CDatum& retResult);
		ERun ExecutePushGlobal (CDatum& retResult);
		ERun ExecutePushInitForEach (CDatum& retResult);
		ERun ExecutePushInt (CDatum& retResult);
		ERun ExecutePushIntShort (CDatum& retResult);
		ERun ExecutePushLiteral (CDatum& retResult);
		ERun ExecutePushLocal (CDatum& retResult);
		ERun ExecutePushLocalItem (CDatum& retResult);
		ERun ExecutePushLocalL0 (CDatum& retResult);
		ERun ExecutePushLocalLength (CDatum& retResult);
		ERun ExecutePushNaN (CDatum& retResult);
		ERun ExecutePushNil (CDatum& retResult);
		ERun ExecutePushObjectItem (CDatum& retResult);
		ERun ExecutePushObjectMethod (CDatum& retResult);
		ERun ExecutePushStr (CDatum& retResult);
		ERun ExecutePushStrNull (CDatum& retResult);
		ERun ExecutePushTensorItem (CDatum& retResult);
		ERun ExecutePushTensorItemI (CDatum& retResult);
		ERun ExecutePushTrue (CDatum& retResult);
		ERun ExecutePushType (CDatum& retResult);
		ERun ExecuteReturn (CDatum& retResult);
		ERun ExecuteSetArrayItem (CDatum& retResult);
		ERun ExecuteSetArrayItemI (CDatum& retResult);
		ERun ExecuteSetForEachItem (CDatum& retResult);
		ERun ExecuteSetGlobal (CDatum& retResult);
		ERun ExecuteSetGlobalItem (CDatum& retResult);
		ERun ExecuteSetLocal (CDatum& retResult);
		ERun ExecuteSetLocalItem (CDatum& retResult);
		ERun ExecuteSetLocalL0 (CDatum& retResult);
		ERun ExecuteSetObjectItem (CDatum& retResult);
		ERun ExecuteSetObjectItem2 (CDatum& retResult);
		ERun ExecuteSetTensorItem (CDatum& retResult);
		ERun ExecuteSetTensorItemI (CDatum& retResult);
		ERun ExecuteSubtract (CDatum& retResult);
		ERun ExecuteSubtract2 (CDatum& retResult);
		ERun ExecuteSubtractInt (CDatum& retResult);

		//	Options

		IHexeVMHost& m_Host;
		IHexeComputeProgress *m_pProgress = NULL;	//	Callback to report progress
		bool m_bAddConcatenatesStrings = false;		//	If TRUE, then + will concatenate string (instead of converting 
													//		them to numbers).

		IInvokeCtx::SLimits m_Limits;				//	Limits on execution

		//	Execution State

		TArray<CString> m_Libraries;				//	Libraries loaded
		CAEONTypeSystem m_Types;					//	Static types
		CDatum m_dExpression;						//	Current expression being executed
		CHexeStack m_Stack;							//	Stack
		DWORD *m_pIP = NULL;						//	Current instruction pointer
		CDatum m_dCodeBank;							//	Current code bank
		const CHexeCode *m_pCodeBank = NULL;		//	Current code bank
		CHexeCallStack m_CallStack;					//	Call stack
		int m_iEventHandlerLevel = 0;				//	>0 = Inside event handler
		DWORDLONG m_dwAbortTime = 0;				//	Abort at this tick (0 = never abort)
		CRandomModule m_RNG;						//	Random number generator

		CDatum m_dGlobalEnv;						//	Process global environment
		CHexeEnvStack m_Env;						//	Environment stack
		CHexeGlobalEnvCache m_GlobalEnvCache;		//	Cache of global environments

		TSortMap<CString, void *> m_LibraryCtx;		//	Ctx for each library
		CHexeSecurityCtx m_UserSecurity;			//	User security context

		IHexeComputeProgress *m_pComputeProgress = NULL;
		DWORDLONG m_dwComputes = 0;					//	Total instructions processed so far.
		DWORDLONG m_dwLibraryTime = 0;				//	Total milliseconds spent executing
													//		native library functions.
#ifdef DEBUG_HISTOGRAM
		bool m_bEnableHistogram = true;				//	If TRUE, then we keep a histogram of
													//	execution times.
#else
		bool m_bEnableHistogram = false;
#endif
		CHexeCodeHistogram m_Histogram;

		CCriticalSection m_cs;
		bool m_bSignalPause = false;

		OpCodeExec m_INSTRUCTION[opCodeCount] = { NULL };

		static SHexarcMsgInfo m_HexarcMsgInfo[];
		static int m_iHexarcMsgInfoCount;
		static TSortMap<CString, int> m_HexarcMsgIndex;
		static TArray<SHexarcMsgPattern> m_HexarcMsgPatterns;
	};

//	CHexeError -----------------------------------------------------------------

class CHexeError
	{
	public:
		static void Create (const CString &sErrorCode, const CString &sErrorDesc, CDatum *retdDatum)
			{ *retdDatum = CDatum::CreateError(sErrorDesc, sErrorCode); }
	};

//	CHexeText ------------------------------------------------------------------

class CHexeTextMarkup
	{
	public:
		static bool ConvertToHTML (const IMemoryBlock &Input, const CString &sFormat, CDatum dParams, IByteStream &Output, CString *retsError);
		static bool EscapeHTML (const IMemoryBlock &Template, CDatum dStruct, IByteStream &Output, CString *retsError = NULL);
		static CString FormatString (CHexeStackEnv& LocalEnv);
		static void WriteHTMLContent (CDatum dValue, IByteStream &Output);
	};

//	CHexe ----------------------------------------------------------------------

class CHexe
	{
	public:
		static bool Boot (void);

		static bool RunFunction (CDatum dFunc, TArray<CDatum>&& Args, CDatum dContext, SAEONInvokeResult& retResult);
		static bool RunFunction1Arg (CDatum dFunc, CDatum dArg, CDatum dContext, SAEONInvokeResult& retResult);
		static bool RunFunction2Args (CDatum dFunc, CDatum dArg1, CDatum dArg2, CDatum dContext, SAEONInvokeResult& retResult);

		static bool InvokeHexarcMsg (const CString &sMsg, CDatum dPayload, CDatum &retdResult);

	private:
		static void Mark (void);

		static bool m_bInitialized;
	};

//	Utility Functions ----------------------------------------------------------

bool HexeGetPolygon2DArg (CDatum dArg, const CPolygon2D **retpPolygon, CPolygon2D *retTempStore, CDatum *retdResult);
bool HexeGetVector2DArg (CHexeStackEnv& LocalEnv, int *ioArg, CVector2D *retVector, CDatum *retdResult);

#include "HexeConsole.h"
#include "HexeProcessors.h"

