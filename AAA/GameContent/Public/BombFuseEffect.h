#pragma once

#include "Effect_Container.h"

NS_BEGIN(Engine)

class CEffect_Part;

class CBombFuseEffect final : public CEffect_Container
{
    GENERATED_BODY(CBombFuseEffect)

public:
    struct BOMB_FUSE_DESC : public CEffect_Container::EFFECT_CONTAINER_DESC
    {
    };

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_BombFuseEffect";

private:
    CBombFuseEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBombFuseEffect(const CBombFuseEffect& Prototype);
    virtual ~CBombFuseEffect() = default;

protected:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT         Ready_EffectPartObjects();

public:
    static CBombFuseEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

private:
    virtual void Free() override;
};

NS_END