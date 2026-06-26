#pragma once
#include "GameContent_Defines.h"
#include "Component.h"

NS_BEGIN(Engine) 
class CTransform; 
class CController; 
NS_END

NS_BEGIN(Client)

// 투사체용 탄도+바운스 무브먼트. CController(ref) 구동, 바닥 바운스 반사.
// 바운스 "처리"는 여기(물리), 바운스 "결과 판단"(파괴/폭발)은 소비자.
class CProjectile_Movement final : public CComponent
{
    GENERATED_BODY(CProjectile_Movement)
private:
    CProjectile_Movement(ID3D11Device*, ID3D11DeviceContext*);
    CProjectile_Movement(const CProjectile_Movement&);
    virtual ~CProjectile_Movement() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override { return S_OK; }

    void  Set_Refs(CTransform* pTransform, CController* pController)
    {
        m_pTransform = pTransform; m_pController = pController;
    }
    void  Set_Physics(_float fGravity, _float fRestitution, _float fHorizDamp)
    {
        m_fGravity = fGravity; m_fRestitution = fRestitution; m_fHorizDamp = fHorizDamp;
    }

    void  Launch(_fvector vVelocity) { XMStoreFloat3(&m_vVelocity, vVelocity); }
    void  Stop() { m_vVelocity = { 0.f, 0.f, 0.f }; }

    // 이동+바운스 1틱. 반환: 이번 프레임 "바닥" 바운스 발생 여부.
    _bool Tick(_float fTimeDelta);

    _bool  Is_Grounded() const { return m_bGrounded; }
    _float3 Get_Velocity() const { return m_vVelocity; }

private:
    CTransform* m_pTransform = { nullptr };
    CController* m_pController = { nullptr };

    _float3 m_vVelocity = { 0.f, 0.f, 0.f };
    _bool   m_bGrounded = { false };

    _float  m_fGravity = { -45.f };
    _float  m_fRestitution = { 0.5f };
    _float  m_fHorizDamp = { 0.7f };

public:
    static  CProjectile_Movement* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END