#pragma once

#include "Kirby_State.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_Body;

class CLIENT_DLL CKirby_AbilityDump final : public CKirby_State
{
private:
	CKirby_AbilityDump();
	virtual ~CKirby_AbilityDump() = default;

private:
	HRESULT Initialize();

public:
	virtual KIRBY_STATE_TYPE Get_StateType() override;

public:
	virtual void Enter(CKirby* pKirby, _int iFlag) override;
	virtual void Update(CKirby* pKirby, const _float fTimeDelta) override;
	virtual void Exit(CKirby* pKirby) override;

public:
	virtual void On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo) override;

public:
	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

private:
	_bool m_bPartsOff{};

private:
	void Update_AbilityDump(CKirby* pKirby, _float fRatio);

public:
	static CKirby_AbilityDump* Create();
private:
	virtual void Free() override;
};

NS_END