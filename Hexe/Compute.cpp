//	Compute.cpp
//
//	Compute
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

static constexpr int MAX_ARRAY_SIZE =					10000000;
static constexpr DWORDLONG STOP_CHECK_COUNT =			0x4000;
static constexpr DWORDLONG COMPUTES_PER_MILLISECOND =	100;

DECLARE_CONST_STRING(FIELD_LENGTH,						"length");
DECLARE_CONST_STRING(FIELD_MSG,							"msg");
DECLARE_CONST_STRING(FIELD_PAYLOAD,						"payload");

DECLARE_CONST_STRING(TYPE_EVENT_HANDLER_CALL,			"eventHandlerCall");

DECLARE_CONST_STRING(TYPENAME_HEXE_FUNCTION,			"hexeFunction");

DECLARE_CONST_STRING(ERR_COLON_EXPECTED,				"':' expected: %s.");
DECLARE_CONST_STRING(ERR_DIVISION_BY_ZERO,				"Divide by zero error.");
DECLARE_CONST_STRING(ERR_INVALID_KEY,					"Invalid key: %s.");
DECLARE_CONST_STRING(ERR_INVALID_OP_CODE,				"Invalid opcode.");
DECLARE_CONST_STRING(ERR_INVALID_PRIMITIVE_SUB,			"Invalid primitive subroutine: not a function.");
DECLARE_CONST_STRING(ERR_NOT_A_FUNCTION,				"Not a function: %s.");
DECLARE_CONST_STRING(ERR_OUT_OF_MEMORY,					"Out of memory.");
DECLARE_CONST_STRING(ERR_NOT_ARRAY_OR_STRUCT,			"Unable to set item: not an array or structure.");
DECLARE_CONST_STRING(ERR_UNBOUND_VARIABLE,				"Undefined identifier: %s.");
DECLARE_CONST_STRING(ERR_EXECUTION_TOOK_TOO_LONG,		"Execution took too long.");
DECLARE_CONST_STRING(ERR_DUPLICATE_VARIABLE,			"Duplicate definition: %s.");
DECLARE_CONST_STRING(ERR_INVALID_OPERAND_COUNT,			"Invalid operand count.");
DECLARE_CONST_STRING(ERR_CANT_SET_FUNCTION_MEMBER,		"Function objects are read-only.");
DECLARE_CONST_STRING(ERR_UNSUPPORTED_OP,				"Unsupported opcode.");

CHexeProcess::ERun CHexeProcess::Execute (CDatum *retResult)

