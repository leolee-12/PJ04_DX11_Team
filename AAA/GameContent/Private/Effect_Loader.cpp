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

IMPLEMENT_SINGLETON(CEffect_Loader)

namespace
{
    struct EFFECT_DB_ENTRY
    {
        const _tchar* szEffectId;    // ?§Ìè∞ ??
        const _tchar* szConfigPath;  // ?úÎãù json (?∞Ïù¥?∞Î°ú ?†Ï?)
    };

    // === ?¥Ìéô??DB : ???¥Ìéô?∏Îäî ?¨Í∏∞????Ï§?Ï∂îÍ? ===
    static constexpr EFFECT_DB_ENTRY s_EffectDB[] =
    {
        { TEXT("WalkSmoke"),              TEXT("../../Resources/YSE/EffectContainer/WalkSmoke_7_01.json") },
        { TEXT("LandingSmoke"),           TEXT("../../Resources/YSE/EffectContainer/LandingSmoke.JSON") },
        { TEXT("CarLanding"),             TEXT("../../Resources/YSE/EffectContainer/CarLanding.JSON") },
        { TEXT("BombHitAim"),             TEXT("../../Resources/YSE/EffectContainer/BombHitAim.JSON") },
        { TEXT("BombAimDot"),             TEXT("../../Resources/YSE/EffectContainer/BombAimDot.JSON") },
        { TEXT("SlideSmoke"),             TEXT("../../Resources/YSE/EffectContainer/SlideSmoke.JSON") },
        { TEXT("InhaleContainer"),        TEXT("../../Resources/YSE/EffectContainer/Inhale_6_24.json") },
        { TEXT("OnLadderEffect"),         TEXT("../../Resources/YSE/EffectContainer/OnLadderEffect.JSON") },
        { TEXT("SwordSlash1"),            TEXT("../../Resources/YSE/EffectContainer/SwordSlash1_Alpha_Color.json") },
        { TEXT("SwordSlash3"),            TEXT("../../Resources/YSE/EffectContainer/SwordSlash3.json") },
        { TEXT("JumpSlash_1"),            TEXT("../../Resources/YSE/EffectContainer/JumpSlash_1.json") },
        { TEXT("SpinSlash"),              TEXT("../../Resources/YSE/EffectContainer/SpinSlash.json") },
        { TEXT("SpinSlashTrail"),         TEXT("../../Resources/YSE/EffectContainer/SpinSlashTrail.json") },
        { TEXT("SpinSlashTrail_Super"),   TEXT("../../Resources/YSE/EffectContainer/SpinSlashTrail_Super.json") },
        { TEXT("SwordChargeEffect"),      TEXT("../../Resources/YSE/EffectContainer/SwordChargeEffect.json") },
        { TEXT("SwordSuperChargeEffect"), TEXT("../../Resources/YSE/EffectContainer/SwordSuperChargeEffect.json") },
        {TEXT("UpwardsSlash"),            TEXT("../../Resources/YSE/EffectContainer/UpwardsSlash.JSON")},

        { TEXT("RockFloor"),              TEXT("../../Resources/YSH/Effects/Proto_RockBurst_0.json") },
        { TEXT("BoostGas"),               TEXT("../../Resources/YSE/EffectContainer/BoostGas.json") },
        { TEXT("MoveGas"),               TEXT("../../Resources/YSE/EffectContainer/MoveGas.json") },
        { TEXT("CarMilkyWay"),            TEXT("../../Resources/YSE/EffectContainer/CarMilkyWay_Final2.json") },

        { TEXT("GetAbilityEffect"),       TEXT("../../Resources/YSH/Effects/GetAbilityEffect.json") },
        { TEXT("DespawnEffect"),          TEXT("../../Resources/CHJ/Effect/DespawnEffect.JSON") },

        { TEXT("DeathSmoke"),             TEXT("../../Resources/YSH/Effects/Proto_DeathSmoke_0.json") },
        { TEXT("RockPush"),               TEXT("../../Resources/YSH/Effects/Proto_RockPush_0.json") },
        { TEXT("RockPull"),               TEXT("../../Resources/YSH/Effects/Proto_RockPull_1.json") },
        { TEXT("RockBounce"),             TEXT("../../Resources/YSH/Effects/Proto_RockBounce_2.json") },

        { TEXT("Gorilla_SwingR"),         TEXT("../../Resources/YSH/Effects/SwingR.json") },
        { TEXT("Gorilla_SwingL"),         TEXT("../../Resources/YSH/Effects/SwingL.json") },
        { TEXT("Gorilla_Landing"),        TEXT("../../Resources/YSH/Effects/Gorilla_Landing.json") },

        { TEXT("StampR"),                 TEXT("../../Resources/YSH/Effects/StampR.json") },
        { TEXT("StampL"),                 TEXT("../../Resources/YSH/Effects/StampL.json") },
        { TEXT("Big_ShockWave"),          TEXT("../../Resources/YSH/Effects/Big_ShockWave.json") },
        { TEXT("Stamp_RingR"),            TEXT("../../Resources/YSH/Effects/Stamp_RingR.json") },
        { TEXT("Stamp_RingL"),            TEXT("../../Resources/YSH/Effects/Stamp_RingL.json") },

        { TEXT("BombExplosion"),          TEXT("../../Resources/YSH/Effects/BombExplosion.JSON") },
        { TEXT("CommonHit"),              TEXT("../../Resources/CHJ/Effect/CommonHit.JSON") },
        { TEXT("SpitObject"),             TEXT("../../Resources/CHJ/Effect/SpitObject.JSON") },
        { TEXT("SpitAir"),                TEXT("../../Resources/YSH/Effects/Spit_Air.JSON") },

        { TEXT("BombFuseEffect"),         TEXT("../../Resources/CHJ/Effect/BombFuseEffect.JSON") },

        { TEXT("FlowerPetals"),           TEXT("../../Resources/Map/Effect/Proto_FlowerPetals_0.JSON") },
        { TEXT("FlowerWing"),             TEXT("../../Resources/Map/Effect/Proto_FlowerWing.JSON") },
        { TEXT("LensFlare"),              TEXT("../../Resources/Map/Effect/Proto_LensFlare_0.JSON") },
        { TEXT("Split_Starblock"),        TEXT("../../Resources/Map/Effect/Proto_Split_Starblock_0.JSON") },
        { TEXT("Split_Starblock_Big"),    TEXT("../../Resources/Map/Effect/Proto_Split_Starblock_Big.JSON") },
        { TEXT("Split_Stone"),            TEXT("../../Resources/Map/Effect/Proto_Split_Stone_0.JSON") },
        { TEXT("Split_Stone_Big"),        TEXT("../../Resources/Map/Effect/Proto_Split_Stone_Big.JSON") },
        { TEXT("Split_Stone_Ultra"),      TEXT("../../Resources/Map/Effect/Proto_Split_Stone_Ultra.JSON") },
        { TEXT("Split_Bush"),             TEXT("../../Resources/Map/Effect/Proto_Split_Bush_0.JSON") },
        { TEXT("Split_Coaster"),          TEXT("../../Resources/Map/Effect/Proto_Split_Coaster_0.JSON") },
        { TEXT("Split_Cylinder"),         TEXT("../../Resources/Map/Effect/Proto_Split_Cylinder_1.JSON") },
        { TEXT("BreakWallEffect"),        TEXT("../../Resources/Map/Effect/Proto_BreakWallEffect_0.JSON") },
        { TEXT("ItemEffect"),             TEXT("../../Resources/Map/Effect/Proto_ItemEffect_0.JSON") },
        { TEXT("BubbleAura"),             TEXT("../../Resources/CHJ/Effect/BubbleAura.JSON") },
        { TEXT("LaunchSmoke"),            TEXT("../../Resources/CHJ/Effect/LaunchSmoke.JSON") },
        { TEXT("MoveSmoke"),              TEXT("../../Resources/CHJ/Effect/MoveSmoke.JSON") },
        { TEXT("MonsterLandingSmoke"),    TEXT("../../Resources/CHJ/Effect/MonsterLandingSmoke.JSON") },

        { TEXT("SwordTrail_BK"),          TEXT("../../Resources/CHJ/Effect/SwordTrail_BK.JSON") },
        { TEXT("Tornado_BK"),             TEXT("../../Resources/CHJ/Effect/Tornado_BK.JSON") },
        { TEXT("EssenceAura"),            TEXT("../../Resources/CHJ/Effect/EssenceAura.JSON") },
        { TEXT("PickUpEffect"),           TEXT("../../Resources/CHJ/Effect/PickUpEffect.JSON") },
        { TEXT("DropStarEffect"),         TEXT("../../Resources/CHJ/Effect/DropStarEffect.JSON") },

        // Armadillo
        { TEXT("RutA"),                   TEXT("../../Resources/YSH/Effects/RutA.JSON") },
        { TEXT("RutB"),                   TEXT("../../Resources/YSH/Effects/RutB.JSON") },
        { TEXT("Dust"),                   TEXT("../../Resources/YSH/Effects/Dust.JSON") },
        { TEXT("Dust_Landing"),           TEXT("../../Resources/YSH/Effects/Dust_Landing.JSON") },
        { TEXT("TwinDust"),               TEXT("../../Resources/YSH/Effects/TwinDust.JSON") },
        { TEXT("RollWind"),               TEXT("../../Resources/YSH/Effects/RollWind.JSON") },
        { TEXT("TwinSpinWind"),           TEXT("../../Resources/YSH/Effects/TwinSpinWind.JSON") },
        { TEXT("PartnerWind"),            TEXT("../../Resources/YSH/Effects/PartnerWind.JSON") },
        { TEXT("WallImpact"),             TEXT("../../Resources/YSH/Effects/WallImpact.JSON") },

        //Leopard
        { TEXT("LeoSlash_L"),             TEXT("../../Resources/YSH/Effects/Leopard/LeoSlash_L.JSON") },
        { TEXT("LeoSlash_R"),             TEXT("../../Resources/YSH/Effects/Leopard/LeoSlash_R.JSON") },
        { TEXT("Leopard_Meteo"),          TEXT("../../Resources/YSH/Effects/Leopard/Leopard_Meteo.JSON") },
        { TEXT("Nail_Trail"),             TEXT("../../Resources/YSH/Effects/Leopard/Nail_Trail.JSON") },
        { TEXT("Afterimage_Assault"),     TEXT("../../Resources/YSH/Effects/Leopard/Afterimage_Assault.JSON") },
        { TEXT("Afterimage_Jump"),        TEXT("../../Resources/YSH/Effects/Leopard/Afterimage_Jump.JSON") },
        { TEXT("ClawAssault"),            TEXT("../../Resources/YSH/Effects/Leopard/ClawAssault.JSON") },
        { TEXT("ClawJump"),               TEXT("../../Resources/YSH/Effects/Leopard/ClawJump.JSON") },
        { TEXT("Leopard_Floor"),          TEXT("../../Resources/YSH/Effects/Leopard/Leopard_Floor.JSON") },
        { TEXT("Leopard_Flash_R"),        TEXT("../../Resources/YSH/Effects/Leopard/Leopard_Flash_R.JSON") },
        { TEXT("Leopard_Flash_L"),        TEXT("../../Resources/YSH/Effects/Leopard/Leopard_Flash_L.JSON") },
        { TEXT("Leopard_Impact"),         TEXT("../../Resources/YSH/Effects/Leopard/Leopard_Impact.JSON") },
        { TEXT("Assault_Smoke"),          TEXT("../../Resources/YSH/Effects/Leopard/Assault_Smoke.JSON") },
        { TEXT("Nail_Smoke"),             TEXT("../../Resources/YSH/Effects/Leopard/Nail_Smoke.JSON") },
        { TEXT("LeoJump_Smoke"),          TEXT("../../Resources/YSH/Effects/Leopard/LeoJump_Smoke.JSON") },

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

        // «¡∑Œ≈‰≈∏¿‘ STATIC 1»∏ µÓ∑œ
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

    m_Epochs[pFx] = ++m_iEpochCounter;

    if (ppOut)
        *ppOut = pFx;

    if (pOutHandle)
        *pOutHandle = { pFx, m_iEpochCounter };

    return S_OK;
}

_bool CEffect_Loader::Is_Current(const FX_HANDLE& h) const
{
    if (nullptr == h.p)
        return false;

    auto it = m_Epochs.find(h.p);
    return it != m_Epochs.end() && it->second == h.iEpoch;
}

void CEffect_Loader::Free()
{
    m_Assets.clear();
    m_Epochs.clear();
    __super::Free();
}
