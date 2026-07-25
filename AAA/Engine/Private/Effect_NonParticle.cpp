#include "Effect_NonParticle.h"

#include "Effect_OrientationUtils.h"
#include "GameInstance.h"

CEffect_NonParticle::CEffect_NonParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Part(pDevice, pContext)
{
    Init_PropertyValue();
}

CEffect_NonParticle::CEffect_NonParticle(const CEffect_NonParticle& Prototype)
    : CEffect_Part(Prototype)
{
    Init_PropertyValue();
}

HRESULT CEffect_NonParticle::Bind_ShaderValue()
{
    if (FAILED(__super::Bind_ShaderValue()))
        return E_FAIL;

    Helper::FloatClamp(m_fAlpha, 0.f, 1.f);
    _float fAlpha = m_fAlpha * Get_FadeOutAlpha();
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &fAlpha, sizeof(fAlpha))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(m_vColor))))
        return E_FAIL;

    return S_OK;
}

void CEffect_NonParticle::Effect_Start()
{
    __super::Effect_Start();
    Resolve_BaseRotation();
    Reset_OrientationTracking();
    Update_Size(0.f, 0.f);
}

void CEffect_NonParticle::On_EffectLoop()
{
    Reset_OrientationTracking();
}

void CEffect_NonParticle::On_Deserialized()
{
    __super::On_Deserialized();
    Resolve_BaseRotation();
    Reset_OrientationTracking();
    Update_Size(0.f, 0.f);
}

void CEffect_NonParticle::Init_PropertyValue()
{
    // Alpha
    m_fAlpha = { 1.0f };

    m_bFadeInOut = { false };

    m_bActive_Alpha_Ratio_0 = true;
    m_fAlpha_Ratio_0 = { 0.5f };

    m_bActive_Alpha_Ratio_1 = false;
    m_fAlpha_Ratio_1 = { 0.75f };

    m_fAlphaStartValue = { 0.f };
    m_fAlpha_Value_0 = { 1.0f };
    m_fAlpha_Value_1 = { 1.0f };
    m_fAlphaEndValue = { 0.f };


    // Size
    m_fSize = { 1.f };

    m_bSizeChange = { false };

    m_bActive_Size_Ratio_0 = false;
    m_fSize_Ratio_0 = { 0.5f };

    m_bActive_Size_Ratio_1 = false;
    m_fSize_Ratio_1 = { 0.75f };

    m_fSizeStartValue = { 1.f };
    m_fSize_Value_0 = { 1.0f };
    m_fSize_Value_1 = { 1.0f };
    m_fSizeEndValue = { 1.f };

    // Color
    m_vColor = { 1.f, 1.f, 1.f };

    m_bColorChange = { false };

    m_bActive_Color_Ratio_0 = false;
    m_fColor_Ratio_0 = { 0.5f };

    m_bActive_Color_Ratio_1 = false;
    m_fColor_Ratio_1 = { 0.75f };

    m_vColorStartValue = { 1.f, 1.f, 1.f };
    m_vColor_Value_0 = { 1.f, 1.f, 1.f };
    m_vColor_Value_1 = { 1.f, 1.f, 1.f };
    m_vColorEndValue = { 1.f, 1.f, 1.f };

    // Rot
    m_vBaseRotationDegree = { 0.f, 0.f, 0.f };
    m_bRandomBaseRotation = false;
    m_vRandomBaseRotationMin = { 0.f, 0.f, 0.f };
    m_vRandomBaseRotationMax = { 360.f, 360.f, 360.f };
    m_vResolvedBaseRotationDegree = m_vBaseRotationDegree;
    m_bRotationChange = { false };
    m_fRotationDegree = { 360.f };
    m_vRotationAxis = { 0.f, 1.f, 0.f };
    m_fRot_Start_Ratio = { 0.f };
    m_fRot_End_Ratio = { 1.f };

    // Orientation
    m_iOrientationMode = ORIENTATION_NONE;
    m_vOrientationDirection = { 0.f, 1.f, 0.f };

    //Move
    m_bMoveChange = { false };

    m_vMoveDir = { 1.f, 0.f, 0.f };
    m_fMoveDistance = 1.f;

    m_fMove_Start_Ratio = { 0.f };
    m_fMove_End_Ratio = { 1.f };

    // MoveSin
    m_bMoveSin = { false };
    m_fSinCyclePerDuration = 1.f;
    m_fAmplitude = 1.f;

    Reset_OrientationTracking();
}

