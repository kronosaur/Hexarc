//	IAnimator2D.cpp
//
//	IAnimator2D Class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(TYPE_BLINK,					"blink");
DECLARE_CONST_STRING(TYPE_CONSTANT,					"constant");
DECLARE_CONST_STRING(TYPE_LINEAR,					"linear");

TArray<bool> IAnimator2D::m_NullBool;
TArray<CLuminousColor> IAnimator2D::m_NullColor;
TArray<double> IAnimator2D::m_NullScalar;
TArray<CString> IAnimator2D::m_NullString;
TArray<CVector2D> IAnimator2D::m_NullVector;
IAnimator2D::SKeyframeDesc IAnimator2D::m_NullKeyframe;

CString IAnimator2D::AsID (Type iType)
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

IAnimator2D::Type IAnimator2D::AsType (const CString& sID)
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

TUniquePtr<IAnimator2D> IAnimator2D::CreateFromStream (IByteStream& Stream)

//	CreateFromStream
//
//	Read from a stream.

	{
	DWORD dwImplID = Stream.ReadDWORD();

	CString sID = CString::Deserialize(Stream);
	Obj2DProp iProp = ILuminousObj2D::ParseProperty(sID);
	if (iProp == Obj2DProp::Unknown)
		throw CException(errFail);

	TUniquePtr<IAnimator2D> pAnimator;
	switch (dwImplID)
		{
		case IMPL_BOOL:
			pAnimator = TUniquePtr<IAnimator2D>(new CBoolAnimator2D(iProp));
			break;

		case IMPL_COLOR:
			pAnimator = TUniquePtr<IAnimator2D>(new CColorAnimator2D(iProp));
			break;

		case IMPL_SCALAR:
			pAnimator = TUniquePtr<IAnimator2D>(new CScalarAnimator2D(iProp));
			break;

		case IMPL_STRING:
			pAnimator = TUniquePtr<IAnimator2D>(new CStringAnimator2D(iProp));
			break;

		case IMPL_VECTOR:
			pAnimator = TUniquePtr<IAnimator2D>(new CVectorAnimator2D(iProp));
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

DWORD IAnimator2D::GetImplID () const

//	GetImplID
//
//	Returns the implementation ID.

	{
	switch (GetPropertyType())
		{
		case ObjPropType::Bool:
			return IMPL_BOOL;

		case ObjPropType::Color:
			return IMPL_COLOR;

		case ObjPropType::Scalar:
			return IMPL_SCALAR;

		case ObjPropType::String:
			return IMPL_STRING;

		case ObjPropType::Vector:
			return IMPL_VECTOR;

		default:
			throw CException(errFail);
		}
	}

void IAnimator2D::Write (IByteStream& Stream) const

//	Write
//
//	Write to a stream.

	{
	Stream.Write(GetImplID());
	ILuminousObj2D::GetPropertyDesc(m_iProp).sID.Serialize(Stream);

	Stream.Write(m_Keyframes.GetCount());
	for (int i = 0; i < m_Keyframes.GetCount(); i++)
		{
		Stream.Write(m_Keyframes[i].iFrame);
		AsID(m_Keyframes[i].iType).Serialize(Stream);
		Stream.Write(m_Keyframes[i].iBlinkInterval);
		}

	OnWrite(Stream);
	}
