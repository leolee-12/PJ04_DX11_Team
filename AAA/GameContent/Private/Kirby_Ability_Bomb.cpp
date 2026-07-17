#include "Kirby_Ability_Bomb.h"

#include "GameInstance.h"
#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

#include "Projectile_Manager.h"
#include "KirbyBomb.h"

#include "Effect_Loader.h"

CKirby_Ability_Bomb::CKirby_Ability_Bomb()
{
}

HRESULT CKirby_Ability_Bomb::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_wstrAttackModeName = L"밤";

    return S_OK;
}

COPY_ABILITY_TYPE CKirby_Ability_Bomb::Get_AbilityType()
{
    return COPY_ABILITY_TYPE::BOMB;
}

void CKirby_Ability_Bomb::Enter_AttackState(CKirby* pKirby, _int iFlag)
{
    m_bKeyUp = false;
    m_bReserveAttack = false;
    m_bReserveKeyUp = false;

    m_eBombState = BOMB_STATE::BOMB_STATE_END;
    
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_MaxHorizontalSpeed(s_fBombMaxHorizontalSpeed);

    _bool bIsGround = pMovement->Is_Grounded();

    if(!bIsGround)
        Change_BombState(pKirby, BOMB_STATE::CHARGE_START_FALL);
    else if(pKirby->Has_MoveDir())
        Change_BombState(pKirby, BOMB_STATE::MOVE_THROW);
    else
        Change_BombState(pKirby, BOMB_STATE::CHARGE_START);

    m_bReqEndAttackState = false;
}

void CKirby_Ability_Bomb::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
    Update_BombState(pKirby, fTimeDelta);
}

void CKirby_Ability_Bomb::Exit_AttackState(CKirby* pKirby)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);

    m_bKeyUp = false;
    m_bReserveAttack = false;
    m_bReserveKeyUp = false;
}

_bool CKirby_Ability_Bomb::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        case KIRBY_COMMAND_TYPE::MOVE_TOP:
        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
        {
            if (!pCommand->IsPress())
                return false;

            Move_Command* pMoveCommand = static_cast<Move_Command*>(pCommand);
            if (m_eBombState == BOMB_STATE::MOVE_THROW)
            {
                pKirby->Add_MoveDir(pMoveCommand->Get_Dir());
                return true;
            }
            else if (m_eBombState == BOMB_STATE::CHARGING ||         
                m_eBombState == BOMB_STATE::CHARGING_LANDING)
            {
                Acc_AimInput(pMoveCommand->Get_Dir());
                return true;
            }
            else if (m_eBombState == BOMB_STATE::CHARGING_FALL)
            {
                pKirby->Add_MoveDir(pMoveCommand->Get_Dir());
                Acc_AimInput(pMoveCommand->Get_Dir());
                return true;
            }

            return true;
        }
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (pCommand->IsUp())
            {
                if (m_bReserveAttack == true)
                    m_bReserveKeyUp = true;
                else 
                    m_bKeyUp = true;
            }
            else if (pCommand->IsDown())
            {
                if (!m_bReserveAttack)
                {
                    m_bReserveAttack = true;
                    m_bReserveKeyUp = false;
                }
            }
            return true;
        }
    }

    return false;
}

_bool CKirby_Ability_Bomb::Enter_Attack_KeyDown(CKirby* pKirby)
{
    pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);

    m_bKeyUp = false;

    return true;
}

_bool CKirby_Ability_Bomb::Enter_Attack_KeyPress(CKirby* pKirby)
{
    // 무시
    return true;
}

_bool CKirby_Ability_Bomb::Enter_Attack_KeyUp(CKirby* pKirby)
{
    // 무시
    return true;
}

void CKirby_Ability_Bomb::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
    __super::On_Damaged_KirbyState(pKirby, tInfo);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_MaxFallVelocity(CKirby::s_fMaxFallVelocity);
    pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
    pMovement->Set_RotationSpeed(CKirby::s_fRot_Speed_Degree);

    if (m_pBomb != nullptr)
    {
        m_pBomb->Despawn();
        m_pBomb = nullptr;
    }

    Despawn_BombHitAimEffect();
    Despawn_BombAimDots();
}

