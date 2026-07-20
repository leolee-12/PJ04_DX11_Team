#pragma once
#include "Projectile.h"

NS_BEGIN(Client)

class CProjectile_MoonShot final : public CProjectile
{
    GENERATED_BODY(CProjectile_MoonShot)
public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Projectile_MoonShot";
    static constexpr const _tchar* POOL_KEY = L"MetaMoonShot";

private:
    CProjectile_MoonShot(ID3D11Device*, ID3D11DeviceContext*);
    CProjectile_MoonShot(const CProjectile_MoonShot&);
    virtual ~CProjectile_MoonShot() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    virtual HRESULT Ready_Visual() override;

public:
    static CProjectile_MoonShot* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END