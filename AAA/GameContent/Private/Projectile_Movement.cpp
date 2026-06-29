#include "Projectile_Movement.h"
#include "Transform.h"
#include "Controller.h"
#include "PhysX/characterkinematic/PxController.h"   // eCOLLISION_DOWN

_bool CProjectile_Movement::Tick(_float fTimeDelta)
{
    if (!m_pController || !m_pTransform) return false;

    m_vVelocity.y += m_fGravity * fTimeDelta;            // 중력 -> 포물선

    _vector vDisp = XMLoadFloat3(&m_vVelocity) * fTimeDelta;
    _uint   iFlags = m_pController->Move(vDisp, 0.001f, fTimeDelta);

    m_pTransform->Set_State(STATE::POSITION, m_pController->Get_FootPosition()); // 동기(오프셋은 소비자 조정)

    m_bGrounded = (iFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN) != 0;

    if (m_bGrounded)                                    // 바닥 바운스 반사
    {
        m_vVelocity.y = -m_vVelocity.y * m_fRestitution;
        _float fDamp = powf(m_fHorizDamp, fTimeDelta);                      // 기존의 매 프레임마다 곱하던 행위를 시간 기반으로 수정
        m_vVelocity.x *= fDamp;
        m_vVelocity.z *= fDamp;
        return true;                                    // 바운스 발생 보고
    }
    return false;
}

CProjectile_Movement::CProjectile_Movement(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent{ pDevice, pContext } {
}
CProjectile_Movement::CProjectile_Movement(const CProjectile_Movement& Prototype)
    : CComponent(Prototype) {
}
HRESULT CProjectile_Movement::Initialize_Prototype() { return S_OK; }

CProjectile_Movement* CProjectile_Movement::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CProjectile_Movement* p = new CProjectile_Movement(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed : CProjectile_Movement"); Safe_Release(p); }
    return p;
}
Engine::CComponent* CProjectile_Movement::Clone(void* pArg) { return new CProjectile_Movement(*this); }
void CProjectile_Movement::Free() { __super::Free(); }