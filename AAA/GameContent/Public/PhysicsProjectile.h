#pragma once
#include "Projectile.h"

NS_BEGIN(Engine) class CController; NS_END
NS_BEGIN(Client)
class CProjectile_Movement;

// CCT 바운스 투사체 공통 부모. 비행/바운스/수명은 여기, 종료 연출(파괴·폭발)은 자식.
class CLIENT_DLL CPhysicsProjectile abstract : public CProjectile
{
    GENERATED_BODY_ABSTRACT(CPhysicsProjectile)

protected:
    CPhysicsProjectile(ID3D11Device*, ID3D11DeviceContext*);
    CPhysicsProjectile(const CPhysicsProjectile&);
    virtual ~CPhysicsProjectile() = default;

public:
    virtual void Update(_float fTimeDelta) override;                 // 비행/종료 디스패치 + 히트박스 추적
    virtual void Launch(const _float3& vPos, const _float3& vDir) override;

protected:
    virtual HRESULT Ready_Movement() override;       // CCT + 무브먼트 생성/연결
    virtual void    Kill() override;                 // CCT/무브먼트 정지 후 베이스 Kill

    // 자식 훅
    virtual void On_Bounce(_int iCount) {}           // 바닥 바운스: 카운트 정책(파괴/폭발 트리거)
    virtual void Update_Terminal(_float dt) {}       // 비행 종료 후 진행(파괴 애님/폭발)
    virtual void Tick_Visual(_float dt) {}           // 매 프레임 비주얼(애니메이터 등)
    virtual void On_Launched() {}                    // 발사직후 호출될 무언가

    void    Stop_Flying() { m_bFlying = false; }     // 자식이 종료 진입 시 호출

protected:
    Engine::CController* m_pController = { nullptr };
    CProjectile_Movement* m_pMovement = { nullptr };
    _int    m_iBounceCount = { 0 };
    _bool   m_bFlying = { true };
};
NS_END