//	Execute
//
//	Continues execution

	{
	int i;
	CDatum dValue;
	int iCount;
	bool bCondition;
	CComplexArray *pArray;
	CComplexStruct *pStruct;

	//	Set abort time

	if (m_dwMaxExecutionTime)
		m_dwAbortTime = ::sysGetTickCount64() + m_dwMaxExecutionTime;
	else
		m_dwAbortTime = 0;

	//	Run

	while (true)
		{
		switch (GetOpCode(*m_pIP))
			{
			case opNoOp:
				m_pIP++;
				break;

			case opDefine:
				{
				bool bNew;
				//	NOTE: We leave the value of the definition on the stack
				m_pCurGlobalEnv->SetAt(m_pCodeBank->GetString(GetOperand(*m_pIP)), m_Stack.Get(), &bNew);
				if (!bNew)
					return RuntimeError(strPattern(ERR_DUPLICATE_VARIABLE, m_pCodeBank->GetString(GetOperand(*m_pIP))), *retResult);

				m_pIP++;
				break;
				}

			case opPushNil:
				m_Stack.Push(CDatum());
				m_pIP++;
				break;

			case opPushTrue:
				m_Stack.Push(CDatum(true));
				m_pIP++;
				break;

			case opPushNaN:
				m_Stack.Push(CDatum::CreateNaN());
				m_pIP++;
				break;

			case opPushIntShort:
				m_Stack.Push(CDatum(CHexeCode::GetOperandInt(*m_pIP)));
				m_pIP++;
				break;

			case opPushInt:
				m_pIP++;
				m_Stack.Push(CDatum((int)*m_pIP));
				m_pIP++;
				break;

			case opPushDatum:
				m_Stack.Push(m_pCodeBank->GetDatum(GetOperand(*m_pIP)));
				m_pIP++;
				break;

			case opPushStr:
				m_Stack.Push(m_pCodeBank->GetString(GetOperand(*m_pIP)));
				m_pIP++;
				break;

			case opPushStrNull:
				m_Stack.Push(CDatum(NULL_STR));
				m_pIP++;
				break;

			case opPushGlobal:
				if (!m_pCurGlobalEnv->Find(m_pCodeBank->GetStringLiteral(GetOperand(*m_pIP)), &dValue))
					return RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, m_pCodeBank->GetStringLiteral(GetOperand(*m_pIP))), *retResult);

				m_Stack.Push(dValue);
				m_pIP++;
				break;

			case opSetGlobal:
				{
				int iPos;
				if (!m_pCurGlobalEnv->FindPos(m_pCodeBank->GetStringLiteral(GetOperand(*m_pIP)), &iPos))
					return RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, m_pCodeBank->GetStringLiteral(GetOperand(*m_pIP))), *retResult);

				m_pCurGlobalEnv->SetAt(iPos, m_Stack.Get());
				m_pIP++;
				break;
				}

			case opSetGlobalItem:
				{
				int iPos;
				if (!m_pCurGlobalEnv->FindPos(m_pCodeBank->GetStringLiteral(GetOperand(*m_pIP)), &iPos))
					return RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, m_pCodeBank->GetStringLiteral(GetOperand(*m_pIP))), *retResult);

				CDatum dValue = m_Stack.Pop();
				CDatum dKey = m_Stack.Pop();
				CDatum dResult;
				if (!ExecuteSetAt(m_pCurGlobalEnv->GetAt(iPos), dKey, dValue, &dResult))
					{
					*retResult = dResult;
					return ERun::Error;
					}

				m_pCurGlobalEnv->SetAt(iPos, dResult);
				m_Stack.Push(dResult);

				m_pIP++;
				break;
				}

			case opJump:
				m_pIP += CHexeCode::GetOperandInt(*m_pIP);
				break;

			case opJumpIfNil:
				if (m_Stack.Pop().IsNil())
					m_pIP += CHexeCode::GetOperandInt(*m_pIP);
				else
					m_pIP++;
				break;

			case opJumpIfNilNoPop:
				if (m_Stack.Get().IsNil())
					m_pIP += CHexeCode::GetOperandInt(*m_pIP);
				else
					m_pIP++;
				break;

			case opJumpIfNotNilNoPop:
				if (!m_Stack.Get().IsNil())
					m_pIP += CHexeCode::GetOperandInt(*m_pIP);
				else
					m_pIP++;
				break;

			case opCompareStep:
				{
				//	The stack has (in reverse order)
				//
				//	0: step value
				//	1: to value
				//	2: loop index.
				//
				//	If step value is positive, then we check to see if the loop 
				//	value is less that or equal to to. Otherwise, we check to
				//	see if it is greater than or equal to to. We push the result
				//	(either true or nil) on the stack and leave the other values
				//	alone.

				CNumberValue Step(m_Stack.Get());
				CNumberValue To(m_Stack.Get(1));
				CNumberValue I(m_Stack.Get(2));
				int iCompare = I.Compare(To);

				if (Step.IsNegative())
					{
					//	Is loop index >= to? Then continue looping.

					if (iCompare == 1 || iCompare == 0)
						m_Stack.Push(CDatum(true));
					else
						m_Stack.Push(CDatum());
					}
				else
					{
					//	If loop index <= to? Then continue looping.

					if (iCompare == -1 || iCompare == 0)
						m_Stack.Push(CDatum(true));
					else
						m_Stack.Push(CDatum());
					}
				
				m_pIP++;
				break;
				}

			case opIncStep:
				{
				DWORD dwOperand = GetOperand(*m_pIP);

				//	Pop the step

				CDatum dStep = m_Stack.Pop();

				//	Pop the to value (which we no longer need)

				m_Stack.Pop();

				//	Pop the current loop index value

				CNumberValue I(m_Stack.Pop());

				//	Increment

				I.Add(dStep);

				//	Set the value

				m_pLocalEnv->SetArgumentValue((dwOperand >> 8), (dwOperand & 0xff), I.GetDatum());

				m_pIP++;
				break;
				}

			case opMakeArray:
				iCount = GetOperand(*m_pIP);
				if (iCount == 0)
					m_Stack.Push(CDatum());
				else
					{
					pArray = new CComplexArray;
					pArray->InsertEmpty(iCount);
					for (i = 0; i < iCount; i++)
						pArray->SetElement(iCount - 1 - i, m_Stack.Pop());

					m_Stack.Push(CDatum(pArray));
					}

				m_pIP++;
				break;

			case opMakeStruct:
				pStruct = new CComplexStruct;
				iCount = GetOperand(*m_pIP);
				if (iCount > 0)
					{
					for (i = 0; i < iCount; i++)
						{
						CDatum dValue = m_Stack.Pop();
						CDatum dKey = m_Stack.Pop();

						pStruct->SetElement(dKey, dValue);
						}
					}

				m_Stack.Push(CDatum(pStruct));
				m_pIP++;
				break;

			case opMakeObject:
				{
				CString sFullyQualifiedName = m_Stack.Pop();
				CDatum dObj = CDatum::CreateObject(FindType(sFullyQualifiedName));

				iCount = GetOperand(*m_pIP);
				if (iCount > 0)
					{
					for (i = 0; i < iCount; i++)
						{
						CDatum dValue = m_Stack.Pop();
						CDatum dKey = m_Stack.Pop();

						dObj.SetElement((const CString &)dKey, dValue);
						}
					}

				m_Stack.Push(dObj);
				m_pIP++;
				break;
				}

			case opMakeAsType:
				{
				CString sFullyQualifiedName = m_Stack.Pop();
				CDatum dType = FindType(sFullyQualifiedName);
				CDatum dValue = m_Stack.Pop();
				CDatum dNewValue = CDatum::CreateAsType(dType, dValue);
				m_Stack.Push(dNewValue);
				m_pIP++;
				break;
				}

			case opMakeDatatype:
				{
				CString sFullyQualifiedName = m_Stack.Pop();
				CDatum dType = FindType(sFullyQualifiedName);
				m_Stack.Push(dType);
				m_pIP++;
				break;
				}

			case opPushCoreType:
				{
				DWORD dwCoreType = GetOperand(*m_pIP);
				if (dwCoreType > 0 && (int)dwCoreType < m_Types.GetCoreTypeCount())
					m_Stack.Push(m_Types.GetCoreType(dwCoreType));
				else
					m_Stack.Push(CDatum());

				m_pIP++;
				break;
				}

			case opNot:
				if (m_Stack.Pop().IsNil())
					m_Stack.Push(true);
				else
					m_Stack.Push(CDatum());
				m_pIP++;
				break;

			case opIsEqual:
				iCount = GetOperand(*m_pIP);
				if (iCount == 0)
					bCondition = true;
				else
					{
					bCondition = true;
					dValue = m_Stack.Pop();

					for (i = 1; i < iCount; i++)
						{
						CDatum dValue2 = m_Stack.Pop();
						if (bCondition && !ExecuteIsEquivalent(dValue, dValue2))
							bCondition = false;
						}
					}

				if (bCondition)
					m_Stack.Push(CDatum(true));
				else
					m_Stack.Push(CDatum());

				m_pIP++;
				break;

			case opIsIdentical:
				iCount = GetOperand(*m_pIP);
				if (iCount == 0)
					bCondition = true;
				else
					{
					bCondition = true;
					dValue = m_Stack.Pop();

					for (i = 1; i < iCount; i++)
						{
						CDatum dValue2 = m_Stack.Pop();
						if (bCondition && !ExecuteIsIdentical(dValue, dValue2))
							bCondition = false;
						}
					}

				if (bCondition)
					m_Stack.Push(CDatum(true));
				else
					m_Stack.Push(CDatum());

				m_pIP++;
				break;

			case opIsNotEqual:
				iCount = GetOperand(*m_pIP);
				if (iCount == 0)
					bCondition = true;
				else
					{
					bCondition = true;
					dValue = m_Stack.Pop();

					for (i = 1; i < iCount; i++)
						{
						CDatum dValue2 = m_Stack.Pop();
						if (bCondition && !ExecuteIsEquivalent(dValue, dValue2))
							bCondition = false;
						}
					}

				if (!bCondition)
					m_Stack.Push(CDatum(true));
				else
					m_Stack.Push(CDatum());

				m_pIP++;
				break;

			case opIsNotIdentical:
				iCount = GetOperand(*m_pIP);
				if (iCount == 0)
					bCondition = true;
				else
					{
					bCondition = true;
					dValue = m_Stack.Pop();

					for (i = 1; i < iCount; i++)
						{
						CDatum dValue2 = m_Stack.Pop();
						if (bCondition && !ExecuteIsIdentical(dValue, dValue2))
							bCondition = false;
						}
					}

				if (!bCondition)
					m_Stack.Push(CDatum(true));
				else
					m_Stack.Push(CDatum());

				m_pIP++;
				break;

			case opIsLess:
				iCount = GetOperand(*m_pIP);
				if (iCount == 0)
					bCondition = false;
				else if (iCount == 1)
					bCondition = (ExecuteCompare(m_Stack.Pop(), CDatum()) == -1);
				else
					{
					bCondition = true;
					dValue = m_Stack.Pop();

					for (i = 1; i < iCount; i++)
						{
						CDatum dValue2 = m_Stack.Pop();
						if (bCondition && ExecuteCompare(dValue, dValue2) != 1)
							bCondition = false;

						dValue = dValue2;
						}
					}

				if (bCondition)
					m_Stack.Push(CDatum(true));
				else
					m_Stack.Push(CDatum());

				m_pIP++;
				break;

			case opIsGreater:
				iCount = GetOperand(*m_pIP);
				if (iCount == 0)
					bCondition = false;
				else if (iCount == 1)
					bCondition = (ExecuteCompare(m_Stack.Pop(), CDatum()) == 1);
				else
					{
					bCondition = true;
					dValue = m_Stack.Pop();

					for (i = 1; i < iCount; i++)
						{
						CDatum dValue2 = m_Stack.Pop();
						if (bCondition && ExecuteCompare(dValue, dValue2) != -1)
							bCondition = false;

						dValue = dValue2;
						}
					}

				if (bCondition)
					m_Stack.Push(CDatum(true));
				else
					m_Stack.Push(CDatum());

				m_pIP++;
				break;

			case opIsLessOrEqual:
				iCount = GetOperand(*m_pIP);
				if (iCount == 0)
					bCondition = true;
				else if (iCount == 1)
					bCondition = (ExecuteCompare(m_Stack.Pop(), CDatum()) != 1);
				else
					{
					bCondition = true;
					dValue = m_Stack.Pop();

					for (i = 1; i < iCount; i++)
						{
						CDatum dValue2 = m_Stack.Pop();
						if (bCondition && ExecuteCompare(dValue, dValue2) == -1)
							bCondition = false;

						dValue = dValue2;
						}
					}

				if (bCondition)
					m_Stack.Push(CDatum(true));
				else
					m_Stack.Push(CDatum());

				m_pIP++;
				break;

			case opIsGreaterOrEqual:
				iCount = GetOperand(*m_pIP);
				if (iCount == 0)
					bCondition = true;
				else if (iCount == 1)
					bCondition = (ExecuteCompare(m_Stack.Pop(), CDatum()) != -1);
				else
					{
					bCondition = true;
					dValue = m_Stack.Pop();

					for (i = 1; i < iCount; i++)
						{
						CDatum dValue2 = m_Stack.Pop();
						if (bCondition && ExecuteCompare(dValue, dValue2) == 1)
							bCondition = false;

						dValue = dValue2;
						}
					}

				if (bCondition)
					m_Stack.Push(CDatum(true));
				else
					m_Stack.Push(CDatum());

				m_pIP++;
				break;

			case opMakeFunc:
				CHexeFunction::Create(m_dCodeBank, GetOperand(*m_pIP), m_dCurGlobalEnv, m_dLocalEnv, &dValue);
				m_Stack.Push(dValue);
				m_pIP++;
				break;

			case opMakeFunc2:
				{
				CDatum dAttribs = m_Stack.Pop();
				m_Stack.Push(CHexeFunction::Create(m_dCodeBank, GetOperand(*m_pIP), m_dCurGlobalEnv, m_dLocalEnv, dAttribs));
				m_pIP++;
				break;
				}

			case opMakePrimitive:
				CHexePrimitive::Create((CDatum::ECallType)GetOperand(*m_pIP), &dValue);
				m_Stack.Push(dValue);
				m_pIP++;
				break;

			case opMakeApplyEnv:
				{
				m_LocalEnvStack.Save(m_dCurGlobalEnv, m_pCurGlobalEnv, m_dLocalEnv, m_pLocalEnv);
				m_pLocalEnv = new CHexeLocalEnvironment;
				m_dLocalEnv = CDatum(m_pLocalEnv);

				//	The top of the stack is a list of arguments

				CDatum dArgList = m_Stack.Pop();

				//	Add the remaining args to the environment

				int iFixedArgCount = GetOperand(*m_pIP) - 1;
				for (i = 0; i < iFixedArgCount; i++)
					m_pLocalEnv->SetArgumentValue(0, (iFixedArgCount - 1) - i, m_Stack.Pop());

				//	Now add the args in the list

				for (i = 0; i < dArgList.GetCount(); i++)
					m_pLocalEnv->SetArgumentValue(0, iFixedArgCount + i, dArgList.GetElement(i));

				m_pIP++;
				break;
				}

			case opMakeEnv:
				{
				m_LocalEnvStack.Save(m_dCurGlobalEnv, m_pCurGlobalEnv, m_dLocalEnv, m_pLocalEnv);
				m_pLocalEnv = new CHexeLocalEnvironment;
				m_dLocalEnv = CDatum(m_pLocalEnv);

				int iArgCount = GetOperand(*m_pIP);
				for (i = 0; i < iArgCount; i++)
					m_pLocalEnv->SetArgumentValue(0, (iArgCount - 1) - i, m_Stack.Pop());

				m_pIP++;
				break;
				}

			case opMakeBlockEnv:
				{
				CDatum dPrevEnv = m_dLocalEnv;

				m_LocalEnvStack.Save(m_dCurGlobalEnv, m_pCurGlobalEnv, m_dLocalEnv, m_pLocalEnv);
				m_pLocalEnv = new CHexeLocalEnvironment;
				m_dLocalEnv = CDatum(m_pLocalEnv);

				m_pLocalEnv->SetParentEnv(dPrevEnv);
				m_pLocalEnv->ResetNextArg();

				m_pIP++;
				break;
				}

			case opMakeMethodEnv:
				{
				m_LocalEnvStack.Save(m_dCurGlobalEnv, m_pCurGlobalEnv, m_dLocalEnv, m_pLocalEnv);
				m_pLocalEnv = new CHexeLocalEnvironment;
				m_dLocalEnv = CDatum(m_pLocalEnv);

				int iArgCount = GetOperand(*m_pIP);

				//	Look for the this pointer. If we have a valid (non-nil) this
				//	pointer, then we include it as the first parameter.

				CDatum dThis = m_Stack.Get(iArgCount);
				if (!dThis.IsNil())
					iArgCount++;

				//	Pop arguments into local environment.

				for (i = 0; i < iArgCount; i++)
					m_pLocalEnv->SetArgumentValue(0, (iArgCount - 1) - i, m_Stack.Pop());

				//	Pop the this pointer, if necessary.

				if (dThis.IsNil())
					m_Stack.Pop();

				m_pIP++;
				break;
				}

			case opEnterEnv:
				{
				CHexeFunction *pFunction = CHexeFunction::Upconvert(m_dExpression);

				//	Set the global environment to match the function

				CHexeGlobalEnvironment *pGlobalEnv = pFunction->GetGlobalEnvPointer();
				if (pGlobalEnv)
					{
					m_dCurGlobalEnv = pFunction->GetGlobalEnv();
					m_pCurGlobalEnv = pGlobalEnv;
					}

				//	Set the local environment of the function

				m_pLocalEnv->SetParentEnv(pFunction->GetLocalEnv());
				m_pLocalEnv->ResetNextArg();

				m_pIP++;
				break;
				}

			case opDefineArg:
				m_pLocalEnv->SetNextArgKey(m_pCodeBank->GetString(GetOperand(*m_pIP)));
				m_pIP++;
				break;

			case opExitEnv:
				m_LocalEnvStack.Restore(&m_dCurGlobalEnv, &m_pCurGlobalEnv, &m_dLocalEnv, &m_pLocalEnv);
				m_pIP++;
				break;

			case opCall:
				{
				CDatum dNewExpression = m_Stack.Pop();
				
				//	If the function changes the instruction pointer, then set up 
				//	the call context here.

				DWORD *pNewIP;
				CDatum dNewCodeBank;
				CDatum::ECallType iCallType = dNewExpression.GetCallInfo(&dNewCodeBank, &pNewIP);

				//	Handle it

				switch (iCallType)
					{
					case CDatum::ECallType::Call:
						{
						m_CallStack.Save(m_dExpression, m_dCodeBank, ++m_pIP);
						m_dExpression = dNewExpression;
						m_dCodeBank = dNewCodeBank;
						m_pCodeBank = CHexeCode::Upconvert(m_dCodeBank);

						m_pIP = pNewIP;
						break;
						}

					case CDatum::ECallType::Library:
						{
						DWORDLONG dwStart = ::sysGetTickCount64();

						CDatum dResult;
						CDatum::InvokeResult iResult = dNewExpression.Invoke(this, m_dLocalEnv, m_UserSecurity.GetExecutionRights(), &dResult);

						m_dwLibraryTime += ::sysGetTickCount64() - dwStart;

						if (iResult != CDatum::InvokeResult::ok)
							{
							ERun iRunResult = ExecuteHandleInvokeResult(iResult, dNewExpression, dResult, retResult);
							if (iRunResult == ERun::OK)
								break;

							return iRunResult;
							}

						//	Restore the environment

						m_LocalEnvStack.Restore(&m_dCurGlobalEnv, &m_pCurGlobalEnv, &m_dLocalEnv, &m_pLocalEnv);

						//	Done

						m_Stack.Push(dResult);
						m_pIP++;
						break;
						}

					case CDatum::ECallType::Invoke:
						{
						//	The first argument is the message

						CDatum dMsg = m_dLocalEnv.GetElement(0);

						//	Make a payload out of the arguments

						CComplexArray *pArray = new CComplexArray;
						for (i = 1; i < m_dLocalEnv.GetCount(); i++)
							pArray->Insert(m_dLocalEnv.GetElement(i));

						//	Restore the environment and advance IP

						m_LocalEnvStack.Restore(&m_dCurGlobalEnv, &m_pCurGlobalEnv, &m_dLocalEnv, &m_pLocalEnv);
						m_pIP++;

						//	Make sure we haven't exceeded our execution time.

						if (m_dwAbortTime && ::sysGetTickCount64() >= m_dwAbortTime)
							return RuntimeError(ERR_EXECUTION_TOOK_TOO_LONG, *retResult);

						//	Send the message

						if (!SendHexarcMessage(dMsg, CDatum(pArray), &dValue))
							{
							*retResult = dValue;
							return ERun::Error;
							}

						//	Async request

						*retResult = dValue;
						return ERun::AsyncRequest;
						}

					default:
						return RuntimeError(strPattern(ERR_NOT_A_FUNCTION, dNewExpression.AsString()), *retResult);
					}

				break;
				}

			case opHexarcMsg:
				{
				CDatum dPayload = m_Stack.Pop();
				CDatum dMsg = m_Stack.Pop();

				m_pIP++;

				//	Make sure we haven't exceeded our execution time.

				if (m_dwAbortTime && ::sysGetTickCount64() >= m_dwAbortTime)
					return RuntimeError(ERR_EXECUTION_TOOK_TOO_LONG, *retResult);

				//	Send the message

				if (!SendHexarcMessage(dMsg, dPayload, &dValue))
					{
					*retResult = dValue;
					return ERun::Error;
					}

				//	Async request

				*retResult = dValue;
				return ERun::AsyncRequest;
				}

			case opReturn:
				{
				m_CallStack.Restore(&m_dExpression, &m_dCodeBank, &m_pIP, &m_pCodeBank);

				//	If we have a NULL codebank then it means that need to return
				//	to a library invocation.

				if (m_pCodeBank == NULL)
					{
					//	Restore the call stack

					CDatum dPrimitive = m_dExpression;
					CDatum dContext = m_dCodeBank.GetElement(0);
					m_dExpression = m_dCodeBank.GetElement(1);
					m_dCodeBank = m_dCodeBank.GetElement(2);
					m_pCodeBank = CHexeCode::Upconvert(m_dCodeBank);

					//	Is this an event handler return?

					if (strEquals(dContext, TYPE_EVENT_HANDLER_CALL))
						{
						//	Must be in an event handler.

						if (m_iEventHandlerLevel <= 0)
							throw CException(errFail);

						//	Pop the result

						CDatum dEventHandlerResult = m_Stack.Pop();

						//	Continue
						//
						//	NOTE: We don't need to restore m_LocalEnvStack because the function 
						//	did that in its return code (call to opExitEnv).

						m_iEventHandlerLevel--;
						*retResult = dEventHandlerResult;
						return ERun::EventHandlerDone;
						}

					//	Otherwise, we return to a library function.

					else
						{
						//	Pop the result

						CDatum dSubResult = m_Stack.Pop();

						//	Continue library invocation

						DWORDLONG dwStart = ::sysGetTickCount64();

						CDatum dResult;
						CDatum::InvokeResult iResult = dPrimitive.InvokeContinues(this, dContext, dSubResult, &dResult);

						m_dwLibraryTime += ::sysGetTickCount64() - dwStart;

						if (iResult != CDatum::InvokeResult::ok)
							{
							ERun iRunResult = ExecuteHandleInvokeResult(iResult, dPrimitive, dResult, retResult);
							if (iRunResult == ERun::OK)
								break;

							return iRunResult;
							}

						m_LocalEnvStack.Restore(&m_dCurGlobalEnv, &m_pCurGlobalEnv, &m_dLocalEnv, &m_pLocalEnv);

						//	Done

						m_Stack.Push(dResult);
						m_pIP++;
						}
					}

				break;
				}

			case opPushLocal:
				{
				DWORD dwOperand = GetOperand(*m_pIP);
				m_Stack.Push(m_pLocalEnv->GetArgument((dwOperand >> 8), (dwOperand & 0xff)));
				m_pIP++;
				break;
				}

			case opPushLocalItem:
				{
				DWORD dwOperand = GetOperand(*m_pIP);
				int iIndex = (int)m_Stack.Pop();
				CDatum dList = m_pLocalEnv->GetArgument((dwOperand >> 8), (dwOperand & 0xff));

				//	For structs we push a tuple of key, value

				if (dList.GetBasicType() == CDatum::typeStruct)
					{
					CComplexArray *pTuple = new CComplexArray;
					pTuple->Append(dList.GetKey(iIndex));
					pTuple->Append(dList.GetElement(this, iIndex));
					m_Stack.Push(CDatum(pTuple));
					}

				//	Otherwise, just the value

				else
					m_Stack.Push(dList.GetElement(this, iIndex));

				m_pIP++;
				break;
				}

			case opPushLocalLength:
				{
				DWORD dwOperand = GetOperand(*m_pIP);
				m_Stack.Push(m_pLocalEnv->GetArgument((dwOperand >> 8), (dwOperand & 0xff)).GetCount());
				m_pIP++;
				break;
				}

			case opPopLocal:
				{
				DWORD dwOperand = GetOperand(*m_pIP);
				m_pLocalEnv->SetArgumentValue((dwOperand >> 8), (dwOperand & 0xff), m_Stack.Pop());
				m_pIP++;
				break;
				}

			case opSetLocal:
				{
				DWORD dwOperand = GetOperand(*m_pIP);
				m_pLocalEnv->SetArgumentValue((dwOperand >> 8), (dwOperand & 0xff), m_Stack.Get());
				m_pIP++;
				break;
				}

			case opIncLocalInt:
				{
				DWORD dwOperand = GetOperand(*m_pIP);
				CDatum dValue = m_pLocalEnv->GetArgument((dwOperand >> 8), (dwOperand & 0xff));

				m_pLocalEnv->SetArgumentValue((dwOperand >> 8), (dwOperand & 0xff), CDatum((int)dValue + 1));
				m_pIP++;
				break;
				}

			case opSetLocalItem:
				{
				DWORD dwOperand = GetOperand(*m_pIP);

				CDatum dValue = m_Stack.Pop();
				CDatum dKey = m_Stack.Pop();
				CDatum dResult;
				if (!ExecuteSetAt(m_pLocalEnv->GetArgument((dwOperand >> 8), (dwOperand & 0xff)), dKey, dValue, &dResult))
					{
					*retResult = dResult;
					return ERun::Error;
					}

				m_pLocalEnv->SetArgumentValue((dwOperand >> 8), (dwOperand & 0xff), dResult);
				m_Stack.Push(dResult);

				m_pIP++;
				break;
				}

			case opAppendLocalItem:
				{
				DWORD dwOperand = GetOperand(*m_pIP);

				CDatum dValue = m_Stack.Pop();
				CDatum dArray = m_pLocalEnv->GetArgument((dwOperand >> 8), (dwOperand & 0xff));
				if (dArray.IsNil())
					{
					CComplexArray *pArray = new CComplexArray;
					pArray->Insert(dValue);
					m_pLocalEnv->SetArgumentValue((dwOperand >> 8), (dwOperand & 0xff), CDatum(pArray));
					}
				else
					{
					//	Append in place.
					//
					//	NOTE: We can do this because this opcode is only used 
					//	when we have control over the local variable that we're
					//	manipulating.
					//
					//	If we cannot determine the provenance of the variable
					//	(i.e., we don't know who else has a pointer to the
					//	array) then we need to make a copy.

					dArray.Append(dValue);
					}

				m_pIP++;
				break;
				}

			case opAdd:
				{
				iCount = GetOperand(*m_pIP);

				if (iCount == 2)
					{
					CDatum dB = m_Stack.Pop();
					CDatum dA = m_Stack.Pop();

					m_Stack.Push(ExecuteOpAdd(dA, dB, m_bAddConcatenatesStrings));
					}
				else if (iCount == 0)
					m_Stack.Push(CDatum(0));
				else
					{
					CNumberValue Result(m_Stack.Pop());

					for (i = 1; i < iCount; i++)
						Result.Add(m_Stack.Pop());

					m_Stack.Push(Result.GetDatum());
					}

				m_pIP++;
				break;
				}

			case opDivide:
				iCount = GetOperand(*m_pIP);

				if (iCount == 2)
					{
					CDatum dDivisor = m_Stack.Pop();
					CNumberValue Dividend(m_Stack.Pop());
					if (!Dividend.Divide(dDivisor))
						return RuntimeError(ERR_DIVISION_BY_ZERO, *retResult);

					m_Stack.Push(Dividend.GetDatum());
					}
				else if (iCount == 1)
					{
					CDatum dDivisor = m_Stack.Pop();
					CNumberValue Dividend(1.0);
					if (!Dividend.Divide(dDivisor))
						return RuntimeError(ERR_DIVISION_BY_ZERO, *retResult);

					m_Stack.Push(Dividend.GetDatum());
					}
				else if (iCount < 1)
					m_Stack.Push(0);
				else
					{
					CNumberValue Result(m_Stack.Pop());
					for (i = 2; i < iCount; i++)
						Result.Multiply(m_Stack.Pop());

					if (!Result.DivideReversed(m_Stack.Pop()))
						return RuntimeError(ERR_DIVISION_BY_ZERO, *retResult);

					m_Stack.Push(Result.GetDatum());
					}

				m_pIP++;
				break;

			case opMultiply:
				iCount = GetOperand(*m_pIP);

				if (iCount < 1)
					m_Stack.Push(0);
				else
					{
					CNumberValue Result(m_Stack.Pop());

					for (i = 1; i < iCount; i++)
						Result.Multiply(m_Stack.Pop());

					m_Stack.Push(Result.GetDatum());
					}

				m_pIP++;
				break;

			case opPower:
				iCount = GetOperand(*m_pIP);

				if (iCount == 2)
					{
					CDatum dPower = m_Stack.Pop();
					CNumberValue Value(m_Stack.Pop());
					Value.Power(dPower);
					m_Stack.Push(Value.GetDatum());
					}
				else
					{
					m_Stack.Push(CDatum());
					}

				m_pIP++;
				break;

			case opMod:
				iCount = GetOperand(*m_pIP);

				if (iCount == 2)
					{
					CDatum dDivisor = m_Stack.Pop();
					CNumberValue Dividend(m_Stack.Pop());
					if (!Dividend.Mod(dDivisor))
						return RuntimeError(ERR_DIVISION_BY_ZERO, *retResult);

					m_Stack.Push(Dividend.GetDatum());
					}
				else
					{
					m_Stack.Push(CDatum());
					}

				m_pIP++;
				break;

			case opSubtract:
				iCount = GetOperand(*m_pIP);

				if (iCount == 2)
					{
					CDatum dB = m_Stack.Pop();
					CDatum dA = m_Stack.Pop();

					m_Stack.Push(ExecuteOpSubtract(dA, dB));
					}
				else if (iCount == 0)
					m_Stack.Push(CDatum(0));

				else if (iCount == 1)
					{
					CNumberValue Result(CDatum(0));
					Result.Subtract(m_Stack.Pop());
					m_Stack.Push(Result.GetDatum());
					}
				else
					{
					CNumberValue Result(CDatum(0));

					for (i = 0; i < iCount - 1; i++)
						Result.Subtract(m_Stack.Pop());

					Result.Add(m_Stack.Pop());
					m_Stack.Push(Result.GetDatum());
					}

				m_pIP++;
				break;

			case opPushArrayItem:
				iCount = GetOperand(*m_pIP);

				if (iCount == 2)
					{
					int iIndex = m_Stack.Pop();
					CDatum dArray = m_Stack.Pop();
					if (iIndex >= 0 && iIndex < dArray.GetCount())
						m_Stack.Push(dArray.GetElement(iIndex));
					else
						m_Stack.Push(CDatum());
					}
				else
					{
					m_Stack.Push(CDatum());
					}

				m_pIP++;
				break;

			case opSetArrayItem:
				{
				//	LATER: Check for reference loops

				int iIndex = m_Stack.Pop();
				CDatum dArray = m_Stack.Pop();
				CDatum dValue = m_Stack.Pop();
				if (iIndex >= 0 && iIndex < dArray.GetCount())
					{
					dArray.SetElement(iIndex, dValue);
					m_Stack.Push(dValue);
					}
				else
					{
					m_Stack.Push(CDatum());
					}

				m_pIP++;
				break;
				}

			case opPushStructItem:
				iCount = GetOperand(*m_pIP);

				if (iCount == 2)
					{
					CDatum dField = m_Stack.Pop();
					CDatum dStruct = m_Stack.Pop();
					m_Stack.Push(dStruct.GetElement(dField.AsString()));
					}
				else
					{
					m_Stack.Push(CDatum());
					}

				m_pIP++;
				break;

			case opSetStructItem:
				{
				//	LATER: Check for reference loops
				//	LATER: Handle errors inside SetElement.

				CDatum dField = m_Stack.Pop();
				CDatum dStruct = m_Stack.Pop();
				CDatum dValue = m_Stack.Pop();
				dStruct.SetElement(dField.AsString(), dValue);
				m_Stack.Push(dValue);

				m_pIP++;
				break;
				}

			case opPushObjectItem:
				{
				iCount = GetOperand(*m_pIP);
				if (iCount != 2)
					return RuntimeError(ERR_INVALID_OPERAND_COUNT, *retResult);

				//	NOTE: We always expect this to be a string. Compilers should
				//	convert to string if necessary. Otherwise, performance will
				//	suffer.

				CDatum dField = m_Stack.Pop();
				const CString &sField = dField;
				CDatum dObject = m_Stack.Pop();

				switch (dObject.GetBasicType())
					{
					case CDatum::typeNil:
						ExecuteNilMemberItem(sField);
						break;

					case CDatum::typeArray:
						ExecuteArrayMemberItem(dObject, sField);
						break;

					case CDatum::typeCustom:
						if (!ExecuteCustomMemberItem(dObject, sField, *retResult))
							return ERun::Error;
						break;

					case CDatum::typeObject:
						if (!ExecuteObjectMemberItem(dObject, sField, *retResult))
							return ERun::Error;
						break;

					case CDatum::typeTable:
						ExecuteTableMemberItem(dObject, sField);
						break;

					default:
						m_Stack.Push(dObject.GetElement(sField));
						break;
					}

				m_pIP++;
				break;
				}

			case opSetObjectItem:
				{
				//	LATER: Check for reference loops
				//	LATER: Handle errors inside SetElement.

				CDatum dField = m_Stack.Pop();
				CDatum dObject = m_Stack.Pop();
				CDatum dValue = m_Stack.Pop();

				//	NOTE: For performance reasons, this should always be a 
				//	CDatum::typeString. Compilers should enforce this.

				const CString &sField = dField;

				switch (dObject.GetBasicType())
					{
					case CDatum::typeCustom:
						if (!ExecuteSetCustomMemberItem(dObject, sField, dValue, *retResult))
							return ERun::Error;
						break;

					default:
						dObject.SetElement(sField, dValue);
						break;
					}

				m_Stack.Push(dValue);
				m_pIP++;
				break;
				}

			case opPushObjectMethod:
				if (!ExecutePushObjectMethod(*retResult))
					return ERun::Error;
				m_pIP++;
				break;

			case opPop:
				m_Stack.Pop(GetOperand(*m_pIP));
				m_pIP++;
				break;

			case opMakeFlagsFromArray:
				{
				CDatum dMap = m_Stack.Pop();
				CDatum dArray = m_Stack.Pop();
				CDatum dResult;
				if (!ExecuteMakeFlagsFromArray(dArray, dMap, &dResult))
					{
					*retResult = dResult;
					return ERun::Error;
					}

				m_Stack.Push(dResult);
				m_pIP++;
				break;
				}

			case opMapResult:
				{
				DWORD dwOperand = GetOperand(*m_pIP);
				DWORD dwFlags = (DWORD)(int)m_Stack.Pop();
				CDatum dOriginal = m_Stack.Pop();
				CDatum dValue = m_Stack.Pop();
				CDatum dArray = m_pLocalEnv->GetArgument((dwOperand >> 8), (dwOperand & 0xff));

				//	If we're excluding nil and the value is Nil, then nothing to do

				if ((dwFlags & FLAG_MAP_EXCLUDE_NIL) && dValue.IsNil())
					;

				//	Otherwise, append the value to the array

				else
					{
					if (dwFlags & FLAG_MAP_ORIGINAL)
						dValue = dOriginal;

					if (dArray.IsNil())
						{
						CComplexArray *pArray = new CComplexArray;
						pArray->Insert(dValue);
						m_pLocalEnv->SetArgumentValue((dwOperand >> 8), (dwOperand & 0xff), CDatum(pArray));
						}
					else
						dArray.Append(dValue);
					}

				m_pIP++;
				break;
				}

			case opError:
				{
				iCount = GetOperand(*m_pIP);

				CDatum dErrorDesc = (iCount > 1 ? m_Stack.Pop() : CDatum());
				CDatum dErrorCode = (iCount > 0 ? m_Stack.Pop() : CDatum());
				CHexeError::Create(dErrorCode.AsString(), dErrorDesc.AsString(), retResult);
				return ERun::Error;
				}

			case opHalt:
				*retResult = m_Stack.Pop();
				return ERun::OK;

			default:
				return RuntimeError(ERR_INVALID_OP_CODE, *retResult);
			}

		//	Track computes

		m_dwComputes++;

		//	Check to make sure we're not in an infinite loop.

		if ((m_dwComputes % STOP_CHECK_COUNT) == 0)
			{
			if (m_dwAbortTime && ::sysGetTickCount64() >= m_dwAbortTime)
				return RuntimeError(ERR_EXECUTION_TOOK_TOO_LONG, *retResult);

			if (m_pComputeProgress)
				{
				m_pComputeProgress->OnCompute(m_dwComputes, m_dwLibraryTime);
				if (m_pComputeProgress->OnAbortCheck())
					return RuntimeError(ERR_EXECUTION_TOOK_TOO_LONG, *retResult);
				}
			}
		}

	return ERun::OK;
	}

