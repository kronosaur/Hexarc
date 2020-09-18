//	MsgMandelbrot.cpp
//
//	CCodeSlingerEngine class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

DECLARE_CONST_STRING(PORT_CODE_COMMAND,					"Code.command");

DECLARE_CONST_STRING(FIELD_CX_TASK,						"cxTask");
DECLARE_CONST_STRING(FIELD_CY_TASK,						"cyTask");
DECLARE_CONST_STRING(FIELD_GRID_OPTIONS,				"gridOptions");
DECLARE_CONST_STRING(FIELD_TASK_ID,						"taskID");
DECLARE_CONST_STRING(FIELD_X_TASK,						"xTask");
DECLARE_CONST_STRING(FIELD_Y_TASK,						"yTask");

DECLARE_CONST_STRING(MSG_CODE_MANDELBROT_TASK,			"Code.mandelbrotTask")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data");

DECLARE_CONST_STRING(ERR_NO_TASK_ADDRESSES,				"Unable to find worker modules.");
DECLARE_CONST_STRING(ERR_NO_TASKS,						"Unable to divide work into tasks.");
DECLARE_CONST_STRING(ERR_INVALID_PARAMS,				"Invalid parameters.");

static constexpr DWORD MESSAGE_TIMEOUT =				3000 * 1000;

class CMandelbrot
	{
	public:
		CMandelbrot (void) { }
		CMandelbrot (double xCenter, double yCenter, double rPixelSize, int cxImage, int yImage);

		static CMandelbrot FromDatum (CDatum dDef);
		double GetCenterX (void) const { return m_xCenter; }
		double GetCenterY (void) const { return m_yCenter; }
		double GetPixelSize (void) const { return m_rPixelSize; }
		int GetImageHeight (void) const { return m_cyImage; }
		int GetImageWidth (void) const { return m_cxImage; }
		bool IsEmpty (void) const { return (m_cxImage <= 0 || m_cyImage <= 0 || m_rPixelSize <= 0.0); }
		void Render (int x, int y, int cxWidth, int cyHeight, CRGBA32Image &retOutput) const;

	private:
		double m_xCenter = 0.0;
		double m_yCenter = 0.0;
		double m_rPixelSize = 0.001;
		int m_cxImage = 0;
		int m_cyImage = 0;
	};

class CGridTask
	{
	public:
		CGridTask (const CString &sAddr, const SArchonMessage &Msg) :
				m_sAddr(sAddr),
				m_Msg(Msg)
			{ }

		const CString &GetAddr (void) const { return m_sAddr; }
		const SArchonMessage &GetMsg (void) const { return m_Msg; }
		bool IsComplete (void) const { return m_bComplete; }
		void Mark (void) { m_Msg.dPayload.Mark(); }
		void SetComplete (bool bValue = true) { m_bComplete = bValue; }

	private:
		CString m_sAddr;						//	Address to send message to
		SArchonMessage m_Msg;					//	Message to send

		bool m_bComplete = false;				//	TRUE if task is complete
	};

class CGridSession : public ISessionHandler
	{
	public:
		CGridSession (void) { }

	protected:
		bool IsComplete (void) const;
		bool IsVerbose (void) const { return m_bIsVerbose; }
		bool SetTaskComplete (int iTask);
		void SetVerbose (bool bValue = true) { m_bIsVerbose = bValue; }

		//	CGridSession virtuals
		virtual bool OnCreateTasks (const SArchonMessage &Msg, TArray<CGridTask> &Tasks) = 0;
		virtual bool OnTaskComplete (const SArchonMessage &Msg) = 0;

	private:
		//	ISessionHandler virtuals
		virtual void OnMark (void) override;
		virtual bool OnProcessMessage (const SArchonMessage &Msg) override;
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override;

		CCriticalSection m_cs;
		
		TArray<CGridTask> m_Tasks;

		bool m_bIsVerbose = false;
	};

class CMandelbrotSession : public CGridSession
	{
	public:
		CMandelbrotSession (CCodeSlingerEngine &Engine) : 
				m_Engine(Engine)
			{ }

	protected:
		//	CGridSession virtuals
		virtual bool OnCreateTasks (const SArchonMessage &Msg, TArray<CGridTask> &Tasks) override;
		virtual bool OnTaskComplete (const SArchonMessage &Msg) override;

	private:
		enum class States
			{
			unknown,
			done,
			};

		CCodeSlingerEngine &m_Engine;
		States m_iState = States::unknown;

		CRGBA32Image m_Output;						//	Output image
	};

