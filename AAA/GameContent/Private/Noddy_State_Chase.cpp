#include "Noddy_State_Chase.h"
#include "Monster.h"
#include "Monster_Movement.h"
#include "Transform.h"

HRESULT CNoddy_State_Chase::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    if (FAILED(__super::Initialize(tInfo, fSpeed)))
        return E_FAIL;

    m_bIsInterruptible = false;
    return S_OK;
}

MONSTER_STATE_TYPE CNoddy_State_Chase::Get_StateType()
{
    return MONSTER_STATE_TYPE::CHASE;
}

void CNoddy_State_Chase::Enter(MONSTER_STATE_TYPE ePrevState)
{
    if (nullptr == m_pOwner)
        return;

    __super::Enter(ePrevState);   

    m_fTimer = 0.f;

    // 감지 순간 방향 "고정"(추적 없음). 타깃 없으면 현재 바라보는 방향.
    const MONSTER_BLACKBOARD& BB = m_pOwner->Get_BlackBoard();
    _vector vDir = XMVectorSetY(XMLoadFloat3(&BB.vDirToTargetXZ), 0.f);

    if (XMVector3LessOrEqual(XMVector3LengthSq(vDir), XMVectorReplicate(1e-6f)))
        vDir = XMVectorSetY(m_pOwner->Get_Transform()->Get_State(STATE::LOOK), 0.f);

    XMStoreFloat3(&m_MoveDir, XMVector3Normalize(vDir));
}

void CNoddy_State_Chase::Update(_float fTimeDelta)
{
    if (nullptr == m_pOwner)
        return;

    m_fTimer += fTimeDelta;

    // 걸음 종료 -> 두리번(FIND)
    if (m_fTimer >= s_fWalkTime)
    {
        m_pOwner->Change_State(MONSTER_STATE_TYPE::FIND);
        return;
    }

    if (m_pMovement)
    {
        _float p01 = m_fTimer / s_fWalkTime;
        m_pMovement->Set_WindowMoveSpeed(m_fSpeed, p01);
    }

    __super::Update(fTimeDelta);
}

void CNoddy_State_Chase::Apply_Movement(_float fTimeDelta)
{
    m_pOwner->Add_MoveDir(XMLoadFloat3(&m_MoveDir));
}

CNoddy_State_Chase* CNoddy_State_Chase::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    CNoddy_State_Chase* pInstance = new CNoddy_State_Chase();
    if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
    {
        MSG_BOX("Failed to Created : CNoddy_State_Chase");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CNoddy_State_Chase::Free()
{
    __super::Free();
}
