#pragma once

#include "Effect_Container.h"

NS_BEGIN(Engine)

class CSplit_Cage final : public CEffect_Container
{
    GENERATED_BODY(CSplit_Cage)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Split_Cage";

private:
    CSplit_Cage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CSplit_Cage(const CSplit_Cage& Prototype);
    virtual ~CSplit_Cage() = default;

protected:
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_EffectPartObjects();

public:
    static CSplit_Cage* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
};

NS_END
