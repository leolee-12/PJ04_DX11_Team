#include "Effect_Loader.h"
#include "GameInstance_Proxy.h"
#include "GameObject_Factory.h"
#include "Effect_Container.h"
#include "DataLoader.h"

#include "RectCommon.h"
#include "MeshCommon.h"
#include "RectParticleCommon.h"
#include "MeshParticleCommon.h"
#include "RectEmitterCommon.h"
#include "MeshEmitterCommon.h"
#include "TrailCommon.h"
#include "DistortionCommon.h"

IMPLEMENT_SINGLETON(CEffect_Loader)

namespace
{
    struct EFFECT_DB_ENTRY
    {
        const _tchar* szEffectId;  
        const _tchar* szConfigPath;
    };

    static constexpr EFFECT_DB_ENTRY s_EffectDB[] =
    {
        { TEXT("WalkSmoke"),              TEXT("../../Resources/EffectContainerJSON/YSE/WalkSmoke_7_01.json") },
        { TEXT("LandingSmoke"),           TEXT("../../Resources/EffectContainerJSON/YSE/LandingSmoke.JSON") },
        { TEXT("CarLanding"),             TEXT("../../Resources/EffectContainerJSON/YSE/CarLanding.JSON") },
        { TEXT("BombHitAim"),             TEXT("../../Resources/EffectContainerJSON/YSE/BombHitAim.JSON") },
        { TEXT("BombAimDot"),             TEXT("../../Resources/EffectContainerJSON/YSE/BombAimDot.JSON") },
        { TEXT("SlideSmoke"),             TEXT("../../Resources/EffectContainerJSON/YSE/SlideSmoke.JSON") },
        { TEXT("InhaleContainer"),        TEXT("../../Resources/EffectContainerJSON/YSE/Inhale_6_24.json") },
        { TEXT("OnLadderEffect"),         TEXT("../../Resources/EffectContainerJSON/YSE/OnLadderEffect.JSON") },
        { TEXT("SwordSlash1"),            TEXT("../../Resources/EffectContainerJSON/YSE/SwordSlash1_Alpha_Color.json") },
        { TEXT("SwordSlash3"),            TEXT("../../Resources/EffectContainerJSON/YSE/SwordSlash3.json") },
        { TEXT("SwordJumpSpin"),            TEXT("../../Resources/YSE/EffectContainer/Sword/SwordJumpSpin.JSON") },
        { TEXT("SwordJumpSpinTrail1"),        TEXT("../../Resources/YSE/EffectContainer/Sword/SwordJumpSpinTrail1.JSON") },
        { TEXT("SwordJumpSpinTrail2"),        TEXT("../../Resources/YSE/EffectContainer/Sword/SwordJumpSpinTrail2.JSON") },
        { TEXT("SwordSpinSlash"),         TEXT("../../Resources/YSE/EffectContainer/Sword/SwordSpinSlash.JSON") },
        { TEXT("SwordSuperSpinSlash"),    TEXT("../../Resources/YSE/EffectContainer/Sword/SwordSuperSpinSlash.JSON") },
        { TEXT("SwordSpinSlashTrail"),    TEXT("../../Resources/YSE/EffectContainer/Sword/SwordSpinSlashTrail.JSON") },
        { TEXT("SwordChargeEffect"),      TEXT("../../Resources/EffectContainerJSON/YSE/SwordChargeEffect.json") },
        { TEXT("SwordSuperChargeEffect"), TEXT("../../Resources/EffectContainerJSON/YSE/SwordSuperChargeEffect.json") },
        { TEXT("MetaChargeEffect"),       TEXT("../../Resources/YSE/EffectContainer/MetaKnightSword/MetaChargeEffect.JSON") },
        { TEXT("MetaSuperChargeEffect"),  TEXT("../../Resources/YSE/EffectContainer/MetaKnightSword/MetaSuperChargeEffect.JSON") },
        { TEXT("MetaSlash1"),             TEXT("../../Resources/YSE/EffectContainer/MetaKnightSword/MetaSlash1.JSON") },
        { TEXT("MetaSlash2"),             TEXT("../../Resources/YSE/EffectContainer/MetaKnightSword/MetaSlash2.JSON") },
        { TEXT("MetaDecisiveSlash"),     TEXT("../../Resources/YSE/EffectContainer/MetaKnightSword/MetaDecisiveSlash.JSON") },
        { TEXT("MetaSuperSpinSlash"),      TEXT("../../Resources/YSE/EffectContainer/MetaKnightSword/MetaSuperSpinSlash.JSON") },
        { TEXT("MetaSwordJumpSpin"),          TEXT("../../Resources/YSE/EffectContainer/MetaKnightSword/MetaSwordJumpSpin.JSON") },
        { TEXT("MetaSwordJumpSpinTrail1"),  TEXT("../../Resources/YSE/EffectContainer/MetaKnightSword/MetaSwordJumpSpinTrail1.JSON") },
        { TEXT("MetaSwordJumpSpinTrail2"),  TEXT("../../Resources/YSE/EffectContainer/MetaKnightSword/MetaSwordJumpSpinTrail2.JSON") },
        {TEXT("UpwardsSlash"),            TEXT("../../Resources/EffectContainerJSON/YSE/UpwardsSlash.JSON")},
        { TEXT("RockFloor"),              TEXT("../../Resources/EffectContainerJSON/YSH/Proto_RockBurst_0.json") },
        { TEXT("BoostGas"),               TEXT("../../Resources/EffectContainerJSON/YSE/BoostGas.json") },
        { TEXT("MoveGas"),                TEXT("../../Resources/EffectContainerJSON/YSE/MoveGas.json") },
        { TEXT("CylinderRollGas"),        TEXT("../../Resources/EffectContainerJSON/YSE/CylinderRollGas.JSON") },
        { TEXT("CarMilkyWay"),            TEXT("../../Resources/EffectContainerJSON/YSE/CarMilkyWay_Final2.json") },
        { TEXT("CarThinGas"),             TEXT("../../Resources/EffectContainerJSON/YSE/CarThinGas.JSON") },
        { TEXT("CoasterWind"),            TEXT("../../Resources/YSE/EffectContainer/CoasterWind.json") },
        { TEXT("HammerSwing"),            TEXT("../../Resources/YSE/EffectContainer/HammerSwing.JSON") },
        { TEXT("HammerSwingFinal"),       TEXT("../../Resources/YSE/EffectContainer/HammerSwingFinal.JSON") },
        { TEXT("WheelHammer"),            TEXT("../../Resources/YSE/EffectContainer/WheelHammer.JSON") },
        { TEXT("OnigorosiHammerFirst"),   TEXT("../../Resources/YSE/EffectContainer/OnigorosiHammerFirst.JSON") },
        { TEXT("OnigorosiHammerSecond"),  TEXT("../../Resources/YSE/EffectContainer/OnigorosiHammerSecond.JSON") },
        { TEXT("OnigorosiHammerEnd"),     TEXT("../../Resources/YSE/EffectContainer/OnigorosiHammerEnd.JSON") },

        { TEXT("GetAbilityEffect"),       TEXT("../../Resources/EffectContainerJSON/YSE/GetAbilityEffect.JSON") },
        { TEXT("GetDeformEffect"),        TEXT("../../Resources/EffectContainerJSON/YSE/GetDeformEffect .JSON") },
        { TEXT("DespawnEffect"),          TEXT("../../Resources/EffectContainerJSON/CHJ/DespawnEffect.JSON") },

        { TEXT("DeathSmoke"),             TEXT("../../Resources/EffectContainerJSON/YSH/Proto_DeathSmoke_0.json") },
        { TEXT("RockPush"),               TEXT("../../Resources/EffectContainerJSON/YSH/Proto_RockPush_0.json") },
        { TEXT("RockPull"),               TEXT("../../Resources/EffectContainerJSON/YSH/Proto_RockPull_1.json") },
        { TEXT("RockBounce"),             TEXT("../../Resources/EffectContainerJSON/YSH/Proto_RockBounce_2.json") },

        { TEXT("Gorilla_SwingR"),         TEXT("../../Resources/EffectContainerJSON/YSH/SwingR.json") },
        { TEXT("Gorilla_SwingL"),         TEXT("../../Resources/EffectContainerJSON/YSH/SwingL.json") },
        { TEXT("Gorilla_Landing"),        TEXT("../../Resources/EffectContainerJSON/YSH/Gorilla_Landing.json") },

        { TEXT("StampR"),                 TEXT("../../Resources/EffectContainerJSON/YSH/StampR.json") },
        { TEXT("StampL"),                 TEXT("../../Resources/EffectContainerJSON/YSH/StampL.json") },
        { TEXT("Big_ShockWave"),          TEXT("../../Resources/EffectContainerJSON/YSH/Big_ShockWave.json") },
        { TEXT("Stamp_RingR"),            TEXT("../../Resources/EffectContainerJSON/YSH/Stamp_RingR.json") },
        { TEXT("Stamp_RingL"),            TEXT("../../Resources/EffectContainerJSON/YSH/Stamp_RingL.json") },

        { TEXT("BombExplosion"),          TEXT("../../Resources/EffectContainerJSON/YSH/BombExplosion.JSON") },
        { TEXT("CommonHit"),              TEXT("../../Resources/EffectContainerJSON/CHJ/CommonHit.JSON") },
        { TEXT("SpitObject"),             TEXT("../../Resources/EffectContainerJSON/CHJ/SpitObject.JSON") },
        { TEXT("SpitAir"),                TEXT("../../Resources/EffectContainerJSON/YSH/Spit_Air.JSON") },

        { TEXT("BombFuseEffect"),         TEXT("../../Resources/EffectContainerJSON/CHJ/BombFuseEffect.JSON") },

        { TEXT("FlowerPetals"),           TEXT("../../Resources/EffectContainerJSON/MAP/Proto_FlowerPetals_0.JSON") },
        { TEXT("FlowerWing"),             TEXT("../../Resources/EffectContainerJSON/MAP/Proto_FlowerWing.JSON") },
        { TEXT("LensFlare"),              TEXT("../../Resources/Map/Effect/Proto_LensFlare_0.JSON") },
        { TEXT("Split_Starblock"),        TEXT("../../Resources/EffectContainerJSON/MAP/Proto_Split_Starblock_0.JSON") },
        { TEXT("Split_Starblock_Big"),    TEXT("../../Resources/EffectContainerJSON/MAP/Proto_Split_Starblock_Big.JSON") },
        { TEXT("Split_Stone"),            TEXT("../../Resources/EffectContainerJSON/MAP/Proto_Split_Stone_0.JSON") },
        { TEXT("Split_Stone_Big"),        TEXT("../../Resources/EffectContainerJSON/MAP/Proto_Split_Stone_Big.JSON") },
        { TEXT("Split_Stone_Ultra"),      TEXT("../../Resources/EffectContainerJSON/MAP/Proto_Split_Stone_Ultra.JSON") },
        { TEXT("Split_Bush"),             TEXT("../../Resources/EffectContainerJSON/MAP/Proto_Split_Bush_0.JSON") },
        { TEXT("Split_Coaster"),          TEXT("../../Resources/EffectContainerJSON/MAP/Proto_Split_Coaster_0.JSON") },
        { TEXT("Split_Cylinder"),         TEXT("../../Resources/EffectContainerJSON/MAP/Proto_Split_Cylinder_1.JSON") },
        { TEXT("BreakWallEffect"),        TEXT("../../Resources/EffectContainerJSON/MAP/Proto_BreakWallEffect_0.JSON") },
        { TEXT("ItemEffect"),             TEXT("../../Resources/EffectContainerJSON/MAP/Proto_ItemEffect_0.JSON") },
        { TEXT("BubbleAura"),             TEXT("../../Resources/EffectContainerJSON/CHJ/BubbleAura.JSON") },
        { TEXT("LaunchSmoke"),            TEXT("../../Resources/EffectContainerJSON/CHJ/LaunchSmoke.JSON") },
        { TEXT("MoveSmoke"),              TEXT("../../Resources/EffectContainerJSON/CHJ/MoveSmoke.JSON") },
        { TEXT("MonsterLandingSmoke"),    TEXT("../../Resources/EffectContainerJSON/CHJ/MonsterLandingSmoke.JSON") },
        { TEXT("VanishEffect"),           TEXT("../../Resources/Map/Effect/Proto_VanishEffect_0.JSON") },

        { TEXT("SwordTrail_BK"),          TEXT("../../Resources/EffectContainerJSON/CHJ/SwordTrail_BK.JSON") },
        { TEXT("Tornado_BK"),             TEXT("../../Resources/EffectContainerJSON/CHJ/Tornado_BK.JSON") },
        { TEXT("EssenceAura"),            TEXT("../../Resources/EffectContainerJSON/CHJ/EssenceAura.JSON") },
        { TEXT("PickUpEffect"),           TEXT("../../Resources/EffectContainerJSON/CHJ/PickUpEffect.JSON") },
        { TEXT("DropStarEffect"),         TEXT("../../Resources/EffectContainerJSON/CHJ/DropStarEffect.JSON") },

#pragma region Armadillo Effects
        { TEXT("RutA"),                   TEXT("../../Resources/EffectContainerJSON/YSH/RutA.JSON") },
        { TEXT("RutB"),                   TEXT("../../Resources/EffectContainerJSON/YSH/RutB.JSON") },
        { TEXT("Dust"),                   TEXT("../../Resources/EffectContainerJSON/YSH/Dust.JSON") },
        { TEXT("Dust_Landing"),           TEXT("../../Resources/EffectContainerJSON/YSH/Dust_Landing.JSON") },
        { TEXT("TwinDust"),               TEXT("../../Resources/EffectContainerJSON/YSH/TwinDust.JSON") },
        { TEXT("RollWind"),               TEXT("../../Resources/EffectContainerJSON/YSH/RollWind.JSON") },
        { TEXT("TwinSpinWind"),           TEXT("../../Resources/EffectContainerJSON/YSH/TwinSpinWind.JSON") },
        { TEXT("PartnerWind"),            TEXT("../../Resources/EffectContainerJSON/YSH/PartnerWind.JSON") },
        { TEXT("WallImpact"),             TEXT("../../Resources/EffectContainerJSON/YSH/WallImpact.JSON") },
#pragma endregion

#pragma region Leopard Effects
        //Leopard
        { TEXT("LeoSlash_L"),             TEXT("../../Resources/EffectContainerJSON/YSH/LeoSlash_L.JSON") },
        { TEXT("LeoSlash_R"),             TEXT("../../Resources/EffectContainerJSON/YSH/LeoSlash_R.JSON") },
        { TEXT("Leopard_Meteo"),          TEXT("../../Resources/EffectContainerJSON/YSH/Leopard_Meteo.JSON") },
        { TEXT("Nail_Trail"),             TEXT("../../Resources/EffectContainerJSON/YSH/Nail_Trail.JSON") },
        { TEXT("Afterimage_Assault"),     TEXT("../../Resources/EffectContainerJSON/YSH/Afterimage_Assault.JSON") },
        { TEXT("Afterimage_Jump"),        TEXT("../../Resources/EffectContainerJSON/YSH/Afterimage_Jump.JSON") },
        { TEXT("ClawAssault"),            TEXT("../../Resources/EffectContainerJSON/YSH/ClawAssault.JSON") },
        { TEXT("ClawJump"),               TEXT("../../Resources/EffectContainerJSON/YSH/ClawJump.JSON") },
        { TEXT("Leopard_Floor"),          TEXT("../../Resources/EffectContainerJSON/YSH/Leopard_Floor.JSON") },
        { TEXT("Leopard_Flash_R"),        TEXT("../../Resources/EffectContainerJSON/YSH/Leopard_Flash_R.JSON") },
        { TEXT("Leopard_Flash_L"),        TEXT("../../Resources/EffectContainerJSON/YSH/Leopard_Flash_L.JSON") },
        { TEXT("Leopard_Impact"),         TEXT("../../Resources/EffectContainerJSON/YSH/Leopard_Impact.JSON") },
        { TEXT("Assault_Smoke"),          TEXT("../../Resources/EffectContainerJSON/YSH/Assault_Smoke.JSON") },
        { TEXT("Nail_Smoke"),             TEXT("../../Resources/EffectContainerJSON/YSH/Nail_Smoke.JSON") },
        { TEXT("LeoJump_Smoke"),          TEXT("../../Resources/EffectContainerJSON/YSH/LeoJump_Smoke.JSON") },

        { TEXT("MoonShot"),                   TEXT("../../Resources/YSE/EffectContainer/MetaKnightSword/MoonShot.JSON") },
#pragma endregion

#pragma region  MetaKnight Effects
        { TEXT("Meta_IntroLocking"),          TEXT("../../Resources/YSH/Effects/Metaknight/Meta_IntroLocking.JSON") },
        { TEXT("Meta_AppearFlash"),           TEXT("../../Resources/YSH/Effects/Metaknight/Meta_AppearFlash.JSON") },
        { TEXT("Meta_Attack1Flash"),          TEXT("../../Resources/YSH/Effects/Metaknight/Meta_Attack1Flash.JSON") },
        { TEXT("Meta_DemoUpperCharge"),       TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperCharge.JSON") },
        { TEXT("Meta_UpperCharge"),           TEXT("../../Resources/YSH/Effects/Metaknight/Meta_UpperCharge.JSON") },

        { TEXT("Meta_DemoUpperUp"),           TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperUp.JSON") },
        { TEXT("Meta_DemoUpperCharge_End"),   TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperCharge_End.JSON") },
        { TEXT("Meta_DemoUpperFinal"),        TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperFinal.JSON") },
        { TEXT("Meta_Locking"),               TEXT("../../Resources/YSH/Effects/Metaknight/Meta_Locking.JSON") },
        { TEXT("Meta_LockingSpark"),          TEXT("../../Resources/YSH/Effects/Metaknight/Meta_LockingSpark.JSON") },
        { TEXT("Meta_MoonShot"),              TEXT("../../Resources/YSH/Effects/Metaknight/Meta_MoonShot.JSON") },

        { TEXT("Meta_Slash1"),                TEXT("../../Resources/YSH/Effects/Metaknight/Meta_Slash1.JSON") },
        { TEXT("Meta_Slash2"),                TEXT("../../Resources/YSH/Effects/Metaknight/Meta_Slash2.JSON") },
        { TEXT("Meta_Slash3"),                TEXT("../../Resources/YSH/Effects/Metaknight/Meta_Slash3.JSON") },

        { TEXT("Meta_Rock"),                  TEXT("../../Resources/YSH/Effects/Metaknight/Meta_Rock.JSON") },
        { TEXT("Meta_FallCharge"),            TEXT("../../Resources/YSH/Effects/Metaknight/Meta_FallCharge.JSON") },

        { TEXT("Meta_DemoUpperAtk1"),         TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperAtk1.JSON") },
        { TEXT("Meta_DemoUpperAtk2"),         TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperAtk2.JSON") },
        { TEXT("Meta_DemoUpperAtk3"),         TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperAtk3.JSON") },
        { TEXT("Meta_DemoUpperAtk4"),         TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperAtk4.JSON") },
        { TEXT("Meta_DemoUpperAtk5"),         TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperAtk5.JSON") },
        { TEXT("Meta_DemoUpperAtk6"),         TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperAtk6.JSON") },
        { TEXT("Meta_DemoUpperAtk7"),         TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperAtk7.JSON") },
        { TEXT("Meta_DemoUpperAtk8"),         TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperAtk8.JSON") },
        { TEXT("Meta_DemoUpperAtk9"),         TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperAtk9.JSON") },
        { TEXT("Meta_DemoUpperAtk10"),        TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperAtk10.JSON") },
        { TEXT("Meta_DemoUpperAtk11"),        TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperAtk11.JSON") },
        { TEXT("Meta_DemoUpperAtk12"),        TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperAtk12.JSON") },
        { TEXT("Meta_DemoUpperAtk13"),        TEXT("../../Resources/YSH/Effects/Metaknight/Meta_DemoUpperAtk13.JSON") },
#pragma endregion
    };
}

HRESULT CEffect_Loader::Ready(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iLevel)
{
    m_pProxy = pProxy;

    for (const auto& tEntry : s_EffectDB)
    {
        const _wstring strEffectId = tEntry.szEffectId;

        string strContent;
        if (FAILED(CDataLoader::Read_Json(tEntry.szConfigPath, &strContent)))
            continue;

        json jEffect = json::parse(strContent);
        if (!jEffect.contains("Prototype_Tag"))
            continue;

        const _wstring strProtoTag = StrToWstr(jEffect["Prototype_Tag"].get<string>());

        // 프로토타입 STATIC 1회 등록
        if (!pProxy->Has_Prototype(iLevel, strProtoTag))
        {
            auto* pReg = CGameObject_Factory::GetInstance()->Get_Registration(strProtoTag);
            if (!pReg)
                continue;
            pReg->ResourceLoader(pProxy, pDevice, pContext, iLevel);
            pProxy->Add_Prototype(iLevel, strProtoTag.c_str(),
                pReg->CreatorFunc(pDevice, pContext));
        }

        m_Assets[strEffectId] = EFFECT_ASSET{ strProtoTag, std::move(jEffect) };
    }

    if (FAILED(pProxy->Add_Prototype(iLevel, CRectCommon::PROTOTYPE_TAG, CRectCommon::Create(pDevice, pContext))))
        return E_FAIL;
    if (FAILED(pProxy->Add_Prototype(iLevel, CMeshCommon::PROTOTYPE_TAG, CMeshCommon::Create(pDevice, pContext))))
        return E_FAIL;
    if (FAILED(pProxy->Add_Prototype(iLevel, CRectParticleCommon::PROTOTYPE_TAG, CRectParticleCommon::Create(pDevice, pContext))))
        return E_FAIL;
    if (FAILED(pProxy->Add_Prototype(iLevel, CMeshParticleCommon::PROTOTYPE_TAG, CMeshParticleCommon::Create(pDevice, pContext))))
        return E_FAIL;
    if (FAILED(pProxy->Add_Prototype(iLevel, CRectEmitterCommon::PROTOTYPE_TAG, CRectEmitterCommon::Create(pDevice, pContext))))
        return E_FAIL;
    if (FAILED(pProxy->Add_Prototype(iLevel, CMeshEmitterCommon::PROTOTYPE_TAG, CMeshEmitterCommon::Create(pDevice, pContext))))
        return E_FAIL;
    if (FAILED(pProxy->Add_Prototype(iLevel, CTrailCommon::PROTOTYPE_TAG, CTrailCommon::Create(pDevice, pContext))))
        return E_FAIL;
    if (FAILED(pProxy->Add_Prototype(iLevel, CDistortionCommon::PROTOTYPE_TAG, CDistortionCommon::Create(pDevice, pContext))))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Loader::Spawn(const _wstring& strEffectId, _uint iTargetLevel,
    const _float3& vPos, const _float3& vLook, const _float3& vRotDeg,
    const _float4x4* pParent, Engine::CEffect_Container** ppOut,
    FX_HANDLE* pOutHandle)
{
    auto it = m_Assets.find(strEffectId);
    if (it == m_Assets.end())
        return E_FAIL;

    auto& asset = it->second;

    Engine::CEffect_Container::EFFECT_CONTAINER_DESC desc{};
    Engine::CEffect_Container* pFx = nullptr;

    if (FAILED(m_pProxy->Spawn_Effect(iTargetLevel, strEffectId, asset.strProtoTag,
        desc, &asset.Config, &pFx)) || !pFx)
        return E_FAIL;

    pFx->EffectContainer_Start(vPos, vLook, pParent);

    if (vRotDeg.x != 0.f || vRotDeg.y != 0.f || vRotDeg.z != 0.f)
    {
        _matrix matRot = XMMatrixRotationRollPitchYaw(
            XMConvertToRadians(vRotDeg.x),
            XMConvertToRadians(vRotDeg.y),
            XMConvertToRadians(vRotDeg.z));

        _float3 vScale = pFx->Get_Transform()->Get_Scaled();

        pFx->Get_Transform()->Set_State(STATE::RIGHT, XMVector3Normalize(matRot.r[0]) * vScale.x);
        pFx->Get_Transform()->Set_State(STATE::UP, XMVector3Normalize(matRot.r[1]) * vScale.y);
        pFx->Get_Transform()->Set_State(STATE::LOOK, XMVector3Normalize(matRot.r[2]) * vScale.z);
    }

    m_Epochs[pFx] = { ++m_iEpochCounter, iTargetLevel };

    if (ppOut)
        *ppOut = pFx;

    if (pOutHandle)
        *pOutHandle = { pFx, m_iEpochCounter };

    return S_OK;
}

void CEffect_Loader::Clear_Epochs(_uint iLevel)
{
    for (auto it = m_Epochs.begin(); it != m_Epochs.end(); )
        it = (it->second.iLevel == iLevel)
        ? m_Epochs.erase(it) : next(it);
}

_bool CEffect_Loader::Is_Current(const FX_HANDLE& h) const
{
    if (nullptr == h.p)
        return false;

    auto it = m_Epochs.find(h.p);
    return it != m_Epochs.end() && it->second.iEpoch == h.iEpoch;
}

void CEffect_Loader::Free()
{
    m_Assets.clear();
    m_Epochs.clear();
    __super::Free();
}
