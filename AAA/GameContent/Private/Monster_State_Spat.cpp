#include "Monster_State_Spat.h"
#include "Monster.h"

HRESULT CMonster_State_Spat::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    return __super::Initialize(tInfo, fSpeed);
}

MONSTER_STATE_TYPE CMonster_State_Spat::Get_StateType() { return MONSTER_STATE_TYPE::SPAT; }

void CMonster_State_Spat::Enter(MONSTER_STATE_TYPE ePrevState)
{
    if (!m_pOwner || !m_pAnimator)
        return;

    m_fSpinAngle = 0.f;
    m_bSpinBone = m_pAnimator->Has_Bone("RotL");

    m_pOwner->Enable_Controller(false);
    m_pOwner->Enable_Colliders(false);
    m_pOwner->Enable_ProjectileBox(true);

    m_fLifeTime = s_fMaxLifeTime;

    if (!m_PlayInfo.strAniName.empty())
        m_pAnimator->Play(&m_PlayInfo);
}

void CMonster_State_Spat::Update(_float fTimeDelta)
{
    if (!m_pOwner || !m_pAnimator)
        return;

    CTransform* pT = m_pOwner->Get_Transform();
    _vector vVel = XMLoadFloat3(&m_pOwner->Get_SpatVelocity());
    pT->Set_State(STATE::POSITION,
        pT->Get_State(STATE::POSITION) + vVel * fTimeDelta);
    _vector vLook = pT->Get_State(STATE::LOOK);

    m_fSpinAngle += s_fSpinSpeedDeg * fTimeDelta;

    if (m_bSpinBone)
    {
        m_pAnimator->SetBoneRotation("RotL", m_fSpinAngle, XMVectorSet(0.f, 0.f, 1.f, 0.f));
    }
    else
    {
        pT->Rotate(XMQuaternionRotationAxis(vLook,
            XMConvertToRadians(s_fSpinSpeedDeg) * fTimeDelta));
    }

    m_fLifeTime -= fTimeDelta;
    if (m_fLifeTime <= 0.f)
        m_pOwner->Despawn_Spat();
}

void CMonster_State_Spat::Exit(MONSTER_STATE_TYPE eNextState)
{
    if (!m_pOwner)
        return;

    m_bSpinBone = false;

    m_pOwner->Enable_ProjectileBox(false);
}

CMonster_State_Spat* CMonster_State_Spat::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    CMonster_State_Spat* pInstance = new CMonster_State_Spat();
    if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
    {
        MSG_BOX("Failed to Created : CMonster_State_Spat"); Safe_Release(pInstance);
    }
    return pInstance;
}


void CMonster_State_Spat::Free() 
{
    __super::Free();
}