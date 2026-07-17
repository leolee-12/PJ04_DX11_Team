#include "LensFlare.h"
#include "GameContent_const.h"
#include "RectEmitterCommon.h"
#include "MeshEmitterCommon.h"

CLensFlare::CLensFlare(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CLensFlare::CLensFlare(const CLensFlare& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CLensFlare::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CLensFlare::Initialize(void* pArg)
{
    EFFECT_CONTAINER_DESC* pDesc = static_cast<EFFECT_CONTAINER_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CLensFlare::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CLensFlare::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CLensFlare::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CLensFlare::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CLensFlare::Ready_EffectPartObjects()
{
    CRectEmitterCommon::RECT_EMITTER_COMMON_DESC tRectDesc{};
    tRectDesc.iVIBufferLevel = VI_Rect.iLevelID;
    tRectDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
    tRectDesc.bUseTextureCom = true;
    tRectDesc.iTextureLevel = m_iPrototypeLevel;
    tRectDesc.bUseMaskCom = false;
    tRectDesc.bCustomShader = false;

    tRectDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_CircleGlow2");
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, TEXT("Line"), &tRectDesc)))
        return E_FAIL;

    tRectDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle06");
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, TEXT("Circle1"), &tRectDesc)))
        return E_FAIL;

    CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tMeshDesc{};
    tMeshDesc.iModelLevel = m_iPrototypeLevel;
    tMeshDesc.bUseDiffuseTexture = false;
    tMeshDesc.bUseUnknownTexture = true;
    tMeshDesc.bUseNormalTexture = false;
    tMeshDesc.bUseMRATexture = false;
    tMeshDesc.bUseTextureCom = true;
    tMeshDesc.iTextureLevel = m_iPrototypeLevel;
    tMeshDesc.bUseMaskCom = true;
    tMeshDesc.iMaskLevel = m_iPrototypeLevel;
    tMeshDesc.bCustomShader = false;

    tMeshDesc.wstrModelTag = TEXT("Prototype_Component_Model_LensFlare_Common_Circle01");
    tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_CircleGradation");
    tMeshDesc.wstrMaskTag = TEXT("Prototype_Component_Texture_LensFlare_ThunderRoot2");
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Hexagon1"), &tMeshDesc)))
        return E_FAIL;

    tMeshDesc.wstrModelTag = TEXT("Prototype_Component_Model_LensFlare_Common_Ring01");
    tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Ring08");
    tMeshDesc.wstrMaskTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle11");
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Ring1"), &tMeshDesc)))
        return E_FAIL;

    tMeshDesc.wstrModelTag = TEXT("Prototype_Component_Model_LensFlare_Common_Circle01");
    tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle01");
    tMeshDesc.bUseMaskCom = false;
    tMeshDesc.wstrMaskTag = L"";
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Circle2"), &tMeshDesc)))
        return E_FAIL;

    tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle02");
    tMeshDesc.bUseMaskCom = true;
    tMeshDesc.wstrMaskTag = TEXT("Prototype_Component_Texture_LensFlare_ThunderRoot2");
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Hexagon2"), &tMeshDesc)))
        return E_FAIL;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Hexagon3"), &tMeshDesc)))
        return E_FAIL;

    tMeshDesc.wstrModelTag = TEXT("Prototype_Component_Model_LensFlare_Common_Ring01");
    tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Ring08");
    tMeshDesc.wstrMaskTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle11");
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Ring2"), &tMeshDesc)))
        return E_FAIL;

    tMeshDesc.wstrModelTag = TEXT("Prototype_Component_Model_LensFlare_Common_Circle01");
    tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle04");
    tMeshDesc.wstrMaskTag = TEXT("Prototype_Component_Texture_LensFlare_CircleGradation");
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Circle3"), &tMeshDesc)))
        return E_FAIL;

    return S_OK;
}

CLensFlare* CLensFlare::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLensFlare* pInstance = new CLensFlare(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CLensFlare");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CLensFlare::Clone(void* pArg)
{
    CLensFlare* pInstance = new CLensFlare(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CLensFlare");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLensFlare::Free()
{
    __super::Free();
}