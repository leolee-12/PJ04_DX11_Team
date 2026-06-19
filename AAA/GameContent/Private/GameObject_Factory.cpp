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
#include "TestMap.h"
#include "TestMarb1e.h"
#include "TestMarb1eMap.h"
#include "TestTriggerBox.h"
#include "TestParticle.h"
#include "Material_Object.h"

//UI Container
#include "UI_TestImageContainer.h"
#include "UI_Title.h"
#include "UI_GenericContainer.h"
#include "UI_PointStar.h"
#include "UI_KirbyStatus.h"
#include "UI_FadeOut.h"
#include "UI_LoadingCurtain.h"
#include "UI_FadeIn.h"

// UI Parts
#include "UI_Image.h"
#include "UI_SpriteAnim.h"
#include "UI_Text.h"
#include "UI_Effect.h"
#include "UI_GaugeFill.h"

#include "UI_Curtain.h"
#include "UI_Eraser.h"
#include "UI_SpriteAnimCurtain.h"
#include "UI_CurtainTexture.h"


// Kirby
#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Sword.h"
#include "Kirby_SwordHat.h"

// Effect_Container
#include "WalkSmoke.h"
#include "SwordSlash.h"
#include "VacuumContainer.h"

// Effect_Part
#include "SmokeSphereOriginal.h"
#include "SmokeLowPoly.h"
#include "SmokeTail.h"
#include "Common_Curve03.h"
#include "Common_Circle01.h"
#include "InhaleEffect.h"
#include "Vacuum.h"
#include "TornadoSpinReverse.h"

//sky
#include "SkySphere.h"

// Monster
#include "BladeKnight.h"
#include "BladeKnight_Body.h"
#include "BladeKnight_Sword.h"

//Miniboss
#include "GigantEdge.h"
#include "GigantEdge_Body.h"
#include "GigantEdge_Shield.h"
#include "GigantEdge_Sword.h"

// LevelDesign
#include "LevelDesign_Unsupported.h"
#include "LevelDesign_Breakable.h"
#include "LevelDesign_Rail.h"

IMPLEMENT_SINGLETON(CGameObject_Factory)

#define CREATOR(CLASS) \
        [](ID3D11Device* pDevice, ID3D11DeviceContext* pContext) {\
            CEffect_Allocator::PrototypeScope _ps; \
            return dynamic_cast<CBase*>(CLASS::Create(pDevice, pContext)); \
        }

#define LOADER(...) \
        [](CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iLevelIndex) { __VA_ARGS__; }