void CHexeProcess::ExecuteArrayMemberItem (CDatum dArray, const CString &sField)

//	ExecuteArrayMemberItem
//
//	Push array properties.

	{
	if (strEqualsNoCase(sField, FIELD_LENGTH))
		{
		m_Stack.Push(dArray.GetCount());
		}
	else
		m_Stack.Push(CDatum());
	}

CDatum CHexeProcess::ExecuteBinaryOp (EOpCodes iOp, CDatum dLeft, CDatum dRight)

//	ExecuteBinaryOp
//
//	This is used by callers to duplicate the functionality of VM.

	{
	switch (iOp)
		{
		case opAdd:
			return ExecuteOpAdd(dLeft, dRight, true);
			
		case opDivide:
			{
			CNumberValue Dividend(dLeft);
			if (!Dividend.Divide(dRight))
				{
				CDatum dError;
				CHexeError::Create(NULL_STR, ERR_DIVISION_BY_ZERO, &dError);
				return dError;
				}

			return Dividend.GetDatum();
			}

		case opIsEqual:
			return CDatum(ExecuteIsEquivalent(dLeft, dRight));

		case opIsGreater:
			return CDatum(ExecuteCompare(dLeft, dRight) == -1);

		case opIsGreaterOrEqual:
			return CDatum(ExecuteCompare(dLeft, dRight) != 1);

		case opIsIdentical:
			return CDatum(ExecuteIsIdentical(dLeft, dRight));

		case opIsLess:
			return CDatum(ExecuteCompare(dLeft, dRight) == 1);

		case opIsLessOrEqual:
			return CDatum(ExecuteCompare(dLeft, dRight) != -1);

		case opIsNotEqual:
			return CDatum(!ExecuteIsEquivalent(dLeft, dRight));

		case opIsNotIdentical:
			return CDatum(!ExecuteIsIdentical(dLeft, dRight));

		case opMod:
			{
			CNumberValue Result(dLeft);
			if (!Result.Mod(dRight))
				{
				CDatum dError;
				CHexeError::Create(NULL_STR, ERR_DIVISION_BY_ZERO, &dError);
				return dError;
				}

			return Result.GetDatum();
			}

		case opMultiply:
			{
			CNumberValue Result(dLeft);
			Result.Multiply(dRight);
			return Result.GetDatum();
			}

		case opPower:
			{
			CNumberValue Result(dLeft);
			Result.Power(dRight);
			return Result.GetDatum();
			}

		case opSubtract:
			return ExecuteOpSubtract(dLeft, dRight);

		default:
			{
			CDatum dError;
			CHexeError::Create(NULL_STR, ERR_UNSUPPORTED_OP, &dError);
			return dError;
			}
		}
	}

