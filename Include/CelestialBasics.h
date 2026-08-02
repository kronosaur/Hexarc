//	CelestialBasics.h
//
//	CelestialCore header file
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CCelestial;
class ICelestialDataset;

class CCelestialCoord
    {
    public:
        //  Optimize empty constructor

        CCelestialCoord (void) { }

        CCelestialCoord (double rRA, double rDec) :
                m_rRA(rRA),
                m_rDec(rDec)
            { }

        inline double GetDec (void) const { return m_rDec; }
        inline double GetRA (void) const { return m_rRA; }
        inline void SetDec (double rDec) { m_rDec = rDec; }
        inline void SetDecDegrees (double rValue) { m_rDec = mathDegreesToRadians(rValue); }
        inline void SetRA (double rRA) { m_rRA = rRA; }
        inline void SetRADegrees (double rValue) { m_rRA = mathDegreesToRadians(rValue); }

        static double ArcDistance (const CCelestialCoord &C1, const CCelestialCoord &C2);
        inline static double ArcsecondsToRadians (double rValue) { return 4.848136790e-6 * rValue; }
        inline static double DegreesToRadians (double rValue) { return 0.0174532925 * rValue; }

    private:
        double m_rRA;						//	Radians
        double m_rDec;
    };

class CStarMagnitude
    {
    public:
        CStarMagnitude (void) { }

        CStarMagnitude (double rV) :
                m_rB((float)rV),
                m_rV((float)rV)
            { }

        CStarMagnitude (double rB, double rV) :
                m_rB((float)rB),
                m_rV((float)rV)
            { }

        inline operator double () const { return m_rV; }

		CRGBA32 CalcRGBColor (void) const;
        inline double GetBlue (void) const { return m_rB; }
        inline double GetVisual (void) const { return m_rV; }
        inline void SetBlue (double rValue) { m_rB = (float)rValue; }
        void SetTychoMag (double rBT, double rVT);
        inline void SetVisual (double rValue) { m_rV = (float)rValue; }

    private:
		struct SColorDesc
			{
			double rMinBV;					//	If we're >= this B-V, then this color
			CRGBA32 rgbColor;
			};

        float m_rB;
        float m_rV;

		static SColorDesc COLOR_TABLE[];
		static int COLOR_TABLE_COUNT;
    };

class CCelestialObjectSet
	{
	public:
		CLargeSet &GetSet (const ICelestialDataset *pDataset) { return *m_Sets.SetAt((DWORD_PTR)pDataset); }
		const CLargeSet *GetSet (const ICelestialDataset *pDataset) const { return m_Sets.GetAt((DWORD_PTR)pDataset); }

	private:
		TSortMap<DWORD_PTR, CLargeSet> m_Sets;
	};
