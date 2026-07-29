#include "GameObject_Factory.h"
#include "GameContent_const.h"
#include "GameInstance.h"
#include "GameObject.h"
#include "Camera_Free.h"
#include "UI_Include.h"

// Kirby
#include "Kirby_Include.h"

// Effect
#include "SwordHitEffect.h"
#include "WarpOutStart.h"
#include "WarpOutEnd.h"
#include "WarpInEffect.h"
#include "Effect_Include.h"

//sky
#include "SkySphere.h"

// Monster
#include "Monster_Include.h"

//Miniboss
#include "MiniBoss_Include.h"

//MainBoss
#include "MainBoss_Include.h"

//Boss Effect
#include "BossEffect_Include.h"

// LevelDesign
#include "LD_Include.h"

// EnvObject
#include "Env_Include.h"

// Projectile
#include "Projectile_Include.h"

// Ability Bubble
#include "EssenceBubble.h"
#include "DroppedBubble.h"

#include "Ability_Model.h"

//System Object
#include "SystemObject_Include.h"

#include "NPC_CutScene_Include.h"

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
    Register(CUI_CurtainStatic::PROTOTYPE_TAG, TEXT("UI_OBJECT"), CREATOR(CUI_CurtainStatic), LOADER());
    Register(CUI_CurtainStamp::PROTOTYPE_TAG, TEXT("UI_OBJECT"), CREATOR(CUI_CurtainStamp), LOADER());
    Register(CUI_CurtainFadeOut::PROTOTYPE_TAG, TEXT("UI_OBJECT"), CREATOR(CUI_CurtainFadeOut), LOADER());
}

void CGameObject_Factory::Register_Camera()
{
    Register(TEXT("Proto_CameraFree"), TEXT("CAMERA_OBJECT"),
        CREATOR(CCamera_Free), LOADER());
}

void CGameObject_Factory::Register_Test()
{
    Register(CTestContainer::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CTestContainer),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CDistortionCommon::PROTOTYPE_TAG,
                CDistortionCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CTestContainer::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSE/Effect/Common_Ring03/Model_Common_Ring03.ysh"));
            TRY_ADD_PROTO(pProxy, Texture_TestNormal.iLevelID, Texture_TestNormal.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_TestNormal.szFileTag,
                    Texture_TestNormal.iNumTex));
        )
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

            // Kirby_DeformRollerCoaster_Main
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_DeformRollerCoaster_Main::PROTOTYPE_TAG,
                CKirby_DeformRollerCoaster_Main::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Kirby_DeformRollerCoaster_Main"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/RollerCoaster/RollerCoaster.ysh",
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

            // Meta Sword
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_MetaSword::PROTOTYPE_TAG, CKirby_MetaSword::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_MetaSword"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/MetaSword/MetaSword.ysh"));
            // Meta Hat
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_MetaHat::PROTOTYPE_TAG, CKirby_MetaHat::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_MetaHat"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/MetaHat/MetaHat.ysh"));

            // Toy Hammer
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_ToyHammer::PROTOTYPE_TAG,
                CKirby_ToyHammer::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_ToyHammer"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/Hammer/ToyHammer/ToyHammer.ysh"));

            // Toy Hat
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_ToyHat::PROTOTYPE_TAG,
                CKirby_ToyHat::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_ToyHat"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/Hammer/ToyHat/ToyHat.ysh"));

            // Bomb Hat
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_BombHat::PROTOTYPE_TAG, CKirby_BombHat::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_BombHat"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/Bomb/Hat/BombHat.ysh"));

            // Crash Hat
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_CrashHat::PROTOTYPE_TAG,
                CKirby_CrashHat::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_CrashHat"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/Crash/CrashHat/CrashHat.ysh"));

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

            // Sleep Hat
            TRY_ADD_PROTO(pProxy, iLevelIndex, CKirby_SleepHat::PROTOTYPE_TAG,
                CKirby_SleepHat::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SleepHat"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSE/Sleep/SleepHat/SleepHat.ysh"));

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
                    XMMatrixScaling(1.4f, 1.4f, 1.4f) * XMMatrixRotationY(XMConvertToRadians(180.f))));
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

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_RabbitEnemyBig_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM,  "../../Resources/CHJ/Monster/RabbitEnemyBig/Body/RabbitEnemyBig.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );

    // Gigatzo
    Register
    (
        CGigatzo::PROTOTYPE_TAG, TEXT("Monster"),
        CREATOR(CGigatzo),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CGigatzo_Body::PROTOTYPE_TAG,
                CGigatzo_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Gigatzo_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM,
                    "../../Resources/CHJ/Monster/Gigatzo/Body/Gigatzo.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));

            // GigatzoBullet (Projectile)
            TRY_ADD_PROTO(pProxy, iLevelIndex, CGigatzoBullet::PROTOTYPE_TAG, CGigatzoBullet::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CGigatzoBullet::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/Gigatzo/Bullet/Gigatzo_Bullet.ysh",
                    XMMatrixScaling(1.25f, 1.25f, 1.25f) * XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );

    // Noddy
    Register
    (
        CNoddy::PROTOTYPE_TAG, TEXT("Monster"),
        CREATOR(CNoddy),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CNoddy_Body::PROTOTYPE_TAG, CNoddy_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Noddy_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/Noddy/Body/Noddy.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );

    // RangerEnemy
    Register
    (
        CRangerEnemy::PROTOTYPE_TAG, TEXT("Monster"),
        CREATOR(CRangerEnemy),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRangerEnemy_Body::PROTOTYPE_TAG, CRangerEnemy_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_RangerEnemy_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/RangerEnemy/Body/RangerEnemy.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
        )
    );

    // SirKibble
    Register
    (
        CSirKibble::PROTOTYPE_TAG, TEXT("Monster"),
        CREATOR(CSirKibble),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSirKibble_Body::PROTOTYPE_TAG, CSirKibble_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SirKibble_Body"),
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Monster/SirKibble/Body/SirKibble.ysh",
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
            TRY_ADD_PROTO(pProxy, iLevelIndex, CWaddleDee_Hat::PROTOTYPE_TAG, CWaddleDee_Hat::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CCage_WaddleDee::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/WaddleDee/Body/Model_Anim.ysh", XMMatrixRotationY(XMConvertToRadians(180.f))));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_TownHat_Pharmacy"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Gimmick/TownHat/Pharmacy.ysh"));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_TownHat_FoodShop"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Gimmick/TownHat/FoodShop.ysh"));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_TownHat_Knowledge"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Gimmick/TownHat/Knowledge.ysh"));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_TownHat_RollingBall"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Gimmick/TownHat/RollingBall.ysh"));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_TownHat_DeliveryService"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Gimmick/TownHat/DeliveryService.ysh", XMMatrixTranslation(0.f, 0.5f, 0.f)));

            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_TownHat_Arena"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Gimmick/TownHat/Arena.ysh"));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CWaddleDee_Body::EYE_TEX_PROTO, 
                CTexture::Create(pDevice, pContext, L"../../Resources/YSH/WaddleDee/Body/DeeEye.%02d.dds", CWaddleDee_Body::EYE_COUNT));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CWaddleDee_Body::EYEMASK_TEX_PROTO,
                CTexture::Create(pDevice, pContext, L"../../Resources/YSH/WaddleDee/Body/DeeEyeMask.%02d.dds", CWaddleDee_Body::EYE_COUNT));
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
    Register(CUI_UIFadeOut::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_UIFadeOut), LOADER());
    Register(CUI_AbilityDiscard::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_AbilityDiscard), LOADER());
    Register(CUI_ClickQTE::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_ClickQTE), LOADER());
    Register(CUI_CreditCoordinator::PROTOTYPE_TAG, TEXT("UI_CONTAINER"), CREATOR(CUI_CreditCoordinator), LOADER());
}