int CHexeProcess::ExecuteCompare (CDatum dValue1, CDatum dValue2)

//	ExecuteCompare
//
//	Compares dValue1 and dValue2 and returns:
//
//	0	If dValue1 is equivalent to dValue2 (same as ExecuteIsEquivalent)
//	1	If dValue1 is GREATER THAN dValue2
//	-1	If dValue1 is LESS THAN dValue2

	{
	int i;
	CDatum::Types iType1 = dValue1.GetBasicType();
	CDatum::Types iType2 = dValue2.GetBasicType();

	//	If both types are equal, then compare

	if (iType1 == iType2)
		{
		switch (iType1)
			{
			case CDatum::typeNil:
			case CDatum::typeTrue:
			case CDatum::typeNaN:
				return 0;

			case CDatum::typeInteger32:
				return KeyCompare((int)dValue1, (int)dValue2);

			case CDatum::typeInteger64:
				return KeyCompare((DWORDLONG)dValue1, (DWORDLONG)dValue2);

			case CDatum::typeDouble:
				return KeyCompare((double)dValue1, (double)dValue2);

			case CDatum::typeIntegerIP:
				return KeyCompare((const CIPInteger &)dValue1, (const CIPInteger &)dValue2);

			case CDatum::typeString:
				return KeyCompare(strToLower(dValue1), strToLower(dValue2));

			case CDatum::typeDateTime:
				return KeyCompare((const CDateTime &)dValue1,  (const CDateTime &)dValue2);

			case CDatum::typeTimeSpan:
				return KeyCompare((const CDateTime &)dValue1,  (const CDateTime &)dValue2);

			case CDatum::typeArray:
				{
				int iCount = Min(dValue1.GetCount(), dValue2.GetCount());
				for (i = 0; i < iCount; i++)
					{
					int iCompare = ExecuteCompare(dValue1.GetElement(i), dValue2.GetElement(i));
					if (iCompare != 0)
						return iCompare;
					}

				return KeyCompare(dValue1.GetCount(), dValue2.GetCount());
				}

			case CDatum::typeStruct:
				{
				int iCount = Min(dValue1.GetCount(), dValue2.GetCount());
				for (i = 0; i < iCount; i++)
					{
					int iCompare = KeyCompare(strToLower(dValue1.GetKey(i)), strToLower(dValue2.GetKey(i)));
					if (iCompare != 0)
						return iCompare;

					iCompare = ExecuteCompare(dValue1.GetElement(i), dValue2.GetElement(i));
					if (iCompare != 0)
						return iCompare;
					}

				return KeyCompare(dValue1.GetCount(), dValue2.GetCount());
				}

			case CDatum::typeDatatype:
				return KeyCompare(((const IDatatype &)dValue1).GetFullyQualifiedName(), ((const IDatatype &)dValue2).GetFullyQualifiedName());

			default:
				return KeyCompare(dValue1.AsString(), dValue2.AsString());
			}
		}

	//	If one of the types is a number, then compare as numbers

	else if (dValue1.IsNumber() || dValue2.IsNumber())
		{
		CNumberValue Number1(dValue1);
		CNumberValue Number2(dValue2);

		//	If either number is invalid, then it counts as 0

		if (Number1.IsValidNumber()
				&& Number2.IsValidNumber())
			return Number1.Compare(Number2);
		else if (Number1.IsValidNumber())
			return Number1.Compare(CDatum(0));
		else if (Number2.IsValidNumber())
			return CNumberValue(CDatum(0)).Compare(Number2);
		else
			return 0;
		}

	//	If iType1 is nil then everything is greater than it, except nil.

	else if (iType1 == CDatum::typeNil)
		{
		switch (iType2)
			{
			case CDatum::typeString:
				return (((const CString &)dValue2).IsEmpty() ? 0 : -1);

			case CDatum::typeArray:
			case CDatum::typeStruct:
				return (dValue2.GetCount() == 0 ? 0 : -1);

			default:
				return -1;
			}
		}

	//	If iType2 is nil then everything is greater than it, except nil.

	else if (iType2 == CDatum::typeNil)
		{
		switch (iType1)
			{
			case CDatum::typeString:
				return (((const CString &)dValue1).IsEmpty() ? 0 : 1);

			case CDatum::typeArray:
			case CDatum::typeStruct:
				return (dValue1.GetCount() == 0 ? 0 : 1);

			default:
				return 1;
			}
		}

	//	Otherwise, compare as strings

	else
		return KeyCompare(dValue1.AsString(), dValue2.AsString());
	}