void CCodeSlingerEngine::MsgMandelbrot (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgMandelbrot
//
//	This is a proof-of-concept testing distributed computation. Eventually we'll
//	move this out of here.
//
//	Code.msgMandelbrot xCenter yCenter rPixelSize cxImage cyImage options
//
//	xCenter, yCenter: Image center in Mandelbrot coordinates.
//	rPixelSize: Size of a pixel in Mandelbrot space (assumes square pixels)
//	cxImage, cyImage: Size of image in pixels.
//	options:
//
//		gridOptions: Distributed compute options
//
//			nodeCount: Number of nodes to distribute to. If Nil or 0, we default
//				to 1.
//
//	IMPLEMENTATION
//
//	1.	We start a session to generate the Mandelbrot.
//	2.	Get a list of all machines to send to (including ours).
//	3.	Generate data structure of requests for each worker.
//	4.	Send out messages to process each slice.
//		a.	Each worker processes their slice and send back
//			a partial image.
//	5.	As replies come back, update the final image.
//	6.	If workers time out, resend (or abort for now).
//	7.	When all images are done, return.

	{
	//	We start a session to do the actual work. Inside StartSession we call:
	//
	//	CGridSession::OnStartSession, which calls 
	//		CMandelbrotSession::OnCreateTasks
	//
	//	Inside OnCreateTasks we split up the work into CGridTask objects and 
	//	then CGridSession sends messages out to do the work. When the messages
	//	return, we get called at:
	//
	//	CGridSession::OnProcessMessage, which calls
	//		CMandelbrotSession::OnTaskComplete
	//
	//	Inside OnTaskComplete we take the partial result and combine it into
	//	the real result. Eventually, once we have the full result, we send a
	//	reply from inside OnTaskComplete.

	StartSession(Msg, new CMandelbrotSession(*this));
	}

void CCodeSlingerEngine::MsgMandelbrotTask (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgMandelbrotTask
//
//	Generates a portion of a larger Mandelbrot.
//
//	Code.msgMandelbrotTask xCenter yCenter rPixelSize cxImage cyImage options
//
//	xCenter, yCenter: Full image center in Mandelbrot coordinates.
//	rPixelSize: Size of a pixel in Mandelbrot space (assumes square pixels)
//	cxImage, cyImage: Size of full image in pixels.
//	options:
//
//		taskID: ID of the task.
//		xTask:
//		yTask: Upper-left corner of image for this task to generate
//		cxTask:
//		cyTask: Width and height of image for this task to generate.
//
//	The result is a struct with the following elements:
//
//	data: A CRGBA32Image object serialized to a stream.
//	taskID: ID of the task.
//	xTask, yTask: The task image position
//	cxTask, cyTask: The task image width and height

	{
	CDatum dOptions = Msg.dPayload.GetElement(5);
	CDatum dTaskID = dOptions.GetElement(FIELD_TASK_ID);
	int xTask = dOptions.GetElement(FIELD_X_TASK);
	int yTask = dOptions.GetElement(FIELD_Y_TASK);
	int cxTask = dOptions.GetElement(FIELD_CX_TASK);
	int cyTask = dOptions.GetElement(FIELD_CY_TASK);

	CMandelbrot Mandelbrot = CMandelbrot::FromDatum(Msg.dPayload);
	if (Mandelbrot.IsEmpty())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_PARAMS, Msg);
		return;
		}

	CRGBA32Image Output;
	Mandelbrot.Render(xTask, yTask, cxTask, cyTask, Output);
	if (Output.IsEmpty())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_PARAMS, Msg);
		return;
		}

	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_TASK_ID, dTaskID);
	dResult.SetElement(FIELD_X_TASK, xTask);
	dResult.SetElement(FIELD_Y_TASK, yTask);
	dResult.SetElement(FIELD_CX_TASK, cxTask);
	dResult.SetElement(FIELD_CY_TASK, cyTask);

	SendMessageReply(MSG_REPLY_DATA, dResult, Msg);
	}

