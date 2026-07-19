#include "GameObject_Factory.h"
#include "GameContent_const.h"
#include "GameInstance.h"
#include "GameObject.h"
#include "Camera_Free.h"

//UI Container
#include "UI_Title.h"
#include "UI_GenericContainer.h"
#include "UI_PointStar.h"
#include "UI_KirbyStatus.h"
#include "UI_FadeOut.h"
#include "UI_LoadingCurtain.h"
#include "UI_FadeIn.h"
#include "UI_BossStatus.h"
#include "UI_FlashCurtain.h"
#include "UI_MovableContainer.h"
#include "UI_StageClear.h"
#include "UI_CoordinatorContainer.h"
#include "UI_MissionBoard.h"
#include "UI_MissionPanel.h"
#include "UI_LetterBox.h"
#include "UI_TitleLogo.h"
#include "UI_CutFade.h"
#include "UI_Dialogue.h"
#include "UI_QTE.h"

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
#include "Kirby_DeformCylinder_Demo.h"
#include "Kirby_DeformCylinder_Main.h"
#include "Kirby_BombHat.h"
#include "Kirby_IceHat.h"
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
#include "Gorilla_Swing.h"
#include "Gorilla_Ring.h"
#include "DespawnEffect.h"
#include "GetAbilityEffect.h"
#include "SwordChargeEffect.h"
#include "BombExplosion.h"
#include "BoostGas.h"
#include "CarMilkyWay.h"
#include "CommonHit.h"
#include "SpitObject.h"
#include "BombFuseEffect.h"
#include "FlowerPetals.h"
#include "Split_Starblock.h"
#include "Split_Stone.h"
#include "Split_Bush.h"
#include "Split_Coaster.h"
#include "Split_Cylinder.h"
#include "LensFlare.h"
#include "ItemEffect.h"
#include "BombHitAim.h"
#include "BombAimDot.h"
#include "BreakWallEffect.h"
#include "BubbleAura.h"
#include "LaunchSmoke.h"
#include "Kirby_SwordTrail.h"
#include "SmokeCollection.h"
#include "LandingSmoke.h"
#include "MoveSmoke.h"
#include "CarLanding.h"
#include "PickUpEffect.h"
#include "DropStarEffect.h"
#include "SmokeSphereOriginalEmitter.h"
#include "SwordTrail_BK.h"
#include "Tornado_BK.h"
#include "EssenceAura.h"

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
#include "SmokeEmitter.h"
#include "SpinWind.h"
#include "Car_00_MilkyWay.h"
#include "StarParticle.h"
#include "Star2DParticle.h"
#include "SwordCharge.h"
#include "Swing_Smoke.h"
#include "Shockwave.h"
#include "SphereParticle.h"
#include "SphereMesh.h"
#include "Common_SphereNoise.h"
#include "HitMark.h"
#include "Bubble.h"
#include "StarEmitter.h"
#include "Sparkle.h"
#include "MeshEmitterCommon.h"
#include "RectCommon.h"
#include "MeshCommon.h"
#include "RectParticleCommon.h"
#include "MeshParticleCommon.h"
#include "RectEmitterCommon.h"
#include "TrailCommon.h"
#include "EssenceCrown.h"

//sky
#include "SkySphere.h"

// Monster
#include "BladeKnight.h"
#include "NormalEnemy.h"
#include "Kabu.h"
#include "BrontoBurt.h"
#include "PoppyBrosJr.h"
#include "Cappy.h"
#include "NormalEnemyWild.h"
#include "Dekabu.h"
#include "Bouncy.h"
#include "RabbitEnemy.h"

// MonsterPart
#include "BladeKnight_Body.h"
#include "BladeKnight_Sword.h"
#include "NormalEnemy_Body.h"
#include "Kabu_Body.h"
#include "BrontoBurt_Body.h"
#include "PoppyBrosJr_Body.h"
#include "Cappy_Body.h"
#include "Cappy_Hat.h"
#include "NormalEnemyWild_Body.h"
#include "Dekabu_Body.h"
#include "Bouncy_Body.h"
#include "RabbitEnemy_Body.h"

//Miniboss
#include "GigantEdge.h"
#include "GigantEdge_Body.h"
#include "GigantEdge_Shield.h"
#include "GigantEdge_Sword.h"

//MainBoss
#include "Boss_Gorilla.h"
#include "Boss_GorillaRush.h"
#include "Boss_Gorilla_Body.h"
#include "CutsceneGorilla.h"
#include "GorillaNamePlate.h"
#include "Boss_Gorilla_RockHole.h"
#include "Boss_Cage.h"
#include "Boss_Cage_Body.h"
#include "Cage_WaddleDee.h"

#include "Boss_Armadillo.h"
#include "Boss_Armadillo_Body.h"
#include "Projectile_Partner.h"
#include "Boss_Armadillo_Cage.h"

#include "Boss_Leopard.h"
#include "Boss_Leopard_Body.h"
#include "Projectile_Nail.h"

//Boss Effect
#include "Armadillo_RutA.h"
#include "Armadillo_RutB.h"
#include "Armadillo_Dust.h"
#include "Armadillo_RollWind.h"
#include "Armadillo_SpinWind.h"
#include "Armadillo_WallImpact.h"

#include "Leopard_Slash.h"
#include "Leopard_Meteo.h"
#include "Nail_Trail.h"
#include "Leopard_Afterimage_Assault.h"
#include "Leopard_Afterimage_Jump.h"
#include "Leopard_ClawAssault.h"
#include "Leopard_ClawJump.h"
#include "Leopard_Flash.h"
#include "Leopard_Floor.h"
#include "Leopard_Impact.h"
#include "Nail_Smoke.h"
#include "LeoJump_Smoke.h"
#include "Assault_Smoke.h"

// LevelDesign
#include "LevelDesign_Unsupported.h"
#include "LevelDesign_Starblock.h"
#include "LevelDesign_Rail.h"
#include "LD_AudioArea.h"
#include "LD_LensFlare.h"
#include "LD_Stage1BossDemo.h"
#include "LD_SlopeBoardA.h"
#include "LD_SlopeBoardB.h"
#include "LD_SlopeBoardC.h"
#include "LD_DeformCarBreakWall.h"
#include "LD_GarageRadio.h"
#include "LD_DeformObject.h"
#include "LD_CopyEssence.h"

// EnvObject
#include "EnvTrigger_Generic.h"
#include "EnvTrigger_RenderGlobals.h"
#include "EnvTrigger_EventPublisher.h"
#include "EnvVolume_Effect.h"
#include "EnvVolume_Culling.h"
#include "EnvVolume_Light.h"
#include "Env_SpotLight.h"

// Projectile
#include "Projectile_Boulder.h"
#include "EnemyBomb.h"
#include "KirbyBomb.h"
#include "Spit_Projectile.h"
#include "KoKabu.h"

