#include "UI_CurtainAnimBase.h"
#include "GameInstance_Proxy.h"
#include "GameContent_const.h"
#include "Shader.h"
#include "Texture.h"
#include "VIBuffer_Rect.h"

CUI_CurtainAnimBase::CUI_CurtainAnimBase(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIPartObject{ pDevice, pContext }
    , m_fAlpha{ 1.f }, m_iTextureLevel{ ETOUI(LEVEL::STATIC) }
    , m_fStartSize{ 600.f }, m_fEndSize{ 0.f }, m_fShrinkDuration{ 1.2f }, m_fSpinSpeedDeg{ 120.f }
    , m_fStartDelay{ 0.f }, m_bDisableOnFinish{ true }, m_bShowWhileWaiting{ false }
    , m_bLoop{ false }, m_bPlay{ false }
{
}
CUI_CurtainAnimBase::CUI_CurtainAnimBase(const CUI_CurtainAnimBase& Prototype)
    : CUIPartObject(Prototype)
    , m_fAlpha{ Prototype.m_fAlpha }, m_iTextureLevel{ Prototype.m_iTextureLevel }
    , m_strTextureProtoTag{ Prototype.m_strTextureProtoTag }
    , m_bPlay{ Prototype.m_bPlay }, m_bLoop{ Prototype.m_bLoop }
    , m_fStartSize{ Prototype.m_fStartSize }, m_fEndSize{ Prototype.m_fEndSize }
    , m_fShrinkDuration{ Prototype.m_fShrinkDuration }, m_fSpinSpeedDeg{ Prototype.m_fSpinSpeedDeg }
    , m_fStartDelay{ Prototype.m_fStartDelay }, m_bDisableOnFinish{ Prototype.m_bDisableOnFinish }
    , m_bShowWhileWaiting{ Prototype.m_bShowWhileWaiting }
{
}
HRESULT CUI_CurtainAnimBase::Initialize_Prototype() { return __super::Initialize_Prototype(); }

HRESULT CUI_CurtainAnimBase::Initialize(void* pArg)
{
    UI_CURTAINANIM_DESC Default{};
    UI_CURTAINANIM_DESC* pDesc = pArg ? static_cast<UI_CURTAINANIM_DESC*>(pArg) : &Default;

    if (pDesc->szTextureProtoTag)
    {
        m_iTextureLevel = static_cast<_int>(pDesc->iTextureLevel);
        m_strTextureProtoTag = pDesc->szTextureProtoTag;
    }
    m_fAlpha = pDesc->fAlpha;
    m_bPlay = pDesc->bPlay; m_bLoop = pDesc->bLoop;
    m_fStartSize = pDesc->fStartSize; m_fEndSize = pDesc->fEndSize;
    m_fShrinkDuration = pDesc->fShrinkDuration; m_fSpinSpeedDeg = pDesc->fSpinSpeedDeg;
    m_fStartDelay = pDesc->fStartDelay; m_bDisableOnFinish = pDesc->bDisableOnFinish;
    m_bShowWhileWaiting = pDesc->bShowWhileWaiting;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    m_eRenderLayer = static_cast<RENDERUIID>(pDesc->iRenderLayer);
    m_pTransformCom->Set_Scale(m_fStartSize, m_fStartSize, 1.f);
    m_pTransformCom->Set_State(STATE::POSITION,
        XMVectorSet(pDesc->vPosition.x, pDesc->vPosition.y, pDesc->fZOrder, 1.f));

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_fAccTime = 0.f; m_fSpinAngle = 0.f; m_bFinished = false;
    m_bPrevPlay = false; m_bPrevLoop = m_bLoop;
    m_fDelayAcc = 0.f; m_bArmed = false;
    return S_OK;
}

void CUI_CurtainAnimBase::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (m_bArmed)                                                           // 딜레이 대기: Active지만 Late_Update에서 렌더 스킵
    {
        m_fDelayAcc += fTimeDelta;
        if (m_fDelayAcc < m_fStartDelay)
            return;
        m_bArmed = false;
        m_bPlay = true; m_bPrevPlay = false;    // 딜레이 끝 → 재생 시작
    }

    if (m_bPlay && !m_bPrevPlay) { m_fAccTime = 0.f; m_fSpinAngle = 0.f; m_bFinished = false; }
    if (m_bLoop && !m_bPrevLoop && !m_bPlay) { m_fAccTime = 0.f; m_fSpinAngle = 0.f; m_bFinished = false; m_bPlay = true; }
    m_bPrevPlay = m_bPlay; m_bPrevLoop = m_bLoop;

    if (!m_bPlay) return;

    m_fAccTime += fTimeDelta;
    m_fSpinAngle += XMConvertToRadians(m_fSpinSpeedDeg) * fTimeDelta;

    _float t = 1.f;
    if (m_fShrinkDuration > 0.f)
    {
        t = m_fAccTime / m_fShrinkDuration;
        if (t >= 1.f)
        {
            if (m_bLoop) { m_fAccTime = fmodf(m_fAccTime, m_fShrinkDuration); t = m_fAccTime / m_fShrinkDuration; }
            else { t = 1.f; m_bFinished = true; m_bPlay = false; }
        }
    }

    _float s = m_fStartSize + (m_fEndSize - m_fStartSize) * t;
    const _float c = cosf(m_fSpinAngle), sn = sinf(m_fSpinAngle);   // s=0서도 복구되게 basis 직접 구성
    m_pTransformCom->Set_State(STATE::RIGHT, XMVectorSet(c * s, sn * s, 0.f, 0.f));
    m_pTransformCom->Set_State(STATE::UP, XMVectorSet(-sn * s, c * s, 0.f, 0.f));
    m_pTransformCom->Set_State(STATE::LOOK, XMVectorSet(0.f, 0.f, 1.f, 0.f));
}

