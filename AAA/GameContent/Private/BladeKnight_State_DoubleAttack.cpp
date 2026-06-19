#include "BladeKnight_State_DoubleAttack.h"
#include "BladeKnight.h"
#include "BladeKnight_Body.h"
#include "Animator.h"
#include "Monster_Movement.h"

HRESULT CBladeKnight_State_DoubleAttack::Initialize()
{
	return S_OK;
}

MONSTER_STATE_TYPE CBladeKnight_State_DoubleAttack::Get_StateType()
{
	return MONSTER_STATE_TYPE::DOUBLE_ATTACK;
}

void CBladeKnight_State_DoubleAttack::On_Enter(CBladeKnight* pBladeKnight)
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

	if (CMonster_Movement* pMovement = pBladeKnight->Get_Movement())
		pMovement->Set_MoveSpeed(3.f);

	CBladeKnight_Body* pBody = pBladeKnight->Get_Body();
	if (pBody == nullptr)
		return;

	CAnimator* pAnim = (pBody != nullptr) ? pBody->Get_Animator() : nullptr;
	if (pAnim != nullptr)
	{
		CAnimator::ANI_PLAY_INFO AniInfo{};
		AniInfo.strAniName = "AttackStart";
		AniInfo.bLoop = false;
		AniInfo.fSpeed = 1.25f;
		pAnim->Play(&AniInfo); // Queue 클리어 + 시작

		AniInfo.strAniName = "DoubleAttack";
		AniInfo.fSpeed = 1.50f;
		pAnim->Enqueue(AniInfo);
	}
}

void CBladeKnight_State_DoubleAttack::On_Update(CBladeKnight* pBladeKnight, _float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (nullptr == pBladeKnight)
		return;

	CAnimator* pAnim = pBladeKnight->Get_Body() ? pBladeKnight->Get_Body()->Get_Animator() : nullptr;
	if (pAnim != nullptr && pAnim->Is_Finished())
		pBladeKnight->Change_State(MONSTER_STATE_TYPE::RETREAT);

	if (pAnim && pAnim->Get_CurrentAnimName() != "AttackStart" && !pBladeKnight->Get_BlackBoard().bMoveLocked)
		pBladeKnight->Add_MoveDir(m_vLungeDir);
}

void CBladeKnight_State_DoubleAttack::On_Exit(CBladeKnight* pBladeKnight)
{
	if (nullptr == pBladeKnight)
		return;
}

CBladeKnight_State_DoubleAttack* CBladeKnight_State_DoubleAttack::Create()
{
	CBladeKnight_State_DoubleAttack* pInstance = new CBladeKnight_State_DoubleAttack();
	if (FAILED(pInstance->Initialize()))
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
