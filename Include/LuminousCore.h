//	LuminousCore.h
//
//	LuminousCore Classes
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "Foundation.h"

#include "LuminousComponents.h"

//	ILuminousElement
//
//	This is the base class for all Luminous objects. It handles containment and
//	properties.

class ILuminousElement
	{
	public:

		ILuminousElement (DWORD dwID, const CString& sName) :
				m_dwID(dwID),
				m_sName(sName)
			{ }

		virtual ~ILuminousElement () { }

		DWORD GetID () const { return m_dwID; }
		const CString& GetName () const { return m_sName; }
		SequenceNumber GetSeq () const { return m_Seq; }
		void SetID (DWORD dwID) { m_dwID = dwID; }
		void SetName (const CString& sName) { m_sName = sName; }
		void SetSeq (SequenceNumber Seq) { m_Seq = Seq; }

	private:

		DWORD m_dwID = 0;
		CString m_sName;
		SequenceNumber m_Seq = 1;
	};

#include "LuminousCanvas.h"
#include "LuminousScene2D.h"
#include "LuminousScene3D.h"

//	Animation ------------------------------------------------------------------

class ILuminousAnimator : public ILuminousElement
	{
	};

class CLuminousTimeline
	{
	};

//	3D Voxels ------------------------------------------------------------------

class CLuminousVoxelModel
	{
	};

//	3D Scene -------------------------------------------------------------------

class ILuminousObject : public ILuminousElement
	{
	};

class CLuminousSceneModel
	{
	};

