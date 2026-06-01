#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CEffect abstract : public CGameObject
{
public:
    struct EFFECT_DESC : public CGameObject::GAMEOBJECT_DESC
    {
        _float4x4* pParentMatrix{};
    };

protected:
    CEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect(const CEffect& Prototype);
    virtual ~CEffect() = default;

protected:
    HRESULT Initialize_Prototype() override;
    HRESULT Initialize(void* pArg) override;

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;

protected:
    _float m_fTotlaTime{};
    _float m_fAccTime{};

    _float m_fPlaySpeed{};

    _bool m_bLoop{};
    _bool m_bIsFinished{};

    _float4x4* m_pParentMatrix{};

protected:
    virtual void Free() override;
};

NS_END