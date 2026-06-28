#include "Effect_NonParticle.h"

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

HRESULT CEffect_NonParticle::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_NonParticle::Initialize(void* pArg)
{
    EFFECT_NONEPARTICLE_DESC* pDesc = static_cast<EFFECT_NONEPARTICLE_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CEffect_NonParticle::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CEffect_NonParticle::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CEffect_NonParticle::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CEffect_NonParticle::Render()
{
    return S_OK;
}

void CEffect_NonParticle::Effect_Start()
{
    __super::Effect_Start();
}

HRESULT CEffect_NonParticle::Bind_ShaderValue()
{
    if (FAILED(__super::Bind_ShaderValue()))
        return E_FAIL;

    Helper::FloatClamp(m_fAlpha, 0.f, 1.f);
    _float fAlpha = m_fAlpha * Get_FadeOutFactor();
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &fAlpha, sizeof(fAlpha))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(m_vColor))))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_NonParticle::Ready_Components()
{
    return S_OK;
}

void CEffect_NonParticle::Init_PropertyValue()
{
    // Alpha
    m_fAlpha = { 1.0f };

    m_bFadeInOut = { false };

    m_AlphaRatioValue.reserve(4);

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

    m_SizeRatioValue.reserve(4);

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

    m_ColorRatioValue.reserve(4);

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
    m_bRotationChange = { false };
    m_fRotationDegree = { 360.f };
    m_vRotationAxis = { 0.f, 1.f, 0.f };
    m_fRot_Start_Ratio = { 0.f };
    m_fRot_End_Ratio = { 1.f };

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
}

void CEffect_NonParticle::Update_Core(const _float fTimeDelta, const _float fRatio)
{
    __super::Update_Core(fTimeDelta, fRatio);

    // Update
    Update_Alpha(fTimeDelta, fRatio);
    Update_Size(fTimeDelta, fRatio);
    Update_Color(fTimeDelta, fRatio);
    Update_Rot(fTimeDelta, fRatio);

    Update_Move(fTimeDelta, fRatio);       // Move관련 가장 먼저
    Update_MoveSin(fTimeDelta, fRatio);
}

void CEffect_NonParticle::Update_Alpha(const _float fTimeDelta, const _float fRatio)
{
    if (m_bFadeInOut == true)
    {
        const _float fStartRatio = 0.f;
        const _float fEndRatio = 1.f;

        m_AlphaRatioValue.push_back({ fStartRatio, m_fAlphaStartValue });

        if (m_bActive_Alpha_Ratio_0 == true)
            m_AlphaRatioValue.push_back({ m_fAlpha_Ratio_0, m_fAlpha_Value_0 });
        if (m_bActive_Alpha_Ratio_1 == true)
            m_AlphaRatioValue.push_back({ m_fAlpha_Ratio_1, m_fAlpha_Value_1 });

        m_AlphaRatioValue.push_back({ fEndRatio, m_fAlphaEndValue });

        for (_uint i = 0; i < m_AlphaRatioValue.size() - 1; ++i)
        {
            if (fRatio <= m_AlphaRatioValue[i + 1].fRatio)
            {
                _float fSmoothStep = Helper::FloatSmoothStep(m_AlphaRatioValue[i].fRatio, m_AlphaRatioValue[i + 1].fRatio, fRatio);
                m_fAlpha = m_AlphaRatioValue[i].fValue + (m_AlphaRatioValue[i + 1].fValue - m_AlphaRatioValue[i].fValue) * fSmoothStep;

                break;
            }
        }
    }
     /*
    else
    {
        m_fAlpha = 1.f;
    }
    */

    m_AlphaRatioValue.clear();
}

void CEffect_NonParticle::Update_Size(const _float fTimeDelta, const _float fRatio)
{
    if (m_bSizeChange == true)
    {
        const _float fStartRatio = 0.f;
        const _float fEndRatio = 1.f;

        m_SizeRatioValue.push_back({ fStartRatio, m_fSizeStartValue });

        if (m_bActive_Size_Ratio_0 == true)
            m_SizeRatioValue.push_back({ m_fSize_Ratio_0, m_fSize_Value_0 });
        if (m_bActive_Size_Ratio_1 == true)
            m_SizeRatioValue.push_back({ m_fSize_Ratio_1, m_fSize_Value_1 });

        m_SizeRatioValue.push_back({ fEndRatio, m_fSizeEndValue });

        _float fTargetRatio = (fRatio > 1.f) ? 1.f : fRatio;

        for (_uint i = 0; i < m_SizeRatioValue.size() - 1; ++i)
        {
            if (fTargetRatio <= m_SizeRatioValue[i + 1].fRatio)
            {
                _float fSmoothStep = Helper::FloatSmoothStep(m_SizeRatioValue[i].fRatio, m_SizeRatioValue[i + 1].fRatio, fTargetRatio);
                m_fSize = m_SizeRatioValue[i].fValue + (m_SizeRatioValue[i + 1].fValue - m_SizeRatioValue[i].fValue) * fSmoothStep;
                break;
            }
        }

        if (m_fSize < Helper::fEpsilon)
            m_fSize = Helper::fEpsilon;

        m_pTransformCom->Set_Scale(m_fSize, m_fSize, m_fSize);
    }

    m_SizeRatioValue.clear();
}

void CEffect_NonParticle::Update_Color(const _float fTimeDelta, const _float fRatio)
{
    if (m_bColorChange == true)
    {
        const _float fStartRatio = 0.f;
        const _float fEndRatio = 1.f;

        m_ColorRatioValue.push_back({ fStartRatio, m_vColorStartValue });

        if (m_bActive_Color_Ratio_0 == true)
            m_ColorRatioValue.push_back({ m_fColor_Ratio_0, m_vColor_Value_0 });
        if (m_bActive_Color_Ratio_1 == true)
            m_ColorRatioValue.push_back({ m_fColor_Ratio_1, m_vColor_Value_1 });

        m_ColorRatioValue.push_back({ fEndRatio, m_vColorEndValue });

        for (_uint i = 0; i < m_ColorRatioValue.size() - 1; ++i)
        {
            if (fRatio <= m_ColorRatioValue[i + 1].fRatio)
            {
                _float fSmoothStep = Helper::FloatSmoothStep(m_ColorRatioValue[i].fRatio, m_ColorRatioValue[i + 1].fRatio, fRatio);

                m_vColor.x = m_ColorRatioValue[i].vValue.x + (m_ColorRatioValue[i + 1].vValue.x - m_ColorRatioValue[i].vValue.x) * fSmoothStep;
                m_vColor.y = m_ColorRatioValue[i].vValue.y + (m_ColorRatioValue[i + 1].vValue.y - m_ColorRatioValue[i].vValue.y) * fSmoothStep;
                m_vColor.z = m_ColorRatioValue[i].vValue.z + (m_ColorRatioValue[i + 1].vValue.z - m_ColorRatioValue[i].vValue.z) * fSmoothStep;

                break;
            }
        }
    }

    m_ColorRatioValue.clear();
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

    _float3 vScale = m_pTransformCom->Get_Scaled();

    _matrix matBaseRot = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(m_vBaseRotationDegree.x),
        XMConvertToRadians(m_vBaseRotationDegree.y),
        XMConvertToRadians(m_vBaseRotationDegree.z));

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

void CEffect_NonParticle::Free()
{
    __super::Free();
}