#pragma once

#include "Effect_Part.h"

class CEffect_Quad abstract : public CEffect_Part
{
public:
    struct EFFECT_QUAD_DESC : public CGameObject::GAMEOBJECT_DESC
    {

    };

protected:
    CEffect_Quad(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect_Quad(const CEffect_Quad& Prototype);
    virtual ~CEffect_Quad() = default;

protected:
    HRESULT Initialize_Prototype() override;
    HRESULT Initialize(void* pArg) override;

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;

protected:


protected:
    virtual void Free() override;
};

