#include "Effect_Part.h"

#include "GameInstance.h"

CEffect_Part::CEffect_Part(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject(pDevice, pContext)
{
    Init_PropertyValue();
}

CEffect_Part::CEffect_Part(const CEffect_Part& Prototype)
    : CGameObject(Prototype)
{
    Init_PropertyValue();
}

HRESULT CEffect_Part::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Part::Initialize(void* pArg)
{
    EFFECT_PART_DESC* pDesc = static_cast<EFFECT_PART_DESC*>(pArg);

    // Texture
    m_bUseTextureCom = pDesc->bUseTextureCom;
    m_iTextureLevel = pDesc->iTextureLevel;
    m_wstrTextureTag = pDesc->wstrTextureTag;

    // Mask
    m_bUseMaskCom = pDesc->bUseMaskCom;
    m_iMaskLevel = pDesc->iMaskLevel;
    m_wstrMaskTag = pDesc->wstrMaskTag;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;
   
    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CEffect_Part::MoveUVScroll(const _float fTimeDelta, const _bool bUpdate, _float2& vUV, const _float2 vSpeed)
{
    if (bUpdate == true)
    {
        vUV.x = vUV.x + vSpeed.x * fTimeDelta;
        vUV.y = vUV.y + vSpeed.y * fTimeDelta;

        vUV.x = fmodf(vUV.x, 1.f);
        vUV.y = fmodf(vUV.y, 1.f);
    }
}

void CEffect_Part::Priority_Update(_float fTimeDelta)
{

}

void CEffect_Part::Update(_float fTimeDelta)
{
    Update_Value(fTimeDelta);

    Update_UVScroll(fTimeDelta);
}

void CEffect_Part::Late_Update(_float fTimeDelta)
{
    if (m_bActive == false)
        return;

    // ㅇㄷ dd
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CEffect_Part::Render()
{
    return S_OK;
}

HRESULT CEffect_Part::Bind_ShaderValue()
{
    Helper::FloatClamp(m_fAlpha, 0.f, 1.f);
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(m_fAlpha))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(m_vColor))))
        return E_FAIL;

    // Texture
    if (m_pTextureCom != nullptr && m_bUseTextureCom == true)
    {
        if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseTexture", &m_bUseTextureCom, sizeof(m_bUseTextureCom))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vTextureTiling", &m_vTextureTiling, sizeof(m_vTextureTiling))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vTextureOffset", &m_vTextureOffset, sizeof(m_vTextureOffset))))
            return E_FAIL;
    }
    else
    {
        _bool bFalse = false;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseTexture", &bFalse, sizeof(bFalse))))
            return E_FAIL;
    }

    // Mask
    if (m_pMaskCom != nullptr && m_bUseMaskCom == true)
    {
        if (FAILED(m_pMaskCom->Bind_ShaderResource(m_pShaderCom, "g_Mask", 0)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseMask", &m_bUseMaskCom, sizeof(m_bUseMaskCom))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vMaskTiling", &m_vMaskTiling, sizeof(m_vMaskTiling))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vMaskOffset", &m_vMaskOffset, sizeof(m_vMaskOffset))))
            return E_FAIL;
    }
    else
    {
        _bool bFalse = false;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseMask", &bFalse, sizeof(bFalse))))
            return E_FAIL;
    }

    return S_OK;
}

void CEffect_Part::Update_EffectPart(const _float fTimeDelta, const _float fActiveTime, const _float fRatio)
{

}

void CEffect_Part::Update_UVScroll(const _float fTimeDelta)
{
    MoveUVScroll(fTimeDelta, m_bTextureUVScroll, m_vTextureOffset, m_vTextureUVSpeed);
    MoveUVScroll(fTimeDelta, m_bMaskUVScroll, m_vMaskOffset, m_vMaskUVSpeed);
}

HRESULT CEffect_Part::Ready_Components()
{
    if (m_bUseTextureCom == true)
    {
        m_pTextureCom = Add_Component<CTexture>(m_iTextureLevel, m_wstrTextureTag, TEXT("Com_Texture"));
        if (m_pTextureCom == nullptr)
            return E_FAIL;
    }

    if (m_bUseMaskCom == true)
    {
        m_pMaskCom = Add_Component<CTexture>(m_iMaskLevel, m_wstrMaskTag, TEXT("Com_Mask"));
        if (m_pMaskCom == nullptr)
            return E_FAIL;
    }
    return S_OK;
}

