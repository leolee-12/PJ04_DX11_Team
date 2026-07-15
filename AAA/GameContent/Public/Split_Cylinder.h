#pragma once

#include "Effect_Container.h"

NS_BEGIN(Engine)

class CEffect_Part;

class CSplit_Cylinder final : public CEffect_Container
{
    GENERATED_BODY(CSplit_Cylinder)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Split_Cylinder";

private:
    CSplit_Cylinder(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CSplit_Cylinder(const CSplit_Cylinder& Prototype);
    virtual ~CSplit_Cylinder() = default;

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
    static CSplit_Cylinder* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
private:
    virtual void Free() override;
};

NS_END
