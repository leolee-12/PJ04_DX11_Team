#include "Cappy.h"
#include "GameInstance.h"
#include "GameContent_AnimEvents.h"

#include "Cappy_Brain.h"
#include "Cappy_Hat.h"
#include "Cappy_Body.h"

#include "Monster_StateMachine.h"
#include "Monster_Movement.h"
#include "Monster_State_Fall.h"
#include "Monster_State_Landing.h"
#include "Monster_State_Spat.h"
#include "Monster_State_Captured.h"
#include "Monster_State_KnockBack.h"
#include "Monster_State_KnockBackDeath.h"
#include "Monster_State_KnockOut.h"

#include "Cappy_State_Idle.h"
#include "Cappy_State_Detect.h"
#include "Cappy_State_Chase.h"
#include "Cappy_State_HatLose.h"

CCappy::CCappy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CCappy::CCappy(const CCappy& Prototype)
	: CMonster ( Prototype )
{
}

HRESULT CCappy::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CCappy::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_eCopyAbility = COPY_ABILITY_TYPE::NONE;
	m_fCullDist = 95.f;

	if (m_pTransformCom)
		m_pTransformCom->Set_RotationPerSec(180.f);

	return S_OK;
}

void CCappy::Update(_float fTimeDelta)
{
	if (m_bDeferFinal)
	{
		if (!Hat_Out())
		{
			m_bDeferFinal = false; 
			Set_Active(false);
			return;
		}
		CContainerObject::Update(fTimeDelta);	//  AI 는 정지, 모자 파트는 Update해야 함
		return;
	}

	Check_HatlessReflex();
	__super::Update(fTimeDelta);
	Sync_PartFlags();
}

_bool CCappy::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
	Out.fRadius = { 0.5f };
	Out.fHeight = { 0.75f };

	return true;
}

CAnimator* CCappy::Get_BodyAnimator() const
{
	return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

_bool CCappy::Can_BeInhaled(const INHALE_QUERY& q) const
{
	// 모자가 부착되어 있으면 몸은 안빨림
	if (m_pHat && (!m_pHat->Is_Detached() || m_pHat->Is_Capturing()))
		return false;

	return __super::Can_BeInhaled(q);
}

void CCappy::On_Swallowed()
{
	SWALLOW_EVENT payload{ this };
	m_pGameInstance_Proxy->Publish(EventTag::Swallowed, &payload);
	m_pCaptor = nullptr;

	if (Hat_Out())
	{
		if (m_pBody) m_pBody->Set_Active(false);  // 몸통 숨김(입 속)
		m_bDeferFinal = true;                      // 모자 소멸까지 유지
		return;
	}
	Set_Active(false);
}

void CCappy::On_SpatBegin()
{
	__super::On_SpatBegin();                   // Set_Active(true)+콜off+SPAT
	if (m_pBody) m_pBody->Set_Active(true);    // 몸통 다시 렌더(별 사출)
	m_bDeferFinal = false;
	if (m_pHat) m_pHat->Set_Thrown(false);
}

void CCappy::On_SpatEnd()
{
	if (Hat_Out())
	{
		if (m_pBody) m_pBody->Set_Active(false);
		m_bDeferFinal = true;                  // 모자 아직 밖 → Despawn 보류
		return;
	}
	__super::On_SpatEnd();
}

void CCappy::Enable_Colliders(_bool bEnable)
{
	__super::Enable_Colliders(bEnable);

	if (m_pHat)
		m_pHat->Enable_HurtBox(bEnable && !m_pHat->Is_Detached());
}

CMonsterBrain* CCappy::Create_Brain()
{
	return CCappy_Brain::Create(this);
}

HRESULT CCappy::Ready_State()
{
	if (nullptr == m_pStateMachine)
		return E_FAIL;

	if (FAILED(__super::Ready_State()))
		return E_FAIL;

	ANI_PLAY_INFO Info{};
	
	// 임시 작성 
	// IDLE
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::IDLE, CCappy_State_Idle::Create(Info))))
		return E_FAIL;

	// DETECT
	CCappy_State_Detect* pDetect = CCappy_State_Detect::Create(Info);
	if (nullptr == pDetect)
		return E_FAIL;
	pDetect->Set_NextState(m_iAIType == 1 
		? MONSTER_STATE_TYPE::CHASE 
		: MONSTER_STATE_TYPE::IDLE);
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::DETECT, pDetect)))
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

	Info.strAniName = "Wait";
	Info.bLoop = true;
	Info.fSpeed = 1.25f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::CHASE, CCappy_State_Chase::Create(Info, 5.f))))    
		return E_FAIL;

	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::HAT_LOSE, CCappy_State_HatLose::Create(Info))))
		return E_FAIL;

	return S_OK;
}

