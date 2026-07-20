#include "Kirby_Deform_RollerCoaster.h"

CKirby_Deform_RollerCoaster::CKirby_Deform_RollerCoaster()
{
}

HRESULT CKirby_Deform_RollerCoaster::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_wstrAttackModeName = L"제트 코스터 머금기";

    return S_OK;
}

DEFORM_TYPE CKirby_Deform_RollerCoaster::Get_DeformType()
{
    return DEFORM_TYPE::COASTER;
}

void CKirby_Deform_RollerCoaster::Enter_Deform(CKirby* pKirby)
{
}

void CKirby_Deform_RollerCoaster::Exit_Deform(CKirby* pKirby)
{
}

void CKirby_Deform_RollerCoaster::Enter_AttackState(CKirby* pKirby, _int iFlag)
{
}

void CKirby_Deform_RollerCoaster::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
}

void CKirby_Deform_RollerCoaster::Exit_AttackState(CKirby* pKirby)
{
}

_bool CKirby_Deform_RollerCoaster::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    return false;
}

void CKirby_Deform_RollerCoaster::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
}

CKirby_Deform_RollerCoaster* CKirby_Deform_RollerCoaster::Create()
{
    CKirby_Deform_RollerCoaster* pInstance = new CKirby_Deform_RollerCoaster();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Deform_RollerCoaster");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Deform_RollerCoaster::Free()
{
    __super::Free();
}
