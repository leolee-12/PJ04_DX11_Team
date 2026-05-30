#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CEffect_Part abstract : public CGameObject
{
public:
    struct EFFECT_PART_DESC : public CGameObject::GAMEOBJECT_DESC
    {
        _float fStartRatio{};
        _float fEndRatio{};

        _float3 vLocalPos;
    };

protected:
    CEffect_Part(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect_Part(const CEffect_Part& Prototype);
    virtual ~CEffect_Part() = default;

protected:
    HRESULT Initialize_Prototype() override;
    HRESULT Initialize(void* pArg) override;

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;

protected:
    _float m_fAccTime{};
    _float m_fDelayTime{};
    _float m_fDuration{ 1.f };

    _bool m_bIsPlay{};
    _bool m_bActive{};
    _bool m_bLoop{};

protected:
    virtual void Free() override;
};

NS_END