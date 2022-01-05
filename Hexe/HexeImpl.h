//	HexeImpl.h
//
//	Hexe header file
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#pragma once

enum EOperandTypes
	{
	operandNone,				//	No operand for opcode
	operandIntShort,			//	Operand is an integer encoded in the lower 24 bits of the opcode
	operandCodeOffset,			//	Operand is a block offset pointing to code
	operandStringOffset,		//	Operand is a block offset pointing to a string
	operandDatumOffset,			//	Operand is a block offset pointing to a serialized datum
	operandInt,					//	Operand is an integer following the opcode
	};

struct SOpCodeInfo
	{
	SOpCodeInfo (DWORD dwOpCodeArg, const CString &sOpCodeArg, EOperandTypes iOperandArg) :
			dwOpCode(dwOpCodeArg),
			sOpCode(sOpCodeArg),
			iOperand(iOperandArg)
		{ }

	DWORD dwOpCode;							//	Opcode
	const CString &sOpCode;					//	Opcode name
	EOperandTypes iOperand;					//	Type of operand
	};

class COpCodeDatabase
	{
	public:
		COpCodeDatabase (void);

		DWORD *Advance (DWORD *pPos);
		inline SOpCodeInfo *GetInfo (DWORD dwOpCode) { return m_Info[dwOpCode >> 24]; }

	private:
		SOpCodeInfo *m_Info[opCodeCount];
	};

inline DWORD GetOpCode (DWORD dwCode) { return (dwCode & 0xff000000); }
inline DWORD GetOperand (DWORD dwCode) { return (dwCode & 0x00ffffff); }
inline DWORD MakeOpCode (DWORD dwOpCode, DWORD dwOperand) { return (dwOpCode | dwOperand); }

extern COpCodeDatabase g_OpCodeDb;

//	CHexeCode ------------------------------------------------------------------
//
//	This is a datum that contains a block of HexeCode.

class CHexeCode : public TExternalDatum<CHexeCode>
	{
	public:
		static void Create (const CHexeCodeIntermediate &Intermediate, int iEntryPoint, CDatum *retdEntryPoint);
		static void CreateFunctionCall (const CString &sFunction, const TArray<CDatum> &Args, CDatum *retdEntryPoint);
		static void CreateFunctionCall (int iArgCount, CDatum *retdEntryPoint);
		static void CreateInvokeCall (const TArray<CDatum> &Args, CDatum *retdEntryPoint);
		static const CString &StaticGetTypename (void);

		inline DWORD *GetCode (int iOffset) { return (DWORD *)(m_Code.GetPointer() + iOffset); }
		CDatum GetDatum (int iOffset);
		static int GetOperandInt (DWORD dwCode);
		inline CString GetString (int iOffset) { return CString(m_Code.GetPointer() + iOffset); }
		inline CString GetStringLiteral (int iOffset) { return CString(m_Code.GetPointer() + iOffset, -1, true); }

	protected:
		//	IComplexDatum
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:
		enum BlockTypes
			{
			blockUnknown =			0x00000000,

			blockCode =				0x10000000,
			blockString =			0x20000000,
			blockDatum =			0x30000000,

			blockTypeMask =			0xf0000000,
			blockLenMask =			0x0fffffff,
			};

		typedef DWORD BLOCKHEADER;

		inline static BLOCKHEADER ComposeHeader (BlockTypes iType, DWORD dwSize) { return ((DWORD)iType | dwSize); }
		inline static DWORD GetBlockSize (BLOCKHEADER dwHeader) { return (dwHeader & blockLenMask); }

		CBuffer m_Code;
	};

//	CHexeFunction --------------------------------------------------------------

class CHexePrimitive : public TExternalDatum<CHexePrimitive>
	{
	public:
		static void Create (CDatum::ECallType iPrimitive, CDatum *retdFunc);
		static const CString &StaticGetTypename (void);

		//	IComplexDatum
		virtual bool CanInvoke (void) const override { return true; }
		virtual CDatum::ECallType GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const override { return m_iPrimitive; }

	private:
		CDatum::ECallType m_iPrimitive;
	};

class CHexeFunction : public TExternalDatum<CHexeFunction>
	{
	public:
		CHexeFunction (void) { }

		static void Create (CDatum dCodeBank, int iCodeOffset, CDatum dGlobalEnv, CDatum dLocalEnv, CDatum *retdFunc);
		static const CString &StaticGetTypename (void);

		DWORD *GetCode (CDatum *retdCodeBank);
		inline CDatum GetGlobalEnv (void) { return m_dGlobalEnv; }
		inline CHexeGlobalEnvironment *GetGlobalEnvPointer (void) { return m_pGlobalEnv; }
		inline CDatum GetLocalEnv (void) { return m_dLocalEnv; }

		//	IComplexDatum
		virtual bool CanInvoke (void) const override { return true; }
		virtual CDatum::ECallType GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const override;
		virtual int GetCount (void) const override;
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual CString GetKey (int iIndex) const override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override;

	protected:
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override { m_dHexeCode.Mark(); m_dGlobalEnv.Mark(); m_dLocalEnv.Mark(); }
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:
		CDatum m_dHexeCode;
		int m_iOffset = 0;

		CDatum m_dGlobalEnv;
		CHexeGlobalEnvironment *m_pGlobalEnv = NULL;

		CDatum m_dLocalEnv;
	};

