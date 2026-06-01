#include "Sample_MeshEffect.h"

#include "GameInstance.h"
#include "GameContent_const.h"

CSample_MeshEffect::CSample_MeshEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
{
}

CSample_MeshEffect::CSample_MeshEffect(const CSample_MeshEffect& Prototype)
    : CEffect_Mesh(Prototype)
{
}

HRESULT CSample_MeshEffect::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CSample_MeshEffect::Initialize(void* pArg)
{
    SAMPLE_MESHEFFECT_DESC tDesc{};

    tDesc.iModelLevel = ETOUI(LEVEL::GAMEPLAY);
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_SmokeSphereOriginal");

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnKnownTexture = true;

    tDesc.bUseTextureCom = false;
    //tDesc.iTextureLevel = Texture_LoadingUI.iLevelID;
    //tDesc.wstrTextureTag = Texture_LoadingUI.szProtoTag;

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    return S_OK;
}

void CSample_MeshEffect::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CSample_MeshEffect::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CSample_MeshEffect::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CSample_MeshEffect::Render()
{
    __super::Render();

    return S_OK;
}

CSample_MeshEffect* CSample_MeshEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSample_MeshEffect* pInstance = new CSample_MeshEffect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSample_MeshEffect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSample_MeshEffect::Clone(void* pArg)
{
    CSample_MeshEffect* pInstance = new CSample_MeshEffect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSample_MeshEffect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSample_MeshEffect::Free()
{
    __super::Free();
}
