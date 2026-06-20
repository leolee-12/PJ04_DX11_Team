#include "UI_GaugeFill.h"
#include "GameInstance.h"
#include "GameContent_const.h"

namespace
{
    constexpr const _tchar* GAUGE_TEXTURE_COM = TEXT("Com_Texture");
    constexpr const _tchar* GAUGE_TEXTURE_TEMP_COM = TEXT("Com_Texture_Temp");

    _int Normalize_FillDirection(_int iDirection)
    {
        if (iDirection == ETOUI(UI_GAUGE_FILL_DIRECTION::RIGHT_TO_LEFT))
            return iDirection;

        return ETOI(UI_GAUGE_FILL_DIRECTION::LEFT_TO_RIGHT);
    }

    _bool Is_ValidGaugePass(_int iPass)
    {
        return iPass == ETOI(UI_EFFECT_PASS::GAUGE_FILL_COLOR) ||
            iPass == ETOI(UI_EFFECT_PASS::GAUGE_FILL_TEXTURE);
    }

    _int Normalize_GaugePass(_int iPass)
    {
        return Is_ValidGaugePass(iPass) ?
            iPass :
            ETOI(UI_EFFECT_PASS::GAUGE_FILL_COLOR);
    }

    static _float4 Lerp4(const _float4& a, const _float4& b, _float t)
    {
        return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t };
    }
}

