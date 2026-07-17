#pragma once
#include "Effect_Container.h"

NS_BEGIN(Client)

class CNail_Trail final : public CEffect_Container
{
    GENERATED_BODY(CNail_Trail)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Nail_Trail";

private:
    CNail_Trail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CNail_Trail(const CNail_Trail& Prototype);
    virtual ~CNail_Trail() = default;

protected:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

private:
    HRESULT Ready_EffectPartObjects();

public:
    static CNail_Trail* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

private:
    virtual void Free() override;
};

NS_END