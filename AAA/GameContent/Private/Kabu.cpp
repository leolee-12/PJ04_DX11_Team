#include "Kabu.h"
#include "GameInstance.h"
#include "Monster_RailMovement.h"
#include "Animator.h"
#include "Monster_StateMachine.h"
#include "Effect_Loader.h"
#include "Effect_Container.h"

#include "Kabu_Body.h"
#include "Kabu_Brain.h"

#include "Kabu_State_Idle.h"
#include "Kabu_State_WarpIn.h"
#include "Kabu_State_WarpOut.h"

#include "Monster_State_Fall.h"
#include "Monster_State_Landing.h"
#include "Monster_State_KnockBack.h"
#include "Monster_State_KnockBackDeath.h"
#include "Monster_State_KnockOut.h"
#include "Monster_State_Captured.h"
#include "Monster_State_Spat.h"


CKabu::CKabu(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonster{ pDevice, pContext }
{
}

CKabu::CKabu(const CKabu& Prototype)
    : CMonster (Prototype)
{
}

HRESULT CKabu::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKabu::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

        m_fMaxHP = 12.f;
        m_fCurHP = m_fMaxHP;

    m_eCopyAbility = COPY_ABILITY_TYPE::NONE;
    m_fCullDist = 175.f;

    Update_MoveSmokeSocket();

    return S_OK;
}

void CKabu::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    Update_MoveSmokeSocket();
    Update_MoveSmokeFx();
}

void CKabu::Damaged(const ATTACK_INFO& tInfo)
{
    if (HIT_TYPE::BREAKERABLE_HIT == tInfo.eHitType ||
        HIT_TYPE::HAMMER_PRESS == tInfo.eHitType)
    {
        if (!Is_Active() || Is_Invincible())
            return;

        if (MONSTER_STATE_TYPE::FLATTEN == Get_StateType())
            return;

        if (Block_Hit(tInfo))
            return;

        Stop_MoveSmokeFx(true);
        Resolve_DamageReactSFX(tInfo);

        auto* pRail = static_cast<CMonster_RailMovement*>(m_pMovement);

        if (pRail && pRail->Has_Rail() && pRail->Is_OffPath())
            Despawn();
        else
            Change_State(MONSTER_STATE_TYPE::FLATTEN);

        return;
    }

    __super::Damaged(tInfo);
}

_bool CKabu::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
    Out.fRadius = { 0.75f };
    Out.fHeight = { 0.40f };

    return true;
}

CAnimator* CKabu::Get_BodyAnimator() const
{
    return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

void CKabu::Set_RailDesc(const LD_RAIL_DESC& Desc)
{
    if (nullptr == m_pMovement)
        return;

    static_cast<CMonster_RailMovement*>(m_pMovement)->Set_Rail(Desc);
}

HRESULT CKabu::Bind_Rail(const RAIL_BIND_CONTEXT& Context)
{
    if (nullptr == Context.pRailDesc || nullptr == m_pMovement)
        return E_FAIL;

    Set_RailDesc(*Context.pRailDesc);
    return S_OK;
}

void  CKabu::Set_Visible(_bool bVisible)
{
    m_bVisible = bVisible;
    if (m_pBody)
        m_pBody->Set_Visible(bVisible);

    // 안보일 때 히트박스 Off
    Enable_Colliders(bVisible);
    Enable_Controller(bVisible);
}

CMonsterBrain* CKabu::Create_Brain()
{
    return CKabu_Brain::Create(this);
}

HRESULT CKabu::Create_Movement()
{
    m_pMovement = Add_Component<CMonster_RailMovement>(TEXT("Com_Movement"), CMonster_RailMovement::Create(m_pDevice, m_pContext));

    if (nullptr == m_pMovement)
        return E_FAIL;

    static_cast<CMonster_RailMovement*>(m_pMovement)->Set_SpinSpeed(720.f);

    return S_OK;
}

HRESULT CKabu::Ready_State()
{
    if (m_pStateMachine == nullptr)
        return E_FAIL;

    if (FAILED(__super::Ready_State()))
        return E_FAIL;

    ANI_PLAY_INFO Info{};

    // IDLE - 이후 CKabu_State_Idle 로 교체
    Info.strAniName = "Wait";
    Info.bLoop = true;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::IDLE, CKabu_State_Idle::Create(Info))))
        return E_FAIL;

    // Fall
    Info.strAniName = "Fall";
    Info.bLoop = true;
    Info.fSpeed = 1.5f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::FALL, CMonster_State_Fall::Create(Info))))
        return E_FAIL;

    // Landing
    Info.strAniName = "Landing";
    Info.bLoop = false;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::LANDING, CMonster_State_Landing::Create(Info))))
        return E_FAIL;

    Info.strAniName = "Damage";
    Info.bLoop = false;
    Info.fSpeed = 1.5f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK, CMonster_State_KnockBack::Create(Info))))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK_DEATH, CMonster_State_KnockBackDeath::Create(Info))))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_OUT, CMonster_State_KnockOut::Create(Info))))
        return E_FAIL;

    // Captured / Spat → Damage(loop)
    Info.bLoop = true;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::CAPTURED, CMonster_State_Captured::Create(Info))))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::SPAT, CMonster_State_Spat::Create(Info))))
        return E_FAIL;

    // Warp_Out 
    Info.strAniName = "Warp1";
    Info.bLoop = false;
    Info.fSpeed = 1.5f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::WARPOUT, CKabu_State_WarpOut::Create(Info))))
        return E_FAIL;

    Info.strAniName = "Warp2";
    Info.bLoop = false;
    Info.fSpeed = 1.5f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::WARPIN, CKabu_State_WarpIn::Create(Info))))
        return E_FAIL;

    return S_OK;
}

