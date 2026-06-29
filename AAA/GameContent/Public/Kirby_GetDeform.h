#pragma once

#include "Kirby_State.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_Body;

class CLIENT_DLL CKirby_GetDeform final : public CKirby_State
{
private:
	enum DEFORM_STATE { DEFORM, DEFORM_END };

	CKirby_GetDeform();
	virtual ~CKirby_GetDeform() = default;

private:
	HRESULT Initialize();

public:
	virtual KIRBY_STATE_TYPE Get_StateType() override;

public:
	virtual void Enter(CKirby* pKirby) override;
	virtual void Update(CKirby* pKirby, const _float fTimeDelta) override;
	virtual void Exit(CKirby* pKirby) override;

public:
	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

private:
	CGameInstance_Proxy* m_pGameInstance_Proxy = {};

	DEFORM_STATE m_eDeformState{};

public:
	static CKirby_GetDeform* Create();
private:
	virtual void Free() override;
};

NS_END