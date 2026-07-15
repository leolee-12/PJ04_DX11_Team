#include "PoppyBrosJr.h"
#include "GameInstance.h"
#include "Animator.h"
#include "GameContent_AnimEvents.h"
#include "LevelDesign_LoadTypes.h"

#include "Monster_Movement.h"
#include "PoppyBrosJr_Body.h"
#include "PoppyBrosJr_Brain.h"

#include "Monster_StateMachine.h"
#include "Monster_State_Idle.h"
#include "Monster_State_Chase.h"
#include "Monster_State_Patrol.h"
#include "Monster_State_Fall.h"
#include "Monster_State_Landing.h"
#include "Monster_State_Spat.h"
#include "Monster_State_Captured.h"
#include "Monster_State_KnockBack.h"
#include "Monster_State_KnockBackDeath.h"
#include "Monster_State_KnockOut.h"

// Attack ( throw ) 
#include "PoppyBrosJr_State_Idle.h"
#include "PoppyBrosJr_State_Throw.h"

#include "PoppyBrosJr_State_WindUp.h"

#include "Projectile.h"
#include "Projectile_Manager.h"
#include "EnemyBomb.h"

CPoppyBrosJr::CPoppyBrosJr(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CPoppyBrosJr::CPoppyBrosJr(const CPoppyBrosJr& Prototype)
	:CMonster ( Prototype )
{
}

HRESULT CPoppyBrosJr::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CPoppyBrosJr::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_eCopyAbility = COPY_ABILITY_TYPE::BOMB;		
	m_fCullDist = 95.f;

	if (m_pTransformCom)
		m_pTransformCom->Set_RotationPerSec(180.f);

	if (nullptr != pArg)
		Set_ThrowLevel(static_cast<const LD_OBJECT_DESC*>(pArg)->strThrowLv);

	return S_OK;
}

_bool CPoppyBrosJr::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
	Out.fRadius = { 0.5f };
	Out.fHeight = { 0.75f };

	return true;
}

CAnimator* CPoppyBrosJr::Get_BodyAnimator() const
{
	return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

void CPoppyBrosJr::Release_Bomb()
{
	if (nullptr == m_pHeldBomb)
		return;

	static_cast<CProjectile_Bomb*>(m_pHeldBomb)->Despawn();
	m_pHeldBomb = nullptr;
}

CMonsterBrain* CPoppyBrosJr::Create_Brain()
{
	return CPoppyBrosJr_Brain::Create(this);
}

HRESULT CPoppyBrosJr::Ready_State()
{
	if (m_pStateMachine == nullptr)
		return E_FAIL;

	if (FAILED(__super::Ready_State()))
		return E_FAIL;

	ANI_PLAY_INFO Info{};

	// IDLE
	Info.strAniName = "EnemyWait2";
	Info.bLoop = true;
	Info.fSpeed = 1.5f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::IDLE, CMonster_State_Idle::Create(Info))))
		return E_FAIL;

	// DETECT
	//Info.strAniName = "Find";
	//Info.bLoop = false;
	//Info.fSpeed = 1.25f;
	//if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::DETECT, CMonster_State_Detect::Create(Info))))
	//	return E_FAIL;

	//CHASE
	Info.strAniName = "Walk";
	Info.bLoop = true;
	Info.fSpeed = 1.5f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::CHASE, CMonster_State_Chase::Create(Info, 3.f))))
		return E_FAIL;

	// PATROL
	Info.strAniName = "Walk";
	Info.bLoop = true;
	Info.fSpeed = 1.5f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::PATROL, CMonster_State_Patrol::Create(Info, 3.f))))
		return E_FAIL;

	// FALL
	Info.strAniName = "Fall";
	Info.bLoop = true;
	Info.fSpeed = 1.5f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::FALL, CMonster_State_Fall::Create(Info))))
		return E_FAIL;

	// LANDING
	Info.strAniName = "Landing";
	Info.bLoop = false;
	Info.fSpeed = 1.f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::LANDING, CMonster_State_Landing::Create(Info))))
		return E_FAIL;

	// KNOCKBACK
	Info.strAniName = "Damage";
	Info.bLoop = false;
	Info.fSpeed = 2.0f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK, CMonster_State_KnockBack::Create(Info))))
		return E_FAIL;

	// KNOCKBACKDEATH
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK_DEATH, CMonster_State_KnockBackDeath::Create(Info))))
		return E_FAIL;

	// KNOCKOUT
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_OUT, CMonster_State_KnockOut::Create(Info))))
		return E_FAIL;

	// CAPTURED
	Info.bLoop = true;
	Info.fSpeed = 1.25f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::CAPTURED, CMonster_State_Captured::Create(Info))))
		return E_FAIL;

	// SPAT
	Info.fSpeed = 1.f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::SPAT, CMonster_State_Spat::Create(Info))))
		return E_FAIL;

	// ATTACK
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::ATTACK, CPoppyBrosJr_State_Throw::Create())))
		return E_FAIL;

	// WINDUP
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::WINDUP, CPoppyBrosJr_State_WindUp::Create())))
		return E_FAIL;

	return S_OK;
}

