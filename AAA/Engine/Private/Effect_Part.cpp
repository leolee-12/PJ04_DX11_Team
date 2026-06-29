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

void CEffect_Part::Priority_Update(_float fTimeDelta)
{

}

void CEffect_Part::Update(_float fTimeDelta)
{
    Update_FadeOut(fTimeDelta);
    Update_Value(fTimeDelta);
}

void CEffect_Part::Late_Update(_float fTimeDelta)
{
}

HRESULT CEffect_Part::Render()
{
    return S_OK;
}

void CEffect_Part::Effect_Start()
{
    m_bIsPlay = true;
    m_bActive = false;
    m_fAccTime = 0.f;

    m_bFadeOutActive = false;
    m_bFadeOutFinished = false;
    m_fFadeOutDuration = 0.3f;
    m_fAccFadeOutTime = 0.f;
    m_fFadeOutAlpha = 1.f;
}

void CEffect_Part::Start_FadeOut(_float fFadeOutDuration)
{
    if (m_bFadeOutActive == true)
        return;

    m_bFadeOutActive = true;
    m_bFadeOutFinished = false;
    m_fFadeOutDuration = fFadeOutDuration;
    m_fAccFadeOutTime = 0.f;
    m_fFadeOutAlpha = 1.f;

    if (m_fFadeOutDuration <= Helper::fEpsilon)
    {
        m_fFadeOutAlpha = 0.f;
        m_bFadeOutFinished = true;
    }
}

void CEffect_Part::Update_PlayValue(_bool bIsPlay, _bool bLoop, _float fDuration, _float fAccTime)
{
    m_bIsPlay = bIsPlay;
    m_bLoop = bLoop;
    m_fDuration = fDuration;
    m_fAccTime = fAccTime;
}

void CEffect_Part::Set_ParentMatrix(const _float4x4* pParentMatrix)
{
    m_pParentMatrix = pParentMatrix;
}

void CEffect_Part::Compute_CombinedWorldMatrix()
{
    if (m_pParentMatrix != nullptr)
    {
        XMStoreFloat4x4(&m_CombinedWorldMatrix,
            XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) * XMLoadFloat4x4(m_pParentMatrix));
    }
    else
    {
        XMStoreFloat4x4(&m_CombinedWorldMatrix, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
    }
}

void CEffect_Part::Update_UVScroll(const _float fTimeDelta, const _float fRatio)
{
    MoveUVScroll(fRatio, m_bTextureUVScroll, m_vTextureUVScrollCount, m_vTextureOffset, m_vCurTextureUVOffset);
    MoveUVScroll(fRatio, m_bMaskUVScroll, m_vMaskUVScrollCount, m_vMaskOffset, m_vCurMaskUVOffset);
}

void CEffect_Part::MoveUVScroll(const _float fRatio, const _bool bUpdate, const _float2 vScrollCount, const _float2 vBaseUV, _float2& vOutUV)
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

HRESULT CEffect_Part::Bind_ShaderValue()
{
    // Texture
    if (m_pTextureCom != nullptr && m_bUseTextureCom == true)
    {
        if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseTexture", &m_bUseTextureCom, sizeof(m_bUseTextureCom))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vTextureTiling", &m_vTextureTiling, sizeof(m_vTextureTiling))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vTextureOffset", &m_vCurTextureUVOffset, sizeof(m_vCurTextureUVOffset))))
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
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vMaskOffset", &m_vCurMaskUVOffset, sizeof(m_vCurMaskUVOffset))))
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
    m_iShaderPass = { 0 };
    m_iMirror = Sampler::DEFAULT;

    m_vLocalPos = { 0.f, 0.f, 0.f };

    m_bIsPlay = { true };

    m_bLoop = { true };

    m_fDuration = { 1.f };
    m_fAccTime = { 0.f };

    m_fStartRatio = { 0.f };
    m_fEndRatio = { 1.f };

    m_bFadeOutActive = false;
    m_bFadeOutFinished = false;
    m_fFadeOutDuration = 0.3f;
    m_fAccFadeOutTime = 0.f;
    m_fFadeOutAlpha = 1.f;

    // Texture
    m_bUseTextureCom = false;
    m_vTextureTiling = { 1.f, 1.f };
    m_vTextureOffset = { 0.f, 0.f };

    m_bTextureUVScroll = false;
    m_vTextureUVScrollCount = { 0.f, 0.f };

    // Mask
    m_bUseMaskCom = false;
    m_vMaskTiling = { 1.f, 1.f };
    m_vMaskOffset = { 0.f, 0.f };

    m_bMaskUVScroll = false;
    m_vMaskUVScrollCount = { 0.f, 0.f };
}

void CEffect_Part::Update_FadeOut(_float fTimeDelta)
{
    if (m_bFadeOutActive == false || m_bFadeOutFinished == true)
        return;

    m_fAccFadeOutTime += fTimeDelta;

    _float fFadeOutRatio = m_fAccFadeOutTime / m_fFadeOutDuration;
    Helper::FloatClamp(fFadeOutRatio, 0.f, 1.f);

    m_fFadeOutAlpha = 1.f - Helper::FloatSmoothStep(0.f, 1.f, fFadeOutRatio);

    if (fFadeOutRatio >= 1.f)
    {
        m_fFadeOutAlpha = 0.f;
        m_bFadeOutFinished = true;
    }
}

void CEffect_Part::Update_Value(const _float fTimeDelta)
{
    if (m_bIsPlay == false)
        return;  

    _float fContainerRatio = 0.f;
    if (m_fDuration > Helper::fEpsilon)
        fContainerRatio = m_fAccTime / m_fDuration;

    Helper::FloatClamp(fContainerRatio, 0.f, 1.f);

    const _float fPartRange = m_fEndRatio - m_fStartRatio;

    if (fPartRange < 0.f || fContainerRatio < m_fStartRatio || fContainerRatio > m_fEndRatio)
        m_bActive = false;
    else
        m_bActive = true;

    if (m_bActive == true)
    {
        _float fPartRatio = 0.f;
        if (fPartRange > Helper::fEpsilon)
            fPartRatio = (fContainerRatio - m_fStartRatio) / fPartRange;

        Helper::FloatClamp(fPartRatio, 0.f, 1.f);
        Update_Core(fTimeDelta, fPartRatio);
    }


    if (m_fAccTime >= m_fDuration)
    {
        if (m_bLoop == true)
        {
            m_fAccTime = 0.f;
        }
        else
        {
            m_bIsPlay = false;
            m_bActive = false;
            m_fAccTime = m_fDuration;
        }
    }
}

void CEffect_Part::Update_Core(const _float fTimeDelta, const _float fRatio)
{
    Update_UVScroll(fTimeDelta, fRatio);

    Update_EffectPart(fTimeDelta, fRatio);
}

void CEffect_Part::Update_EffectPart(const _float fTimeDelta, const _float fRatio)
{

}

_float CEffect_Part::Get_PartDuration() const
{
    _float fPartRange = m_fEndRatio - m_fStartRatio;
    if (fPartRange < 0.f)
        fPartRange = 0.f;

    return m_fDuration * fPartRange;
}

void CEffect_Part::Free()
{
    __super::Free();
}