void CGameObject_Factory::Register_NonAnimObject()
{
    Register(CEnvTrigger_Generic::PROTOTYPE_TAG, TEXT("ENV_TRIGGER"), CREATOR(CEnvTrigger_Generic), LOADER());
    Register(CEnvTrigger_RenderGlobals::PROTOTYPE_TAG, TEXT("ENV_TRIGGER"), CREATOR(CEnvTrigger_RenderGlobals), LOADER());
    Register(CEnvTrigger_EventPublisher::PROTOTYPE_TAG, TEXT("ENV_TRIGGER"), CREATOR(CEnvTrigger_EventPublisher), LOADER());
    Register(CEnvTrigger_Debug::PROTOTYPE_TAG, TEXT("ENV_TRIGGER"), CREATOR(CEnvTrigger_Debug), LOADER());
    Register(CEnvTrigger_LevelChange::PROTOTYPE_TAG, TEXT("ENV_TRIGGER"), CREATOR(CEnvTrigger_LevelChange), LOADER());

    Register(CEnvVolume_Effect::PROTOTYPE_TAG, TEXT("ENV_TRIGGER"), CREATOR(CEnvVolume_Effect), LOADER());
    Register(CEnvVolume_Culling::PROTOTYPE_TAG, TEXT("ENV_TRIGGER"), CREATOR(CEnvVolume_Culling), LOADER());
    Register(CEnvVolume_Light::PROTOTYPE_TAG, TEXT("ENV_TRIGGER"), CREATOR(CEnvVolume_Light), LOADER());
    Register(CEnv_SpotLight::PROTOTYPE_TAG, TEXT("ENV_TRIGGER"), CREATOR(CEnv_SpotLight), LOADER());

    Register(CLevelDesign_Unsupported::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLevelDesign_Unsupported), LOADER());
    Register(CLevelDesign_Rail::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLevelDesign_Rail), LOADER());
    Register(CLD_AudioArea::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLD_AudioArea), LOADER());
    Register(CLD_LensFlare::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLD_LensFlare), LOADER());

    Register(CLD_WaterArea::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLD_WaterArea),
        LOADER(TRY_ADD_PROTO(pProxy, iLevelIndex, CLD_WaterArea::MODEL_PROTO_TAG,
            Create_TextureHubModel(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Gimmick/NonAnim/Water/Water.ysh", false));));

    Register(CLevelDesign_Starblock::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLevelDesign_Starblock),
        LOADER(TRY_ADD_PROTO(pProxy, ETOUI(LEVEL::STATIC), CLevelDesign_Starblock::STARBLOCK_MODEL_PROTO_TAG,
            Create_TextureHubModel(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Gimmick/NonAnim/Star/H1W1.ysh", false));));
    Register(CLD_MeteorGenerator::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLD_MeteorGenerator),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeteorRock_Large::PROTOTYPE_TAG, CMeteorRock_Large::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeteorRock_Large::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/CHJ/Gimmick/VolcanoRock/Large/VolcanoRock_L.ysh"));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeteorRock_Small::PROTOTYPE_TAG, CMeteorRock_Small::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeteorRock_Small::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/CHJ/Gimmick/VolcanoRock/Small/VolcanoRock_S.ysh"));
        )
    );

    Register(CLD_BattleBoundary::PROTOTYPE_TAG, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLD_BattleBoundary),
        LOADER(TRY_ADD_PROTO(pProxy, iLevelIndex, CLD_BattleBoundary::MODEL_PROTO_TAG,
            Create_TextureHubModel(pDevice, pContext, MODEL::NONANIM, CLD_BattleBoundary::MODEL_PATH, true));));

    Register(CLD_BattleBoundary::PROTOTYPE_TAG_CYLINDRICAL, TEXT("LEVELDESIGN_OBJECT"), CREATOR(CLD_BattleBoundary),
        LOADER(TRY_ADD_PROTO(pProxy, iLevelIndex, CLD_BattleBoundary::MODEL_PROTO_TAG_CYLINDRICAL,
            Create_TextureHubModel(pDevice, pContext, MODEL::NONANIM, CLD_BattleBoundary::MODEL_PATH_CYLINDRICAL, true));));
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
            Create_TextureHubModel(pDevice, pContext, MODEL::ANIM, "../../Resources/CHJ/Gimmick/CopyEssence/CopyEssence.ysh", true));));
}

