#include "Vacuum.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CInhaleEffect::CInhaleEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
{
}

CInhaleEffect::CInhaleEffect(const CInhaleEffect& Prototype)
    : CEffect_Mesh(Prototype)
{
}

HRESULT CInhaleEffect::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CInhaleEffect::Initialize(void* pArg)
{
    INHALE_EFFECT_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_InhaleEffect");

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnKnownTexture = true;

    tDesc.bUseTextureCom = true;
    tDesc.iTextureLevel = Texture_Wind01.iLevelID;
    tDesc.wstrTextureTag = Texture_Wind01.szProtoTag;

    tDesc.bUseMaskCom = true;
    tDesc.iMaskLevel = Texture_Wind01.iLevelID;
    tDesc.wstrMaskTag = Texture_Wind01.szProtoTag;

    tDesc.bCustomShader = false;
    tDesc.iShaderLevel = 0;
    tDesc.wstrShaderTag = L"";

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    return S_OK;
}

void CInhaleEffect::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CInhaleEffect::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CInhaleEffect::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CInhaleEffect::Render()
{
    __super::Render();

    return S_OK;
}

CInhaleEffect* CInhaleEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CInhaleEffect* pInstance = new CInhaleEffect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CInhaleEffect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CInhaleEffect::Clone(void* pArg)
{
    CInhaleEffect* pInstance = new CInhaleEffect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CInhaleEffect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CInhaleEffect::Free()
{
    __super::Free();
}