#include "Kirby_Deform_Car.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

#include "Effect_Loader.h"

#include "Movement_Child.h"

#include "Kirby_Deform_Model.h"

CKirby_Deform_Car::CKirby_Deform_Car()
{
}

HRESULT CKirby_Deform_Car::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_wstrAttackModeName = L"자동차 머금기";

    Set_FullBodyAni(DEFORM_ANI::WAIT, "Idling", true, false, 0.1f, 1.5f);
    Set_FullBodyAni(DEFORM_ANI::RUN, "Moving", true, false, 0.1f, 1.5f);

    Set_FullBodyAni(DEFORM_ANI::FALL, "Fall", true, false, 0.1f, 1.5f);
    Set_FullBodyAni(DEFORM_ANI::LANDING, "Landing", false, false, 0.1f, 2.f);

    Set_FullBodyAni(DEFORM_ANI::JUMP_START, "JumpStart", false, false, 0.1f, 1.5f);
    Set_FullBodyAni(DEFORM_ANI::JUMP, "Jump", false, false, 0.1f, 1.5f);

    Set_FullBodyAni(DEFORM_ANI::DAMAGED, "Damage", false, false, 0.1f, 1.5f);

    Set_FullBodyAni(DEFORM_ANI::SPIT_START, "SpitStart", false, false, 0.1f, 1.5f);

    m_fMaxBoostTime = 0.5f;

    return S_OK;
}

DEFORM_TYPE CKirby_Deform_Car::Get_DeformType()
{
    return DEFORM_TYPE::CAR;
}

void CKirby_Deform_Car::Enter_Deform(CKirby* pKirby)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_RotationSpeed(s_fDeformCar_Rot_Speed_Degree);
    pMovement->Set_MaxHorizontalSpeed(s_fCarSpeed);
    pMovement->Set_MaxFallVelocity(s_fMaxCarFallVelocity);

    // CCT
    pKirby->Set_CCTSize(s_fCar_CCT_Radius, s_fCar_CCT_Height);

    // Hurt Box
    CCollider::COLLIDER_DESC tHurtDesc{};
    tHurtDesc.pOwner = pKirby;
    tHurtDesc.vCenter = { 0.f, 0.f, 0.f };
    tHurtDesc.fRadius = s_fCar_CCT_Radius + CKirby::s_fHurtBoxRadiusPadding;
    tHurtDesc.fHeight = s_fCar_CCT_Height;
    pKirby->Set_ColliderDesc(CKirby::HURT_BOX, tHurtDesc);

    // Hit Box
    CCollider::COLLIDER_DESC BreakerableHitBoxDesc{};
    BreakerableHitBoxDesc.pOwner = pKirby;
    BreakerableHitBoxDesc.vCenter = _float3(0.f, -1.f, 1.5f);
    BreakerableHitBoxDesc.fRadius = 2.5f;
    BreakerableHitBoxDesc.fHeight = 0.f;
    pKirby->Set_ColliderDesc(CKirby::BREAKERABLE_HITBOX, BreakerableHitBoxDesc);

    m_fRollDegree = 0.f;
}

void CKirby_Deform_Car::Exit_Deform(CKirby* pKirby)
{
    pKirby->Set_ColliderEnabled(CKirby::BREAKERABLE_HITBOX, false);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_RotationSpeed(CKirby::s_fRot_Speed_Degree);
    pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
    pMovement->Set_MaxFallVelocity(CKirby::s_fMaxFallVelocity);

    pKirby->Set_CCTSize(CKirby::s_fCCT_Radius, CKirby::s_fCCT_Height);

    // Hurt Box
    CCollider::COLLIDER_DESC tHurtDesc{};
    tHurtDesc.pOwner = pKirby;
    tHurtDesc.vCenter = _float3(0.f, 0.f, 0.f);
    tHurtDesc.fRadius = CKirby::s_fCCT_Radius + CKirby::s_fHurtBoxRadiusPadding;
    tHurtDesc.fHeight = CKirby::s_fCCT_Height;
    pKirby->Set_ColliderDesc(CKirby::HURT_BOX, tHurtDesc);
}

