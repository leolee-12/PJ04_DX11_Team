#include "BladeKnight.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "Animator.h"
#include "GameContent_AnimEvents.h"

#include "BladeKnight_Body.h"
#include "BladeKnight_Sword.h"
#include "BladeKnight_FSM.h"

// 상태
#include "Monster_StateMachine.h"
#include "Monster_State_Attack.h"
#include "Monster_State_Chase.h"
#include "Monster_State_Retreat.h"

#include "Transform.h"

CBladeKnight::CBladeKnight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonster{ pDevice, pContext }
{
}

CBladeKnight::CBladeKnight(const CBladeKnight& Prototype)
    : CMonster( Prototype )
{

}

HRESULT CBladeKnight::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CBladeKnight::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_PartObjects()))
        return E_FAIL;

    if (FAILED(Ready_Movement()))
        return E_FAIL;

    if (FAILED(Ready_AI()))
        return E_FAIL;

    if (FAILED(Ready_AnimEvents()))
        return E_FAIL;

    m_eCopyAbility = COPY_ABILITY_TYPE::SWORD;

    return S_OK;
}

void CBladeKnight::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Priority_Update(fTimeDelta);
}

void CBladeKnight::Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Update(fTimeDelta);

#ifdef _DEBUG
    if (nullptr != m_pGameInstance_Proxy)
    {
        if (m_pGameInstance_Proxy->Key_Down(DIK_1))
            Change_State(MONSTER_STATE_TYPE::IDLE);

        if (m_pGameInstance_Proxy->Key_Down(DIK_2))
            Change_State(MONSTER_STATE_TYPE::CHASE);

        if (m_pGameInstance_Proxy->Key_Down(DIK_3))
            Change_State(MONSTER_STATE_TYPE::RETREAT);

        if (m_pGameInstance_Proxy->Key_Down(DIK_4))
            Change_State(MONSTER_STATE_TYPE::ATTACK);

        if (m_pGameInstance_Proxy->Key_Down(DIK_6))
        {
            m_pMovement->KO(XMVectorSet(0.f,0.f,0.f,1.f), 10.f);
        }

        if (m_pGameInstance_Proxy->Key_Down(DIK_8))
        {
            Set_Target(nullptr);
        }
    }

    // 날아가는 넉백 테스트
    _float fTarget = m_pMovement->Is_Launched() ? 45.f : 0.f;

    _float fPrev = m_fTiltCurDeg;
    m_fTiltCurDeg += (fTarget - m_fTiltCurDeg) * m_fTiltLerp * fTimeDelta;      // 목표로 Lerp
    _float fDelta = m_fTiltCurDeg - fPrev;                                      // 이번 프레임 더할 양

    if (fabsf(fDelta) > 1e-5f)
    {
        _vector vRight = m_pTransformCom->Get_State(STATE::RIGHT);
        m_pTransformCom->Rotate(XMQuaternionRotationAxis(vRight, XMConvertToRadians(-fDelta)));
    }

#endif
}

void CBladeKnight::Late_Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Late_Update(fTimeDelta);
}

_bool CBladeKnight::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
    Out.fRadius = { 0.6f };
    Out.fHeight = { 0.75f };
    return true;
}

void CBladeKnight::Play_StateAnimation(MONSTER_STATE_TYPE eState)
{
    if (nullptr == m_pBody || nullptr == m_pSword || nullptr == m_pMovement)
        return;

    CAnimator* pBodyAnimator = m_pBody->Get_Animator();
    if (pBodyAnimator == nullptr)
        return;

    CAnimator* pSwordAnimator = m_pSword->Get_Animator();
    if (pSwordAnimator == nullptr)
        return;

    CAnimator::ANI_PLAY_INFO AnimInfo{};

    switch (eState)
    {
    case MONSTER_STATE_TYPE::IDLE:
    {
        // IDLE :: FindWait 모션
        AnimInfo.strAniName = "FindWait";
        AnimInfo.bLoop = true;
        pBodyAnimator->Play(&AnimInfo);
        //pSwordAnimator->Play("Thrust", false, false);
        break;
    }
    case MONSTER_STATE_TYPE::CHASE:
    {
        m_pMovement->Set_MoveSpeed(2.f);
        pBodyAnimator->Play("Move", true, false);
        //pSwordAnimator->Play("Thrust", false, false);
        break;
    }

    case MONSTER_STATE_TYPE::ATTACK:
    {
        AnimInfo.strAniName = "AttackStart";
        AnimInfo.bLoop = false;
        pBodyAnimator->Play(&AnimInfo);

        switch (Get_CurrentAttackType())
        {
        case BLADEKNIGHT_ATTACK_TYPE::ATTACK:
        {
            AnimInfo.strAniName = "Attack";
            AnimInfo.bLoop = false;
            pBodyAnimator->Enqueue(AnimInfo);
            break;
        }
        case BLADEKNIGHT_ATTACK_TYPE::DOUBLE_ATTACK:
        {
            AnimInfo.strAniName = "DoubleAttack";
            AnimInfo.bLoop = false;
            pBodyAnimator->Enqueue(AnimInfo);
            break;
        }
        case BLADEKNIGHT_ATTACK_TYPE::TORNADO_ATTACK:
        {
            AnimInfo.strAniName = "TornadoAttack";
            AnimInfo.bLoop = false;
            pBodyAnimator->Enqueue(AnimInfo);
            break;
        }

        }
        break;
    }
    case MONSTER_STATE_TYPE::RETREAT:
    {
        m_pMovement->Set_MoveSpeed(1.f);
        AnimInfo.strAniName = "Retreat";
        AnimInfo.bLoop = false;
        pBodyAnimator->Play(&AnimInfo);
        break;
    }

    default:
    {
        m_pBody->Get_Animator()->Play("Wait", true, false);
        pSwordAnimator->Play("Thrust", false, false);
        break;
    }
    }
}

