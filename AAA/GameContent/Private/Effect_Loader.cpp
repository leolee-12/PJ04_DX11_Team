#include "Effect_Loader.h"
#include "GameInstance_Proxy.h"
#include "GameObject_Factory.h"
#include "Effect_Container.h"
#include "DataLoader.h"

IMPLEMENT_SINGLETON(CEffect_Loader)

namespace
{
    struct EFFECT_DB_ENTRY
    {
        const _tchar* szEffectId;    // 스폰 키
        const _tchar* szConfigPath;  // 튜닝 json (데이터로 유지)
    };

    // === 이펙트 DB : 새 이펙트는 여기에 한 줄 추가 ===
    static constexpr EFFECT_DB_ENTRY s_EffectDB[] =
    {
        { TEXT("WalkSmoke"),             TEXT("../../Resources/YSE/EffectContainer/WalkSmoke_6_19.json") },
        { TEXT("InhaleContainer"),       TEXT("../../Resources/YSE/EffectContainer/Inhale_6_24.json") },
        { TEXT("SwordSlash1"),           TEXT("../../Resources/YSE/EffectContainer/SwordSlash1_Alpha_Color.json") },
        { TEXT("SwordSlash3"),           TEXT("../../Resources/YSE/EffectContainer/SwordSlash3.json") },
        { TEXT("JumpSlash_1"),           TEXT("../../Resources/YSE/EffectContainer/JumpSlash_1.json") },
        { TEXT("SpinSlash"),             TEXT("../../Resources/YSE/EffectContainer/SpinSlash.json") },
        { TEXT("SpinSlashTrail"),        TEXT("../../Resources/YSE/EffectContainer/SpinSlashTrail.json") },
        { TEXT("SpinSlashTrail_Super"),  TEXT("../../Resources/YSE/EffectContainer/SpinSlashTrail_Super.json") },
        { TEXT("RockFloor"),             TEXT("../../Resources/YSH/Effects/Proto_RockBurst_0.json") },
        { TEXT("DeathSmoke"),            TEXT("../../Resources/YSH/Effects/Proto_DeathSmoke_0.json") },
    };
}

HRESULT CEffect_Loader::Ready(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
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
        if (!pProxy->Has_Prototype(ETOUI(LEVEL::STATIC), strProtoTag))
        {
            auto* pReg = CGameObject_Factory::GetInstance()->Get_Registration(strProtoTag);
            if (!pReg)
                continue;
            pReg->ResourceLoader(pProxy, pDevice, pContext, ETOUI(LEVEL::STATIC));
            pProxy->Add_Prototype(ETOUI(LEVEL::STATIC), strProtoTag.c_str(),
                pReg->CreatorFunc(pDevice, pContext));
        }

        m_Assets[strEffectId] = EFFECT_ASSET{ strProtoTag, std::move(jEffect) };
    }

    return S_OK;
}

HRESULT CEffect_Loader::Spawn(const _wstring& strEffectId, _uint iTargetLevel,
    const _float3& vPos, const _float3& vLook, const _float3& vRotDeg,
    const _float4x4* pParent, Engine::CEffect_Container** ppOut)
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

    if (ppOut)
        *ppOut = pFx;

    return S_OK;
}

void CEffect_Loader::Free()
{
    m_Assets.clear();
    __super::Free();
}
