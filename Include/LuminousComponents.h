//	LuminousComponents.h
//
//	LuminousCore Classes
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#pragma once

class CLuminousColorTheme;

class CLuminousCornerRadius
	{
	public:

		double GetBottomLeft () const { return m_rLowerLeft; }
		double GetBottomRight () const { return m_rLowerRight; }
		double GetTopLeft () const { return m_rUpperLeft; }
		double GetTopRight () const { return m_rUpperRight; }
		bool IsUniform () const { return (m_rLowerLeft == m_rLowerRight && m_rLowerRight == m_rUpperLeft && m_rUpperLeft == m_rUpperRight); }
		void SetBottomLeft (double rValue) { m_rLowerLeft = rValue; }
		void SetBottomRight (double rValue) { m_rLowerRight = rValue; }
		void SetTopLeft (double rValue) { m_rUpperLeft = rValue; }
		void SetTopRight (double rValue) { m_rUpperRight = rValue; }

	private:

		double m_rUpperLeft = 0.0;
		double m_rUpperRight = 0.0;
		double m_rLowerLeft = 0.0;
		double m_rLowerRight = 0.0;
	};

class CLuminousColor
	{
	public:

		enum class EType
			{
			None,

			Clear,
			Solid,
			Theme,
			Role,
			LinearGradient,
			RadialGradient,
			ImagePattern,
			};

		enum class EShade
			{
			None = 0,

			Primary,
			Secondary,
			Tertiary,
			Grayscale,

			Red,
			Orange,
			Yellow,
			Green,
			Blue,
			Purple,
			Pink,

			MAX,
			};

		enum class ERole
			{
			None = 0,

			Paper,					//	Neutral Tone 10
			Light,					//	Primary Tone 10
			Highlight,				//	Primary Tone 20
			Overlay,				//	Primary Tone 30
			Accent,					//	Primary Tone 40
			Marker,					//	Primary Tone 50
			Solid,					//	Primary Tone 60
			SolidText,				//	Text on Solid
			Emphasis,				//	Primary Tone 70
			Contrast,				//	Primary Tone 80
			Dark,					//	Primary Tone 90
			Ink,					//	Primary Tone 100

			Error,					//	Alert Error
			ErrorText,				//	Text on Error
			Warning,				//	Alert Warning
			WarningText,			//	Text on Warning
			Link,					//	Link
			LinkText,				//	Text on Link
			LinkHover,				//	Link Hover
			LinkHoverText,			//	Text on Link Hover

			MAX,
			};

		static constexpr int MIN_TONE = 1;
		static constexpr int MAX_TONE = 10;

		CLuminousColor () {}
		CLuminousColor (EType iType) : m_iType(iType)
			{ }

		CLuminousColor (const CRGBA32& rgbColor) :
				m_iType(EType::Solid)
			{
			m_Value.rgbColor = rgbColor;
			}

		CLuminousColor (EShade iShade, int iTone) :
				m_iType(EType::Theme)
			{
			m_Value.ThemeColor.iShade = iShade;
			m_Value.ThemeColor.iToneIndex = ToToneIndex(iTone);
			}

		CLuminousColor (ERole iRole) :
				m_iType(EType::Role)
			{
			m_Value.iRole = iRole;
			}

		CLuminousColor (const CLuminousColor& Src) { Copy(Src); }

		static CLuminousColor CreateFromStream (IByteStream& Stream);

		CLuminousColor& operator= (const CLuminousColor& Src) { if (this != &Src) Copy(Src); return *this; }

		bool operator== (const CLuminousColor &Src) const;
		bool operator!= (const CLuminousColor &Src) const { return !(*this == Src); }

		CString GetThemeColorID () const;
		CRGBA32 GetSolidColor () const { return (m_iType == EType::Solid ? m_Value.rgbColor : CRGBA32()); }
		CRGBA32 GetSolidColor (const CLuminousColorTheme& Theme) const;
		EType GetType () const { return m_iType; }
		bool IsEmpty () const { return m_iType == EType::None; }
		void Read (IByteStream& Stream) { *this = CreateFromStream(Stream); }
		void Write (IByteStream& Stream) const;

		static const CLuminousColor Null;
		static CLuminousColor ParseShade (EShade iShade, CStringView sTone, const CLuminousColor& Default = CLuminousColor());
		static CLuminousColor ParseShadeCompatible (EShade iShade, CStringView sTone, const CLuminousColor& Default = CLuminousColor());
		static CLuminousColor ParseThemeColor (const CString& sValue, const CLuminousColor& Default = CLuminousColor());

	private:

		struct SThemeColor
			{
			EShade iShade = EShade::None;
			int iToneIndex:8 = 0;		//	0-9 (index into tone table).
			};

		union UValue
			{
			CRGBA32 rgbColor;
			SThemeColor ThemeColor;
			ERole iRole;
			};

		static constexpr DWORD TYPE_NONE =			0x00000000;
		static constexpr DWORD TYPE_CLEAR =			0x00000001;
		static constexpr DWORD TYPE_SOLID =			0x00000002;
		static constexpr DWORD TYPE_THEME_V1 =		0x00000003;
		static constexpr DWORD TYPE_THEME =			0x00000004;
		static constexpr DWORD TYPE_ROLE =			0x00000005;

		static constexpr DWORD SHADE_NONE_ID =		0x00000100;
		static constexpr DWORD SHADE_PRIMARY_ID =	0x00000200;
		static constexpr DWORD SHADE_SECONDARY_ID =	0x00000300;
		static constexpr DWORD SHADE_TERTIARY_ID =	0x00000400;
		static constexpr DWORD SHADE_GRAYSCALE_ID =	0x00000500;
		static constexpr DWORD SHADE_RED_ID =		0x00000600;
		static constexpr DWORD SHADE_ORANGE_ID =	0x00000700;
		static constexpr DWORD SHADE_YELLOW_ID =	0x00000800;
		static constexpr DWORD SHADE_GREEN_ID =		0x00000900;
		static constexpr DWORD SHADE_BLUE_ID =		0x00000A00;
		static constexpr DWORD SHADE_PURPLE_ID =	0x00000B00;
		static constexpr DWORD SHADE_PINK_ID =		0x00000C00;

		static constexpr DWORD SHADE_MASK =			0x0000FF00;
		static constexpr DWORD TONE_MASK =			0x000000FF;

		static constexpr DWORD ROLE_NONE_ID =			0x00000000;
		static constexpr DWORD ROLE_PAPER_ID =			0x00000001;
		static constexpr DWORD ROLE_HIGHLIGHT_ID =		0x00000002;
		static constexpr DWORD ROLE_OVERLAY_ID =		0x00000003;
		static constexpr DWORD ROLE_ACCENT_ID =			0x00000004;
		static constexpr DWORD ROLE_MARKER_ID =			0x00000005;
		static constexpr DWORD ROLE_SOLID_ID =			0x00000006;
		static constexpr DWORD ROLE_SOLID_TEXT_ID =		0x00000007;
		static constexpr DWORD ROLE_EMPHASIS_ID =		0x00000008;
		static constexpr DWORD ROLE_CONTRAST_ID =		0x00000009;
		static constexpr DWORD ROLE_DARK_ID =			0x0000000A;
		static constexpr DWORD ROLE_INK_ID =			0x0000000B;
		static constexpr DWORD ROLE_ERROR_ID =			0x0000000C;
		static constexpr DWORD ROLE_ERROR_TEXT_ID =		0x0000000D;
		static constexpr DWORD ROLE_WARNING_ID =		0x0000000E;
		static constexpr DWORD ROLE_WARNING_TEXT_ID =	0x0000000F;
		static constexpr DWORD ROLE_LINK_ID =			0x00000010;
		static constexpr DWORD ROLE_LINK_TEXT_ID =		0x00000011;
		static constexpr DWORD ROLE_LINK_HOVER_ID =		0x00000012;
		static constexpr DWORD ROLE_LINK_HOVER_TEXT_ID = 0x00000013;
		static constexpr DWORD ROLE_LIGHT_ID =			0x00000014;

		void Copy (const CLuminousColor& Src);

		static CString ComposeThemeColorID (CStringView sShade, int iToneIndex);
		static ERole DecodeRole (DWORD dwValue);
		static EShade DecodeShade (DWORD dwValue);
		static SThemeColor DecodeThemeColor (DWORD dwValue);
		static DWORD EncodeRole (ERole iRole);
		static DWORD EncodeThemeColor (EShade iShade, int iToneIndex);
		static DWORD EncodeShade (EShade iShade);
		static int ToTone (int iToneIndex) { return (iToneIndex + 1) * 10; }
		static int ToToneIndex (int iTone) { ASSERT(iTone >= 0 && iTone <= 100); return Max(0, iTone-1) / 10; }

		EType m_iType = EType::None;
		UValue m_Value = { };
	};

