//	HexeProcessImpl.h
//
//	Hexe header file
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Hexe.h

#pragma once

#ifdef DEBUG
//#define DEBUG_PROC_ENV
#endif

class CHexeCode;
class CHexeGlobalEnvironment;
class CHexeLocalEnvironment;

//	CHexeStack -----------------------------------------------------------------

class CHexeStack
	{
	public:
		static constexpr int DEFAULT_SIZE = 4000;

		CHexeStack (int iSize = DEFAULT_SIZE) { m_Stack = new CDatum [iSize]; m_iAlloc = iSize; }
		CHexeStack (const CHexeStack &Src) = delete;
		CHexeStack (CHexeStack &&Src) noexcept = delete;

		~CHexeStack () { delete [] m_Stack; }

		CHexeStack& operator= (const CHexeStack &Src) = delete;
		CHexeStack& operator= (CHexeStack &&Src) noexcept = delete;

		void DeleteAll (void) { m_iTop = -1; }
		int GetCount () const { return m_iTop + 1; }
		void Mark (void);

#ifdef DEBUG_PROC_ENV
		CDatum Get (void) const { ValidatePos(m_iTop); return m_Stack[m_iTop]; }
		CDatum Get (int iIndex) const { ValidatePos(m_iTop - iIndex); return m_Stack[m_iTop - iIndex]; }
		CDatum& GetRef (void) { ValidatePos(m_iTop); return m_Stack[m_iTop]; }

		CDatum Pop (void) { ValidatePos(m_iTop); return m_Stack[m_iTop--]; }
		void Pop (int iCount) { m_iTop -= iCount; if (m_iTop < -1) throw CException(errFail); }
		void Push (CDatum dData) { if (dData.GetBasicType() == CDatum::typeUnknown) throw CException(errFail); ValidatePos(m_iTop+1); m_Stack[++m_iTop] = dData; }
		void Replace (CDatum dValue, int iPop = 1) { m_iTop -= (iPop - 1); ValidatePos(m_iTop); m_Stack[m_iTop] = dValue; }
#else
		__forceinline CDatum Get (void) const { return m_Stack[m_iTop]; }
		__forceinline CDatum Get (int iIndex) const { return m_Stack[m_iTop - iIndex]; }
		__forceinline CDatum& GetRef (void) { return m_Stack[m_iTop]; }

		__forceinline CDatum Pop (void) { return m_Stack[m_iTop--]; }
		__forceinline int PopInt (void) { return m_Stack[m_iTop--].raw_GetInt32(); }
		__forceinline void Pop (int iCount) { m_iTop -= iCount; }
		__forceinline void Push (CDatum dData) { m_Stack[++m_iTop] = dData; }
		__forceinline void Replace (CDatum dValue, int iPop = 1) { m_iTop -= (iPop - 1); m_Stack[m_iTop] = dValue; }
		__forceinline void Set (CDatum dValue) { m_Stack[m_iTop] = dValue; }
#endif

		CDatum SafeGet (void) const { return (m_iTop == -1 ? CDatum() : m_Stack[m_iTop]); }
		CDatum SafeGet (int iIndex) const;
		CDatum SafePop (void) { return (m_iTop == -1 ? CDatum() : m_Stack[m_iTop--]); }
		void SafePop (int iCount) { if (m_iTop + 1 >= iCount) m_iTop -= iCount; else m_iTop = -1; }

	private:

		void ValidatePos (int iPos) const { if (iPos < 0 || iPos >= m_iAlloc) throw CException(errFail); }

		CDatum* m_Stack;
		int m_iAlloc = 0;						//	Allocated size of stack
		int m_iTop = -1;						//	Index to item at top of stack
	};

//	CHexeCallStack -------------------------------------------------------------