// Ability Bubble
#include "EssenceBubble.h"
#include "DroppedBubble.h"

#include "Ability_Model.h"

//System Object
#include "KirbySpawnPoint.h"
#include "CameraDirector.h"
#include "Dialogue_Director.h"
#include "Dialogue_Arranger.h"

// CutSceneActor
#include "DialogueDee.h"

// NPC
#include "WaddleDee.h"
#include "WaddleDee_Body.h"

#include "DropStar.h"
#include "DropStar_Body.h"

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

namespace
{
    CModel* Create_TextureHubModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const _char* pModelFilePath,
        _bool bCookCollisionMesh, _fmatrix PreTransformMatrix = XMMatrixIdentity())
    {
        CModel::MODEL_LOAD_DESC Desc{};
        Desc.eType = eType;
        Desc.pModelFilePath = pModelFilePath;
        Desc.bCookCollisionMesh = bCookCollisionMesh;
        XMStoreFloat4x4(&Desc.PreTransformMatrix, PreTransformMatrix);

        return CModel::Create_WithTextureHub(pDevice, pContext, Desc);
    }

    void Register_BubbleShared(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iLevelIndex)
    {
        TRY_ADD_PROTO(pProxy, iLevelIndex, CAbility_Model::PROTOTYPE_TAG, CAbility_Model::Create(pDevice, pContext));
        
        // 능력 추가될 때마다 아래에 추가
        
        // Sword
        TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Ability_Model_Sword"),
            CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/Sword/Sword/Sword.ysh",
                XMMatrixRotationY(XMConvertToRadians(180.f))));

        // Bomb
        TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Ability_Model_Bomb"),
            CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Gimmick/CopyEssence/Bomb/KirbyBomb.ysh",
                XMMatrixRotationY(XMConvertToRadians(180.f))));

        // Ice
        TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Ability_Model_Ice"),
            CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Gimmick/CopyEssence/IceHat/Ice_Hat.ysh",
                XMMatrixRotationY(XMConvertToRadians(180.f))));
    }
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
    Register_UIContainer();
    Register_Camera();
    Register_Test();
    Register_MiniBoss();
    Register_MainBoss();
    Register_Container();
    Register_NonAnimObject();
    Register_AnimObject();
    Register_Effect();
    Register_BossEffect();
    Register_Bubble();


    Register_Cutscene();
    Register_SystemObject();
}

void CGameObject_Factory::Register_UI()
{
    Register(CUI_Image::PROTOTYPE_TAG, TEXT("UI_OBJECT"), CREATOR(CUI_Image), LOADER());
    Register(CUI_SpriteAnim::PROTOTYPE_TAG, TEXT("UI_OBJECT"), CREATOR(CUI_SpriteAnim), LOADER());
    Register(CUI_Text::PROTOTYPE_TAG, TEXT("UI_OBJECT"), CREATOR(CUI_Text), LOADER());
    Register(CUI_Effect::PROTOTYPE_TAG, TEXT("UI_OBJECT"), CREATOR(CUI_Effect), LOADER());
    Register(CUI_GaugeFill::PROTOTYPE_TAG, TEXT("UI_OBJECT"), CREATOR(CUI_GaugeFill), LOADER());
    Register(CUI_Curtain::PROTOTYPE_TAG, TEXT("UI_OBJECT"), CREATOR(CUI_Curtain), LOADER());
    Register(CUI_Eraser::PROTOTYPE_TAG, TEXT("UI_OBJECT"), CREATOR(CUI_Eraser), LOADER());
    Register(CUI_SpriteAnimCurtain::PROTOTYPE_TAG, TEXT("UI_OBJECT"), CREATOR(CUI_SpriteAnimCurtain), LOADER());
    Register(CUI_CurtainTexture::PROTOTYPE_TAG, TEXT("UI_OBJECT"), CREATOR(CUI_CurtainTexture), LOADER());
}

void CGameObject_Factory::Register_Camera()
{
    Register(TEXT("Proto_CameraFree"), TEXT("CAMERA_OBJECT"),
        CREATOR(CCamera_Free), LOADER());
}

void CGameObject_Factory::Register_Test()
{
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
            // Model
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

            // Kirby_DeformCylinder_Demo
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_DeformCylinder_Demo::PROTOTYPE_TAG,
                CKirby_DeformCylinder_Demo::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Kirby_DeformCylinder_Demo"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/DeformCylinder/Demo/Demo.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));

            // Kirby_DeformCylinder_Main
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_DeformCylinder_Main::PROTOTYPE_TAG,
                CKirby_DeformCylinder_Main::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Kirby_DeformCylinder_Main"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/DeformCylinder/Model/Main.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));

            // Ability
            // Sword
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_Sword::PROTOTYPE_TAG, CKirby_Sword::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Sword"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/Sword/Sword/Sword.ysh"));
            // Sword Hat
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_SwordHat::PROTOTYPE_TAG, CKirby_SwordHat::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SwordHat"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/Sword/Hat/SwordHat.ysh"));

            // Bomb Hat
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_BombHat::PROTOTYPE_TAG, CKirby_BombHat::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_BombHat"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/Bomb/Hat/BombHat.ysh"));
            // Bomb
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirbyBomb::PROTOTYPE_TAG, CKirbyBomb::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirbyBomb::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/AnimModel/KirbyBomb/KirbyBomb.ysh", 
                    XMMatrixRotationX(XMConvertToRadians(90.f)) * XMMatrixRotationY(XMConvertToRadians(180.f))));

            // Ice Hat
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_IceHat::PROTOTYPE_TAG,
                CKirby_IceHat::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_IceHat"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Gimmick/CopyEssence/IceHat/Ice_Hat.ysh"));

            //Kirby_Projectile
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSpit_Projectile::PROTOTYPE_TAG,
                CSpit_Projectile::Create(pDevice, pContext));
        )
    ); 

