//	CelestialCore.h
//
//	CelestialCore header file
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "Foundation.h"
#include "LuminousCore.h"
#include "CelestialBasics.h"
#include "CelestialDataset.h"

class CCelestial
    {
    public:
        enum EDatasets
            {
            formatNone,
			formatBSCStarNames,
			formatConstellationLines,
			formatDSOs,
            formatTycho2,
            };

		bool FindStarByID (const CString &sID, ICelestialDataset::SStarEntry *retStar = NULL) const;
        inline int GetDatasetCount (void) const { return m_Datasets.GetCount(); }
        void GetChartData (const ICelestialDataset::SChartOptions &Options, ICelestialDataset::SChartData &retData) const;
        bool OpenDataset (EDatasets iDataset, const CString &sFilespec, IProgressEvents *pProgress = NULL, CString *retsError = NULL);
		void SelectObjects (const TArray<CString> &Objs, CCelestialObjectSet &Selection) const;

		static const CString &DSOTypeToID (ICelestialDataset::EDeepSkyObjectType iType);
		static bool FindConstellationByAbbreviation (const CString &sAbbr, int *retiIndex = NULL);
		static bool FindGreekLetterByAbbreviation (const CString &sAbbr, int *retiIndex = NULL);
		static const CString &GetConstellationGenitive (int iIndex);
		static const CString &GetConstellationName (int iIndex);
		static const CString &GetGreekLetter (int iIndex);

    private:
        struct SDataset
            {
            SDataset (void) :
                    pDataset(NULL)
                { }

            ~SDataset (void)
                {
                if (pDataset)
                    delete pDataset;
                }

            EDatasets iDataset;
            CString sFilespec;
            ICelestialDataset *pDataset;
            };

		struct SConstellationName
			{
			SConstellationName (const char *pszAbbreviationArg, const char *pszNameArg, const char *pszGenitiveArg) :
					pszAbbreviation(pszAbbreviationArg),
					sName(pszNameArg),
					sGenitive(pszGenitiveArg)
				{ }

			const char *pszAbbreviation;	//	"And"
			CString sName;					//	"Andromeda"
			CString sGenitive;				//	"Andromedae"
			};

		struct SDSOTypeData
			{
			SDSOTypeData (const char *pszType) :
					sID(pszType)
				{ }

			CString sID;
			};

		struct SGreekLetter
			{
			SGreekLetter (const char *pszLetterArg, const char *pszSymbolArg, const char *pszNameArg, const char *pszHTMLArg) :
					sLetter(pszLetterArg),
					pszSymbol(pszSymbolArg),
					sName(pszNameArg),
					sHTML(pszHTMLArg)
				{ }

			CString sLetter;				//	{alpha character}
			const char *pszSymbol;			//	"alp"
			CString sName;					//	"alpha"
			CString sHTML;					//	"&alpha;"
			};

        TArray<SDataset> m_Datasets;

		static SConstellationName CONSTELLATION[];
		static int CONSTELLATION_COUNT;
		static SDSOTypeData DSO_TYPE_DATA[ICelestialDataset::dsoTypeCount];
		static SGreekLetter GREEK_ALPHABET[];
		static int GREEK_ALPHABET_COUNT;
    };