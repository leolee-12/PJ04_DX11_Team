#pragma once

#include "GameContent_Defines.h"
#include "Movement.h"

NS_BEGIN(Client)

class CMovement_Child final : public CMovement
{
    GENERATED_BODY(CMovement_Child)

    PROPERTY(_float,    m_fMass,                    L"질량",                 L"RigidBody")
                                                                             
    PROPERTY(_bool,     m_bUseGravity,              L"중력 적용",            L"RigidBody")
    PROPERTY(_float,    m_fRBGravity,               L"중력 (-)",             L"RigidBody")
    PROPERTY(_float,    m_fGravityScale,            L"중력 비율",            L"RigidBody")
                                                                             
    PROPERTY(_float,    m_fLinearDrag,              L"공기 저항 (+)",        L"RigidBody")

    PROPERTY(_bool,     m_bUseGroundFriction,       L"바닥 마찰력 적용",     L"RigidBody")
    PROPERTY(_float,    m_fGroundFriction,          L"바닥 마찰력 (+)",      L"RigidBody")

    PROPERTY(_float,    m_fMaxHorizontalSpeed,      L"최대 수평 속도 (+)",   L"RigidBody")
    PROPERTY(_float,    m_fMaxFallVelocity,         L"최대 낙하 속도 (-)",   L"RigidBody")
    PROPERTY(_float,    m_fJumpVelocity,            L"순간 점프 속도 (+)",   L"RigidBody")

    PROPERTY(_float,    m_fRotation_Speed_Degree,   L"회전 속도 (Degree)",    L"RigidBody")

    PROPERTY(_float,    m_fMaxCoyoteTime,           L"Coyote Time",           L"RigidBody")

    // 접지 관련
    PROPERTY(_float, m_fGroundPermitDistance,       L"GroundPermitDistance",    L"RigidBody")
    PROPERTY(_float, m_RayOriginOffsetFromFoot,     L"RayOriginOffsetFromFoot", L"RigidBody")


public:
    enum class FORCE_MODE
    {
        FORCE,           // F = m * a. dt 적용됨. 질량 영향 받음.
        ACCELERATION,    // a. dt 적용됨. 질량 영향 안 받음.
        IMPULSE,         // 순간 충격량. 질량 영향 받음.
        VELOCITY_CHANGE  // 순간 속도 변화. 질량 영향 안 받음.
    };

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Prototype_Component_Movement_Child";

private:
    CMovement_Child(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CMovement_Child(const CMovement_Child& Prototype);
    virtual ~CMovement_Child() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    // Rigidbody 메인 업데이트. 외부 입력 방향을 인자로 받지 않는다.
    _bool Update_RigidBody(_float fTimeDelta);

    _bool Check_GroundBelow();

    // Unity Rigidbody.AddForce 느낌.
    void Add_Force(_fvector vValue, FORCE_MODE eMode = FORCE_MODE::FORCE);
    void Add_Acceleration(_fvector vAccel);
    void Add_Impulse(_fvector vImpulse);
    void Add_VelocityChange(_fvector vDeltaVelocity);

    // 캐릭터용 편의 함수. 내부적으로는 velocity/impulse만 건드린다.
    _bool Try_Jump();
    _bool Try_Jump(_float fJumpVelocity);
    void  Force_Jump();
    void  Force_Jump(_float fJumpVelocity);

    // Velocity 제어.
    void Set_Velocity(_fvector vVelocity);
    void Set_VelocityX(_float fX);
    void Set_VelocityY(_float fY);
    void Set_VelocityZ(_float fZ);
    void Add_Velocity(_fvector vDeltaVelocity);

    _float3 Get_Velocity() const { return m_vVelocity; }
    _float  Get_Speed() const;
    _float  Get_HorizontalSpeed() const;

    // 설정.
    void Set_Mass(_float fMass);
    void Set_Gravity(_float fGravity);
    void Set_GravityScale(_float fGravityScale);
    void Set_LinearDrag(_float fLinearDrag);
    void Set_GroundFriction(_float fGroundFriction);
    void Set_MaxHorizontalSpeed(_float fMaxHorizontalSpeed);
    void Set_MaxFallVelocity(_float fMaxFallVelocity);
    void Set_JumpVelocity(_float fJumpVelocity);
    void Set_UseGravity(_bool bUseGravity);
    void Set_UseGroundFriction(_bool bUseGroundFriction);
    void Set_StopHorizontalOnSideHit(_bool bStop) { m_bStopHorizontalOnSideHit = bStop; }

    // 정지 / 초기화.
    void Stop();
    void Stop_Horizontal();
    void Stop_Vertical();
    void Clear_Forces();

    // 텔레포트 / 리스폰 / 시작 위치 변경 후 필요.
    // Engine의 CMovement::Sync_To_Controller()에 virtual을 붙일 수 있으면 override로 사용.
    virtual void Sync_To_Controller() override;

    // 회전
    void Rotate_To_Direction(_fvector vDir, _float fTimeDelta);

private:
    void Integrate_Forces(_float fTimeDelta, _vector& vVelocity);
    void Apply_Drag(_float fTimeDelta, _vector& vVelocity);
    void Clamp_Velocity(_vector& vVelocity);
    void Move_Controller(_fvector vVelocity, _float fTimeDelta, _vector& vOutVelocity);
    void Sync_BaseVelocityFields();

    void Update_CoyoteTimer(_float fDeltaTime);

private:
    _float3 m_vVelocity = { 0.f, 0.f, 0.f };
    _float3 m_vForce = { 0.f, 0.f, 0.f };
    _float3 m_vAcceleration = { 0.f, 0.f, 0.f };

    _bool  m_bStopHorizontalOnSideHit = { false };

private:
    _float m_fAccCoyoteTime{};

public:
    static CMovement_Child* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
private:
    virtual void Free() override;
};

NS_END