#pragma region 몬스터
    
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
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/BladeKnight/Body/BladeKnight.ysh",
                XMMatrixRotationY(XMConvertToRadians(180.f))));

            // BladeKnight Sword
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBladeKnight_Sword::PROTOTYPE_TAG, CBladeKnight_Sword::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_BladeKnight_Sword"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/BladeKnight/Sword/BladeKnight_Sword.ysh"));
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
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/NormalEnemy/Body/NormalEnemy.ysh",
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
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/Kabu/Body/Kabu.ysh",
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
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/BrontoBurt/Body/BrontoBurt.ysh",
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
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/PoppyBrosJr/Body/PoppyBrosJr.ysh",
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

    // 7. NormalEnemyWild
    Register
    (
        CNormalEnemyWild::PROTOTYPE_TAG, TEXT("Monster"),
        CREATOR(CNormalEnemyWild),
        LOADER
        (
            // NormalEnemy_Body
            TRY_ADD_PROTO(pProxy, iLevelIndex, CNormalEnemyWild_Body::PROTOTYPE_TAG, CNormalEnemyWild_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_NormalEnemyWild_Body"),
                    CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/NormalEnemyWild/Body/NormalEnemyWild.ysh",
                        XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );

    // 8. Dekabu 
    Register
    (
        CDekabu::PROTOTYPE_TAG, TEXT("Monster"),
        CREATOR(CDekabu),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CDekabu_Body::PROTOTYPE_TAG, CDekabu_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Dekabu_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/Dekabu/Body/Dekabu.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));

            // KoKabu(Projectile) - Model : Kabu 와 동일
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKokabu::PROTOTYPE_TAG, CKokabu::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CKokabu::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/Kabu/Body/Kabu.ysh",
                    XMMatrixScaling(1.f, 1.f, 1.f) * XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );

    // Bouncy
    Register
    (
        CBouncy::PROTOTYPE_TAG, TEXT("Monster"),
        CREATOR(CBouncy),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBouncy_Body::PROTOTYPE_TAG, CBouncy_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Bouncy_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/Bouncy/Body/Bouncy.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );

    // RabbitEnemy
    Register
    (
        CRabbitEnemy::PROTOTYPE_TAG, TEXT("Monster"),
        CREATOR(CRabbitEnemy),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRabbitEnemy_Body::PROTOTYPE_TAG, CRabbitEnemy_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_RabbitEnemy_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM,  "../../Resources/CHJ/Monster/RabbitEnemy/Body/RabbitEnemy.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );

#pragma endregion

    // NPC
    Register
    (
        CWaddleDee::PROTOTYPE_TAG, TEXT("NPC"),
        CREATOR(CWaddleDee),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CWaddleDee_Body::PROTOTYPE_TAG, CWaddleDee_Body::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CCage_WaddleDee::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/WaddleDee/Body/Model_Anim.ysh", XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );

    // Drop Star
    Register
    (
        CDropStar::PROTOTYPE_TAG, TEXT("ETC"),
        CREATOR(CDropStar),
        LOADER
        (
                TRY_ADD_PROTO(pProxy, iLevelIndex, CDropStar_Body::PROTOTYPE_TAG, CDropStar_Body::Create(pDevice, pContext));
   
                TRY_ADD_PROTO(pProxy, iLevelIndex,  CDropStar_Body::MODEL_PROTO_TAG,
                                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/DropStar/DropStar.ysh",
                                    XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );
}

void CGameObject_Factory::Register_UIContainer()
{
    Register(CUI_Title::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_Title), LOADER());
    Register(CUI_GenericContainer::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_GenericContainer), LOADER());
    Register(CUI_PointStar::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_PointStar), LOADER());
    Register(CUI_KirbyStatus::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_KirbyStatus), LOADER());
    Register(CUI_FadeOut::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_FadeOut), LOADER());
    Register(CUI_LoadingCurtain::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_LoadingCurtain), LOADER());
    Register(CUI_FadeIn::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_FadeIn), LOADER());
    Register(CUI_BossStatus::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_BossStatus), LOADER());
    Register(CUI_FlashCurtain::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_FlashCurtain), LOADER());
    Register(CUIMovableContainer::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUIMovableContainer), LOADER());
    Register(CUI_StageClear::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_StageClear), LOADER());
    Register(CUICoordinatorContainer::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUICoordinatorContainer), LOADER());
    Register(CUI_MissionBoard::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_MissionBoard), LOADER());
    Register(CUI_MissionPanel::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_MissionPanel), LOADER());
    Register(CUI_LetterBox::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_LetterBox), LOADER());
    Register(CUI_TitleLogo::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_TitleLogo), LOADER());
    Register(CUI_CutFade::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_CutFade), LOADER());
    Register(CUI_Dialogue::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_Dialogue), LOADER());
    Register(CUI_QTE::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_QTE), LOADER());
}

