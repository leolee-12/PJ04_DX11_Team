#include "Kirby_Deform_RollerCoaster.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

#include "Kirby_DeformDump.h"

#include "LevelDesign_Rail.h"
#include "RailTrack.h"

#include "Effect_Loader.h"

namespace
{
    constexpr _float fMinSpeed = 14.f;
    constexpr _float fMaxSpeed = 80.f;
    constexpr _float fSlopeAcceleration = 45.f;
    constexpr _float fArrivalDeceleration = 30.f;
}

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
    constexpr _float fRadius = 0.5f;
    constexpr _float fHeight = 3.1f;

    // Hurt Box
    CCollider::COLLIDER_DESC tHurtDesc{};
    tHurtDesc.pOwner = pKirby;
    tHurtDesc.vCenter = _float3(0.f, 0.7f, 0.f);
    tHurtDesc.fRadius = fRadius + CKirby::s_fHurtBoxRadiusPadding;
    tHurtDesc.fHeight = fHeight;
    pKirby->Set_ColliderDesc(CKirby::HURT_BOX, tHurtDesc);

    CUTSCENE_CAMERA_DESC camDesc{};
    camDesc.eCam = ECutsceneCam::Coaster;
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &camDesc);
}

void CKirby_Deform_RollerCoaster::Exit_Deform(CKirby* pKirby)
{
    // Hurt Box
    CCollider::COLLIDER_DESC tHurtDesc{};
    tHurtDesc.pOwner = pKirby;
    tHurtDesc.vCenter = _float3(0.f, 0.f, 0.f);
    tHurtDesc.fRadius = CKirby::s_fCCT_Radius + CKirby::s_fHurtBoxRadiusPadding;
    tHurtDesc.fHeight = CKirby::s_fCCT_Height;
    pKirby->Set_ColliderDesc(CKirby::HURT_BOX, tHurtDesc);

    CUTSCENE_CAMERA_DESC camDesc{};
    camDesc.eCam = ECutsceneCam::Area;
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &camDesc);
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
    m_fAccRailSpeed = { 10.f };
    m_bNearDestination = false;

    m_fRailLength = m_pRailTrack->Get_Length();
    m_fCurRailDist = 0.f;
    m_iLeftRight = 0;
    m_fLeftRightDegree = 0.f;

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
        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        {
            if (!pCommand->IsPress())
                return false;

            if (m_eRollerCoasterState == DEFORM_ROLLERCOASTER_STATE::RUNNING)
                m_iLeftRight -= 1;

            return true;
        }
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
        {
            if (!pCommand->IsPress())
                return false;

            if (m_eRollerCoasterState == DEFORM_ROLLERCOASTER_STATE::RUNNING)
                m_iLeftRight += 1;

            return true;
        }
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
        case KIRBY_COMMAND_TYPE::EMOTE_TOP:
        {
            if (!pCommand->IsDown())
                return false;

            m_pGameInstance_Proxy->Play_SFX(L"HeroVoice_Scream1.wav", 0.2f);

            return true;
        }
        case KIRBY_COMMAND_TYPE::EMOTE_DOWN:
        {
            if (!pCommand->IsDown())
                return false;

            m_pGameInstance_Proxy->Play_SFX(L"HeroVoice_Scream2.wav", 0.2f);

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

void CKirby_Deform_RollerCoaster::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
    pKirby->Add_HP(-tInfo.fDamage);
    pKirby->Start_DamageInvincibility();

    m_pGameInstance_Proxy->Play_SFX(L"HeroVoice_Damage4.wav", 0.15f);

    //if (m_eRollerCoasterState == DEFORM_ROLLERCOASTER_STATE::RUNNING)
    //{
    //    m_fAccRailSpeed = fMinSpeed;
    //}
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
            m_RunningSound = m_pGameInstance_Proxy->Play_SFX_Section_Loop(L"HeroDeformRollerCoaster_Running.wav", 0.04741f, 0.23682f, 0.05f);

    /*        if (FAILED(CEffect_Loader::GetInstance()->Spawn(L"CoasterWind", pKirby->Get_LevelIndex(),
                _float3{ 0.f, 0.f, 0.f }, _float3{ 0.f, 0.f, 1.f }, _float3{ 0.f, 0.f, 0.f },
                pKirby->Get_Transform()->Get_WorldMatrixPtr(), nullptr, &hCoasterWind)))
            {
                hCoasterWind.Clear();
            }*/

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

            constexpr _float fVolumeScale = 0.15f;
            _float fVolume = m_fCurFrameMoveDist * fVolumeScale;
            Helper::FloatClamp(fVolume, 0.05f, 0.2f);
            m_RunningSound.Set_Volume(fVolume);

            constexpr _float fAniSpeedScale = 3.f;
            _float fAniSpeed = powf(m_fCurFrameMoveDist * fAniSpeedScale, 2.f);
            Helper::FloatClamp(fAniSpeed, 1.5f, 8.f);

            CAnimator* pAnimator = pKirby->Get_CurrentDeformModel()->Get_Animator();
            pAnimator->Set_PlaySpeed(fAniSpeed);

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
        {
            if (m_RunningSound.Is_Valid())
                m_RunningSound.Stop();
            m_fCurFrameMoveDist = 0.f;

            CTransform* pTransform = pKirby->Get_Transform();
            _vector vLook = XMVectorSetY(pTransform->Get_State(STATE::LOOK), 0.f);

            if (XMVectorGetX(XMVector3LengthSq(vLook)) > Helper::fEpsilon)
                pTransform->LookTo(XMVector3Normalize(vLook), XMVectorSet(0.f, 1.f, 0.f, 0.f));

            //if(CEffect_Loader::GetInstance()->Is_Current(hCoasterWind))
            //    hCoasterWind.p->Start_FadeOut();

            //hCoasterWind.Clear();

            break;
        }
        case DEFORM_ROLLERCOASTER_STATE::WAIT:
        {
            break;
        }
    }
}

