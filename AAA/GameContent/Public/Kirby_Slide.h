#pragma once

#include "Kirby_State.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_Body;

class CLIENT_DLL CKirby_Slide final : public CKirby_State
{
private:
	enum SLIDE_STATE { SLIDE_START, SLIDE, BACK_JUMP, STATE_END };

private:
	static constexpr _float s_fSlideGroundFriction = 30.f;
	static constexpr _float s_fMaxSlideHorizontalSpeed = 23.f;
	static constexpr _float s_fMaxSlideTime = 0.6f;

private:
	CKirby_Slide();
	virtual ~CKirby_Slide() = default;

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
	virtual void On_KirbyCollisionEnter(CKirby* pKirby, _uint iColliderType, CCollider* pOther) override;

public:
	virtual void On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo) override;

private:
	void Change_SlideState(CKirby* pKirby, SLIDE_STATE eNextState);
	void Enter_SlideState(CKirby* pKirby, SLIDE_STATE eState);
	void Update_SlideState(CKirby* pKirby, const _float fTimeDelta);
	void Exit_SlideState(CKirby* pKirby, SLIDE_STATE eState);

private:
	SLIDE_STATE m_eSlideState{ STATE_END };

	_float3 m_vSlideDir{};
	_float m_fSlideTime{};

public:
	static CKirby_Slide* Create();
private:
	virtual void Free() override;
};

NS_END
