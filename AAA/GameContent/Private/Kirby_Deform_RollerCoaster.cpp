#include "Kirby_Deform_RollerCoaster.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

#include "Kirby_DeformDump.h"

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

    m_bReqEndAttackState = false;

    m_fRailLength = m_pRailTrack->Get_Length();
    m_fCurRailDist = 0.f;

    Set_OnRail(pKirby, m_fCurRailDist);

    m_eRollerCoasterState = DEFORM_ROLLERCOASTER_STATE::ROLLERCOASTER_STATE_END;
    Change_CoasterState(pKirby, DEFORM_ROLLERCOASTER_STATE::RUNNING);
}

void CKirby_Deform_RollerCoaster::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
    Update_CoasterState(pKirby, fTimeDelta);
}

void CKirby_Deform_RollerCoaster::Exit_AttackState(CKirby* pKirby)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_UseGravity(true);
}

_bool CKirby_Deform_RollerCoaster::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Dump
        case KIRBY_COMMAND_TYPE::DUMP:
        {
            if (!pCommand->IsPress())
                return false;

            if (m_eRollerCoasterState == DEFORM_ROLLERCOASTER_STATE::WAIT)
            {
                if (pKirby->Can_Dump() == true)
                {
                    Change_CoasterState(pKirby, DEFORM_ROLLERCOASTER_STATE::ROLLERCOASTER_STATE_END);
                    pKirby->Reset_DumpCool();
                    return true;
                }

                pKirby->Req_AbilityDumpCoolDecrease();
            }

            return true;
        }
    }

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

void CKirby_Deform_RollerCoaster::Change_CoasterState(CKirby* pKirby, DEFORM_ROLLERCOASTER_STATE eNext)
{
    if (m_eRollerCoasterState == eNext)
        return;

    Exit_CoasterState(pKirby, m_eRollerCoasterState);

    m_eRollerCoasterState = eNext;

    Enter_CoasterState(pKirby, m_eRollerCoasterState);
}

void CKirby_Deform_RollerCoaster::Enter_CoasterState(CKirby* pKirby, DEFORM_ROLLERCOASTER_STATE eState)
{
    CAnimator* pAnimator = pKirby->Get_CurrentDeformModel()->Get_Animator();

    switch (eState)
    {
        case DEFORM_ROLLERCOASTER_STATE::RUNNING:
        {
            pAnimator->Play("Running", true, false, 0.1f, 1.5f);
            break;
        }
        case DEFORM_ROLLERCOASTER_STATE::WAIT:
        {
            pAnimator->Play("Wait", true, false, 0.1f, 2.f);
            break;
        }
        case DEFORM_ROLLERCOASTER_STATE::ROLLERCOASTER_STATE_END:
        {
            m_bReqEndAttackState = true;
            pKirby->Change_State(KIRBY_STATE_TYPE::DEFORM_DUMP, DEFORM_DUMP_STATE_FLAG::SPIT_DEFORM_JUMP);
            break;
        }
    }
}

void CKirby_Deform_RollerCoaster::Update_CoasterState(CKirby* pKirby, _float fTimeDelta)
{
    switch (m_eRollerCoasterState)
    {
        case DEFORM_ROLLERCOASTER_STATE::RUNNING:
        {
            Update_OnRail(pKirby, fTimeDelta);

            if (m_fCurRailDist >= m_fRailLength - Helper::fEpsilon)
                Change_CoasterState(pKirby, DEFORM_ROLLERCOASTER_STATE::WAIT);
            break;
        }
        case DEFORM_ROLLERCOASTER_STATE::WAIT:
        {
            break;
        }
    }
}

void CKirby_Deform_RollerCoaster::Exit_CoasterState(CKirby* pKirby, DEFORM_ROLLERCOASTER_STATE eState)
{
    switch (eState)
    {
        case DEFORM_ROLLERCOASTER_STATE::RUNNING:
            break;
        case DEFORM_ROLLERCOASTER_STATE::WAIT:
            break;
    }
}

_bool CKirby_Deform_RollerCoaster::Update_OnRail(CKirby* pKirby, _float fTimeDelta)
{
    if (m_fCurRailDist > m_fRailLength)
    {
        return false;
    }

    m_fCurRailDist += 600.f * fTimeDelta;
    Helper::FloatClamp(m_fCurRailDist, 0.f, m_fRailLength);

    return Set_OnRail(pKirby, m_fCurRailDist);
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
