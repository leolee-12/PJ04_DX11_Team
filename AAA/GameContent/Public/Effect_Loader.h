#pragma once
#include "Base.h"
#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
class CEffect_Container;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CEffect_Loader final : public CBase
{
    DECLARE_SINGLETON(CEffect_Loader)

public:
    // 시작 시 1회: 매니페스트 읽어 STATIC 프로토 등록 + 작성값 캐시
    HRESULT Ready(const _tchar* strManifestPath,
        CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

    // 이펙트 스폰해줘
    HRESULT Spawn(const _wstring& strEffectId, _uint iTargetLevel,
        const _float3& vPos,
        const _float3& vLook = { 0.f, 0.f, 0.f },
        const _float3& vRotDeg = { 0.f, 0.f, 0.f },
        const _float4x4* pParent = nullptr,
        Engine::CEffect_Container** ppOut = nullptr);

private:
    CEffect_Loader() = default;
    virtual ~CEffect_Loader() = default;

    struct EFFECT_ASSET { _wstring strProtoTag; json Config; };
    unordered_map<_wstring, EFFECT_ASSET> m_Assets;     // effectId → asset
    CGameInstance_Proxy* m_pProxy{};

public:
    virtual void Free() override;
};

NS_END