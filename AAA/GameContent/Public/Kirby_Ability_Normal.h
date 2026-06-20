#pragma once

#include "Kirby_Ability.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CAnimator;
class CEffect_Container;

class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

class CKirby;
class CMovement_Child;

class CMonster;

class CLIENT_DLL CKirby_Ability_Normal final : public CKirby_Ability
{
private:
	enum class INHALE_STATE
	{
		INHALE_LOOP,
		SUPER_INHALE_START, SUPER_INHALE_LOOP,
		INHALE_END
	};

	enum class INHALE_MOVE_STATE
	{
		WAIT, WALK, FALL
	};

private:
	CKirby_Ability_Normal();
	virtual ~CKirby_Ability_Normal() = default;

private:
	HRESULT Initialize();

public:
	virtual COPY_ABILITY_TYPE Get_AbilityType() override;

	virtual void Enter_Ability(CKirby* pKirby) override;
	virtual ABILITY_UPDATE_RESULT Update_Ability(CKirby* pKirby, _float fTimeDelta) override;
	virtual void Exit_Ability(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

	virtual _bool Enter_Attack_KeyDown(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyPress(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyUp(CKirby* pKirby) override;

	virtual _bool Can_Attack(KIRBY_ATTACK_LOCATION eAttackLocation) override;

	_bool IsSuperInhale();
	_bool Change_Ability(CKirby* pKirby, COPY_ABILITY_TYPE eAbility);

private:
	INHALE_STATE m_eInhaleState{};

	INHALE_MOVE_STATE m_eCurMoveState{};
	INHALE_MOVE_STATE m_ePreMoveState{};

	_float m_MaxSuperInHaleTime{};
	_float m_AccSuperInHaleTime{};

	_bool m_bForceEnterSuperInhaleStart{};

	_bool m_bReqInhale{};

	CEffect_Container* m_pInhaleEffect{};

	// Event
	CGameInstance_Proxy* m_pGameInstance_Proxy{};

	SUBHANDLE m_hSwallowedEvent{};
	_bool m_bSubscribedSwallowedEvent{};

private:
	void Interpolation_Inhale(CAnimator* pAnimator);
	void Choose_InhaleAniName(_string& strAniName);

	_bool Change_Ability(CKirby* pKirby);
	void Reset_Default(CKirby* pKirby);

	void Start_InhaleCollider(CKirby* pKirby);
	void End_InhaleCollider(CKirby* pKirby);

	// Event
	void Start_SwallowedEvent(CKirby* pKirby);
	void End_SwallowedEvent();
	void On_Swallowed(CKirby* pKirby, CMonster* pMonster);

public:
	static CKirby_Ability_Normal* Create();
private:
	virtual void Free() override;
};

NS_END