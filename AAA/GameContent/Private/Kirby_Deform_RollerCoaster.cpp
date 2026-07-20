#include "Kirby_Deform_RollerCoaster.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

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

void CKirby_Deform_RollerCoaster::Enter_DeformState_Deform_End(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext)
{
    m_pGameInstance_Proxy->Play_SFX(L"HeroBasic_GetAbility.wav", 0.2f);

    pKirby->Get_Body()->Set_Active(false);

    CKirby_Deform_Model* pDeformModel_Main = pKirby->Get_DeformPart_Model(DeformContext.eDeformType, KIRBY_DEFORM_MODEL_TYPE::MAIN);
    pDeformModel_Main->Set_Active(true);

    pDeformModel_Main->Get_Animator()->Play("Deform", false, true, 0.f, 1.5f);

    COPY_ABILITY_TYPE m_eAbilityType = pKirby->Get_KirbyAbility()->Get_AbilityType();
    pKirby->Change_HatSocketMatrix(m_eAbilityType, pDeformModel_Main->Get_HatBoneMatirx());
}

_bool CKirby_Deform_RollerCoaster::Update_DeformState_Deform_End(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext, _float fTimeDelta)
{
    CKirby_Deform_Model* pDeformModel_Main = pKirby->Get_DeformPart_Model(DeformContext.eDeformType, KIRBY_DEFORM_MODEL_TYPE::MAIN);
    CAnimator* pMainAnimator = pDeformModel_Main->Get_Animator();

    if (pMainAnimator->Is_Finished())
        return true;

    return false;
}

void CKirby_Deform_RollerCoaster::Exit_DeformState_Deform_End(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext)
{
    KIRBY_ABILITY_CHANGED Desc{};
    Desc.bBegin = false;
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_Ability_Changed, &Desc);

    m_pGameInstance_Proxy->Set_TimeScale(1.f);
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
