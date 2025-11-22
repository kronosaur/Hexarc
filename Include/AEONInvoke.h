//	AEONInvoke.h
//
//	AEON Invoke Implementation
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#pragma once

class CHexeLocalEnvironment;

//	IInvokeCtx -----------------------------------------------------------------

class IInvokeCtx
	{
	public:

		//	NOTE: Must match HexeProcessImpl.h

		static constexpr DWORD EXEC_RIGHT_SIDE_EFFECTS =		0x00000001;
		static constexpr DWORD EXEC_RIGHT_INVOKE =				0x00000002;

		static constexpr DWORD EXEC_FLAG_CONSTRUCTOR =			0x00010000;

		struct SInputOptions
			{
			CString sPrompt;
			bool bNoEcho = false;
			};

		struct SLimits
			{
			int iMaxExecutionTimeSec = 30;				//	Max execution time (in seconds) before a stop check
			int iMaxStackDepth = 10'000;				//	Max stack depth
			int iMaxArrayLen = 10'000'000;				//	Max array length
			int iMaxStringSize = 1'000'000;				//	Max string size in bytes
			int iMaxBufferSize = 100'000'000;			//	Max buffer size in bytes
			};

		enum class EErrorMsg
			{
			Unknown,

			ArrayLimit,
			};

		virtual ~IInvokeCtx () { }

		inline bool CheckArrayLimit (DWORDLONG dwSize, CDatum& retdResult) const;
		virtual CString GetErrorMsg (EErrorMsg iError) const { return NULL_STR; }
		virtual bool GetInput (const IInvokeCtx::SInputOptions &Options, CDatum& retdResult) = 0;
		virtual void *GetLibraryCtx (const CString &sLibrary) { return NULL; }
		virtual const SLimits& GetLimits () const = 0;
		virtual CDatum GetProcessID () const { return CDatum(); }
		virtual CDatum GetProgramInfo () const { return CDatum(); }
		virtual CRandomModule& GetRandomModule () = 0;
		virtual CDatum GetSystemObject () const { return CDatum(); }
		virtual CAEONTypeSystem& GetTypeSystem () = 0;
		virtual CDatum GetUsername () const { return CDatum(); }
		virtual CDatum GetVMInfo () const { return CDatum(); }
		virtual void Output (CDatum dValue) = 0;
		virtual CDatum SerializeProcess () const = 0;
		virtual void SetAsyncProgressFunc (CDatum dFunc) { }
		virtual void SetUserSecurity (const CString &sUsername, const CAttributeList &Rights) { }
		virtual bool VMLibraryInvoke (DWORD dwEntryPoint, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult) = 0;

		static bool CanExecute (DWORD dwFuncFlags, DWORD dwRightsGranted)
			{
			DWORD dwRightsRequired = dwFuncFlags & EXECUTION_FLAGS_MASK;
			return ((dwRightsRequired & dwRightsGranted) == dwRightsRequired);
			}

	private:

		static constexpr DWORD EXECUTION_FLAGS_MASK = 0x0000FFFF;
	};

inline bool IInvokeCtx::CheckArrayLimit (DWORDLONG dwSize, CDatum& retdResult) const
	{
	const SLimits &Limits = GetLimits();
	if (dwSize > (DWORDLONG)Limits.iMaxArrayLen)
		{
		retdResult = CDatum(GetErrorMsg(EErrorMsg::ArrayLimit));
		return false;
		}
	
	return true;
	}

//	CHexeLocalEnvironment ------------------------------------------------------

class CHexeStackEnv
	{
	public:

		CHexeStackEnv () { }
		CHexeStackEnv (CDatum* pStart, int iCount);
		CHexeStackEnv (TArray<CDatum>&& Stack);

		void AppendArgumentValue (CDatum dValue);
		CDatum GetArgument (int iIndex) const { return (iIndex < m_iCount ? m_pStart[iIndex] : CDatum()); }
		int GetCount () const { return m_iCount; }
		CDatum GetElement (int iIndex) const { return GetArgument(iIndex); }
		CDatum MathAverage () const { return AsDatum().MathAverage(); }
		CDatum MathMax () const { return AsDatum().MathMax(); }
		CDatum MathMedian () const { return AsDatum().MathMedian(); }
		CDatum MathMin () const { return AsDatum().MathMin(); }
		CDatum MathSum () const { return AsDatum().MathSum(); }

	private:

		CDatum AsDatum () const;
		void Expand (CDatum *pStart, int iCount);

		CDatum* m_pStart = NULL;
		int m_iCount = 0;

		TArray<CDatum> m_Stack;
	};