CUI_GaugeFill::CUI_GaugeFill(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIPartObject{ pDevice, pContext }
    , m_vColor{ 1.f, 1.f, 1.f, 1.f }
    , m_fAlpha{ 1.f }
    , m_fFillRatio{ 1.f }
    , m_fFillSoftness{ 0.f }
    , m_iFillDirection{ ETOI(FILL_DIRECTION::LEFT_TO_RIGHT) }
    , m_iShaderPass{ ETOI(UI_EFFECT_PASS::GAUGE_FILL_COLOR) }
    , m_fMaskPower{ 1.f }
    , m_iMaskChannel{ 3 }
    , m_iInvertMask{ 0 }
    , m_iTextureLevel{ ETOUI(LEVEL::STATIC) }
    , m_fGhostHold{ 1.0f }
    , m_fGhostDrain{ 0.6f }
    , m_fBlinkRate{ 5.0f }
    , m_fFillSpeed{ 2.8f }
    , m_vGhostColorStart{ 1.f, 0.92f, 0.2f, 1.f }
    , m_vGhostColorEnd{ 1.f, 1.f,   1.f,  1.f }
    , m_fHealGray{ 0.4f }
    , m_fHealBright{ 0.6f }
{
}

CUI_GaugeFill::CUI_GaugeFill(const CUI_GaugeFill& Prototype)
    : CUIPartObject( Prototype )
    , m_vColor{ Prototype.m_vColor }
    , m_fAlpha{ Prototype.m_fAlpha }
    , m_fFillRatio{ Prototype.m_fFillRatio }
    , m_fFillSoftness{ Prototype.m_fFillSoftness }
    , m_iFillDirection{ Prototype.m_iFillDirection }
    , m_iShaderPass{ Prototype.m_iShaderPass }
    , m_fMaskPower{ Prototype.m_fMaskPower }
    , m_iMaskChannel{ Prototype.m_iMaskChannel }
    , m_iInvertMask{ Prototype.m_iInvertMask }
    , m_iTextureLevel{ Prototype.m_iTextureLevel }
    , m_strTextureProtoTag{ Prototype.m_strTextureProtoTag }
    , m_fGhostHold{ Prototype.m_fGhostHold }
    , m_fGhostDrain{ Prototype.m_fGhostDrain }
    , m_fBlinkRate{ Prototype.m_fBlinkRate }
    , m_fFillSpeed{ Prototype.m_fFillSpeed }
    , m_vGhostColorStart{ Prototype.m_vGhostColorStart }
    , m_vGhostColorEnd{ Prototype.m_vGhostColorEnd }
    , m_fHealGray{ Prototype.m_fHealGray }
    , m_fHealBright{ Prototype.m_fHealBright }
{
}

HRESULT CUI_GaugeFill::Initialize_Prototype()
{
    return __super::Initialize_Prototype();
}

HRESULT CUI_GaugeFill::Initialize(void* pArg)
{
    UI_GAUGEFILL_DESC Default{};
    UI_GAUGEFILL_DESC* pDesc = pArg ? static_cast<UI_GAUGEFILL_DESC*>(pArg) : &Default;

    if (pDesc->szTextureProtoTag)
    {
        m_iTextureLevel = static_cast<_int>(pDesc->iTextureLevel);
        m_strTextureProtoTag = pDesc->szTextureProtoTag;
    }

    m_vColor = pDesc->vColor;
    m_fAlpha = pDesc->fAlpha;
    m_fFillRatio = clamp(pDesc->fFillRatio, 0.f, 1.f);
    m_fFillSoftness = max(0.f, pDesc->fFillSoftness);
    m_iFillDirection = Normalize_FillDirection(pDesc->iFillDirection);
    m_iShaderPass = Normalize_GaugePass(pDesc->iShaderPass);
    m_fMaskPower = max(0.0001f, pDesc->fMaskPower);
    m_iMaskChannel = pDesc->iMaskChannel;
    m_iInvertMask = pDesc->iInvertMask;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    m_eRenderLayer = static_cast<RENDERUIID>(pDesc->iRenderLayer);

    m_pTransformCom->Set_Scale(pDesc->vSize.x, pDesc->vSize.y, 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(pDesc->vPosition.x, pDesc->vPosition.y, 1.f, 1.f));

    if (FAILED(Ready_Components(pDesc)))
        return E_FAIL;

    return S_OK;
}

void CUI_GaugeFill::Priority_Update(_float fTimeDelta)
{
}

void CUI_GaugeFill::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    m_fGhostAlpha = 1.f;

    if (m_bAppearSweep)
    {
        m_fFillRatio = min(m_fFillRatio + m_fAppearSpeed * fTimeDelta, m_fTargetRatio);
        m_fGhostRatio = m_fFillRatio;
        if (m_fFillRatio >= m_fTargetRatio - 1e-5f)
        {
            m_fFillRatio = m_fGhostRatio = m_fTargetRatio;
            m_bAppearSweep = false;
        }
        return;
    }

    if (m_eGhostMode == GHOST_MODE::DRAIN)
    {
        // ≥Î∂˚°Í»Ú ªˆ ±Ù∫˝
        m_fBlinkPhase += max(0.f, m_fBlinkRate) * fTimeDelta;
        const _float t = 0.5f + 0.5f * sinf(m_fBlinkPhase * 6.2831853f);
        m_vGhostColorCur = Lerp4(m_vGhostColorStart, m_vGhostColorEnd, t);

        m_fGhostHoldAcc += fTimeDelta;
        if (m_fGhostHoldAcc >= max(0.f, m_fGhostHold))
        {
            m_fGhostRatio -= max(0.01f, m_fGhostDrain) * fTimeDelta;
            if (m_fGhostRatio <= m_fFillRatio)
            {
                m_fGhostRatio = m_fFillRatio;
                m_eGhostMode = GHOST_MODE::NONE;
            }
        }
    }
    else if (m_eGhostMode == GHOST_MODE::FILL)
    {
        m_vGhostColorCur = HealPreview(m_vColor);

        m_fGhostHoldAcc += fTimeDelta;
        if (m_fGhostHoldAcc >= max(0.f, m_fGhostHold)) 
        {
            m_fFillRatio = (m_fFillSpeed <= 0.f)
                ? m_fGhostRatio
                : min(m_fFillRatio + m_fFillSpeed * fTimeDelta, m_fGhostRatio);

            if (m_fFillRatio >= m_fGhostRatio - 1e-5f)
            {
                m_fFillRatio = m_fGhostRatio;
                m_eGhostMode = GHOST_MODE::NONE;
            }
        }
    }
    else
        m_fGhostRatio = m_fFillRatio;
}

void CUI_GaugeFill::Late_Update(_float fTimeDelta)
{
    m_pGameInstance_Proxy->Add_RenderGroup_UI(m_eRenderLayer, this);
}

