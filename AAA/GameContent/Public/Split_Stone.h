#pragma once

#include "Effect_Container.h"

NS_BEGIN(Engine)

class CSplit_Stone final : public CEffect_Container
{
    GENERATED_BODY(CSplit_Stone)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Split_Stone";

private:
    CSplit_Stone(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CSplit_Stone(const CSplit_Stone& Prototype);
    virtual ~CSplit_Stone() = default;

protected:
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_EffectPartObjects();

public:
    static CSplit_Stone* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
};

NS_END