#include "Dekabu.h"
#include "GameInstance.h"
#include "Monster_StateMachine.h"
#include "GameContent_AnimEvents.h"

#include "Dekabu_Body.h"
#include "Dekabu_Brain.h"

#include "Monster_State_Idle.h"
#include "Monster_State_Fall.h"
#include "Monster_State_Landing.h"
#include "Monster_State_KnockBack.h"
#include "Monster_State_KnockBackDeath.h"
#include "Monster_State_KnockOut.h"
#include "Monster_State_Captured.h"
#include "Monster_State_Spat.h"
#include "Dekabu_State_Alert.h"
#include "Dekabu_State_Attack.h"

#include "Projectile_Manager.h"
#include "Kokabu.h"

CDekabu::CDekabu(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CDekabu::CDekabu(const CDekabu& Prototype)
	: CMonster(Prototype)
{
}

HRESULT CDekabu::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CDekabu::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

		m_fMaxHP = 48.f;
		m_fCurHP = m_fMaxHP;

	m_eCopyAbility = COPY_ABILITY_TYPE::NONE;
	m_fCullDist = 175.f;

	m_TraitFlags |= MT_STRONG_INHALE_ONLY;

	return S_OK;
}

_bool CDekabu::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
	Out.fRadius = { 1.0f };
	Out.fHeight = { 0.6f };
	return true;
}

CAnimator* CDekabu::Get_BodyAnimator() const
{
	return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

CMonsterBrain* CDekabu::Create_Brain()
{
	return CDekabu_Brain::Create(this);
}

HRESULT CDekabu::Ready_State()
{
    if (m_pStateMachine == nullptr)
        return E_FAIL;

    if (FAILED(__super::Ready_State()))
        return E_FAIL;

    ANI_PLAY_INFO Info{};

    // IDLE (Wait)
    Info.strAniName = "Wait";
    Info.bLoop = true;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::IDLE, CMonster_State_Idle::Create(Info))))
        return E_FAIL;

    // Fall
    Info.strAniName = "Fall";
    Info.bLoop = true;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::FALL, CMonster_State_Fall::Create(Info))))
        return E_FAIL;

    // Landing
    Info.strAniName = "Landing";
    Info.bLoop = false;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::LANDING, CMonster_State_Landing::Create(Info))))
        return E_FAIL;

    // Damage (KnockBack / KnockBackDeath / KnockOut)
    Info.strAniName = "Damage";
    Info.bLoop = false;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK, CMonster_State_KnockBack::Create(Info))))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK_DEATH, CMonster_State_KnockBackDeath::Create(Info))))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_OUT, CMonster_State_KnockOut::Create(Info))))
        return E_FAIL;

    // Captured / Spat (Damage loop)
    Info.bLoop = true;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::CAPTURED, CMonster_State_Captured::Create(Info))))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::SPAT, CMonster_State_Spat::Create(Info))))
        return E_FAIL;

    // WINDUP = ALERT 
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::WINDUP, CDekabu_State_Alert::Create())))
        return E_FAIL;

    // ATTACK 
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::ATTACK, CDekabu_State_Attack::Create())))
        return E_FAIL;

    return S_OK;
}

void CDekabu::Apply_AIVariation(const _wstring& strVariation)
{
    if (strVariation == L"ShootKabu")
        m_iAIType = 1;
    else
        m_iAIType = 0;
}

HRESULT CDekabu::Ready_PartObjects()
{
    m_pBody = Add_MonsterPart<CDekabu_Body>(CDekabu_Body::PROTOTYPE_TAG, TEXT("Body"));

    if (nullptr == m_pBody)
        return E_FAIL;

    return S_OK;
}

HRESULT CDekabu::Ready_AnimEvents()
{
    CAnimator* pAnim = Get_BodyAnimator();
    if (nullptr == pAnim)
        return E_FAIL;

    pAnim->Set_EventCallback(
        [this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase)
        {
            if (Handle_SoundAnimEvent(e, phase))
                return;

            switch (static_cast<EANIM_EVENT>(e.iEventType))
            {
            case EANIM_EVENT::Projectile:
                if (phase == ANIM_EVENT_PHASE::POINT)
                    Fire_KoKabu();
                break;
            default:
                break;
            }
        });
    return S_OK;
}

void CDekabu::Fire_KoKabu()
{
    CProjectile* pProj = nullptr;
    CProjectile_Manager::GetInstance()->Spawn(Get_PrototypeLevelIndex(), Get_LevelIndex(), L"KoKabu", CKokabu::PROTOTYPE_TAG, &pProj);
    if (nullptr == pProj)
        return;

    _float3 vPos, vDir;
    XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));
    XMStoreFloat3(&vDir, XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)));

    pProj->Launch(vPos, vDir);
}

CDekabu* CDekabu::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CDekabu* pInstance = new CDekabu(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CDekabu");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CDekabu::Clone(void* pArg)
{
    CDekabu* pInstance = new CDekabu(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CDekabu");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CDekabu::Free()
{
    __super::Free();
}
