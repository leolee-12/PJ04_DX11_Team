#include "BladeKnight_State_DoubleAttack.h"
#include "BladeKnight.h"
#include "BladeKnight_Body.h"
#include "Monster_Movement.h"

HRESULT CBladeKnight_State_DoubleAttack::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	return S_OK;
}

MONSTER_STATE_TYPE CBladeKnight_State_DoubleAttack::Get_StateType()
{
	return MONSTER_STATE_TYPE::DOUBLE_ATTACK;
}

void CBladeKnight_State_DoubleAttack::Enter()
{
	if (nullptr == m_pOwner)
		return;

	const MONSTER_BLACKBOARD& BB = m_pOwner->Get_BlackBoard();
	m_vLungeDir = BB.vDirToTargetXZ;

	_vector vDir = XMLoadFloat3(&m_vLungeDir);
	if (!XMVector3Equal(vDir, XMVectorZero()))
	{
		_vector vMyPos = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);
		m_pOwner->Get_Transform()->LookAt(vMyPos + vDir);
	}

	if (m_pMovement)
		m_pMovement->Set_MoveSpeed(3.f);

	if (m_pAnimator)
	{
		CAnimator::ANI_PLAY_INFO AniInfo{};
		AniInfo.strAniName = "AttackStart";
		AniInfo.bLoop = false;
		AniInfo.fSpeed = 1.25f;
		m_pAnimator->Play(&AniInfo); // Queue 클리어 + 시작

		AniInfo.strAniName = "DoubleAttack";
		AniInfo.fSpeed = 1.50f;
		m_pAnimator->Enqueue(AniInfo);
	}
}

void CBladeKnight_State_DoubleAttack::Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (nullptr == m_pOwner)
		return;

	if (m_pAnimator && m_pAnimator->Is_Finished())
		m_pOwner->Change_State(MONSTER_STATE_TYPE::RETREAT);

	if (m_pAnimator && m_pAnimator->Get_CurrentAnimName() != "AttackStart" && !m_pOwner->Get_BlackBoard().bMoveLocked)
		m_pOwner->Add_MoveDir(m_vLungeDir);
}

void CBladeKnight_State_DoubleAttack::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (nullptr == m_pOwner)
		return;
}

CBladeKnight_State_DoubleAttack* CBladeKnight_State_DoubleAttack::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CBladeKnight_State_DoubleAttack* pInstance = new CBladeKnight_State_DoubleAttack();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CBladeKnight_State_DoubleAttack");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBladeKnight_State_DoubleAttack::Free()
{
	__super::Free();
}