void CEffect_NonParticle::Update_Core(const _float fTimeDelta, const _float fRatio)
{
    __super::Update_Core(fTimeDelta, fRatio);

    // Update
    Update_Alpha(fTimeDelta, fRatio);
    Update_Size(fTimeDelta, fRatio);
    Update_Color(fTimeDelta, fRatio);
    Update_Rot(fTimeDelta, fRatio);

    Update_Move(fTimeDelta, fRatio); // Apply base movement first.
    Update_MoveSin(fTimeDelta, fRatio);
    Update_Orbit(fRatio);
    Update_Orientation();
}

void CEffect_NonParticle::Update_Alpha(const _float fTimeDelta, const _float fRatio)
{
    if (m_bFadeInOut == true)
        m_fAlpha = Evaluate_FloatCurve(
            fRatio, m_fAlpha, true,
            m_fAlphaStartValue, m_fAlphaEndValue,
            m_bActive_Alpha_Ratio_0, m_fAlpha_Ratio_0, m_fAlpha_Value_0,
            m_bActive_Alpha_Ratio_1, m_fAlpha_Ratio_1, m_fAlpha_Value_1,
            false);
}

void CEffect_NonParticle::Update_Size(const _float fTimeDelta, const _float fRatio)
{
    _float fSize = Evaluate_FloatCurve(
        fRatio, m_fSize, m_bSizeChange,
        m_fSizeStartValue, m_fSizeEndValue,
        m_bActive_Size_Ratio_0, m_fSize_Ratio_0, m_fSize_Value_0,
        m_bActive_Size_Ratio_1, m_fSize_Ratio_1, m_fSize_Value_1,
        false);

    if (fSize < Helper::fEpsilon)
        fSize = Helper::fEpsilon;

    if (m_bSizeChange == false)
        m_fSize = fSize;

    Apply_PropertyScale(fSize, fRatio);
}

void CEffect_NonParticle::Update_Color(const _float fTimeDelta, const _float fRatio)
{
    if (m_bColorChange == true)
        m_vColor = Evaluate_Float3Curve(
            fRatio, m_vColor, true,
            m_vColorStartValue, m_vColorEndValue,
            m_bActive_Color_Ratio_0, m_fColor_Ratio_0, m_vColor_Value_0,
            m_bActive_Color_Ratio_1, m_fColor_Ratio_1, m_vColor_Value_1,
            false);
}

void CEffect_NonParticle::Update_Rot(const _float fTimeDelta, const _float fRatio)
{
    _float fAnimDegree = 0.f;

    if (m_bRotationChange == true)
    {
        const _float fRotRange = m_fRot_End_Ratio - m_fRot_Start_Ratio;

        if (fabsf(fRotRange) > Helper::fEpsilon)
        {
            _float fSubRatio = (fRatio - m_fRot_Start_Ratio) / fRotRange;
            Helper::FloatClamp(fSubRatio, 0.f, 1.f);
            fAnimDegree = m_fRotationDegree * fSubRatio;
        }
    }

    _vector vAxis = XMLoadFloat3(&m_vRotationAxis);
    if (XMVectorGetX(XMVector3LengthSq(vAxis)) <= Helper::fEpsilon)
        vAxis = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    else
        vAxis = XMVector3Normalize(vAxis);

    const _float3& vBaseRotationDegree = m_bRandomBaseRotation == true
        ? m_vResolvedBaseRotationDegree
        : m_vBaseRotationDegree;

    m_fRoll = XMConvertToRadians(vBaseRotationDegree.z + fAnimDegree);
    _float3 vScale = m_pTransformCom->Get_Scaled();

    _matrix matBaseRot = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(vBaseRotationDegree.x),
        XMConvertToRadians(vBaseRotationDegree.y),
        XMConvertToRadians(vBaseRotationDegree.z));

    if (Use_LocalRotationAxis() == true)
        vAxis = XMVector3Normalize(XMVector3TransformNormal(vAxis, matBaseRot));

    _matrix matAnimRot = XMMatrixRotationAxis(
        vAxis,
        XMConvertToRadians(fAnimDegree));

    _matrix matRot = matBaseRot * matAnimRot;

    m_pTransformCom->Set_State(STATE::RIGHT, XMVector3Normalize(matRot.r[0]) * vScale.x);
    m_pTransformCom->Set_State(STATE::UP, XMVector3Normalize(matRot.r[1]) * vScale.y);
    m_pTransformCom->Set_State(STATE::LOOK, XMVector3Normalize(matRot.r[2]) * vScale.z);
}

