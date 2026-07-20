#include "LD_MeteorGenerator.h"
#include "LevelDesign_Registry.h"
#include "GameInstance.h"

CLD_MeteorGenerator::CLD_MeteorGenerator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevelDesignObject(pDevice, pContext)
{
}

CLD_MeteorGenerator::CLD_MeteorGenerator(const CLD_MeteorGenerator& Prototype)
    : CLevelDesignObject(Prototype)
{
}

HRESULT CLD_MeteorGenerator::Initialize_Prototype()
{
    return __super::Initialize_Prototype();
}

HRESULT CLD_MeteorGenerator::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void CLD_MeteorGenerator::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
    if (nullptr == pOutData)
        return;

    pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_MeteorGenerator::Register_LevelDesignSpecs()
{
    const _wstring ObjectNames[] =
    {
        L"VolcanoRockLarge",
        L"VolcanoRockSmall",
    };

    for (const _wstring& strObjectName : ObjectNames)
    {
        LD_SPAWN_SPEC Spec{};
        Spec.strObjectName = strObjectName;
        Spec.strPrototypeTag = PROTOTYPE_TAG;
        Spec.strLayerTag = LAYER_TAG;
        Spec.eCategory = LD_CATEGORY::GIMMICK;
        Spec.pPrototypeFactory = &Create_Prototype;
        Spec.pBuildDesc = nullptr;
        Spec.bUseFactoryResourceLoader = true;

        CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
    }
}

CGameObject* CLD_MeteorGenerator::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return CLD_MeteorGenerator::Create(pDevice, pContext);
}

CLD_MeteorGenerator* CLD_MeteorGenerator::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLD_MeteorGenerator* pInstance = new CLD_MeteorGenerator(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CLD_MeteorGenerator");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CLD_MeteorGenerator::Clone(void* pArg)
{
    CLD_MeteorGenerator* pInstance = new CLD_MeteorGenerator(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CLD_MeteorGenerator");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CLD_MeteorGenerator::Free()
{
    __super::Free();
}