bool CHexeProcess::ExecuteCustomMemberItem (CDatum dObject, const CString &sField, CDatum &retdResult)

//	ExecuteCustomMemberItem
//
//	Push an object member.

	{
	const CString &sTypename = dObject.GetTypename();

	if (strEquals(sTypename, TYPENAME_HEXE_FUNCTION))
		{
		m_Stack.Push(dObject.GetElement(sField));
		}
	else
		m_Stack.Push(dObject.GetElement(sField));

	return true;
	}

CHexeProcess::ERun CHexeProcess::ExecuteHandleInvokeResult (CDatum::InvokeResult iInvokeResult, CDatum dExpression, CDatum dInvokeResult, CDatum *retResult)

//	ExecuteHandleInvokeResult
//
//	Process the result of a primitive invoke when FALSE is returned.
//	We return FALSE if an error should be returned.

	{
	switch (iInvokeResult)
		{
		case CDatum::InvokeResult::ok:
			*retResult = dInvokeResult;
			return ERun::OK;

		case CDatum::InvokeResult::error:
			if (dInvokeResult.IsNil())
				CHexeError::Create(NULL_STR, strPattern(ERR_NOT_A_FUNCTION, dExpression.AsString()), retResult);
			else
				*retResult = dInvokeResult;
			return ERun::Error;

		//	If the primitive returns FALSE with an array then it means
		//	that we are calling a subroutine. The array has the following 
		//	elements:
		//
		//	0.	Function to call
		//	1.	Array of parameters to function
		//	2.	Context data for InvokeContinues

		case CDatum::InvokeResult::runFunction:
			{
			CDatum dFunction = dInvokeResult.GetElement(0);
			CDatum dArgs = dInvokeResult.GetElement(1);
			CDatum dContext = dInvokeResult.GetElement(2);

			//	Validate function

			CDatum dNewCodeBank;
			DWORD *pNewIP;
			if (dFunction.GetCallInfo(&dNewCodeBank, &pNewIP) != CDatum::ECallType::Call)
				return RuntimeError(ERR_INVALID_PRIMITIVE_SUB, *retResult);

			//	Encode everything we need to save into the code bank
			//	datum (we unpack it in opReturn).

			CComplexArray *pSaved = new CComplexArray;
			pSaved->Insert(dContext);
			pSaved->Insert(m_dExpression);
			pSaved->Insert(m_dCodeBank);

			//	Save the library function in the call stack so we return to it 
			//	when we're done

			m_CallStack.Save(dExpression, CDatum(pSaved), m_pIP);

			//	Set up environment

			m_LocalEnvStack.Save(m_dCurGlobalEnv, m_pCurGlobalEnv, m_dLocalEnv, m_pLocalEnv);
			m_pLocalEnv = new CHexeLocalEnvironment;
			m_dLocalEnv = CDatum(m_pLocalEnv);

			int iArgCount = dArgs.GetCount();
			for (int i = 0; i < iArgCount; i++)
				m_pLocalEnv->SetArgumentValue(0, i, dArgs.GetElement(i));

			//	Make the call

			m_dExpression = dFunction;
			m_dCodeBank = dNewCodeBank;
			m_pCodeBank = CHexeCode::Upconvert(m_dCodeBank);

			m_pIP = pNewIP;

			//	Continue processing

			return ERun::OK;
			}

		//	Input Request

		case CDatum::InvokeResult::runInputRequest:
			{
			//	Restore the environment and advance IP

			m_LocalEnvStack.Restore(&m_dCurGlobalEnv, &m_pCurGlobalEnv, &m_dLocalEnv, &m_pLocalEnv);
			m_pIP++;

			//	Block until we get input

			*retResult = CDatum();
			return ERun::InputRequest;
			}

		//	Convert to a Hexarc message send.

		case CDatum::InvokeResult::runInvoke:
			{
			const CString &sMsg = dInvokeResult.GetElement(FIELD_MSG);
			CDatum dPayload = dInvokeResult.GetElement(FIELD_PAYLOAD);

			//	Restore the environment and advance IP

			m_LocalEnvStack.Restore(&m_dCurGlobalEnv, &m_pCurGlobalEnv, &m_dLocalEnv, &m_pLocalEnv);
			m_pIP++;

			//	Send message. NOTE: We call the version that does not check to
			//	see if the user has invoke rights. Since we've wrapped this 
			//	inside a library function, we assume the library is responsible
			//	for security.

			CDatum dValue;
			if (!SendHexarcMessageSafe(sMsg, dPayload, &dValue))
				{
				*retResult = dValue;
				return ERun::Error;
				}

			//	Async request

			*retResult = dValue;
			return ERun::AsyncRequest;
			}

		//	Otherwise, generic error (this should never happen).

		default:
			return RuntimeError(strPattern(ERR_NOT_A_FUNCTION, dExpression.AsString()), *retResult);
		}
	}

