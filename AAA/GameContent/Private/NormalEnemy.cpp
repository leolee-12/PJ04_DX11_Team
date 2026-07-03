#include "NormalEnemy.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "Animator.h"
#include "GameContent_AnimEvents.h"
#include "Monster_StateMachine.h"
#include "NormalEnemy_Brain.h"

// Parts
#include "NormalEnemy_Body.h"

// 공용 상태
#include "Monster_State_Idle.h"
#include "Monster_State_Detect.h"
#include "Monster_State_Fall.h"
#include "Monster_State_Landing.h"
#include "Monster_State_Chase.h"
#include "Monster_State_Captured.h"
#include "Monster_State_KnockBack.h"
#include "Monster_State_KnockOut.h"
#include "Monster_State_Spat.h"
#include "Monster_State_KnockBackDeath.h"
#include "Monster_State_Patrol.h"

// 전용 상태
#include "NormalEnemy_State_Find.h"
#include "NormalEnemy_State_Chase.h"
#include "NormalEnemy_State_Brake.h"

CNormalEnemy::CNormalEnemy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster { pDevice, pContext }
{
}

CNormalEnemy::CNormalEnemy(const CNormalEnemy& Prototype)
	: CMonster( Prototype )
{
}

HRESULT CNormalEnemy::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CNormalEnemy::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_eCopyAbility = COPY_ABILITY_TYPE::NONE;

	if (m_pTransformCom)
		m_pTransformCom->Set_RotationPerSec(360.f);

	// 임시 
	XMStoreFloat3(&m_vBasePos, m_pTransformCom->Get_State(STATE::POSITION));

	return S_OK;
}

void CNormalEnemy::Priority_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CNormalEnemy::Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

#ifdef _DEBUG
	if (m_pGameInstance_Proxy->Is_EditMode())
	{
		if (m_pMovement) m_pMovement->Sync_To_Controller();
		return;
	}
#endif

	__super::Update(fTimeDelta);
}

void CNormalEnemy::Late_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Late_Update(fTimeDelta);
}

_bool CNormalEnemy::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
	Out.fRadius = { 0.6f };
	Out.fHeight = { 0.75f };
	return true;
}

CAnimator* CNormalEnemy::Get_BodyAnimator() const
{
	return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

CMonsterBrain* CNormalEnemy::Create_Brain()
{
	return CNormalEnemy_Brain::Create(this);
}

HRESULT CNormalEnemy::Ready_State()
{
	if (m_pStateMachine == nullptr)
		return E_FAIL;

	// State Idle
	ANI_PLAY_INFO Info{};

	Info.strAniName = "Wait";
	Info.bLoop = true;
	Info.fSpeed = 2.0f;

	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::IDLE, CMonster_State_Idle::Create(Info))))
		return E_FAIL;
	
	// State Detect - 애니메이션 자체에 CHASE로의 전이가 고정되어 있음
	Info.strAniName = "Find";
	Info.bLoop = false;
	Info.fSpeed = 1.25f;

	CMonster_State_Detect* pState = CMonster_State_Detect::Create(Info);
	if (pState == nullptr)
		return E_FAIL;
	pState->Set_NextState(MONSTER_STATE_TYPE::CHASE);

	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::DETECT, pState)))
		return E_FAIL;

	// State Fall 
	Info.strAniName = "Fall";
	Info.bLoop = true;
	Info.fSpeed = 1.5f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::FALL, CMonster_State_Fall::Create(Info))))
		return E_FAIL;

	// State Landing
	Info.strAniName = "Landing";
	Info.bLoop = false;
	Info.fSpeed = 1.25f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::LANDING, CMonster_State_Landing::Create(Info))))
		return E_FAIL;

	// State KnockBack
	Info.strAniName = "Damage";
	Info.bLoop = false;
	Info.fSpeed = 2.0f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK, CMonster_State_KnockBack::Create(Info))))
		return E_FAIL;

	// State KnockBackDeath
	Info.strAniName = "Damage";
	Info.bLoop = false;
	Info.fSpeed = 2.0f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK_DEATH, CMonster_State_KnockBackDeath::Create(Info))))
		return E_FAIL;

	// State KnockOut
	Info.strAniName = "Damage";
	Info.bLoop = false;
	Info.fSpeed = 2.0f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_OUT, CMonster_State_KnockOut::Create(Info))))
		return E_FAIL;

	// State Captured
	Info.strAniName = "Damage";
	Info.bLoop = true;
	Info.fSpeed = 1.25f;

	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::CAPTURED, CMonster_State_Captured::Create(Info))))
		return E_FAIL;

	// State Spat
	Info.strAniName = "Damage";
	Info.bLoop = true;
	Info.fSpeed = 1.f;

	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::SPAT, CMonster_State_Spat::Create(Info))))
		return E_FAIL;

	// State Patrol 
	Info.strAniName = "Walk";
	Info.bLoop = true;
	Info.fSpeed = 1.5f;

	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::PATROL, CMonster_State_Patrol::Create(Info, 2.f))))
		return E_FAIL;

	// 전용 상태
	// State Chase 
	Info.strAniName = "Run";
	Info.bLoop = true;
	Info.fSpeed = 1.25f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::CHASE, CNormalEnemy_State_Chase::Create(Info, 3.0f))))
		return E_FAIL;

	// State Brake (Chase의 이동속도와 동일하게 설정)
	Info.strAniName = "Brake";
	Info.bLoop = false;
	Info.fSpeed = 1.5f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::BRAKE, CNormalEnemy_State_Brake::Create(Info, 4.0f))))
		return E_FAIL;

	// State Find
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::FIND, CNormalEnemy_State_Find::Create())))
		return E_FAIL;

	return S_OK;
}

