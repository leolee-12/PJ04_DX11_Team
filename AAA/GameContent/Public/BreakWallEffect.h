#pragma once

#include "Effect_Container.h"

NS_BEGIN(Engine)

class CBreakWallEffect final : public CEffect_Container
{
    GENERATED_BODY(CBreakWallEffect)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_BreakWallEffect";

private:
    CBreakWallEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBreakWallEffect(const CBreakWallEffect& Prototype);
    virtual ~CBreakWallEffect() = default;

protected:
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_EffectPartObjects();

public:
    static CBreakWallEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
};

NS_END