class CHexeLibraryFunction : public TExternalDatum<CHexeLibraryFunction>
	{
	public:

		static void Create (const SLibraryFuncDef &Def, CDatum *retdFunc);
		static const CString &StaticGetTypename (void);

		//	IComplexDatum

		virtual bool CanInvoke (void) const override { return true; }
		virtual CDatum::ECallType GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const override { return CDatum::ECallType::Library; }
		virtual CDatum::InvokeResult Invoke (IInvokeCtx *pCtx, CDatum dLocalEnv, DWORD dwExecutionRights, CDatum *retdResult) override;
		virtual CDatum::InvokeResult InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, CDatum *retdResult) override;

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const { return (size_t)m_sName.GetLength() + 2; }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:

		static CDatum::InvokeResult HandleSpecialResult (CDatum *retdResult);

		CString m_sName;
		FHexeLibraryFunc m_pfFunc;
		DWORD m_dwData;
		CString m_sArgList;
		CString m_sHelpLine;
		DWORD m_dwExecutionRights;
	};

//	CHexeGlobalEnvironment -----------------------------------------------------

class CHexeGlobalEnvironment : public TExternalDatum<CHexeGlobalEnvironment>
	{
	public:
		CHexeGlobalEnvironment (void) { }
		CHexeGlobalEnvironment (CHexeGlobalEnvironment *pSrc) { m_Env = pSrc->m_Env; m_ServiceSecurity = pSrc->m_ServiceSecurity; }

		static const CString &StaticGetTypename (void);

		inline bool Find (const CString &sIdentifier, CDatum *retdValue) { return m_Env.Find(sIdentifier, retdValue); }
		inline bool FindPos (const CString &sIdentifier, int *retiPos) { return m_Env.FindPos(sIdentifier, retiPos); }
		inline CDatum GetAt (int iIndex) { return m_Env[iIndex]; }
		inline void GetServiceSecurity (CHexeSecurityCtx *retCtx) { m_ServiceSecurity.GetServiceSecurity(retCtx); }
		inline void SetAt (int iIndex, CDatum dValue) { m_Env[iIndex] = dValue; }
		inline void SetAt (const CString &sIdentifier, CDatum dValue, bool *retbNew = NULL) { m_Env.SetAt(sIdentifier, dValue, retbNew); }
		inline void SetServiceSecurity (const CHexeSecurityCtx &Ctx) { m_ServiceSecurity.SetServiceSecurity(Ctx); }

		//	IComplexDatum
		virtual int GetCount (void) const override { return m_Env.GetCount(); }
		virtual CDatum GetElement (int iIndex) const override { return (iIndex < m_Env.GetCount() ? m_Env[iIndex] : CDatum()); }
		virtual CDatum GetElement (const CString &sKey) const override { CDatum *pFound = m_Env.GetAt(sKey); return (pFound ? *pFound : CDatum()); }
		virtual CString GetKey (int iIndex) const override { return m_Env.GetKey(iIndex); }
		virtual bool IsArray (void) const override { return true; }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { SetAt(sKey, dDatum); }

	protected:
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:
		TSortMap<CString, CDatum> m_Env;			//	Global environment
		CHexeSecurityCtx m_ServiceSecurity;			//	Service that defined this environment
	};

//	CHexeLocalEnvironment ------------------------------------------------------

class CHexeLocalEnvironment : public TExternalDatum<CHexeLocalEnvironment>
	{
	public:
		static const CString &StaticGetTypename (void);

		bool FindArgument (const CString &sArg, int *retiLevel, int *retiIndex);
		CDatum GetArgument (int iLevel, int iIndex);
		inline CDatum GetParentEnv (void) { return m_dParentEnv; }
		inline void ResetNextArg (void) { m_iNextArg = 0; }
		void SetArgumentKey (int iLevel, int iIndex, const CString &sKey);
		void SetArgumentValue (int iLevel, int iIndex, CDatum dValue);
		inline void SetElement (int iIndex, CDatum dValue) { m_Array[iIndex].dValue = dValue; }
		inline void SetNextArgKey (const CString &sKey) { SetArgumentKey(0, m_iNextArg++, sKey); }
		inline void SetParentEnv (CDatum dParentEnv) { m_dParentEnv = dParentEnv; }

		//	IComplexDatum
		virtual int GetCount (void) const override { return m_Array.GetCount(); }
		virtual CDatum GetElement (int iIndex) const override { return (iIndex < m_Array.GetCount() ? m_Array[iIndex].dValue : CDatum()); }
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual CString GetKey (int iIndex) const override { return m_Array[iIndex].sArg; }
		virtual bool IsArray (void) const override { return true; }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override;

	protected:
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:
		struct SEntry
			{
			CString sArg;
			CDatum dValue;
			};

		TArray<SEntry> m_Array;
		CDatum m_dParentEnv;
		int m_iNextArg;
	};

