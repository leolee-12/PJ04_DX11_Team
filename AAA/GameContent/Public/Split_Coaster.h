#pragma once

#include "Effect_Container.h"

NS_BEGIN(Engine)

class CSplit_Coaster final : public CEffect_Container
{
    GENERATED_BODY(CSplit_Coaster)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Split_Coaster";

private:
    CSplit_Coaster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CSplit_Coaster(const CSplit_Coaster& Prototype);
    virtual ~CSplit_Coaster() = default;

protected:
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_EffectPartObjects();

public:
    static CSplit_Coaster* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
};

NS_END
