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
#include "TestMeshEmitter.h"
#include "TestMeshParticle.h"
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
#include "UI_BossStatus.h"
#include "UI_FlashCurtain.h"

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
#include "Kirby_DeformCar_Demo.h"
#include "Kirby_DeformCar_Main.h"
#include "Kirby_Sword.h"
#include "Kirby_SwordHat.h"

// Effect_Container
#include "WalkSmoke.h"
#include "SwordSlash1.h"
#include "InhaleContainer.h"
#include "Sword_JumpSlash.h"
#include "Sword_SpinSlash.h"
#include "Sword_SpinSlashTrail.h"
#include "RockBurst.h"
#include "DeathSmoke.h"
#include "RockBounce.h"
#include "RockPull.h"
#include "RockPush.h"
#include "DespawnEffect.h"

#include "BoostGas.h"

// Effect_Part
#include "SmokeSphereOriginal.h"
#include "SmokeLowPoly.h"
#include "SmokeTail.h"
#include "Common_Curve03.h"
#include "Common_Circle01.h"
#include "InhaleEffect.h"
#include "Vacuum.h"
#include "TornadoSpinReverse.h"
#include "Common_Ring03.h"
#include "Common_JumpSlash.h"
#include "Common_SpinSlash.h"
#include "Common_SpinSlashTrail.h"
#include "RockEffect.h"
#include "RockFloorEffect.h"
#include "SmokeParticle.h"
#include "SpinWind.h"

//sky
#include "SkySphere.h"

// Monster
#include "BladeKnight.h"
#include "BladeKnight_Body.h"
#include "BladeKnight_Sword.h"
#include "NormalEnemy.h"
#include "NormalEnemy_Body.h"
#include "Kabu.h"
#include "Kabu_Body.h"
#include "BrontoBurt.h"
#include "BrontoBurt_Body.h"
#include "PoppyBrosJr.h"
#include "PoppyBrosJr_Body.h"
#include "Cappy.h"
#include "Cappy_Body.h"
#include "Cappy_Hat.h"

//Miniboss
#include "GigantEdge.h"
#include "GigantEdge_Body.h"
#include "GigantEdge_Shield.h"
#include "GigantEdge_Sword.h"

//MainBoss
#include "Boss_Gorilla.h"
#include "Boss_Gorilla_Body.h"
#include "CutsceneGorilla.h"
#include "GorillaNamePlate.h"
#include "Boss_Gorilla_RockHole.h"

// LevelDesign
#include "LevelDesign_Unsupported.h"
#include "LevelDesign_Starblock.h"
#include "LevelDesign_Rail.h"
#include "LevelDesign_EventObject.h"

