//	HexeImpl.h
//
//	Hexe header file
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#pragma once

enum EOperandTypes
	{
	operandNone,				//	No operand for opcode
	operandIntShort,			//	Operand is an integer encoded in the lower 24 bits of the opcode
	operandIntShort2,			//	Operand is two ints, an 8-bit arg and a 16-bit value, encoded in the lower 24 bits of the opcode
	operandCodeOffset,			//	Operand is a block offset pointing to code
	operandStringOffset,		//	Operand is a block offset pointing to a string
	operandDatumOffset,			//	Operand is a block offset pointing to a serialized datum
	operandInt,					//	Operand is an integer following the opcode
	operandLibCall,				//	Operand is a count of args followed by a function ID
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
		SOpCodeInfo *GetInfo (DWORD dwOpCode) { return m_Info[dwOpCode >> 24]; }

	private:
		SOpCodeInfo *m_Info[opCodeCount];
	};

inline DWORD GetOpCode (DWORD dwCode) { return (dwCode & 0xff000000); }
inline DWORD GetOperand (DWORD dwCode) { return (dwCode & 0x00ffffff); }
inline DWORD MakeOpCode (DWORD dwOpCode, DWORD dwOperand) { return (dwOpCode | dwOperand); }
inline DWORD MakeOperand2 (DWORD dwArg, DWORD dwValue) { return ((dwArg & 0xff) << 16) | (dwValue & 0xffff); }
inline DWORD GetOperand2Arg (DWORD dwOperand) { return (dwOperand >> 16) & 0xff; }
inline DWORD GetOperand2Value (DWORD dwOperand) { return (dwOperand & 0xffff); }

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
		static void CreateFunctionCall (CStringView sFunction, int iArgCount, CDatum& retdEntryPoint);
		static void CreateInvokeCall (const TArray<CDatum> &Args, CDatum *retdEntryPoint);
		static const CString &StaticGetTypename (void);

		DWORD *GetCode (int iOffset) { return (DWORD *)(m_Code.GetPointer() + iOffset); }
		int GetDataBlockCount () const { return m_DataOffsets.GetCount(); }
		CDatum GetDatum (int iOffset) const;
		CDatum GetDatumFromID (int iID) const { if (iID < 0 || iID >= m_DataCache.GetCount()) throw CException(errFail); return m_DataCache[iID]; }
		static int GetOperandInt (DWORD dwCode);
		CString GetString (int iOffset) const { return CString(m_Code.GetPointer() + iOffset); }
		CString GetStringLiteral (int iOffset) const { return CString(m_Code.GetPointer() + iOffset, -1, true); }

	protected:

		//	IComplexDatum

		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:
		enum class EBlockType
			{
			Unknown =			0x00000000,

			Code =				0x10000000,
			String =			0x20000000,
			Datum =				0x30000000,
			};

		typedef DWORD BLOCKHEADER;

		static constexpr DWORD BLOCK_TYPE_MASK =	0xf0000000;
		static constexpr DWORD BLOCK_LEN_MASK =		0x0fffffff;

		static constexpr DWORD VERSION =		2;
		static constexpr DWORD VERSION_NEW =	0xffffffff;

		static BLOCKHEADER ComposeHeader (EBlockType iType, DWORD dwSize) { return ((DWORD)iType | dwSize); }
		CDatum CreateDatum (int iID) const;
		static DWORD GetBlockSize (BLOCKHEADER dwHeader) { return (dwHeader & BLOCK_LEN_MASK); }
		static EBlockType GetBlockType (BLOCKHEADER dwHeader) { return (EBlockType)(dwHeader & BLOCK_TYPE_MASK); }

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override;

		CBuffer m_Code;
		TArray<int> m_CodeOffsets;
		TArray<int> m_DataOffsets;

		TArray<CDatum> m_DataCache;
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
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override { CDatum((int)m_iPrimitive).SerializeAEON(Stream, Serialized); }

	private:
		CDatum::ECallType m_iPrimitive;
	};

class CHexeFunction : public TExternalDatum<CHexeFunction>
	{
	public:

		CHexeFunction (void) : m_dDatatype(CAEONTypes::Get(IDatatype::FUNCTION)) { }

		static CDatum Create (CDatum dCodeBank, int iCodeOffset, CDatum dGlobalEnv, CDatum dLocalEnv, CDatum dAttribs, CDatum dDatatype);
		static void Create (CDatum dCodeBank, int iCodeOffset, CDatum dGlobalEnv, CDatum dLocalEnv, CDatum *retdFunc)
			{ *retdFunc = Create(dCodeBank, iCodeOffset, dGlobalEnv, dLocalEnv, CDatum(), CAEONTypes::Get(IDatatype::FUNCTION)); }
		static const CString &StaticGetTypename (void);

		CDatum GetAttribs () const { return m_dAttribs; }
		DWORD *GetCode (CDatum *retdCodeBank);
		CDatum GetGlobalEnv (void) { return m_dGlobalEnv; }
		CHexeGlobalEnvironment *GetGlobalEnvPointer (void) { return m_pGlobalEnv; }
		CDatum GetLocalEnv (void) { return m_dLocalEnv; }

		//	IComplexDatum

		virtual void CacheInvokeResult (CHexeLocalEnvironment& LocalEnv, CDatum dResult) override;
		virtual bool CanInvoke (void) const override { return true; }
		virtual bool Contains (CDatum dValue) const override;
		virtual CDatum::ECallType GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const override;
		virtual int GetCount (void) const override;
		virtual CDatum GetDatatype () const override { return m_dDatatype; }
		virtual CDatum GetElement (int iIndex) const override;
		virtual CDatum GetElement (const CString &sKey) const override;
		virtual CString GetKey (int iIndex) const override;
		virtual CDatum::InvokeResult Invoke (IInvokeCtx *pCtx, CHexeLocalEnvironment& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult) override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override;

	protected:

		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override;

		CDatum m_dHexeCode;
		int m_iOffset = 0;
		CDatum m_dDatatype;
		CDatum m_dAttribs;

		CDatum m_dGlobalEnv;
		CHexeGlobalEnvironment *m_pGlobalEnv = NULL;

		CDatum m_dLocalEnv;

		bool m_bCached = false;
		TSortMap<CString, CDatum> m_Cache;
	};