//	CGridSession ---------------------------------------------------------------

bool CGridSession::IsComplete (void) const

//	IsComplete
//
//	Returns TRUE if all tasks are complete.

	{
	CSmartLock Lock(m_cs);

	for (int i = 0; i < m_Tasks.GetCount(); i++)
		if (!m_Tasks[i].IsComplete())
			return false;

	return true;
	}

void CGridSession::OnMark (void)

//	OnMark
//
//	Mark data in use

	{
	for (int i = 0; i < m_Tasks.GetCount(); i++)
		m_Tasks[i].Mark();
	}

bool CGridSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Handle message

	{
	//	LATER: There is no current way of associating a reply with a task.
	//	For now, if we get an error, we fail.

	if (IsError(Msg))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, Msg.dPayload);
		return false;
		}

	//	Otherwise, send the reply to our subclass

	return OnTaskComplete(Msg);
	}

bool CGridSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start a session

	{
	//	Ask our subclass to create tasks

	if (!OnCreateTasks(Msg, m_Tasks))
		return false;

	//	If no tasks were created, then we fail.

	if (m_Tasks.GetCount() == 0)
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_NO_TASKS);
		return false;
		}

	//	Send messages to each machine to do the work.

	for (int i = 0; i < m_Tasks.GetCount(); i++)
		{
		const CGridTask &Task = m_Tasks[i];

		SendMessageCommand(Task.GetAddr(),
				Task.GetMsg().sMsg,
				Task.GetMsg().sReplyAddr,
				Task.GetMsg().dPayload,
				MESSAGE_TIMEOUT);
		}

	if (IsVerbose())
		GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("Created %d task(s)", m_Tasks.GetCount()));

	//	Wait for messages.

	return true;
	}

bool CGridSession::SetTaskComplete (int iTask)

//	SetTaskComplete
//
//	Sets the given task complete. Returns TRUE if all tasks are complete.

	{
	CSmartLock Lock(m_cs);

	if (iTask >= 0 && iTask < m_Tasks.GetCount())
		m_Tasks[iTask].SetComplete(true);

	return IsComplete();
	}

//	CMandelbrotSession ---------------------------------------------------------

bool CMandelbrotSession::OnCreateTasks (const SArchonMessage &Msg, TArray<CGridTask> &Tasks)

//	OnCreateTasks
//
//	Divide the work into tasks.

	{
	//	Parse message

	CMandelbrot MandelbrotDef = CMandelbrot::FromDatum(Msg.dPayload);
	if (MandelbrotDef.IsEmpty())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_PARAMS);
		return false;
		}

	CDatum dOptions = Msg.dPayload.GetElement(5);
	CDatum dGridOptions = dOptions.GetElement(FIELD_GRID_OPTIONS);

	//	Set some properties

	SetVerbose(m_Engine.IsVerbose());

	//	Get a list of all CodeSlinger modules in the arcology.

	TArray<CString> TaskAddresses = GetProcessCtx()->GetTransporter().GetArcologyPortAddresses(PORT_CODE_COMMAND);
	if (TaskAddresses.GetCount() == 0)
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_NO_TASK_ADDRESSES);
		return false;
		}

	//	For each module, we send multiple messages because each module can 
	//	handle multiple threads.

	constexpr int TASKS_PER_MODULE = 3;

	//	Compute the number of tasks to split the rendering to. We never split
	//	to more than the height of the image.

	const int iTaskCount = Min(MandelbrotDef.GetImageHeight(), TaskAddresses.GetCount() * TASKS_PER_MODULE);
	if (iTaskCount == 0)
		{
		//	This should never happen, but just in case.
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_INVALID_PARAMS);
		return false;
		}

	const int cySegmentBase = MandelbrotDef.GetImageHeight() / iTaskCount;
	int cySegmentRemainder = MandelbrotDef.GetImageHeight() % iTaskCount;
	int yNextSegment = 0;

	//	Generate tasks

	for (int i = 0; i < iTaskCount; i++)
		{
		//	Compute the image segment.

		int cySegment = cySegmentBase;
		if (cySegmentRemainder)
			{
			if (i == TaskAddresses.GetCount() - 1)
				cySegment += cySegmentRemainder;
			else
				{
				cySegment++;
				cySegmentRemainder--;
				}
			}

		const int xSegment = 0;
		const int ySegment = yNextSegment;
		yNextSegment += cySegment;
		const int cxSegment = MandelbrotDef.GetImageWidth();

		//	Generate the message.

		const CString sTaskAddr = TaskAddresses[i % TaskAddresses.GetCount()];

		SArchonMessage TaskMsg;
		TaskMsg.sMsg = MSG_CODE_MANDELBROT_TASK;

		CDatum dPayload(CDatum::typeArray);
		dPayload.Append(MandelbrotDef.GetCenterX());
		dPayload.Append(MandelbrotDef.GetCenterY());
		dPayload.Append(MandelbrotDef.GetPixelSize());
		dPayload.Append(MandelbrotDef.GetImageWidth());
		dPayload.Append(MandelbrotDef.GetImageHeight());

		CDatum dTaskOptions(CDatum::typeStruct);
		dTaskOptions.SetElement(FIELD_TASK_ID, i);
		dTaskOptions.SetElement(FIELD_X_TASK, xSegment);
		dTaskOptions.SetElement(FIELD_Y_TASK, ySegment);
		dTaskOptions.SetElement(FIELD_CX_TASK, cxSegment);
		dTaskOptions.SetElement(FIELD_CY_TASK, cySegment);

		dPayload.Append(dTaskOptions);

		TaskMsg.dPayload = dPayload;
		TaskMsg.sReplyAddr = GenerateAddress(PORT_CODE_COMMAND);

		//	Create the task

		Tasks.Insert(CGridTask(sTaskAddr, TaskMsg));
		}

	return true;
	}

