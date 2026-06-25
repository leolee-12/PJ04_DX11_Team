#pragma once
#include "Base.h"
#include "Effect_Container.h"

NS_BEGIN(Engine)

class CGameInstance_Proxy;
class CShader;

class CEffect_Manager final : public CBase
{
public:
    static constexpr const _tchar* EFFECT_LAYER_TAG = L"Layer_Effect";

private:
    struct POOL_KEY {
        _uint    iLevel;
        _wstring strProtoTag;
        bool operator==(const POOL_KEY& o) const
        {
            return iLevel == o.iLevel && strProtoTag == o.strProtoTag;
        }
    };
    struct POOL_KEY_HASH {
        size_t operator()(const POOL_KEY& k) const
        {
            return std::hash<_wstring>()(k.strProtoTag) ^ (size_t(k.iLevel) * 131);
        }
    };

private:
    CEffect_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~CEffect_Manager() = default;

public:
    HRESULT Initialize();
    HRESULT Spawn(_uint iLevel,
        const _wstring& strEffectKey,          
        const _wstring& strProtoTag,           
        const CEffect_Container::EFFECT_CONTAINER_DESC& desc,
        const json* pConfig = nullptr,      
        CEffect_Container** ppOut = nullptr);

    void    Return(_uint iLevel, const _wstring& strEffectKey, CEffect_Container* pEffect);
    void    Clear_Level(_uint iLevel);

    void    Set_PrototypeLevel(_uint iLevel) { m_iProtoLevel = iLevel; }

    CShader* Get_2DShader() { return m_p2DShader; }
    CShader* Get_MeshShader() { return m_pMeshShader; }

private:
    ID3D11Device* m_pDevice = { nullptr };
    ID3D11DeviceContext* m_pContext = { nullptr };
    CGameInstance_Proxy* m_pGameInstance_Proxy = { nullptr };

    CShader* m_p2DShader = { nullptr };
    CShader* m_pMeshShader = { nullptr };

    _uint m_iSpawnCounter = { 0 };

    _uint m_iProtoLevel = { 0 };
    unordered_map<POOL_KEY, vector<CEffect_Container*>, POOL_KEY_HASH> m_Dormant;

public:
    static CEffect_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual void Free() override;
};

NS_END

//CEffect_Container* pFx = nullptr;
//pGI->Spawn_Effect(iLevel, L"Proto_InhaleContainer", desc, &pFx); // 휴면 재사용 or 신규
//if (pFx) pFx->EffectContainer_Start(vPos, vLook, pParentMat);    // m_bIsPlay=true + accTime=0 + 파트 Effect_Start = 초기값 리셋