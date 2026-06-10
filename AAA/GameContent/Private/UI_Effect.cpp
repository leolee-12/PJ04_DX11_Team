#include "UI_Effect.h"
#include "GameInstance.h"
#include "GameContent_const.h"

CUI_Effect::CUI_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIPartObject{ pDevice, pContext }
    , m_vColor{ 1.f, 1.f, 1.f, 1.f }
    , m_fAlpha{ 1.f }
    , m_vEffectTiling{ 1.f, 1.f }
    , m_vEffectOffset{ 0.f, 0.f }
    , m_fMaskPower{ 1.f }
    , m_fEffectIntensity{ 1.f }
    , m_fRevealProgress{ 1.f }
    , m_fRevealSoftness{ 0.05f }
    , m_iMaskChannel{ 3 }
    , m_iInvertMask{ 0 }
    , m_iTextureLevel{ ETOUI(LEVEL::STATIC) }
    , m_iMaskTextureLevel{ ETOUI(LEVEL::STATIC) }
    , m_iMaskTexIndex{ 0 }
    , m_iShaderPass  {ETOI(EFFECT_PASS::MASKED_TEXTURE)}
{
}

CUI_Effect::CUI_Effect(const CUI_Effect& Prototype)
    : CUIPartObject( Prototype )
    , m_vColor{ Prototype.m_vColor }
    , m_fAlpha{ Prototype.m_fAlpha }
    , m_vEffectTiling{ Prototype.m_vEffectTiling }
    , m_vEffectOffset{ Prototype.m_vEffectOffset }
    , m_fMaskPower{ Prototype.m_fMaskPower }
    , m_fEffectIntensity{ Prototype.m_fEffectIntensity }
    , m_fRevealProgress{ Prototype.m_fRevealProgress }
    , m_fRevealSoftness{ Prototype.m_fRevealSoftness }
    , m_iMaskChannel{ Prototype.m_iMaskChannel }
    , m_iInvertMask{ Prototype.m_iInvertMask }
    , m_iTextureLevel{ Prototype.m_iTextureLevel }
    , m_strTextureProtoTag{ Prototype.m_strTextureProtoTag }
    , m_iMaskTextureLevel{ Prototype.m_iMaskTextureLevel }
    , m_strMaskTextureProtoTag{ Prototype.m_strMaskTextureProtoTag }
    , m_iMaskTexIndex{ Prototype.m_iMaskTexIndex }
    , m_iShaderPass{ Prototype.m_iShaderPass }
{
}

HRESULT CUI_Effect::Initialize_Prototype()
{
    return __super::Initialize_Prototype();
}

HRESULT CUI_Effect::Initialize(void* pArg)
{
    UI_EFFECT_DESC Default{};
    UI_EFFECT_DESC* pDesc = pArg ? static_cast<UI_EFFECT_DESC*>(pArg) : &Default;

    if (pDesc->szTextureProtoTag)
    {
        m_iTextureLevel = static_cast<_int>(pDesc->iTextureLevel);
        m_strTextureProtoTag = pDesc->szTextureProtoTag;
    }

    if (pDesc->szMaskTextureProtoTag)
    {
        m_iMaskTextureLevel = static_cast<_int>(pDesc->iMaskTextureLevel);
        m_strMaskTextureProtoTag = pDesc->szMaskTextureProtoTag;
    }

    m_vColor = pDesc->vColor;
    m_fAlpha = pDesc->fAlpha;
    m_vEffectTiling = pDesc->vEffectTiling;
    m_vEffectOffset = pDesc->vEffectOffset;
    m_fMaskPower = pDesc->fMaskPower;
    m_fEffectIntensity = pDesc->fEffectIntensity;
    m_fRevealProgress = pDesc->fRevealProgress;
    m_fRevealSoftness = pDesc->fRevealSoftness;
    m_iMaskChannel = pDesc->iMaskChannel;
    m_iInvertMask = pDesc->iInvertMask;
    m_iMaskTexIndex = pDesc->iMaskTexIndex;
    m_iShaderPass = pDesc->iShaderPass;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    m_eRenderLayer = static_cast<RENDERUIID>(pDesc->iRenderLayer);

    m_pTransformCom->Set_Scale(pDesc->vSize.x, pDesc->vSize.y, 1.f);
    m_pTransformCom->Set_State(
        STATE::POSITION,
        XMVectorSet(pDesc->vPosition.x, pDesc->vPosition.y, 1.f, 1.f));

    if (FAILED(Ready_Components(pDesc)))
        return E_FAIL;

    return S_OK;
}