void CGameObject_Factory::Register_Effect()
{
    Register(CSwordJumpSpin::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSwordJumpSpin),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwordJumpSpin::MODEL_PROTO_TAG, CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSE/Effect/Sword/00_SwordJumpSpin/Common_Curve03.ysh", XMMatrixRotationY(XMConvertToRadians(90.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwordJumpSpin::TAIL_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext, TEXT("../../Resources/YSE/Effect/Sword/00_SwordJumpSpin/common_tail.dds"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwordJumpSpin::SCROLL_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext, TEXT("../../Resources/YSE/Effect/Sword/00_SwordJumpSpin/common_scroll06.dds"), 1));
        )
    );

    Register(CHammerSwing::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CHammerSwing),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CDistortionCommon::PROTOTYPE_TAG, CDistortionCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CHammerSwing::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSE/Effect/Hammer/00_HammerSwing/SwingTorus3.ysh"));
            TRY_ADD_PROTO(pProxy, Texture_HammerSwing_Distortion.iLevelID,
                Texture_HammerSwing_Distortion.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_HammerSwing_Distortion.szFileTag, Texture_HammerSwing_Distortion.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_HammerSwing_Shape.iLevelID, Texture_HammerSwing_Shape.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_HammerSwing_Shape.szFileTag, Texture_HammerSwing_Shape.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_HammerSwing_Edge.iLevelID, Texture_HammerSwing_Edge.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_HammerSwing_Edge.szFileTag, Texture_HammerSwing_Edge.iNumTex));
        )
    );

    Register(CWheelHammer::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CWheelHammer),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CDistortionCommon::PROTOTYPE_TAG, CDistortionCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CWheelHammer::MODEL_PROTO_TAG, CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSE/Effect/Hammer/00_HammerSwing/SwingTorus3.ysh"));
            TRY_ADD_PROTO(pProxy, Texture_HammerSwing_Distortion.iLevelID, Texture_HammerSwing_Distortion.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_HammerSwing_Distortion.szFileTag, Texture_HammerSwing_Distortion.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_HammerSwing_Shape.iLevelID, Texture_HammerSwing_Shape.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_HammerSwing_Shape.szFileTag, Texture_HammerSwing_Shape.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_HammerSwing_Edge.iLevelID, Texture_HammerSwing_Edge.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_HammerSwing_Edge.szFileTag, Texture_HammerSwing_Edge.iNumTex));
        )
    );
    Register(COnigorosiHammerFirst::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(COnigorosiHammerFirst),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, COnigorosiHammerFirst::MODEL_PROTO_TAG, CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSE/Effect/Hammer/Common/OnigorosiFirst.ysh"));
            TRY_ADD_PROTO(pProxy, Texture_OnigorosiHammerFirst.iLevelID, Texture_OnigorosiHammerFirst.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_OnigorosiHammerFirst.szFileTag, Texture_OnigorosiHammerFirst.iNumTex));
        )
    );
    Register(CCoasterWind::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CCoasterWind),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectEmitterCommon::PROTOTYPE_TAG, CRectEmitterCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_CoasterWind.iLevelID, Texture_CoasterWind.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_CoasterWind.szFileTag, Texture_CoasterWind.iNumTex));
        )
    );

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
                CTexture::Create(pDevice, pContext, Texture_CommonHit01.szFileTag, Texture_CommonHit01.iNumTex));
        ));

    Register(CSwordHitEffect::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSwordHitEffect),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshParticleCommon::PROTOTYPE_TAG, CMeshParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_Meta_Ring1.iLevelID, Texture_Meta_Ring1.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_Ring1.szFileTag, Texture_Meta_Ring1.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_SwordHit_Flash02.iLevelID, Texture_SwordHit_Flash02.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_SwordHit_Flash02.szFileTag, Texture_SwordHit_Flash02.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_Meta_HitFire1.iLevelID, Texture_Meta_HitFire1.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_HitFire1.szFileTag, Texture_Meta_HitFire1.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_Meta_HitFire2.iLevelID, Texture_Meta_HitFire2.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_HitFire2.szFileTag, Texture_Meta_HitFire2.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_Meta_Line2.iLevelID, Texture_Meta_Line2.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_Line2.szFileTag, Texture_Meta_Line2.iNumTex));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwordHitEffect::MODEL_PROTO_TAG_CIRCLE,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Lock/Metaknight_00_Common_Circle01.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwordHitEffect::MODEL_PROTO_TAG_RING,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Lock/Metaknight_00_Common_Ring03High.ysh"));
        )
    );

    Register(CWarpOutStart::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CWarpOutStart),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectParticleCommon::PROTOTYPE_TAG,
                CRectParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_Kabu_CommonLine.iLevelID, Texture_Kabu_CommonLine.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Kabu_CommonLine.szFileTag, Texture_Kabu_CommonLine.iNumTex));
        )
    );

    Register(CWarpOutEnd::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CWarpOutEnd),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectCommon::PROTOTYPE_TAG,
                CRectCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshParticleCommon::PROTOTYPE_TAG,
                CMeshParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_Kabu_FlashCircle.iLevelID, Texture_Kabu_FlashCircle.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Kabu_FlashCircle.szFileTag, Texture_Kabu_FlashCircle.iNumTex));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CWarpOutEnd::SMOKE_MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/CHJ/Effect/SmokeMesh/Model_SmokeSphereOriginal.ysh"));
        )
    );

    Register(CWarpInEffect::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CWarpInEffect),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectParticleCommon::PROTOTYPE_TAG,
                CRectParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_Kabu_CommonLine.iLevelID, Texture_Kabu_CommonLine.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Kabu_CommonLine.szFileTag, Texture_Kabu_CommonLine.iNumTex));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectCommon::PROTOTYPE_TAG,
                CRectCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshParticleCommon::PROTOTYPE_TAG,
                CMeshParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_Kabu_FlashCircle.iLevelID, Texture_Kabu_FlashCircle.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Kabu_FlashCircle.szFileTag, Texture_Kabu_FlashCircle.iNumTex));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CWarpInEffect::SMOKE_MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/CHJ/Effect/SmokeMesh/Model_SmokeSphereOriginal.ysh"));
        )
    );

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
                CTexture::Create(pDevice, pContext, Texture_CommonSparkle02.szFileTag, Texture_CommonSparkle02.iNumTex));

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

    Register(COnLadderEffect::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(COnLadderEffect),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectCommon::PROTOTYPE_TAG,
                CRectCommon::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, Texture_Star2D.iLevelID, Texture_Star2D.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Star2D.szFileTag,
                    Texture_Star2D.iNumTex));
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

    // CarThinGas
    Register(CCarThinGas::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CCarThinGas),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshEmitterCommon::PROTOTYPE_TAG,
                CMeshEmitterCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CCarThinGas::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSE/Effect/CarThinGas/Common_00_PuncSub.ysh"));
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
            TRY_ADD_PROTO(pProxy, iLevelIndex, Model_Stone.szProtoTag, CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Stone/Stone.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, Model_StoneDust.szProtoTag, CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Stone/StoneDust.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, Model_SmokeSphereOriginal.szProtoTag, CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );

    // 10
    Register(CSplit_Stone::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSplit_Stone),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshEmitterCommon::PROTOTYPE_TAG, CMeshEmitterCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Nature_Piece01"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/LandBreak/Nature_Piece01.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Nature_Piece02"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/LandBreak/Nature_Piece02.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, Model_Stone.szProtoTag, CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Stone/Stone.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, Model_StoneDust.szProtoTag, CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Stone/StoneDust.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_StoneHiMesh"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Stone/StoneHiMesh.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, Model_SmokeSphereOriginal.szProtoTag, CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
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
            TRY_ADD_PROTO(pProxy, iLevelIndex, Model_StoneDust.szProtoTag, CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/Stone/StoneDust.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, Model_SmokeSphereOriginal.szProtoTag, CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
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
            TRY_ADD_PROTO(pProxy, iLevelIndex, Model_SmokeSphereOriginal.szProtoTag, CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
        )
    );

    // 15
    Register(CSplit_Cylinder::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSplit_Cylinder),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshEmitterCommon::PROTOTYPE_TAG, CMeshEmitterCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Cylinder_DrainM"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/CylinderBreak/DrainM.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_Cylinder_PieceM"), CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Map/Effect/CylinderBreak/PieceM.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, Model_SmokeSphereOriginal.szProtoTag, CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
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

    // VanishEffect
    Register(CVanishEffect::PROTOTYPE_TAG, TEXT("Effect_Container"),
        CREATOR(CVanishEffect),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeSphereOriginal"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeSphereOriginal/Model_SmokeSphereOriginal.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, TEXT("Prototype_Component_Model_SmokeLowPoly"),
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/Test/Effect/SmokeLowPoly/Model_SmokeLowPoly.ysh"));
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

    // Shared Sword / Meta SpinSlash
    Register(CSwordSpinSlash::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSwordSpinSlash),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG,
                CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CDistortionCommon::PROTOTYPE_TAG,
                CDistortionCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwordSpinSlash::MODEL_PROTO_TAG_RING,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSE/Effect/Sword/00_SwordSpinSlash/Common_Ring03.ysh",
                    XMMatrixRotationX(XMConvertToRadians(90.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwordSpinSlash::MODEL_PROTO_TAG_RING_HIGH,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSE/Effect/Sword/00_SwordSpinSlash/Common_Ring03High.ysh",
                    XMMatrixRotationX(XMConvertToRadians(90.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwordSpinSlash::DISTORTION_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/Sword/00_SwordSpinSlash/indirectwarpring2_normal.dds"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwordSpinSlash::SPIN_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/Sword/00_SwordSpinSlash/common_spin02.dds"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwordSpinSlash::MASK_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/Sword/00_SwordSpinSlash/common_circle05.dds"), 1));
        )
    );

    Register(CSwordSuperSpinSlash::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CSwordSuperSpinSlash),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG,
                CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwordSuperSpinSlash::MODEL_PROTO_TAG_RING,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSE/Effect/Sword/00_SwordSuperSpinSlash/Common_Ring03.ysh",
                    XMMatrixRotationX(XMConvertToRadians(90.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwordSuperSpinSlash::SPIN01_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/Sword/00_SwordSuperSpinSlash/common_spin01.dds"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwordSuperSpinSlash::SPIN06_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/Sword/00_SwordSuperSpinSlash/common_spin06.dds"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CSwordSuperSpinSlash::CIRCLE04_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/Sword/00_SwordSuperSpinSlash/common_circle04.dds"), 1));
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

    Register(CMeteorExplosion::PROTOTYPE_TAG, TEXT("Effect_Container"), CREATOR(CMeteorExplosion),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshEmitterCommon::PROTOTYPE_TAG, CMeshEmitterCommon::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeteorExplosion::PIECE_SMALL_MODEL_TAG,
                        CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/CHJ/Gimmick/VolcanoRock/Piece/VolcanoRock_Piece_PieceSmall.ysh",
                            XMMatrixRotationY(XMConvertToRadians(180.f))));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeteorExplosion::PIECE_COOL_MODEL_TAG,
                        CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/CHJ/Gimmick/VolcanoRock/Piece/VolcanoRock_Piece_PieceCool.ysh",
                            XMMatrixRotationY(XMConvertToRadians(180.f))));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeteorExplosion::SPHERE_MODEL_TAG,
                        CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/CHJ/Effect/Meteor/Common_Spere.ysh"));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeteorExplosion::SMOKE_MODEL_TAG,
                        CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/CHJ/Effect/SmokeMesh/Model_SmokeSphereOriginal.ysh"));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeteorExplosion::PUFF_MODEL_TAG,
                        CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/CHJ/Effect/Meteor/Common_SmokeSphereFadeLarge.ysh"));
        ));
}

