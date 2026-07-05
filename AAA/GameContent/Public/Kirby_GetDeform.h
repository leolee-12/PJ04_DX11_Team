#pragma once

#include "Kirby_State.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
class CAnimator;

class CEffect_Container;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_Body;

class CLIENT_DLL CKirby_GetDeform final : public CKirby_State
{
private:	
	enum DEFORM_STATE { SUPER_INHALE_START, SUPER_INHALE_LOOP, DEFORM, DEFORM_END, DEFORM_STATE_END };

	static constexpr _float s_fDeform_End_Rot_Speed_Degree = 560.f;

private:
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

private:
	DEFORM_STATE m_eDeformState{};

	// 변신할 타입
	DEFORM_TYPE m_eDeformType{};

	// Effect
	CEffect_Container* m_pInhaleEffect{};

	_float3 m_vRotationDir{};

public:
	static CKirby_GetDeform* Create();
private:
	virtual void Free() override;
};

NS_END