#pragma once

#include "Launcher_Defines.h"
#include "Level.h"

// 아레나 전용 로딩 레벨.
// 어제 만든 커튼/스탬프 전환 UI를 로딩 화면으로 띄우면서
// 백그라운드로 아레나(ARENA) 리소스를 로드하고, 완료되면 CArena 로 전환한다.

NS_BEGIN(Client)

class CLoader;

class CLevel_ArenaLoading final : public CLevel
{
private:
    CLevel_ArenaLoading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~CLevel_ArenaLoading() = default;

public:
    virtual HRESULT Initialize();
    virtual void    Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;

private:
    static constexpr _float MIN_DISPLAY_TIME = 3.f;

    CLoader* m_pLoader = { nullptr };
    _float   m_fElapsed = { 0.f };
    _bool    m_bTransitioned = { false };

private:
    virtual HRESULT Ready_Events() override { return S_OK; }
    HRESULT Load_LoadingUI();

public:
    static CLevel_ArenaLoading* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual void Free() override;
};

NS_END