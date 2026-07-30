#include "RangerEnemy.h"
#include "GameInstance.h"
#include "Monster_StateMachine.h"

#include "RangerEnemy_Body.h"
#include "RangerEnemy_Brain.h"
#include "Monster_Movement.h"

#include "Monster_State_Idle.h"
#include "Monster_State_Fall.h"
#include "Monster_State_Landing.h"
#include "Monster_State_KnockBack.h"
#include "Monster_State_KnockBackDeath.h"
#include "Monster_State_KnockOut.h"
#include "Monster_State_Captured.h"
#include "Monster_State_Spat.h"

CRangerEnemy::CRangerEnemy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonster{ pDevice, pContext }
{
}

CRangerEnemy::CRangerEnemy(const CRangerEnemy& Prototype)
    : CMonster(Prototype)
{
}

HRESULT CRangerEnemy::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CRangerEnemy::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_eCopyAbility = COPY_ABILITY_TYPE::NONE;
    m_fCullDist = 175.f;

    if (m_pMovement)
        m_pMovement->Set_Stats(0.f, 360.f, -20.f, 0.f);

    return S_OK;
}

_bool CRangerEnemy::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
    Out.fRadius = { 0.6f };
    Out.fHeight = { 1.0f };
    return true;
}

CAnimator* CRangerEnemy::Get_BodyAnimator() const
{
    return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

CMonsterBrain* CRangerEnemy::Create_Brain()
{
    return CRangerEnemy_Brain::Create(this);
}

HRESULT CRangerEnemy::Ready_State()
{
    if (nullptr == m_pStateMachine)
        return E_FAIL;

    if (FAILED(__super::Ready_State()))
        return E_FAIL;

    ANI_PLAY_INFO Info{};

    Info.strAniName = "Wait";
    Info.bLoop = true;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::IDLE, CMonster_State_Idle::Create(Info))))
        return E_FAIL;

    Info.strAniName = "Fall";
    Info.bLoop = true;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::FALL, CMonster_State_Fall::Create(Info))))
        return E_FAIL;

    Info.strAniName = "Landing";
    Info.bLoop = false;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::LANDING, CMonster_State_Landing::Create(Info))))
        return E_FAIL;

    Info.strAniName = "Damage";
    Info.bLoop = false;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK, CMonster_State_KnockBack::Create(Info))))
        return E_FAIL;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK_DEATH, CMonster_State_KnockBackDeath::Create(Info))))
        return E_FAIL;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_OUT, CMonster_State_KnockOut::Create(Info))))
        return E_FAIL;

    Info.bLoop = true;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::CAPTURED, CMonster_State_Captured::Create(Info))))
        return E_FAIL;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::SPAT, CMonster_State_Spat::Create(Info))))
        return E_FAIL;

    return S_OK;
}

HRESULT CRangerEnemy::Ready_PartObjects()
{
    m_pBody = Add_MonsterPart<CRangerEnemy_Body>(CRangerEnemy_Body::PROTOTYPE_TAG, TEXT("Body"));
    if (nullptr == m_pBody)
        return E_FAIL;

    return S_OK;
}

CRangerEnemy* CRangerEnemy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CRangerEnemy* pInstance = new CRangerEnemy(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CRangerEnemy");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CRangerEnemy::Clone(void* pArg)
{
    CRangerEnemy* pInstance = new CRangerEnemy(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CRangerEnemy");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CRangerEnemy::Free()
{
    __super::Free();
}
