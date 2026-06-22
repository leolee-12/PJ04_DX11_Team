#pragma once

#include "Effect_Container.h"

NS_BEGIN(Engine)

class CEffect_Part;

class CVacuumContainer final : public CEffect_Container
{
    GENERATED_BODY(CVacuumContainer)

public:
    struct VACUUM_CONTAINER_DESC : public CEffect_Container::EFFECT_CONTAINER_DESC
    {

    };

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_VacuumContainer";

private:
    CVacuumContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CVacuumContainer(const CVacuumContainer& Prototype);
    virtual ~CVacuumContainer() = default;

protected:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

public:
    void On_SuperInhale();
    void Off_SuperInhale();

private:
    HRESULT Ready_EffectPartObjects();

public:
    static CVacuumContainer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
private:
    virtual void Free() override;
};

NS_END