#include "GameObject_Factory.h"
#include "GameContent_const.h"
#include "GameInstance.h"
#include "GameObject.h"
#include "Camera_Free.h"
#include "Terrain.h"
#include "TestFiona.h"
#include "TestNonAnim.h"
#include "TestRect.h"
#include "TestEffectQuad.h"
#include "Sample_MeshEffect.h"
#include "TestMap.h"
#include "TestMarb1e.h"
#include "TestMarb1eMap.h"

IMPLEMENT_SINGLETON(CGameObject_Factory)

#define CREATOR(CLASS) \
        [](ID3D11Device* pDevice, ID3D11DeviceContext* pContext)  \
{ return dynamic_cast<CBase*>(CLASS::Create(pDevice, pContext)); }

#define LOADER(...) \
        [](CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext) { __VA_ARGS__; }

#define TRY_ADD_PROTO(proxy, level, tag, createExpr) \
      if (!proxy->Has_Prototype(level, tag)) \
          proxy->Add_Prototype(level, tag, createExpr)

void CGameObject_Factory::Copy_RegisteredTags(vector<wstring>* pOutTags)
{
    if (nullptr == pOutTags)
        return;

    pOutTags->clear();
    for (auto& Pair : m_Registrations)
    {
        pOutTags->push_back(Pair.first);
    }
}

void CGameObject_Factory::Copy_TagsByCategory(map<wstring, vector<wstring>>* pOutMap)
{
    pOutMap->clear();
    for (auto& [tag, reg] : m_Registrations)
        (*pOutMap)[reg.strCategory].push_back(tag);
}

void CGameObject_Factory::RegisterAll()
{
    Register_UI();
    Register_Camera();
    Register_Test();
    Register_Container();
    Register_UIContainer();
    Register_NonAnimObject();
    Register_AnimObject();
    Register_Effect();

}

void CGameObject_Factory::Register_UI()
{
}

void CGameObject_Factory::Register_Camera()
{
    Register(TEXT("Proto_CameraFree"), TEXT("CAMERA_OBJECT"),
        CREATOR(CCamera_Free), LOADER()
    );
}