void CEffect_NonParticle::Update_Move(const _float fTimeDelta, const _float fRatio)
{
    _vector vBasePos = XMLoadFloat3(&m_vLocalPos);

    const _float fMoveRange = m_fMove_End_Ratio - m_fMove_Start_Ratio;

    if (m_bMoveChange == true && fMoveRange > Helper::fEpsilon &&
        fRatio >= m_fMove_Start_Ratio && fRatio <= m_fMove_End_Ratio)
    {
        _float fSubRatio = (fRatio - m_fMove_Start_Ratio) / fMoveRange;

        _float fCurDistance = m_fMoveDistance * fSubRatio;

        vBasePos += XMVector3Normalize(XMLoadFloat3(&m_vMoveDir)) * fCurDistance;
    }

    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(vBasePos, 1.f));
}

void CEffect_NonParticle::Update_MoveSin(const _float fTimeDelta, const _float fRatio)
{
    if (m_bMoveSin == false)
        return;

    _float fCurOffsetY = sinf(fRatio * XM_2PI * m_fSinCyclePerDuration) * m_fAmplitude;

    _vector vCurPos = m_pTransformCom->Get_State(STATE::POSITION);

    m_pTransformCom->Set_State(STATE::POSITION, vCurPos + XMVectorSet(0.f, fCurOffsetY, 0.f, 0.f));
}

void CEffect_NonParticle::Update_Orientation()
{
    const _vector vCurrentPosition =
        XMVectorSetW(m_pTransformCom->Get_State(STATE::POSITION), 0.f);

    _float3 vCurrentPositionFloat3{};
    XMStoreFloat3(&vCurrentPositionFloat3, vCurrentPosition);

    if (m_bHasPreviousOrientationPosition == true)
    {
        const _vector vPreviousPosition =
            XMLoadFloat3(&m_vPreviousOrientationPosition);
        const _vector vVelocity = vCurrentPosition - vPreviousPosition;

        if (XMVectorGetX(XMVector3LengthSq(vVelocity)) > Helper::fEpsilon)
            XMStoreFloat3(&m_vOrientationVelocity, vVelocity);
    }

    m_vPreviousOrientationPosition = vCurrentPositionFloat3;
    m_bHasPreviousOrientationPosition = true;

    if (Is_NonParticleOrientationEnabled() == false)
        return;

    _matrix BaseRotation = XMMatrixIdentity();
    BaseRotation.r[0] = XMVectorSetW(
        XMVector3Normalize(m_pTransformCom->Get_State(STATE::RIGHT)),
        0.f);
    BaseRotation.r[1] = XMVectorSetW(
        XMVector3Normalize(m_pTransformCom->Get_State(STATE::UP)),
        0.f);
    BaseRotation.r[2] = XMVectorSetW(
        XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)),
        0.f);

    const _matrix OrientationRotation =
        EffectOrientation::Make_UpAlignedRotation(
            Make_NonParticleOrientationUp(),
            BaseRotation);
    const _float3 vScale = m_pTransformCom->Get_Scaled();

    m_pTransformCom->Set_State(
        STATE::RIGHT,
        XMVector3Normalize(OrientationRotation.r[0]) * vScale.x);
    m_pTransformCom->Set_State(
        STATE::UP,
        XMVector3Normalize(OrientationRotation.r[1]) * vScale.y);
    m_pTransformCom->Set_State(
        STATE::LOOK,
        XMVector3Normalize(OrientationRotation.r[2]) * vScale.z);
}