void CUI_Effect::Priority_Update(_float fTimeDelta)
{
}

void CUI_Effect::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CUI_Effect::Late_Update(_float fTimeDelta)
{
    m_pGameInstance_Proxy->Add_RenderGroup_UI(m_eRenderLayer, this);
}

HRESULT CUI_Effect::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (m_iShaderPass < 0 ||
        static_cast<_uint>(m_iShaderPass) >= m_pShaderCom->Get_NumPasses())
        return E_FAIL;

    if (FAILED(m_pShaderCom->Begin(static_cast<_uint>(m_iShaderPass))))
        return E_FAIL;

    if (FAILED(m_pVIBufferCom->Bind_Resources()))
        return E_FAIL;

    if (FAILED(m_pVIBufferCom->Render()))
        return E_FAIL;

    return S_OK;
}

void CUI_Effect::Deserialize_Internal(const json& j)
{
    __super::Deserialize_Internal(j);

    if (!m_pEffectTextureCom && !m_strTextureProtoTag.empty())
    {
        m_pEffectTextureCom = Add_Component<CTexture>(
            static_cast<_uint>(m_iTextureLevel),
            m_strTextureProtoTag,
            TEXT("Com_EffectTexture"));
    }

    if (!m_pMaskTextureCom && !m_strMaskTextureProtoTag.empty())
    {
        m_pMaskTextureCom = Add_Component<CTexture>(
            static_cast<_uint>(m_iMaskTextureLevel),
            m_strMaskTextureProtoTag,
            TEXT("Com_MaskTexture"));
    }
}

HRESULT CUI_Effect::Set_Texture(_int iLevel, const _wstring& strProtoTag)
{
    if (strProtoTag.empty())
        return E_FAIL;

    auto iter = m_Components.find(TEXT("Com_EffectTexture"));
    if (iter != m_Components.end())
    {
        Safe_Release(iter->second);
        m_Components.erase(iter);
        m_pEffectTextureCom = nullptr;
    }

    m_iTextureLevel = iLevel;
    m_strTextureProtoTag = strProtoTag;

    m_pEffectTextureCom = Add_Component<CTexture>(
        static_cast<_uint>(m_iTextureLevel),
        m_strTextureProtoTag,
        TEXT("Com_EffectTexture"));

    return m_pEffectTextureCom ? S_OK : E_FAIL;
}

HRESULT CUI_Effect::Set_MaskTexture(_int iLevel, const _wstring& strProtoTag)
{
    if (strProtoTag.empty())
        return E_FAIL;

    auto iter = m_Components.find(TEXT("Com_MaskTexture"));
    if (iter != m_Components.end())
    {
        Safe_Release(iter->second);
        m_Components.erase(iter);
        m_pMaskTextureCom = nullptr;
    }

    m_iMaskTextureLevel = iLevel;
    m_strMaskTextureProtoTag = strProtoTag;

    m_pMaskTextureCom = Add_Component<CTexture>(
        static_cast<_uint>(m_iMaskTextureLevel),
        m_strMaskTextureProtoTag,
        TEXT("Com_MaskTexture"));

    return m_pMaskTextureCom ? S_OK : E_FAIL;
}

