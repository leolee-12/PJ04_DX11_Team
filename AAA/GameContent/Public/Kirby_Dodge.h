#pragma once

#include "Kirby_State.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_Body;

enum DODGE_STATE_FLAG { DODGE_NONE, DODGE_LEFT, DODGE_RIGHT, DODGE_FRONT, DODGE_BACK };

class CLIENT_DLL CKirby_Dodge final : public CKirby_State
{
private:
	enum Dodge_State { DODGE_START, DODGE1, DODGE2, DODGE_END };
	enum DODGE_ANI_DIR { ANI_LEFT, ANI_RIGHT, ANI_FRONT, ANI_BACK };

	static constexpr _float s_fDodgeMaxHorizontalSpeed = 12.f;
	static constexpr _float s_fDodgeJumpSpeed = 26.f;

private:
	CKirby_Dodge();
	virtual ~CKirby_Dodge() = default;

private:
	HRESULT Initialize();

public:
	virtual KIRBY_STATE_TYPE Get_StateType() override;

public:
	virtual void Enter(CKirby* pKirby, _int iFlag) override;
	virtual void Update(CKirby* pKirby, const _float fTimeDelta) override;
	virtual void Exit(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

public:
	virtual void On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo) override;

private:
	void Change_DodgeState(CKirby* pKirby, Dodge_State eNext);
	void Enter_DodgeState(CKirby* pKirby, Dodge_State eState);
	void Update_DodgeState(CKirby* pKirby, _float fTimeDelta);
	void Exit_DodgeState(CKirby* pKirby, Dodge_State eState);

	void Set_DodgeAniDir(CKirby* pKirby);
	void DodgeMove(CKirby* pKirby);

	void Update_SlowTimer(_float fTimeDelta);

private:
	Dodge_State m_eDodgeState{ DODGE_END };
	DODGE_STATE_FLAG m_eDodgeFlag{};

	DODGE_ANI_DIR m_eDodgeAniDir{};

	_float3 m_vDodgeDir{};

	_int m_iEvasionCount{};
	_float m_fSlowMaxTime{};
	_float m_fSlowAccTime{};

public:
	static CKirby_Dodge* Create();
private:
	virtual void Free() override;
};

NS_END
