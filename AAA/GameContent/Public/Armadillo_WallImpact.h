#pragma once

#include "Effect_Container.h"

NS_BEGIN(Engine)

class CEffect_Part;

class CArmadillo_WallImpact final : public CEffect_Container
{
    GENERATED_BODY(CArmadillo_WallImpact)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Armadillo_WallImpact";

private:
    CArmadillo_WallImpact(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CArmadillo_WallImpact(const CArmadillo_WallImpact& Prototype);
    virtual ~CArmadillo_WallImpact() = default;

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
    static CArmadillo_WallImpact* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
private:
    virtual void Free() override;
};

NS_END