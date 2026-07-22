#pragma once

#include "Kirby_State.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_Body;

class CLIENT_DLL CKirby_UpperCalibur final : public CKirby_State
{
private:
	CKirby_UpperCalibur();
	virtual ~CKirby_UpperCalibur() = default;

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

public:
	virtual void Request_PositionSync(CKirby* pKirby, const KIRBY_POSITION_SYNC_BEGIN_DESC* pDesc) override;
	virtual void Request_PositionSync_End(CKirby* pKirby, const KIRBY_POSITION_SYNC_END_DESC* pDesc) override;

public:
	static CKirby_UpperCalibur* Create();
private:
	virtual void Free() override;
};

NS_END