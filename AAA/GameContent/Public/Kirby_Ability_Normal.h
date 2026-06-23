#pragma once

#include "Kirby_Ability.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CAnimator;
class CEffect_Container;

class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

class CKirby;
class CMovement_Child;

class CMonster;

class IInhalable;

class CLIENT_DLL CKirby_Ability_Normal final : public CKirby_Ability
{
private:
	enum class INHALE_STATE
	{
		INHALE_LOOP,
		SUPER_INHALE_START, SUPER_INHALE_LOOP,
		INHALE_END,
		STUFFED_START,

		STUFFED_SPIT,

		NORMAL_EXIT,
	};

	enum class INHALE_MOVE_STATE
	{
		WAIT, WALK, FALL
	};

	enum class MOUTH_STATE { DEFAULT, STUFFFED };

private:
	CKirby_Ability_Normal();
	virtual ~CKirby_Ability_Normal() = default;

private:
	HRESULT Initialize();

public:
	virtual COPY_ABILITY_TYPE Get_AbilityType() override;

	virtual void Enter_Ability(CKirby* pKirby) override;
	virtual ABILITY_UPDATE_RESULT Update_Ability(CKirby* pKirby, _float fTimeDelta) override;
	virtual void Exit_Ability(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

	virtual _bool Enter_Attack_KeyDown(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyPress(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyUp(CKirby* pKirby) override;

	virtual _bool Can_Attack(KIRBY_ATTACK_LOCATION eAttackLocation) override;

private:
	CGameInstance_Proxy* m_pGameInstance_Proxy{};

	// State
	MOUTH_STATE m_eMouthState{};

	INHALE_STATE m_eInhaleState{};
	INHALE_MOVE_STATE m_eCurMoveState{};

	// Timer
	_float m_fMaxSuperInhaleTime{};
	_float m_fAccSuperInhaleTime{};

	// ReqEnd
	_bool m_bReqEndInhale{};

	// Event
	SUBHANDLE m_hInhaleCapturedEvent{};
	_bool m_bSubscribedInhaleCapturedEvent{};

	// Effect
	CEffect_Container* m_pInhaleEffect{};

	_bool m_bSuperInhaleEffectRaised{};
	_float3 m_vInhaleEffectStartPos{};
	_float3 m_vInhaleEffectEndPos{};

	// Inhaleable(Stuffed)
	_bool m_bInhaleCancelLocked{};
	IInhalable* m_pCapturedInhalable{};

private:
	void Change_InhaleState(CKirby* pKirby, INHALE_STATE eNext);
	void Enter_InhaleState(CKirby* pKirby, INHALE_STATE eState);
	void Update_InhaleState(CKirby* pKirby, _float fTimeDelta);
	void Exit_InhaleState(CKirby* pKirby, INHALE_STATE eState);

	void Change_MouthState(MOUTH_STATE eMouthState);
	void Update_InhaleMoveState(CKirby* pKirby);
	void Play_InhaleLoopAnimation(CKirby* pKirby);

	// Timer
	void Update_SuperInhaleTimer(_float fTimeDelta);
	// Kirby Reset
	void Restore_KirbyAfterInhale(CKirby* pKirby);

	// Inhale Collider
	void Start_InhaleCollider(CKirby* pKirby);
	void End_InhaleCollider(CKirby* pKirby);

	// Event
	void Subscribe_InhaleCapturedEvent(CKirby* pKirby);
	void Unsubscribe_InhaleCapturedEvent();
	void Handle_InhaleCaptured(CKirby* pKirby, IInhalable* pInhaleable);

	void Update_SuperInhaleEffectRise(_float fRatio);

	// Inhaleable(Stuffed)
	void  Capture_Inhalable(IInhalable* pInhalable) { m_pCapturedInhalable = pInhalable; }
	void  Spit_Inhalable(CKirby* pKirby);

public:
	static CKirby_Ability_Normal* Create();
private:
	virtual void Free() override;
};

NS_END