void CGameObject_Factory::Register_NonAnimObject()
{
    Register(CEnvTrigger_Generic::PROTOTYPE_TAG, TEXT("ENV_TRIGGER"), CREATOR(CEnvTrigger_Generic), LOADER());
    Register(CEnvTrigger_RenderGlobals::PROTOTYPE_TAG, TEXT("ENV_TRIGGER"), CREATOR(CEnvTrigger_RenderGlobals), LOADER());
    Register(CEnvTrigger_EventPublisher::PROTOTYPE_TAG, TEXT("ENV_TRIGGER"), CREATOR(CEnvTrigger_EventPublisher), LOADER());
    Register(CEnvVolume_Effect::PROTOTYPE_TAG, TEXT("ENV_TRIGGER"), CREATOR(CEnvVolume_Effect), LOADER());
    Register(CEnvVolume_Culling::PROTOTYPE_TAG, TEXT("ENV_TRIGGER"), CREATOR(CEnvVolume_Culling), LOADER());
    Register(CEnvVolume_Light::PROTOTYPE_TAG, TEXT("ENV_TRIGGER"), CREATOR(CEnvVolume_Light), LOADER());
    Register(CEnv_SpotLight::PROTOTYPE_TAG, TEXT("ENV_TRIGGER"), CREATOR(CEnv_SpotLight), LOADER());

    Register(CLevelDesign_Unsupported::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLevelDesign_Unsupported), LOADER());
    Register(CLevelDesign_Rail::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLevelDesign_Rail), LOADER());
    Register(CLD_AudioArea::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLD_AudioArea), LOADER());
    Register(CLD_LensFlare::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLD_LensFlare), LOADER());
    Register(CLevelDesign_Starblock::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLevelDesign_Starblock),
        LOADER(TRY_ADD_PROTO(pProxy, iLevelIndex, CLevelDesign_Starblock::STARBLOCK_MODEL_PROTO_TAG,
            CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Gimmick/NonAnim/Star/H1W1.ysh"));));
}

void CGameObject_Factory::Register_AnimObject()
{
    const ResourceLoader DeformObjectLoader = LOADER(
        TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Proto_Component_Model_DeformCar"),
            Create_TextureHubModel(pDevice, pContext, MODEL::ANIM, "../../Resources/Map/Gimmick/Anim/DeformCar/DeformCar.ysh",
                true, XMMatrixRotationY(XMConvertToRadians(180.f))));
        TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Proto_Component_Model_DeformCoaster"),
            Create_TextureHubModel(pDevice, pContext, MODEL::ANIM, "../../Resources/Map/Gimmick/Anim/DeformCoaster/DeformCoaster.ysh",
                true, XMMatrixRotationY(XMConvertToRadians(180.f))));
        TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Proto_Component_Model_DeformCylinder"),
            Create_TextureHubModel(pDevice, pContext, MODEL::ANIM, "../../Resources/Map/Gimmick/Anim/DeformCylinder/DeformCylinder.ysh",
                true, XMMatrixRotationY(XMConvertToRadians(180.f))));
            );

    Register(CLD_DeformObject::PROTOTYPE_TAG_CAR, TEXT("DEFORM_OBJECT"),
        [](ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
        {
            return dynamic_cast<CBase*>(CLD_DeformObject::Create_ByType(pDevice, pContext, DEFORM_TYPE::CAR));
        },
        DeformObjectLoader);

    Register(CLD_DeformObject::PROTOTYPE_TAG_COASTER, TEXT("DEFORM_OBJECT"),
        [](ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
        {
            return dynamic_cast<CBase*>(CLD_DeformObject::Create_ByType(pDevice, pContext, DEFORM_TYPE::COASTER));
        },
        DeformObjectLoader);

    Register(CLD_DeformObject::PROTOTYPE_TAG_CYLINDER, TEXT("DEFORM_OBJECT"),
        [](ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
        {
            return dynamic_cast<CBase*>(CLD_DeformObject::Create_ByType(pDevice, pContext, DEFORM_TYPE::CYLINDER));
        },
        DeformObjectLoader);

    Register(CLD_Stage1BossDemo::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLD_Stage1BossDemo),
        LOADER(TRY_ADD_PROTO(pProxy, iLevelIndex, CLD_Stage1BossDemo::MODEL_PROTO_TAG,
            Create_TextureHubModel(pDevice, pContext, MODEL::ANIM, "../../Resources/Map/Gimmick/Anim/Level1BossDemoBg/Level1BossDemoBg.ysh", true));));

    Register(CLD_SlopeBoardA::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLD_SlopeBoardA),
        LOADER(TRY_ADD_PROTO(pProxy, iLevelIndex, CLD_SlopeBoardA::MODEL_PROTO_TAG,
            Create_TextureHubModel(pDevice, pContext, MODEL::ANIM, "../../Resources/Map/Gimmick/Anim/SlopeBoard/SlopeBoardA.ysh", false));));

    Register(CLD_SlopeBoardB::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLD_SlopeBoardB),
        LOADER(TRY_ADD_PROTO(pProxy, iLevelIndex, CLD_SlopeBoardB::MODEL_PROTO_TAG,
            Create_TextureHubModel(pDevice, pContext, MODEL::ANIM, "../../Resources/Map/Gimmick/Anim/SlopeBoard/SlopeBoardB.ysh", false));));

    Register(CLD_SlopeBoardC::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLD_SlopeBoardC),
        LOADER(TRY_ADD_PROTO(pProxy, iLevelIndex, CLD_SlopeBoardC::MODEL_PROTO_TAG,
            Create_TextureHubModel(pDevice, pContext, MODEL::ANIM, "../../Resources/Map/Gimmick/Anim/SlopeBoard/SlopeBoardC.ysh", true));));

    Register(CLD_DeformCarBreakWall::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLD_DeformCarBreakWall),
        LOADER(TRY_ADD_PROTO(pProxy, iLevelIndex, CLD_DeformCarBreakWall::MODEL_PROTO_TAG,
            Create_TextureHubModel(pDevice, pContext, MODEL::ANIM, "../../Resources/Map/Gimmick/Anim/DeformCarBreakWall/DeformCarBreakWall.ysh", false));));

    Register(CLD_GarageRadio::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLD_GarageRadio),
        LOADER(TRY_ADD_PROTO(pProxy, iLevelIndex, CLD_GarageRadio::MODEL_PROTO_TAG,
            Create_TextureHubModel(pDevice, pContext, MODEL::ANIM, "../../Resources/Map/Gimmick/Anim/GarageRadio/GarageRadio.ysh", false));));

    Register(CLD_DeformObject::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLD_DeformObject),
        LOADER(TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Proto_Component_Model_DeformCar"),
            Create_TextureHubModel(pDevice, pContext, MODEL::ANIM, "../../Resources/Map/Gimmick/Anim/DeformCar/DeformCar.ysh", true, XMMatrixRotationY(XMConvertToRadians(180.f))));));

    Register(CLD_CopyEssence::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLD_CopyEssence),
        LOADER(TRY_ADD_PROTO(pProxy, iLevelIndex, CLD_CopyEssence::MODEL_PROTO_TAG,
            Create_TextureHubModel(pDevice, pContext, MODEL::ANIM, "../../Resources/Map/Gimmick/Anim/DeformCarBreakWall/DeformCarBreakWall.ysh", true));));
}

