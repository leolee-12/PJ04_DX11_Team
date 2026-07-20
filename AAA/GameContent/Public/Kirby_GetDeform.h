#pragma once

#include "Kirby_State.h"

#include "Kirby_DeformContext.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
class CAnimator;

class CEffect_Container;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_Body;
class CKirby_Deform;

struct DEFORM_ACQUIRED_EVENT;

class CLIENT_DLL CKirby_GetDeform final : public CKirby_State
{
private:	
	enum DEFORM_STATE { SUPER_INHALE_START, SUPER_INHALE_LOOP, DEFORM_STATE_DEFORM, DEFORM_STATE_DEFORM_END, DEFORM_STATE_END };

	static constexpr _float s_fDeform_End_Rot_Speed_Degree = 560.f;

private:
	CKirby_GetDeform();
	virtual ~CKirby_GetDeform() = default;

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

public:
	virtual _bool Ignore_TimeScale() { return true; }

private:
	void Change_GetDeformState(CKirby* pKirby, DEFORM_STATE eNext);
	void Enter_GetDeformState(CKirby* pKirby, DEFORM_STATE eState);
	void Update_GetDeformState(CKirby* pKirby, _float fTimeDelta);
	void Exit_GetDeformState(CKirby* pKirby, DEFORM_STATE eState);

	void Set_RotationDir(CKirby* pKirby);

	// Event
	void Subscribe_DeformEvent(CKirby* pKirby);
	void Unsubscribe_DeformEvent();
	void Handle_DeformEvent(CKirby* pKirby, const DEFORM_ACQUIRED_EVENT* pDesc);

private:
	DEFORM_STATE m_eDeformState{};

	// 변신할 타입
	POST_DEFORM_END_CONTEXT m_tPostDeformEndContext{};
	CKirby_Deform* pPendingDeform{};
	DEFORM_TYPE m_eDeformType{};

	// Effect
	CEffect_Container* m_pInhaleEffect{};

	_float3 m_vRotationDir{};

	// Event
	SUBHANDLE m_DeformEvent{};
	_bool m_bSubscribedDeformEvent{};

public:
	static CKirby_GetDeform* Create();
private:
	virtual void Free() override;
};

NS_END