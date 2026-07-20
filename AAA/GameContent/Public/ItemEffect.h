#pragma once

#include "Effect_Container.h"

NS_BEGIN(Engine)

class CItemEffect final : public CEffect_Container
{
    GENERATED_BODY(CItemEffect)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_ItemEffect";

private:
    CItemEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CItemEffect(const CItemEffect& Prototype);
    virtual ~CItemEffect() = default;

protected:
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_EffectPartObjects();

public:
    static CItemEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
};

NS_END