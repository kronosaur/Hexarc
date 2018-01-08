//	HexeProcessImpl.h
//
//	Hexe header file
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Hexe.h

#pragma once

//	CHexeStack -----------------------------------------------------------------

class CHexeStack
	{
	public:
		CHexeStack (void) : m_iTop(-1) { }

		void DeleteAll (void) { m_Stack.DeleteAll(); m_iTop = -1; }
		inline CDatum Get (void) { return (m_iTop == -1 ? CDatum() : m_Stack[m_iTop]); }
		void Mark (void);
		inline CDatum Pop (void) { return (m_iTop == -1 ? CDatum() : m_Stack[m_iTop--]); }
		inline void Pop (int iCount) { if (m_iTop + 1 >= iCount) m_iTop -= iCount; else m_iTop = -1; }
		void Push (CDatum dData);

	private:
		TArray<CDatum> m_Stack;
		int m_iTop;						//	Index to item at top of stack
	};

//	CHexeCallStack -------------------------------------------------------------

class CHexeCallStack
	{
	public:
		inline void DeleteAll (void) { m_Stack.DeleteAll(); }
		void Mark (void);
		void Restore (CDatum *retdExpression, CDatum *retdCodeBank, DWORD **retpIP, CHexeCode **retpCodeBank);
		void Save (CDatum dExpression, CDatum dCodeBank, DWORD *pIP);

	private:
		struct SExecuteCtx
			{
			CDatum dExpression;
			CDatum dCodeBank;
			DWORD *pIP;
			};

		TArray<SExecuteCtx> m_Stack;
	};

//	CHexeEnvStack --------------------------------------------------------------

class CHexeEnvStack
	{
	public:
		inline void DeleteAll (void) { m_Stack.DeleteAll(); }
		void Mark (void);
		void Restore (CDatum *retdGlobalEnv, CHexeGlobalEnvironment **retpGlobalEnv, CDatum *retdLocalEnv, CHexeLocalEnvironment **retpLocalEnv);
		void Save (CDatum dGlobalEnv, CHexeGlobalEnvironment *pGlobalEnv, CDatum dLocalEnv, CHexeLocalEnvironment *pLocalEnv);
	
	private:
		struct SEnvCtx
			{
			CDatum dGlobalEnv;
			CHexeGlobalEnvironment *pGlobalEnv;

			CDatum dLocalEnv;
			CHexeLocalEnvironment *pLocalEnv;
			};

		TArray<SEnvCtx> m_Stack;
	};

