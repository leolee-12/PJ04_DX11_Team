#pragma once
#include "Base.h"
#include "Engine_Macro.h"
#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)
class CDropStar;
class CLIENT_DLL CDropStar_Manager final : public CBase
{
	DECLARE_SINGLETON(CDropStar_Manager)

public:
	static constexpr const _tchar* DROPSTAR_LAYER_TAG = L"Layer_DropStar";

    enum class STAR_SPAWN_TYPE { SWEEP, CIRCLE };
    enum class STAR_SPAWN_PRESET 
    { 
        // GORILLA 
        GORILLA_ARM_SWEEP_RIGHT, 
        GORILLA_ARM_SWEEP_LEFT,
        AFTER_GORILLA_ARM_SPIN,
        ROCK_IMPACT,
        PRESET_END
    };

    struct STAR_SPAWN_DESC {
        STAR_SPAWN_TYPE     eType = STAR_SPAWN_TYPE::SWEEP;
        _uint               iCount = { 6 };
        _float              fLifeTime = { 3.f };         //  별들 수명시간 
        _float              fDelayStart = { 0.f };
        _float              fDelayStep = { 0.08f };      // SWEEP 개당 램프
        _float              fJitter = { 0.5f };
        _float3             vLocalOffset = {};          // 캐스터 로컬 시작점
        _float              fRange = { 8.f };           // SWEEP : 밴드(호)의 길이 / CIRCLE :  반경
        _float              fLaunchSpeed = { 0.f };     // 0 : 제자리 / > 0 바깥방향 속도
        // SWEEP 전용
        _float              fStartDeg = { 0.f };        // Look 기준 시작 오프셋 (0을 주면 Look 축에서 시작 / 0이 아니라면 회전(회전방향 적용) 시킨 위치에서 시작)
        _float              fSweepDeg = { 90.f };       // 총 회전량
    };

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
    CDropStar_Manager();
    virtual ~CDropStar_Manager() = default;

public:
    HRESULT                 Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    HRESULT                 Register_At_Static(const _tchar* szProtoTag, ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

    HRESULT                 Spawn(_uint iTargetLevel, const _float3& vPos, const _float3& vLook = { 0.f, 0.f, 0.f }, const _float3& vDir = { 0.f, 0.f, 0.f }, _float fDelay = 0.f, _float fLife = 3.f, CDropStar * *ppOut = nullptr);
    HRESULT                 Spawn_Pattern(_uint iLayerLevel, _fmatrix matCaster, const STAR_SPAWN_DESC& Desc);
    HRESULT                 Spawn_Preset(_uint iLayerLevel, _fmatrix matCaster, const _wstring& strPreset, const _float3& vOffset = {});

    STAR_SPAWN_DESC         Get_Preset(STAR_SPAWN_PRESET ePreset) const;
    void                    Return(_uint iLevel, const _wstring& strKey, CDropStar* pStar);
    void                    Clear_Level(_uint iLevel);

private:
    _vector                 Compute_StarPos(_fvector vCenter, _fvector vLook, const CDropStar_Manager::STAR_SPAWN_DESC& Desc, _uint iIndex);
    STAR_SPAWN_PRESET       Name_To_Preset(const _wstring& strName) const;

private:
    CGameInstance_Proxy*    m_pGameInstance_Proxy = { nullptr };
    _uint                   m_iSpawnCounter = { 0 };

    unordered_map<POOL_KEY, vector<CDropStar*>, POOL_KEY_HASH> m_Dormant;


public:
    virtual void Free() override;
};

NS_END