void CGameObject_Factory::Register_Effect()
{
    // BoostGas
    Register(CBoostGas::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CBoostGas),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeEmitter::PROTOTYPE_TAG, CSmokeEmitter::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );
    
    // CarMilkyWay
    Register(CCarMilkyWay::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CCarMilkyWay),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CCar_00_MilkyWay::PROTOTYPE_TAG, CCar_00_MilkyWay::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Car_00_MilkyWay"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSE/Effect/CarMilkyWay/Car_00_MilkyWay.ysh"));
            TRY_ADD_PROTO(pProxy, Texture_MilkyWayMask.iLevelID, Texture_MilkyWayMask.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_MilkyWayMask.szFileTag, Texture_MilkyWayMask.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_Gradiant.iLevelID, Texture_Gradiant.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Gradiant.szFileTag, Texture_Gradiant.iNumTex));
        )
    );

    // GetAbilityEffect
    Register(CGetAbilityEffect::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CGetAbilityEffect),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CStar2DParticle::PROTOTYPE_TAG, CStar2DParticle::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_Star2D.iLevelID, Texture_Star2D.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Star2D.szFileTag, Texture_Star2D.iNumTex));
        )
    );

    // SwordChargeEffect
    Register(CSwordChargeEffect::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSwordChargeEffect),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwordCharge::PROTOTYPE_TAG, CSwordCharge::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SwordCharge"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSE/Effect/SwordCharge/Model_Common_Ring03.ysh"));
            TRY_ADD_PROTO(pProxy, Texture_ChargeNoise.iLevelID, Texture_ChargeNoise.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_ChargeNoise.szFileTag, Texture_ChargeNoise.iNumTex));
        )
    );

    Register(CDespawnEffect::PROTOTYPE_TAG, TEXT("Effect_Container"), 
        CREATOR(CDespawnEffect),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeParticle::PROTOTYPE_TAG,
                CSmokeParticle::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/CHJ/Effect/SmokeMesh/Model_SmokeSphereOriginal.ysh"));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CStarParticle::PROTOTYPE_TAG,
                CStarParticle::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_StarSmooth"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/CHJ/Effect/Star/Common_00_Common_StarSmooth2.ysh"));
         
        ));

    // 3
    Register(CBombExplosion::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CBombExplosion),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeParticle::PROTOTYPE_TAG,
                CSmokeParticle::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/CHJ/Effect/SmokeMesh/Model_SmokeSphereOriginal.ysh"));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CSphereParticle::PROTOTYPE_TAG,
                CSphereParticle::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_CommonSphere"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/CHJ/Effect/Bomb/Sphere/Common_00_Common_Sphere02.ysh"));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CCommon_SphereNoise::PROTOTYPE_TAG,
                CCommon_SphereNoise::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SphereNoise"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/CHJ/Effect/Bomb/SphereNoise/Common_00_Common_SphereNoise.ysh"));
        ));

    // 4 
    Register(CCommonHit::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CCommonHit),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CStarParticle::PROTOTYPE_TAG, CStarParticle::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_StarSmooth"),
                            CModel::Create(pDevice, pContext, MODEL::NONANIM,  "../../Resources/CHJ/Effect/Star/Common_00_Common_StarSmooth.ysh"));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CHitMark::PROTOTYPE_TAG, CHitMark::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_CommonHit01.iLevelID, Texture_CommonHit01.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_CommonHit01.szFileTag,
                    Texture_CommonHit01.iNumTex));
        ));

    // 5 

    Register(CSpitObject::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CSpitObject),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBubble::PROTOTYPE_TAG, CBubble::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, Texture_CommonRing01.iLevelID, Texture_CommonRing01.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_CommonRing01.szFileTag,
                Texture_CommonRing01.iNumTex));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CStarEmitter::PROTOTYPE_TAG, CStarEmitter::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_StarSmooth"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/CHJ/Effect/Star/Common_00_Common_StarSmooth.ysh"));


        ));

    // 6 
    Register(CBombFuseEffect::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CBombFuseEffect),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSparkle::PROTOTYPE_TAG, CSparkle::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, Texture_CommonSparkle02.iLevelID, Texture_CommonSparkle02.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_CommonSparkle02.szFileTag,
                    Texture_CommonSparkle02.iNumTex));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeEmitter::PROTOTYPE_TAG, CSmokeEmitter::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeMesh"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/CHJ/Effect/SmokeMesh/Model_SmokeSphereOriginal.ysh"));

        ));

    // 7 Bubble Aura (EssenceBubble / DorppedBubble 공용)
    Register(CBubbleAura::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CBubbleAura),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectCommon::PROTOTYPE_TAG, CRectCommon::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectEmitterCommon::PROTOTYPE_TAG, CRectEmitterCommon::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectParticleCommon::PROTOTYPE_TAG, CRectParticleCommon::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, Texture_CommonRing01.iLevelID, Texture_CommonRing01.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_CommonRing01.szFileTag, Texture_CommonRing01.iNumTex));
            
            TRY_ADD_PROTO(pProxy, Texture_Star2D.iLevelID, Texture_Star2D.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Star2D.szFileTag, Texture_Star2D.iNumTex));

            TRY_ADD_PROTO(pProxy, Texture_CommonSparkle01.iLevelID, Texture_CommonSparkle01.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_CommonSparkle01.szFileTag, Texture_CommonSparkle01.iNumTex));

        ));

    Register(CLaunchSmoke::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CLaunchSmoke),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshEmitterCommon::PROTOTYPE_TAG,
                CMeshEmitterCommon::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeMesh"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/CHJ/Effect/SmokeMesh/Model_SmokeSphereOriginal.ysh"));
        ));

    Register(CPickUpEffect::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CPickUpEffect),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectCommon::PROTOTYPE_TAG, CRectCommon::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectParticleCommon::PROTOTYPE_TAG, CRectParticleCommon::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, Texture_CommonRing01.iLevelID, Texture_CommonRing01.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_CommonRing01.szFileTag, Texture_CommonRing01.iNumTex));

            TRY_ADD_PROTO(pProxy, Texture_CommonSparkle01.iLevelID, Texture_CommonSparkle01.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_CommonSparkle01.szFileTag, Texture_CommonSparkle01.iNumTex));

        ));

    Register(CDropStarEffect::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CDropStarEffect),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectCommon::PROTOTYPE_TAG, CRectCommon::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, Texture_CommonRing01.iLevelID, Texture_CommonRing01.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_CommonRing01.szFileTag, Texture_CommonRing01.iNumTex));

        ));

    // CKirby_SwordTrail
    Register(CKirby_SwordTrail::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CKirby_SwordTrail),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CTrailCommon::PROTOTYPE_TAG, CTrailCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_SwordTrail.iLevelID, Texture_SwordTrail.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_SwordTrail.szFileTag, Texture_SwordTrail.iNumTex));
        ));

    // SmokeCollection
    Register(CSmokeCollection::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CSmokeCollection),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG,
                CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeLowPoly::PROTOTYPE_TAG,
                CSmokeLowPoly::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeLowPoly"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/Test/Effect/SmokeLowPoly/Model_SmokeLowPoly.ysh"));
        ));

    // LandingSmoke
    Register(CLandingSmoke::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CLandingSmoke),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG,
                CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeMesh"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/CHJ/Effect/SmokeMesh/Model_SmokeSphereOriginal.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeLowPolyMesh"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/CHJ/Effect/SmokeLowPoly/Model_SmokeLowPoly.ysh"));
        ));

    Register(CMoveSmoke::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CMoveSmoke),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshEmitterCommon::PROTOTYPE_TAG,
                CMeshEmitterCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeMesh"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/CHJ/Effect/SmokeMesh/Model_SmokeSphereOriginal.ysh"));
        ));

    // CarLanding
    Register(CCarLanding::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CCarLanding),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshParticleCommon::PROTOTYPE_TAG,
                CMeshParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeLowPoly"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/Test/Effect/SmokeLowPoly/Model_SmokeLowPoly.ysh"));
        ));

    // SmokeSphereOriginalEmitter
    Register(CSmokeSphereOriginalEmitter::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CSmokeSphereOriginalEmitter),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshEmitterCommon::PROTOTYPE_TAG,
                CMeshEmitterCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        ));

    // 8
    Register(CFlowerPetals::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CFlowerPetals),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshEmitterCommon::PROTOTYPE_TAG, CMeshEmitterCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Flower"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Flower/Flower_00_TopL.ysh"));
        )
    );

    // 9
    Register(CSplit_Starblock::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSplit_Starblock),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshEmitterCommon::PROTOTYPE_TAG, CMeshEmitterCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Starblock_Piece01"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Starblock/BlockBombH2W2Piece.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Starblock_Piece02"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Starblock/BlockStarH2W2Piece.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Stone"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Stone/Stone.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_StoneDust"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Stone/StoneDust.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );

    // 10
    Register(CSplit_Stone::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSplit_Stone),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshEmitterCommon::PROTOTYPE_TAG, CMeshEmitterCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Nature_Piece01"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/LandBreak/Nature_Piece01.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Nature_Piece02"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/LandBreak/Nature_Piece02.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Stone"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Stone/Stone.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_StoneDust"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Stone/StoneDust.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_StoneHiMesh"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Stone/StoneHiMesh.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );

    // 11
    Register(CSplit_Bush::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSplit_Bush),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshEmitterCommon::PROTOTYPE_TAG, CMeshEmitterCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_BushLeafL"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Bush/BasicL.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_BushLeafM"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Bush/BasicM.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_BushLeafS"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Bush/BasicS.ysh"));
        )
    );