void CKirby_Deform_Car::Enter_AttackState(CKirby* pKirby, _int iFlag)
{
    m_fAccBoostTime = m_fMaxBoostTime;
    m_bReqEndAttackState = false;

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_MaxHorizontalSpeed(s_fMaxBoostSpeed);

    if(pMovement->Is_Grounded())
        m_eBoostJumpState = BOOST_JUMP_STATE::GROUND;
    else
        m_eBoostJumpState = BOOST_JUMP_STATE::FALL;

    m_eDeformCar_State = DEFORM_CAR_STATE::DEFORM_CAR_END;
    Change_DeformCarState(pKirby, DEFORM_CAR_STATE::BOOST);
}

void CKirby_Deform_Car::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
    Update_DeformCarState(pKirby, fTimeDelta);
}

void CKirby_Deform_Car::Exit_AttackState(CKirby* pKirby)
{
    pKirby->Set_ColliderEnabled(CKirby::BREAKERABLE_HITBOX, false);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_MaxHorizontalSpeed(s_fCarSpeed);

    Effect_Stop(m_pBoostGas);
    Effect_Stop(m_pBoostWind);
}

_bool CKirby_Deform_Car::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();
    //m_vRotDir
    switch (eCommandType)
    {
        // Move Press
        case KIRBY_COMMAND_TYPE::MOVE_TOP:
        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
        {
            if (!pCommand->IsPress())
                return false;

            Move_Command* pMoveCommand = static_cast<Move_Command*>(pCommand);

            if (m_eDeformCar_State == DEFORM_CAR_STATE::BOOST_END)
            {
                pKirby->Add_MoveDir(pMoveCommand->Get_Dir());

                pKirby->Change_State(KIRBY_STATE_TYPE::RUN);
                return true;
            }

            if (m_eDeformCar_State == DEFORM_CAR_STATE::BOOST)
            {
                _vector vWishDir = XMLoadFloat3(&m_vRotDir);
                vWishDir += XMLoadFloat3(&pMoveCommand->Get_Dir());

                XMStoreFloat3(&m_vRotDir, vWishDir);

                return true;
            }

            return true;
        }

        // Attack
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (!pCommand->IsPress())
                return false;

            m_fAccBoostTime = m_fMaxBoostTime;
            return true;
        }

        // Jump Down
        case KIRBY_COMMAND_TYPE::JUMP:
        {
            if (!pCommand->IsDown())
                return false;

            if (m_eDeformCar_State == DEFORM_CAR_STATE::BOOST_END)
            {
                pKirby->Change_State(KIRBY_STATE_TYPE::JUMP);
                return true;
            }

            if (m_eDeformCar_State == DEFORM_CAR_STATE::BOOST)
            {
                CMovement_Child* pMovement = pKirby->Get_Movement();

                if (pMovement->Try_Jump())
                    Change_BoostJumpState(pKirby, BOOST_JUMP_STATE::JUMP_START);

                return true;
            }

            return true;
        }
    }

    return false;
}

_bool CKirby_Deform_Car::Enter_Attack_KeyDown(CKirby* pKirby)
{
    pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);
    return true;
}

_bool CKirby_Deform_Car::Enter_Attack_KeyPress(CKirby* pKirby)
{
    // 무시
    return true;
}

_bool CKirby_Deform_Car::Enter_Attack_KeyUp(CKirby* pKirby)
{
    // 무시
    return true;
}

void CKirby_Deform_Car::Update_RunState(CKirby* pKirby, _float fTimeDelta)
{
    Roll_Wheel(pKirby, fTimeDelta);
}

void CKirby_Deform_Car::Change_DeformCarState(CKirby* pKirby, DEFORM_CAR_STATE eNext)
{
    if (m_eDeformCar_State == eNext)
        return;

    Exit_DeformCarState(pKirby, m_eDeformCar_State);

    m_eDeformCar_State = eNext;

    Enter_DeformCarState(pKirby, m_eDeformCar_State);
}

