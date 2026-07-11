#include "Kirby_Dialogue.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_Dialogue::CKirby_Dialogue()
{
}

HRESULT CKirby_Dialogue::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Dialogue::Get_StateType()
{
    return KIRBY_STATE_TYPE::DIALOGUE;
}

void CKirby_Dialogue::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);
}

void CKirby_Dialogue::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);
}

void CKirby_Dialogue::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

_bool CKirby_Dialogue::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    return false;
}

void CKirby_Dialogue::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
}

void CKirby_Dialogue::Request_Dialogue(CKirby* pKirby, const SEQUENCE_KIRBY_WARP_DESC* pDesc)
{
    CTransform* pTransform = pKirby->Get_Transform();

    _vector vPos = XMLoadFloat3(&pDesc->vPosition);
    pTransform->Set_State(STATE::POSITION, vPos);

    _vector vLook = XMLoadFloat3(&pDesc->vLook);
    vLook = XMVector3Normalize(vLook);
    pTransform->LookTo(vLook);

    pKirby->Get_Movement()->Sync_To_Controller();
}

void CKirby_Dialogue::Request_DialogueAnim(CKirby* pKirby, const SEQUENCE_KIRBY_ANIM_DESC* pDesc)
{
    CAnimator* pAnimator = pKirby->Get_CurrentDeformModel()->Get_Animator();
    pAnimator->Play(pDesc->strClip, pDesc->bLoop, true, pDesc->fBlendDuration, pDesc->fSpeed);
}

CKirby_Dialogue* CKirby_Dialogue::Create()
{
    CKirby_Dialogue* pInstance = new CKirby_Dialogue();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Dialogue");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Dialogue::Free()
{
    __super::Free();
}