HRESULT CNormalEnemy::Ready_AnimEvents()
{
	if (nullptr == m_pBody)
		return E_FAIL;

	CAnimator* pAnim = m_pBody->Get_Animator();
	if (nullptr == pAnim)
		return E_FAIL;

	pAnim->Set_EventCallback(
		[this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase)
		{
			if (Handle_SharedAnimEvent(e, phase))
				return;

			switch (static_cast<EANIM_EVENT>(e.iEventType))
			{
			case EANIM_EVENT::SetEye:
			{
				if (phase == ANIM_EVENT_PHASE::POINT)
				{
					m_pBody->Set_Eye((_uint)e.iIntParam);
				}
				break;
			}
			default:
				break;
			}
		});


	return S_OK;
}

void CNormalEnemy::Apply_AIVariation(const _wstring& strVariation)
{
	// 이동형 → PATROL(1) / 제자리(Wait·WaitPursuit) → 정찰(0). 추격은 감지 자동.
	if (strVariation == L"RgPursuit"          // 범위 배회
		|| strVariation == L"MoveOnRailPursuit") // 레일이동(PATROL로 근사)
		m_iAIType = 1;
	else
		m_iAIType = 0;
}

void CNormalEnemy::On_Exit(MONSTER_STATE_TYPE eNextState)
{
	if (nullptr == m_pBody)
		return;

	switch (eNextState)
	{
	case MONSTER_STATE_TYPE::KNOCK_BACK:
	case MONSTER_STATE_TYPE::KNOCK_BACK_DEATH:
	case MONSTER_STATE_TYPE::KNOCK_OUT:
	case MONSTER_STATE_TYPE::CAPTURED:
	case MONSTER_STATE_TYPE::SPAT:
		break;
	default:
		m_pBody->Set_Eye(0);
		break;
	}
}

HRESULT	CNormalEnemy::Ready_PartObjects()
{
	// Body 
	m_pBody = Add_MonsterPart<CNormalEnemy_Body>(
		CNormalEnemy_Body::PROTOTYPE_TAG, TEXT("Body"));
	if (nullptr == m_pBody)
		return E_FAIL;

	return S_OK;
}

void CNormalEnemy::On_Deserialized()
{
	__super::On_Deserialized();
}

CNormalEnemy* CNormalEnemy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNormalEnemy* pInstance = new CNormalEnemy(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CNormalEnemy");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CNormalEnemy::Clone(void* pArg)
{
	CNormalEnemy* pInstance = new CNormalEnemy(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CNormalEnemy");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNormalEnemy::Free()
{
	__super::Free();
}
