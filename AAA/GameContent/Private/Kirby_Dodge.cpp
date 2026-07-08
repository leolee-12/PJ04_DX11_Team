#include "Kirby_Dodge.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"
#include "Kirby_Deform.h"

#include "Movement_Child.h"

CKirby_Dodge::CKirby_Dodge()
{
}

HRESULT CKirby_Dodge::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_fSlowMaxTime = 0.6f;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Dodge::Get_StateType()
{
    return KIRBY_STATE_TYPE::DODGE;
}

void CKirby_Dodge::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    m_eDodgeFlag = static_cast<DODGE_STATE_FLAG>(iFlag);

    _vector vCamForward =XMVector3Normalize(
        XMVectorSetY(XMLoadFloat4(m_pGameInstance_Proxy->Get_CamLook()),0.f));
    _vector vCamRight = XMVector3Normalize(
        XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vCamForward));

    _vector vDodgeDir = XMVectorZero();

    switch (m_eDodgeFlag)
    {
        case DODGE_STATE_FLAG::DODGE_FRONT:
            vDodgeDir = vCamForward;
            break;

        case DODGE_STATE_FLAG::DODGE_BACK:
            vDodgeDir = -vCamForward;
            break;

        case DODGE_STATE_FLAG::DODGE_LEFT:
            vDodgeDir = -vCamRight;
            break;

        case DODGE_STATE_FLAG::DODGE_RIGHT:
            vDodgeDir = vCamRight;
            break;
    }

    vDodgeDir = XMVector3Normalize(vDodgeDir);
    XMStoreFloat3(&m_vDodgeDir, vDodgeDir);

    Set_DodgeAniDir(pKirby);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_MaxHorizontalSpeed(s_fDodgeMaxHorizontalSpeed);

    m_iEvasionCount = 1;

    Change_DodgeState(pKirby, Dodge_State::DODGE_START);
}

void CKirby_Dodge::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    Update_SlowTimer(fTimeDelta);
    Update_DodgeState(pKirby, fTimeDelta);
}

void CKirby_Dodge::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);

    pKirby->Put_WeaponOnBack(false);

    m_eDodgeFlag = DODGE_STATE_FLAG::DODGE_NONE;
}

_bool CKirby_Dodge::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    return false;
}

void CKirby_Dodge::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
    if(m_iEvasionCount > 0)
    {
        --m_iEvasionCount;
        m_fSlowAccTime = m_fSlowMaxTime;
        m_pGameInstance_Proxy->Lerp_TimeScale(0.5f, 1.f, 2.f);
    }
}

void CKirby_Dodge::Change_DodgeState(CKirby* pKirby, Dodge_State eNext)
{
    if (m_eDodgeState == eNext)
        return;

    Exit_DodgeState(pKirby, m_eDodgeState);

    m_eDodgeState = eNext;

    Enter_DodgeState(pKirby, m_eDodgeState);
}

void CKirby_Dodge::Enter_DodgeState(CKirby* pKirby, Dodge_State eState)
{
    _string strAnimationName{};

    _float fAniSpeed = 1.f;

    switch (eState)
    {
        case Dodge_State::DODGE_START:
        {
            pKirby->Get_KirbyAbility()->Play_AbilityAni(pKirby, ABILITY_ANI::DODGE_START);
            return;
        }
        case Dodge_State::DODGE1:
        {
            pKirby->Put_WeaponOnBack(true);

            switch (m_eDodgeAniDir)
            {
                case DODGE_ANI_DIR::ANI_LEFT:   strAnimationName = "DodgeLeft1"; break;
                case DODGE_ANI_DIR::ANI_RIGHT:  strAnimationName = "DodgeRight1"; break;
                case DODGE_ANI_DIR::ANI_FRONT:  strAnimationName = "DodgeFront1"; break;
                case DODGE_ANI_DIR::ANI_BACK:   strAnimationName = "DodgeBack1"; break;
            }
            pKirby->Get_Movement()->Try_Jump(s_fDodgeJumpSpeed);
            fAniSpeed = 2.f;
            break;
        }
        case Dodge_State::DODGE2:
        {
            switch (m_eDodgeAniDir)
            {
                case DODGE_ANI_DIR::ANI_LEFT:   strAnimationName = "DodgeLeft2"; break;
                case DODGE_ANI_DIR::ANI_RIGHT:  strAnimationName = "DodgeRight2"; break;
                case DODGE_ANI_DIR::ANI_FRONT:  strAnimationName = "DodgeFront2"; break;
                case DODGE_ANI_DIR::ANI_BACK:   strAnimationName = "DodgeBack2"; break;
            }
            fAniSpeed = 1.5f;
            pKirby->Get_Movement()->Try_Jump(s_fDodgeJumpSpeed);
            break;
        }
        case Dodge_State::DODGE_END:
        {
            Transition_Fall_OR_Wait_OR_Run(pKirby);
            break;
        }
    }

    if (strAnimationName.empty())
        return;

    CKirby_Body* pBody = pKirby->Get_Body();
    pBody->Get_Animator()->Play(strAnimationName, false, true, 0.1f, fAniSpeed);
}

