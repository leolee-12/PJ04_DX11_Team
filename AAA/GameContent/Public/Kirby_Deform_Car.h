#pragma once

#include "Kirby_Deform.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Deform_Car final : public CKirby_Deform
{
private:
	CKirby_Deform_Car();
	virtual ~CKirby_Deform_Car() = default;

private:
	HRESULT Initialize();

public:
	virtual DEFORM_TYPE Get_DeformType() override;

	virtual void Enter_Deform(CKirby* pKirby) override;
	virtual void Exit_Deform(CKirby* pKirby) override;

	virtual void Enter_DeformState(CKirby* pKirby) override;
	virtual void Update_DeformState(CKirby* pKirby, _float fTimeDelta) override;
	virtual void Exit_DeformState(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

	virtual _bool Enter_Attack_KeyDown(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyPress(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyUp(CKirby* pKirby) override;

public:
	static CKirby_Deform_Car* Create();
private:
	virtual void Free() override;
};

NS_END