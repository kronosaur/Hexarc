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
		static constexpr int DEFAULT_SIZE = 4000;

		CHexeStack (int iSize = DEFAULT_SIZE) { m_Stack.InsertEmpty(DEFAULT_SIZE); }

		void DeleteAll (void) { m_Stack.DeleteAll(); m_iTop = -1; }
		CDatum Get (void) const { return m_Stack[m_iTop]; }
		CDatum Get (int iIndex) const { return m_Stack[m_iTop - iIndex]; }

		int GetCount () const { return m_iTop + 1; }
		void Mark (void);
		CDatum Pop (void) { return m_Stack[m_iTop--]; }
		void Pop (int iCount) { m_iTop -= iCount; }
		void Push (CDatum dData) { m_Stack[++m_iTop] = dData; }

		CDatum SafeGet (void) const { return (m_iTop == -1 ? CDatum() : m_Stack[m_iTop]); }
		CDatum SafeGet (int iIndex) const;
		CDatum SafePop (void) { return (m_iTop == -1 ? CDatum() : m_Stack[m_iTop--]); }
		void SafePop (int iCount) { if (m_iTop + 1 >= iCount) m_iTop -= iCount; else m_iTop = -1; }
		void SafePush (CDatum dData);

	private:
		TArray<CDatum> m_Stack;
		int m_iTop = -1;						//	Index to item at top of stack
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

