#include "Kabu_Brain.h"
#include"Kabu.h"
#include "Monster_RailMovement.h"

CKabu_Brain::CKabu_Brain()
{
}

HRESULT CKabu_Brain::Initialize(CMonster* pOwner)
{
    if (FAILED(__super::Initialize(pOwner)))
        return E_FAIL;

    return S_OK;
}

void CKabu_Brain::Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
    if (nullptr == m_pOwner)
        return;

    if (!Can_Decide(BlackBoard))
        return;

    UNREFERENCED_PARAMETER(fTimeDelta);

    if (m_pOwner->Get_StateType() != MONSTER_STATE_TYPE::IDLE)
        return;

    if (!m_pOwner->Has_State(MONSTER_STATE_TYPE::WARPOUT))
        return;

    CMonster_RailMovement* pRailMove = static_cast<CMonster_RailMovement*>(m_pOwner->Get_Movement());
    if (pRailMove == nullptr)
        return;

    CKabu* pKabu = static_cast<CKabu*>(m_pOwner);

    if (!pKabu->Is_Visible())
    {
        if (pRailMove->Has_Rail())
            m_pOwner->Change_State(MONSTER_STATE_TYPE::WARPIN);
        return;
    }

    if (!pRailMove->Is_OffPath())
        return;

    if (pRailMove->Is_OffPath())
        m_pOwner->Change_State(MONSTER_STATE_TYPE::WARPOUT);
}

CKabu_Brain* CKabu_Brain::Create(CMonster* pOwner)
{
    CKabu_Brain* pInstance = new CKabu_Brain();
    if (FAILED(pInstance->Initialize(pOwner)))
    {
        MSG_BOX("Failed to Created : CKabu_Brain");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CKabu_Brain::Free()
{
    __super::Free();
}
