#include "Effect_Loader.h"
#include "GameInstance_Proxy.h"
#include "GameObject_Factory.h"
#include "Effect_Container.h"
#include "DataLoader.h"

IMPLEMENT_SINGLETON(CEffect_Loader)

HRESULT CEffect_Loader::Ready(const _tchar* strManifestPath,
    CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    m_pProxy = pProxy;

    string strManifest;
    if (FAILED(CDataLoader::Read_Json(strManifestPath, &strManifest))) return E_FAIL;

    json jManifest = json::parse(strManifest);
    if (!jManifest.contains("Effects")) return E_FAIL;

    for (auto& [strId, jPath] : jManifest["Effects"].items())
    {
        const _wstring strEffectId = StrToWstr(strId);
        const _wstring strJsonPath = StrToWstr(jPath.get<string>());

        string strContent;
        if (FAILED(CDataLoader::Read_Json(strJsonPath.c_str(), &strContent))) continue;
        json jEffect = json::parse(strContent);
        if (!jEffect.contains("Prototype_Tag")) continue;

        const _wstring strProtoTag = StrToWstr(jEffect["Prototype_Tag"].get<string>());

        // 프로토 STATIC 1회 등록 (팩토리 경유)
        if (!pProxy->Has_Prototype(ETOUI(LEVEL::STATIC), strProtoTag))
        {
            auto* pReg = CGameObject_Factory::GetInstance()->Get_Registration(strProtoTag);
            if (!pReg) continue;
            pReg->ResourceLoader(pProxy, pDevice, pContext);                 // 파트/컴포넌트(2단계 STATIC)
            pProxy->Add_Prototype(ETOUI(LEVEL::STATIC), strProtoTag.c_str(),
                pReg->CreatorFunc(pDevice, pContext));       // 메인 컨테이너 프로토 STATIC
        }

        m_Assets[strEffectId] = EFFECT_ASSET{ strProtoTag, std::move(jEffect) };
    }
    return S_OK;
}

HRESULT CEffect_Loader::Spawn(const _wstring& strEffectId, _uint iTargetLevel,
    const _float3& vPos, const _float3& vLook, const _float4x4* pParent,
    Engine::CEffect_Container** ppOut)
{
    auto it = m_Assets.find(strEffectId);
    if (it == m_Assets.end()) return E_FAIL;
    auto& asset = it->second;

    Engine::CEffect_Container::EFFECT_CONTAINER_DESC desc{};
    Engine::CEffect_Container* pFx = nullptr;

    if (FAILED(m_pProxy->Spawn_Effect(iTargetLevel, strEffectId, asset.strProtoTag,
        desc, &asset.Config, &pFx)) || !pFx)
        return E_FAIL;

    pFx->EffectContainer_Start(vPos, vLook, pParent);
    if (ppOut) *ppOut = pFx;
    return S_OK;
}

void CEffect_Loader::Free()
{
    m_Assets.clear();
    __super::Free();
}