_bool CKirby_Deform_RollerCoaster::Update_OnRail(CKirby* pKirby, _float fTimeDelta)
{
    if (m_fCurRailDist >= m_fRailLength)
        return false;

    // 거의 도착 트리거
    if (m_bNearDestination == false && pKirby->Get_DeformEndTrigger())
        m_bNearDestination = true;


    if (m_bNearDestination)
        m_fAccRailSpeed -= fArrivalDeceleration * fTimeDelta;
    else
        m_fAccRailSpeed += m_fSlopeRatio * fSlopeAcceleration * fTimeDelta;


    Helper::FloatClamp(m_fAccRailSpeed, fMinSpeed, fMaxSpeed);

    m_fCurFrameMoveDist = m_fAccRailSpeed * fTimeDelta;

    m_fCurRailDist += m_fCurFrameMoveDist;
    Helper::FloatClamp(m_fCurRailDist, 0.f, m_fRailLength);

    Set_RotLeftRight(fTimeDelta);

    return Set_OnRail(pKirby, m_fCurRailDist);
}

_bool CKirby_Deform_RollerCoaster::Set_OnRail(CKirby* pKirby, _float fRailDist)
{
    _float3 vPosition{};
    _float3 vTangent{};

    if (!m_pRailTrack->Sample(fRailDist, &vPosition, &vTangent))
        return false;

    m_fSlopeRatio = -XMVectorGetY(XMVector3Normalize(XMLoadFloat3(&vTangent)));

    _vector vLook = XMVector3Normalize(XMLoadFloat3(&vTangent));
    _vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    _vector vRight = XMVector3Normalize(XMVector3Cross(vWorldUp, vLook));
    _vector vBaseUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

    _vector vLeftRightQuaternion = XMQuaternionRotationAxis(vLook, XMConvertToRadians(-m_fLeftRightDegree));
    _vector vLeftRightUp = XMVector3Rotate(vBaseUp, vLeftRightQuaternion);

    CTransform* pTransform = pKirby->Get_Transform();
    pTransform->LookTo(vLook, vLeftRightUp);
    pTransform->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&vPosition), 1.f));
    pKirby->Get_Movement()->Sync_To_Controller();

    return true;
}

void CKirby_Deform_RollerCoaster::Set_RotLeftRight(_float fTimeDelta)
{
    constexpr _float fMaxDegree = 55.f;

    Helper::IntClamp(m_iLeftRight, -1, 1);
    const _float fTargetDegree = static_cast<_float>(m_iLeftRight) * fMaxDegree;

    _float fRotDelta = fTargetDegree - m_fLeftRightDegree;

    constexpr _float fRotSpeed = 500.f;
    Helper::FloatClamp(fRotDelta, -fRotSpeed * fTimeDelta, fRotSpeed * fTimeDelta);
    
    m_fLeftRightDegree += fRotDelta;

    m_iLeftRight = 0;
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
