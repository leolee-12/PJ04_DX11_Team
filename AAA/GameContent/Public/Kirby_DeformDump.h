#pragma once

#include "Kirby_State.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_Body;

enum DEFORM_DUMP_STATE_FLAG : _int { DEFAULT = -1, SPIT_START_JUMP, SPIT_DEFORM_JUMP };

class CLIENT_DLL CKirby_DeformDump final : public CKirby_State
{
private:
	enum DEFORM_DUMP_STATE { SPIT_START, SPIT_DEFORM, DEFORM_DUMP_STATE_END };

private:
	CKirby_DeformDump();
	virtual ~CKirby_DeformDump() = default;

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
	void Change_DeformDumpState(CKirby* pKirby, DEFORM_DUMP_STATE eNext);
	void Enter_DeformDumpState(CKirby* pKirby, DEFORM_DUMP_STATE eState);
	void Update_DeformDumpState(CKirby* pKirby, _float fTimeDelta);
	void Exit_DeformDumpState(CKirby* pKirby, DEFORM_DUMP_STATE eState);

private:
	DEFORM_DUMP_STATE m_eDeformDumpState{};
	_int m_iDeformDumpFlag{};

public:
	static CKirby_DeformDump* Create();
private:
	virtual void Free() override;
};

NS_END