class CHexeCallStack
	{
	public:

		static constexpr DWORD NULL_CALL =		0x00;
		static constexpr DWORD FUNC_CALL =		0x01;
		static constexpr DWORD SYSTEM_CALL =	0x02;

		struct SFrame
			{
			DWORD dwType = NULL_CALL;
			DWORD dwFlags = 0;
			DWORD* pIP = NULL;
			CDatum dExpression;
			CDatum dCodeBank;

			CDatum dPrimitive;
			CDatum dContext;
			};

		CHexeCallStack ();

		void DeleteAll () { m_Stack.DeleteAll(); }
		int GetCount () const { return m_Stack.GetCount(); }
		void Mark ();

		const SFrame& Top () const;
		void PushFunCall (CDatum dExpression, CDatum dCodeBank, DWORD* pIP);
		void PushSysCall (CDatum dExpression, CDatum dCodeBank, DWORD* pIP, CDatum dPrimitive, CDatum dContext, DWORD dwFlags);
		void Pop ();

	private:

		static constexpr int DEFAULT_STACK_SIZE = 100;

		TArray<SFrame> m_Stack;
		SFrame m_Null;
	};

//	CHexeEnvStack --------------------------------------------------------------

class CHexeEnvStack
	{
	public:

		CHexeEnvStack () { }
		CHexeEnvStack (const CHexeEnvStack &Src) = delete;
		CHexeEnvStack (CHexeEnvStack &&Src) = delete;

		~CHexeEnvStack () { DeleteAll(); }

		CHexeEnvStack& operator= (const CHexeEnvStack &Src) = delete;
		CHexeEnvStack& operator= (CHexeEnvStack &&Src) = delete;

		void DeleteAll ();
		CHexeGlobalEnvironment& GetGlobalEnv (void) const { return *m_pCurGlobalEnv; }
		CDatum GetGlobalEnvClosure () const { return m_dCurGlobalEnv; }
		CHexeLocalEnvironment& GetLocalEnv (void) const { return m_pCurLocalEnv.AsEnv(); }
		CHexeLocalEnvironment& GetLocalEnvAt (int iLevel) const;
		CDatum GetLocalEnvClosure () const;
		void Init (CDatum dGlobalEnv);
		void Mark ();
		void PopFrame ();
		void PushNewFrame (int iArgCount = 0);
		void PushNewFrameAsChild (int iArgCount = 0);
		void SetGlobalEnv (CDatum dGlobalEnv, CHexeGlobalEnvironment *pGlobalEnv = NULL);
		void SetLocalEnvParent (CDatum dLocalEnv);

	private:

		struct SEnvCtx
			{
			CDatum dGlobalEnv;
			CHexeGlobalEnvironment *pGlobalEnv = NULL;

			//	dLocalEnv is one of the following values:
			//
			//	nil: pLocalEnv is the local environment, but we don't own it.
			//	true: pLocalEnv is the local environment, and we own it.
			//	CHexeLocalEnvironment datum: pLocalEnv is the local environment,
			//		but we will garbage collect it.

			CDatum dLocalEnv;
			CHexeLocalEnvironment *pLocalEnv = NULL;
			};

		CHexeLocalEnvPointer AllocEnv (int iArgCount);
		void FreeEnv (CHexeLocalEnvPointer&& pEnv);
		void SetLocalEnv (SEnvCtx& Ctx, const CHexeLocalEnvPointer& pLocalEnv);
		void SetLocalEnv (SEnvCtx& Ctx, CHexeLocalEnvPointer&& pLocalEnv);

		TArray<SEnvCtx> m_Stack;
		TStack<CHexeLocalEnvPointer> m_Free;

		CDatum m_dCurGlobalEnv;
		CHexeGlobalEnvironment *m_pCurGlobalEnv = NULL;

		CHexeLocalEnvPointer m_pCurLocalEnv;
	};

class CHexeSecurityCtx
	{
	public:

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

class CHexeCodeHistogram
	{
	public:
		void AddEntry (DWORD dwOpCode, DWORD dwTime);
		void DebugOutput (void);

	private:

		struct SHistogramEntry
			{
			int iCount = 0;
			DWORD dwTotalTime = 0;
			};

		SHistogramEntry m_Histogram[opCodeCount];
	};

class CHexeGlobalEnvCache
	{
	public:

		static constexpr DWORD INVALID_ID = 0xffffffff;

		void DeleteAll () { m_IDs.DeleteAll(); }
		DWORD GetID (int iIndex) const { return (iIndex >= 0 && iIndex < m_IDs.GetCount() ? m_IDs[iIndex] : INVALID_ID); }
		void SetID (int iIndex, DWORD dwID);

	private:

		TArray<DWORD> m_IDs;
	};


