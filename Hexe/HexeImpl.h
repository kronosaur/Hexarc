//	HexeImpl.h
//
//	Hexe header file
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#pragma once

typedef DWORD OPCODE;

//	OpCodes --------------------------------------------------------------------
//
//	A code block consists of an array of DWORDs. Each DWORD is either an opcode
//	or data.
//
//	An opcode uses the high-order byte as the opcode. The remaining bits (24)
//	are used as data, if necessary.
//
//	Data DWORDs are either a CDatum or an opcode-specific type (e.g., a jump
//	offset).

enum EOpCodes
	{
	opNoOp =				0x00000000,		//	No operation

	opDefine =				0x01000000,		//	Defines variable in current environment
	opPushIntShort =		0x02000000,		//	Pushes operand as int
	opPushStr =				0x03000000,		//	Pushes operand as string block
	opPushStrNull =			0x04000000,		//	Pushes CDatum("")
	opPushGlobal =			0x05000000,		//	Pushes the global variable
	opMakeFunc =			0x06000000,
	opEnterEnv =			0x07000000,
	opDefineArg =			0x08000000,
	opExitEnv =				0x09000000,
	opReturn =				0x0a000000,
	opPushLocal =			0x0b000000,
	opMakeEnv =				0x0c000000,
	opCall =				0x0d000000,
	opPushInt =				0x0e000000,		//	Pushes operand as int
	opAdd =					0x0f000000,
	opDivide =				0x10000000,
	opMultiply =			0x11000000,
	opSubtract =			0x12000000,
	opJump =				0x13000000,
	opJumpIfNil =			0x14000000,
	opPushNil =				0x15000000,		//	Pushes CDatum()
	opPushTrue =			0x16000000,		//	Pushes CDatum(CDatum::constTrue)
	opIsEqual =				0x17000000,
	opIsLess =				0x18000000,
	opIsGreater =			0x19000000,
	opIsLessOrEqual =		0x1a000000,
	opIsGreaterOrEqual =	0x1b000000,
	opMakeArray =			0x1c000000,
	opPop =					0x1d000000,
	opHexarcMsg =			0x1e000000,
	opMakeApplyEnv =		0x1f000000,
	opMakePrimitive =		0x20000000,
	opPushDatum =			0x21000000,
	opError =				0x22000000,
	opSetLocal =			0x23000000,
	opSetGlobal =			0x24000000,
	opPopLocal =			0x25000000,
	opMakeBlockEnv =		0x26000000,
	opNot =					0x27000000,
	opSetLocalItem =		0x28000000,
	opSetGlobalItem =		0x29000000,
	opMakeStruct =			0x2a000000,
	opPushLocalLength =		0x2b000000,
	opPushLocalItem =		0x2c000000,
	opAppendLocalItem =		0x2d000000,
	opIncLocalInt =			0x2e000000,
	opMakeFlagsFromArray =	0x2f000000,
	opMapResult =			0x30000000,
	opJumpIfNilNoPop =		0x31000000,
	opJumpIfNotNilNoPop =	0x32000000,
	opIsNotEqual =			0x33000000,

	opHalt =				0xff000000,

	opCodeCount =			256,
	};

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

//	CHexeCodeIntermediate ------------------------------------------------------

class CHexeCodeIntermediate
	{
	public:
		int CreateCodeBlock (void);
		int CreateDatumBlock (CDatum dDatum);
		inline int GetCodeBlockPos (int iBlock) { return m_CodeBlocks[iBlock].GetPos(); }
		void RewriteShortOpCode (int iBlock, int iPos, OPCODE opCode, DWORD dwOperand = 0);
		void WriteShortOpCode (int iBlock, OPCODE opCode, DWORD dwOperand = 0);
		void WriteLongOpCode (int iBlock, OPCODE opCode, DWORD dwData);
		void WriteLongOpCode (int iBlock, OPCODE opCode, CDatum dData);

		inline const CBuffer &GetCodeBlock (int iIndex) const { return m_CodeBlocks[iIndex]; }
		inline int GetCodeBlockCount (void) const { return m_CodeBlocks.GetCount(); }
		inline CDatum GetDatumBlock (int iIndex) const { return m_DatumBlocks[iIndex]; }
		inline int GetDatumBlockCount (void) const { return m_DatumBlocks.GetCount(); }

	private:
		TArray<CBuffer> m_CodeBlocks;
		TArray<CDatum> m_DatumBlocks;
	};

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
		virtual bool OnDeserialize (CDatum::ESerializationFormats iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnSerialize (CDatum::ESerializationFormats iFormat, IByteStream &Stream) const override;

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
		static void Create (CDatum::ECallTypes iPrimitive, CDatum *retdFunc);
		static const CString &StaticGetTypename (void);

		//	IComplexDatum
		virtual bool CanInvoke (void) const override { return true; }
		virtual CDatum::ECallTypes GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const override { return m_iPrimitive; }

	private:
		CDatum::ECallTypes m_iPrimitive;
	};

class CHexeFunction : public TExternalDatum<CHexeFunction>
	{
	public:
		CHexeFunction (void) : m_pGlobalEnv(NULL) { }

		static void Create (CDatum dCodeBank, int iCodeOffset, CDatum dGlobalEnv, CDatum dLocalEnv, CDatum *retdFunc);
		static const CString &StaticGetTypename (void);

		DWORD *GetCode (CDatum *retdCodeBank);
		inline CDatum GetGlobalEnv (void) { return m_dGlobalEnv; }
		inline CHexeGlobalEnvironment *GetGlobalEnvPointer (void) { return m_pGlobalEnv; }
		inline CDatum GetLocalEnv (void) { return m_dLocalEnv; }

		//	IComplexDatum
		virtual bool CanInvoke (void) const override { return true; }
		virtual CDatum::ECallTypes GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const override;
		virtual int GetCount (void) const override;
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual CString GetKey (int iIndex) const override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override;

	protected:
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override { m_dHexeCode.Mark(); m_dGlobalEnv.Mark(); m_dLocalEnv.Mark(); }

	private:
		CDatum m_dHexeCode;
		int m_iOffset;

		CDatum m_dGlobalEnv;
		CHexeGlobalEnvironment *m_pGlobalEnv;

		CDatum m_dLocalEnv;
	};

class CHexeLibraryFunction : public TExternalDatum<CHexeLibraryFunction>
	{
	public:

		static void Create (const SLibraryFuncDef &Def, CDatum *retdFunc);
		static const CString &StaticGetTypename (void);

		//	IComplexDatum
		virtual bool CanInvoke (void) const override { return true; }
		virtual CDatum::ECallTypes GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const override { return CDatum::funcLibrary; }
		virtual CDatum::InvokeResult Invoke (IInvokeCtx *pCtx, CDatum dLocalEnv, DWORD dwExecutionRights, CDatum *retdResult) override;
		virtual CDatum::InvokeResult InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, CDatum *retdResult) override;

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
		inline void SetAt (const CString &sIdentifier, CDatum dValue) { m_Env.SetAt(sIdentifier, dValue); }
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
		virtual void OnSerialize (CDatum::ESerializationFormats iFormat, CComplexStruct *pStruct) const override;

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
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::ESerializationFormats iFormat, CComplexStruct *pStruct) const override;

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
