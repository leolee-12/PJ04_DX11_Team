#pragma once

#include "Effect_Container.h"

NS_BEGIN(Engine)

class CCoasterWind final : public CEffect_Container
{
    GENERATED_BODY(CCoasterWind)

public:
    struct COASTER_WIND_DESC : public CEffect_Container::EFFECT_CONTAINER_DESC
    {
    };

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_CoasterWind";

private:
    CCoasterWind(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CCoasterWind(const CCoasterWind& Prototype);
    virtual ~CCoasterWind() = default;

protected:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Priority_Update(_float fTimeDelta) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_EffectPartObjects();

public:
    static CCoasterWind* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

private:
    virtual void Free() override;
};

NS_END