void CKirby_Ability_Bomb::Change_BombState(CKirby* pKirby, BOMB_STATE eNext)
{
    if (m_eBombState == eNext)
        return;

    Exit_BombState(pKirby, m_eBombState);

    m_eBombState = eNext;

    Enter_BombState(pKirby, m_eBombState);
}

void CKirby_Ability_Bomb::Enter_BombState(CKirby* pKirby, BOMB_STATE eState)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    CMovement_Child* pMovement = pKirby->Get_Movement();

    switch (eState)
    {
        case BOMB_STATE::BOMB_STATE_END:
            if (!Handle_ReserveAttack(pKirby))
                m_bReqEndAttackState = true;
            break;

        case BOMB_STATE::MOVE_THROW:
            pAnimator->Play("BombShoot", false, true, 0.1f, 2.f);
            Spawn_Bomb(pKirby);
            break;

        case BOMB_STATE::CHARGE_START:
            pMovement->Set_RotationSpeed(s_fBombRot_Speed_Degree);
            pAnimator->Play("BombThrowCharge", true, false, 0.1f, 1.5f);
            Reset_Aim(pKirby);
            Spawn_Bomb(pKirby);
            m_bPlayingRotAni = false;
            m_bAimRotating = false;
            break;
        case BOMB_STATE::CHARGING:
            pAnimator->Play("BombThrowCharge", true, false, 0.1f, 1.5f);
            break;

        case BOMB_STATE::CHARGE_START_FALL:
        {
            pMovement->Set_RotationSpeed(s_fBombRot_Speed_Degree);
            pMovement->Set_MaxFallVelocity(s_fBombMaxFallVelocity);

            pAnimator->Play("BombFall", false, false, 0.1f, 1.5f);
            Reset_Aim(pKirby);
            Spawn_Bomb(pKirby);
            m_bPlayingRotAni = false;
            m_bAimRotating = false;
            break;
        }
        case BOMB_STATE::CHARGING_FALL:
            pAnimator->Play("BombFall", false, false, 0.1f, 1.5f);
            break;

        case BOMB_STATE::CHARGING_LANDING:
        {
            pMovement->Set_MaxFallVelocity(CKirby::s_fMaxFallVelocity);

            pAnimator->Play("BombLanding", false, false, 0.1f, 1.5f);
            break;
        }

        case BOMB_STATE::THROW:
            pAnimator->Play("BombThrow", false, false, 0.1f, 2.5f);
            Throw_Bomb(pKirby, 70.f, 15.f);
            break;
        case BOMB_STATE::CHARGING_THROW: // 임시
            pAnimator->Play("BombThrow", false, false, 0.1f, 2.5f);
            Throw_BombToAim();
            break;
    }
}