HRESULT CUI_GaugeFill::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (m_iShaderPass < 0 ||
        static_cast<_uint>(m_iShaderPass) >= m_pShaderCom->Get_NumPasses())
    {
        m_iShaderPass = ETOI(UI_EFFECT_PASS::GAUGE_FILL_COLOR);
    }

    if (FAILED(m_pShaderCom->Begin(static_cast<_uint>(m_iShaderPass))))
        return E_FAIL;

    if (FAILED(m_pVIBufferCom->Bind_Resources()))
        return E_FAIL;

    if (FAILED(m_pVIBufferCom->Render()))
        return E_FAIL;

    return S_OK;
}

void CUI_GaugeFill::Deserialize_Internal(const json& j)
{
    __super::Deserialize_Internal(j);

    m_fFillRatio = clamp(m_fFillRatio, 0.f, 1.f);
    m_fFillSoftness = max(0.f, m_fFillSoftness);
    m_iFillDirection = Normalize_FillDirection(m_iFillDirection);
    m_iShaderPass = Normalize_GaugePass(m_iShaderPass);
    m_fMaskPower = max(0.0001f, m_fMaskPower);
    
    if (m_strTextureProtoTag.empty())
        return;

    if (m_pTextureCom)
        return;

    m_pTextureCom = Add_Component<CTexture>(static_cast<_uint>(m_iTextureLevel), m_strTextureProtoTag, GAUGE_TEXTURE_COM);
}

_float4 CUI_GaugeFill::HealPreview(const _float4& c) const
{
    const _float lum = 0.299f * c.x + 0.587f * c.y + 0.114f * c.z;   // »÷µµ
    const _float g = clamp(m_fHealGray, 0.f, 1.f);
    const _float b = m_fHealBright;
    return {
        (c.x * (1.f - g) + lum * g) * b,
        (c.y * (1.f - g) + lum * g) * b,
        (c.z * (1.f - g) + lum * g) * b,
        c.w
    };
}

void CUI_GaugeFill::Set_TargetRatio(_float fRatio)
{
    fRatio = clamp(fRatio, 0.f, 1.f);

    if (!m_bGaugeInit)
    {
        m_fFillRatio = m_fGhostRatio = m_fTargetRatio = fRatio;
        m_bGaugeInit = true;
        return;
    }

    const _float fPrev = m_fFillRatio;
    m_fTargetRatio = fRatio;

    if (fRatio < fPrev)            // ««∞› °Ê DRAIN
    {
        m_fFillRatio = fRatio;                 // «Œ≈© ¡ÔΩ√ °È
        if (m_eGhostMode != GHOST_MODE::DRAIN || fPrev > m_fGhostRatio)
            m_fGhostRatio = fPrev;             // ¡˜¿¸ = ¿‹ªÛ ªÛ«—
        m_eGhostMode = GHOST_MODE::DRAIN;
        m_fGhostHoldAcc = 0.f;
    }
    else if (fRatio > fPrev)       // »˙ °Ê FILL
    {
        m_fGhostRatio = fRatio;
        m_eGhostMode = GHOST_MODE::FILL;
        m_fGhostHoldAcc = 0.f;
    }
}

void CUI_GaugeFill::Set_FillRatio(_float fRatio)
{
    m_fFillRatio = clamp(fRatio, 0.f, 1.f);
}

HRESULT CUI_GaugeFill::Set_Texture(_int iLevel, const _wstring& strProtoTag)
{
    if (strProtoTag.empty())
        return E_FAIL;

    auto iterTemp = m_Components.find(GAUGE_TEXTURE_TEMP_COM);
    if (iterTemp != m_Components.end())
    {
        Safe_Release(iterTemp->second);
        m_Components.erase(iterTemp);
    }

    CTexture* pNewTexture = Add_Component<CTexture>(
        static_cast<_uint>(iLevel),
        strProtoTag,
        GAUGE_TEXTURE_TEMP_COM);

    if (!pNewTexture)
        return E_FAIL;

    auto iter = m_Components.find(GAUGE_TEXTURE_COM);
    if (iter != m_Components.end())
    {
        Safe_Release(iter->second);
        m_Components.erase(iter);
    }

    m_Components.erase(GAUGE_TEXTURE_TEMP_COM);
    m_Components.emplace(GAUGE_TEXTURE_COM, pNewTexture);

    m_iTextureLevel = iLevel;
    m_strTextureProtoTag = strProtoTag;
    m_pTextureCom = pNewTexture;

    return S_OK;

}

