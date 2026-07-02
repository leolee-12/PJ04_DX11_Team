#pragma once

#include "Effect_Container.h"

NS_BEGIN(Engine)

class CEffect_Part;

class CGetAbilityEffect final : public CEffect_Container
{
    GENERATED_BODY(CGetAbilityEffect)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_GetAbilityEffect";

private:
    CGetAbilityEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CGetAbilityEffect(const CGetAbilityEffect& Prototype);
    virtual ~CGetAbilityEffect() = default;

protected:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_EffectPartObjects();

public:
    static CGetAbilityEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
private:
    virtual void Free() override;
};

NS_END