#define TRY_ADD_PROTO(proxy, level, tag, createExpr) \
      if (!proxy->Has_Prototype(level, tag)) { \
          CEffect_Allocator::PrototypeScope _ps; \
          proxy->Add_Prototype(level, tag, createExpr); \
      }

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
    Register_MiniBoss();
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
        LOADER());

    Register(CUI_SpriteAnim::PROTOTYPE_TAG, TEXT("UI_OBJECT"),
        CREATOR(CUI_SpriteAnim),
        LOADER());

    Register(CUI_Text::PROTOTYPE_TAG, TEXT("UI_OBJECT"),
        CREATOR(CUI_Text),
        LOADER());

    Register(CUI_Effect::PROTOTYPE_TAG, TEXT("UI_OBJECT"),
        CREATOR(CUI_Effect),
        LOADER());

    Register(CUI_GaugeFill::PROTOTYPE_TAG, TEXT("UI_OBJECT"),
        CREATOR(CUI_GaugeFill),
        LOADER());

    Register(CUI_Curtain::PROTOTYPE_TAG, TEXT("UI_OBJECT"),
        CREATOR(CUI_Curtain),
        LOADER());

    Register(CUI_Eraser::PROTOTYPE_TAG, TEXT("UI_OBJECT"),
        CREATOR(CUI_Eraser),
        LOADER());

    Register(CUI_SpriteAnimCurtain::PROTOTYPE_TAG, TEXT("UI_OBJECT"),
        CREATOR(CUI_SpriteAnimCurtain),
        LOADER());

    Register(CUI_CurtainTexture::PROTOTYPE_TAG, TEXT("UI_OBJECT"),
        CREATOR(CUI_CurtainTexture),
        LOADER());


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
            pProxy->Add_Prototype(iLevelIndex, TEXT("Prototype_Component_Model_Fiona"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/Test/Test/Aligator/Aligator_Anim.ysh"
                    //, XMMatrixRotationY(XMConvertToRadians(180.f))
                ))
        )
    );


    Register(TEXT("Proto_TestNonAnim"), TEXT("TEST_OBJECT"),
        CREATOR(CTestNonAnim),
        LOADER
        (
            pProxy->Add_Prototype(iLevelIndex, TEXT("Prototype_Component_Model_NonAnim"),
                CModel::Create(pDevice, pContext, MODEL::MAP, "../../Resources/Test/Test/Marb1e/Land_GsAllBuilding_0.ysh"))
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


    Register(CTestMarb1e::PROTOTYPE_TAG, TEXT("TEST_OBJECT"),
        CREATOR(CTestMarb1e),
        LOADER(
            pProxy->Add_Prototype(iLevelIndex, TEXT("Prototype_Component_Model_Marb1e"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/Test/Test/Aligator/Aligator_Anim.ysh"
                    //, XMMatrixRotationY(XMConvertToRadians(180.f))
                ))
        )
    );

    Register(CTestMarb1eMap::PROTOTYPE_TAG, TEXT("TEST_OBJECT"),
        CREATOR(CTestMarb1eMap),
        LOADER(
            pProxy->Add_Prototype(iLevelIndex, TEXT("Prototype_Component_Model_Map"),
                CModel::Create(pDevice, pContext, MODEL::MAP, "../../Resources/Test/Test/Marb1e/Land_GsAllBuilding_0.ysh"))
        )
    );

    Register(CMaterial_Object::PROTOTYPE_TAG, TEXT("TEST_OBJECT"),
        CREATOR(CMaterial_Object),
        LOADER(
            pProxy->Add_Prototype(iLevelIndex, TEXT("Prototype_Component_Model_MaterialObject"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Shader_Test_Object/Model_SmokeSphereOriginal.ysh"))
        )
    );




    Register(CSkySphere::PROTOTYPE_TAG, TEXT("TEST_OBJECT"),
        CREATOR(CSkySphere),
        LOADER()
    );

    // Effect_Container
    // 1. WalkSmoke
    Register(CWalkSmoke::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CWalkSmoke),
        LOADER
        (
            // SmokeSphereOriginal
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeSphereOriginal::PROTOTYPE_TAG,
                CSmokeSphereOriginal::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
            // SmokeLowPoly
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeLowPoly::PROTOTYPE_TAG,
                CSmokeLowPoly::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeLowPoly"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeLowPoly/Model_SmokeLowPoly.ysh"));
            // SmokeTail
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeTail::PROTOTYPE_TAG,
                CSmokeTail::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeTail"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeTail/Model_SmokeTail.ysh"));

            //TRY_ADD_PROTO(pProxy, iLevelIndex, CTestParticle::PROTOTYPE_TAG,
            //    CTestParticle::Create(pDevice, pContext));
        )
    );

    // 2. SwordSlash
    Register(CSwordSlash::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSwordSlash),
        LOADER
        (           
            // Common_Curve03
            TRY_ADD_PROTO(pProxy, iLevelIndex, CCommon_Curve03::PROTOTYPE_TAG,
                CCommon_Curve03::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Common_Curve03"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSE/Effect/Common_Curve03/Model_Common_Curve03.ysh"));
            
            //// Common_Circle01
            //TRY_ADD_PROTO(pProxy, iLevelIndex, CCommon_Circle01::PROTOTYPE_TAG,
            //    CCommon_Circle01::Create(pDevice, pContext));
            //TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Common_Circle01"),
            //    CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSE/Effect/Common_Circle01/Model_Common_Circle01.ysh"));
        )
    );


    // 2. VacuumContainer
    Register(CVacuumContainer::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CVacuumContainer),
        LOADER
        (           
            // InhaleEffect
            TRY_ADD_PROTO(pProxy, iLevelIndex, CInhaleEffect::PROTOTYPE_TAG,
                CInhaleEffect::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_InhaleEffect"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSE/Effect/InhaleEffect2M/Common_00_InhaleEffect2M.ysh",
                    XMMatrixRotationX(XMConvertToRadians(90.f))));
            TRY_ADD_PROTO(pProxy, Texture_Wind01.iLevelID, Texture_Wind01.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Wind01.szFileTag, Texture_Wind01.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_Wind02.iLevelID, Texture_Wind02.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Wind02.szFileTag, Texture_Wind02.iNumTex));

            // Vacuum
            TRY_ADD_PROTO(pProxy, iLevelIndex, CVacuum::PROTOTYPE_TAG,
                CVacuum::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Common_00_Vacuum"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSE/Effect/Vacuum/Common_00_Vacuum.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
            
            TRY_ADD_PROTO(pProxy, Texture_Twincle.iLevelID, Texture_Twincle.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Twincle.szFileTag, Texture_Twincle.iNumTex));

            // Tornado Spin Reverse
            TRY_ADD_PROTO(pProxy, iLevelIndex, CTornadoSpinReverse::PROTOTYPE_TAG,
                CTornadoSpinReverse::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_TornadoSpinReverse"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSE/Effect/TornadoSpinReverse/Tornado_00_TornadoSpinReverse.ysh",
                    XMMatrixRotationX(XMConvertToRadians(90.f))
                ));
        )
    );

    Register(CTestTriggerBox::PROTOTYPE_TAG, TEXT("TEST_OBJECT"),
        CREATOR(CTestTriggerBox),
        LOADER()
    );
}