#pragma region 몬스터 이펙트
    
    // BladeKnight
    Register(CSwordTrail_BK::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CSwordTrail_BK),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CTrailCommon::PROTOTYPE_TAG, CTrailCommon::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, Texture_BK_CommonSlash.iLevelID, Texture_BK_CommonSlash.szProtoTag,
                            CTexture::Create(pDevice, pContext, Texture_BK_CommonSlash.szFileTag, Texture_BK_CommonSlash.iNumTex));
        ));

    Register(CTornado_BK::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CTornado_BK),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex,
                TEXT("Prototype_Component_Model_BK_TornadoRing_Spin01"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/CHJ/Effect/Monster/BladeKnight/Mesh/Spin01/Common_Ring03.ysh"));

            TRY_ADD_PROTO(pProxy, iLevelIndex,
                TEXT("Prototype_Component_Model_BK_TornadoRing_Spin02"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/CHJ/Effect/Monster/BladeKnight/Mesh/Spin02/Common_Ring03.ysh"));
        ));


#pragma endregion

    // 12
    Register(CBreakWallEffect::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CBreakWallEffect),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshEmitterCommon::PROTOTYPE_TAG, CMeshEmitterCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_StoneDust"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Stone/StoneDust.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );

    // 13
    Register(CItemEffect::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CItemEffect),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectCommon::PROTOTYPE_TAG, CRectCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectEmitterCommon::PROTOTYPE_TAG, CRectEmitterCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_ItemCircle.iLevelID, Texture_ItemCircle.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_ItemCircle.szFileTag, Texture_ItemCircle.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_ItemSparkle01.iLevelID, Texture_ItemSparkle01.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_ItemSparkle01.szFileTag, Texture_ItemSparkle01.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_ItemSparkle02.iLevelID, Texture_ItemSparkle02.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_ItemSparkle02.szFileTag, Texture_ItemSparkle02.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_ItemSparkle03.iLevelID, Texture_ItemSparkle03.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_ItemSparkle03.szFileTag, Texture_ItemSparkle03.iNumTex));
        ));

    Register(CBombHitAim::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CBombHitAim),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectCommon::PROTOTYPE_TAG,
                CRectCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_BombHitAim.iLevelID, Texture_BombHitAim.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_BombHitAim.szFileTag,
                    Texture_BombHitAim.iNumTex));
        ));
    Register(CBombAimDot::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CBombAimDot),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectCommon::PROTOTYPE_TAG,
                CRectCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_BombAimDot.iLevelID, Texture_BombAimDot.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_BombAimDot.szFileTag,
                    Texture_BombAimDot.iNumTex));
        ));

    // 14
    Register(CSplit_Coaster::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSplit_Coaster),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshEmitterCommon::PROTOTYPE_TAG, CMeshEmitterCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Coaster_Bar"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/CoasterBreak/Coaster_Bar.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Coaster_Jet"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/CoasterBreak/Coaster_Jet.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Coaster_Tip01L"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/CoasterBreak/Coaster_Tip01L.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Coaster_Tip02L"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/CoasterBreak/Coaster_Tip02L.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Coaster_Tire"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/CoasterBreak/Coaster_Tire.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Coaster_WingA"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/CoasterBreak/Coaster_WingA.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Coaster_WingB"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/CoasterBreak/Coaster_WingB.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );

    // 15
    Register(CSplit_Cylinder::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSplit_Cylinder),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshEmitterCommon::PROTOTYPE_TAG, CMeshEmitterCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Cylinder_DrainM"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/CylinderBreak/DrainM.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Cylinder_PieceM"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/CylinderBreak/PieceM.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );

    // 16
    Register(CLensFlare::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CLensFlare),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectEmitterCommon::PROTOTYPE_TAG, CRectEmitterCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshEmitterCommon::PROTOTYPE_TAG, CMeshEmitterCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_LensFlare_Common_Circle01"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/LensFlare/Common_Circle01.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_LensFlare_Common_Ring01"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/LensFlare/Common_Ring01.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Texture_LensFlare_CircleGlow2"), CTexture::Create(pDevice, pContext, TEXT("../../Resources/Map/Effect/LensFlare/circleglow2.png"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Texture_LensFlare_Common_Circle06"), CTexture::Create(pDevice, pContext, TEXT("../../Resources/Map/Effect/LensFlare/common_circle06.png"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Texture_LensFlare_CircleGradation"), CTexture::Create(pDevice, pContext, TEXT("../../Resources/Map/Effect/LensFlare/circlegradation.png"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Texture_LensFlare_ThunderRoot2"), CTexture::Create(pDevice, pContext, TEXT("../../Resources/Map/Effect/LensFlare/thunderroot2.png"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Texture_LensFlare_Common_Ring08"), CTexture::Create(pDevice, pContext, TEXT("../../Resources/Map/Effect/LensFlare/common_ring08.png"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Texture_LensFlare_Common_Circle11"), CTexture::Create(pDevice, pContext, TEXT("../../Resources/Map/Effect/LensFlare/common_circle11.png"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Texture_LensFlare_Common_Circle01"), CTexture::Create(pDevice, pContext, TEXT("../../Resources/Map/Effect/LensFlare/common_circle01.png"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Texture_LensFlare_Common_Circle02"), CTexture::Create(pDevice, pContext, TEXT("../../Resources/Map/Effect/LensFlare/common_circle02.png"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Texture_LensFlare_Common_Circle04"), CTexture::Create(pDevice, pContext, TEXT("../../Resources/Map/Effect/LensFlare/common_circle04.png"), 1));
        ));

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
    
    Register(CEssenceAura::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CEssenceAura),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CEssenceCrown::PROTOTYPE_TAG, CEssenceCrown::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_EssenceCrown"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/CHJ/Effect/CopyEssence/Ring01/Common_Ring03.ysh"));
        )
    );
}

void CGameObject_Factory::Register_BossEffect()
{
    Gorilla_Effect();
    Armadillo_Effect();
    Leopard_Effect();
}

