#pragma once
#include "Base.h"
#include "Engine_Macro.h"
#include "GameContent_Defines.h"


NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)
class CAbility_Bubble;

// 능력 버블 풀 (Proto 자체 등록)
// 소비자는 dev/ctx 만 넘기고 버블 proto/등록은 모른다.
class CLIENT_DLL CBubble_Manager final : public CBase
{
	DECLARE_SINGLETON(CBubble_Manager)

public:
    static constexpr const _tchar* BUBBLE_LAYER_TAG = L"Layer_AbilityBubble";
    enum class BUBBLE_KIND { ESSENCE, DROPPED };

private:
    struct POOL_KEY {
        _uint iLevel; _wstring strKey;
        bool operator==(const POOL_KEY& o) const
        {
            return iLevel == o.iLevel && strKey == o.strKey;
        }
    };
    struct POOL_KEY_HASH {
        size_t operator()(const POOL_KEY& k) const
        {
            return hash<_wstring>()(k.strKey) ^ (size_t(k.iLevel) * 131);
        }
    };

private:
    CBubble_Manager();
    virtual ~CBubble_Manager() = default;

public:
    // 생성 후 1회: 버블 proto STATIC 등록
    HRESULT                 Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    HRESULT                 Register_At_Static(const _tchar* szProtoTag, ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

    // 기본 Spawn - Launch 안할 때 사용
    HRESULT                 Spawn(_uint iTargetLevel, BUBBLE_KIND eKind, COPY_ABILITY_TYPE eAbility, const _float3& vPos, CAbility_Bubble** ppOut);

    // Dropped Bubble 발사할 때 방향 전달
    HRESULT                 Spawn(_uint iTargetLevel, BUBBLE_KIND eKind, COPY_ABILITY_TYPE eAbility, const _float3& vPos, const _float3& vDir, CAbility_Bubble** ppOut);

    void                    Return(_uint iLevel, const _wstring& strKey, CAbility_Bubble* pBubble);
    void                    Clear_Level(_uint iLevel);

private:
    const _tchar*           Proto_Of(BUBBLE_KIND eKind) const;
    const _tchar*           Model_Of(COPY_ABILITY_TYPE eAbility) const;
    _wstring                Make_Key(BUBBLE_KIND eKind, COPY_ABILITY_TYPE eAbility) const;

private:
    CGameInstance_Proxy*    m_pGameInstance_Proxy = { nullptr };

    _uint                   m_iSpawnCounter = { 0 };
    unordered_map<POOL_KEY, vector<CAbility_Bubble*>, POOL_KEY_HASH> m_Dormant;

public:
    virtual void            Free() override;
};

NS_END