class CHexeLocalEnvPointer
	{
	public:

		CHexeLocalEnvPointer () { }
		CHexeLocalEnvPointer (const CHexeLocalEnvPointer &Src) = delete;
		CHexeLocalEnvPointer (CHexeLocalEnvPointer &&Src) noexcept;
		explicit CHexeLocalEnvPointer (CDatum dEnv);
		explicit CHexeLocalEnvPointer (int iArgCount);
		explicit CHexeLocalEnvPointer (CHexeLocalEnvironment *pEnv) : m_pEnv(pEnv) { }

		~CHexeLocalEnvPointer () { CleanUp(); }

		CHexeLocalEnvPointer &operator= (const CHexeLocalEnvPointer &Src) = delete;
		CHexeLocalEnvPointer &operator= (CHexeLocalEnvPointer &&Src) noexcept;

		operator CHexeLocalEnvironment* () const { return m_pEnv; }

		CHexeLocalEnvironment& AsEnv () const { return *m_pEnv; }
		void DeleteAll () { CleanUp(); m_dEnv = CDatum(); m_pEnv = NULL; }
		CDatum GetClosure () const;
		CDatum GetDatum () const { return m_dEnv; }
		CHexeLocalEnvironment* GetEnv () const { return m_pEnv; }
		CHexeLocalEnvironment* GetHandoff () { ASSERT(m_dEnv.IsIdenticalToNil()); CHexeLocalEnvironment *pEnv = m_pEnv; m_pEnv = NULL; return pEnv; }
		bool IsEmpty () const { return m_pEnv == NULL; }
		void Mark ();
		bool TrackedByGC () const { return m_pEnv && !m_dEnv.IsIdenticalToNil(); }

	private:

		//	This class is used to hold a pointer to a local environment. We 
		//	either hold a CDatum (in which case, GC handles everything) or
		//	we have a unique pointer to a CHexeLocalEnvironment (in which case
		//	we own it and need to manage it).

		void CleanUp ();

		mutable CDatum m_dEnv;
		CHexeLocalEnvironment *m_pEnv = NULL;
	};

class CHexeLocalEnvironment : public TExternalDatum<CHexeLocalEnvironment>
	{
	public:

		CHexeLocalEnvironment () { m_pArray = m_BaseArray; }
		explicit CHexeLocalEnvironment (int iCount);

		static const CString &StaticGetTypename (void);

		void AppendArgumentValue (CDatum dValue);
		bool FindArgument (const CString &sArg, int *retiLevel, int *retiIndex);
		CDatum GetArgument (int iIndex) const { return (iIndex < GetArgumentCount() ? m_pArray[iIndex].dValue : CDatum()); }
		CDatum GetArgument (int iLevel, int iIndex);
		CDatum GetArgumentsAsCacheKey () const;
		CHexeLocalEnvironment& GetEnvAtLevel (int iLevel);
		int GetNextArg () const { return m_iNextArg; }
		CHexeLocalEnvironment* GetParentEnv ();
		CHexeLocalEnvironment* GetParentEnvHandoff ();
		CDatum GetParentEnvClosure ();
		void IncArgumentValue (int iIndex, int iInc);
		CDatum IncArgumentValueInt32 (int iIndex, int iInc);
		void Init (int iCount = 0);
		CString MakeCacheKey () const;
		inline CDatum OpAdd (int iIndex, CDatum dValue);
		void ResetNextArg (void) { m_iNextArg = 0; }
		void SetArgumentKey (int iLevel, int iIndex, CStringView sKey);
		void SetArgumentValue (int iIndex, CDatum dValue) { ASSERT(iIndex < GetAllocSize()); m_pArray[iIndex].dValue = dValue; }
		void SetArgumentValue (int iLevel, int iIndex, CDatum dValue);
		void SetElement (int iIndex, CDatum dValue) { ASSERT(iIndex < GetAllocSize()); m_pArray[iIndex].dValue = dValue; }
		void SetNextArg (int iValue) { GrowArray(iValue); m_iNextArg = iValue; }
		void SetNextArgKey (CStringView sKey) { SetArgumentKey(0, m_iNextArg, sKey); if (m_iNextArg < m_iArgCount) m_iNextArg++; }
		void SetParentEnv (CDatum dParentEnv);
		void SetParentEnv (CHexeLocalEnvPointer&& ParentEnv);

		//	IComplexDatum
		virtual bool Contains (CDatum dValue) const override;
		virtual int GetCount (void) const override { return GetArgumentCount(); }
		virtual CDatum GetElement (int iIndex) const override { return (iIndex < GetArgumentCount() ? m_pArray[iIndex].dValue : CDatum()); }
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual CString GetKey (int iIndex) const override { return m_pArray[iIndex].sArg; }
		virtual bool IsArray (void) const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil () const { return GetArgumentCount() == 0; }
		virtual CDatum MathMax () const override;
		virtual CDatum MathMin () const override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override;

	protected:

		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		static constexpr int DEFAULT_SIZE = 10;

		struct SEntry
			{
			CString sArg;
			CDatum dValue;
			};

		int GetAllocSize () const { return Max(DEFAULT_SIZE, m_DynamicArray.GetCount()); }
		int GetArgumentCount () const { return m_iArgCount; }
		void GrowArray (int iNewCount);

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override;

		CHexeLocalEnvPointer m_ParentEnv;
		int m_iArgCount = 0;
		int m_iNextArg = 0;

		SEntry m_BaseArray[DEFAULT_SIZE];
		SEntry *m_pArray = NULL;

		TArray<SEntry> m_DynamicArray;
	};

