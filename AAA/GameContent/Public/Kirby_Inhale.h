#pragma once

#include "Kirby_State.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Inhale final : public CKirby_State
{
private:
	enum class INHALE_STATE
	{
		INHALE,
		SUPER_INHALE_START, SUPER_INHALE_LOOP,
		INHALE_END 
	};

private:
	CKirby_Inhale();
	virtual ~CKirby_Inhale() = default;

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
	INHALE_STATE m_eInhaleState{};

	_float m_MaxSuperInHaleTime{};
	_float m_AccSuperInHaleTime{};

public:
	static CKirby_Inhale* Create();
private:
	virtual void Free() override;
};

NS_END