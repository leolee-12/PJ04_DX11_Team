#pragma once

#include "Base.h"
#include "Effect.h"

NS_BEGIN(Engine)

class CGameInstance_Proxy;
class CShader;

class CEffect_Manager final : public CBase
{
public:
    static constexpr const _tchar* EFFECT_LAYER_TAG = L"Layer_90_Effect";

private:
    CEffect_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~CEffect_Manager() = default;

public:
    HRESULT Initialize();
    HRESULT Spawn(_uint iLevel,
        const _wstring& strProtoTag,
        const CEffect::EFFECT_DESC& desc,
        CEffect** ppOut = nullptr);

    CShader* Get_2DShader() { return m_p2DShader; }
    CShader* Get_MeshShader() { return m_pMeshShader; }

private:
    ID3D11Device* m_pDevice = { nullptr };
    ID3D11DeviceContext* m_pContext = { nullptr };
    CGameInstance_Proxy* m_pGameInstance_Proxy = { nullptr };

    CShader* m_p2DShader = { nullptr };
    CShader* m_pMeshShader = { nullptr };


    _uint m_iSpawnCounter = { 0 };

public:
    static CEffect_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual void Free() override;
};

NS_END