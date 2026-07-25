#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

namespace EffectOrientation
{
    inline _vector Project_DirectionOnPlane(_vector vDirection, _vector vPlaneNormal)
    {
        return vDirection -
            vPlaneNormal * XMVectorGetX(XMVector3Dot(vDirection, vPlaneNormal));
    }

    inline _matrix Make_UpAlignedRotation(_vector vUp, _matrix BaseRotation)
    {
        vUp = XMVector3Normalize(vUp);

        _vector vLook = Project_DirectionOnPlane(BaseRotation.r[2], vUp);
        _vector vRight = XMVectorZero();

        if (XMVectorGetX(XMVector3LengthSq(vLook)) <= Helper::fEpsilon)
        {
            vRight = Project_DirectionOnPlane(BaseRotation.r[0], vUp);

            if (XMVectorGetX(XMVector3LengthSq(vRight)) <= Helper::fEpsilon)
            {
                const _vector vReference =
                    fabsf(XMVectorGetX(XMVector3Dot(vUp, XMVectorSet(0.f, 1.f, 0.f, 0.f)))) < 0.999f
                    ? XMVectorSet(0.f, 1.f, 0.f, 0.f)
                    : XMVectorSet(0.f, 0.f, 1.f, 0.f);

                vRight = XMVector3Cross(vReference, vUp);
            }

            vRight = XMVector3Normalize(vRight);
            vLook = XMVector3Normalize(XMVector3Cross(vRight, vUp));
        }
        else
        {
            vLook = XMVector3Normalize(vLook);
            vRight = XMVector3Normalize(XMVector3Cross(vUp, vLook));
            vLook = XMVector3Normalize(XMVector3Cross(vRight, vUp));
        }

        _matrix Rotation = XMMatrixIdentity();
        Rotation.r[0] = XMVectorSetW(vRight, 0.f);
        Rotation.r[1] = XMVectorSetW(vUp, 0.f);
        Rotation.r[2] = XMVectorSetW(vLook, 0.f);
        return Rotation;
    }

    inline _float4x4 Make_ConstrainedBillboardWorldMatrix(
        const _float4x4& WorldMatrix,
        _vector vWorldUp,
        const _float4x4& ViewMatrix)
    {
        if (XMVectorGetX(XMVector3LengthSq(vWorldUp)) <= Helper::fEpsilon)
            return WorldMatrix;

        _matrix BillboardMatrix = XMLoadFloat4x4(&WorldMatrix);

        const _float fScaleX = XMVectorGetX(XMVector3Length(BillboardMatrix.r[0]));
        const _float fScaleY = XMVectorGetX(XMVector3Length(BillboardMatrix.r[1]));
        const _float fScaleZ = XMVectorGetX(XMVector3Length(BillboardMatrix.r[2]));

        vWorldUp = XMVector3Normalize(vWorldUp);

        const _matrix InverseViewMatrix = XMMatrixInverse(
            nullptr,
            XMLoadFloat4x4(&ViewMatrix));

        _vector vLook = Project_DirectionOnPlane(InverseViewMatrix.r[2], vWorldUp);

        if (XMVectorGetX(XMVector3LengthSq(vLook)) <= Helper::fEpsilon)
            vLook = Project_DirectionOnPlane(InverseViewMatrix.r[1], vWorldUp);

        if (XMVectorGetX(XMVector3LengthSq(vLook)) <= Helper::fEpsilon)
            vLook = Project_DirectionOnPlane(InverseViewMatrix.r[0], vWorldUp);

        if (XMVectorGetX(XMVector3LengthSq(vLook)) <= Helper::fEpsilon)
            return WorldMatrix;

        vLook = XMVector3Normalize(vLook);
        _vector vRight = XMVector3Normalize(XMVector3Cross(vWorldUp, vLook));
        vLook = XMVector3Normalize(XMVector3Cross(vRight, vWorldUp));

        BillboardMatrix.r[0] = XMVectorSetW(vRight * fScaleX, 0.f);
        BillboardMatrix.r[1] = XMVectorSetW(vWorldUp * fScaleY, 0.f);
        BillboardMatrix.r[2] = XMVectorSetW(vLook * fScaleZ, 0.f);

        _float4x4 Result{};
        XMStoreFloat4x4(&Result, BillboardMatrix);
        return Result;
    }
}

NS_END