HRESULT CUI_Effect::Ready_Components(UI_EFFECT_DESC* pDesc)
{
    m_pShaderCom = Add_Component<CShader>(Shader_UI.iLevelID, Shader_UI.szProtoTag, TEXT("Com_Shader"));
    if (!m_pShaderCom)
        return E_FAIL;

    m_pVIBufferCom = Add_Component<CVIBuffer_Rect>(VI_Rect.iLevelID, VI_Rect.szProtoTag, TEXT("Com_Model"));
    if (!m_pVIBufferCom)
        return E_FAIL;

    if (!m_strTextureProtoTag.empty())
    {
        m_pEffectTextureCom = Add_Component<CTexture>(static_cast<_uint>(m_iTextureLevel), m_strTextureProtoTag, TEXT("Com_EffectTexture"));
        if (!m_pEffectTextureCom)
            return E_FAIL;
    }

    if (!m_strMaskTextureProtoTag.empty())
    {
        m_pMaskTextureCom = Add_Component<CTexture>(static_cast<_uint>(m_iMaskTextureLevel), m_strMaskTextureProtoTag, TEXT("Com_MaskTexture"));
        if (!m_pMaskTextureCom)
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CUI_Effect::Bind_ShaderResources()
{
    if (m_pParentMatrix)
    {
        Compute_CombinedWorldMatrix(
            XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

        if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
            return E_FAIL;
    }
    else if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix",
        m_pTransformCom->Get_WorldMatrixPtr())))
    {
        return E_FAIL;
    }

    if (FAILED(Bind_ShaderResource(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW, PROJ_TYPE::ORTHO)))
        return E_FAIL;

    if (FAILED(Bind_ShaderResource(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ, PROJ_TYPE::ORTHO)))
        return E_FAIL;

    const _bool bNeedEffectTexture =
        m_iShaderPass == ETOI(EFFECT_PASS::MASKED_TEXTURE) ||
        m_iShaderPass == ETOI(EFFECT_PASS::MASKED_ADD) ||
        m_iShaderPass == ETOI(EFFECT_PASS::BRUSH_REVEAL);

    const _bool bNeedMaskTexture =
        m_iShaderPass == ETOI(EFFECT_PASS::MASKED_TEXTURE) ||
        m_iShaderPass == ETOI(EFFECT_PASS::MASKED_COLOR) ||
        m_iShaderPass == ETOI(EFFECT_PASS::MASKED_ADD) ||
        m_iShaderPass == ETOI(EFFECT_PASS::BRUSH_REVEAL);

    if (bNeedEffectTexture)
    {
        if (!m_pEffectTextureCom)
            return E_FAIL;

        if (FAILED(m_pEffectTextureCom->Bind_ShaderResource(m_pShaderCom, "g_EffectTexture", m_iTexIndex)))
            return E_FAIL;
    }

    if (bNeedMaskTexture)
    {
        if (!m_pMaskTextureCom)
            return E_FAIL;

        if (FAILED(m_pMaskTextureCom->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", m_iMaskTexIndex)))
            return E_FAIL;
    }

    if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(m_vColor))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(m_fAlpha))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_vEffectTiling", &m_vEffectTiling,
        sizeof(m_vEffectTiling))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_vEffectOffset", &m_vEffectOffset,
        sizeof(m_vEffectOffset))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fMaskPower", &m_fMaskPower, sizeof(m_fMaskPower))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fEffectIntensity", &m_fEffectIntensity,
        sizeof(m_fEffectIntensity))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fRevealProgress", &m_fRevealProgress,
        sizeof(m_fRevealProgress))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fRevealSoftness", &m_fRevealSoftness,
        sizeof(m_fRevealSoftness))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaskChannel", &m_iMaskChannel,
        sizeof(m_iMaskChannel))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_iInvertMask", &m_iInvertMask, sizeof(m_iInvertMask))))
        return E_FAIL;

    return S_OK;
}

CUI_Effect* CUI_Effect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Effect* pInstance = new CUI_Effect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_Effect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_Effect::Clone(void* pArg)
{
    CUI_Effect* pInstance = new CUI_Effect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_Effect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_Effect::Free()
{
    __super::Free();
}