_bool CEffect_NonParticle::Is_NonParticleOrientationEnabled() const
{
    return
        m_iOrientationMode > ORIENTATION_NONE &&
        m_iOrientationMode < ORIENTATION_END;
}

_vector CEffect_NonParticle::Make_NonParticleOrientationUp() const
{
    _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

    switch (m_iOrientationMode)
    {
    case ORIENTATION_VELOCITY:
        vUp = XMLoadFloat3(&m_vOrientationVelocity);

        if (XMVectorGetX(XMVector3LengthSq(vUp)) <= Helper::fEpsilon)
            vUp = XMLoadFloat3(&m_vMoveDir);
        break;

    case ORIENTATION_RADIAL_OUTWARD:
    case ORIENTATION_RADIAL_INWARD:
    {
        const _vector vPosition =
            XMVectorSetW(m_pTransformCom->Get_State(STATE::POSITION), 0.f);
        const _vector vPivot = m_bOrbitChange == true
            ? XMLoadFloat3(&m_vOrbitPivot)
            : XMVectorZero();
        vUp = vPosition - vPivot;

        if (XMVectorGetX(XMVector3LengthSq(vUp)) <= Helper::fEpsilon)
            vUp = XMLoadFloat3(&m_vOrientationVelocity);

        if (m_iOrientationMode == ORIENTATION_RADIAL_INWARD)
            vUp = XMVectorNegate(vUp);
        break;
    }

    case ORIENTATION_DIRECTION:
        vUp = XMLoadFloat3(&m_vOrientationDirection);
        break;

    case ORIENTATION_NONE:
    default:
        break;
    }

    if (XMVectorGetX(XMVector3LengthSq(vUp)) <= Helper::fEpsilon)
        return XMVectorSet(0.f, 1.f, 0.f, 0.f);

    return XMVector3Normalize(vUp);
}

_float4x4 CEffect_NonParticle::Make_NonParticleConstrainedBillboardWorldMatrix(
    const _float4x4& WorldMatrix) const
{
    if (Is_NonParticleOrientationEnabled() == false)
        return WorldMatrix;

    const _matrix ParentWorldMatrix = m_pParentMatrix != nullptr
        ? XMLoadFloat4x4(m_pParentMatrix)
        : XMMatrixIdentity();
    const _vector vWorldUp = XMVector3TransformNormal(
        Make_NonParticleOrientationUp(),
        ParentWorldMatrix);

    return EffectOrientation::Make_ConstrainedBillboardWorldMatrix(
        WorldMatrix,
        vWorldUp,
        *m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType));
}

void CEffect_NonParticle::Resolve_BaseRotation()
{
    m_vResolvedBaseRotationDegree = m_vBaseRotationDegree;

    if (m_bRandomBaseRotation == false)
        return;

    _float3 vMin = m_vRandomBaseRotationMin;
    _float3 vMax = m_vRandomBaseRotationMax;

    if (vMax.x < vMin.x)
        std::swap(vMin.x, vMax.x);
    if (vMax.y < vMin.y)
        std::swap(vMin.y, vMax.y);
    if (vMax.z < vMin.z)
        std::swap(vMin.z, vMax.z);

    m_vResolvedBaseRotationDegree.x = m_pGameInstance_Proxy->RandomFloat(vMin.x, vMax.x);
    m_vResolvedBaseRotationDegree.y = m_pGameInstance_Proxy->RandomFloat(vMin.y, vMax.y);
    m_vResolvedBaseRotationDegree.z = m_pGameInstance_Proxy->RandomFloat(vMin.z, vMax.z);
}

void CEffect_NonParticle::Reset_OrientationTracking()
{
    m_vOrientationVelocity = m_vMoveDir;
    m_vPreviousOrientationPosition = {};
    m_bHasPreviousOrientationPosition = false;
}