class CHexeLibraryFunction : public TExternalDatum<CHexeLibraryFunction>
	{
	public:

		static void Create (const SLibraryFuncDef &Def, CDatum *retdFunc);
		static const CString &StaticGetTypename (void);

		void SetArgList (CString &&sValue) { m_sArgList = std::move(sValue); }
		void SetExecFlags (DWORD dwValue) { m_dwExecFlags = dwValue; }
		void SetFunction (FHexeLibraryFunc pfFunc, DWORD dwData = 0) { m_pfFunc = pfFunc; m_dwData = dwData; }
		void SetHelp (CString &&sValue) { m_sHelpLine = std::move(sValue); }
		void SetName (CString &&sValue) { m_sName = std::move(sValue); }

		//	IComplexDatum

		virtual CString AsString () const override { return strPattern("[%s]: %s", GetTypename(), m_sName); }
		virtual bool CanInvoke (void) const override { return true; }
		virtual CDatum::ECallType GetCallInfo (CDatum *retdCodeBank, DWORD **retpIP) const override { return CDatum::ECallType::Library; }
		virtual CDatum GetDatatype () const override { return (m_dDatatype.IsNil() ? CAEONTypeSystem::GetCoreType(IDatatype::FUNCTION) : m_dDatatype); }
		virtual CDatum::InvokeResult Invoke (IInvokeCtx *pCtx, CHexeLocalEnvironment& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult) override;
		virtual CDatum::InvokeResult InvokeContinues (IInvokeCtx *pCtx, CDatum dContext, CDatum dResult, SAEONInvokeResult& retResult) override;
		virtual CDatum::InvokeResult InvokeLibrary (IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, DWORD dwExecutionRights, SAEONInvokeResult& retResult) override;

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const { return (size_t)m_sName.GetLength() + 2; }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override { CDatum(m_sName).SerializeAEON(Stream, Serialized); }

	protected:

		virtual void OnMarked (void) override { m_dDatatype.Mark(); }

	private:

		static CDatum::InvokeResult HandleSpecialResult (SAEONInvokeResult& retResult);

		CString m_sName;
		FHexeLibraryFunc m_pfFunc = NULL;
		DWORD m_dwData = 0;
		CDatum m_dDatatype;

		CString m_sArgList;
		CString m_sHelpLine;
		DWORD m_dwExecFlags = 0;
	};

//	CHexeGlobalEnvironment -----------------------------------------------------

class CHexeGlobalEnvironment : public TExternalDatum<CHexeGlobalEnvironment>
	{
	public:

		CHexeGlobalEnvironment () { }

		static const CString &StaticGetTypename (void);

		bool AddSymbol (CStringView sIdentifier, CDatum dValue, DWORD* retdwID = NULL);
		bool FindSymbol (CStringView sIdentifier, DWORD* retdwID = NULL) { return m_Index.Find(sIdentifier, retdwID); }
		CDatum GetAt (DWORD dwID) { if (dwID >= (DWORD)m_Data.GetCount()) throw CException(errFail); return m_Data[(int)dwID]; }
		void GetServiceSecurity (CHexeSecurityCtx *retCtx) { m_ServiceSecurity.GetServiceSecurity(retCtx); }
		void SetAt (DWORD dwID, CDatum dValue) { if (dwID >= (DWORD)m_Data.GetCount()) throw CException(errFail); m_Data[(int)dwID] = dValue; }
		void SetAt (CStringView sSymbol, CDatum dValue);
		void SetServiceSecurity (const CHexeSecurityCtx &Ctx) { m_ServiceSecurity.SetServiceSecurity(Ctx); }

		//	IComplexDatum

		virtual int GetCount (void) const override { return m_Data.GetCount(); }
		virtual CDatum GetElement (int iIndex) const override { return (iIndex >= 0 && iIndex < m_Index.GetCount() ? m_Data[m_Index[iIndex]] : CDatum()); }
		virtual CDatum GetElement (const CString &sKey) const override { DWORD* pFound = m_Index.GetAt(sKey); return (pFound ? m_Data[*pFound] : CDatum()); }
		virtual CString GetKey (int iIndex) const override { return (iIndex >= 0 && iIndex < m_Index.GetCount() ? m_Index.GetKey(iIndex) : NULL_STR); }
		virtual bool IsArray (void) const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { SetAt(sKey, dDatum); }

	protected:

		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override { }
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override { }

		TArray<CDatum> m_Data;						//	Data values by ID
		TSortMap<CString, DWORD> m_Index;			//	Index of symbol to ID
		CHexeSecurityCtx m_ServiceSecurity;			//	Service that defined this environment
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

		CString ComposeError (const CString &sError) { return m_pStream->ComposeError(sError); }
		ETokens GetToken (CDatum *retdDatum = NULL) const { if (retdDatum) *retdDatum = m_dToken; return m_iToken; }
		bool Init (CCharStream *pStream) { m_pStream = pStream; return true; }
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
