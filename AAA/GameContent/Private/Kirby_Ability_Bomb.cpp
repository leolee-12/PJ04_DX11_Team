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
    m_eBombState = BOMB_STATE::BOMB_STATE_END;
    
    CMovement_Child* pMovement = pKirby->Get_Movement();
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
                m_eBombState == BOMB_STATE::CHARGING_FALL ||
                m_eBombState == BOMB_STATE::CHARGING_LANDING)
            {
                Acc_AimInput(pMoveCommand->Get_Dir());
                return true;
            }

            return true;
        }
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (pCommand->IsUp())
            {
                m_bKeyUp = true;
            }
            else if (pCommand->IsDown())
            {
                m_bReserveAttack = true;
                m_bKeyUp = false;
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

    if (m_pBomb != nullptr)
    {
        m_pBomb->Despawn();
        m_pBomb = nullptr;
    }

    if (m_pBombHitAim != nullptr)
    {
        m_pBombHitAim->EffectContainer_Stop();
        m_pBombHitAim = nullptr;
    }
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
            pAnimator->Play("BombThrowCharge", true, false, 0.1f, 1.5f);
            Reset_Aim(pKirby);
            Spawn_Bomb(pKirby);
            break;
        case BOMB_STATE::CHARGING:
            pAnimator->Play("BombThrowCharge", true, false, 0.1f, 1.5f);
            break;

        case BOMB_STATE::CHARGE_START_FALL:
            pAnimator->Play("BombFall", false, false, 0.1f, 1.5f);
            Reset_Aim(pKirby);
            Spawn_Bomb(pKirby);
            break;
        case BOMB_STATE::CHARGING_FALL:
            pAnimator->Play("BombFall", false, false, 0.1f, 1.5f);
            break;

        case BOMB_STATE::CHARGING_LANDING:
            pAnimator->Play("BombLanding", false, false, 0.1f, 1.5f);
            break;

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
            Update_AimPrediction();

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
            Update_AimPrediction();

            if (m_bKeyUp)
                Change_BombState(pKirby, BOMB_STATE::CHARGING_THROW);
            else if (IsGround)
                Change_BombState(pKirby, BOMB_STATE::CHARGING_LANDING);
            break;
        }
        case BOMB_STATE::CHARGING_LANDING:
        {
            Cal_Aim(fTimeDelta);
            Update_AimPrediction();

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
    switch (eState)
    {
        case BOMB_STATE::MOVE_THROW:
            break;
        case BOMB_STATE::CHARGE_START:
            break;
        case BOMB_STATE::CHARGING:
            break;
        case BOMB_STATE::THROW:
            break;
        case BOMB_STATE::CHARGING_LANDING:
            break;
        case BOMB_STATE::CHARGING_THROW:
            break;
    }
}

_bool CKirby_Ability_Bomb::Handle_ReserveAttack(CKirby* pKirby)
{
    if (!m_bReserveAttack)
        return false;

    if (pKirby->Has_MoveDir())
        Change_BombState(pKirby, BOMB_STATE::MOVE_THROW);
    else
        Change_BombState(pKirby, BOMB_STATE::CHARGE_START);

    m_bReserveAttack = false;

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

    Despawn_BombHitAim();
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

        if (fLengthSq > fMax * fMax)
        {
            vBombToTargetXZ = XMVector3Normalize(vBombToTargetXZ) * fMax;
            vTarget = XMVectorSetY(vBombPos + vBombToTargetXZ, fTargetY);
        }
        else if (fLengthSq < fMin * fMin && fLengthSq > Helper::fEpsilon)
        {
            vBombToTargetXZ = XMVector3Normalize(vBombToTargetXZ) * fMin;
            vTarget = XMVectorSetY(vBombPos + vBombToTargetXZ, fTargetY);
        }

        XMStoreFloat3(&m_vAimTargetPos, vTarget);
    }

    m_vAimInput = {};
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

    Update_BombHitAim();
}

void CKirby_Ability_Bomb::Update_BombHitAim()
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

void CKirby_Ability_Bomb::Despawn_BombHitAim()
{
    if (m_pBombHitAim == nullptr)
        return;

    Effect_Stop(m_pBombHitAim);
    m_pBombHitAim = nullptr;
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