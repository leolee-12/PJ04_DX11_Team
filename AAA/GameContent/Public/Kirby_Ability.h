#pragma once

#include "Base.h"
#include "Animator.h"

#include "GameContent_Defines.h"

#include "Kirby_Command.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

enum class KIRBY_ABILITY_TYPE
{
	NORMAL
};

enum class KIRBY_ATTACK_LOCATION { GROUND, AIR };

class CKirby;

enum class ABILITY_ANI
{
	WAIT, RUN, FALL,
	JUMP_L, JUMP_R, JUMP_END_L, JUMP_END_R,
	LANDING,
	END
};

class CLIENT_DLL CKirby_Ability abstract : public CBase
{
protected:
	CKirby_Ability();
	virtual ~CKirby_Ability() = default;

protected:
	HRESULT Initialize();

public:
	virtual KIRBY_ABILITY_TYPE Get_AbilityType() = 0;

	virtual void Enter_Ability(CKirby* pKirby) = 0;
	virtual void Update_Ability(CKirby* pKirby, _float fTimeDelta) = 0;
	virtual void Exit_Ability(CKirby* pKirby) = 0;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) = 0;

	virtual void Down_Attack(CKirby* pKirby) = 0;
	virtual void Up_Attack(CKirby* pKirby) = 0;

	virtual _bool Can_Attack(KIRBY_ATTACK_LOCATION eAttackLocation);

	const CAnimator::ANI_PLAY_INFO* Get_AniInfo(ABILITY_ANI eAbilityAni);

	_bool IsFinished() { return m_bIsFinished; }
	_bool IsEndAttack() { return m_bEndAttack; }

protected:
	_bool m_bIsFinished{};
	_bool m_bEndAttack{ true };

	vector<CAnimator::ANI_PLAY_INFO> m_tAniInfos;

protected:
	virtual void Free() override;
};

NS_END