void CGameObject_Factory::Register_BossEffect()
{
    Gorilla_Effect();
    Armadillo_Effect();
    Leopard_Effect();
    Metaknight_Effect();
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

    Register(CBoss_Metaknight::PROTOTYPE_TAG, TEXT("MainBoss"),
        CREATOR(CBoss_Metaknight),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Metaknight_Body::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/Boss/Metaknight/Body/Model_Anim.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Metaknight_Body::PROTOTYPE_TAG, CBoss_Metaknight_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Metaknight_Sword::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Sword/Sword.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Metaknight_Sword::PROTOTYPE_TAG,
                CBoss_Metaknight_Sword::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Metaknight_ReplicaSword::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/ReplicaSword/ReplicaSword.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Metaknight_ReplicaSword::PROTOTYPE_TAG,
                CBoss_Metaknight_ReplicaSword::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Metaknight_Mant::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/Boss/Metaknight/Mant/Mant_Anim.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Metaknight_Mant::PROTOTYPE_TAG,
                CBoss_Metaknight_Mant::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Metaknight_EscapeMant::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/Boss/Metaknight/EscapeMant/EscapeMant_Anim.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CBoss_Metaknight_EscapeMant::PROTOTYPE_TAG,
                CBoss_Metaknight_EscapeMant::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CProjectile_MoonShot::PROTOTYPE_TAG, CProjectile_MoonShot::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CAttackDecal::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/RockDecal/Cube.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CAttackDecal::PROTOTYPE_TAG, CAttackDecal::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CProjectile_Rock::MODEL_PROTO_TAG_A,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/Boss/Metaknight/Rock/BurstTornadoDebris_Anim.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CProjectile_Rock::MODEL_PROTO_TAG_B,
                CModel::Create(pDevice, pContext, MODEL::ANIM, "../../Resources/YSH/Boss/Metaknight/Rock/BurstTornadoDebrisB_Anim.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CProjectile_Rock::PROTOTYPE_TAG, CProjectile_Rock::Create(pDevice, pContext));
        )
    );

    Register(CMetaknightNamePlate::PROTOTYPE_TAG, TEXT("NamePlate"),
        CREATOR(CMetaknightNamePlate),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaknightNamePlate::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/NamePlate/Model.ysh",
                    XMMatrixRotationY(XMConvertToRadians(180.f))));
        ));

    Register(CExcalibur::PROTOTYPE_TAG, TEXT("MainBoss"),
        CREATOR(CExcalibur),
        LOADER
        (
            TRY_ADD_PROTO(pProxy, iLevelIndex, CExcalibur::PROTOTYPE_TAG,
                CExcalibur::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CExcalibur_Body::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Excalibur/Body/Sword.ysh"
                    , XMMatrixRotationX(XMConvertToRadians(-90.f)) * XMMatrixTranslation(0.f, 1.4f, 0.f)
                ));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CExcalibur_Body::PROTOTYPE_TAG,
                CExcalibur_Body::Create(pDevice, pContext));

            TRY_ADD_PROTO(pProxy, iLevelIndex, CExcalibur_GetIt::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Excalibur/GetIt/Model.ysh"
                    , XMMatrixRotationY(XMConvertToRadians(180.f))
                ));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CExcalibur_GetIt::PROTOTYPE_TAG, CExcalibur_GetIt::Create(pDevice, pContext));
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