//	CLispParser ----------------------------------------------------------------

class CLispParser
	{
	public:
		enum ETokens
			{
			tkEOF,					//	End of file
			tkError,				//	Parsing error (retdDatum is error message)

			tkOpenParen,			//	'('
			tkCloseParen,			//	')'
			tkOpenBrace,			//	'{'
			tkCloseBrace,			//	'}'
			tkQuote,				//	single-quote
			tkColon,				//	':'

			tkIdentifier,			//	A valid HexeLisp identifier

			tkStringDatum,			//	A literal string
			tkIntegerDatum,			//	A literal integer
			tkIPIntegerDatum,		//	An infinite precision integer
			tkFloatDatum,			//	A double
			tkOtherDatum			//	Another literal (e.g., datetime)
			};

		CLispParser (void) : m_pStream(NULL) { }

		inline CString ComposeError (const CString &sError) { return m_pStream->ComposeError(sError); }
		inline ETokens GetToken (CDatum *retdDatum = NULL) const { if (retdDatum) *retdDatum = m_dToken; return m_iToken; }
		inline bool Init (CCharStream *pStream) { m_pStream = pStream; return true; }
		ETokens ParseToken (CDatum *retdDatum = NULL);

	private:
		bool ParseCommentCPP (void);
		bool ParseCommentSemi (void);
		ETokens ParseIdentifier (CDatum *retdDatum);
		ETokens ParseStringLiteral (CDatum *retdDatum);

		CCharStream *m_pStream;
		ETokens m_iToken;
		CDatum m_dToken;
	};

//	CLispCompiler --------------------------------------------------------------

class CLispCompiler
	{
	public:
		bool CompileTerm (CCharStream *pStream, CHexeCodeIntermediate &Code, int *retiBlock, CString *retsError);

	private:
		bool CompileExpression (int iBlock);
		bool CompileFunctionCall (int iBlock);
		bool CompileIdentifier (int iBlock);
		bool CompileLiteralExpression (int iBlock);
		bool CompileLiteralStruct (int iBlock);
		bool CompileLocalDef (int iBlock, int iVarPos, const CString &sVar = NULL_STR);
		bool CompileLocalDefExpression (int iBlock, int iVarPos, const CString &sVar);
		bool CompileSpecialFormAnd (int iBlock);
		bool CompileSpecialFormApply (int iBlock);
		bool CompileSpecialFormBlock (int iBlock);
		bool CompileSpecialFormDefine (int iBlock);
		bool CompileSpecialFormEnum (int iBlock);
		bool CompileSpecialFormError (int iBlock);
		bool CompileSpecialFormIf (int iBlock);
		bool CompileSpecialFormInvoke (int iBlock);
		bool CompileSpecialFormLambda (int iBlock);
		bool CompileSpecialFormListOp (int iBlock, DWORD dwOpCode, int iMinArgs = 0, int iMaxArgs = -1);
		bool CompileSpecialFormMap (int iBlock);
		bool CompileSpecialFormNot (int iBlock);
		bool CompileSpecialFormOr (int iBlock);
		bool CompileSpecialFormQuote (int iBlock);
		bool CompileSpecialFormSetAt (int iBlock);
		bool CompileSpecialFormSetBang (int iBlock);
		bool CompileSpecialFormSwitch (int iBlock);
		bool CompileSpecialFormWhile (int iBlock);
		bool CompileStruct (int iBlock);

		void EnterLocalEnvironment (void);
		void ExitLocalEnvironment (void);

		bool IsNilSymbol (CDatum dDatum);
		bool IsTrueSymbol (CDatum dDatum);

		CLispParser m_Parser;
		CHexeCodeIntermediate *m_pCode;
		CString m_sError;

		CDatum m_dCurrentEnv;
		CHexeLocalEnvironment *m_pCurrentEnv;
	};

//	CLambdaParseExtension ------------------------------------------------------

class CLambdaParseExtension : public IAEONParseExtension
	{
	public:
		CLambdaParseExtension (CHexeProcess &Process) : m_Process(Process) { }

		//	IAEONParseExtension virtuals
		virtual bool ParseAEONArray (CCharStream &Stream, CDatum *retDatum) override;

	private:
		bool ParseHexeLispDef (CCharStream *pStream, CDatum *retdDatum, CString *retsError);

		CHexeProcess &m_Process;
	};

//	Libraries

extern SLibraryFuncDef g_CoreLibraryDef[];
extern const int g_iCoreLibraryDefCount;

extern SLibraryFuncDef g_CoreVectorLibraryDef[];
extern const int g_iCoreVectorLibraryDefCount;