void CKirby_Deform_Car::Enter_DeformCarState(CKirby* pKirby, DEFORM_CAR_STATE eState)
{
    CKirby_Deform_Model* pModel = pKirby->Get_DeformPart_Model(DEFORM_TYPE::CAR);

    switch (m_eDeformCar_State)
    {
        case DEFORM_CAR_STATE::BOOST:
        {
            pModel->Get_Animator()->Play("Boost", true, false, 0.1f, 2.f);

            pModel->Set_KirbyEye(KIRBY_EYE_STATE::ANGRY);

            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_MaxHorizontalSpeed(s_fMaxBoostSpeed);

            BoostEffectStart(pKirby, m_pBoostGas, L"BoostGas");

            pKirby->Set_ColliderEnabled(CKirby::BREAKERABLE_HITBOX, true);

            CEffect_Loader::GetInstance()->Spawn(L"CarMilkyWay", pKirby->Get_LevelIndex(),
                _float3(0.f, 1.3f, 2.4f), _float3(0.f, 0.f, 0.f), _float3(0.f, 180.f, 0.f),
                pKirby->Get_RenderWorldMatrixPtr(), &m_pBoostWind);

            m_fMinimumBoostTime = 0.2f;
            break;
        }
        case DEFORM_CAR_STATE::BOOST_END:
        {
            pModel->Get_Animator()->Play("BoostEnd", false, false, 0.1f, 2.f);
            break;
        }
        case DEFORM_CAR_STATE::CRASH:
        {
            CMovement_Child* pMovement = pKirby->Get_Movement();

            _vector vBackDir = -pKirby->Get_Transform()->Get_State(STATE::LOOK);
            vBackDir = XMVector3Normalize(XMVectorSetY(vBackDir, 0.f));
            pMovement->Set_Velocity(vBackDir * 17.f);

            _float fJumpVelocity{};
            if (pMovement->Is_Grounded())
                fJumpVelocity = 20.f;
            else
                fJumpVelocity = 10.f;
            pMovement->Add_Velocity(XMVectorSet(0.f, fJumpVelocity, 0.f, 0.f));

            pModel->Get_Animator()->Play("Crash", false, false, 0.1f, 1.5f);
            break;
        }
        case DEFORM_CAR_STATE::DEFORM_CAR_END:
            m_bReqEndAttackState = true;
            break;
    }
}

void CKirby_Deform_Car::Update_DeformCarState(CKirby* pKirby, _float fTimeDelta)
{
    CKirby_Deform_Model* pModel = pKirby->Get_DeformPart_Model(DEFORM_TYPE::CAR);
    CMovement_Child* pMovement = pKirby->Get_Movement();

    switch (m_eDeformCar_State)
    {
        case DEFORM_CAR_STATE::BOOST:
        {
            Roll_Wheel(pKirby, fTimeDelta);
            Update_BoostJumpState(pKirby, fTimeDelta);

            // 회전
            _vector vRotDir = XMLoadFloat3(&m_vRotDir);
            if (XMVectorGetX(XMVector3LengthSq(vRotDir)) > Helper::fEpsilon)
                pMovement->Rotate_To_Direction(vRotDir, fTimeDelta);

            // 이동
            _vector vLook = pKirby->Get_Transform()->Get_State(STATE::LOOK);
            vLook = XMVector3Normalize(XMVectorSetY(vLook, 0.f));

            _vector vBoostAcceleration = vLook * s_fBoostAcceleration;
            pMovement->Add_Acceleration(vBoostAcceleration);

            // 살짝 Boost 진행된 후 충돌

            if (Check_FrontCollision(pKirby) == true && m_fMinimumBoostTime <= 0.f)
                Change_DeformCarState(pKirby, DEFORM_CAR_STATE::CRASH);

            if (m_fMinimumBoostTime > 0.f)
                m_fMinimumBoostTime -= fTimeDelta;

            m_fAccBoostTime -= fTimeDelta;

            if (m_eBoostJumpState != BOOST_JUMP_STATE::GROUND)
                break;

            if (m_fAccBoostTime <= 0.f)
            {
                m_fAccBoostTime = 0.f;
                Change_DeformCarState(pKirby, DEFORM_CAR_STATE::BOOST_END);
            }
            break;
        }
        case DEFORM_CAR_STATE::BOOST_END:
        {
            Roll_Wheel(pKirby, fTimeDelta);

            // 회전
            _vector vRotDir = XMLoadFloat3(&m_vRotDir);
            if (XMVectorGetX(XMVector3LengthSq(vRotDir)) > Helper::fEpsilon)
                pMovement->Rotate_To_Direction(vRotDir, fTimeDelta);

            if (pModel->Get_Animator()->Is_Finished())
                Change_DeformCarState(pKirby, DEFORM_CAR_STATE::DEFORM_CAR_END);
            break;
        }
        case DEFORM_CAR_STATE::CRASH:
        {
            if (pModel->Get_Animator()->Is_Finished())
                Change_DeformCarState(pKirby, DEFORM_CAR_STATE::DEFORM_CAR_END);
            break;
        }
    }

    m_vRotDir = {};
}

