#include "Kirby_Deform_Car.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

CKirby_Deform_Car::CKirby_Deform_Car()
{
}

HRESULT CKirby_Deform_Car::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    Set_FullBodyAni(DEFORM_ANI::WAIT, "Idling", true, false, 0.1f, 1.5f);
    Set_FullBodyAni(DEFORM_ANI::RUN, "Moving", true, false, 0.1f, 1.5f);

    Set_FullBodyAni(DEFORM_ANI::FALL, "Fall", true, false, 0.1f, 1.5f);
    Set_FullBodyAni(DEFORM_ANI::LANDING, "Landing", false, false, 0.1f, 2.f);

    Set_FullBodyAni(DEFORM_ANI::JUMP_START, "JumpStart", false, false, 0.1f, 1.5f);
    Set_FullBodyAni(DEFORM_ANI::JUMP, "Jump", false, false, 0.1f, 1.5f);

    return S_OK;
}

DEFORM_TYPE CKirby_Deform_Car::Get_DeformType()
{
    return DEFORM_TYPE::CAR;
}

void CKirby_Deform_Car::Enter_Deform(CKirby* pKirby)
{
}

void CKirby_Deform_Car::Exit_Deform(CKirby* pKirby)
{
}

void CKirby_Deform_Car::Enter_DeformState(CKirby* pKirby)
{
}

void CKirby_Deform_Car::Update_DeformState(CKirby* pKirby, _float fTimeDelta)
{
}

void CKirby_Deform_Car::Exit_DeformState(CKirby* pKirby)
{
}

_bool CKirby_Deform_Car::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    return false;
}

_bool CKirby_Deform_Car::Enter_Attack_KeyDown(CKirby* pKirby)
{
    return false;
}

_bool CKirby_Deform_Car::Enter_Attack_KeyPress(CKirby* pKirby)
{
    return false;
}

_bool CKirby_Deform_Car::Enter_Attack_KeyUp(CKirby* pKirby)
{
    return false;
}

CKirby_Deform_Car* CKirby_Deform_Car::Create()
{
    CKirby_Deform_Car* pInstance = new CKirby_Deform_Car();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Deform_Car");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Deform_Car::Free()
{
    __super::Free();
}