void CEffect_Part::Init_PropertyValue()
{
    m_vLocalPos = { 0.f, 0.f, 0.f };

    m_bIsPlay = { true };

    m_bLoop = { true };

    m_fDuration = { 5.f };
    m_fAccTime = { 0.f };
    m_fDelayTime = { 0.f };

    m_fAlpha = { 1.0f };

    // Alpha
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
    m_fRotSpeed = { 360.f };
    m_vRotationAxis = { 0.f, 1.f, 0.f };
    m_fRot_Start_Ratio = { 0.f };
    m_fRot_End_Ratio = { 1.f };

    //Move
    m_bMoveChange = { false };
    m_fMoveSpeed = 1.f;
    m_vMoveDir = { 1.f, 0.f, 0.f };
    m_fMove_Start_Ratio = { 0.f };
    m_fMove_End_Ratio = { 1.f };

    m_bMoveSin = { false };
    m_fSinCyclePerDuration = 1.f;
    m_fAmplitude = 1.f;

    // Texture
    m_bUseTextureCom = false;
    m_vTextureTiling = { 1.f, 1.f };
    m_vTextureOffset = { 0.f, 0.f };

    m_bTextureUVScroll = false;
    m_vTextureUVSpeed = { 0.f, 0.f };

    // Mask
    m_bUseMaskCom = false;
    m_vMaskTiling = { 1.f, 1.f };
    m_vMaskOffset = { 0.f, 0.f };

    m_bMaskUVScroll = false;
    m_vMaskUVSpeed = { 0.f, 0.f };
}

void CEffect_Part::Update_Value(_float fTimeDelta)
{
    if (m_bIsPlay == false)
        return;

    // Time Update
    m_fAccTime += fTimeDelta;

    if (m_fAccTime < m_fDelayTime)
    {
        m_bActive = false;
        return;
    }

    m_bActive = true;

    // Time
    const _float fActiveTime = m_fAccTime - m_fDelayTime;
    _float fRatio = fActiveTime / m_fDuration;
    Helper::FloatClamp(fRatio, 0.f, 1.f);

    Update_Alpha(fTimeDelta, fActiveTime, fRatio);
    Update_Size(fTimeDelta, fActiveTime, fRatio);
    Update_Color(fTimeDelta, fActiveTime, fRatio);
    Update_Rot(fTimeDelta, fActiveTime, fRatio);
    Update_Move(fTimeDelta, fActiveTime, fRatio);
    Update_MoveSin(fTimeDelta, fActiveTime, fRatio);

    Update_EffectPart(fTimeDelta, fActiveTime, fRatio);

    // Time Update
    if (fActiveTime >= m_fDuration)
    {
        if(m_bMoveChange)
            // 이거 나중에 부모 위치 반영한 로컬로 바꿔줘야 함.
            m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&m_vLocalPos), 1.f));

        if (m_bLoop)
        {
            m_fAccTime = 0.f;
        }
        else
        {
            m_fAccTime = m_fDelayTime + m_fDuration;
            m_bIsPlay = false;
        }
    }
}

void CEffect_Part::Update_Alpha(const _float fTimeDelta, const _float fActiveTime, const _float fRatio)
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
 /*   else
    {
        m_fAlpha = 1.f;
    }*/

    m_AlphaRatioValue.clear();
}

void CEffect_Part::Update_Size(const _float fTimeDelta, const _float fActiveTime, const _float fRatio)
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

void CEffect_Part::Update_Color(const _float fTimeDelta, const _float fActiveTime, const _float fRatio)
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

void CEffect_Part::Update_Rot(const _float fTimeDelta, const _float fActiveTime, const _float fRatio)
{
    if (m_bRotationChange == false)
        return;

    if (fRatio >= m_fRot_Start_Ratio && fRatio <= m_fRot_End_Ratio)
    {
        m_pTransformCom->Rotate(XMQuaternionRotationAxis(XMLoadFloat3(&m_vRotationAxis),
            XMConvertToRadians(m_fRotSpeed) * fTimeDelta));

    }
}

void CEffect_Part::Update_Move(const _float fTimeDelta, const _float fActiveTime, const _float fRatio)
{
    if (m_bMoveChange == false)
        return;

    if (fRatio >= m_fMove_Start_Ratio && fRatio <= m_fMove_End_Ratio)
    {
        _vector vPosition =  m_pTransformCom->Get_State(STATE::POSITION);
        vPosition += XMVectorSetW(XMVector3Normalize(XMLoadFloat3(&m_vMoveDir)), 0.f) * m_fMoveSpeed * fTimeDelta;
        m_pTransformCom->Set_State(STATE::POSITION, vPosition);
    }
}

void CEffect_Part::Update_MoveSin(const _float fTimeDelta, const _float fActiveTime, const _float fRatio)
{
    if (m_bMoveSin == false)
    {
        if (m_fPreOffsetY != 0.f)
        {
            _vector vCurPos = m_pTransformCom->Get_State(STATE::POSITION);
            m_pTransformCom->Set_State(STATE::POSITION, vCurPos - XMVectorSet(0.f, m_fPreOffsetY, 0.f, 0.f));
            m_fPreOffsetY = 0.f;
        }

        return;
    }

    _float fCurOffsetY = sinf(fRatio * XM_2PI * m_fSinCyclePerDuration) * m_fAmplitude;

    _vector vCurPos = m_pTransformCom->Get_State(STATE::POSITION);

    _float fDY = fCurOffsetY - m_fPreOffsetY;
    m_pTransformCom->Set_State(STATE::POSITION, vCurPos + XMVectorSet(0.f, fDY, 0.f, 0.f));
    
    m_fPreOffsetY = fCurOffsetY;
}

void CEffect_Part::Free()
{
    __super::Free();
}