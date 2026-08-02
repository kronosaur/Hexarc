//	CelestialDataset.h
//
//	CelestialCore header file
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class ICelestialDataset
    {
    public:
        enum EConstants
            {
            MAX_STAR_NAME = 16,
            };

		enum EDeepSkyObjectType
			{
			dsoUnknown =					-1,

			dsoGalaxy =						0,
			dsoOpenCluster =				1,
			dsoGlobularCluster =			2,
			dsoPlanetaryNebula =			3,
			dsoDiffuseNebula =				4,
			dsoSupernovaRemnant =			5,
			dsoStarCloud =					6,
			dsoStellarObject =				7,

			dsoTypeCount =					8,
			};

        struct SStarEntry
            {
            TFixedString<MAX_STAR_NAME> sName;
            CCelestialCoord Pos;            //  Position (RA, Dec in radians)
            CStarMagnitude Mag;             //  B-V magnitude
            };

		struct SStarNameEntry
			{
			CString sID;					//	"Alp Cen"
			CString sShortName;				//	"{Alpha}"
			CString sLongName;				//	"Alpha Centauri"
			CString sProperName;			//	"Rigil Kent"
            CCelestialCoord Pos;            //  Position (RA, Dec in radians)
            CStarMagnitude LimitingMag;     //  B-V magnitude
			};

		struct SLineEntry
			{
			CCelestialCoord From;
			CCelestialCoord To;
            CStarMagnitude LimitingMag;
			};

		struct SConstellationEntry
			{
			CString sShortName;
			CString sLongName;
			TArray<CCelestialCoord> Boundary;
			TArray<SLineEntry> Lines;
			};

		struct SDeepSkyEntry
			{
			CString sShortName;				//	"M104"
			CString sLongName;				//	"Messier 104"
			CString sProperName;			//	"Sombrero Galaxy"
			CString sDesignation;			//	"NGC 4594"
			CString sConstellation;			//	Constellation
            CCelestialCoord Pos;            //  Position (RA, Dec in radians)
            CStarMagnitude LimitingMag;     //  B-V magnitude
			EDeepSkyObjectType iType;
			double rMajorAxis;				//	In arcseconds
			double rMinorAxis;
			int iAngle;						//	For galaxies, the angle of the major axis
			};

		struct SMilkyWayPatchEntry
			{
			CCelestialCoord Pos;
			int iIntensity;					//	1-10
			};

		struct SChartData
			{
			TArray<SStarEntry> Stars;
			TArray<SStarNameEntry> StarNames;
			TSortMap<CString, SConstellationEntry> Constellations;
			TArray<SDeepSkyEntry> DSOs;
			TArray<SMilkyWayPatchEntry> MilkyWay;
			};

		struct SChartOptions
			{
			SChartOptions (void) :
					Center(0.0, 0.0),
					rViewRadius(1.5 * 60.0 * 60.0),
					LimitingMag(6.0),
					LimitingMagDSOs(0.0)
				{ }

			CCelestialCoord Center;
			double rViewRadius;				//	In radians
			CStarMagnitude LimitingMag;		//	Limiting magnitude
			CStarMagnitude LimitingMagDSOs;	//	Limiting magnitude for DSOs
			CCelestialObjectSet Highlights;
			};

        virtual ~ICelestialDataset (void) { }

        virtual void AccumulateData (const SChartOptions &Options, SChartData &retData) const { }
		virtual bool FindStarByID (const CString &sID, SStarEntry *retStar = NULL) const { return false; }
        virtual CString GetName (void) const = 0;
        virtual bool Open (CCelestial &Celestial, const CString &sFilespec, IProgressEvents *pProgress = NULL, CString *retsError = NULL) = 0;
		virtual void Select (const TArray<CString> &Objs, CCelestialObjectSet &Selection) const { }

    };


