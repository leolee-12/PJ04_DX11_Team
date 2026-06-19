#include "Effect_NoneParticle.h"

#include "GameInstance.h"

CEffect_NoneParticle::CEffect_NoneParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Part(pDevice, pContext)
{
    Init_PropertyValue();
}

CEffect_NoneParticle::CEffect_NoneParticle(const CEffect_NoneParticle& Prototype)
    : CEffect_Part(Prototype)
{
    Init_PropertyValue();
}

HRESULT CEffect_NoneParticle::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_NoneParticle::Initialize(void* pArg)
{
    EFFECT_NONEPARTICLE_DESC* pDesc = static_cast<EFFECT_NONEPARTICLE_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CEffect_NoneParticle::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CEffect_NoneParticle::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CEffect_NoneParticle::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CEffect_NoneParticle::Render()
{
    return S_OK;
}

void CEffect_NoneParticle::Effect_Start()
{
    __super::Effect_Start();
}

HRESULT CEffect_NoneParticle::Bind_ShaderValue()
{
    if (FAILED(__super::Bind_ShaderValue()))
        return E_FAIL;

    Helper::FloatClamp(m_fAlpha, 0.f, 1.f);
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(m_fAlpha))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(m_vColor))))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_NoneParticle::Ready_Components()
{
    return S_OK;
}

void CEffect_NoneParticle::Init_PropertyValue()
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

void CEffect_NoneParticle::Update_Core(const _float fTimeDelta, const _float fRatio)
{
    __super::Update_Core(fTimeDelta, fRatio);

    // Update
    Update_Alpha(fTimeDelta, fRatio);
    Update_Size(fTimeDelta, fRatio);
    Update_Color(fTimeDelta, fRatio);
    Update_Rot(fTimeDelta, fRatio);

    Update_Move(fTimeDelta, fRatio);       // Move관련 가장 먼저
    Update_MoveSin(fTimeDelta, fRatio);

    Update_UVScroll(fTimeDelta, fRatio);
}

void CEffect_NoneParticle::Update_Alpha(const _float fTimeDelta, const _float fRatio)
{
    if (m_bFadeInOut == true)
    {
        const _float fStartRatio = m_fStartRatio;
        const _float fEndRatio = m_fEndRatio;

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

void CEffect_NoneParticle::Update_Size(const _float fTimeDelta, const _float fRatio)
{
    if (m_bSizeChange == true)
    {
        const _float fStartRatio = m_fStartRatio;
        const _float fEndRatio = m_fEndRatio;

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

void CEffect_NoneParticle::Update_Color(const _float fTimeDelta, const _float fRatio)
{
    if (m_bColorChange == true)
    {
        const _float fStartRatio = m_fStartRatio;
        const _float fEndRatio = m_fEndRatio;

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

void CEffect_NoneParticle::Update_Rot(const _float fTimeDelta, const _float fRatio)
{
    if (m_bRotationChange == false)
        return;

    if (fRatio >= m_fRot_Start_Ratio && fRatio <= m_fRot_End_Ratio)
    {
        _float fSubRatio = (fRatio - m_fRot_Start_Ratio) / (m_fRot_End_Ratio - m_fRot_Start_Ratio);

        _float fCurDegree = m_fRotationDegree * fSubRatio;

        m_pTransformCom->Rotation(XMLoadFloat3(&m_vRotationAxis), XMConvertToRadians(fCurDegree));
    }
}

void CEffect_NoneParticle::Update_Move(const _float fTimeDelta, const _float fRatio)
{
    _vector vBasePos = XMLoadFloat3(&m_vLocalPos);

    if (m_bMoveChange == true &&
        fRatio >= m_fMove_Start_Ratio && fRatio <= m_fMove_End_Ratio)
    {
        _float fSubRatio = (fRatio - m_fMove_Start_Ratio) / (m_fMove_End_Ratio - m_fMove_Start_Ratio);

        _float fCurDistance = m_fMoveDistance * fSubRatio;

        vBasePos += XMVector3Normalize(XMLoadFloat3(&m_vMoveDir)) * fCurDistance;
    }

    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(vBasePos, 1.f));
}

void CEffect_NoneParticle::Update_MoveSin(const _float fTimeDelta, const _float fRatio)
{
    if (m_bMoveSin == false)
        return;

    _float fCurOffsetY = sinf(fRatio * XM_2PI * m_fSinCyclePerDuration) * m_fAmplitude;

    _vector vCurPos = m_pTransformCom->Get_State(STATE::POSITION);

    m_pTransformCom->Set_State(STATE::POSITION, vCurPos + XMVectorSet(0.f, fCurOffsetY, 0.f, 0.f));
}

void CEffect_NoneParticle::Update_UVScroll(const _float fTimeDelta, const _float fRatio)
{
    MoveUVScroll(fRatio, m_bTextureUVScroll, m_vTextureUVScrollCount, m_vTextureOffset, m_vCurTextureUVOffset);
    MoveUVScroll(fRatio, m_bMaskUVScroll, m_vMaskUVScrollCount, m_vMaskOffset, m_vCurMaskUVOffset);
}

void CEffect_NoneParticle::MoveUVScroll(const _float fRatio, const _bool bUpdate, const _float2 vScrollCount, const _float2 vBaseUV, _float2& vOutUV)
{
    if (bUpdate == false)
    {
        vOutUV = vBaseUV;
        return;
    }

    vOutUV.x = vBaseUV.x + vScrollCount.x * fRatio;
    vOutUV.y = vBaseUV.y + vScrollCount.y * fRatio;

    vOutUV.x = fmodf(vOutUV.x, 1.f);
    vOutUV.y = fmodf(vOutUV.y, 1.f);
}

void CEffect_NoneParticle::Free()
{
    __super::Free();
}