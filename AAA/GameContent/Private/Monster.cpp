#include "Monster.h"
#include "Monster_StateMachine.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "Monster_Brain_FSM.h"
#include "Monster_State_Idle.h"
#include "Monster_State_Captured.h"
#include "Collider.h"
#include "GameContent_const.h"

#pragma warning(push, 0)
#ifdef new
#undef new
#endif
#include <PhysX/PxPhysicsAPI.h>
#if defined(_DEBUG) && defined(DBG_NEW)
#define new DBG_NEW
#endif
#pragma warning(pop)

CMonster::CMonster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCharacter{ pDevice, pContext }
{
}

CMonster::CMonster(const CMonster& Prototype)
	: CCharacter ( Prototype )
{
}

HRESULT CMonster::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMonster::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	SetUp_Collider_CallBack();

	return S_OK;
}

void CMonster::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMonster::Update(_float fTimeDelta)
{
	Update_AI(fTimeDelta);

	__super::Update(fTimeDelta);
}

void CMonster::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	if (m_pInteractCollider)
	{
		m_pInteractCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pInteractCollider);
#endif
	}

	if (m_pHurtBox)
	{
		{
			m_pHurtBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
			m_pGameInstance_Proxy->Add_DebugComponent(m_pHurtBox);
#endif
		}
	}
}

HRESULT CMonster::Render()
{
	return S_OK;
}

void CMonster::On_Deserialized()
{
	if (nullptr != m_pMovement)
		m_pMovement->Sync_To_Controller();
}

void CMonster::Set_Target(CGameObject* pTarget)
{
	m_BlackBoard.pTarget = pTarget;

	if (nullptr == pTarget)
	{
		m_BlackBoard.bCanSeeTarget = false;
		m_BlackBoard.fDistToTarget = FLT_MAX;
		m_BlackBoard.vTargetPos = {};
		m_BlackBoard.vLastKnownPos = {};
	}
}

void CMonster::Add_MoveDir(const _float3& vWishDir)
{
	XMStoreFloat3(&m_vWishDir,
		XMLoadFloat3(&m_vWishDir) + XMLoadFloat3(&vWishDir));
}

_bool CMonster::Has_MoveDir() const
{
	_vector vWishDir = XMLoadFloat3(&m_vWishDir);

	if (XMVector3Equal(vWishDir, XMVectorZero()))
		return false;

	return true;
}

void CMonster::Clear_MoveDir()
{
	XMStoreFloat3(&m_vWishDir, XMVectorZero());
}

_bool CMonster::Change_State(MONSTER_STATE_TYPE eNewState)
{
	if (nullptr == m_pStateMachine)
		return false;

	return m_pStateMachine->Change_State(eNewState);
}

_bool	CMonster::Has_State(MONSTER_STATE_TYPE eState) const
{
	if (nullptr == m_pStateMachine)
		return false;

	return m_pStateMachine->Has_State(eState);
}

MONSTER_STATE_TYPE	CMonster::Get_StateType() const
{
	if (nullptr == m_pStateMachine)
		return MONSTER_STATE_TYPE::IDLE;

	return m_pStateMachine->Get_StateType();
}

HRESULT CMonster::Ready_Collider()
{
	_float fRadius;

	_float3 vFootPos;
	XMStoreFloat3(&vFootPos, m_pTransformCom->Get_State(STATE::POSITION));

	if ((fRadius = Get_InteractRadius()) != 0.f)
	{
		CCollider::COLLIDER_DESC ColliderDesc{};
		ColliderDesc.pOwner = this;
		ColliderDesc.vCenter = vFootPos;
		ColliderDesc.fRadius = fRadius;

		m_pInteractCollider = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
			TEXT("InteractCol_Com"), &ColliderDesc);
		if (m_pInteractCollider == nullptr)
			return E_FAIL;

		m_pGameInstance_Proxy->Register_Collider(m_pInteractCollider, ETOUI(COLLISION_LAYER::MONSTER_D_RANGE));
	}


	if ((fRadius = Get_HurtBoxRadius()) != 0.f)
	{
		CCollider::COLLIDER_DESC HurtDesc{};
		HurtDesc.pOwner = this;
		HurtDesc.vCenter = _float3(vFootPos.x, vFootPos.y + fRadius * 0.5f, vFootPos.z);
		HurtDesc.fRadius = fRadius;

		m_pHurtBox = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
			TEXT("MonHurtBox_Com"), &HurtDesc);
		if (m_pHurtBox == nullptr)
			return E_FAIL;

		m_pGameInstance_Proxy->Register_Collider(m_pHurtBox, ETOUI(COLLISION_LAYER::MONSTER_HURT));
	}

	return S_OK;
}