void CKirby_Dodge::Update_DodgeState(CKirby* pKirby, _float fTimeDelta)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    switch (m_eDodgeState)
    {
        case Dodge_State::DODGE_START:
            if(pAnimator->Is_Finished())
                Change_DodgeState(pKirby, Dodge_State::DODGE1);

            break;

        case Dodge_State::DODGE1:
        {
            DodgeMove(pKirby);

            CMovement_Child* pMovement = pKirby->Get_Movement();
            _bool bIsGound = pMovement->Is_Grounded();

            if (bIsGound)
                Change_DodgeState(pKirby, Dodge_State::DODGE2);
            else if (pAnimator->Is_Finished())
                Transition_Fall_OR_Wait_OR_Run(pKirby);

            break;
        }

        case Dodge_State::DODGE2:
        {
            _float fRatio = pAnimator->Get_Progress();
            if (fRatio <= 0.85f)
                DodgeMove(pKirby);
            else
                pKirby->Get_Movement()->Stop_Horizontal();

            if (pAnimator->Is_Finished())
                Change_DodgeState(pKirby, Dodge_State::DODGE_END);
            break;
        }
    }
}

void CKirby_Dodge::Exit_DodgeState(CKirby* pKirby, Dodge_State eState)
{
    switch (eState)
    {
        case Dodge_State::DODGE_START:
            break;
        case Dodge_State::DODGE1:
            break;
        case Dodge_State::DODGE2:
            break;
        case Dodge_State::DODGE_END:
            break;
    }
}

void CKirby_Dodge::Set_DodgeAniDir(CKirby* pKirby)
{
    _vector vDodgeDir = XMLoadFloat3(&m_vDodgeDir);

    _vector vKirbyLook = XMVector3Normalize(
        XMVectorSetY(pKirby->Get_Transform()->Get_State(STATE::LOOK), 0.f));
    _vector vKirbyRight = XMVector3Normalize(
        XMVectorSetY(pKirby->Get_Transform()->Get_State(STATE::RIGHT), 0.f));

    _float fLookDot = XMVectorGetX(XMVector3Dot(vDodgeDir, vKirbyLook));
    _float fRightDot = XMVectorGetX(XMVector3Dot(vDodgeDir, vKirbyRight));

    if (fabsf(fLookDot) >= fabsf(fRightDot))
    {
        if (fLookDot >= 0)
            m_eDodgeAniDir = DODGE_ANI_DIR::ANI_FRONT;
        else
            m_eDodgeAniDir = DODGE_ANI_DIR::ANI_BACK;
    }
    else
    {
        if (fRightDot >= 0)
            m_eDodgeAniDir = DODGE_ANI_DIR::ANI_RIGHT;
        else
            m_eDodgeAniDir = DODGE_ANI_DIR::ANI_LEFT;
    }
}

void CKirby_Dodge::DodgeMove(CKirby* pKirby)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Add_Acceleration(XMVectorSet(0.f, -120.f, 0.f, 0.f));
    constexpr _float fSpeed = 15.f;
    pMovement->Set_VelocityX(m_vDodgeDir.x * fSpeed);
    pMovement->Set_VelocityZ(m_vDodgeDir.z * fSpeed);
}

void CKirby_Dodge::Update_SlowTimer(_float fTimeDelta)
{
    if (m_fSlowAccTime > 0) {
        m_fSlowAccTime -= fTimeDelta;

        if (m_fSlowAccTime <= 0.f)
        {
            m_fSlowAccTime = 0.f;
            m_pGameInstance_Proxy->Set_TimeScale(1.f);
        }
    }
}

CKirby_Dodge* CKirby_Dodge::Create()
{
    CKirby_Dodge* pInstance = new CKirby_Dodge();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Dodge");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Dodge::Free()
{
    __super::Free();
}
