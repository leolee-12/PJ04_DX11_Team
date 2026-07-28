#pragma once

#include "EnvObject.h"

NS_BEGIN(Client)

class CEnvObject_Interact : public CEnvObject
{
public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvObject_Interact";

protected:
    CEnvObject_Interact(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEnvObject_Interact(const CEnvObject_Interact& Prototype);
    virtual ~CEnvObject_Interact() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;

protected:
    virtual HRESULT Ready_InteractComponents();

private:
    void Snap_ToGround();

public:
    static CEnvObject_Interact* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END