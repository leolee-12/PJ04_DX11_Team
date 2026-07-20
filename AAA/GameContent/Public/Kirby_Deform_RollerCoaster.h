#pragma once

#include "Kirby_Deform.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Deform_RollerCoaster final : public CKirby_Deform
{
private:
	CKirby_Deform_RollerCoaster();
	virtual ~CKirby_Deform_RollerCoaster() = default;

private:
	HRESULT Initialize();

public:
	virtual DEFORM_TYPE Get_DeformType() override;

	// Deform 진입
	virtual void Enter_Deform(CKirby* pKirby) override;
	virtual void Exit_Deform(CKirby* pKirby) override;

	// Get_Deform 변신 추가 로직
	virtual void Enter_AttackState(CKirby* pKirby, _int iFlag) override;
	virtual void Update_AttackState(CKirby* pKirby, _float fTimeDelta) override;
	virtual void Exit_AttackState(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

	virtual _bool HasDemoModel() { return false; }

	// DeformDump용 함수
	virtual void On_DumpSpitStart(CKirby* pKirby) override {};

public:
	virtual void On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo) override;

public:
	virtual _bool Should_ForceEnterAttackState() override { return true; }

public:
	static CKirby_Deform_RollerCoaster* Create();

private:
	virtual void Free() override;
};

NS_END