void CGameObject_Factory::Metaknight_Effect()
{
    Register(CMoonShot::PROTOTYPE_TAG, TEXT("00.Metaknight_Effect"), CREATOR(CMoonShot),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG,
                CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMoonShot::TEXTURE_PROTO_TAG_FIRE_FORM,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/MetaKnightSword/MoonShot/fireform01.dds"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMoonShot::MODEL_PROTO_TAG_MOON,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSE/Effect/MetaKnightSword/MoonShot/MoonShot.ysh"));
        )
    );

    Register(CMetaSlash1::PROTOTYPE_TAG, TEXT("00.Metaknight_Effect"), CREATOR(CMetaSlash1),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG,
                CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaSlash1::MODEL_PROTO_TAG_RING,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSE/Effect/MetaKnightSword/MetaSlash1/Common_Ring03.ysh",
                    XMMatrixRotationX(XMConvertToRadians(90.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaSlash1::SPIN_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/MetaKnightSword/MetaSlash1/common_spin01.dds"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaSlash1::CIRCLE05_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/MetaKnightSword/MetaSlash1/common_circle05.dds"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaSlash1::CIRCLE06_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/MetaKnightSword/MetaSlash1/common_circle06.dds"), 1));
        )
    );

    Register(CMetaSlash2::PROTOTYPE_TAG, TEXT("00.Metaknight_Effect"), CREATOR(CMetaSlash2),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG,
                CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaSlash2::MODEL_PROTO_TAG_RING,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSE/Effect/MetaKnightSword/MetaSlash2/Common_Ring03.ysh",
                    XMMatrixRotationX(XMConvertToRadians(90.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaSlash2::SPIN_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/MetaKnightSword/MetaSlash2/common_spin01.dds"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaSlash2::CIRCLE05_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/MetaKnightSword/MetaSlash2/common_circle05.dds"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaSlash2::CIRCLE06_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/MetaKnightSword/MetaSlash2/common_circle06.dds"), 1));
        )
    );

    Register(CMetaDecisiveSlash::PROTOTYPE_TAG, TEXT("00.Metaknight_Effect"), CREATOR(CMetaDecisiveSlash),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG,
                CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaDecisiveSlash::MODEL_PROTO_TAG_RING,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSE/Effect/MetaKnightSword/MetaDecisiveSlash/Common_Ring03.ysh",
                    XMMatrixRotationX(XMConvertToRadians(90.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaDecisiveSlash::SPIN_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/MetaKnightSword/MetaDecisiveSlash/common_spin01.dds"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaDecisiveSlash::CIRCLE05_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/MetaKnightSword/MetaDecisiveSlash/common_circle05.dds"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaDecisiveSlash::CIRCLE06_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/MetaKnightSword/MetaDecisiveSlash/common_circle06.dds"), 1));
        )
    );

    Register(CMetaSuperSpinSlash::PROTOTYPE_TAG, TEXT("00.Metaknight_Effect"), CREATOR(CMetaSuperSpinSlash),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG,
                CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CRectParticleCommon::PROTOTYPE_TAG,
                CRectParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaSuperSpinSlash::MODEL_PROTO_TAG_RING,
                CModel::Create(pDevice, pContext, MODEL::NONANIM,
                    "../../Resources/YSE/Effect/MetaKnightSword/MetaSuperSpinSlash/Common_Ring03.ysh",
                    XMMatrixRotationX(XMConvertToRadians(90.f))));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaSuperSpinSlash::SPIN01_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/MetaKnightSword/MetaSuperSpinSlash/common_spin01.dds"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaSuperSpinSlash::SPIN06_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/MetaKnightSword/MetaSuperSpinSlash/common_spin06.dds"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaSuperSpinSlash::CIRCLE04_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/MetaKnightSword/MetaSuperSpinSlash/common_circle04.dds"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaSuperSpinSlash::CIRCLE05_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/MetaKnightSword/MetaSuperSpinSlash/common_circle05.dds"), 1));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMetaSuperSpinSlash::CIRCLE01_TEXTURE_PROTO_TAG,
                CTexture::Create(pDevice, pContext,
                    TEXT("../../Resources/YSE/Effect/MetaKnightSword/MetaSuperSpinSlash/common_circle01.dds"), 1));
        )
    );

    Register(CMeta_IntroLocking::PROTOTYPE_TAG, TEXT("00.Metaknight_Effect"), CREATOR(CMeta_IntroLocking),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshParticleCommon::PROTOTYPE_TAG, CMeshParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_Meta_Flash1.iLevelID, Texture_Meta_Flash1.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_Flash1.szFileTag, Texture_Meta_Flash1.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_Meta_Ring1.iLevelID, Texture_Meta_Ring1.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_Ring1.szFileTag, Texture_Meta_Ring1.iNumTex));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_IntroLocking::MODEL_PROTO_TAG_CIRCLE,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Lock/Metaknight_00_Common_Circle01.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_IntroLocking::MODEL_PROTO_TAG_RING,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Lock/Metaknight_00_Common_Ring03High.ysh"));
        )
    );

    Register(CMeta_UpperCharge::PROTOTYPE_TAG, TEXT("00.Metaknight_Effect"), CREATOR(CMeta_UpperCharge),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshParticleCommon::PROTOTYPE_TAG, CMeshParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_Meta_Flash1.iLevelID, Texture_Meta_Flash1.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_Flash1.szFileTag, Texture_Meta_Flash1.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_Meta_Shine1.iLevelID, Texture_Meta_Shine1.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_Shine1.szFileTag, Texture_Meta_Shine1.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_Meta_CircleFlash.iLevelID, Texture_Meta_CircleFlash.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_CircleFlash.szFileTag, Texture_Meta_CircleFlash.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_Meta_Line1.iLevelID, Texture_Meta_Line1.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_Line1.szFileTag, Texture_Meta_Line1.iNumTex));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_UpperCharge::MODEL_PROTO_TAG_CIRCLE,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Lock/Metaknight_00_Common_Circle01.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_UpperCharge::MODEL_PROTO_TAG_RING,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Lock/Metaknight_00_Common_Ring03High.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_UpperCharge::MODEL_PROTO_TAG_THUNDER,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Thunder/Metaknight_00_Common_ThunderLine.ysh"));
        )
    );

    Register(CMeta_DemoUpperCharge::PROTOTYPE_TAG, TEXT("00.Metaknight_Effect"), CREATOR(CMeta_DemoUpperCharge),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshParticleCommon::PROTOTYPE_TAG, CMeshParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_Meta_Flash1.iLevelID, Texture_Meta_Flash1.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_Flash1.szFileTag, Texture_Meta_Flash1.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_Meta_CircleFlash.iLevelID, Texture_Meta_CircleFlash.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_CircleFlash.szFileTag, Texture_Meta_CircleFlash.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_Meta_Line1.iLevelID, Texture_Meta_Line1.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_Line1.szFileTag, Texture_Meta_Line1.iNumTex));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_DemoUpperCharge::MODEL_PROTO_TAG_CIRCLE,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Lock/Metaknight_00_Common_Circle01.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_DemoUpperCharge::MODEL_PROTO_TAG_RING,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Lock/Metaknight_00_Common_Ring03High.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_DemoUpperCharge::MODEL_PROTO_TAG_THUNDER,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Thunder/Metaknight_00_Common_ThunderLine.ysh"));
        )
    );

    Register(CMeta_DemoUpperUp::PROTOTYPE_TAG, TEXT("00.Metaknight_Effect"), CREATOR(CMeta_DemoUpperUp),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_Meta_Flash1.iLevelID, Texture_Meta_Flash1.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_Flash1.szFileTag, Texture_Meta_Flash1.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_Meta_HitFire1.iLevelID, Texture_Meta_HitFire1.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_HitFire1.szFileTag, Texture_Meta_HitFire1.iNumTex));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_DemoUpperUp::MODEL_PROTO_TAG_CIRCLE,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Lock/Metaknight_00_Common_Circle01.ysh"));
        )
    );

    Register(CMeta_DemoUpperAtk::PROTOTYPE_TAG, TEXT("00.Metaknight_Effect"), CREATOR(CMeta_DemoUpperAtk),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshParticleCommon::PROTOTYPE_TAG, CMeshParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_Meta_Ring1.iLevelID, Texture_Meta_Ring1.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_Ring1.szFileTag, Texture_Meta_Ring1.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_Meta_HitFire1.iLevelID, Texture_Meta_HitFire1.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_HitFire1.szFileTag, Texture_Meta_HitFire1.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_Meta_HitFire2.iLevelID, Texture_Meta_HitFire2.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_HitFire2.szFileTag, Texture_Meta_HitFire2.iNumTex));
            TRY_ADD_PROTO(pProxy, Texture_Meta_Line2.iLevelID, Texture_Meta_Line2.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_Line2.szFileTag, Texture_Meta_Line2.iNumTex));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_DemoUpperAtk::MODEL_PROTO_TAG_CIRCLE,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Lock/Metaknight_00_Common_Circle01.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_DemoUpperAtk::MODEL_PROTO_TAG_RING,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Lock/Metaknight_00_Common_Ring03High.ysh"));
        )
    );

    Register(CMeta_DemoUpperFinal::PROTOTYPE_TAG, TEXT("00.Metaknight_Effect"), CREATOR(CMeta_DemoUpperFinal),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshParticleCommon::PROTOTYPE_TAG, CMeshParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_Meta_Line2.iLevelID, Texture_Meta_Line2.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_Line2.szFileTag, Texture_Meta_Line2.iNumTex));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_DemoUpperFinal::MODEL_PROTO_TAG_CIRCLE,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Lock/Metaknight_00_Common_Circle01.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_DemoUpperFinal::MODEL_PROTO_TAG_RING,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Lock/Metaknight_00_Common_Ring03High.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_DemoUpperFinal::MODEL_PROTO_TAG_ROCK,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Lock/RockEffectModel.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_DemoUpperFinal::MODEL_PROTO_TAG_THUNDER,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Thunder/Metaknight_00_Common_ThunderLine.ysh"));
        )
    );

    Register(CMeta_MoonShot::PROTOTYPE_TAG, TEXT("00.Metaknight_Effect"), CREATOR(CMeta_MoonShot),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_MoonShot::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Moon/Metaknight_00_MoonShot.ysh"));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_MoonShot::MODEL_PROTO_TAG_TOP,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Moon/Metaknight_00_MoonShotTop.ysh"));
        )
    );

    Register(CMeta_Slash::PROTOTYPE_TAG, TEXT("00.Metaknight_Effect"), CREATOR(CMeta_Slash),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, Texture_Meta_Slash2.iLevelID, Texture_Meta_Slash2.szProtoTag,
                CTexture::Create(pDevice, pContext, Texture_Meta_Slash2.szFileTag, Texture_Meta_Slash2.iNumTex));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_Slash::MODEL_PROTO_TAG,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Slash/Metaknight_00_Common_Ring03High.ysh"
                , XMMatrixRotationX(XMConvertToRadians(90.f))));
        )
    );

    Register(CMeta_Rock::PROTOTYPE_TAG, TEXT("00.Metaknight_Effect"), CREATOR(CMeta_Rock),
        LOADER(
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeshParticleCommon::PROTOTYPE_TAG, CMeshParticleCommon::Create(pDevice, pContext));
            TRY_ADD_PROTO(pProxy, iLevelIndex, CMeta_Rock::MODEL_PROTO_TAG_ROCK,
                CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Boss/Metaknight/Effect/Rock/BurstTornadoDebrisB.ysh"));
        )
    );
}

void CGameObject_Factory::Free()
{
    m_Registrations.clear();
}
