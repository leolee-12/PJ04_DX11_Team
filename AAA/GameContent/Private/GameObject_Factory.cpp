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
#include "TestMarb1e.h"
#include "TestMarb1eMap.h"
#include "UI_Image.h"
#include "UI_TestImageContainer.h"

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
    Register(CUI_Image::PROTOTYPE_TAG, TEXT("UI_OBJECT"),
        CREATOR(CUI_Image),
        LOADER(
            TRY_ADD_PROTO(pProxy, Shader_UI.iLevelID, Shader_UI.szProtoTag,
                CShader::Create(pDevice, pContext, Shader_UI.szFileTag,
                    VTXTEX::Elements, VTXTEX::iNumElements));

    TRY_ADD_PROTO(pProxy, VI_Rect.iLevelID, VI_Rect.szProtoTag,
        CVIBuffer_Rect::Create(pDevice, pContext));

    TRY_ADD_PROTO(pProxy, ETOUI(LEVEL::STATIC), TEXT("Proto_Tex_TestUI"),
        CTexture::Create(pDevice, pContext,
            TEXT("../../Resources/Models/Test/Aligator/ModelC_BaseColor.1238033416.png"), 1));
        )
    );
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
        LOADER(
            TRY_ADD_PROTO(pProxy, Texture_Common_Flash02.iLevelID, Texture_Common_Flash02.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Common_Flash02.szFileTag, Texture_Common_Flash02.iNumTex));

            TRY_ADD_PROTO(pProxy, Texture_TestMask.iLevelID, Texture_TestMask.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_TestMask.szFileTag, Texture_TestMask.iNumTex));
        )
    );
    Register(CSample_MeshEffect::PROTOTYPE_TAG, TEXT("TEST_OBJECT"),
        CREATOR(CSample_MeshEffect),
        LOADER(
            TRY_ADD_PROTO(pProxy, (ETOUI(LEVEL::GAMEPLAY)), TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Models/Test/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));

            TRY_ADD_PROTO(pProxy, Texture_Common_Flash02.iLevelID, Texture_Common_Flash02.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Common_Flash02.szFileTag, Texture_Common_Flash02.iNumTex));

            TRY_ADD_PROTO(pProxy, Texture_TestMask.iLevelID, Texture_TestMask.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_TestMask.szFileTag, Texture_TestMask.iNumTex));
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
}

void CGameObject_Factory::Register_Container()
{
   
}

void CGameObject_Factory::Register_UIContainer()
{
    Register(CUI_TestImageContainer::PROTOTYPE_TAG, TEXT("UI_CONTAINER_TEST"),
        CREATOR(CUI_TestImageContainer),
        LOADER(
            auto* pImageReg = CGameObject_Factory::GetInstance()
            ->Get_Registration(CUI_Image::PROTOTYPE_TAG);

    if (pImageReg)
    {
        pImageReg->ResourceLoader(pProxy, pDevice, pContext);

        TRY_ADD_PROTO(pProxy, ETOUI(LEVEL::STATIC), CUI_Image::PROTOTYPE_TAG,
            pImageReg->CreatorFunc(pDevice, pContext));
    }
        )
    );
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
