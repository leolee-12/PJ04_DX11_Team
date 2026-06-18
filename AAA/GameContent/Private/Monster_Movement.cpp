#include "Monster_Movement.h"
#include "Transform.h"

#pragma warning(push, 0)
#ifdef new
#undef new
#endif
#include <PhysX/PxPhysicsAPI.h>
#if defined(_DEBUG) && defined(DBG_NEW)
#define new DBG_NEW
#endif
#pragma warning(pop)


CMonster_Movement::CMonster_Movement(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMovement{ pDevice, pContext }
{
}

CMonster_Movement::CMonster_Movement(const CMonster_Movement& Prototype)
	: CMovement (Prototype)
{
}

HRESULT CMonster_Movement::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMonster_Movement::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CMonster_Movement::Launch(_fvector vHorizDir, _float fHorizSpeed, _float fUpSpeed)
{
	// 수평 방향
	_vector vDir = XMVector3Normalize(XMVectorSetY(vHorizDir, 0.f));
	XMStoreFloat3(&m_vHorizVel, vDir * fHorizSpeed);

	// 수직 방향
	m_fVerticalVelocity = fUpSpeed;

	m_bLaunched = true;
}

_bool CMonster_Movement::Update_Launched(_float fTimeDelta)
{
	if (nullptr == m_pTransform || nullptr == m_pController)
		return m_bGrounded;

	// 1) 수평 : 조향/가감속 없음. 처음 꽂은 속도를 관성 유지
	_float3 vHoriz;
	XMStoreFloat3(&vHoriz, XMLoadFloat3(&m_vHorizVel) * fTimeDelta);

	// 2) 수직 : 중력 - Move와 동일
	Calc_Vertical(fTimeDelta);

	// 3) CCT 이동  +  위치 반영 - Move와 동일
	physx::PxControllerFilters filters;
	filters.mFilterFlags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;

	physx::PxControllerCollisionFlags flags = m_pController->move(
		physx::PxVec3(vHoriz.x, m_fVerticalVelocity * fTimeDelta, vHoriz.z),
		0.001f, fTimeDelta, filters);

	const physx::PxExtendedVec3& foot = m_pController->getFootPosition();
	m_pTransform->Set_State(STATE::POSITION,
		XMVectorSet((_float)foot.x, (_float)foot.y, (_float)foot.z, 1.f));

	// 4) 착지 : 반사(바운스) 
	m_bGrounded = flags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN);
	if (m_bGrounded && m_fVerticalVelocity < 0.f)
	{
		_float fBounceUp = -m_fVerticalVelocity * m_fRestitution;		// 뒤집고 반발계수만큼 감쇠

		if (fBounceUp < m_fBounceStopSpeed)
		{
			m_fVerticalVelocity = 0.f;
			m_vHorizVel = {};
			m_bLaunched = false;
		}
		else
		{
			m_fVerticalVelocity = fBounceUp;
			XMStoreFloat3(&m_vHorizVel, XMLoadFloat3(&m_vHorizVel) * m_fBounceFriction);
		}
	}

	return m_bGrounded;
}

CMonster_Movement* CMonster_Movement::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMonster_Movement* pInstance = new CMonster_Movement(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMonster_Movement");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CMonster_Movement::Clone(void* pArg)
{
	CMonster_Movement* pInstance = new CMonster_Movement(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMonster_Movement");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_Movement::Apply_Facing(_fvector vFaceDir, _float fTimeDelta)
{
	if (m_bLockFacing)				// 시선 유지 필요한 움직임
		return;				
	
	__super::Apply_Facing(vFaceDir, fTimeDelta);
}

void CMonster_Movement::Free()
{
	__super::Free();
}
