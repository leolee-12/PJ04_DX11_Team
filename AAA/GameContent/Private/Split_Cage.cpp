#include "Split_Cage.h"
#include "GameContent_const.h"
#include "MeshParticleCommon.h"

CSplit_Cage::CSplit_Cage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CSplit_Cage::CSplit_Cage(const CSplit_Cage& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CSplit_Cage::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

HRESULT CSplit_Cage::Ready_EffectPartObjects()
{
    CMeshParticleCommon::MESH_PARTICLE_COMMON_DESC tDesc{};
    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.bUseTextureCom = false;
    tDesc.bUseDiffuseTexture = true;
    tDesc.bUseNormalTexture = true;
    tDesc.bUseMRATexture = true;
    tDesc.bCustomShader = true;
    tDesc.iShaderLevel = Shader_EffectRock.iLevelID;
    tDesc.wstrShaderTag = Shader_EffectRock.szProtoTag;

    tDesc.wstrModelTag = L"Prototype_Component_Model_CagePiece01";
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"CagePiece01", &tDesc)))
        return E_FAIL;

    tDesc.wstrModelTag = L"Prototype_Component_Model_CagePiece02";
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"CagePiece02", &tDesc)))
        return E_FAIL;

    tDesc.wstrModelTag = L"Prototype_Component_Model_CagePiece03";
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshParticleCommon::PROTOTYPE_TAG, L"CagePiece03", &tDesc)))
        return E_FAIL;

    return S_OK;
}

CSplit_Cage* CSplit_Cage::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSplit_Cage* pInstance = new CSplit_Cage(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSplit_Cage");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSplit_Cage::Clone(void* pArg)
{
    CSplit_Cage* pInstance = new CSplit_Cage(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSplit_Cage");
        Safe_Release(pInstance);
    }

    return pInstance;
}