void CGameObject_Factory::Register_Bubble()
{
    //Ability Bubble - DroppedBubble
    Register
    (
        CDroppedBubble::PROTOTYPE_TAG, TEXT("ETC"),
        CREATOR(CDroppedBubble),
        LOADER
        (
            Register_BubbleShared(pProxy, pDevice, pContext, iLevelIndex);
        )
    );

    //Ability Bubble - EssenceBubble
    Register
    (
        CEssenceBubble::PROTOTYPE_TAG, TEXT("ETC"),
        CREATOR(CEssenceBubble),
        LOADER
        (
            Register_BubbleShared(pProxy, pDevice, pContext, iLevelIndex);
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

            TRY_ADD_PROTO(pProxy, iLevelIndex, CCage_WaddleDee::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/WaddleDee/Body/Model_Anim.ysh"
                    , XMMatrixRotationY(XMConvertToRadians(180.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CCage_WaddleDee::PROTOTYPE_TAG, CCage_WaddleDee::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Cage_Body::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/Gimmick/CaptiveCage/CageL/CageL_Anim_TopL.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Cage_Body::PROTOTYPE_TAG, CBoss_Cage_Body::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Cage::PROTOTYPE_TAG, CBoss_Cage::Create(pDevice, pContext));
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

    Register(CBoss_Cage::PROTOTYPE_TAG, TEXT("MainBoss"),
        CREATOR(CBoss_Cage),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CCage_WaddleDee::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/WaddleDee/Body/Model_Anim.ysh"
                    , XMMatrixRotationY(XMConvertToRadians(180.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CCage_WaddleDee::PROTOTYPE_TAG, CCage_WaddleDee::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Cage_Body::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/Gimmick/CaptiveCage/CageL/CageL_Anim_TopL.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Cage_Body::PROTOTYPE_TAG, CBoss_Cage_Body::Create(pDevice, pContext));
        )
    );

    Register(CBoss_Armadillo::PROTOTYPE_TAG, TEXT("MainBoss"),
        CREATOR(CBoss_Armadillo),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Armadillo_Body::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/Boss/Armadillo/body/Body.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Armadillo_Body::PROTOTYPE_TAG, CBoss_Armadillo_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CProjectile_Partner::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/Boss/Armadillo/Partner/PartnerModel_Anim.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CProjectile_Partner::PROTOTYPE_TAG,
                CProjectile_Partner::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Armadillo_Cage::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/Boss/Armadillo/Cage/Cage_Anim.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Armadillo_Cage::PROTOTYPE_TAG,
                CBoss_Armadillo_Cage::Create(pDevice, pContext));
        )
    );

    Register(CBoss_Leopard::PROTOTYPE_TAG, TEXT("MainBoss"),
        CREATOR(CBoss_Leopard),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Leopard_Body::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/Boss/Leopard/Body/Model_Anim.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Leopard_Body::PROTOTYPE_TAG, CBoss_Leopard_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CProjectile_Nail::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Leopard/Nail/Nail.ysh"
                    , XMMatrixTranslation(0.f, 0.f, -0.8f)));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CProjectile_Nail::PROTOTYPE_TAG,
                CProjectile_Nail::Create(pDevice, pContext));
        )
    );

    Register(CBoss_GorillaRush::PROTOTYPE_TAG, TEXT("MainBoss"),
        CREATOR(CBoss_GorillaRush),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Boss_Gorilla_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/Boss/Gorilla/Body/Gorilla.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Gorilla_Body::PROTOTYPE_TAG,
                CBoss_Gorilla_Body::Create(pDevice, pContext));
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

    Register(CDialogueDee::PROTOTYPE_TAG, TEXT("Dialogue"),
        CREATOR(CDialogueDee),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CCage_WaddleDee::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM,
                    "../../Resources/YSH/WaddleDee/Body/Model_Anim.ysh"
                    , XMMatrixRotationY(XMConvertToRadians(180.f))));
        ));
}

void CGameObject_Factory::Register_SystemObject()
{
    Register(CKirbySpawnPoint::PROTOTYPE_TAG, TEXT("System_Object"), CREATOR(CKirbySpawnPoint), LOADER());
    Register(CCameraDirector::PROTOTYPE_TAG, TEXT("System_Object"), CREATOR(CCameraDirector), LOADER());
    Register(CDialogue_Director::PROTOTYPE_TAG, TEXT("System_Object"), CREATOR(CDialogue_Director), LOADER());
    Register(CDialogue_Arranger::PROTOTYPE_TAG, TEXT("System_Object"), CREATOR(CDialogue_Arranger), LOADER());
    Register(CSkySphere::PROTOTYPE_TAG, TEXT("TEST_OBJECT"), CREATOR(CSkySphere), LOADER());
}

void CGameObject_Factory::Gorilla_Effect()
{
    Register(CRockBurst::PROTOTYPE_TAG, TEXT("00.Gorilla_Effect"), CREATOR(CRockBurst),
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

    Register(CDeathSmoke::PROTOTYPE_TAG, TEXT("00.Gorilla_Effect"), CREATOR(CDeathSmoke),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeParticle::PROTOTYPE_TAG,
                CSmokeParticle::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );

    Register(CRockBounce::PROTOTYPE_TAG, TEXT("00.Gorilla_Effect"), CREATOR(CRockBounce),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeParticle::PROTOTYPE_TAG,
                CSmokeParticle::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );

    Register(CRockPull::PROTOTYPE_TAG, TEXT("00.Gorilla_Effect"), CREATOR(CRockPull),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeParticle::PROTOTYPE_TAG,
                CSmokeParticle::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );

    Register(CRockPush::PROTOTYPE_TAG, TEXT("00.Gorilla_Effect"), CREATOR(CRockPush),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSmokeParticle::PROTOTYPE_TAG,
                CSmokeParticle::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );

    Register(CGorilla_Swing::PROTOTYPE_TAG, TEXT("00.Gorilla_Effect"), CREATOR(CGorilla_Swing),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwing_Smoke::PROTOTYPE_TAG,
                CSwing_Smoke::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CSpinWind::PROTOTYPE_TAG,
                CSpinWind::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSpinWind::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSH/Boss/Gorilla/ArmSpinWind/BossGorilla_00_TornadoPieceMedium.ysh"));
        )
    );

    Register(CGorilla_Ring::PROTOTYPE_TAG, TEXT("00.Gorilla_Effect"), CREATOR(CGorilla_Ring),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CShockwave::PROTOTYPE_TAG,
                CShockwave::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CShockwave::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Gorilla/Shockwave/BossGorilla_00_Donut.ysh"
                    , XMMatrixScaling(10.f, 10.f, 10.f)));
        )
    );
}

