#pragma once
#include "Component.h"

NS_BEGIN(physx)
class PxController;
NS_END

NS_BEGIN(Engine)
class CTransform;

class ENGINE_DLL CMovement : public CComponent
{
    GENERATED_BODY(CMovement)
    PROPERTY(_float, m_fMoveSpeed, L"MoveSpeed", L"Physics")
    PROPERTY(_float, m_fRotSpeedDeg, L"RotSpeed/deg", L"Physics")
    PROPERTY(_float, m_fGravity, L"Gravity", L"Physics")
    PROPERTY(_float, m_fJumpSpeed, L"JumpSpeed", L"Physics")
    PROPERTY(_float, m_fMaxFallSpeed, L"MaxFallSpeed", L"Physics")
    PROPERTY(_float, m_fAccel, L"Accel", L"Physics")   // 가속 (units/sec^2)
    PROPERTY(_float, m_fDecel, L"Decel", L"Physics")   // 감

protected:
    CMovement(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CMovement(const CMovement& Prototype);
    virtual ~CMovement() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    void            Set_Refs(CTransform* pTransform, physx::PxController* pController);
    void            Set_Stats(_float fMoveSpeed, _float fRotSpeedDeg, _float fGravity, _float fJumpSpeed);
    void            Set_Acceleration(_float Accel, _float Decel);
    void            Set_MoveSpeed(_float fMoveSpeed) { m_fMoveSpeed = fMoveSpeed; }
    _float          Get_MoveSpeed() const { return m_fMoveSpeed; }
    _float          Get_VerticalVelocity() { return m_fVerticalVelocity; }
    _bool           Move(_fvector vWishDir, _float fTimeDelta);
    void            Jump();
    _bool           Is_Grounded() const { return m_bGrounded; }

    virtual void    Sync_To_Controller();

protected:
    virtual void Calc_Vertical(_float fTimeDelta);
    virtual void Apply_Facing(_fvector vFaceDir, _float fTimeDelta); // 기본 : 이동방향을 바라보게 회전

protected:
    CTransform* m_pTransform = { nullptr };
    physx::PxController* m_pController = { nullptr };

    _float m_fVerticalVelocity = { 0.f };
    _bool  m_bGrounded = { false };

    _float3 m_vHorizVel = { 0.f, 0.f, 0.f };

private:
    _bool  m_bJumpQueued = { false };

public:
    static CMovement* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END