//	SciAstro.h
//
//	SciAstro Classes
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "Foundation.h"
#include "AEON.h"

class CAEONNBodyIntegrator : public TExternalDatum<CAEONNBodyIntegrator>
	{
	public:

		enum class EUnitSystem
			{
			Unknown,

			AU_Day_mu,
			AU_Year_mu,
			};

		enum class EIntegrator
			{
			Unknown,

			Euler,
			VelocityVerlet,
			AdaptiveRungeKutta4,
			};

		enum class EReferenceFrame
			{
			Unknown,

			Barycentric,						//	Center of mass of the system.
			Heliocentric,						//	Center of mass of the sun.
			};

		CAEONNBodyIntegrator () { }

		static CDatum Create (CDatum dOptions);

		void Advance ();
		CDatum AdvanceAndGenerateTrajectory (int iFrames);
		double AsDistanceFromAU (double rValue) const;
		double AsMUFromSolarMass (double rValue) const;
		double AsTime (const CTimeSpan& Time) const;

		//	IComplexDatum

		virtual CString AsString () const override { return strPattern("NBody Integrator"); }
		virtual size_t CalcMemorySize () const override { return 0; }
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeAEONObject; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString& sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString& sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl (CDatum dObj, const CString& sMethod, IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override 
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual bool IsNil () const override { return false; }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override;
		virtual void SetElement (const CString& sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

		static CString AsID (EIntegrator iIntegrator);
		static CString AsID (EReferenceFrame iFrame);
		static CString AsID (EUnitSystem iUnitSystem);
		static TArray<IDatatype::SMemberDesc> GetMembers ();
		static EUnitSystem ParseUnitSystem (CStringView sValue);
		static EIntegrator ParseIntegrator (CStringView sValue);
		static EReferenceFrame ParseReferenceFrame (CStringView sValue);
		static const CString& StaticGetTypename (void);

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString& sTypename, IByteStream& Stream) override;
		virtual void OnMarked (void) override { m_dBodies.Mark(); }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream& Stream) const override;

	private:

		static constexpr double G_SOLAR_MASS_MKS = 1.3271244e20;	//	Gravitational constant * solar mass (m^3/s^2)

		static constexpr double METERS_PER_AU = 149597870700.0;

		static constexpr double SECONDS_PER_DAY = 86400.0;
		static constexpr double SECONDS_PER_YEAR = 31557600.0;	//	365.25 days

		TArray<CVector3D> ComputeAccel (const TArray<double>& MU, const TArray<CVector3D>& Positions) const;
		void ComputeVelocityVerlet (const TArray<double>& MU, const TArray<CVector3D>& Positions, const TArray<CVector3D>& Velocities, TArray<CVector3D>& retNewPositions, TArray<CVector3D>& retNewVelocities) const;
		bool InitInterfaces ();

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override;

		EUnitSystem m_iUnitSystem = EUnitSystem::Unknown;
		EIntegrator m_iIntegrator = EIntegrator::Unknown;
		EReferenceFrame m_iReferenceFrame = EReferenceFrame::Unknown;
		double m_time = 0.0;					//	Current time.
		double m_dt = 0.0;						//	Time step to use for integration.
		double m_epsilon = 0.0;					//	Softening parameter (for close encounters).

		int m_substepsPerFrame = 1;				//	Number of substeps to perform per frame (for display purposes).
		double m_dtMin = 0.0;					//	Minimum time step (for adaptive integrators).
		double m_dtMax = 0.0;					//	Maximum time step (for adaptive integrators).
		double m_relErrorTolerance = 0.0;		//	Error tolerance (for adaptive integrators).
		double m_absErrorTolerance = 0.0;		//	Absolute error tolerance (for adaptive integrators).
		int m_maxStepsPerFrame = 1000;			//	Maximum number of steps to perform per frame (for display purposes).

		CDatum m_dBodies;

		//	Interfaces for quick access to data. Call InitInterfaces() before using.

		IAEONTable* m_pBodies = NULL;
		TArray<double> *m_pMU = NULL;
		TArray<CVector3D> *m_pPositions = NULL;
		TArray<CVector3D>* m_pVelocities = NULL;

		static TDatumPropertyHandler<CAEONNBodyIntegrator> m_Properties;
		static TDatumMethodHandler<CAEONNBodyIntegrator> m_Methods;
	};

class CSciAstro
	{
	public:

		static bool Boot ();

		static DWORD NBODY_INTEGRATOR_TYPE;
		static DWORD NBODY_SCHEMA;
		static DWORD NBODY_TRAJECTORY_SCHEMA;

		static int NBODY_MU_COLUMN_INDEX;
		static int NBODY_POS_COLUMN_INDEX;
		static int NBODY_VEL_COLUMN_INDEX;

		static int NBODY_TRAJECTORY_FRAME_COLUMN_INDEX;
		static int NBODY_TRAJECTORY_TIME_COLUMN_INDEX;
		static int NBODY_TRAJECTORY_INDEX_COLUMN_INDEX;
		static int NBODY_TRAJECTORY_POS_COLUMN_INDEX;
		static int NBODY_TRAJECTORY_VEL_COLUMN_INDEX;

	private:

		static bool m_bRegistered;
	};