void CKirby_Ability_Bomb::Update_BombState(CKirby* pKirby, _float fTimeDelta)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();
    CMovement_Child* pMovement = pKirby->Get_Movement();
    _bool IsGround = pMovement->Is_Grounded();

    switch (m_eBombState)
    {
        case BOMB_STATE::MOVE_THROW:
        {
            if (pAnimator->Get_Progress() >= 0.35f)
                Throw_Bomb(pKirby, 10.f, 20.f);

            if (pAnimator->Is_Finished())
                Change_BombState(pKirby, BOMB_STATE::BOMB_STATE_END);
            break;
        }
        case BOMB_STATE::CHARGE_START:
        {
            if (!pAnimator->Is_Blending())
            {
                if(m_bKeyUp)
                    Change_BombState(pKirby, BOMB_STATE::THROW);
                else 
                    Change_BombState(pKirby, BOMB_STATE::CHARGING);
            }
            break;
        }
        case BOMB_STATE::CHARGING:
        {
            Cal_Aim(fTimeDelta);
            Update_AimFacing(pKirby, fTimeDelta);
            Update_AimPrediction();
            Update_BombAimDots(fTimeDelta);

            if (m_bKeyUp)
                Change_BombState(pKirby, BOMB_STATE::CHARGING_THROW);
            break;
        }
        case BOMB_STATE::CHARGE_START_FALL:
        {
            if (!pAnimator->Is_Blending())
            {
                if (m_bKeyUp)
                    Change_BombState(pKirby, BOMB_STATE::THROW);
                else
                    Change_BombState(pKirby, BOMB_STATE::CHARGING_FALL);
            }
            else if(IsGround)
            {
                Change_BombState(pKirby, BOMB_STATE::CHARGING_LANDING);
            }
            break;
        }
        case BOMB_STATE::CHARGING_FALL:
        {
            Cal_Aim(fTimeDelta);
            Update_AimFacing(pKirby, fTimeDelta);
            Update_AimPrediction();
            Update_BombAimDots(fTimeDelta);

            if (m_bKeyUp)
                Change_BombState(pKirby, BOMB_STATE::CHARGING_THROW);
            else if (IsGround)
                Change_BombState(pKirby, BOMB_STATE::CHARGING_LANDING);
            break;
        }
        case BOMB_STATE::CHARGING_LANDING:
        {
            Cal_Aim(fTimeDelta);
            Update_AimFacing(pKirby, fTimeDelta);
            Update_AimPrediction();
            Update_BombAimDots(fTimeDelta);

            if (m_bKeyUp)
                Change_BombState(pKirby, BOMB_STATE::CHARGING_THROW);
            else if (pAnimator->Is_Finished())
                Change_BombState(pKirby, BOMB_STATE::CHARGING);
            break;
        }
        case BOMB_STATE::THROW:
        {
            if (pAnimator->Is_Finished())
                Change_BombState(pKirby, BOMB_STATE::BOMB_STATE_END);
            break;
        }
        case BOMB_STATE::CHARGING_THROW: // 임시
        {
            if (pAnimator->Is_Finished())
                Change_BombState(pKirby, BOMB_STATE::BOMB_STATE_END);
            break;
        }
    }
}

void CKirby_Ability_Bomb::Exit_BombState(CKirby* pKirby, BOMB_STATE eState)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();

    switch (eState)
    {
    case BOMB_STATE::BOMB_STATE_END:
        break;

    case BOMB_STATE::MOVE_THROW:
        break;

    case BOMB_STATE::CHARGE_START:
        break;

    case BOMB_STATE::CHARGING:
        break;

    case BOMB_STATE::CHARGE_START_FALL:

        break;
    case BOMB_STATE::CHARGING_FALL:
        break;

    case BOMB_STATE::CHARGING_LANDING:
        break;

    case BOMB_STATE::THROW:
        pMovement->Set_RotationSpeed(CKirby::s_fRot_Speed_Degree);
        break;

    case BOMB_STATE::CHARGING_THROW:
        pMovement->Set_RotationSpeed(CKirby::s_fRot_Speed_Degree);
        break;
    }
}

_bool CKirby_Ability_Bomb::Handle_ReserveAttack(CKirby* pKirby)
{
    if (!m_bReserveAttack)
        return false;

    m_bKeyUp = m_bReserveKeyUp;
    m_bReserveAttack = false;
    m_bReserveKeyUp = false;

    if (pKirby->Has_MoveDir())
        Change_BombState(pKirby, BOMB_STATE::MOVE_THROW);
    else
        Change_BombState(pKirby, BOMB_STATE::CHARGE_START);

    return true;
}

void CKirby_Ability_Bomb::Spawn_Bomb(CKirby* pKirby)
{
    if (m_pBomb != nullptr)
        return;

    CProjectile* pProjectile{};

    if (FAILED(CProjectile_Manager::GetInstance()->Spawn(pKirby->Get_PrototypeLevelIndex(), pKirby->Get_LevelIndex(),
        CKirbyBomb::POOL_KEY, CKirbyBomb::PROTOTYPE_TAG, &pProjectile)))
        return;

    CKirbyBomb* pBomb = dynamic_cast<CKirbyBomb*>(pProjectile);

    if (pBomb == nullptr)
        return;

    const _float4x4* pSocketBone = pKirby->Get_Body()->Get_BoneMatrixPtr("RHaveL");

    pBomb->Attach_To_Socket(pSocketBone, pKirby->Get_Transform()->Get_WorldMatrixPtr(), pBomb->Get_PreRotInverse());

    m_pBomb = pBomb;
    m_bPredictedHit = false;
}