void CGameObject_Factory::Armadillo_Effect()
{
    Register(CArmadillo_RutA::PROTOTYPE_TAG, TEXT("00.Armadillo_Effect"), CREATOR(CArmadillo_RutA),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CArmadillo_RutA::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Armadillo/Floor/EffectModel_RollingRutA.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );
    Register(CArmadillo_RutB::PROTOTYPE_TAG, TEXT("00.Armadillo_Effect"), CREATOR(CArmadillo_RutB),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CArmadillo_RutB::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Armadillo/Floor/EffectModel_RollingRutB.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );
    Register(CArmadillo_Dust::PROTOTYPE_TAG, TEXT("00.Armadillo_Effect"), CREATOR(CArmadillo_Dust),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectParticleCommon::PROTOTYPE_TAG, CRectParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CArmadillo_Dust::TEX_PROTOTAG,
                CTexture::Create(pDevice, pContext, L"../../Resources/YSH/Boss/Armadillo/Floor/smoke02.png", 1));
        )
    );
    Register(CArmadillo_RollWind::PROTOTYPE_TAG, TEXT("00.Armadillo_Effect"), CREATOR(CArmadillo_RollWind),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CArmadillo_RollWind::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Armadillo/Effect/Rolling/BossArmadillo_00_Common_SphereRolling.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );
    Register(CArmadillo_SpinWind::PROTOTYPE_TAG, TEXT("00.Armadillo_Effect"), CREATOR(CArmadillo_SpinWind),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSpinWind::PROTOTYPE_TAG,
                CSpinWind::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSpinWind::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSH/Boss/Gorilla/ArmSpinWind/BossGorilla_00_TornadoPieceMedium.ysh"));
        )
    );
    Register(CArmadillo_WallImpact::PROTOTYPE_TAG, TEXT("00.Armadillo_Effect"), CREATOR(CArmadillo_WallImpact),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwordCharge::PROTOTYPE_TAG, CSwordCharge::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SwordCharge"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSE/Effect/SwordCharge/Model_Common_Ring03.ysh"));
            TRY_ADD_PROTO(pProxy, Texture_ChargeNoise.iLevelID, Texture_ChargeNoise.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_ChargeNoise.szFileTag, Texture_ChargeNoise.iNumTex));
        )
    );
}

void CGameObject_Factory::Leopard_Effect()
{
    Register(CLeopard_Slash::PROTOTYPE_TAG, TEXT("00.Leopard_Effect"), CREATOR(CLeopard_Slash),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CLeopard_Slash::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Leopard/Effect/Ring/BossLeopard_00_Common_Ring02.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, Texture_LeoSlash.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_LeoSlash.szFileTag, Texture_LeoSlash.iNumTex));
        )
    );
    Register(CLeopard_Meteo::PROTOTYPE_TAG, TEXT("00.Leopard_Effect"), CREATOR(CLeopard_Meteo),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CLeopard_Meteo::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Leopard/Effect/Meteo/BossLeopard_00_Common_MeteoSpere.ysh"));
        )
    );
    Register(CNail_Trail::PROTOTYPE_TAG, TEXT("00.Leopard_Effect"), CREATOR(CNail_Trail), 
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CTrailCommon::PROTOTYPE_TAG, CTrailCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_NailTrail.iLevelID, Texture_NailTrail.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_NailTrail.szFileTag, Texture_NailTrail.iNumTex));
        )
    );
    Register(CLeopard_Afterimage_Assault::PROTOTYPE_TAG, TEXT("00.Leopard_Effect"), CREATOR(CLeopard_Afterimage_Assault),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CLeopard_Afterimage_Assault::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Leopard/Effect/Afterimage/BossLeopard_00_AssaultSlashEffectModel.ysh"
            , XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );
    Register(CLeopard_Afterimage_Jump::PROTOTYPE_TAG, TEXT("00.Leopard_Effect"), CREATOR(CLeopard_Afterimage_Jump),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CLeopard_Afterimage_Jump::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Leopard/Effect/Afterimage/BossLeopard_00_JumpAttackLeopardModel.ysh"
            , XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );
    Register(CLeopard_ClawAssault::PROTOTYPE_TAG, TEXT("00.Leopard_Effect"), CREATOR(CLeopard_ClawAssault),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CLeopard_ClawAssault::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Leopard/Effect/Claw/BossLeopard_00_AssultSlashClawModel.ysh"
                    , XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );
    Register(CLeopard_ClawJump::PROTOTYPE_TAG, TEXT("00.Leopard_Effect"), CREATOR(CLeopard_ClawJump),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CLeopard_ClawJump::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Leopard/Effect/Claw/BossLeopard_00_JumpAttackClawModel.ysh"
                    , XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );
    Register(CLeopard_Flash::PROTOTYPE_TAG, TEXT("00.Leopard_Effect"), CREATOR(CLeopard_Flash),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CLeopard_Flash::MODEL_PROTO_TAG_RING,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Leopard/Effect/Ring/BossLeopard_00_Common_Ring02.ysh",
                    XMMatrixRotationX(XMConvertToRadians(90.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CLeopard_Flash::MODEL_PROTO_TAG_CIRCLE,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Leopard/Effect/Circle/BossLeopard_00_Common_Circle01.ysh"));
        )
    );
    Register(CLeopard_Floor::PROTOTYPE_TAG, TEXT("00.Leopard_Effect"), CREATOR(CLeopard_Floor),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshParticleCommon::PROTOTYPE_TAG, CMeshParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CLeopard_Floor::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Leopard/Effect/Floor/GroundRockEffectModel_TopL.ysh"));
        )
    );
    Register(CLeopard_Impact::PROTOTYPE_TAG, TEXT("00.Leopard_Effect"), CREATOR(CLeopard_Impact),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CLeopard_Impact::MODEL_PROTO_TAG_RING,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Leopard/Effect/Ring/BossLeopard_00_Common_Ring02.ysh",
                    XMMatrixRotationX(XMConvertToRadians(90.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CLeopard_Impact::MODEL_PROTO_TAG_CIRCLE,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Leopard/Effect/Circle/BossLeopard_00_Common_Circle01.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, Texture_ImpactRing.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_ImpactRing.szFileTag, Texture_ImpactRing.iNumTex));
            TRY_ADD_PROTO(pProxy, iLevelIndex, Texture_ImpactCircle.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_ImpactCircle.szFileTag, Texture_ImpactCircle.iNumTex));
        )
    );

    Register(CNail_Smoke::PROTOTYPE_TAG, TEXT("00.Leopard_Effect"), CREATOR(CNail_Smoke),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshParticleCommon::PROTOTYPE_TAG, CMeshParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CNail_Smoke::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Leopard/Effect/Smoke/BossLeopard_00_SmokeSphereOriginal.ysh"));
        )
    );
    Register(CLeoJump_Smoke::PROTOTYPE_TAG, TEXT("00.Leopard_Effect"), CREATOR(CLeoJump_Smoke),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshParticleCommon::PROTOTYPE_TAG, CMeshParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CLeoJump_Smoke::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Leopard/Effect/Smoke/BossLeopard_00_SmokeSphereOriginal.ysh"));
        )
    );
    Register(CAssault_Smoke::PROTOTYPE_TAG, TEXT("00.Leopard_Effect"), CREATOR(CAssault_Smoke),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshParticleCommon::PROTOTYPE_TAG, CMeshParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CAssault_Smoke::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Leopard/Effect/Smoke/BossLeopard_00_SmokeSphereOriginal.ysh"));
        )
    );
}

void CGameObject_Factory::Free()
{
    m_Registrations.clear();
}