// Projectile
#include "Projectile_Boulder.h"
#include "EnemyBomb.h"

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
    Register_MainBoss();
    Register_Container();
    Register_UIContainer();
    Register_NonAnimObject();
    Register_AnimObject();
    Register_Effect();

    Register_Cutscene();
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
    // 0. WalkSmoke
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
            //TRY_ADD_PROTO(pProxy, iLevelIndex, CTestMeshParticle::PROTOTYPE_TAG,
            //    CTestMeshParticle::Create(pDevice, pContext));
            //TRY_ADD_PROTO(pProxy, iLevelIndex, CTestMeshEmitter::PROTOTYPE_TAG,
            //    CTestMeshEmitter::Create(pDevice, pContext));
        )
    );

    // 1. InhaleContainer
    Register(CInhaleContainer::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CInhaleContainer),
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

            //// Tornado Spin Reverse
            //TRY_ADD_PROTO(pProxy, iLevelIndex, CTornadoSpinReverse::PROTOTYPE_TAG,
            //    CTornadoSpinReverse::Create(pDevice, pContext));
            //TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_TornadoSpinReverse"),
            //    CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSE/Effect/TornadoSpinReverse/Tornado_00_TornadoSpinReverse.ysh",
            //        XMMatrixRotationX(XMConvertToRadians(90.f))
            //    ));
        )
    );

    // 2. SwordSlash
    Register(CSwordSlash1::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSwordSlash1),
        LOADER
        (
            // Common_Ring03
            TRY_ADD_PROTO(pProxy, iLevelIndex, CCommon_Ring03::PROTOTYPE_TAG,
                CCommon_Ring03::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Common_Ring03"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSE/Effect/Common_Ring03/Model_Common_Ring03.ysh",
                    XMMatrixRotationX(XMConvertToRadians(-90.f))));
            TRY_ADD_PROTO(pProxy, Texture_Common_Ring02.iLevelID, Texture_Common_Ring02.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Common_Ring02.szFileTag, Texture_Common_Ring02.iNumTex));

            TRY_ADD_PROTO(pProxy, Texture_SwordSlash2.iLevelID, Texture_SwordSlash2.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_SwordSlash2.szFileTag, Texture_SwordSlash2.iNumTex));
        )
    );

    // 3. JumpSlash
    Register(CSword_JumpSlash::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSword_JumpSlash),
        LOADER
        (
            // Common_JumpSlash
            TRY_ADD_PROTO(pProxy, iLevelIndex, CCommon_JumpSlash::PROTOTYPE_TAG,
                CCommon_JumpSlash::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Common_JumpSlash"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSE/Effect/Common_JumpSlash/Model_Common_JumpSlash.ysh"));
            TRY_ADD_PROTO(pProxy, Texture_Common_JumpSlash.iLevelID, Texture_Common_JumpSlash.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Common_JumpSlash.szFileTag, Texture_Common_JumpSlash.iNumTex));

            // Common_Curve03
            TRY_ADD_PROTO(pProxy, iLevelIndex, CCommon_Curve03::PROTOTYPE_TAG,
                CCommon_Curve03::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Common_Curve03"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSE/Effect/Common_Curve03/Model_Common_Curve03.ysh",
                    XMMatrixRotationY(XMConvertToRadians(90.f))));
            TRY_ADD_PROTO(pProxy, Texture_Common_Flash02.iLevelID, Texture_Common_Flash02.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Common_Flash02.szFileTag, Texture_Common_Flash02.iNumTex));
        )
    );

    // 4. SpinSlash
    Register(CSword_SpinSlash::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSword_SpinSlash),
        LOADER
        (
            // Common_SpinSlash
            TRY_ADD_PROTO(pProxy, iLevelIndex, CCommon_SpinSlash::PROTOTYPE_TAG,
                CCommon_SpinSlash::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Common_SpinSlash"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSE/Effect/Common_SpinSlash/Model_Common_SpinSlash.ysh",
                    XMMatrixRotationX(XMConvertToRadians(90.f))));
            TRY_ADD_PROTO(pProxy, Texture_Common_SpinSlash_1.iLevelID, Texture_Common_SpinSlash_1.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Common_SpinSlash_1.szFileTag, Texture_Common_SpinSlash_1.iNumTex));

            TRY_ADD_PROTO(pProxy, Texture_Common_SpinSlash_2.iLevelID, Texture_Common_SpinSlash_2.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Common_SpinSlash_2.szFileTag, Texture_Common_SpinSlash_2.iNumTex));
        )
    );

    // 5. SpinSlashTrail
    Register(CSword_SpinSlashTrail::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSword_SpinSlashTrail),
        LOADER
        (
            // Common_SpinSlashTrail
            TRY_ADD_PROTO(pProxy, iLevelIndex, CCommon_SpinSlashTrail::PROTOTYPE_TAG,
                CCommon_SpinSlashTrail::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Common_SpinSlashTrail"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSE/Effect/Common_SpinSlashTrail/Model_Common_SpinSlashTrail.ysh",
                    XMMatrixRotationX(XMConvertToRadians(90.f))));
            TRY_ADD_PROTO(pProxy, Texture_Common_SpinSlashTrail.iLevelID, Texture_Common_SpinSlashTrail.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Common_SpinSlashTrail.szFileTag, Texture_Common_SpinSlashTrail.iNumTex));
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

            // Kirby_DeformCar_Demo
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_DeformCar_Demo::PROTOTYPE_TAG,
                CKirby_DeformCar_Demo::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Kirby_DeformCar_Demo"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/DeformCar/Demo.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));

            // Kirby_DeformCar_Main
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_DeformCar_Main::PROTOTYPE_TAG,
                CKirby_DeformCar_Main::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Kirby_DeformCar_Main"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/DeformCar/Main.ysh",
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

    // 2. NormalEnemy
    Register
    (
        CNormalEnemy::PROTOTYPE_TAG, TEXT("Monster"),
        CREATOR(CNormalEnemy),
        LOADER
        (
            // NormalEnemy_Body
            TRY_ADD_PROTO(pProxy, iLevelIndex, CNormalEnemy_Body::PROTOTYPE_TAG,
                CNormalEnemy_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_NormalEnemy_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/NormalEnemy/NormalEnemy.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );

    // 3. Kabu
    Register
    (
        CKabu::PROTOTYPE_TAG, TEXT("Monster"),
        CREATOR(CKabu),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKabu_Body::PROTOTYPE_TAG, CKabu_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Kabu_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/Kabu/Kabu.ysh",
                    XMMatrixScaling(1.f, 1.f, 1.f) * XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );

    // 4. BrontoBurt
    Register
    (
        CBrontoBurt::PROTOTYPE_TAG, TEXT("Monster"),
        CREATOR(CBrontoBurt),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBrontoBurt_Body::PROTOTYPE_TAG, CBrontoBurt_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_BrontoBurt_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/BrontoBurt/BrontoBurt.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );

    // 5. PoppyBrosJr
    Register
    (
        CPoppyBrosJr::PROTOTYPE_TAG, TEXT("Monster"),
        CREATOR(CPoppyBrosJr),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CPoppyBrosJr_Body::PROTOTYPE_TAG, CPoppyBrosJr_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_PoppyBrosJr_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/PoppyBrosJr/Model/PoppyBrosJr.ysh",
                XMMatrixRotationY(XMConvertToRadians(180.f))));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CEnemyBomb::PROTOTYPE_TAG,
                CEnemyBomb::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CEnemyBomb::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/PoppyBrosJr/EnemyBomb/EnemyBomb.ysh"
                    , XMMatrixRotationX(XMConvertToRadians(90.f))* XMMatrixRotationY(XMConvertToRadians(180.f))
                ));
        )
    );

    // 6. Cappy
    Register
    (
        CCappy::PROTOTYPE_TAG, TEXT("Monster"),
        CREATOR(CCappy),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CCappy_Body::PROTOTYPE_TAG, CCappy_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Cappy_Body"),
                    CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/Cappy/Body/Cappy_Body.ysh",
                        XMMatrixRotationY(XMConvertToRadians(180.f))));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CCappy_Hat::PROTOTYPE_TAG, CCappy_Hat::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Cappy_Hat"),
                    CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/Cappy/Hat/Cappy_Hat.ysh",
                        XMMatrixRotationY(XMConvertToRadians(180.f))));
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

    Register(CUI_BossStatus::PROTOTYPE_TAG,
        TEXT("UI_CONTAINER"),
        CREATOR(CUI_BossStatus),
        LOADER()
    );

    Register(CUI_FlashCurtain::PROTOTYPE_TAG,
        TEXT("UI_CONTAINER"),
        CREATOR(CUI_FlashCurtain),
        LOADER());
}