bool CHexeProcess::ExecuteIsEquivalent (CDatum dValue1, CDatum dValue2)

//	ExecuteIsEquivalent
//
//	Returns TRUE if dValue1 is equivalent to dValue2
//
//	Nil == ""
//	Nil == {}
//	Nil == ()
//	"abc" == "ABC"

	{
	int i;
	CDatum::Types iType1 = dValue1.GetBasicType();
	CDatum::Types iType2 = dValue2.GetBasicType();

	//	If both types are equal, then compare

	if (iType1 == iType2)
		{
		switch (iType1)
			{
			case CDatum::typeNil:
			case CDatum::typeTrue:
			case CDatum::typeNaN:
				return true;

			case CDatum::typeInteger32:
				return (int)dValue1 == (int)dValue2;

			case CDatum::typeInteger64:
				return (DWORDLONG)dValue1 == (DWORDLONG)dValue2;

			case CDatum::typeDouble:
				return (double)dValue1 == (double)dValue2;

			case CDatum::typeIntegerIP:
				return ((const CIPInteger &)dValue1) == ((const CIPInteger &)dValue2);

			case CDatum::typeString:
				return strEquals(strToLower(dValue1), strToLower(dValue2));

			case CDatum::typeDateTime:
				return ((const CDateTime &)dValue1) == ((const CDateTime &)dValue2);

			case CDatum::typeTimeSpan:
				return ((const CTimeSpan &)dValue1) == ((const CTimeSpan &)dValue2);

			case CDatum::typeArray:
				{
				if (dValue1.GetCount() != dValue2.GetCount())
					return false;

				for (i = 0; i < dValue1.GetCount(); i++)
					if (!ExecuteIsEquivalent(dValue1.GetElement(i), dValue2.GetElement(i)))
						return false;

				return true;
				}

			case CDatum::typeStruct:
				{
				if (dValue1.GetCount() != dValue2.GetCount())
					return false;

				for (i = 0; i < dValue1.GetCount(); i++)
					if (!strEquals(strToLower(dValue1.GetKey(i)), strToLower(dValue2.GetKey(i)))
							|| !ExecuteIsEquivalent(dValue1.GetElement(i), dValue2.GetElement(i)))
						return false;

				return true;
				}

			case CDatum::typeDatatype:
				return strEquals(((const IDatatype &)dValue1).GetFullyQualifiedName(), ((const IDatatype &)dValue2).GetFullyQualifiedName());

			default:
				return false;
			}
		}

	//	If one of the types is nil, then compare

	else if (iType1 == CDatum::typeNil || iType2 == CDatum::typeNil)
		{
		if (iType2 == CDatum::typeNil)
			{
			Swap(dValue1, dValue2);
			Swap(iType1, iType2);
			}

		switch (iType2)
			{
			case CDatum::typeString:
				return ((const CString &)dValue2).IsEmpty();

			case CDatum::typeArray:
			case CDatum::typeStruct:
				return dValue2.GetCount() == 0;

			default:
				return false;
			}
		}

	//	If one of the types is a number, then compare as numbers

	else if (dValue1.IsNumber() || dValue2.IsNumber())
		{
		CNumberValue Number1(dValue1);
		CNumberValue Number2(dValue2);

		return (Number1.IsValidNumber()
				&& Number2.IsValidNumber()
				&& Number1.Compare(Number2) == 0);
		}

	//	Otherwise, cannot compare

	else
		return false;
	}