void CKirby_Deform_Car::Exit_DeformCarState(CKirby* pKirby, DEFORM_CAR_STATE eStaten)
{
    CKirby_Deform_Model* pModel = pKirby->Get_DeformPart_Model(DEFORM_TYPE::CAR);
    CMovement_Child* pMovement = pKirby->Get_Movement();
    
    switch (m_eDeformCar_State)
    {
        case DEFORM_CAR_STATE::BOOST:
        {
            pModel->Set_KirbyEye(KIRBY_EYE_STATE::IDLE);

            pKirby->Set_ColliderEnabled(CKirby::BREAKERABLE_HITBOX, false);

            Effect_Stop(m_pBoostGas);
            Effect_Stop(m_pBoostWind);

            pModel->Stop_SoundHandle();

            break;
        }
        case DEFORM_CAR_STATE::BOOST_END:
            pMovement->Set_MaxHorizontalSpeed(s_fCarSpeed);
            break;
        case DEFORM_CAR_STATE::CRASH:
            break;
    }
}

void CKirby_Deform_Car::Change_BoostJumpState(CKirby* pKirby, BOOST_JUMP_STATE eNext)
{
    if (m_eBoostJumpState == eNext)
        return;

    Exit_BoostJumpState(pKirby, m_eBoostJumpState);

    m_eBoostJumpState = eNext;

    Enter_BoostJumpState(pKirby, m_eBoostJumpState);
}

void CKirby_Deform_Car::Enter_BoostJumpState(CKirby* pKirby, BOOST_JUMP_STATE eState)
{
    switch (m_eBoostJumpState)
    {
        case BOOST_JUMP_STATE::GROUND:
        {
            CKirby_Deform_Model* pModel = pKirby->Get_DeformPart_Model(DEFORM_TYPE::CAR);
            pModel->Get_Animator()->Play("Boost", true, false, 0.1f, 2.f);
            pModel->Set_KirbyEye(KIRBY_EYE_STATE::ANGRY);
            break;
        }
        case BOOST_JUMP_STATE::JUMP_START:
        {
            CKirby_Deform_Model* pModel = pKirby->Get_DeformPart_Model(DEFORM_TYPE::CAR);
            Play_DeformAni(pKirby, DEFORM_ANI::JUMP_START);
            pModel->Set_KirbyEye(KIRBY_EYE_STATE::IDLE);
        }
            break;

        case BOOST_JUMP_STATE::JUMP:
            Play_DeformAni(pKirby, DEFORM_ANI::JUMP);
            break;

        case BOOST_JUMP_STATE::FALL:
            Play_DeformAni(pKirby, DEFORM_ANI::FALL);
            break;

        case BOOST_JUMP_STATE::LANDING:
            Play_DeformAni(pKirby, DEFORM_ANI::LANDING);
            break;
    }
}

