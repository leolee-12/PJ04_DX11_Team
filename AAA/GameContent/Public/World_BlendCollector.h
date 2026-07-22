#pragma once
#include "GameObject.h"
#include "BlendRenderable.h"
#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
class CModel;
NS_END

NS_BEGIN(Client)

class CWorld_BlendCollector final : public CGameObject
{
public:
    static constexpr const _tchar* PROTOTYPE_TAG = TEXT("Proto_World_BlendCollector");
    static constexpr const _tchar* LAYER_TAG = TEXT("Layer_World_BlendCollector");
    static constexpr const _tchar* OBJECT_TAG = TEXT("World_BlendCollector");

private:
    struct BLEND_SUBMIT_DATA
    {
        IBlendRenderable* pOwner = { nullptr };
        CGameObject* pRefOwner = { nullptr };
        _uint iMesh = { 0u };
        _float fViewDepth = { 0.f };
    };

private:
    CWorld_BlendCollector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CWorld_BlendCollector(const CWorld_BlendCollector& Prototype);
    virtual ~CWorld_BlendCollector() = default;

public:
    void Submit(IBlendRenderable* pOwner, CGameObject* pRefOwner, CModel* pModel, const _float4x4* pWorld, _uint iMesh);
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

    static CWorld_BlendCollector* Find(CGameInstance_Proxy* pProxy);

private:
    void BeginFrame_IfNeeded(_int64 iCurrentFrame);
    void Clear_Submissions();

private:
    vector<BLEND_SUBMIT_DATA> m_Submitted;

    _bool m_bRegisteredThisFrame = { false };
    _int64 m_iLastSubmitFrame = { 0 };

public:
    static CWorld_BlendCollector* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

private:
    virtual void Free() override;
};

NS_END