#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CEffect abstract : public CGameObject
{
public:
    struct EFFECT_DESC : public CGameObject::GAMEOBJECT_DESC
    {
        _float3 vPosition = {};
        _float3 vForward = { 0.f, 0.f, 1.f };
        _float3 vScale = { 1.f, 1.f, 1.f };
        _float  fLifeTime = 1.f;           // <= 0 이면 수동 종료

        const _float4x4* pAttachBone = nullptr;
        CGameObject* pAttachOwner = nullptr;
        _float3          vAttachOffset = {};
    };

protected:
    CEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect(const CEffect& Prototype);
    virtual ~CEffect() = default;

public:
    virtual HRESULT Initialize_Prototype() override { return S_OK; }
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Priority_Update(_float fTimeDelta) override {}
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override = 0;

protected:
    virtual void On_Spawn() {}
    virtual void On_Tick(_float fTimeDelta) {}
    virtual void On_Expire() {}

protected:
    _float           m_fAge = 0.f;
    _float           m_fLifeTime = 1.f;
    _float3          m_vForward = { 0.f, 0.f, 1.f };
    CGameObject* m_pAttachOwner = nullptr;   // AddRef
    const _float4x4* m_pAttachBone = nullptr;
    _float3          m_vAttachOffset = {};
    _bool            m_bExpireFired = false;

public:
    virtual void Free() override;
};

NS_END