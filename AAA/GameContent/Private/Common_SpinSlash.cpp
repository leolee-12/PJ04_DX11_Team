#include "Common_SpinSlash.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CCommon_SpinSlash::CCommon_SpinSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
{
}

CCommon_SpinSlash::CCommon_SpinSlash(const CCommon_SpinSlash& Prototype)
    : CEffect_Mesh(Prototype)
{
}

HRESULT CCommon_SpinSlash::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CCommon_SpinSlash::Initialize(void* pArg)
{
    COMMON_SPINSLASH_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_Common_SpinSlash");

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnKnownTexture = true;

    tDesc.bUseTextureCom = true;
    tDesc.iTextureLevel = Texture_Common_SpinSlash_2.iLevelID;
    tDesc.wstrTextureTag = Texture_Common_SpinSlash_2.szProtoTag;

    tDesc.bUseMaskCom = false;
    tDesc.iMaskLevel = 0;
    tDesc.wstrMaskTag = L"";

    tDesc.bCustomShader = false;
    tDesc.iShaderLevel = 0;
    tDesc.wstrShaderTag = L"";

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    return S_OK;
}

void CCommon_SpinSlash::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CCommon_SpinSlash::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (m_bFadeOutActive == false || m_bFadeOutFinished == true)
        return;

    m_fAccFadeOutTime += fTimeDelta;

    _float fFadeOutRatio = m_fAccFadeOutTime / m_fFadeOutDuration;
    Helper::FloatClamp(fFadeOutRatio, 0.f, 1.f);

    const _float fFadeOutStep = Helper::FloatSmoothStep(0.f, 1.f, fFadeOutRatio);
    m_fAlpha = m_fFadeOutStartAlpha * (1.f - fFadeOutStep);

    if (fFadeOutRatio >= 1.f)
    {
        m_fAlpha = 0.f;
        m_bFadeOutFinished = true;
    }
}

void CCommon_SpinSlash::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CCommon_SpinSlash::Render()
{
    __super::Render();

    return S_OK;
}

void CCommon_SpinSlash::Effect_Start()
{
    __super::Effect_Start();

    m_bFadeOutActive = false;
    m_bFadeOutFinished = false;
    m_fFadeOutDuration = 0.3f;
    m_fAccFadeOutTime = 0.f;
    m_fFadeOutStartAlpha = 1.f;
}

void CCommon_SpinSlash::Start_FadeOut(_float fFadeOutDuration)
{
    if (m_bFadeOutActive == true)
        return;

    m_bFadeOutActive = true;
    m_bFadeOutFinished = false;
    m_fFadeOutDuration = fFadeOutDuration;
    m_fAccFadeOutTime = 0.f;
    m_fFadeOutStartAlpha = m_fAlpha;

    if (m_fFadeOutDuration <= Helper::fEpsilon)
    {
        m_fAlpha = 0.f;
        m_bFadeOutFinished = true;
    }
}

CCommon_SpinSlash* CCommon_SpinSlash::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCommon_SpinSlash* pInstance = new CCommon_SpinSlash(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CCommon_SpinSlash");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CCommon_SpinSlash::Clone(void* pArg)
{
    CCommon_SpinSlash* pInstance = new CCommon_SpinSlash(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CCommon_SpinSlash");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCommon_SpinSlash::Free()
{
    __super::Free();
}