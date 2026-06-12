#include "RigidBody.h"
#include "Transform.h"
#include "GameInstance_Proxy.h"

#pragma warning(push, 0)
#ifdef new
#undef new
#endif
#include <PhysX/PxPhysicsAPI.h>
#if defined(_DEBUG) && defined(DBG_NEW)
#define new DBG_NEW
#endif
#pragma warning(pop)

using namespace physx;

CRigidBody::CRigidBody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent(pDevice, pContext) {
}
CRigidBody::CRigidBody(const CRigidBody& Prototype)
    : CComponent(Prototype), m_bKinematic(Prototype.m_bKinematic) {
}

HRESULT CRigidBody::Initialize_Prototype() { return S_OK; }
HRESULT CRigidBody::Initialize(void* pArg) { return S_OK; }

void CRigidBody::Set_Body(CTransform* pTransform, PxRigidDynamic* pBody)
{
    m_pBody = pBody;
    if (m_pTransform != pTransform) {
        Safe_AddRef(pTransform);
        Safe_Release(m_pTransform);
        m_pTransform = pTransform;
    }
    if (m_pBody && m_bKinematic)
        m_pBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
}

void CRigidBody::Sync_From_Body()
{
    if (nullptr == m_pTransform || nullptr == m_pBody) return;

    const PxTransform t = m_pBody->getGlobalPose();
    const _float3 vScale = m_pTransform->Get_Scaled();   // 기존 스케일 보존

    XMMATRIX matRot = XMMatrixRotationQuaternion(XMVectorSet(t.q.x, t.q.y, t.q.z, t.q.w));
    m_pTransform->Set_State(STATE::RIGHT, matRot.r[0] * vScale.x);
    m_pTransform->Set_State(STATE::UP, matRot.r[1] * vScale.y);
    m_pTransform->Set_State(STATE::LOOK, matRot.r[2] * vScale.z);
    m_pTransform->Set_State(STATE::POSITION, XMVectorSet(t.p.x, t.p.y, t.p.z, 1.f));
}

void CRigidBody::Sync_To_Body()
{
    if (nullptr == m_pTransform || nullptr == m_pBody) return;

    _float3 p; XMStoreFloat3(&p, m_pTransform->Get_State(STATE::POSITION));

    // 정규화한 기저 → 쿼터니언 (스케일 제거)
    XMMATRIX m(XMVector3Normalize(m_pTransform->Get_State(STATE::RIGHT)),
        XMVector3Normalize(m_pTransform->Get_State(STATE::UP)),
        XMVector3Normalize(m_pTransform->Get_State(STATE::LOOK)),
        XMVectorSet(0.f, 0.f, 0.f, 1.f));
    _float4 q; XMStoreFloat4(&q, XMQuaternionRotationMatrix(m));

    m_pBody->setGlobalPose(PxTransform(PxVec3(p.x, p.y, p.z), PxQuat(q.x, q.y, q.z, q.w)));
    if (!m_bKinematic) {
        m_pBody->setLinearVelocity(PxVec3(0.f));
        m_pBody->setAngularVelocity(PxVec3(0.f));
    }
}

void CRigidBody::Add_Force(_fvector v) {
    if (m_pBody) {
        _float3 f; XMStoreFloat3(&f, v);
        m_pBody->addForce(PxVec3(f.x, f.y, f.z), PxForceMode::eFORCE);
    }
}
void CRigidBody::Add_Impulse(_fvector v) {
    if (m_pBody) {
        _float3 f; XMStoreFloat3(&f, v);
        m_pBody->addForce(PxVec3(f.x, f.y, f.z), PxForceMode::eIMPULSE);
    }
}
void CRigidBody::Set_LinearVelocity(_fvector v) {
    if (m_pBody) {
        _float3 f; XMStoreFloat3(&f, v);
        m_pBody->setLinearVelocity(PxVec3(f.x, f.y, f.z));
    }
}
void CRigidBody::WakeUp() { if (m_pBody && !m_bKinematic) m_pBody->wakeUp(); }

void CRigidBody::Set_Kinematic(_bool bKinematic)
{
    m_bKinematic = bKinematic;
    if (m_pBody) m_pBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, bKinematic);
}

CRigidBody* CRigidBody::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CRigidBody* pInstance = new CRigidBody(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created : CRigidBody"); Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CRigidBody::Clone(void* pArg)
{
    CRigidBody* pInstance = new CRigidBody(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned : CRigidBody"); Safe_Release(pInstance);
    }
    return pInstance;
}

void CRigidBody::Free()
{
    // 컴포넌트가 body 소유 → 프록시 통해 씬에서 제거 + release
    if (m_pBody) {
        m_pGameInstance_Proxy->Remove_DynamicActor(m_pBody);
        m_pBody = nullptr;
    }
    Safe_Release(m_pTransform);
    __super::Free();
}