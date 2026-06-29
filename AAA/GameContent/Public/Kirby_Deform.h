#pragma once

#include "Base.h"
#include "Animator.h"

#include "GameContent_Defines.h"

#include "Kirby_Command.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Deform abstract : public CBase
{
protected:
	CKirby_Deform();
	virtual ~CKirby_Deform() = default;

protected:
	HRESULT Initialize();

public:
	virtual DEFORM_TYPE Get_DeformType() = 0;

	virtual void Enter_Ability(CKirby* pKirby) = 0;
	virtual void Update_Ability(CKirby* pKirby, _float fTimeDelta) = 0;
	virtual void Exit_Ability(CKirby* pKirby) = 0;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) = 0;

	virtual _bool Enter_Attack_KeyDown(CKirby* pKirby) = 0;
	virtual _bool Enter_Attack_KeyPress(CKirby* pKirby) = 0;
	virtual _bool Enter_Attack_KeyUp(CKirby* pKirby) = 0;

	_bool ReqEndAttackState() { return m_bReqEndAttackState; }

protected:
	_bool m_bReqEndAttackState{ true };

protected:
	virtual void Free() override;
};

NS_END