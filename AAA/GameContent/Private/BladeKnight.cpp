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
#include "Monster_State_Idle.h"
#include "Monster_State_Detect.h"
#include "Monster_State_Fall.h"
#include "Monster_State_Landing.h"
#include "Monster_State_Captured.h"
#include "BladeKnight_State_Chase.h"
#include "BladeKnight_State_Retreat.h"

// 전용 상태
#include "BladeKnight_State_Attack.h"
#include "BladeKnight_State_DoubleAttack.h"
#include "BladeKnight_State_TornadoAttack.h"

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
        if (m_pGameInstance_Proxy->Key_Down(DIK_4))
        {
            m_pMovement->Jump();
        }

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
    //_float fTarget = m_pMovement->Is_Launched() ? 45.f : 0.f;

    //_float fPrev = m_fTiltCurDeg;
    //m_fTiltCurDeg += (fTarget - m_fTiltCurDeg) * m_fTiltLerp * fTimeDelta;      // 목표로 Lerp
    //_float fDelta = m_fTiltCurDeg - fPrev;                                      // 이번 프레임 더할 양

    //if (fabsf(fDelta) > 1e-5f)
    //{
    //    _vector vRight = m_pTransformCom->Get_State(STATE::RIGHT);
    //    m_pTransformCom->Rotate(XMQuaternionRotationAxis(vRight, XMConvertToRadians(-fDelta)));
    //}

#endif
}

void CBladeKnight::Late_Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Late_Update(fTimeDelta);
}

HRESULT CBladeKnight::Render()
{
    return S_OK;
}

CAnimator* CBladeKnight::Get_BodyAnimator() const
{
    return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

CMonsterBrain* CBladeKnight::Create_Brain()
{
    return CBladeKnight_FSM::Create();
}

HRESULT CBladeKnight::Ready_State(CMonster_StateMachine* pStateMachine)
{
    if (pStateMachine == nullptr)
        return E_FAIL;
    
    // 공통 상태에는 사용할 AnimInfo를 넣어줘야 함

    ANI_PLAY_INFO Info{};

    // State Idle
    Info.strAniName = "FindWait";
    Info.bLoop = true; 

    if (FAILED(pStateMachine->Register_State(MONSTER_STATE_TYPE::IDLE, CMonster_State_Idle::Create(Info))))
        return E_FAIL;

    // State Detect
    Info.strAniName = "Find";
    Info.bLoop = false;
    Info.fSpeed = 1.25f;

    if (FAILED(pStateMachine->Register_State(MONSTER_STATE_TYPE::DETECT, CMonster_State_Detect::Create(Info))))
        return E_FAIL;

    // State Fall
    Info.strAniName = "Fall";
    Info.bLoop = true;
    Info.fSpeed = 1.5f;
    if (FAILED(pStateMachine->Register_State(MONSTER_STATE_TYPE::FALL, CMonster_State_Fall::Create(Info))))
        return E_FAIL;

    // State Landing 
    Info.strAniName = "Landing";
    Info.bLoop = false;
    Info.fSpeed = 1.0f;

    if (FAILED(pStateMachine->Register_State(MONSTER_STATE_TYPE::LANDING, CMonster_State_Landing::Create(Info))))
        return E_FAIL;

    // State Captured
    Info.strAniName = "Damage";
    Info.bLoop = true;
    Info.fSpeed = 1.25f;

    if (FAILED(pStateMachine->Register_State(MONSTER_STATE_TYPE::CAPTURED, CMonster_State_Captured::Create(Info))))
        return E_FAIL;

    if (FAILED(pStateMachine->Register_State(MONSTER_STATE_TYPE::CHASE, CBladeKnight_State_Chase::Create())))
        return E_FAIL;

    if (FAILED(pStateMachine->Register_State(MONSTER_STATE_TYPE::RETREAT, CBladeKnight_State_Retreat::Create())))
        return E_FAIL;


    // Blade Knight 전용 상태 등록

    if (FAILED(pStateMachine->Register_State(MONSTER_STATE_TYPE::ATTACK, CBladeKnight_State_Attack::Create())))
        return E_FAIL;

    if (FAILED(pStateMachine->Register_State(MONSTER_STATE_TYPE::DOUBLE_ATTACK, CBladeKnight_State_DoubleAttack::Create())))
        return E_FAIL;

    if (FAILED(pStateMachine->Register_State(MONSTER_STATE_TYPE::TORNADO_ATTACK, CBladeKnight_State_TornadoAttack::Create())))
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
    // Body
    CBladeKnight_Body::BLADEKNIGHT_BODY_DESC BodyDesc{};
    BodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CBladeKnight_Body::PROTOTYPE_TAG, TEXT("Body"), &BodyDesc)))
        return E_FAIL;

    m_pBody = dynamic_cast<CBladeKnight_Body*>(m_PartObjects[TEXT("Body")]);
    if (nullptr == m_pBody)
        return E_FAIL;

    // Sword
    CBladeKnight_Sword::BLADEKNIGHT_SWORD_DESC SwordDesc{};
    SwordDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    SwordDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("RHaveL");

    if (nullptr == SwordDesc.pSocketBoneMatrix)
        return E_FAIL;

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CBladeKnight_Sword::PROTOTYPE_TAG, CBladeKnight_Sword::PART_TAG, &SwordDesc)))
        return E_FAIL;

    m_pSword = dynamic_cast<CBladeKnight_Sword*>(m_PartObjects[CBladeKnight_Sword::PART_TAG]);
    if (nullptr == m_pSword)
        return E_FAIL;

    return S_OK;
}

HRESULT	CBladeKnight::Bind_ShaderResources()
{
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