void CUI_CurtainAnimBase::Late_Update(_float fTimeDelta)
{
    if (m_bArmed && !m_bShowWhileWaiting) return;   // 대기 중 기본 숨김, 플래그면 보이기
    if (m_bFinished && m_bDisableOnFinish) return;  // 끝나면 사라짐
    m_pGameInstance_Proxy->Add_RenderGroup_UI(m_eRenderLayer, this);
}

HRESULT CUI_CurtainAnimBase::Render()
{
    if (FAILED(Bind_CommonMatrices()))       return E_FAIL;
    if (FAILED(Bind_Material(m_pShaderCom))) return E_FAIL; // ▼ 서브클래스
    if (FAILED(m_pShaderCom->Begin(Render_Pass()))) return E_FAIL;  // ▼ 서브클래스
    if (FAILED(m_pVIBufferCom->Bind_Resources())) return E_FAIL;
    if (FAILED(m_pVIBufferCom->Render())) return E_FAIL;
    return S_OK;
}

void CUI_CurtainAnimBase::Deserialize_Internal(const json& j)
{
    __super::Deserialize_Internal(j);
    if (!m_strTextureProtoTag.empty() && !m_pTextureCom)
        m_pTextureCom = Add_Component<CTexture>(static_cast<_uint>(m_iTextureLevel), m_strTextureProtoTag, TEXT("Com_Texture"));
}

HRESULT CUI_CurtainAnimBase::Set_Texture(_int iLevel, const _wstring& strProtoTag)
{
    if (strProtoTag.empty()) return E_FAIL;
    auto iter = m_Components.find(TEXT("Com_Texture"));
    if (iter != m_Components.end()) { Safe_Release(iter->second); m_Components.erase(iter); m_pTextureCom = nullptr; }
    m_iTextureLevel = iLevel; m_strTextureProtoTag = strProtoTag;
    m_pTextureCom = Add_Component<CTexture>(static_cast<_uint>(m_iTextureLevel), m_strTextureProtoTag, TEXT("Com_Texture"));
    return m_pTextureCom ? S_OK : E_FAIL;
}

HRESULT CUI_CurtainAnimBase::Ready_Components()
{
    m_pShaderCom = Add_Component<CShader>(Shader_UI.iLevelID, Shader_UI.szProtoTag, TEXT("Com_Shader"));
    if (!m_pShaderCom) return E_FAIL;
    m_pVIBufferCom = Add_Component<CVIBuffer_Rect>(VI_Rect.iLevelID, VI_Rect.szProtoTag, TEXT("Com_Model"));
    if (!m_pVIBufferCom) return E_FAIL;
    if (!m_strTextureProtoTag.empty())
    {
        m_pTextureCom = Add_Component<CTexture>(static_cast<_uint>(m_iTextureLevel), m_strTextureProtoTag, TEXT("Com_Texture"));
        if (!m_pTextureCom) return E_FAIL;
    }
    return S_OK;
}

HRESULT CUI_CurtainAnimBase::Bind_CommonMatrices()
{
    if (m_pParentMatrix)
    {
        Compute_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
        if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix))) return E_FAIL;
    }
    else if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrixPtr())))
        return E_FAIL;
    if (FAILED(Bind_ShaderResource(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW, PROJ_TYPE::ORTHO))) return E_FAIL;
    if (FAILED(Bind_ShaderResource(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ, PROJ_TYPE::ORTHO))) return E_FAIL;
    return S_OK;
}

void CUI_CurtainAnimBase::Reset_Tranform()
{
    _float s = m_fStartSize;
    const _float c = cosf(m_fSpinAngle), sn = sinf(m_fSpinAngle);   // s=0서도 복구되게 basis 직접 구성
    m_pTransformCom->Set_State(STATE::RIGHT, XMVectorSet(c * s, sn * s, 0.f, 0.f));
    m_pTransformCom->Set_State(STATE::UP, XMVectorSet(-sn * s, c * s, 0.f, 0.f));
    m_pTransformCom->Set_State(STATE::LOOK, XMVectorSet(0.f, 0.f, 1.f, 0.f));
}

void CUI_CurtainAnimBase::Free() { __super::Free(); }