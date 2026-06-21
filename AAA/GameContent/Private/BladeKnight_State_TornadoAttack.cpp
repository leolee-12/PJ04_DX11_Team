#include "BladeKnight_State_TornadoAttack.h"
#include "BladeKnight.h"
#include "BladeKnight_Body.h"
#include "Animator.h"
#include "Monster_Movement.h"

HRESULT CBladeKnight_State_TornadoAttack::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	return S_OK;
}

MONSTER_STATE_TYPE CBladeKnight_State_TornadoAttack::Get_StateType()
{
	return MONSTER_STATE_TYPE::TORNADO_ATTACK;
}

void CBladeKnight_State_TornadoAttack::Enter()
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

	if (m_pAnimator)
	{
		CAnimator::ANI_PLAY_INFO AniInfo{};
		AniInfo.strAniName = "TornadoAttackCharge";
		AniInfo.bLoop = false;
		AniInfo.fSpeed = 1.50f;
		m_pAnimator->Play(&AniInfo); // Queue 클리어 + 시작

		AniInfo.strAniName = "TornadoAttack";
		AniInfo.fSpeed = 1.50f;
		m_pAnimator->Enqueue(AniInfo);
	}
}

void CBladeKnight_State_TornadoAttack::Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (nullptr == m_pOwner)
		return;

	if (m_pAnimator && m_pAnimator->Is_Finished())
		m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
}

void CBladeKnight_State_TornadoAttack::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (nullptr == m_pOwner)
		return;
}

CBladeKnight_State_TornadoAttack* CBladeKnight_State_TornadoAttack::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CBladeKnight_State_TornadoAttack* pInstance = new CBladeKnight_State_TornadoAttack();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CBladeKnight_State_TornadoAttack");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBladeKnight_State_TornadoAttack::Free()
{
	__super::Free();
}