void CKirby_Deform_Car::Update_BoostJumpState(CKirby* pKirby, _float fTimeDelta)
{
    CKirby_Deform_Model* pModel = pKirby->Get_DeformPart_Model(DEFORM_TYPE::CAR);
    CAnimator* pAnimator = pModel->Get_Animator();
    CMovement_Child* pMovement = pKirby->Get_Movement();

    switch (m_eBoostJumpState)
    {
    case BOOST_JUMP_STATE::GROUND:
        break;

    case BOOST_JUMP_STATE::JUMP_START:
        if (pAnimator->Is_Finished())
            Change_BoostJumpState(pKirby, BOOST_JUMP_STATE::JUMP);
        break;

    case BOOST_JUMP_STATE::JUMP:
        if (pMovement->Get_VerticalVelocity() <= 0.f)
            Change_BoostJumpState(pKirby, BOOST_JUMP_STATE::FALL);
        break;

    case BOOST_JUMP_STATE::FALL:
        if (pMovement->Is_Grounded())
            Change_BoostJumpState(pKirby, BOOST_JUMP_STATE::LANDING);
        break;

    case BOOST_JUMP_STATE::LANDING:
        if (pAnimator->Is_Finished())
            Change_BoostJumpState(pKirby, BOOST_JUMP_STATE::GROUND);
        break;
    }
}

void CKirby_Deform_Car::Exit_BoostJumpState(CKirby* pKirby, BOOST_JUMP_STATE eStaten)
{
    switch (m_eBoostJumpState)
    {
    case BOOST_JUMP_STATE::GROUND:
        break;

    case BOOST_JUMP_STATE::JUMP_START:
        break;

    case BOOST_JUMP_STATE::JUMP:
        break;

    case BOOST_JUMP_STATE::FALL:
        break;

    case BOOST_JUMP_STATE::LANDING:
        break;
    }
}

void CKirby_Deform_Car::BoostEffectStart(CKirby* pKirby, CEffect_Container*& pContainer, const _tchar* EffectTag)
{
    CEffect_Loader::GetInstance()->Spawn(EffectTag, pKirby->Get_LevelIndex(),
        _float3(0.f, 0.35f, -3.3f), _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f),
        pKirby->Get_RenderWorldMatrixPtr(), &pContainer);
}

_bool CKirby_Deform_Car::Check_FrontCollision(CKirby* pKirby)
{
    CTransform* pTransform = pKirby->Get_Transform();

    _float3 vCenter{};
    XMStoreFloat3(&vCenter, pTransform->Get_State(STATE::POSITION));
    vCenter.y += 2.f;

    _float3 vDir{};
    XMStoreFloat3(&vDir, XMVector3Normalize(pTransform->Get_State(STATE::LOOK)));

    _float fRadius = 0.1f;
    _float3 vDumpNormal{};
    _float  fDumpDistance{};
    return m_pGameInstance_Proxy->Sweep_Sphere(
        vCenter, fRadius, vDir,
        3.f,
        &vDumpNormal, &fDumpDistance, true, false);
}

void CKirby_Deform_Car::Roll_Wheel(CKirby* pKirby, _float fTimeDelta)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    _float3 vVelocity = pMovement->Get_Velocity();
    _vector vSpeedXZ = XMLoadFloat3(&vVelocity);
    vSpeedXZ = XMVectorSetY(vSpeedXZ, 0.f);

    _float fSpeedXZ = XMVectorGetX(XMVector3Length(vSpeedXZ));
    if (fSpeedXZ < Helper::fEpsilon)
        return;

    constexpr _float fRotScale = 70.f;
    m_fRollDegree += fSpeedXZ * fRotScale * fTimeDelta;
    m_fRollDegree = fmodf(m_fRollDegree, 360.f);

    CAnimator* pAnimator = pKirby->Get_CurrentDeformModel()->Get_Animator();
    pAnimator->SetBoneRotation("L_BTireJ", m_fRollDegree, XMVectorSet(-1.f, 0.f, 0.f, 0.f));
    pAnimator->SetBoneRotation("L_FTireJ", m_fRollDegree, XMVectorSet(-1.f, 0.f, 0.f, 0.f));
    pAnimator->SetBoneRotation("R_BTireJ", m_fRollDegree, XMVectorSet(-1.f, 0.f, 0.f, 0.f));
    pAnimator->SetBoneRotation("R_FTireJ", m_fRollDegree, XMVectorSet(-1.f, 0.f, 0.f, 0.f));
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
