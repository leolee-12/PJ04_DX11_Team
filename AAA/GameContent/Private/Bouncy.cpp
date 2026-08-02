#include "Bouncy.h"
#include "GameInstance.h"
#include "GameContent_AnimEvents.h"
#include "Monster_StateMachine.h"

#include "Bouncy_Body.h"
#include "Bouncy_Brain.h"
#include "Monster_Movement.h"

#include "Bouncy_State_Idle.h"
#include "Bouncy_State_Fall.h"
#include "Monster_State_Landing.h"
#include "Monster_State_KnockBack.h"
#include "Monster_State_KnockBackDeath.h"
#include "Monster_State_KnockOut.h"
#include "Monster_State_Captured.h"
#include "Monster_State_Spat.h"


CBouncy::CBouncy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CBouncy::CBouncy(const CBouncy& Prototype)
	: CMonster (Prototype)
{
}

HRESULT CBouncy::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CBouncy::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

		m_fMaxHP = 12.f;
		m_fCurHP = m_fMaxHP;

	m_eCopyAbility = COPY_ABILITY_TYPE::NONE;
	m_fCullDist = 175.f;

    if (m_pMovement)
        m_pMovement->Set_Stats(6.f, 720.f, -20.f, 11.f);

	return S_OK;
}

_bool CBouncy::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
	Out.fRadius = { 0.75f };
	Out.fHeight = { 0.25f };
	return true;
}

CAnimator* CBouncy::Get_BodyAnimator() const
{
	return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

CMonsterBrain* CBouncy::Create_Brain()
{
	return CBouncy_Brain::Create(this);
}

HRESULT CBouncy::Ready_State()
{
    if (nullptr == m_pStateMachine)
        return E_FAIL;

    if (FAILED(__super::Ready_State()))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::IDLE, CBouncy_State_Idle::Create())))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::FALL, CBouncy_State_Fall::Create())))
        return E_FAIL;

    ANI_PLAY_INFO Info{};

    Info.strAniName = "Landing";
    Info.bLoop = false;
    Info.fSpeed = 1.f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::LANDING, CMonster_State_Landing::Create(Info))))
        return E_FAIL;

    Info.strAniName = "Damage";
    Info.bLoop = false;
    Info.fSpeed = 1.25f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK, CMonster_State_KnockBack::Create(Info))))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK_DEATH, CMonster_State_KnockBackDeath::Create(Info))))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_OUT, CMonster_State_KnockOut::Create(Info))))
        return E_FAIL;

    Info.bLoop = true;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::CAPTURED, CMonster_State_Captured::Create(Info))))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::SPAT, CMonster_State_Spat::Create(Info))))
        return E_FAIL;

    return S_OK;
}

void CBouncy::Apply_AIVariation(const _wstring& strVariation)
{
	// 다른 타입 미구현이라 0으로 고정 
	if (strVariation == L"Wait")
		m_iAIType = 0;
	else
		m_iAIType = 0;
}

HRESULT CBouncy::Ready_PartObjects()
{
	m_pBody = Add_MonsterPart<CBouncy_Body>(CBouncy_Body::PROTOTYPE_TAG, TEXT("Body"));
	if (nullptr == m_pBody)
		return E_FAIL;

	return S_OK;
}

HRESULT CBouncy::Ready_AnimEvents()
{
    if (nullptr == m_pBody)
        return E_FAIL;

    CAnimator* pAnim = m_pBody->Get_Animator();
    if (nullptr == pAnim)
        return E_FAIL;

    pAnim->Set_EventCallback(
        [this](
            const ANIM_EVENT& e,
            ANIM_EVENT_PHASE phase)
        {
            if (Handle_SoundAnimEvent(e, phase))
                return;

            switch (static_cast<EANIM_EVENT>(e.iEventType))
            {
            case EANIM_EVENT::SetEye:
                if (phase == ANIM_EVENT_PHASE::POINT)
                    m_pBody->Set_Eye(
                        static_cast<_uint>(e.iIntParam));
                break;

            default:
                break;
            }
        });

    return S_OK;
}

void CBouncy::On_Exit(MONSTER_STATE_TYPE eNextState)
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

CBouncy* CBouncy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBouncy* pInstance = new CBouncy(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CBouncy");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CBouncy::Clone(void* pArg)
{
	CBouncy* pInstance = new CBouncy(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CBouncy");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CBouncy::Free()
{
	__super::Free();
}