_bool CBladeKnight::Is_StateAnimationFinished() const
{
    if (nullptr == m_pBody)
        return true;      // 확인 불가면 끝난 걸로 간주해서 true를 반환 ( false로 반환 하게 해두면 해당 상태 못빠져나감 )

    CAnimator* pAnim = m_pBody->Get_Animator();

    return (nullptr == pAnim) ? true : pAnim->Is_Finished();
}

_bool CBladeKnight::Try_SelectAttackPattern(_float fDistXZ)
{
    static const BLADEKNIGHT_ATTACK_PATTERN StationaryPatterns[] =
    {
        { BLADEKNIGHT_ATTACK_TYPE::ATTACK,          MONSTER_STATE_TYPE::RETREAT },
        { BLADEKNIGHT_ATTACK_TYPE::ATTACK,          MONSTER_STATE_TYPE::RETREAT },
        { BLADEKNIGHT_ATTACK_TYPE::TORNADO_ATTACK,  MONSTER_STATE_TYPE::IDLE },
    };

    static const BLADEKNIGHT_ATTACK_PATTERN ChasePatterns[] =
    {
        { BLADEKNIGHT_ATTACK_TYPE::ATTACK,                  MONSTER_STATE_TYPE::RETREAT },
        { BLADEKNIGHT_ATTACK_TYPE::DOUBLE_ATTACK,           MONSTER_STATE_TYPE::RETREAT },
        { BLADEKNIGHT_ATTACK_TYPE::TORNADO_ATTACK,          MONSTER_STATE_TYPE::IDLE },
    };

    const BLADEKNIGHT_ATTACK_PATTERN* pPatterns = nullptr;
    _uint iPatternCount = 0;        // 패턴 내부 요소의 개수

    switch (m_eAIStyle)
    {
    case BLADEKNIGHT_AI_STYLE::STATIONARY:
        pPatterns = StationaryPatterns;
        iPatternCount = _countof(StationaryPatterns);
        break;

    case BLADEKNIGHT_AI_STYLE::CHASE:
        if (fDistXZ > 2.f)
            return false;       // 거리가 주어진 값보다 멀다면 공격 재생 안함

        pPatterns = ChasePatterns;
        iPatternCount = _countof(ChasePatterns);
        break;
    }

    if (nullptr == pPatterns || iPatternCount == 0)
        return false;

    m_tCurAttackPattern = pPatterns[m_iAttackPatternIndex];
    ++m_iAttackPatternIndex;

    if (m_iAttackPatternIndex >= iPatternCount)
        m_iAttackPatternIndex = 0;

    return true;    
}

CMonsterBrain* CBladeKnight::Create_Brain()
{
    return CBladeKnight_FSM::Create();
}

HRESULT CBladeKnight::Ready_State(CMonster_StateMachine* pStateMachine)
{
    if (pStateMachine == nullptr)
        return E_FAIL;

    // IDLE 등록 보장
    if (FAILED(__super::Ready_State(pStateMachine)))
        return E_FAIL;

    // BladeKnight 전용 상태 등록
    if (FAILED(pStateMachine->Register_State(MONSTER_STATE_TYPE::ATTACK, CMonster_State_Attack::Create())))
        return E_FAIL;

    if (FAILED(pStateMachine->Register_State(MONSTER_STATE_TYPE::CHASE, CMonster_State_Chase::Create())))
        return E_FAIL;

    if (FAILED(pStateMachine->Register_State(MONSTER_STATE_TYPE::RETREAT, CMonster_State_Retreat::Create())))
        return E_FAIL;

    return S_OK;
}

HRESULT CBladeKnight::Ready_AnimEvents()
{
    if (nullptr == m_pBody)
        return E_FAIL;

    CAnimator* pAnimator = m_pBody->Get_Animator();
    if (nullptr == pAnimator)
        return E_FAIL;

    pAnimator->Set_EventCallback(
        [this](const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
        {
            if (ePhase != ANIM_EVENT_PHASE::POINT)
                return;

            switch (static_cast<EANIM_EVENT>(e.iEventType))
            {
            case EANIM_EVENT::LockMove:
                Get_BlackBoard().bMoveLocked = true;
                break;

            default:
                break;
            }
        });

    return S_OK;
}

HRESULT CBladeKnight::Ready_PartObjects()
{
    //body
    m_pBody = Add_MonsterPart<CBladeKnight_Body>(
        CBladeKnight_Body::PROTOTYPE_TAG, TEXT("Body"));
    if (nullptr == m_pBody) return E_FAIL;


    //sword
    const _float4x4* pSocket = m_pBody->Get_BoneMatrixPtr("RHaveL");
    if (nullptr == pSocket) return E_FAIL;

    m_pSword = Add_MonsterPart<CBladeKnight_Sword>(
        CBladeKnight_Sword::PROTOTYPE_TAG, CBladeKnight_Sword::PART_TAG, pSocket);
    if (nullptr == m_pSword) return E_FAIL;

    return S_OK;
}

void CBladeKnight::On_Deserialized()
{
    __super::On_Deserialized();
}

CBladeKnight* CBladeKnight::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBladeKnight* pInstance = new CBladeKnight(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CBladeKnight");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CBladeKnight::Clone(void* pArg)
{
    CBladeKnight* pInstance = new CBladeKnight(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CBladeKnight");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CBladeKnight::Free()
{
	__super::Free();
}