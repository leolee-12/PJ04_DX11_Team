#include "Cappy_State_Chase.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CCappy_State_Chase::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    if (FAILED(__super::Initialize(tInfo, fSpeed)))
        return E_FAIL;

    m_bUseMoveWindow = true;   

    return S_OK;
}

void CCappy_State_Chase::Update(_float fTimeDelta)
{
    if (m_pOwner == nullptr)
        return;

    const MONSTER_BLACKBOARD& BB = m_pOwner->Get_BlackBoard();
    if (BB.pTarget != nullptr && m_pMovement != nullptr)
        m_pMovement->Face_Smooth(XMLoadFloat3(&BB.vTargetPos), fTimeDelta);

    __super::Update(fTimeDelta);
}

CCappy_State_Chase* CCappy_State_Chase::Create(const ANI_PLAY_INFO& tInfo, _float  fSpeed)
{
    CCappy_State_Chase* pInstance = new CCappy_State_Chase();
    if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
    {
        MSG_BOX("Failed to Created : CCappy_State_Chase");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CCappy_State_Chase::Free() 
{ 
    __super::Free();
}