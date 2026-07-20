#pragma once
#include "Projectile.h"

NS_BEGIN(Client)

class CProjectile_MoonShot final : public CProjectile
{
    GENERATED_BODY(CProjectile_MoonShot)
public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Projectile_MoonShot";
    static constexpr const _tchar* POOL_KEY = L"MetaMoonShot";

    static constexpr _float ARC_DEG = 120.f;       // 부채꼴 각도
    static constexpr _float MOON_H_OFFSET = 0.2f;
    static constexpr _float WAVE_START_R = 1.5f;   // 시작 반지름
    static constexpr _float WAVE_MAX_R = 25.f;     // 소멸 반지름
    static constexpr _float WAVE_SPEED = 18.f;     // 확산 속도

private:
    CProjectile_MoonShot(ID3D11Device*, ID3D11DeviceContext*);
    CProjectile_MoonShot(const CProjectile_MoonShot&);
    virtual ~CProjectile_MoonShot() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

public:
    virtual void Launch(const _float3& vPos, const _float3& vDir) override;
    virtual void On_Activated() override { m_fCurRadius = WAVE_START_R; }
    virtual void On_Impact() override {}

protected:
    virtual HRESULT Ready_Visual() override;
    virtual HRESULT Ready_HitBox() override;

private:
    _float m_fCurRadius = { WAVE_START_R };

public:
    static CProjectile_MoonShot* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END