HRESULT CKabu::Ready_AnimEvents()
{
    if (nullptr == m_pBody)
        return E_FAIL;

    CAnimator* pAnimator = m_pBody->Get_Animator();
    if (nullptr == pAnimator)
        return E_FAIL;

    pAnimator->Set_EventCallback(
        [this](const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
        {
            if (Handle_SoundAnimEvent(e, ePhase))
                return;

            //switch (static_cast<EANIM_EVENT>(e.iEventType))
            //{
            //default:
            //    break;
            //}
        });

    return S_OK;
}

void CKabu::Update_MoveSmokeSocket()
{
    _matrix matSocket = XMMatrixIdentity();

    if (nullptr != m_pBody && nullptr != m_pTransformCom)
    {
        const _float4x4* pBone = m_pBody->Get_BoneMatrixPtr("TopL");
        if (nullptr != pBone)
        {
            _matrix matBoneWorld =
                XMLoadFloat4x4(pBone) *
                XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

            matSocket.r[3] = matBoneWorld.r[3];
        }
    }

    XMStoreFloat4x4(&m_matMoveSmokeSocket, matSocket);
}

void CKabu::Update_MoveSmokeFx()
{
    auto pLoader = CEffect_Loader::GetInstance();
    auto pRailMovement = static_cast<CMonster_RailMovement*>(m_pMovement);

    const _bool bHasRail = nullptr != pRailMovement && pRailMovement->Has_Rail();
    const _bool bOffPath = bHasRail && pRailMovement->Is_OffPath();
    const _bool bRailRiding =
        m_bActive && m_bVisible && !m_CullState.bRenderCull && bHasRail && !bOffPath &&
        nullptr != m_pStateMachine &&
        MONSTER_STATE_TYPE::IDLE == m_pStateMachine->Get_StateType();

    if (!bRailRiding)
    {
        Stop_MoveSmokeFx(!m_bActive || !bOffPath);
        return;
    }

    FX_HANDLE& hSlot = m_Effects[TEXT("MoveSmoke")];
    if (pLoader->Is_Current(hSlot))
        return;

    hSlot.Clear();
    pLoader->Spawn(TEXT("MoveSmoke"), Get_LevelIndex(),
        _float3(0.f, 0.f, 0.f),
        _float3(0.f, 0.f, 0.f),
        _float3(0.f, 0.f, 0.f),
        &m_matMoveSmokeSocket, nullptr, &hSlot);
}

void CKabu::Stop_MoveSmokeFx(_bool bImmediate)
{
    auto it = m_Effects.find(TEXT("MoveSmoke"));
    if (it == m_Effects.end())
        return;

    auto pLoader = CEffect_Loader::GetInstance();
    FX_HANDLE& hSlot = it->second;
    if (pLoader->Is_Current(hSlot))
    {
        if (bImmediate)
            hSlot.p->EffectContainer_Stop();
        else
            hSlot.p->EffectContainer_StopAfterEmission();
    }

    hSlot.Clear();
}

HRESULT  CKabu::Ready_PartObjects()
{
    m_pBody = Add_MonsterPart<CKabu_Body>(
        CKabu_Body::PROTOTYPE_TAG, TEXT("Body"));

    if (nullptr == m_pBody)
        return E_FAIL;

    if (nullptr == m_pBody->Get_BoneMatrixPtr("TopL"))
        return E_FAIL;

    return S_OK;
}

void CKabu::On_Deserialized()
{
    __super::On_Deserialized();
}

CKabu* CKabu::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKabu* pInstance = new CKabu(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKabu");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CKabu::Clone(void* pArg)
{
    CKabu* pInstance = new CKabu(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CKabu");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CKabu::Free()
{
    __super::Free();
}