class CLuminousColorTheme
	{
	public:

		static constexpr int MAX_COLORS = 10;

		struct SColorEntry
			{
			CLuminousColor::EShade iShade = CLuminousColor::EShade::None;
			CRGBA32 Colors[MAX_COLORS];
			};

		struct SRoleEntry
			{
			CLuminousColor::ERole iRole = CLuminousColor::ERole::None;
			CLuminousColor::EShade iShade = CLuminousColor::EShade::None;
			int iTone = 0;
			};

		CLuminousColorTheme () { }
		CLuminousColorTheme (const std::initializer_list<SColorEntry>& Shades, const std::initializer_list<SRoleEntry>& Roles);

		CRGBA32 GetColor (CLuminousColor::EShade iColor, int iToneIndex) const;
		CRGBA32 GetColor (CLuminousColor::ERole iRole) const;

	private:

		SColorEntry m_Colors[(size_t)CLuminousColor::EShade::MAX];
		CRGBA32 m_RoleColors[(size_t)CLuminousColor::ERole::MAX];
	};

class CLuminousFillStyle
	{
	public:

		CLuminousFillStyle () { }
		CLuminousFillStyle (const CLuminousColor& Color) :
				m_Color(Color)
			{ }

		static CLuminousFillStyle CreateFromStream (IByteStream& Stream) { CLuminousFillStyle Result; Result.m_Color.Read(Stream); return Result; }

		bool operator== (const CLuminousFillStyle &Src) const;
		bool operator!= (const CLuminousFillStyle &Src) const { return !(*this == Src); }

		const CLuminousColor& GetColor () const { return m_Color; }
		bool IsEmpty () const { return m_Color.IsEmpty(); }
		void Read (IByteStream& Stream) { *this = CreateFromStream(Stream); }
		void SetColor (const CLuminousColor& Color) { m_Color = Color; }
		void Write (IByteStream& Stream) const { m_Color.Write(Stream); }

		static const CLuminousFillStyle Null;

	private:

		CLuminousColor m_Color;
	};

