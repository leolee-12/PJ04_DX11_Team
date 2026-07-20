#pragma once
#include "Component.h"

NS_BEGIN(physx)
class PxRigidDynamic;
NS_END

NS_BEGIN(Engine)
class CTransform;

class ENGINE_DLL CRigidBody : public CComponent
{
    GENERATED_BODY_ABSTRACT(CRigidBody)
    PROPERTY(_bool, m_bKinematic, L"Kinematic", L"Physics")

protected:
    CRigidBody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CRigidBody(const CRigidBody& Prototype);
    virtual ~CRigidBody() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    // 외부(매니저/게임)에서 만든 body를 컴포넌트가 소유. 기존 body 있으면 교체.
    void  Set_Body(CTransform* pTransform, physx::PxRigidDynamic* pBody);

    void  Sync_From_Body();   // 매 프레임: 시뮬 결과 포즈 → Transform
    void  Sync_To_Body();     // 텔레포트: Transform → body (스폰/리셋 시)

    void  Add_Force(_fvector vForce);     // 지속력 (질량 영향 O)
    void  Add_Impulse(_fvector vImpulse); // 순간 충격
    _vector Get_LinearVelocity() const;
    void  Set_LinearVelocity(_fvector vVel);
    void  Set_Enabled(_bool bEnabled);
    void  Set_SceneQueryEnabled(_bool bEnabled);
    void  Set_Kinematic(_bool bKinematic);
    void  WakeUp();

    physx::PxRigidDynamic* Get_Body() const { return m_pBody; }

protected:
    CTransform* m_pTransform = { nullptr };
    physx::PxRigidDynamic* m_pBody = { nullptr };

public:
    static CRigidBody* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END