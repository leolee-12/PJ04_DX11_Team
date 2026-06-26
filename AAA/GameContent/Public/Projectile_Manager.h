#pragma once
#include "Base.h"
#include "Engine_Macro.h"
#include "GameContent_Defines.h"

NS_BEGIN(Engine) class CGameInstance_Proxy; NS_END
NS_BEGIN(Client)
class CProjectile;

// 전역 투사체 풀. 엔진 싱글톤 매크로 사용(CEffect_Allocator 와 동일 패턴).
// 키 = {level, key}. 휴면은 raw ptr(레이어가 ref 소유), Clear_Level 로 dangling 방지.
class CLIENT_DLL CProjectile_Manager final : public CBase
{
    DECLARE_SINGLETON(CProjectile_Manager)   // GetInstance / DestroyInstance 제공

public:
    static constexpr const _tchar* PROJECTILE_LAYER_TAG = L"Layer_Projectile";

    // 휴면 있으면 pop, 없으면 STATIC 프로토에서 Clone. 발사는 호출측이 Launch().
    HRESULT  Spawn(_uint iLevel, const _wstring& strKey, const _wstring& strProtoTag,
        CProjectile** ppOut);
    void     Return(_uint iLevel, const _wstring& strKey, CProjectile* pProj);
    void     Clear_Level(_uint iLevel);      // 레벨 언로드 시 호출 필수

    void     Set_PrototypeLevel(_uint iLevel) { m_iProtoLevel = iLevel; }

private:
    CProjectile_Manager();                   // 매크로 GetInstance 가 new 로 호출(인자 없음)
    virtual ~CProjectile_Manager() = default;

    struct POOL_KEY {
        _uint iLevel; _wstring strKey;
        bool operator==(const POOL_KEY& o) const { return iLevel == o.iLevel && strKey == o.strKey; }
    };
    struct POOL_KEY_HASH {
        size_t operator()(const POOL_KEY& k) const {
            return std::hash<_wstring>()(k.strKey) ^ (size_t(k.iLevel) * 131);
        }
    };

    Engine::CGameInstance_Proxy* m_pGameInstance_Proxy = { nullptr };
    _uint m_iProtoLevel = { 0 };
    _uint m_iSpawnCounter = { 0 };
    unordered_map<POOL_KEY, vector<CProjectile*>, POOL_KEY_HASH> m_Dormant;

public:
    virtual void Free() override;            // CBase: DestroyInstance 가 Release -> Free
};
NS_END