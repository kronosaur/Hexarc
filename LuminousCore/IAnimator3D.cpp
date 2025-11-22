//	IAnimator3D.cpp
//
//	IAnimator3D Class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(TYPE_BLINK,					"blink");
DECLARE_CONST_STRING(TYPE_CONSTANT,					"constant");
DECLARE_CONST_STRING(TYPE_LINEAR,					"linear");

TArray<bool> IAnimator3D::m_NullBool;
TArray<CLuminousColor> IAnimator3D::m_NullColor;
TArray<double> IAnimator3D::m_NullScalar;
TArray<CString> IAnimator3D::m_NullString;
TArray<CVector3D> IAnimator3D::m_NullVector;
IAnimator3D::SKeyframeDesc IAnimator3D::m_NullKeyframe;

CString IAnimator3D::AsID (Type iType)
	{
	switch (iType)
		{
		case Type::Blink:
			return TYPE_BLINK;

		case Type::Constant:
			return TYPE_CONSTANT;

		case Type::Linear:
			return TYPE_LINEAR;

		default:
			throw CException(errFail);
		}
	}

IAnimator3D::Type IAnimator3D::AsType (const CString& sID)
	{
	if (strEqualsNoCase(sID, TYPE_BLINK))
		return Type::Blink;
	else if (strEqualsNoCase(sID, TYPE_CONSTANT))
		return Type::Constant;
	else if (strEqualsNoCase(sID, TYPE_LINEAR))
		return Type::Linear;
	else
		return Type::Unknown;
	}

TUniquePtr<IAnimator3D> IAnimator3D::CreateFromStream (IByteStream& Stream)

//	CreateFromStream
//
//	Read from a stream.

	{
	DWORD dwImplID = Stream.ReadDWORD();

	CString sID = CString::Deserialize(Stream);
	Obj3DProp iProp = ILuminousObj3D::ParseProperty(sID);
	if (iProp == Obj3DProp::Unknown)
		throw CException(errFail);

	TUniquePtr<IAnimator3D> pAnimator;
	switch (dwImplID)
		{
		case IMPL_BOOL:
			pAnimator = TUniquePtr<IAnimator3D>(new CBoolAnimator3D(iProp));
			break;

		case IMPL_COLOR:
			pAnimator = TUniquePtr<IAnimator3D>(new CColorAnimator3D(iProp));
			break;

		case IMPL_SCALAR:
			pAnimator = TUniquePtr<IAnimator3D>(new CScalarAnimator3D(iProp));
			break;

		case IMPL_STRING:
			pAnimator = TUniquePtr<IAnimator3D>(new CStringAnimator3D(iProp));
			break;

		case IMPL_VECTOR:
			pAnimator = TUniquePtr<IAnimator3D>(new CVectorAnimator3D(iProp));
			break;

		default:
			throw CException(errFail);
		}

	//	Read the keyframes

	int iCount = Stream.ReadDWORD();
	pAnimator->m_Keyframes.InsertEmpty(iCount);
	for (int i = 0; i < iCount; i++)
		{
		pAnimator->m_Keyframes[i].iFrame = Stream.ReadInt();
		CString sType = CString::Deserialize(Stream);
		pAnimator->m_Keyframes[i].iType = AsType(sType);
		if (pAnimator->m_Keyframes[i].iType == Type::Unknown)
			throw CException(errFail);

		pAnimator->m_Keyframes[i].iBlinkInterval = Stream.ReadInt();
		}

	pAnimator->OnRead(Stream);

	return pAnimator;
	}

DWORD IAnimator3D::GetImplID () const

//	GetImplID
//
//	Returns the implementation ID.

	{
	switch (GetPropertyType())
		{
		case Obj3DPropType::Bool:
			return IMPL_BOOL;

		case Obj3DPropType::Color:
			return IMPL_COLOR;

		case Obj3DPropType::Scalar:
			return IMPL_SCALAR;

		case Obj3DPropType::String:
			return IMPL_STRING;

		case Obj3DPropType::Vector:
			return IMPL_VECTOR;

		default:
			throw CException(errFail);
		}
	}

void IAnimator3D::Write (IByteStream& Stream) const

//	Write
//
//	Write to a stream.

	{
	Stream.Write(GetImplID());
	ILuminousObj3D::GetPropertyDesc(m_iProp).sID.Serialize(Stream);

	Stream.Write(m_Keyframes.GetCount());
	for (int i = 0; i < m_Keyframes.GetCount(); i++)
		{
		Stream.Write(m_Keyframes[i].iFrame);
		AsID(m_Keyframes[i].iType).Serialize(Stream);
		Stream.Write(m_Keyframes[i].iBlinkInterval);
		}

	OnWrite(Stream);
	}