class CLuminousFontStyle
	{
	public:

		enum class EType
			{
			Unknown,

			Normal,
			Italic,
			Oblique
			};

		static constexpr double DEFAULT_SIZE = 18.0;
		static constexpr double DEFAULT_LINE_HEIGHT = 1.2;
		static constexpr int DEFAULT_STRETCH = 100;

		static constexpr int WEIGHT_NORMAL = 400;
		static constexpr int WEIGHT_BOLD = 700;

		CLuminousFontStyle () { }
		CLuminousFontStyle (const CString& sTypeface, double rSize = DEFAULT_SIZE, int iWeight = WEIGHT_NORMAL, EType iStyle = EType::Normal, double rLineHeight = DEFAULT_LINE_HEIGHT, int iStretch = DEFAULT_STRETCH) :
				m_sTypeface(sTypeface),
				m_rSize(rSize),
				m_iWeight(iWeight),
				m_iStyle(iStyle),
				m_rLineHeight(rLineHeight),
				m_iStretch(iStretch)
			{ }

		bool operator== (const CLuminousFontStyle &Src) const = default;
		bool operator!= (const CLuminousFontStyle &Src) const = default;

		CString AsHTML () const;
		double GetLineHeight () const { return m_rLineHeight; }
		double GetSize () const { return m_rSize; }
		CString GetSizeAsHTML () const;
		int GetStretch () const { return m_iStretch; }
		EType GetStyle () const { return m_iStyle; }
		CString GetStyleAsID () const { return GetID(m_iStyle); }
		const CString& GetTypeface () const { return m_sTypeface; }
		int GetWeight () const { return m_iWeight; }
		bool IsEmpty () const { return m_sTypeface.IsEmpty(); }
		void SetLineHeight (double rLineHeight) { m_rLineHeight = rLineHeight; }
		void SetSize (double rSize) { m_rSize = rSize; }
		void SetStretch (int iStretch) { m_iStretch = iStretch; }
		void SetStyle (EType iStyle) { m_iStyle = iStyle; }
		void SetTypeface (const CString& sTypeface) { m_sTypeface = sTypeface; }
		void SetWeight (int iWeight) { m_iWeight = iWeight; }

		static CString GetID (EType iStyle);
		static CLuminousFontStyle ParseHTML (const CString& sValue);
		static bool ParseSize (const CString& sValue, double* retrSize = NULL, double* retiLineHeight = NULL);
		static EType ParseStyle (const CString& sValue);
		static int ParseWeight (const CString& sValue);

		static const CLuminousFontStyle Null;

	private:

		CString m_sTypeface;
		double m_rSize = 0.0;				//	Size in pixels
		int m_iWeight = WEIGHT_NORMAL;
		EType m_iStyle = EType::Normal;
		int m_iStretch = DEFAULT_STRETCH;	//	% stretch
		double m_rLineHeight = DEFAULT_LINE_HEIGHT;
	};

