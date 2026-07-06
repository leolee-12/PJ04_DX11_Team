#include "BrontoBurt.h"
#include "GameInstance.h"
#include "Monster_RailMovement.h"
#include "Animator.h"
#include "GameContent_AnimEvents.h"
#include "Monster_StateMachine.h"

#include "BrontoBurt_Body.h"
#include "BrontoBurt_Brain.h"
#include "BrontoBurt_State_Idle.h"
#include "BrontoBurt_State_Return.h"

#include "Monster_State_KnockBack.h"
#include "Monster_State_KnockBackDeath.h"
#include "Monster_State_KnockOut.h"
#include "Monster_State_Captured.h"
#include "Monster_State_Spat.h"


CBrontoBurt::CBrontoBurt(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CBrontoBurt::CBrontoBurt(const CBrontoBurt& Prototype)
	: CMonster(Prototype)
{
}

HRESULT CBrontoBurt::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CBrontoBurt::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_eCopyAbility = COPY_ABILITY_TYPE::NONE;

	if (m_pTransformCom)
		m_pTransformCom->Set_RotationPerSec(360.f);

	return S_OK;
}

_bool CBrontoBurt::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
	Out.fRadius = { 0.5f };
	Out.fHeight = { 0.25f };

	return true;
}

CAnimator* CBrontoBurt::Get_BodyAnimator() const
{
	return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

void CBrontoBurt::Set_RailDesc(const LD_RAIL_DESC& Desc)
{
	if (nullptr == m_pMovement)
		return;

	auto pRail = static_cast<CMonster_RailMovement*>(m_pMovement);
	pRail->Set_Rail(Desc);
	pRail->Snap_ToPath();
}

CMonsterBrain* CBrontoBurt::Create_Brain()
{
	return CBrontoBurt_Brain::Create(this);
}

HRESULT CBrontoBurt::Create_Movement()
{
	m_pMovement = Add_Component<CMonster_RailMovement>(TEXT("Com_Movement"), CMonster_RailMovement::Create(m_pDevice, m_pContext));

	if (nullptr == m_pMovement)
		return E_FAIL;
	auto pRail = static_cast<CMonster_RailMovement*>(m_pMovement);
	pRail->Set_PathSpeed(4.f);
	pRail->Set_FaceTangent(true);

	return S_OK;
}

HRESULT CBrontoBurt::Ready_State()
{
	if (m_pStateMachine == nullptr)
		return E_FAIL;

	if (FAILED(__super::Ready_State()))
		return E_FAIL;

	ANI_PLAY_INFO Info{};

	// IDLE 
	Info.strAniName = "Fly";
	Info.bLoop = true;
	Info.fSpeed = 1.0f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::IDLE, CBrontoBurt_State_Idle::Create(Info))))
		return E_FAIL;

	// KNOCKBACK
	Info.strAniName = "Damage";
	Info.bLoop = false;
	Info.fSpeed = 1.5f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK, CMonster_State_KnockBack::Create(Info))))
		return E_FAIL;

	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK_DEATH, CMonster_State_KnockBackDeath::Create(Info))))
		return E_FAIL;

	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_OUT, CMonster_State_KnockOut::Create(Info))))
		return E_FAIL;

	// CAPTURED
	Info.bLoop = true;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::CAPTURED, CMonster_State_Captured::Create(Info))))
		return E_FAIL;

	// SPAT
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::SPAT, CMonster_State_Spat::Create(Info))))
		return E_FAIL;

	// RETURN
	Info.strAniName = "Fly";
	Info.bLoop = true;
	Info.fSpeed = 1.0f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::RETURN, CBrontoBurt_State_Return::Create(Info))))
		return E_FAIL;

	return S_OK;
}

HRESULT CBrontoBurt::Ready_AnimEvents()
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

void CBrontoBurt::Apply_AIVariation(const _wstring& strVariation)
{
	if (strVariation == L"MoveOnRail") m_iAIType = 0;  // 레일 이동
	else if (strVariation == L"Wait")  m_iAIType = 0;  // 제자리(공중)
	else                               m_iAIType = 0;
}

void CBrontoBurt::On_Exit(MONSTER_STATE_TYPE eNextState)
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

HRESULT CBrontoBurt::Ready_PartObjects()
{
	m_pBody = Add_MonsterPart<CBrontoBurt_Body>(CBrontoBurt_Body::PROTOTYPE_TAG, TEXT("Body"));

	if (nullptr == m_pBody)
		return E_FAIL;

	return S_OK;
}

void CBrontoBurt::On_Deserialized()
{ 
	__super::On_Deserialized();
}

CBrontoBurt* CBrontoBurt::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBrontoBurt* pInstance = new CBrontoBurt(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CBrontoBurt");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CBrontoBurt::Clone(void* pArg)
{
	CBrontoBurt* pInstance = new CBrontoBurt(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CBrontoBurt");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CBrontoBurt::Free()
{
	__super::Free();
}
