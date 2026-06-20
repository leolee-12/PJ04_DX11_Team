#include "BladeKnight_State_TornadoAttack.h"
#include "BladeKnight.h"
#include "BladeKnight_Body.h"
#include "Animator.h"
#include "Monster_Movement.h"

HRESULT CBladeKnight_State_TornadoAttack::Initialize()
{
	return S_OK;
}

MONSTER_STATE_TYPE CBladeKnight_State_TornadoAttack::Get_StateType()
{
	return MONSTER_STATE_TYPE::TORNADO_ATTACK;
}

void CBladeKnight_State_TornadoAttack::On_Enter(CBladeKnight* pBladeKnight)
{
	if (nullptr == pBladeKnight)
		return;

	const MONSTER_BLACKBOARD& BB = pBladeKnight->Get_BlackBoard();
	m_vLungeDir = BB.vDirToTargetXZ;

	_vector vDir = XMLoadFloat3(&m_vLungeDir);
	if (!XMVector3Equal(vDir, XMVectorZero()))
	{
		_vector vMyPos = pBladeKnight->Get_Transform()->Get_State(STATE::POSITION);
		pBladeKnight->Get_Transform()->LookAt(vMyPos + vDir);
	}

	CBladeKnight_Body* pBody = pBladeKnight->Get_Body();
	if (pBody == nullptr)
		return;
	CAnimator* pAnim = (pBody != nullptr) ? pBody->Get_Animator() : nullptr;
	if (pAnim != nullptr)
	{
		CAnimator::ANI_PLAY_INFO AniInfo{};
		AniInfo.strAniName = "TornadoAttackCharge";
		AniInfo.bLoop = false;
		AniInfo.fSpeed = 1.50f;
		pAnim->Play(&AniInfo); // Queue 클리어 + 시작

		AniInfo.strAniName = "TornadoAttack";
		AniInfo.fSpeed = 1.50f;
		pAnim->Enqueue(AniInfo);
	}
}

void CBladeKnight_State_TornadoAttack::On_Update(CBladeKnight* pBladeKnight, _float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (nullptr == pBladeKnight)
		return;

	CAnimator* pAnim = pBladeKnight->Get_Body() ? pBladeKnight->Get_Body()->Get_Animator() : nullptr;
	if (pAnim != nullptr && pAnim->Is_Finished())
		pBladeKnight->Change_State(MONSTER_STATE_TYPE::IDLE);
}

void CBladeKnight_State_TornadoAttack::On_Exit(CBladeKnight* pBladeKnight)
{
	if (nullptr == pBladeKnight)
		return;
}

CBladeKnight_State_TornadoAttack* CBladeKnight_State_TornadoAttack::Create()
{
	CBladeKnight_State_TornadoAttack* pInstance = new CBladeKnight_State_TornadoAttack();
	if (FAILED(pInstance->Initialize()))
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