void CKirby_Ability_Bomb::Throw_Bomb(CKirby* pKirby, _float fDegree, _float fSpeed)
{
    if (m_pBomb == nullptr)
        return;

    _vector vPos = m_pBomb->Get_Transform()->Get_State(STATE::POSITION);

    _vector vLook = pKirby->Get_Transform()->Get_State(STATE::LOOK);
    vLook = XMVector3Normalize(XMVectorSetY(vLook, 0.f));

    _float fRadian = XMConvertToRadians(fDegree);
    _vector vDir = vLook * cosf(fRadian) + XMVectorSet(0.f, 1.f, 0.f, 0.f) * sinf(fRadian);

    _float3 vLaunchPos{};
    _float3 vLaunchDir{};

    XMStoreFloat3(&vLaunchPos, vPos);
    XMStoreFloat3(&vLaunchDir, vDir);

    m_pBomb->Launch(vLaunchPos, vLaunchDir, fSpeed);
    m_pBomb = nullptr;
}

void CKirby_Ability_Bomb::Throw_BombToAim()
{
    if (m_pBomb == nullptr)
        return;

    _float3 vStart{};
    XMStoreFloat3(&vStart, m_pBomb->Get_Transform()->Get_State(STATE::POSITION));

    m_pBomb->Launch_Velocity(vStart, m_vAimLaunchVelocity);  
    m_pBomb = nullptr;
    m_bPredictedHit = false;

    Despawn_BombHitAimEffect();
    Despawn_BombAimDots();
}

void CKirby_Ability_Bomb::Reset_Aim(CKirby* pKirby)
{
    m_vAimInput = {};
    m_vAimLaunchVelocity = {};

    _vector vKirbyPos = pKirby->Get_Transform()->Get_State(STATE::POSITION);

    _vector vLook = pKirby->Get_Transform()->Get_State(STATE::LOOK);
    vLook = XMVector3Normalize(XMVectorSetY(vLook, 0.f));

    constexpr _float fDist = 8.f;
    _vector vTarget = vKirbyPos + vLook * fDist;

    XMStoreFloat3(&m_vAimTargetPos, vTarget);
}

void CKirby_Ability_Bomb::Acc_AimInput(const _float3& vInputDir)
{
    _vector vInput = XMLoadFloat3(&m_vAimInput) + XMLoadFloat3(&vInputDir);
    XMStoreFloat3(&m_vAimInput, vInput);
}

void CKirby_Ability_Bomb::Cal_Aim(_float fTimeDelta)
{
    if (m_pBomb == nullptr)
        return;

    _vector vInput = XMLoadFloat3(&m_vAimInput);

    if (XMVectorGetX(XMVector3LengthSq(vInput)) > Helper::fEpsilon)
    {
        vInput = XMVector3Normalize(vInput);

        _vector vTarget = XMLoadFloat3(&m_vAimTargetPos);

        constexpr _float fSpeed = 10.f;
        vTarget += vInput * fSpeed * fTimeDelta;

        constexpr _float fMin = 0.7f;
        constexpr _float fMax = 10.f;

        _vector vBombPos = m_pBomb->Get_Transform()->Get_State(STATE::POSITION);
        _vector vBombToTargetXZ = XMVectorSetY(vTarget - vBombPos, 0.f);
        _float fLengthSq = XMVectorGetX(XMVector3LengthSq(vBombToTargetXZ));
        _float fTargetY = XMVectorGetY(vTarget);

        _vector vNormalizedBombToTargetXZ = XMVector3Normalize(vBombToTargetXZ);
        if (fLengthSq > fMax * fMax)
        {
            vBombToTargetXZ = vNormalizedBombToTargetXZ * fMax;
            vTarget = XMVectorSetY(vBombPos + vBombToTargetXZ, fTargetY);
        }
        else if (fLengthSq < fMin * fMin && fLengthSq > Helper::fEpsilon)
        {
            vBombToTargetXZ = vNormalizedBombToTargetXZ * fMin;
            vTarget = XMVectorSetY(vBombPos + vBombToTargetXZ, fTargetY);
        }

        XMStoreFloat3(&m_vAimTargetPos, vTarget);
    }

    m_vAimInput = {};
}

