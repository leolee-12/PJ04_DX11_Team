#pragma once

#include "Effect_Container.h"

NS_BEGIN(Engine)

class CSplit_Trash final : public CEffect_Container
{
    GENERATED_BODY(CSplit_Trash)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Split_Trash";

private:
    CSplit_Trash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CSplit_Trash(const CSplit_Trash& Prototype);
    virtual ~CSplit_Trash() = default;

protected:
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_EffectPartObjects();

public:
    static CSplit_Trash* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
};

NS_END