void CGameObject_Factory::Register_NonAnimObject()
{
    Register(CLevelDesign_Unsupported::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLevelDesign_Unsupported), LOADER());
    Register(CLevelDesign_Rail::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLevelDesign_Rail), LOADER());

    Register(CLevelDesign_Starblock::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLevelDesign_Starblock),
        LOADER(TRY_ADD_PROTO(pProxy, ETOUI(LEVEL::GAMEPLAY), CLevelDesign_Starblock::STARBLOCK_MODEL_PROTO_TAG,
            CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Gimmick/NonAnim/Star/H1W1.ysh"));));
}

void CGameObject_Factory::Register_AnimObject()
{
    Register(CLevelDesign_EventObject::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLevelDesign_EventObject),
        LOADER(TRY_ADD_PROTO(pProxy, ETOUI(LEVEL::GAMEPLAY), CLevelDesign_EventObject::LEVEL1BOSSDEMOBG_MODEL_PROTO_TAG,
            CModel::Create_WithTextureHub(pDevice, pContext, MODEL::ANIM, "../../Resources/Map/Gimmick/Anim/Level1BossDemoBg/Level1BossDemoBg.ysh"));));
}

void CGameObject_Factory::Register_Effect()
{
    // 0
    Register(CRockBurst::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CRockBurst),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRockFloorEffect::PROTOTYPE_TAG,
                CRockFloorEffect::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRockFloorEffect::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSH/Boss/Gorilla/RockFloorEffect/RockFloorEffectModel.ysh"));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CRockEffect::PROTOTYPE_TAG,
                CRockEffect::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRockEffect::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSH/Boss/Gorilla/RockEffect/RockEffectModel.ysh"));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CSpinWind::PROTOTYPE_TAG,
                CSpinWind::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSpinWind::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSH/Boss/Gorilla/ArmSpinWind/BossGorilla_00_TornadoPieceMedium.ysh"));
        )
    );

    Register(CDeathSmoke::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CDeathSmoke),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeParticle::PROTOTYPE_TAG,
                CSmokeParticle::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );

    Register(CRockBounce::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CRockBounce),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeParticle::PROTOTYPE_TAG,
                CSmokeParticle::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );

    Register(CRockPull::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CRockPull),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeParticle::PROTOTYPE_TAG,
                CSmokeParticle::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );

    Register(CRockPush::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CRockPush),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeParticle::PROTOTYPE_TAG,
                CSmokeParticle::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );



    // 1
    Register(CBoostGas::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CBoostGas),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeSphereOriginal::PROTOTYPE_TAG,
                CSmokeSphereOriginal::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );

    // 2
    Register(CDespawnEffect::PROTOTYPE_TAG, TEXT("Effect_Container"), 
        CREATOR(CDespawnEffect),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeParticle::PROTOTYPE_TAG,
                CSmokeParticle::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );
}

