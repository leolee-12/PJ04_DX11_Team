#pragma once

#include "Base.h"
#include "Effect.h"

NS_BEGIN(Engine)

class CGameInstance_Proxy;

class CEffect_Manager final : public CBase
{
public:
    static constexpr const _tchar* EFFECT_LAYER_TAG = L"Layer_90_Effect";\

private:
    CEffect_Manager();
    virtual ~CEffect_Manager() = default;

public:
    HRESULT Initialize();
    HRESULT Spawn(_uint iLevel,
        const _wstring& strProtoTag,
        const CEffect::EFFECT_DESC& desc,
        CEffect** ppOut = nullptr);

private:
    CGameInstance_Proxy* m_pGameInstance_Proxy = { nullptr };

    _uint m_iSpawnCounter = { 0 };

public:
    static CEffect_Manager* Create();
    virtual void Free() override;
};

NS_END