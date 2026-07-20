#include "Kirby_Deform_RollerCoaster.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

#include "LevelDesign_Rail.h"
#include "RailTrack.h"

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
    m_pRail = CLevelDesign_Rail::Find_ByUid(m_pGameInstance_Proxy, pKirby->Get_LevelIndex(), m_iRailUid);
    if (m_pRail == nullptr)
    {
        assert(false);
        return;
    }

    m_pRailTrack = m_pRail->Get_RailTrack();
    if (m_pRailTrack == nullptr || !m_pRailTrack->Is_Valid())
    {
        assert(false);
        return;
    }

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Stop();
    pMovement->Clear_Forces();
    pMovement->Set_UseGravity(false);

    m_fRailLength = m_pRailTrack->Get_Length();
    m_fCurRailDist = 0.f;

    Set_OnRail(pKirby, m_fCurRailDist);
}

void CKirby_Deform_RollerCoaster::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
    Set_OnRail(pKirby, m_fCurRailDist);
}

void CKirby_Deform_RollerCoaster::Exit_AttackState(CKirby* pKirby)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_UseGravity(true);
}

_bool CKirby_Deform_RollerCoaster::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    return false;
}

void CKirby_Deform_RollerCoaster::Enter_DeformState_Deform_End(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& tDeformContext)
{
    m_iRailUid = tDeformContext.iRailUid;
    m_iStartNodeIndex = tDeformContext.iStartNodeIndex;

    // Model 교체
    CKirby_Deform_Model* pDeformModel_Main = pKirby->Get_DeformPart_Model(tDeformContext.eDeformType, KIRBY_DEFORM_MODEL_TYPE::MAIN);
    pDeformModel_Main->Set_Active(true);

    // Animation
    pDeformModel_Main->Get_Animator()->Play("Deform", false, true, 0.f, 1.5f);
}

_bool CKirby_Deform_RollerCoaster::Update_DeformState_Deform_End(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& tDeformContext, _float fTimeDelta)
{
    // 유지 필요
    return true;
}

void CKirby_Deform_RollerCoaster::Exit_DeformState_Deform_End(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& tDeformContext)
{
}

_bool CKirby_Deform_RollerCoaster::Update_OnRail(CKirby* pKirby, _float fRailDist, _float fTimeDelta)
{
    m_fCurRailDist += fTimeDelta;

    return Set_OnRail(pKirby, fRailDist);
}

_bool CKirby_Deform_RollerCoaster::Set_OnRail(CKirby* pKirby, _float fRailDist)
{
    _float3 vPosition{};
    _float3 vTangent{};

    if (!m_pRailTrack->Sample(fRailDist, &vPosition, &vTangent))
        return false;

    CTransform* pTransform = pKirby->Get_Transform();
    pTransform->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&vPosition), 1.f));
    pKirby->Get_Movement()->Sync_To_Controller();

    return true;
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