class CLuminousLineStyle
	{
	public:

		enum class ELineCap
			{
			Unknown = 0,

			Butt = 1,				//	A flat edge is added  to each end of the line.
			Round = 2,				//	A rounded end cap is added
			Square = 3,				//	A square end cap is added
			};

		enum class ELineJoin
			{
			Unknown = 0,

			Bevel = 1,				//	A beveled corner
			Round = 2,				//	A rounded corner
			Miter = 3,				//	Creates a sharp corner
			};

		CLuminousLineStyle () { }
		CLuminousLineStyle (const CLuminousColor& Color, 
							double rWidth = 1.0, 
							ELineJoin iLineJoin = ELineJoin::Miter,
							ELineCap iLineCap = ELineCap::Butt,
							double rMiterLimit = 10.0) :
				m_Color(Color),
				m_rLineWidth(rWidth),
				m_iLineJoin(iLineJoin),
				m_iLineCap(iLineCap),
				m_rMiterLimit(rMiterLimit)
			{ }

		static CLuminousLineStyle CreateFromStream (IByteStream& Stream);

		bool operator== (const CLuminousLineStyle &Src) const;
		bool operator!= (const CLuminousLineStyle &Src) const { return !(*this == Src); }

		const CLuminousColor& GetColor () const { return m_Color; }
		ELineCap GetLineCap () const { return m_iLineCap; }
		ELineJoin GetLineJoin () const { return m_iLineJoin; }
		double GetLineWidth () const { return m_rLineWidth; }
		double GetMiterLimit () const { return m_rMiterLimit; }
		bool IsEmpty () const { return m_Color.IsEmpty(); }
		void Read (IByteStream& Stream) { *this = CreateFromStream(Stream); }
		void SetColor (const CLuminousColor& Color) { m_Color = Color; }
		void SetLineWidth (double rWidth) { m_rLineWidth = Max(0.0, rWidth); }
		void Write (IByteStream& Stream) const;

		static const CLuminousLineStyle Null;

	private:

		CLuminousColor m_Color;
		double m_rLineWidth = 1.0;
		ELineCap m_iLineCap = ELineCap::Butt;
		ELineJoin m_iLineJoin = ELineJoin::Miter;
		double m_rMiterLimit = 10.0;
	};

class CLuminousShadowStyle
	{
	public:

		CLuminousShadowStyle () { }
		CLuminousShadowStyle (const CLuminousColor& Color, double rBlur, const CVector2D& vOffset) :
				m_Color(Color),
				m_rBlur(rBlur),
				m_rOffsetX(vOffset.X()),
				m_rOffsetY(vOffset.Y())
			{ }

		bool operator== (const CLuminousShadowStyle &Src) const;
		bool operator!= (const CLuminousShadowStyle &Src) const { return !(*this == Src); }

		double GetBlur () const { return m_rBlur; }
		const CLuminousColor& GetColor () const { return m_Color; }
		CVector2D GetOffset () const { return CVector2D(m_rOffsetX, m_rOffsetY); }
		bool IsEmpty () const { return m_Color.IsEmpty(); }
		void SetColor (const CLuminousColor& Color) { m_Color = Color; }

		static const CLuminousShadowStyle Null;

	private:

		CLuminousColor m_Color;
		double m_rBlur = 0.0;
		double m_rOffsetX = 0.0;
		double m_rOffsetY = 0.0;
	};

