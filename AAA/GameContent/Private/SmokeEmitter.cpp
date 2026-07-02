#include "SmokeEmitter.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CSmokeEmitter::CSmokeEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_MeshEmitter{ pDevice, pContext }
{
}

CSmokeEmitter::CSmokeEmitter(const CSmokeEmitter& Prototype)
    : CEffect_MeshEmitter(Prototype)
{
}

HRESULT CSmokeEmitter::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CSmokeEmitter::Initialize(void* pArg)
{
    SMOKE_EMITTER_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_SmokeSphereOriginal");

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnKnownTexture = true;

    tDesc.bUseTextureCom = false;
    tDesc.iTextureLevel = 0;
    tDesc.wstrTextureTag = L"";

    tDesc.bUseMaskCom = false;
    tDesc.iMaskLevel = 0;
    tDesc.wstrMaskTag = L"";

    tDesc.bCustomShader = false;
    //tDesc.iShaderLevel = 0;
    //tDesc.wstrShaderTag = L"";

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    return S_OK;
}

void CSmokeEmitter::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CSmokeEmitter::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CSmokeEmitter::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CSmokeEmitter::Render()
{
    __super::Render();

    return S_OK;
}

CSmokeEmitter* CSmokeEmitter::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSmokeEmitter* pInstance = new CSmokeEmitter(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSmokeEmitter");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSmokeEmitter::Clone(void* pArg)
{
    CSmokeEmitter* pInstance = new CSmokeEmitter(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSmokeEmitter");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSmokeEmitter::Free()
{
    __super::Free();
}