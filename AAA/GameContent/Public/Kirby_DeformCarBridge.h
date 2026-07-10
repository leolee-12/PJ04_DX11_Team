#pragma once

#include "Kirby_State.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_Body;

class CLIENT_DLL CKirby_DeformCarBridge final : public CKirby_State
{
private:
	enum DEFORM_CAR_BRIDGE_STATE { START, DEFORM_CAR_BRIDGE_END };

private:
	CKirby_DeformCarBridge();
	virtual ~CKirby_DeformCarBridge() = default;

private:
	HRESULT Initialize();

public:
	virtual KIRBY_STATE_TYPE Get_StateType() override;

public:
	virtual void Enter(CKirby* pKirby, _int iFlag) override;
	virtual void Update(CKirby* pKirby, const _float fTimeDelta) override;
	virtual void Exit(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

	virtual void Request_PositionSync(CKirby* pKirby, const KIRBY_POSITION_SYNC_BEGIN_DESC* pDesc) override;
	virtual void Request_PositionSync_End(CKirby* pKirby, const KIRBY_POSITION_SYNC_END_DESC* pDesc) override;

private:
	void Change_BridgeState(CKirby* pKirby, DEFORM_CAR_BRIDGE_STATE eNext);
	void Enter_BridgeState(CKirby* pKirby, DEFORM_CAR_BRIDGE_STATE eState);
	void Update_BridgeState(CKirby* pKirby, _float fTimeDelta);
	void Exit_BridgeState(CKirby* pKirby, DEFORM_CAR_BRIDGE_STATE eState);

private:
	DEFORM_CAR_BRIDGE_STATE m_eBridgeState{ DEFORM_CAR_BRIDGE_STATE::DEFORM_CAR_BRIDGE_END };

public:
	static CKirby_DeformCarBridge* Create();
private:
	virtual void Free() override;
};

NS_END