void CKirby_Ability_Bomb::Update_AimFacing(CKirby* pKirby, _float fTimeDelta)
{
    if (m_pBomb == nullptr)
        return;

    CTransform* pKirbyTransform = pKirby->Get_Transform();
    _vector vKirbyLook = XMVectorSetY(pKirbyTransform->Get_State(STATE::LOOK), 0.f);

    if (XMVectorGetX(XMVector3LengthSq(vKirbyLook)) <= Helper::fEpsilon)
        return;

    vKirbyLook = XMVector3Normalize(vKirbyLook);

    CTransform* pBombTransform = m_pBomb->Get_Transform();
    _vector vBombPos = pBombTransform->Get_State(STATE::POSITION);
    _vector vTargetPos = XMLoadFloat3(&m_vAimTargetPos);
    _vector vThrowDir = XMVectorSetY(vTargetPos - vBombPos, 0.f);

    if (XMVectorGetX(XMVector3LengthSq(vThrowDir)) <= Helper::fEpsilon)
        return;

    vThrowDir = XMVector3Normalize(vThrowDir);

    _float fDot = XMVectorGetX(XMVector3Dot(vKirbyLook, vThrowDir));

    constexpr _float fTurnStartDot = 0.98f;
    constexpr _float fTurnEndDot = 0.9999f;

    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    if (fDot <= fTurnStartDot)
    {
        m_bAimRotating = true;
    }
    else if(fDot > fTurnEndDot)
    {
        m_bAimRotating = false;

        if (m_eBombState == BOMB_STATE::CHARGING && m_bPlayingRotAni == true)
        {
            pAnimator->Play("BombThrowCharge", true, false, 0.1f, 1.5f);
            m_bPlayingRotAni = false;
        }
    }

    if (m_bAimRotating)
    {
        pKirby->Get_Movement()->Rotate_To_Direction(vThrowDir, fTimeDelta);

        if (m_eBombState == BOMB_STATE::CHARGING && m_bPlayingRotAni == false)
        {
            pAnimator->Play("BombThrowRotate", true, false, 0.1f, 1.5f);
            m_bPlayingRotAni = true;
        }
    }
}

void CKirby_Ability_Bomb::Update_AimPrediction()
{
    if (m_pBomb == nullptr)
        return;

    _float3 vStart{};
    XMStoreFloat3(&vStart, m_pBomb->Get_Transform()->Get_State(STATE::POSITION));

    _vector vBombToAimXZ = XMVectorSetY(XMLoadFloat3(&m_vAimTargetPos) - XMLoadFloat3(&vStart), 0.f);
    const _float fHorizontalDist = XMVectorGetX(XMVector3Length(vBombToAimXZ));

    _float3 vHorizontalDir{};

    if (fHorizontalDist > Helper::fEpsilon)
        XMStoreFloat3(&vHorizontalDir, XMVector3Normalize(vBombToAimXZ));

    _float fHorizontalSpeed = fHorizontalDist;
    Helper::FloatClamp(fHorizontalSpeed, 3.f, 15.f);

    constexpr _float fArcHeight = 5.f;
    m_vAimLaunchVelocity = m_pBomb->Cal_LaunchVelocity(vHorizontalDir, fHorizontalSpeed, fArcHeight);

    // Effect
    m_bPredictedHit = m_pBomb->Predict_Trajectory(vStart, m_vAimLaunchVelocity,
        m_PredictedPathPoints, m_vPredictedHitPos, m_vPredictedHitNormal);

    Update_BombHitAimEffect();
}