void CGameObject_Factory::Register_Test()
{
    Register(TEXT("Proto_TestFiona"), TEXT("TEST_OBJECT"),
        CREATOR(CTestFiona),
        LOADER(
            /*pProxy->Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Fiona"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Models/Test/Aligator/aligator.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))))*/
            pProxy->Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Fiona"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/Models/Test/Aligator/Aligator_Anim.ysh"
                    //, XMMatrixRotationY(XMConvertToRadians(180.f))
                ))
        )
    );


    Register(TEXT("Proto_TestNonAnim"), TEXT("TEST_OBJECT"),
        CREATOR(CTestNonAnim),
        LOADER(
            pProxy->Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_NonAnim"),
                CModel::Create(pDevice, pContext, MODEL::MAP, "../../Resources/Models/Test/Marb1e/Land_GsAllBuilding_0.ysh"
                    //,XMMatrixRotationY(XMConvertToRadians(180.f))
                ))
        )
    );

    Register(CTestRect::PROTOTYPE_TAG, TEXT("TEST_OBJECT"),
        CREATOR(CTestRect),
        LOADER()
    );

    Register(CTestEffectQuad::PROTOTYPE_TAG, TEXT("TEST_OBJECT"),
        CREATOR(CTestEffectQuad),
        LOADER()
    );
    Register(CSample_MeshEffect::PROTOTYPE_TAG, TEXT("TEST_OBJECT"),
        CREATOR(CSample_MeshEffect),
        LOADER(
            TRY_ADD_PROTO(pProxy, (ETOUI(LEVEL::GAMEPLAY)), TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Models/Test/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );


    Register(CTestMarb1e::PROTOTYPE_TAG, TEXT("TEST_OBJECT"),
        CREATOR(CTestMarb1e),
        LOADER(
            pProxy->Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Marb1e"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/Models/Test/Marb1e/BladeKnight.ysh"
                    //, XMMatrixRotationY(XMConvertToRadians(180.f))
                ))
        )
    );

    Register(CTestMarb1eMap::PROTOTYPE_TAG, TEXT("TEST_OBJECT"),
        CREATOR(CTestMarb1eMap),
        LOADER(
            pProxy->Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Map"),
                CModel::Create(pDevice, pContext, MODEL::MAP, "../../Resources/Models/Test/Marb1e/Land_GsAllBuilding_0.ysh"))
        )
    );

    /*auto RegisterMap = [this](const _tchar* szObjectProtoTag, const _tchar* szModelProtoTag, const _char* szModelPath)
        {
            Register(szObjectProtoTag, TEXT("MAP_OBJECT"),
                [szObjectProtoTag, szModelProtoTag](ID3D11Device* pDevice, ID3D11DeviceContext* pContext) -> CBase*
                {
                    return dynamic_cast<CBase*>(
                        CTestMap::Create(pDevice, pContext, szObjectProtoTag, szModelProtoTag));
                },
                [szModelProtoTag, szModelPath](CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
                {
                    TRY_ADD_PROTO(
                        pProxy,
                        ETOUI(LEVEL::GAMEPLAY),
                        szModelProtoTag,
                        CModel::Create(
                            pDevice,
                            pContext,
                            MODEL::MAP,
                            szModelPath,
                            XMMatrixRotationY(XMConvertToRadians(180.f))));
                }
            );
        };

    RegisterMap(
        TEXT("Proto_Land_GsAllBuilding_0"),
        TEXT("Prototype_Component_Model_Land_GsAllBuilding_0"),
        "../../Resources/Models/Test/Stage1-0/Land_GsAllBuilding_0.ysh");

    RegisterMap(
        TEXT("Proto_Land_GsBuilding_1"),
        TEXT("Prototype_Component_Model_Land_GsBuilding_1"),
        "../../Resources/Models/Test/Stage1-0/Land_GsBuilding_1.ysh");

    RegisterMap(
        TEXT("Proto_Land_GsBuilding_7"),
        TEXT("Prototype_Component_Model_Land_GsBuilding_7"),
        "../../Resources/Models/Test/Stage1-0/Land_GsBuilding_7.ysh");

    RegisterMap(
        TEXT("Proto_Land_GsDefault_2"),
        TEXT("Prototype_Component_Model_Land_GsDefault_2"),
        "../../Resources/Models/Test/Stage1-0/Land_GsDefault_2.ysh");

    RegisterMap(
        TEXT("Proto_Land_GsDefault_5"),
        TEXT("Prototype_Component_Model_Land_GsDefault_5"),
        "../../Resources/Models/Test/Stage1-0/Land_GsDefault_5.ysh");

    RegisterMap(
       TEXT("Proto_Land_SeRock_3"),
       TEXT("Prototype_Component_Model_Land_SeRock_3"),
       "../../Resources/Models/Test/Stage1-0/Land_SeRock_3.ysh");
    
    RegisterMap(
        TEXT("Proto_Land_SeRock_6"),
        TEXT("Prototype_Component_Model_Land_SeRock_6"),
        "../../Resources/Models/Test/Stage1-0/Land_SeRock_6.ysh");

    RegisterMap(
        TEXT("Proto_Land_Transparent_4"),
        TEXT("Prototype_Component_Model_Land_Transparent_4"),
        "../../Resources/Models/Test/Stage1-0/Land_Transparent_4.ysh");*/
}

void CGameObject_Factory::Register_Container()
{
   
}

void CGameObject_Factory::Register_UIContainer()
{
   
}

void CGameObject_Factory::Register_NonAnimObject()
{
    
}

void CGameObject_Factory::Register_AnimObject()
{
    
}

void CGameObject_Factory::Register_Effect()
{
   
}

void CGameObject_Factory::Free()
{
    m_Registrations.clear();
}
