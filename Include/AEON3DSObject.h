//	AEON3DSObject.h
//
//	3D Studio file AEON integration
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "AEON.h"

class C3DSFile
	{
	public:

		struct SExtents
			{
			CVector3D vMin;
			CVector3D vMax;
			bool bValid = false;
			};

		struct SResource
			{
			CString sPath;
			CString sMaterial;
			CString sChannel;
			DWORD dwTiling = 0;
			double rUScale = 1.0;
			double rVScale = 1.0;
			double rUOffset = 0.0;
			double rVOffset = 0.0;
			};

		bool Init (const IMemoryBlock64& Data, CString* retsError = NULL);

		int GetFaceCount () const { return m_iFaceCount; }
		int GetMaterialCount () const { return m_iMaterialCount; }
		int GetMeshVersion () const { return m_iMeshVersion; }
		double GetMasterScale () const { return m_rMasterScale; }
		int GetObjectCount () const { return m_iObjectCount; }
		const TArray<SResource>& GetResources () const { return m_Resources; }
		const SExtents& GetRawExtents () const { return m_RawExtents; }
		SExtents GetScaledExtents () const;
		int GetVersion () const { return m_iVersion; }
		int GetVertexCount () const { return m_iVertexCount; }
		bool HasKeyframes () const { return m_bHasKeyframes; }

	private:

		struct SChunk
			{
			WORD wID = 0;
			DWORD dwStart = 0;
			DWORD dwDataStart = 0;
			DWORD dwEnd = 0;
			};

		void AccumulatePoint (double x, double y, double z);
		bool ParseMain (const SChunk& Chunk, CString* retsError);
		bool ParseMap (const SChunk& Chunk, const CString& sChannel, SResource& retResource, CString* retsError) const;
		bool ParseMaterial (const SChunk& Chunk, CString* retsError);
		bool ParseMeshData (const SChunk& Chunk, CString* retsError);
		bool ParseNamedObject (const SChunk& Chunk, CString* retsError);
		bool ParsePointArray (const SChunk& Chunk, CString* retsError);
		bool ParseTriangleObject (const SChunk& Chunk, CString* retsError);
		bool ReadChunk (DWORD dwPos, DWORD dwParentEnd, SChunk& retChunk, CString* retsError) const;
		bool ReadFloat (DWORD dwPos, DWORD dwEnd, float& retrValue, CString* retsError) const;
		bool ReadString (DWORD dwPos, DWORD dwEnd, CString& retsValue, DWORD& retdwNext, CString* retsError) const;
		bool ReadUInt16 (DWORD dwPos, DWORD dwEnd, WORD& retwValue, CString* retsError) const;
		bool ReadUInt32 (DWORD dwPos, DWORD dwEnd, DWORD& retdwValue, CString* retsError) const;
		bool SetError (CString* retsError, const CString& sError) const;
		bool SetInvalidChunkError (const SChunk& Chunk, CString* retsError) const;

		const BYTE* m_pData = NULL;
		DWORD m_dwDataSize = 0;

		SExtents m_RawExtents;
		TArray<SResource> m_Resources;
		double m_rMasterScale = 1.0;
		int m_iVersion = 0;
		int m_iMeshVersion = 0;
		int m_iObjectCount = 0;
		int m_iMaterialCount = 0;
		int m_iVertexCount = 0;
		int m_iFaceCount = 0;
		bool m_bHasKeyframes = false;
	};

class CAEON3DSObject : public TExternalDatum<CAEON3DSObject>
	{
	public:

		CAEON3DSObject () { }

		static CDatum Create (CDatum dFileData);
		static CDatum CreateAsType (CDatum dValue) { return Create(dValue); }
		static TArray<IDatatype::SMemberDesc> GetMembers ();
		static const CString& StaticGetTypename ();

		CDatum GetRawData () const { return m_dRawData.Clone(CDatum::EClone::CopyOnWrite); }

		//	IComplexDatum

		virtual CString AsString () const override;
		virtual size_t CalcMemorySize () const override;
		virtual IComplexDatum* Clone (CDatum::EClone iMode) const override { return new CAEON3DSObject(*this); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeAEONObject; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString& sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString& sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl (CDatum dObj, const CString& sMethod, IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual bool IsNil () const override { return m_dRawData.IsNil(); }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override;

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString& sTypename, IByteStream& Stream) override;
		virtual void OnMarked () override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream& Stream) const override;

	private:

		bool Init (CDatum dFileData, CString* retsError = NULL);
		CDatum MakeExtents (const C3DSFile::SExtents& Extents) const;
		CDatum MakeResources () const;

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		CDatum m_dRawData;
		CDatum m_dExtents;
		CDatum m_dRawExtents;
		CDatum m_dResources;
		C3DSFile m_File;

		static TDatumPropertyHandler<CAEON3DSObject> m_Properties;
		static TDatumMethodHandler<CAEON3DSObject> m_Methods;
	};