void CGameObject_Factory::Register_MiniBoss()
{
    Register(CGigantEdge::PROTOTYPE_TAG, TEXT("MiniBoss"),
        CREATOR(CGigantEdge),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_GigantEdge_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/MiniBoss/GigantEdge/Model/GigantEdge.ysh",
                    XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixRotationY(XMConvertToRadians(180.f))));
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

void CGameObject_Factory::Register_MainBoss()
{
    Register(CBoss_Gorilla::PROTOTYPE_TAG, TEXT("MainBoss"),
        CREATOR(CBoss_Gorilla),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Boss_Gorilla_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/Boss/Gorilla/Body/Gorilla.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Gorilla_Body::PROTOTYPE_TAG,
                CBoss_Gorilla_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CProjectile_Boulder::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/Boss/Gorilla/Rock_Projectile/Rock_Anim.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CProjectile_Boulder::PROTOTYPE_TAG,
                CProjectile_Boulder::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Gorilla_RockHole::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/Boss/Gorilla/RockHole/RockHole_Anim.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Gorilla_RockHole::PROTOTYPE_TAG,
                CBoss_Gorilla_RockHole::Create(pDevice, pContext));
        )
    );

    Register(CGorillaNamePlate::PROTOTYPE_TAG, TEXT("NamePlate"),
        CREATOR(CGorillaNamePlate),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_GorillaNamePlate"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSH/Boss/Gorilla/NamePlate/Model_KR.ysh"));
        )
    );
}

void CGameObject_Factory::Register_Cutscene()
{
    Register(CCutsceneGorilla::PROTOTYPE_TAG, TEXT("CutsceneActor"),
        CREATOR(CCutsceneGorilla),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Boss_Gorilla_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/Boss/Gorilla/Body/Gorilla.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );
}

void CGameObject_Factory::Free()
{
    m_Registrations.clear();
}