class CLuminousTextAlign
	{
	public:

		enum class EAlign
			{
			Unknown,

			Start,						//	Left for LtR text, Right otherwise
			End,						//	Right for LtR text, Left otherwise
			Left,
			Center,
			Right,
			};

		enum class EBaseline
			{
			Unknown,

			Alphabetic,					//	Alphabetic baseline
			Top,
			Middle,
			Bottom,

			Hanging,					//	Used by Tibetan and Indic scripts
			Ideographic,				//	Used by Chinese, Japanse, Korean
			};

		enum class EDirection
			{
			Unknown,

			Default,					//	Inherit from parent/env
			LtR,						//	Left-to-Right
			RtL,						//	Right-to-Left
			};

		CLuminousTextAlign (EAlign iAlign = EAlign::Start, EBaseline iBaseline = EBaseline::Alphabetic, EDirection iDirection = EDirection::Default) :
				m_iAlign(iAlign),
				m_iBaseline(iBaseline),
				m_iDirection(iDirection)
			{ }

		bool operator== (const CLuminousTextAlign &Src) const = default;
		bool operator!= (const CLuminousTextAlign &Src) const = default;

		EAlign GetAlign () const { return m_iAlign; }
		CString GetAlignAsHTML () const { return GetAlignAsHTML(m_iAlign); }
		EBaseline GetBaseline () const { return m_iBaseline; }
		CString GetBaselineAsHTML () const { return GetBaselineAsHTML(m_iBaseline); }
		EDirection GetDirection () const { return m_iDirection; }
		CString GetDirectionAsHTML () const { return GetDirectionAsHTML(m_iDirection); }
		bool IsEmpty () const { m_iAlign == EAlign::Start && m_iBaseline == EBaseline::Alphabetic && m_iDirection == EDirection::Default; }
		void SetAlign (EAlign iAlign) { m_iAlign = iAlign; }
		void SetBaseline (EBaseline iBaseline) { m_iBaseline = iBaseline; }
		void SetDirection (EDirection iDirection) { m_iDirection = iDirection; }

		static CString GetAlignAsHTML (EAlign iAlign);
		static CString GetBaselineAsHTML (EBaseline iBaseline);
		static CString GetDirectionAsHTML (EDirection iDirection);
		static EAlign ParseAlign (const CString& sValue);
		static EBaseline ParseBaseline (const CString& sValue);
		static EDirection ParseDirection (const CString& sValue);

		static const CLuminousTextAlign Null;

	private:

		EAlign m_iAlign = EAlign::Start;
		EBaseline m_iBaseline = EBaseline::Alphabetic;
		EDirection m_iDirection = EDirection::Default;
	};

class CLuminousPath2D
	{
	public:

		enum class ESegmentType
			{
			None,

			ArcClockwise,
			ArcCounterClockwise,
			ArcTo,
			ClosePath,
			LineTo,
			MoveTo,
			Rect,
			};

		bool operator== (const CLuminousPath2D &Src) const;
		bool operator!= (const CLuminousPath2D &Src) const { return !(*this == Src); }

		void Arc (const CVector2D& vCenter, double rRadius, double rStartAngle, double rEndAngle, bool bCounterClockwise = false);
		void ArcTo (const CVector2D& v1stTangent, const CVector2D& v2ndTangent, double rRadius);
		void ClosePath ();
		void DeleteAll () { m_Path.DeleteAll(); }
		void GetArc (int iIndex, CVector2D& retCenter, double& retRadius, double& retStartAngle, double& retEndAngle, bool& retCounterClockwise) const;
		void GetArcTo (int iIndex, CVector2D& retTangent1, CVector2D& retTangent2, double& retRadius) const;
		CVector2D GetLineTo (int iIndex) const { if (iIndex >= 0 && iIndex < m_Path.GetCount() && m_Path[iIndex].iType == ESegmentType::LineTo) return m_Path[iIndex].A; else throw CException(errFail); }
		CVector2D GetMoveTo (int iIndex) const { if (iIndex >= 0 && iIndex < m_Path.GetCount() && m_Path[iIndex].iType == ESegmentType::MoveTo) return m_Path[iIndex].A; else throw CException(errFail); }
		void GetRect (int iIndex, CVector2D& retUL, CVector2D& retLR) const;
		int GetSegmentCount () const { return m_Path.GetCount(); }
		ESegmentType GetSegmentType (int iIndex) const { if (iIndex >= 0 && iIndex < m_Path.GetCount()) return m_Path[iIndex].iType; else throw CException(errFail); }
		bool IsEmpty () const { return m_Path.GetCount() == 0; }
		void LineTo (const CVector2D& vPos);
		void MoveTo (const CVector2D& vPos);
		void Rect (const CVector2D& vUL, const CVector2D& vLR);

		static const CLuminousPath2D Null;

	private:

		//	We encode the various parameter for each segment type:
		//
		//	ArcClockwise
		//	ArcCounterClockwise
		//
		//		A = Center
		//		B.x = Radius
		//		C.x = Start angle (radians)
		//		C.y = End angle (radians)
		//
		//	ArcTo
		//
		//		A = 1st Tangent
		//		B = 2nd Tangent
		//		C.x = Radius
		//
		//	ClosePath
		//
		//	LineTo
		//
		//		A = pos
		//
		//	MoveTo
		//
		//		A = pos
		//
		//	Rect
		//
		//		A = Upper-left corner
		//		B = Lower-right corner

		struct SSegment
			{
			bool operator== (const SSegment &Src) const { return (iType == Src.iType) && (A == Src.A) && (B == Src.B) && (C == Src.C); }
			bool operator!= (const SSegment &Src) const { return !(*this == Src); }

			ESegmentType iType = ESegmentType::None;
			CVector2D A;
			CVector2D B;
			CVector2D C;
			};

		TArray<SSegment> m_Path;
	};

