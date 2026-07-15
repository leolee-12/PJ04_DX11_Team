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

    HRESULT                 Spawn(_uint iTargetLevel, _fvector vPos, _float fDelay = 0.f, CDropStar** ppOut = nullptr);

    void                    Return(_uint iLevel, const _wstring& strKey, CDropStar* pStar);
    void                    Clear_Level(_uint iLevel);

private:
    CGameInstance_Proxy*    m_pGameInstance_Proxy = { nullptr };
    _uint                   m_iSpawnCounter = { 0 };

    unordered_map<POOL_KEY, vector<CDropStar*>, POOL_KEY_HASH> m_Dormant;


public:
    virtual void Free() override;
};

NS_END