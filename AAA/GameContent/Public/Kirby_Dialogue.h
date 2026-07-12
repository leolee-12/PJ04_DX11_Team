#pragma once

#include "Kirby_State.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_Body;

class CLIENT_DLL CKirby_Dialogue final : public CKirby_State
{
private:
	CKirby_Dialogue();
	virtual ~CKirby_Dialogue() = default;

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

	// Dialogue
	virtual void Request_Dialogue(CKirby* pKirby, const SEQUENCE_KIRBY_WARP_DESC* pDesc) override;
	virtual void Request_DialogueAnim(CKirby* pKirby, const SEQUENCE_KIRBY_ANIM_DESC* pDesc) override;

public:
	static CKirby_Dialogue* Create();
private:
	virtual void Free() override;
};

NS_END