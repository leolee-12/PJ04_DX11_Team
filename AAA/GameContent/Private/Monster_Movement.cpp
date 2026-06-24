#include "Monster_Movement.h"
#include "Transform.h"
#include "Controller.h"

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

void CMonster_Movement::Knockback(_fvector vAttackerPos, _float fStrength)
{
	m_bKO = false;
	Start_Launch(vAttackerPos, fStrength, 1.2f);
}

void CMonster_Movement::KO(_fvector vAttackerPos, _float fStrength)
{
	m_bKO = true;
	Start_Launch(vAttackerPos, fStrength, 1.4f);
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
	filters.mCCTFilterCallback = &Engine::Get_CCTFilter();

	physx::PxControllerCollisionFlags flags = m_pController->move(
		physx::PxVec3(vHoriz.x, m_fVerticalVelocity * fTimeDelta, vHoriz.z),
		0.001f, fTimeDelta, filters);

	const physx::PxExtendedVec3& foot = m_pController->getFootPosition();
	m_pTransform->Set_State(STATE::POSITION,
		XMVectorSet((_float)foot.x, (_float)foot.y, (_float)foot.z, 1.f));

	m_bGrounded = flags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN);
	if (m_bGrounded && m_fVerticalVelocity < 0.f)
	{
		if (m_bKO || !m_bBouncing)
		{
			m_fVerticalVelocity = 0.f;
			m_vHorizVel = {};
			m_bLaunched = false;
			m_bBouncing = false;
		}
		else
		{
			_float fBounceUp = -m_fVerticalVelocity * m_fRestitution;
			if (fBounceUp < m_fBounceStopSpeed)
			{
				m_fVerticalVelocity = 0.f;
				m_vHorizVel = {};
				m_bLaunched = false;
				m_bBouncing = false;
			}
			else
			{
				m_fVerticalVelocity = fBounceUp;
				XMStoreFloat3(&m_vHorizVel, XMLoadFloat3(&m_vHorizVel) * m_fBounceFriction);

				if (++m_iBounceCount >= m_iMaxBounce)			// 다음 바운스는 정착 -> 이번이 마지막
					m_bBouncing = false;
			}
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

void CMonster_Movement::Start_Launch(_fvector vAttackerPos, _float fStrength, _float fUpRatio)
{
	if (nullptr == m_pTransform)
		return;

	_vector vSelf = m_pTransform->Get_State(STATE::POSITION);
	_vector vAway = XMVectorSetY(XMVectorSubtract(vSelf, vAttackerPos), 0.f);

	if (XMVectorGetX(XMVector3LengthSq(vAway)) < 1e-6f)
		vAway = XMVectorNegate(m_pTransform->Get_State(STATE::LOOK));

	vAway = XMVector3Normalize(XMVectorSetY(vAway, 0.f));

	XMStoreFloat3(&m_vHorizVel, vAway * fStrength);    
	m_fVerticalVelocity = fStrength * fUpRatio;        
	m_bLaunched = true;
	m_iBounceCount = 0;
	m_bBouncing = (m_iMaxBounce > 0);
}

void CMonster_Movement::Free()
{
	__super::Free();
}
