#include "Kirby_Deform_Cylinder.h"

#include "GameInstance.h"

CKirby_Deform_Cylinder::CKirby_Deform_Cylinder()
{
}

HRESULT CKirby_Deform_Cylinder::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

DEFORM_TYPE CKirby_Deform_Cylinder::Get_DeformType()
{
    return DEFORM_TYPE::CYLINDER;
}

void CKirby_Deform_Cylinder::Enter_Deform(CKirby* pKirby)
{
}

void CKirby_Deform_Cylinder::Exit_Deform(CKirby* pKirby)
{
}

void CKirby_Deform_Cylinder::Enter_AttackState(CKirby* pKirby, _int iFlag)
{
}

void CKirby_Deform_Cylinder::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
}

void CKirby_Deform_Cylinder::Exit_AttackState(CKirby* pKirby)
{
}

_bool CKirby_Deform_Cylinder::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    return false;
}

_bool CKirby_Deform_Cylinder::Enter_Attack_KeyDown(CKirby* pKirby)
{
    return true;
}

_bool CKirby_Deform_Cylinder::Enter_Attack_KeyPress(CKirby* pKirby)
{
    return true;
}

_bool CKirby_Deform_Cylinder::Enter_Attack_KeyUp(CKirby* pKirby)
{
    return true;
}

CKirby_Deform_Cylinder* CKirby_Deform_Cylinder::Create()
{
    CKirby_Deform_Cylinder* pInstance = new CKirby_Deform_Cylinder();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Deform_Cylinder");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Deform_Cylinder::Free()
{
    __super::Free();
}
