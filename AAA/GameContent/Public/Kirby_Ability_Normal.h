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

	static constexpr _float s_fSpitSpeed = 14.f;

private:
	CKirby_Ability_Normal();
	virtual ~CKirby_Ability_Normal() = default;

private:
	HRESULT Initialize();

public:
	virtual COPY_ABILITY_TYPE Get_AbilityType() override;

	virtual void Enter_AttackState(CKirby* pKirby) override;
	virtual void Update_AttackState(CKirby* pKirby, _float fTimeDelta) override;
	virtual void Exit_AttackState(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

	virtual _bool Enter_Attack_KeyDown(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyPress(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyUp(CKirby* pKirby) override;

public:
	virtual void On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo) override;

private:
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

	//윤석현 수정 *변수/상수 위치 원하는 위치로 옮기세용*
	//IInhalable* m_pCapturedInhalable{};
	static constexpr _uint  s_iMaxCaptured = 3;
	IInhalable* m_CapturedInhalables[s_iMaxCaptured]{};
	_uint       m_iCapturedCount{};

	static constexpr _uint  s_iMaxCandidates = 8;
	IInhalable* m_FrameCandidates[s_iMaxCandidates]{};
	_uint        m_iFrameCandidateCount{};

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
	_bool Capture_Inhalable(IInhalable* pInhalable);
	void  Spit_Inhalable(CKirby* pKirby);
	void  Add_Candidate(IInhalable* p);
	void  Resolve_Captures(CKirby* pKirby);
	void  Clear_Captured();
	void  Clear_Candidates();

	void Off_InhaleEffect();

public:
	static CKirby_Ability_Normal* Create();
private:
	virtual void Free() override;
};

NS_END