#include "Kirby_Ability_Bomb.h"

#include "GameInstance.h"
#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

#include "Projectile_Manager.h"
#include "KirbyBomb.h"

CKirby_Ability_Bomb::CKirby_Ability_Bomb()
{
}

HRESULT CKirby_Ability_Bomb::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_wstrAttackModeName = L"¹ã";

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
        Change_BombState(pKirby, BOMB_STATE::FALL_CHARGE_START);
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

            if (m_eBombState != BOMB_STATE::MOVE_THROW)
                return true;

            Move_Command* pMoveCommand = static_cast<Move_Command*>(pCommand);
            pKirby->Add_MoveDir(pMoveCommand->Get_Dir());

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
    // ¹«½Ã
    return true;
}

_bool CKirby_Ability_Bomb::Enter_Attack_KeyUp(CKirby* pKirby)
{
    // ¹«½Ã
    return true;
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
            break;
        case BOMB_STATE::CHARGE_START:
            pAnimator->Play("BombThrowCharge", true, false, 0.1f, 1.5f);
            //Spawn_Bomb(pKirby);
            break;
        case BOMB_STATE::CHARGE:
            pAnimator->Play("BombThrowCharge", true, false, 0.1f, 1.5f);
            break;
        case BOMB_STATE::FALL_CHARGE_START:
            pAnimator->Play("BombFall", false, false, 0.1f, 1.5f);
            break;
        case BOMB_STATE::FALL_CHARGE:
            break;
        case BOMB_STATE::CHARGE_LANDING:
            pAnimator->Play("BombLanding", false, false, 0.1f, 1.5f);
            break;
        case BOMB_STATE::THROW:
            pAnimator->Play("BombThrow", false, false, 0.1f, 2.5f);
            //Throw_Bomb(pKirby);
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
                    Change_BombState(pKirby, BOMB_STATE::CHARGE);
            }
            break;
        }
        case BOMB_STATE::CHARGE:
        {
            if (m_bKeyUp)
                Change_BombState(pKirby, BOMB_STATE::THROW);
            break;
        }
        case BOMB_STATE::FALL_CHARGE_START:
        {
            if (!pAnimator->Is_Blending())
            {
                if (m_bKeyUp)
                    Change_BombState(pKirby, BOMB_STATE::THROW);
                else
                    Change_BombState(pKirby, BOMB_STATE::FALL_CHARGE);
            }
            else if(IsGround)
            {
                Change_BombState(pKirby, BOMB_STATE::CHARGE_LANDING);
            }
            break;
        }
        case BOMB_STATE::FALL_CHARGE:
        {
            if (m_bKeyUp)
                Change_BombState(pKirby, BOMB_STATE::THROW);
            else if (IsGround)
                Change_BombState(pKirby, BOMB_STATE::CHARGE_LANDING);
            break;
        }
        case BOMB_STATE::CHARGE_LANDING:
        {
            if (m_bKeyUp)
                Change_BombState(pKirby, BOMB_STATE::THROW);
            else if (pAnimator->Is_Finished())
                Change_BombState(pKirby, BOMB_STATE::CHARGE);
            break;
        }
        case BOMB_STATE::THROW:
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
        case BOMB_STATE::CHARGE:
            break;
        case BOMB_STATE::THROW:
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
    CProjectile* pProjectile{};

    if (FAILED(CProjectile_Manager::GetInstance()->Spawn(pKirby->Get_PrototypeLevelIndex(), pKirby->Get_LevelIndex(),
        CKirbyBomb::POOL_KEY, CKirbyBomb::PROTOTYPE_TAG, &pProjectile)))
        return;

    CKirbyBomb* pBomb = dynamic_cast<CKirbyBomb*>(pProjectile);

    if (pBomb == nullptr)
        return;

    const _float4x4* pSocketBone = pKirby->Get_Body()->Get_BoneMatrixPtr("RHaveL");

    pBomb->Attach_To_Socket(pSocketBone, pKirby->Get_Transform()->Get_WorldMatrixPtr(), XMMatrixIdentity());

    m_pBomb = pBomb;
}

void CKirby_Ability_Bomb::Throw_Bomb(CKirby* pKirby)
{
    _vector vPos = m_pBomb->Get_Transform()->Get_State(STATE::POSITION);

    _vector vLook = pKirby->Get_Transform()->Get_State(STATE::LOOK);
    vLook = XMVector3Normalize(XMVectorSetY(vLook, 0.f));

    constexpr _float fLaunchAngle = 45.f;
    _float fRadian = XMConvertToRadians(fLaunchAngle);
    _vector vDir = vLook * cosf(fRadian) + XMVectorSet(0.f, 1.f, 0.f, 0.f) * sinf(fRadian);

    _float3 vLaunchPos{};
    _float3 vLaunchDir{};

    XMStoreFloat3(&vLaunchPos, vPos);
    XMStoreFloat3(&vLaunchDir, vDir);

    m_pBomb->Launch(vLaunchPos, vLaunchDir);
    m_pBomb = nullptr;
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
