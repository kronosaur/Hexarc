//	FoundationGraphicsCore.h
//
//	Foundation header file
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class CRGBA32Image;

typedef double Metric;

enum EBlendModes
	{
	blendNone =					-1,

	blendNormal =				0,	//	Normal drawing
	blendMultiply =				1,	//	Darkens images
	blendOverlay =				2,	//	Combine multiply/screen
	blendScreen =				3,	//	Brightens images
	blendHardLight =			4,

	blendCompositeNormal =		5,

	//	See BlendModes.cpp to add new blend modes

	blendModeCount =			6,
	};

class CImagePlane
	{
	public:
		enum EChannels
			{
			channelAlpha,
			channelRed,
			channelGreen,
			channelBlue,
			};

		CImagePlane (int cxWidth = 0, int cyHeight = 0);
		virtual ~CImagePlane (void);

		bool AdjustCoords (int *xSrc, int *ySrc, int cxSrc, int cySrc,
						   int *xDest, int *yDest,
						   int *cxWidth, int *cyHeight) const;
		bool AdjustScaledCoords (Metric *xSrc, Metric *ySrc, int cxSrc, int cySrc,
								 Metric xSrcInc, Metric ySrcInc,
								 int *xDest, int *yDest,
								 int *cxDest, int *cyDest);

		inline const RECT &GetClipRect (void) const { return m_rcClip; }
		inline int GetHeight (void) const { return m_cyHeight; }
		inline int GetWidth (void) const { return m_cxWidth; }
		void ResetClipRect (void);
		void SetClipRect (const RECT &rcClip);

	protected:
		int m_cxWidth;
		int m_cyHeight;

		RECT m_rcClip;
	};

class CImageLoader
	{
	public:
		enum EFormats
			{
			formatNone,
			formatUnknown,

			formatBMP,
			formatJPEG,
			formatPNG,
			};

		static EFormats GetFormatFromExtension (const CString &sFilespec);
		static bool Load (IMemoryBlock &Data, EFormats iFormat, CRGBA32Image &Image, CString *retsError = NULL);
	};

