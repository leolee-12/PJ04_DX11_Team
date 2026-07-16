#pragma once

#include "Kirby_Ability.h"

NS_BEGIN(Client)

class CKirby;
class CKirbyBomb;

class CLIENT_DLL CKirby_Ability_Bomb final : public CKirby_Ability
{
private:
	enum BOMB_STATE
	{ 
		MOVE_THROW,
		CHARGE_START, CHARGING,
		CHARGE_START_FALL, CHARGING_FALL,
		CHARGING_LANDING,
		THROW, CHARGING_THROW,
		BOMB_STATE_END
	};

private:
	CKirby_Ability_Bomb();
	virtual ~CKirby_Ability_Bomb() = default;

private:
	HRESULT Initialize();

public:
	virtual COPY_ABILITY_TYPE Get_AbilityType() override;

	virtual void Enter_AttackState(CKirby* pKirby, _int iFlag) override;
	virtual void Update_AttackState(CKirby* pKirby, _float fTimeDelta) override;
	virtual void Exit_AttackState(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

	virtual _bool Enter_Attack_KeyDown(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyPress(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyUp(CKirby* pKirby) override;

private:
	void Change_BombState(CKirby* pKirby, BOMB_STATE eNext);
	void Enter_BombState(CKirby* pKirby, BOMB_STATE eState);
	void Update_BombState(CKirby* pKirby, _float fTimeDelta);
	void Exit_BombState(CKirby* pKirby, BOMB_STATE eState);

	_bool Handle_ReserveAttack(CKirby* pKirby);

private:
	void Spawn_Bomb(CKirby* pKirby);
	void Throw_Bomb(CKirby* pKirby, _float fDegree, _float fSpeed);
	void Throw_BombToAim();

	void Reset_Aim(CKirby* pKirby);
	void Acc_AimInput(const _float3& vInputDir);
	void Cal_Aim(_float fTimeDelta);

	void Update_AimLaunch();

private:
	BOMB_STATE m_eBombState{};

	_bool m_bKeyUp{};

	_bool m_bReserveAttack{};

private:
	CKirbyBomb* m_pBomb{};

	_float3 m_vAimInput{};
	_float3 m_vAimTargetPos{};

	_float3 m_vAimLaunchVelocity{};


	// Effect
	vector<_float3> m_PredictedPathPoints;
	_float3 m_vPredictedHitPosition{};
	_float3 m_vPredictedHitNormal{};
	_bool m_bPredictedHit{};

public:
	static CKirby_Ability_Bomb* Create();

private:
	virtual void Free() override;
};

NS_END
