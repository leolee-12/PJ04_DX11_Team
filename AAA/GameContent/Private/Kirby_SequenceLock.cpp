#include "Kirby_SequenceLock.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_SequenceLock::CKirby_SequenceLock()
{
}

HRESULT CKirby_SequenceLock::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_SequenceLock::Get_StateType()
{
    return KIRBY_STATE_TYPE::SEQUENCE_LOCK;
}

void CKirby_SequenceLock::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);
}

void CKirby_SequenceLock::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);
}

void CKirby_SequenceLock::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

_bool CKirby_SequenceLock::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    return false;
}

void CKirby_SequenceLock::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
}

void CKirby_SequenceLock::Request_SequenceLock(CKirby* pKirby, const KIRBY_LEVEL_SPAWN_DESC* pDesc)
{
    CTransform* pTransform = pKirby->Get_Transform();

    _vector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    pTransform->Set_State(STATE::POSITION, vPos);

    _vector vLook = XMLoadFloat3(&pDesc->vLook);
    vLook = XMVector3Normalize(vLook);
    pTransform->LookTo(vLook);

    pKirby->Get_Movement()->Sync_To_Controller();
}

void CKirby_SequenceLock::Request_SequenceLock_End(CKirby* pKirby, const KIRBY_LEVEL_SLEEP_DESC* pDesc)
{
    pKirby->Change_State(KIRBY_STATE_TYPE::WAIT);
}

CKirby_SequenceLock* CKirby_SequenceLock::Create()
{
    CKirby_SequenceLock* pInstance = new CKirby_SequenceLock();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_SequenceLock");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_SequenceLock::Free()
{
    __super::Free();
}