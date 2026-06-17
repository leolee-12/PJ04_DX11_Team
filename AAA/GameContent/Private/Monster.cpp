#include "Monster.h"
#include "Monster_StateMachine.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "Monster_Brain_FSM.h"

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

void CMonster::Change_State(MONSTER_STATE_TYPE eNewState)
{
	if (nullptr == m_pStateMachine)
		return;

	if (m_pStateMachine->Get_StateType() == eNewState)
		return;

	m_pStateMachine->Change_State(eNewState);
}

MONSTER_STATE_TYPE	CMonster::Get_StateType() const
{
	if (nullptr == m_pStateMachine)
		return MONSTER_STATE_TYPE::IDLE;

	return m_pStateMachine->Get_StateType();
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

	m_pMovement = Add_Component<CMonster_Movement>(TEXT("Com_Movement"), CMonster_Movement::Create(m_pDevice, m_pContext));
	if (nullptr == m_pMovement)
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
	}

	return S_OK;
}

CMonsterBrain* CMonster::Create_Brain()
{
	return CMonster_Brain_FSM::Create(); // 기본은 FSM
}

void CMonster::Perceive(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (nullptr == m_BlackBoard.pTarget)
	{
		m_BlackBoard.bCanSeeTarget = false;
		m_BlackBoard.fDistToTarget = FLT_MAX;
		return;
	}

	CTransform* pTargetTransform = m_BlackBoard.pTarget->Get_Transform();
	if (nullptr == pTargetTransform)
	{
		m_BlackBoard.bCanSeeTarget = false;
		m_BlackBoard.fDistToTarget = FLT_MAX;
		return;
	}

	_vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetPos = pTargetTransform->Get_State(STATE::POSITION);
	_vector vToTarget = vTargetPos - vMyPos;

	XMStoreFloat3(&m_BlackBoard.vTargetPos, vTargetPos);

	m_BlackBoard.fDistToTarget = XMVectorGetX(XMVector3Length(vToTarget));
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

void CMonster::Update_AI(_float fTimeDelta)
{
	// 이전 프레임 이동 요청 초기화
	Clear_MoveDir(); 

	// BlackBoard 갱신
	Perceive(fTimeDelta);	

	// Brain이 상태 변경 판단
	if (nullptr != m_pBrain && !Use_StateMachine())
		m_pBrain->Decide(this, m_BlackBoard, fTimeDelta);

	// 현재 State 실행
	if (nullptr != m_pStateMachine)
		m_pStateMachine->Update_StateMachine(fTimeDelta);
	
	if (nullptr == m_pMovement)
		return;

	// State가 만든 이동 방향을 Movement에 적용
	if (m_pGameInstance_Proxy->Is_EditMode())
	{
		m_pMovement->Sync_To_Controller();
		return;
	}

	if (Has_MoveDir())
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