void CMonster::SetUp_Collider_CallBack()
{
	if (m_pInteractCollider)
	{
		m_pInteractCollider->Set_OnEnter([this](CCollider* pOther) {
			if (ETOUI(COLLISION_LAYER::PLAYER_HURT) == pOther->Get_RegisteredGroup())
				Set_Target(pOther->Get_Owner());
#ifdef _DEBUG
			char szBuf[128];
			sprintf_s(szBuf, "[Monster] Enter <- group %u\n", pOther->Get_RegisteredGroup());
			OutputDebugStringA(szBuf);
#endif // _DEBUG
			});
		m_pInteractCollider->Set_OnExit([this](CCollider* pOther) {
			if (ETOUI(COLLISION_LAYER::PLAYER_HURT) == pOther->Get_RegisteredGroup())
				Set_Target(nullptr);
#ifdef _DEBUG
			char szBuf[128];
			sprintf_s(szBuf, "[Monster] Exit <- group %u\n", pOther->Get_RegisteredGroup());
			OutputDebugStringA(szBuf);
#endif // _DEBUG
			});
	}

	if (m_pHurtBox)
	{
		m_pHurtBox->Set_OnEnter([this](CCollider* pOther) {
			if (ETOUI(COLLISION_LAYER::PLAYER_HURT) == pOther->Get_RegisteredGroup()
				&& Has_Trait(MT_BODYCHECK_DAMAGE))
			{
				_vector vAtkPos = pOther->Get_Owner()->Get_Transform()->Get_State(STATE::POSITION);
				On_Hit(vAtkPos, 1.f);
#ifdef _DEBUG
				char szBuf[128];
				sprintf_s(szBuf, "[Monster] Hurt! HP %.0f/%.0f\n", m_fCurHP, m_fMaxHP);
				OutputDebugStringA(szBuf);
#endif
			}
		});
	}

	return;
}

HRESULT CMonster::Ready_Movement()
{
	_float3 vFootPos;
	XMStoreFloat3(&vFootPos, m_pTransformCom->Get_State(STATE::POSITION));

	m_pController = m_pGameInstance_Proxy->Create_CapsuleController(
		vFootPos,
		Get_CapsuleRadius(),
		Get_CapsuleHeight());

	if (nullptr == m_pController)
		return E_FAIL;

	if (FAILED(Create_Movement()))
		return E_FAIL;

	m_pMovement->Set_Refs(m_pTransformCom, m_pController);

	return S_OK;
}

HRESULT CMonster::Ready_AI()
{
	// 윤석현 수정
	m_pBrain = Create_Brain();
	if (nullptr == m_pBrain)
		return E_FAIL;

	if (Use_StateMachine())
	{
		m_pStateMachine = CMonster_StateMachine::Create(this);  // 초기 IDLE
		if (nullptr == m_pStateMachine)
		{
			Safe_Release(m_pBrain);
			return E_FAIL;
		}

		if (FAILED(Ready_State(m_pStateMachine)))
		{
			Safe_Release(m_pStateMachine);
			Safe_Release(m_pBrain);
			return E_FAIL;
		}

		// 시작 IDLE 지정
		if (!Change_State(MONSTER_STATE_TYPE::IDLE))
		{
			Safe_Release(m_pStateMachine);
			Safe_Release(m_pBrain);
			return E_FAIL;
		}
	}

	return S_OK;
}

CMonsterBrain* CMonster::Create_Brain()
{
	return CMonster_Brain_FSM::Create(); // 기본은 FSM
}

HRESULT CMonster::Create_Movement()
{
	m_pMovement = Add_Component<CMonster_Movement>(TEXT("Com_Movement"), CMonster_Movement::Create(m_pDevice, m_pContext));

	if (nullptr == m_pMovement)
		return E_FAIL;

	return S_OK;
}

HRESULT CMonster::Ready_State(CMonster_StateMachine* pStateMachine)
{
	if (nullptr == pStateMachine)
		return E_FAIL;

	if (FAILED(pStateMachine->Register_State(MONSTER_STATE_TYPE::IDLE, CMonster_State_Idle::Create())))
		return E_FAIL;

	if (FAILED(pStateMachine->Register_State(MONSTER_STATE_TYPE::CAPTURED, CMonster_State_Captured::Create())))
		return E_FAIL;

	return S_OK;
}

void CMonster::On_Damaged(_fvector vAttackerPos, _float fDamage)
{
	m_pMovement->Knockback(vAttackerPos, 6.f);
	//Change_State(MONSTER_STATE_TYPE::HIT);
}