HRESULT CUI_GaugeFill::Ready_Components(UI_GAUGEFILL_DESC* pDesc)
{
    m_pShaderCom = Add_Component<CShader>(Shader_UI.iLevelID, Shader_UI.szProtoTag, TEXT("Com_Shader"));

    if (!m_pShaderCom)
        return E_FAIL;

    m_pVIBufferCom = Add_Component<CVIBuffer_Rect>(VI_Rect.iLevelID, VI_Rect.szProtoTag, TEXT("Com_Model"));

    if (!m_pVIBufferCom)
        return E_FAIL;

    if (!m_strTextureProtoTag.empty())
    {
        m_pTextureCom = Add_Component<CTexture>(static_cast<_uint>(m_iTextureLevel), m_strTextureProtoTag, GAUGE_TEXTURE_COM);

        if (!m_pTextureCom)
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CUI_GaugeFill::Bind_ShaderResources()
{
    if (m_pParentMatrix)
    {
        Compute_CombinedWorldMatrix(
            XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

        if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
            return E_FAIL;
    }
    else if (FAILED(m_pShaderCom->Bind_Matrix(
        "g_WorldMatrix",
        m_pTransformCom->Get_WorldMatrixPtr())))
    {
        return E_FAIL;
    }

    if (FAILED(Bind_ShaderResource(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW, PROJ_TYPE::ORTHO)))
        return E_FAIL;

    if (FAILED(Bind_ShaderResource(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ, PROJ_TYPE::ORTHO)))
        return E_FAIL;

    if (!m_pTextureCom)
        return E_FAIL;

    if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTexIndex)))
        return E_FAIL;

    m_fFillRatio = clamp(m_fFillRatio, 0.f, 1.f);
    m_fFillSoftness = max(0.f, m_fFillSoftness);
    m_iFillDirection = Normalize_FillDirection(m_iFillDirection);
    m_iShaderPass = Normalize_GaugePass(m_iShaderPass);
    m_fMaskPower = max(0.0001f, m_fMaskPower);

    if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(m_vColor)))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(m_fAlpha)))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fFillRatio", &m_fFillRatio, sizeof(m_fFillRatio)))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fFillSoftness", &m_fFillSoftness, sizeof(m_fFillSoftness)))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_iFillDirection", &m_iFillDirection, sizeof(m_iFillDirection)))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fMaskPower", &m_fMaskPower, sizeof(m_fMaskPower)))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaskChannel", &m_iMaskChannel, sizeof(m_iMaskChannel)))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_iInvertMask", &m_iInvertMask, sizeof(m_iInvertMask)))) return E_FAIL;

    _float3 vScale = m_pTransformCom->Get_Scaled();      
    const _float fTexAspect = 1.f;                       
    _float fCapFracU = (vScale.x > 0.0001f)
        ? (vScale.y / vScale.x) * fTexAspect
        : 0.2f;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fCapFracU", &fCapFracU, sizeof(_float)))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fGhostRatio", &m_fGhostRatio, sizeof(_float))))  return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fGhostAlpha", &m_fGhostAlpha, sizeof(_float))))  return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vGhostColor", &m_vGhostColorCur, sizeof(_float4)))) return E_FAIL;

    return S_OK;
}

void CUI_GaugeFill::Play_AppearSweep(_float fTargetRatio, _float fSpeed)
{
    m_bGaugeInit = true;                       // ¿Ã»ƒ Set_TargetRatio(««∞›) ¡§ªÛ µø¿€
    m_eGhostMode = GHOST_MODE::NONE;           // ∞ÌΩ∫∆Æ πÍµÂ æ¯¿Ω
    m_fFillRatio = 0.f;                        // ∫Û ªÛ≈¬ø°º≠ Ω√¿€
    m_fGhostRatio = 0.f;
    m_fTargetRatio = clamp(fTargetRatio, 0.f, 1.f);
    m_fAppearSpeed = (fSpeed > 0.f) ? fSpeed : m_fFillSpeed;
    m_bAppearSweep = true;
}

CUI_GaugeFill* CUI_GaugeFill::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_GaugeFill* pInstance = new CUI_GaugeFill(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_GaugeFill");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_GaugeFill::Clone(void* pArg)
{
    CUI_GaugeFill* pInstance = new CUI_GaugeFill(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_GaugeFill");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_GaugeFill::Free()
{
    __super::Free();
}
