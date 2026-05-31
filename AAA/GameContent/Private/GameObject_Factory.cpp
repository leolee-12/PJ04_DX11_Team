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
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/Models/Test/BladeNight/BladeNight.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))))
        )
    );

    Register(TEXT("Proto_TestNonAnim"), TEXT("TEST_OBJECT"),
        CREATOR(CTestNonAnim),
        LOADER(
            pProxy->Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_NonAnim"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Models/Test/Aligator/Model.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))))
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