void CGameObject_Factory::Register_Container()
{
    // Kirby
    Register
    (
        CKirby::PROTOTYPE_TAG, TEXT("Kirby"),
        CREATOR(CKirby),
        LOADER
        (
            // Kirby_Body
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_Body::PROTOTYPE_TAG,
                CKirby_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Kirby_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/Kirby/Kirby.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));

            // Sword
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_Sword::PROTOTYPE_TAG,
                CKirby_Sword::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Sword"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/Sword/Sword/Sword.ysh"));

            // Sword Hat
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_SwordHat::PROTOTYPE_TAG,
                CKirby_SwordHat::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SwordHat"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/Sword/Hat/SwordHat.ysh"));
        )
    ); 

    // Monster
    
    // 1. BladeKnight(Sword)
    Register
    (
        CBladeKnight::PROTOTYPE_TAG, TEXT("Monster"),
        CREATOR(CBladeKnight),
        LOADER
        (
            // BladeKnight Body
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBladeKnight_Body::PROTOTYPE_TAG,
                CBladeKnight_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_BladeKnight_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/BladeKnight/BladeKnight.ysh",
                XMMatrixRotationY(XMConvertToRadians(180.f))));

            // BladeKnight Sword
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBladeKnight_Sword::PROTOTYPE_TAG, CBladeKnight_Sword::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_BladeKnight_Sword"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/BladeKnight/BladeKnight_Sword.ysh"));
         )
    );
}

void CGameObject_Factory::Register_UIContainer()
{
    Register(CUI_TestImageContainer::PROTOTYPE_TAG,
        TEXT("UI_CONTAINER_TEST"),
        CREATOR(CUI_TestImageContainer),
        LOADER()
    );

    Register(CUI_Title::PROTOTYPE_TAG,
        TEXT("UI_CONTAINER"),
        CREATOR(CUI_Title),
        LOADER()
    );

    Register(CUI_GenericContainer::PROTOTYPE_TAG,
        TEXT("UI_CONTAINER"),
        CREATOR(CUI_GenericContainer),
        LOADER()
    );
    
    Register(CUI_PointStar::PROTOTYPE_TAG,
        TEXT("UI_CONTAINER"),
        CREATOR(CUI_PointStar),
        LOADER()
    );

    Register(CUI_KirbyStatus::PROTOTYPE_TAG,
        TEXT("UI_CONTAINER"),
        CREATOR(CUI_KirbyStatus),
        LOADER()
    );
    
    Register(CUI_FadeOut::PROTOTYPE_TAG,
        TEXT("UI_CONTAINER"),
        CREATOR(CUI_FadeOut),
        LOADER()
    );

    Register(CUI_LoadingCurtain::PROTOTYPE_TAG,
        TEXT("UI_CONTAINER"),
        CREATOR(CUI_LoadingCurtain),
        LOADER()
    );

    Register(CUI_FadeIn::PROTOTYPE_TAG,
        TEXT("UI_CONTAINER"),
        CREATOR(CUI_FadeIn),
        LOADER()
    );
}

void CGameObject_Factory::Register_NonAnimObject()
{
    Register(CLevelDesign_Unsupported::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLevelDesign_Unsupported), LOADER());
    Register(CLevelDesign_Rail::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLevelDesign_Rail), LOADER());

    Register(CLevelDesign_Breakable::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLevelDesign_Breakable),
        LOADER(TRY_ADD_PROTO(pProxy, ETOUI(LEVEL::GAMEPLAY), CLevelDesign_Breakable::STARBLOCK_MODEL_PROTO_TAG,
            CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Gimmick/Star/H1W1.ysh"));));
}

void CGameObject_Factory::Register_AnimObject()
{
    
}

void CGameObject_Factory::Register_Effect()
{
   
}

void CGameObject_Factory::Register_MiniBoss()
{
    Register(CGigantEdge::PROTOTYPE_TAG, TEXT("MiniBoss"),
        CREATOR(CGigantEdge),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_GigantEdge_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/MiniBoss/GigantEdge/Model/GigantEdge.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_GigantEdge_Shield"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/MiniBoss/GigantEdge/Shield/Shield.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_GigantEdge_Sword"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/MiniBoss/GigantEdge/Sword/Sword.ysh",
                    XMMatrixTranslation(0.f, 0.f, -2.5f)));


            TRY_ADD_PROTO(pProxy, iLevelIndex, CGigantEdge_Body::PROTOTYPE_TAG,
                CGigantEdge_Body::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CGigantEdge_Sword::PROTOTYPE_TAG,
                CGigantEdge_Sword::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CGigantEdge_Shield::PROTOTYPE_TAG,
                CGigantEdge_Shield::Create(pDevice, pContext));
        )
    );
}

void CGameObject_Factory::Free()
{
    m_Registrations.clear();
}