void CMonster::On_Death(_fvector vAttackerPos)
{
	m_pMovement->KO(vAttackerPos, 12.f);
	//Change_State(MONSTER_STATE_TYPE::DEAD);
}

void CMonster::Perceive(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (nullptr == m_BlackBoard.pTarget)
	{
		m_BlackBoard.bCanSeeTarget = false;
		m_BlackBoard.fDistToTarget = FLT_MAX;
		m_BlackBoard.fDistToTargetXZ = FLT_MAX;
		m_BlackBoard.fHeightToTarget = 0.f;
		m_BlackBoard.vDirToTargetXZ = {};
		return;
	}

	CTransform* pTargetTransform = m_BlackBoard.pTarget->Get_Transform();
	if (nullptr == pTargetTransform)
	{
		m_BlackBoard.bCanSeeTarget = false;
		m_BlackBoard.fDistToTarget = FLT_MAX;
		m_BlackBoard.fDistToTargetXZ = FLT_MAX;
		m_BlackBoard.fHeightToTarget = 0.f;
		m_BlackBoard.vDirToTargetXZ = {};
		return;
	}

	_vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetPos = pTargetTransform->Get_State(STATE::POSITION);
	_vector vToTarget = vTargetPos - vMyPos;
	_vector vToTargetXZ = XMVectorSetY(vToTarget, 0.f);

	_float fDistXZ = XMVectorGetX(XMVector3Length(vToTargetXZ));

	if (fDistXZ > 0.0001f)
	{
		XMStoreFloat3(
			&m_BlackBoard.vDirToTargetXZ,
			XMVector3Normalize(vToTargetXZ));
	}
	else 
		m_BlackBoard.vDirToTargetXZ = {};

	XMStoreFloat3(&m_BlackBoard.vTargetPos, vTargetPos);

	m_BlackBoard.fDistToTarget = XMVectorGetX(XMVector3Length(vToTarget));
	m_BlackBoard.fDistToTargetXZ = XMVectorGetX(XMVector3Length(vToTargetXZ));
	m_BlackBoard.fHeightToTarget = XMVectorGetY(vToTarget);

	m_BlackBoard.bCanSeeTarget = true;
	m_BlackBoard.vLastKnownPos = m_BlackBoard.vTargetPos;
}

void CMonster::Enable_Controller(_bool bEnable)
{
	if (nullptr == m_pController)
		return;

	physx::PxRigidDynamic* pActor = m_pController->getActor();
	if (nullptr == pActor)
		return;

	const physx::PxU32 iNum = pActor->getNbShapes();
	if (0 == iNum)
		return;

	std::vector<physx::PxShape*> Shapes(iNum);
	pActor->getShapes(Shapes.data(), iNum);
	for (physx::PxShape* pShape : Shapes)
	{
		if (nullptr == pShape)
			continue;
		pShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, bEnable);  // 물리 막힘 on/off
		pShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, bEnable); // 레이/스윕 쿼리 on/off
	}
}

void CMonster::On_Swallowed()
{
	SWALLOW_EVENT payload{ this };
	m_pGameInstance_Proxy->Publish(EVT_SWALLOWED, &payload);
	m_pCaptor = nullptr;
	Set_Active(false);
}

void CMonster::Update_AI(_float fTimeDelta)
{
	// 이전 프레임 이동 요청 초기화
	Clear_MoveDir(); 

	// BlackBoard 갱신
	Perceive(fTimeDelta);	

	// Brain이 상태 변경 판단
	if (nullptr != m_pBrain)
		m_pBrain->Decide(this, m_BlackBoard, fTimeDelta);

	// 현재 State 실행
	if (nullptr != m_pStateMachine)
		m_pStateMachine->Update_StateMachine(fTimeDelta);
	
	if (nullptr == m_pMovement)
		return;

	if (m_pGameInstance_Proxy->Is_EditMode())
	{
		m_pMovement->Sync_To_Controller();
		return;
	}

	if (Get_StateType() == MONSTER_STATE_TYPE::CAPTURED)
		return;

	if (m_pMovement->Is_Launched())
	{
		m_pMovement->Update_Launched(fTimeDelta);
	}
	else if (Has_MoveDir())
	{
		_vector vDir = XMLoadFloat3(&m_vWishDir);
		m_pMovement->Move(vDir, fTimeDelta);
	}
	else
	{
		m_pMovement->Move(XMVectorZero(), fTimeDelta);
	}
}

void CMonster::Free()
{
	Safe_Release(m_pBrain);
	Safe_Release(m_pStateMachine);

	if (nullptr != m_pController)
	{
		m_pGameInstance_Proxy->Release_Controller(m_pController);
		m_pController = nullptr;
	}

	__super::Free();
}
