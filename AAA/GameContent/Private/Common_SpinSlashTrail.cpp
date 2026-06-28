#include "Common_SpinSlashTrail.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CCommon_SpinSlashTrail::CCommon_SpinSlashTrail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
{
}

CCommon_SpinSlashTrail::CCommon_SpinSlashTrail(const CCommon_SpinSlashTrail& Prototype)
    : CEffect_Mesh(Prototype)
{
}

HRESULT CCommon_SpinSlashTrail::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CCommon_SpinSlashTrail::Initialize(void* pArg)
{
    COMMON_SPINSSLASHTRAIL_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_Common_SpinSlashTrail");

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnKnownTexture = true;

    tDesc.bUseTextureCom = false;
    tDesc.iTextureLevel = 0;
    tDesc.wstrTextureTag = L"";

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

void CCommon_SpinSlashTrail::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CCommon_SpinSlashTrail::Update(_float fTimeDelta)
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

void CCommon_SpinSlashTrail::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CCommon_SpinSlashTrail::Render()
{
    __super::Render();

    return S_OK;
}

void CCommon_SpinSlashTrail::Effect_Start()
{
	__super::Effect_Start();

	if (m_bInitialAlphaCached == false)
	{
		m_fInitialAlpha = m_fAlpha;
		m_bInitialAlphaCached = true;
	}

	m_fAlpha = m_fInitialAlpha;
	m_bFadeOutActive = false;
	m_bFadeOutFinished = false;
	m_fFadeOutDuration = 0.3f;
	m_fAccFadeOutTime = 0.f;
	m_fFadeOutStartAlpha = m_fInitialAlpha;
}

void CCommon_SpinSlashTrail::Start_FadeOut(_float fFadeOutDuration)
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

CCommon_SpinSlashTrail* CCommon_SpinSlashTrail::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCommon_SpinSlashTrail* pInstance = new CCommon_SpinSlashTrail(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CCommon_SpinSlashTrail");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CCommon_SpinSlashTrail::Clone(void* pArg)
{
    CCommon_SpinSlashTrail* pInstance = new CCommon_SpinSlashTrail(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CCommon_SpinSlashTrail");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCommon_SpinSlashTrail::Free()
{
    __super::Free();
}