bool CHexeProcess::ExecuteIsIdentical (CDatum dValue1, CDatum dValue2)

//	ExecuteIsIdentical
//
//	Returns TRUE if dValue1 is the same as dValue2

	{
	CDatum::Types iType1 = dValue1.GetBasicType();
	CDatum::Types iType2 = dValue2.GetBasicType();

	if (iType1 == iType2)
		{
		switch (iType1)
			{
			case CDatum::typeNil:
			case CDatum::typeTrue:
			case CDatum::typeNaN:
				return true;

			case CDatum::typeInteger32:
				return (int)dValue1 == (int)dValue2;

			case CDatum::typeInteger64:
				return (DWORDLONG)dValue1 == (DWORDLONG)dValue2;

			case CDatum::typeDouble:
				return (double)dValue1 == (double)dValue2;

			case CDatum::typeIntegerIP:
				return ((const CIPInteger &)dValue1) == ((const CIPInteger &)dValue2);

			case CDatum::typeString:
				//	Case-sensitive compare
				return strEquals(dValue1, dValue2);

			case CDatum::typeDateTime:
				return ((const CDateTime &)dValue1) == ((const CDateTime &)dValue2);

			case CDatum::typeTimeSpan:
				return ((const CTimeSpan &)dValue1) == ((const CTimeSpan &)dValue2);

			case CDatum::typeArray:
				{
				if (dValue1.GetCount() != dValue2.GetCount())
					return false;

				for (int i = 0; i < dValue1.GetCount(); i++)
					if (!ExecuteIsIdentical(dValue1.GetElement(i), dValue2.GetElement(i)))
						return false;

				return true;
				}

			case CDatum::typeStruct:
				{
				if (dValue1.GetCount() != dValue2.GetCount())
					return false;

				for (int i = 0; i < dValue1.GetCount(); i++)
					if (!strEquals(strToLower(dValue1.GetKey(i)), strToLower(dValue2.GetKey(i)))
							|| !ExecuteIsIdentical(dValue1.GetElement(i), dValue2.GetElement(i)))
						return false;

				return true;
				}

			case CDatum::typeDatatype:
				return strEquals(((const IDatatype &)dValue1).GetFullyQualifiedName(), ((const IDatatype &)dValue2).GetFullyQualifiedName());

			default:
				return false;
			}
		}
	else
		{
		switch (iType1)
			{
			case CDatum::typeInteger32:
				{
				switch (iType2)
					{
					case CDatum::typeInteger64:
						{
						int iValue1 = dValue1;
						DWORDLONG dwValue2 = dValue2;
						return (iValue1 >= 0 && (DWORDLONG)iValue1 == dwValue2);
						}

					case CDatum::typeIntegerIP:
						{
						CIPInteger Value1((int)dValue1);
						return Value1 == (const CIPInteger &)dValue2;
						}

					default:
						return false;
					}
				}

			case CDatum::typeInteger64:
				{
				switch (iType2)
					{
					case CDatum::typeInteger32:
						{
						int iValue2 = dValue2;
						DWORDLONG dwValue1 = dValue1;
						return (iValue2 >= 0 && (DWORDLONG)iValue2 == dwValue1);
						}

					case CDatum::typeIntegerIP:
						{
						CIPInteger Value1((DWORDLONG)dValue1);
						return Value1 == (const CIPInteger &)dValue2;
						}

					default:
						return false;
					}
				}

			case CDatum::typeIntegerIP:
				{
				switch (iType2)
					{
					case CDatum::typeInteger32:
						{
						CIPInteger Value2((int)dValue2);
						return Value2 == (const CIPInteger &)dValue1;
						}

					case CDatum::typeInteger64:
						{
						CIPInteger Value2((DWORDLONG)dValue2);
						return Value2 == (const CIPInteger &)dValue1;
						}

					default:
						return false;
					}
				}

			default:
				return false;
			}
		}
	}

bool CHexeProcess::ExecuteMakeFlagsFromArray (CDatum dOptions, CDatum dMap, CDatum *retdResult)

//	ExecuteMakeFlagsFromArray
//
//	dOptions is an array of string options.
//	dMap is a mapping from strings to DWORDs.
//
//	We look up each string option in the map and OR together each of the flags
//	to generate a result.

	{
	int i;
	DWORD dwFlags = 0;

	for (i = 0; i < dOptions.GetCount(); i++)
		{
		DWORD dwFlag = (DWORD)(int)dMap.GetElement(dOptions.GetElement(i).AsString());
		if (dwFlag)
			dwFlags |= dwFlag;
		}

	*retdResult = CDatum((int)dwFlags);
	return true;
	}

void CHexeProcess::ExecuteNilMemberItem (const CString &sField)

//	ExecuteNilMemberItem
//
//	Member items of nil.

	{
	if (strEqualsNoCase(sField, FIELD_LENGTH))
		{
		m_Stack.Push(0);
		}
	else
		m_Stack.Push(CDatum());
	}

bool CHexeProcess::ExecuteObjectMemberItem (CDatum dObject, const CString &sField, CDatum &retdResult)

//	ExecuteObjectMemberItem
//
//	Push an object member.

	{
	const IDatatype &Type = dObject.GetDatatype();
	if (!Type.IsAny())
		{
		auto iMemberType = Type.HasMember(sField);
		switch (iMemberType)
			{
			case IDatatype::EMemberType::InstanceMethod:
			case IDatatype::EMemberType::StaticMethod:
				{
				CString sFunctionName = strPattern("%s$%s", Type.GetFullyQualifiedName(), sField);

				CDatum dValue;
				if (!m_pCurGlobalEnv->Find(sFunctionName, &dValue))
					{
					RuntimeError(strPattern(ERR_UNBOUND_VARIABLE, sFunctionName), retdResult);
					return false;
					}

				m_Stack.Push(dValue);
				break;
				}

			case IDatatype::EMemberType::InstanceVar:
				m_Stack.Push(dObject.GetElement(sField));
				break;

			default:
				m_Stack.Push(CDatum());
				break;
			}
		}
	else
		{
		m_Stack.Push(dObject.GetElement(sField));
		}

	return true;
	}

CDatum CHexeProcess::ExecuteOpAdd (CDatum dLeft, CDatum dRight, bool bConcatenateStrings)

//	ExecuteOpAdd
//
//	Binary operation

	{
	int iValue1;
	int iValue2;

	if (dLeft.IsNil())
		return dRight;
	else if (dRight.IsNil())
		return dLeft;
	else if (bConcatenateStrings
			&& (dLeft.GetBasicType() == CDatum::typeString && dRight.GetBasicType() == CDatum::typeString))
		{
		const CString &sA = dLeft;
		const CString &sB = dRight;
		CString sResult(sA.GetLength() + sB.GetLength());
		utlMemCopy(sA.GetParsePointer(), sResult.GetParsePointer(), sA.GetLength());
		utlMemCopy(sB.GetParsePointer(), sResult.GetParsePointer() + sA.GetLength(), sB.GetLength());
		return CDatum(std::move(sResult));
		}
	else if ((dLeft.GetNumberType(&iValue1) == CDatum::typeInteger32)
			&& (dRight.GetNumberType(&iValue2) == CDatum::typeInteger32))
		{
		LONGLONG iResult = (LONGLONG)iValue1 + (LONGLONG)iValue2;
		if (iResult >= INT_MIN && iResult <= INT_MAX)
			return CDatum((int)iResult);
		else
			{
			CNumberValue Result(dLeft);
			Result.ConvertToIPInteger();
			Result.Add(dRight);
			return Result.GetDatum();
			}
		}
	else if (bConcatenateStrings
			&& (dLeft.GetBasicType() == CDatum::typeString || dRight.GetBasicType() == CDatum::typeString))
		{
		CString sA = dLeft.AsString();
		CString sB = dRight.AsString();
		CString sResult(sA.GetLength() + sB.GetLength());
		utlMemCopy(sA.GetParsePointer(), sResult.GetParsePointer(), sA.GetLength());
		utlMemCopy(sB.GetParsePointer(), sResult.GetParsePointer() + sA.GetLength(), sB.GetLength());
		return CDatum(std::move(sResult));
		}
	else if (dLeft.GetBasicType() == CDatum::typeDateTime && dRight.GetBasicType() == CDatum::typeTimeSpan)
		{
		return CDatum(CDateTime(timeAddTime(dLeft, dRight)));
		}
	else if (dLeft.GetBasicType() == CDatum::typeTimeSpan && dRight.GetBasicType() == CDatum::typeDateTime)
		{
		return CDatum(CDateTime(timeAddTime(dRight, dLeft)));
		}
	else if (dLeft.GetBasicType() == CDatum::typeTimeSpan && dRight.GetBasicType() == CDatum::typeTimeSpan)
		{
		return CDatum(CTimeSpan::Add(dLeft, dRight));
		}
	else
		{
		CNumberValue Result(dLeft);
		Result.Add(dRight);
		return Result.GetDatum();
		}
	}

CDatum CHexeProcess::ExecuteOpSubtract (CDatum dLeft, CDatum dRight)