HRESULT CCappy::Ready_AnimEvents()
{
	if (nullptr == m_pBody || m_pHat == nullptr)
		return E_FAIL;

	CAnimator* pAnimator = m_pBody->Get_Animator();
	if (nullptr == pAnimator)
		return E_FAIL;

	pAnimator->Set_EventCallback(
		[this](const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
		{
			if (Handle_SoundAnimEvent(e, ePhase))
				return;

			switch (static_cast<EANIM_EVENT>(e.iEventType))
			{
			case EANIM_EVENT::MoveWindow:
			{
				auto& BB = Get_BlackBoard();
				if (ePhase == ANIM_EVENT_PHASE::BEGIN)
				{
					BB.bCanMove = true;
					BB.fMoveWinLo = e.fTriggerProgress;
					BB.fMoveWinHi = e.fEndProgress;
				}
				else if (ePhase == ANIM_EVENT_PHASE::END)
					BB.bCanMove = false;
				break;
			}
			case EANIM_EVENT::OnOffPart:
			{
				if (ePhase == ANIM_EVENT_PHASE::BEGIN)
					m_pHat->Set_Thrown(true);
				else if (ePhase == ANIM_EVENT_PHASE::END)
					m_pHat->Set_Thrown(false);
				break;
			}
			default: 
				break;
			}
		});

	return S_OK;
}

void CCappy::Apply_AIVariation(const _wstring& strVariation)
{
	// Wait=0(제자리) / WaitPursuit=1(추격) / MoveTurnCliff=순찰(엔진 대응 타입 없음)
	if (strVariation == L"WaitPursuit")
		m_iAIType = 1;
	else
		m_iAIType = 0;		 // Wait + MoveTurnCliff(일단 0)

}

void CCappy::On_Damaged(const ATTACK_INFO& tInfo)
{
	if (tInfo.eHitType == HIT_TYPE::BREAKERABLE_HIT)
	{
		if (m_pHat && (m_pHat->Is_Detached() || m_pHat->Is_Thrown()))
			m_pHat->Despawn();

		if (m_pMovement && m_pMovement->Is_Grounded())
		{
			Change_State(MONSTER_STATE_TYPE::FLATTEN);
		}
		else
			Despawn();

		return;
	}

	__super::On_Damaged(tInfo);
}

HRESULT CCappy::Ready_PartObjects()
{
	m_pBody = Add_MonsterPart<CCappy_Body>(CCappy_Body::PROTOTYPE_TAG, TEXT("Body"));
	if (nullptr == m_pBody)
		return E_FAIL;

	m_pHat = Add_MonsterPart<CCappy_Hat>(CCappy_Hat::PROTOTYPE_TAG, TEXT("Hat"),
							m_pBody->Get_BoneMatrixPtr("CappyHatL"));
	if (nullptr == m_pHat)
		return E_FAIL;

	return S_OK;
}

void CCappy::On_Deserialized()
{
	__super::On_Deserialized();
}

void CCappy::Sync_PartFlags()
{
	if (nullptr == m_pBody)
		return;

	_bool bHatOn = (m_pHat && !m_pHat->Is_Detached());
	m_pBody->Set_HatAttached(bHatOn);
}

_bool CCappy::Hat_Out() const
{
	// TODO : Swallow-Down 추가되면 재검토 필요 
	// 삼킨 모자는 On_SpatEnd 미발화라 Is_Gone이 안불림
	return m_pHat && m_pHat->Needs_Container();
}

void CCappy::Check_HatlessReflex()
{
	if (nullptr == m_pHat || !m_pHat->Is_Detached())
		return;
	if (!Has_State(MONSTER_STATE_TYPE::HAT_LOSE))
		return;

	const MONSTER_STATE_TYPE eCur = Get_StateType();

	if (eCur == MONSTER_STATE_TYPE::IDLE ||	eCur == MONSTER_STATE_TYPE::CHASE)
	{
		Change_State(MONSTER_STATE_TYPE::HAT_LOSE);
		return;
	}

	if (eCur == MONSTER_STATE_TYPE::DETECT && m_pHat->Is_Capturing())
		Change_State(MONSTER_STATE_TYPE::HAT_LOSE);
}

CCappy* CCappy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCappy* pInstance = new CCappy(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CCappy");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCappy::Clone(void* pArg)
{
	CCappy* pInstance = new CCappy(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CCappy");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCappy::Free()
{
	__super::Free();
}