bool CMandelbrotSession::OnTaskComplete (const SArchonMessage &Msg)

//	OnTaskComplete
//
//	A task has completed. If we're done with the session, we return FALSE. 
//	Otherwise, if we need to wait some more, we return TRUE.

	{
	CDatum dTaskID = Msg.dPayload.GetElement(FIELD_TASK_ID);
	const int xTask = Msg.dPayload.GetElement(FIELD_X_TASK);
	const int yTask = Msg.dPayload.GetElement(FIELD_Y_TASK);
	const int cxTask = Msg.dPayload.GetElement(FIELD_CX_TASK);
	const int cyTask = Msg.dPayload.GetElement(FIELD_CY_TASK);

	if (IsVerbose())
		GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("Task %d Complete: %d,%d [%dx%d]", (int)dTaskID, xTask, yTask, cxTask, cyTask));

	//	Mark as complete. If all tasks are complete, then we're done.

	if (SetTaskComplete(dTaskID))
		{
		SendMessageReply(MSG_REPLY_DATA, CDatum(CString("Success!")));
		return false;
		}

	//	Otherwise, keep waiting.

	return true;
	}

//	CMandelbrot ----------------------------------------------------------------

CMandelbrot::CMandelbrot (double xCenter, double yCenter, double rPixelSize, int cxImage, int cyImage) :
		m_xCenter(xCenter),
		m_yCenter(yCenter),
		m_rPixelSize(rPixelSize),
		m_cxImage(cxImage),
		m_cyImage(cyImage)

//	CMandelbrot constructor

	{
	}

CMandelbrot CMandelbrot::FromDatum (CDatum dDef)

//	FromDatum
//
//	Creates from a message payload. If an error, we return an empty Mandelbrot.
//
//	xCenter
//	yCenter
//	rPixelSize
//	cxImage
//	cyImage
//	options

	{
	double xCenter = dDef.GetElement(0);
	double yCenter = dDef.GetElement(1);
	double rPixelSize = dDef.GetElement(2);
	int cxImage = dDef.GetElement(3);
	int cyImage = dDef.GetElement(4);

	if (rPixelSize <= 0.0 || cxImage <= 0 || cyImage <= 0)
		return CMandelbrot();

	return CMandelbrot(xCenter, yCenter, rPixelSize, cxImage, cyImage);
	}

void CMandelbrot::Render (int x, int y, int cxWidth, int cyHeight, CRGBA32Image &retOutput) const

//	Render
//
//	Render the given part of the image.

	{
	if (cxWidth <= 0 || cyHeight <= 0)
		return;

	retOutput.Create(cxWidth, cyHeight);
	}