HRESULT CPoppyBrosJr::Ready_PartObjects()
{
	m_pBody = Add_MonsterPart<CPoppyBrosJr_Body>(CPoppyBrosJr_Body::PROTOTYPE_TAG, TEXT("Body"));
	if (nullptr == m_pBody)
		return E_FAIL;

	return S_OK;
}

HRESULT CPoppyBrosJr::Ready_AnimEvents()
{
	CAnimator* pAnim = Get_BodyAnimator();
	if (nullptr == pAnim)
		return E_FAIL;

	pAnim->Set_EventCallback(
		[this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase)
		{
			if (Handle_SharedAnimEvent(e, phase))
				return;

			switch (static_cast<EANIM_EVENT>(e.iEventType))
			{
			case EANIM_EVENT::Projectile:
				if (phase == ANIM_EVENT_PHASE::BEGIN)
					Attach_Bomb();
				else if (phase == ANIM_EVENT_PHASE::END)
					Throw_Bomb();
				break;
			case EANIM_EVENT::SetEye:
				if (phase == ANIM_EVENT_PHASE::POINT)
					m_pBody->Set_Eye((_uint)e.iIntParam);
				break;
			default:
				break;
			}
		});
	return S_OK;
}

void CPoppyBrosJr::Apply_AIVariation(const _wstring& strVariation)
{
	// Wait=0(고정 발사) / WaitSearch=1(플레이어 조준)
	if (strVariation == L"WaitSearch")
		m_iAIType = 1;
	else
		m_iAIType = 0;
}

void CPoppyBrosJr::Attach_Bomb()
{
	if (m_pHeldBomb)                       // 이미 들고 있으면 스킵
		return;

	CProjectile* pBomb = nullptr;
	CProjectile_Manager::GetInstance()->Spawn(Get_PrototypeLevelIndex(), Get_LevelIndex(), L"Bomb", CEnemyBomb::PROTOTYPE_TAG, &pBomb);
	if (nullptr == pBomb)
		return;

	_matrix matPre = XMMatrixRotationX(XMConvertToRadians(90.f)) *
		XMMatrixRotationY(XMConvertToRadians(180.f));

	pBomb->Attach_To_Socket(
		m_pBody->Get_BoneMatrixPtr("RHaveL"),
		m_pTransformCom->Get_WorldMatrixPtr(),
		XMMatrixInverse(nullptr, matPre));						// Bake 되어 있던 PreTransform의 역행렬을 곱함
	m_pHeldBomb = pBomb;
}

void CPoppyBrosJr::Throw_Bomb()
{
	if (nullptr == m_pHeldBomb)
		return;

	_matrix matHand =
		XMLoadFloat4x4(m_pBody->Get_BoneMatrixPtr("RHaveL")) *
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	//_vector vHand = m_pHeldBomb->Get_Transform()->Get_State(STATE::POSITION);

	_vector vHand = matHand.r[3];

	_vector vDir = m_pTransformCom->Get_State(STATE::LOOK);
	vDir += XMVectorSet(0.f, 3.f, 0.f, 0.f);		// 포물선 높이

	_float3 vP, vD;
	XMStoreFloat3(&vP, vHand);
	XMStoreFloat3(&vD, vDir);

	m_pHeldBomb->Set_Speed((m_iThrowLv >= 2) ? 22.f : 18.f);

	m_pHeldBomb->Launch(vP, vD);
	m_pHeldBomb = nullptr;
}

void CPoppyBrosJr::Set_ThrowLevel(const _wstring& strThrowLv)
{
	m_iThrowLv = (strThrowLv.size() > 2) ? _wtoi(strThrowLv.c_str() + 2) : 1;
	if (m_iThrowLv < 1) m_iThrowLv = 1;
}

CPoppyBrosJr* CPoppyBrosJr::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPoppyBrosJr* pInstance = new CPoppyBrosJr(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPoppyBrosJr");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPoppyBrosJr::Clone(void* pArg)
{
	CPoppyBrosJr* pInstance = new CPoppyBrosJr(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPoppyBrosJr");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPoppyBrosJr::Free()
{
	__super::Free();
}
