#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CGorilla_Swing final : public CEffect_Container
{
    GENERATED_BODY(CGorilla_Swing)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_GorillaSwing";

private:
    CGorilla_Swing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CGorilla_Swing(const CGorilla_Swing& Prototype);
    virtual ~CGorilla_Swing() = default;

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
    HRESULT Ready_EffectPartObjects();

public:
    static CGorilla_Swing* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
private:
    virtual void Free() override;
};

NS_END