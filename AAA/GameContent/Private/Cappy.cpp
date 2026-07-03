#include "Cappy.h"
#include "GameInstance.h"

#include "Cappy_Brain.h"
#include "Cappy_Hat.h"
#include "Cappy_Body.h"

#include "Monster_StateMachine.h"
#include "Monster_State_Idle.h"
#include "Monster_Movement.h"
#include "Monster_State_Fall.h"
#include "Monster_State_Landing.h"
#include "Monster_State_Spat.h"
#include "Monster_State_Captured.h"
#include "Monster_State_KnockBack.h"
#include "Monster_State_KnockBackDeath.h"
#include "Monster_State_KnockOut.h"

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

	if (m_pTransformCom)
		m_pTransformCom->Set_RotationPerSec(180.f);

	return S_OK;
}

void CCappy::Priority_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CCappy::Update(_float fTimeDelta)
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

void CCappy::Late_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Late_Update(fTimeDelta);
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

CMonsterBrain* CCappy::Create_Brain()
{
	return CCappy_Brain::Create(this);
}

HRESULT CCappy::Ready_State()
{
	if (nullptr == m_pStateMachine)
		return E_FAIL;

	ANI_PLAY_INFO Info{};
	
	// 임시 작성 
	// IDLE
	Info.strAniName = "Wait";
	Info.bLoop = true;
	Info.fSpeed = 1.5f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::IDLE, CMonster_State_Idle::Create(Info))))
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

	return S_OK;
}

HRESULT CCappy::Ready_AnimEvents()
{
	if (nullptr == m_pBody)
		return E_FAIL;

	CAnimator* pAnimator = m_pBody->Get_Animator();
	if (nullptr == pAnimator)
		return E_FAIL;

	pAnimator->Set_EventCallback(
		[this](const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
		{
			if (Handle_SharedAnimEvent(e, ePhase))
				return;

			//switch (static_cast<EANIM_EVENT>(e.iEventType))
			//{
			//default:
			//	break;
			//}
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

void CCappy::On_Swallowed()
{
	__super::On_Swallowed();
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