void CKirby_Ability_Bomb::Update_BombHitAimEffect()
{
    if (m_pBomb == nullptr || m_bPredictedHit == false)
    {
        Effect_Stop(m_pBombHitAim);
        return;
    }

    _vector vNormal = XMLoadFloat3(&m_vPredictedHitNormal);
    if (XMVectorGetX(XMVector3LengthSq(vNormal)) > Helper::fEpsilon)
        vNormal = XMVector3Normalize(vNormal);
    else
        vNormal = XMVectorSet(0.f, 1.f, 0.f, 0.f);

    _vector vPos = XMLoadFloat3(&m_vPredictedHitPos) + vNormal * 0.07f;

    if (m_pBombHitAim == nullptr)
    {
        _float3 vSpawnPos{};
        XMStoreFloat3(&vSpawnPos, vPos);
        CEffect_Loader::GetInstance()->Spawn(L"BombHitAim", m_pBomb->Get_LevelIndex(),
            vSpawnPos, _float3{}, _float3{}, nullptr, &m_pBombHitAim);
    }

    if (m_pBombHitAim == nullptr)
        return;

    _vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    _vector vRightCrossAxis{};
    _float fUpDot = fabsf(XMVectorGetX(XMVector3Dot(vNormal, vWorldUp)));

    if (fUpDot > 0.99f)
        vRightCrossAxis = XMVectorSet(0.f, 0.f, 1.f, 0.f);
    else
        vRightCrossAxis = vWorldUp;

    _vector vRight = XMVector3Normalize(XMVector3Cross(vRightCrossAxis, vNormal));
    _vector vUp = XMVector3Normalize(XMVector3Cross(vNormal, vRight));

    CTransform* pTransform = m_pBombHitAim->Get_Transform();
    pTransform->Set_State(STATE::POSITION, XMVectorSetW(vPos, 1.f));
    pTransform->LookTo(vNormal, vUp);
}

void CKirby_Ability_Bomb::Despawn_BombHitAimEffect()
{
    if (m_pBombHitAim == nullptr)
        return;

    Effect_Stop(m_pBombHitAim);
    m_pBombHitAim = nullptr;
}

void CKirby_Ability_Bomb::Update_BombAimDots(_float fTimeDelta)
{
    if (m_pBomb == nullptr || !m_bPredictedHit || m_PredictedPathPoints.empty())
    {
        Despawn_BombAimDots();
        return;
    }

    m_fBombAimDotStep += 60.f * fTimeDelta;

    while (m_fBombAimDotStep >= static_cast<_float>(s_iBombAimDotInterval))
        m_fBombAimDotStep -= static_cast<_float>(s_iBombAimDotInterval);

    const _uint iMoveIndex = static_cast<_uint>(m_fBombAimDotStep);

    for (_uint i = 0; i < s_iBombAimDotCount; ++i)
    {
        const _uint iPathIndex = iMoveIndex + i * s_iBombAimDotInterval;
        CEffect_Container*& pDot = m_pBombAimDots[i];

        if (m_PredictedPathPoints.size() <= 1 || iPathIndex >= m_PredictedPathPoints.size() - 1)
        {
            //if (pDot != nullptr)
            //    pDot->Set_Active(false);

            Effect_Stop(pDot);

            continue;
        }

        const _float3& vDotPos = m_PredictedPathPoints[iPathIndex];

        if (pDot == nullptr)
            CEffect_Loader::GetInstance()->Spawn(L"BombAimDot", m_pBomb->Get_LevelIndex(), vDotPos, _float3{}, _float3{}, nullptr, &pDot);

        if (pDot == nullptr)
            continue;

        pDot->Set_Active(true);
        pDot->Get_Transform()->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&vDotPos), 1.f));
    }
}

void CKirby_Ability_Bomb::Despawn_BombAimDots()
{
    for (_uint i = 0; i < s_iBombAimDotCount; ++i)
    {
        for (_uint i = 0; i < s_iBombAimDotCount; ++i)
            Effect_Stop(m_pBombAimDots[i]);

        m_fBombAimDotStep = 0.f;
    }

    m_fBombAimDotStep = 0.f;
}

CKirby_Ability_Bomb* CKirby_Ability_Bomb::Create()
{
    CKirby_Ability_Bomb* pInstance = new CKirby_Ability_Bomb();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Ability_Bomb");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Ability_Bomb::Free()
{
    __super::Free();
}