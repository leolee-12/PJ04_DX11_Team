#include "Monster_State_Spat.h"
#include "Monster.h"

HRESULT CMonster_State_Spat::Initialize() { return __super::Initialize(); }
HRESULT CMonster_State_Spat::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    return __super::Initialize(tInfo, fSpeed);
}

MONSTER_STATE_TYPE CMonster_State_Spat::Get_StateType() { return MONSTER_STATE_TYPE::SPAT; }

void CMonster_State_Spat::On_Enter(CMonster* pMonster)
{
    if (!pMonster) return;

    pMonster->Enable_Controller(false);
    pMonster->Enable_Colliders(false);        
    pMonster->Enable_ProjectileBox(true);     

    m_fLifeTime = s_fMaxLifeTime;

    if (!m_PlayInfo.strAniName.empty())
        if (CAnimator* pAnim = pMonster->Get_BodyAnimator())
            pAnim->Play(&m_PlayInfo);
}

void CMonster_State_Spat::On_Update(CMonster* pMonster, _float fTimeDelta)
{
    if (!pMonster) return;

    CTransform* pT = pMonster->Get_Transform();
    _vector vVel = XMLoadFloat3(&pMonster->Get_SpatVelocity());
    pT->Set_State(STATE::POSITION,
        pT->Get_State(STATE::POSITION) + vVel * fTimeDelta);

    _vector vLook = pT->Get_State(STATE::LOOK);
    pT->Rotate(XMQuaternionRotationAxis(vLook,
        XMConvertToRadians(s_fSpinSpeedDeg) * fTimeDelta));

    m_fLifeTime -= fTimeDelta;
    if (m_fLifeTime <= 0.f)
        pMonster->Despawn_Spat();
    // TODO(선택): 벽/맵 충돌 시에도 Despawn_Spat()
}

void CMonster_State_Spat::On_Exit(CMonster* pMonster)
{
    if (!pMonster) return;
    pMonster->Enable_ProjectileBox(false);
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
CMonster_State_Spat* CMonster_State_Spat::Create()
{
    CMonster_State_Spat* pInstance = new CMonster_State_Spat();
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CMonster_State_Spat"); Safe_Release(pInstance);
    }
    return pInstance;
}
void CMonster_State_Spat::Free() { __super::Free(); }