//	ExecuteOpSubtract
//
//	Binary operation

	{
	int iValue1;
	int iValue2;
	if ((dLeft.GetNumberType(&iValue1) == CDatum::typeInteger32)
			&& (dRight.GetNumberType(&iValue2) == CDatum::typeInteger32))
		{
		LONGLONG iResult = (LONGLONG)iValue1 - (LONGLONG)iValue2;
		if (iResult >= INT_MIN && iResult <= INT_MAX)
			return CDatum((int)iResult);
		else
			{
			CNumberValue Result(dLeft);
			Result.ConvertToIPInteger();
			Result.Subtract(dRight);
			return Result.GetDatum();
			}
		}
	else if (dLeft.GetBasicType() == CDatum::typeDateTime && dRight.GetBasicType() == CDatum::typeDateTime)
		{
		return CDatum(timeSpan(dRight, dLeft));
		}
	else if (dLeft.GetBasicType() == CDatum::typeDateTime && dRight.GetBasicType() == CDatum::typeTimeSpan)
		{
		return CDatum(timeSubtractTime(dLeft, dRight));
		}
	else if (dLeft.GetBasicType() == CDatum::typeTimeSpan && dRight.GetBasicType() == CDatum::typeTimeSpan)
		{
		return CDatum(CTimeSpan::Subtract(dLeft, dRight));
		}
	else
		{
		CNumberValue Result(dLeft);
		Result.Subtract(dRight);
		return Result.GetDatum();
		}
	}

bool CHexeProcess::ExecutePushObjectMethod (CDatum &retResult)

//	ExecutePushObjectMethod
//
//	Implement opPushObjectMethod

	{
	int iCount = GetOperand(*m_pIP);

	//	LATER: This should probably be an error.

	if (iCount != 2)
		{
		m_Stack.Push(CDatum());
		return true;
		}

	//	Get the object and method and figure out what to do based on the type of
	//	object on the stack.

	CString sField = m_Stack.Pop().AsString();
	CDatum dObject = m_Stack.Pop();

	switch (dObject.GetBasicType())
		{
		case CDatum::typeArray:
			{
			//	Look for a global function of the form, Array.xyz.

			CString sFunctionName = strPattern("Array.%s", sField);

			CDatum dValue;
			if (!m_pCurGlobalEnv->Find(sFunctionName, &dValue))
				{
				CHexeError::Create(NULL_STR, strPattern(ERR_UNBOUND_VARIABLE, sFunctionName), &retResult);
				return false;
				}

			m_Stack.Push(dValue);

			//	We always push the this pointer.

			m_Stack.Push(dObject);
			break;
			}

		case CDatum::typeTable:
			{
			//	Look for a global function of the form, Table.xyz.

			CString sFunctionName = strPattern("Table.%s", sField);

			CDatum dValue;
			if (!m_pCurGlobalEnv->Find(sFunctionName, &dValue))
				{
				CHexeError::Create(NULL_STR, strPattern(ERR_UNBOUND_VARIABLE, sFunctionName), &retResult);
				return false;
				}

			m_Stack.Push(dValue);

			//	We always push the this pointer.

			m_Stack.Push(dObject);
			break;
			}

		case CDatum::typeObject:
			{
			const IDatatype &Type = dObject.GetDatatype();
			if (!Type.IsAny())
				{
				auto iMemberType = Type.HasMember(sField);
				switch (iMemberType)
					{
					case IDatatype::EMemberType::InstanceMethod:
					case IDatatype::EMemberType::StaticMethod:
						{
						CString sFunctionName = strPattern("%s$%s", Type.GetFullyQualifiedName(), sField);

						CDatum dValue;
						if (!m_pCurGlobalEnv->Find(sFunctionName, &dValue))
							{
							CHexeError::Create(NULL_STR, strPattern(ERR_UNBOUND_VARIABLE, sFunctionName), &retResult);
							return false;
							}

						m_Stack.Push(dValue);

						//	We always push the this pointer.

						m_Stack.Push(dObject);
						break;
						}

					default:
						//	LATER: This should probably be an error.
						m_Stack.Push(CDatum());
						break;
					}
				}
			else
				{
				CDatum dMember = dObject.GetMethod(sField);
				if (dMember.IsNil() || dMember.GetCallInfo() == CDatum::ECallType::None)
					{
					CHexeError::Create(NULL_STR, strPattern(ERR_UNBOUND_VARIABLE, sField), &retResult);
					return false;
					}

				m_Stack.Push(dMember);

				//	If we have no type defined, we assume it is 
				//	member function, so we pass the this pointer.
				//	We only need this because some built-in 
				//	objects don't have a type defined. In the future,
				//	we should define types for all objects.

				m_Stack.Push(dObject);
				}
			break;
			}

		default:
			m_Stack.Push(dObject.GetElement(sField));

			//	Push a nil this pointer. In opMakeMethodEnv we 
			//	detect this and deal with it appropriately.

			m_Stack.Push(CDatum());
			break;
		}

	return true;
	}

bool CHexeProcess::ExecuteSetAt (CDatum dOriginal, CDatum dKey, CDatum dValue, CDatum *retdResult)

//	ExecuteSetAt
//
//	Sets ket and value

	{
	int iKey;
	CString sKey;

	//	If the key an integer or string?

	CDatum::Types iKeyType = dKey.GetBasicType();
	bool bKeyIsInteger = (iKeyType == CDatum::typeInteger32 || iKeyType == CDatum::typeDouble);
	if (bKeyIsInteger)
		{
		iKey = (int)dKey;
		if (iKey < 0)
			{
			CHexeError::Create(NULL_STR, strPattern(ERR_INVALID_KEY, dKey.AsString()), retdResult);
			return false;
			}
		else if (iKey > MAX_ARRAY_SIZE)
			{
			CHexeError::Create(NULL_STR, ERR_OUT_OF_MEMORY, retdResult);
			return false;
			}
		}
	else
		{
		sKey = dKey.AsString();
		if (sKey.IsEmpty())
			{
			CHexeError::Create(NULL_STR, strPattern(ERR_INVALID_KEY, dKey.AsString()), retdResult);
			return false;
			}
		}

	//	Set based on what type of value we have in the original variable.

	switch (dOriginal.GetBasicType())
		{
		//	If nil, then we create a new array or structure, depending on the
		//	key type.

		case CDatum::typeNil:
			{
			if (bKeyIsInteger)
				{
				CComplexArray *pArray = new CComplexArray;
				pArray->InsertEmpty(iKey + 1);
				pArray->SetElement(iKey, dValue);
				*retdResult = CDatum(pArray);
				return true;
				}
			else
				{
				CComplexStruct *pStruct = new CComplexStruct;
				pStruct->SetElement(sKey, dValue);
				*retdResult = CDatum(pStruct);
				return true;
				}
			}

		case CDatum::typeArray:
			{
			if (!bKeyIsInteger)
				{
				CHexeError::Create(NULL_STR, strPattern(ERR_INVALID_KEY, dKey.AsString()), retdResult);
				return false;
				}

			//	Change in place

			dOriginal.SetElement(iKey, dValue);
			*retdResult = dOriginal;
			return true;
			}

		case CDatum::typeStruct:
			{
			if (bKeyIsInteger)
				sKey = dKey.AsString();

			//	Change in place

			dOriginal.SetElement(sKey, dValue);
			*retdResult = dOriginal;
			return true;
			}

		case CDatum::typeCustom:
			{
			if (bKeyIsInteger)
				sKey = dKey.AsString();

			dOriginal.SetElement(sKey, dValue);
			*retdResult = dOriginal;
			return true;
			}

		default:
			CHexeError::Create(NULL_STR, ERR_NOT_ARRAY_OR_STRUCT, retdResult);
			return false;
		}
	}

bool CHexeProcess::ExecuteSetCustomMemberItem (CDatum dObject, const CString &sField, CDatum dValue, CDatum &retdResult)

//	ExecuteSetCustomMemberItem
//
//	Sets a member item.

	{
	const CString &sTypename = dObject.GetTypename();

	if (strEquals(sTypename, TYPENAME_HEXE_FUNCTION))
		{
		RuntimeError(ERR_CANT_SET_FUNCTION_MEMBER, retdResult);
		return false;
		}
	else
		{
		dObject.SetElement(sField, dValue);
		}

	return true;
	}

void CHexeProcess::ExecuteTableMemberItem (CDatum dTable, const CString &sField)

//	ExecuteTableMemberItem
//
//	Push array properties.

	{
	if (strEqualsNoCase(sField, FIELD_LENGTH))
		{
		m_Stack.Push(dTable.GetCount());
		}
	else
		m_Stack.Push(CDatum());
	}

CDatum CHexeProcess::ExecuteUnaryOp (EOpCodes iOp, CDatum dValue)

//	ExecuteUnaryOp
//
//	Duplicates VM functionality.

	{
	switch (iOp)
		{
		case opNot:
			return CDatum(dValue.IsNil());

		case opSubtract:
			{
			CNumberValue Result(CDatum(0));
			Result.Subtract(dValue);
			return Result.GetDatum();
			}

		default:
			{
			CDatum dError;
			CHexeError::Create(NULL_STR, ERR_UNSUPPORTED_OP, &dError);
			return dError;
			}
		}
	}

CHexeProcess::ERun CHexeProcess::RuntimeError (const CString &sError, CDatum &retdResult)

//	CreateRuntimeError
//
//	Helper for returning an error from the compute engine.

	{
	CHexeError::Create(NULL_STR, sError, &retdResult);
	return ERun::Error;
	}

