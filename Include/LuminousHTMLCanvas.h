//	LuminousHTMLCanvas.h
//
//	Classes for rendering to HTML canvas.
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "Foundation.h"
#include "LuminousCore.h"
#include "AEON.h"

class CLuminousCanvasResources
	{
	public:

		void AddNamedResource (const CString& sName, CDatum dResource, SequenceNumber Seq = 1);
		void AddResource (const ILuminousGraphic& Graphic, CDatum dResource);
		int FindNamedResource (const CString& sName) const;
		CDatum GetNamedResource (int iIndex) const { if (iIndex >= 0 && iIndex < m_NamedResources.GetCount()) return m_NamedResources[iIndex].dResource; else throw CException(errFail); }
		int GetNamedResourceCount () const { return m_NamedResources.GetCount(); }
		const CString& GetNamedResourceName (int iIndex) const { if (iIndex >= 0 && iIndex < m_NamedResources.GetCount()) return m_NamedResources.GetKey(iIndex); else throw CException(errFail); }
		SequenceNumber GetNamedResourceSeq (int iIndex) const { if (iIndex >= 0 && iIndex < m_NamedResources.GetCount()) return m_NamedResources[iIndex].Seq; else throw CException(errFail); }
		CDatum GetResource (const ILuminousGraphic& Graphic) const;
		void Mark ();
		void SetSeq (SequenceNumber Seq);

		static CLuminousCanvasResources Read (IByteStream& Stream);
		void Write (IByteStream& Stream) const;

	private:

		struct SNamedEntry
			{
			CDatum dResource;
			SequenceNumber Seq = 0;
			};

		TSortMap<DWORD, CDatum> m_Table;
		TSortMap<CString, SNamedEntry> m_NamedResources;
	};

class CHTMLCanvasRemote
	{
	public:
		enum class ECommand
			{
			none =					0,

			//	Colors and Styles

			fillStyle =				1,
			strokeStyle =			2,

			//	Line Styles

			lineCap =				100,
			lineJoin =				101,
			lineWidth =				102,
			miterLimit =			103,

			//	Rectangles

			rect =					200,
			fillRect =				201,
			strokeRect =			202,
			clearRect =				203,
			clearAll =				204,

			//	Path Commands

			arc =					300,
			arcTo =					301,
			beginPath =				302,
			bezierCurveTo =			303,
			clip =					304,
			closePath =				305,
			fill =					306,
			lineTo =				307,
			moveTo =				308,
			quadraticCurveTo =		309,
			stroke =				310,

			//	Transformations

			scale =					400,

			//	Text

			font =					500,
			textAlign =				501,
			textBaseline =			502,

			//	Image Drawing

			drawImage =				600,
			setResource =			601,
			drawResource =			602,

			//	Internals

			beginUpdate =			1000,
			};

		static CDatum CmdArc (double x, double y, double radius, double startAngle, double endAngle, bool counterClock);
		static CDatum CmdBeginPath ();
		static CDatum CmdBeginUpdate ();
		static CDatum CmdClearRect ();
		static CDatum CmdClearRect (double x, double y, double cxWidth, double cyHeight);
		static CDatum CmdClosePath ();
		static CDatum CmdDrawImage (CDatum dImage, double x, double y);
		static CDatum CmdDrawResource (const CString& sResourceID, double x, double y);
		static CDatum CmdFill ();
		static CDatum CmdFillRect (double x, double y, double cxWidth, double cyHeight);
		static CDatum CmdFillStyle (const CString &sStyle);
		static CDatum CmdLineTo (double x, double y);
		static CDatum CmdLineWidth (double rWidth);
		static CDatum CmdMoveTo (double x, double y);
		static CDatum CmdSetResource (const CString& sName, CDatum dResource, SequenceNumber Seq);
		static CDatum CmdStroke ();
		static CDatum CmdStrokeStyle (const CString &sStyle);

		static ECommand GetCommand (CDatum dData);
		static bool IsClearAll (CDatum dData);
		static bool IsDrawCommand (ECommand iCmd);
		static CDatum RenderAsHTMLCanvasCommands (const CLuminousCanvasModel& Model, const CLuminousCanvasResources& Resources, SequenceNumber Seq = 0);

	private:

		static void AccumulateHTMLCanvasCommands (const CLuminousFillStyle& Style, const CLuminousFillStyle& PrevStyle, CDatum dResult);
		static void AccumulateHTMLCanvasCommands (const CLuminousLineStyle& Style, const CLuminousLineStyle& PrevStyle, CDatum dResult);
		static void AccumulateHTMLCanvasCommands (const CLuminousPath2D& Path, CDatum dResult);
		static void AccumulateHTMLCanvasCommands (const CLuminousShadowStyle& Style, const CLuminousShadowStyle& PrevStyle, CDatum dResult);

	};
