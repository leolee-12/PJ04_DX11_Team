#include "Kirby_DeformCarBridge.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_DeformCarBridge::CKirby_DeformCarBridge()
{
}

HRESULT CKirby_DeformCarBridge::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_DeformCarBridge::Get_StateType()
{
    return KIRBY_STATE_TYPE::DEFORM_CAR_BRIDGE;
}

void CKirby_DeformCarBridge::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    pKirby->Get_CurrentDeformModel()->Stop_SoundHandle();
}

void CKirby_DeformCarBridge::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    Update_BridgeState(pKirby, fTimeDelta);
}

void CKirby_DeformCarBridge::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    m_eBridgeState = DEFORM_CAR_BRIDGE_STATE::DEFORM_CAR_BRIDGE_END;
}

_bool CKirby_DeformCarBridge::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    return false;
}

void CKirby_DeformCarBridge::Request_PositionSync(CKirby* pKirby, const KIRBY_POSITION_SYNC_BEGIN_DESC* pDesc)
{
    switch (pDesc->eType)
    {
        case KIRBY_POSITION_SYNC_CONTEXT::CAR_BRIDGE:
        {
            CTransform* pTransform = pKirby->Get_Transform();
            pTransform->Set_WorldMatrix(pDesc->AnchorWorld);

            pKirby->Get_Movement()->Sync_To_Controller();

            CAnimator* pAnimator = pKirby->Get_DeformPart_Model(DEFORM_TYPE::CAR)->Get_Animator();
            pAnimator->Play("Cut1", false, false, pDesc->fBlendDuration, pDesc->fAnimSpeed);

            Change_BridgeState(pKirby, DEFORM_CAR_BRIDGE_STATE::START);
            break;
        }
        default:
        {
            MSG_BOX("Event Error 1: CKirby_DeformCarBridge");
            break;
        }
    }
}

void CKirby_DeformCarBridge::Request_PositionSync_End(CKirby* pKirby, const KIRBY_POSITION_SYNC_END_DESC* pDesc)
{
    switch (pDesc->eType)
    {
        case KIRBY_POSITION_SYNC_END_REASON::CAR_BRIDGE_END:
        {
            break;
        }
        default:
        {
            MSG_BOX("Event Error 2: CKirby_DeformCarBridge");
            break;
        }
    }
}

void CKirby_DeformCarBridge::Change_BridgeState(CKirby* pKirby, DEFORM_CAR_BRIDGE_STATE eNext)
{
    if (m_eBridgeState == eNext)
        return;

    Exit_BridgeState(pKirby, m_eBridgeState);

    m_eBridgeState = eNext;

    Enter_BridgeState(pKirby, m_eBridgeState);
}

void CKirby_DeformCarBridge::Enter_BridgeState(CKirby* pKirby, DEFORM_CAR_BRIDGE_STATE eState)
{
    switch (eState)
    {
        case DEFORM_CAR_BRIDGE_STATE::START:
        {
            CAnimator* pAnimator = pKirby->Get_DeformPart_Model(DEFORM_TYPE::CAR)->Get_Animator();
            break;
        }
    }
}

void CKirby_DeformCarBridge::Update_BridgeState(CKirby* pKirby, _float fTimeDelta)
{
    switch (m_eBridgeState)
    {
        case DEFORM_CAR_BRIDGE_STATE::START:
            break;
    }
}

void CKirby_DeformCarBridge::Exit_BridgeState(CKirby* pKirby, DEFORM_CAR_BRIDGE_STATE eState)
{
    switch (eState)
    {
        case DEFORM_CAR_BRIDGE_STATE::START:
            break;
    }
}

CKirby_DeformCarBridge* CKirby_DeformCarBridge::Create()
{
    CKirby_DeformCarBridge* pInstance = new CKirby_DeformCarBridge();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_DeformCarBridge");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_DeformCarBridge::Free()
{
    __super::Free();
}
