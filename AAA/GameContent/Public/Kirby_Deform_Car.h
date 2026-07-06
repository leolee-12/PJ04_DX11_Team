#pragma once

#include "Kirby_Deform.h"

NS_BEGIN(Engine)
class CEffect_Container;
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Deform_Car final : public CKirby_Deform
{
private:
	static constexpr _float s_fDeformCar_Rot_Speed_Degree = 480.f;

	static constexpr _float s_fCarSpeed = 13.f;
	static constexpr _float s_fMaxBoostSpeed = 20.f;
	static constexpr _float s_fBoostAcceleration = 180.f;

	enum DEFORM_CAR_STATE { BOOST, BOOST_END, CRUSH, DEFORM_CAR_END };
	enum BOOST_JUMP_STATE { GROUND, JUMP_START, JUMP, FALL, LANDING };

private:
	CKirby_Deform_Car();
	virtual ~CKirby_Deform_Car() = default;

private:
	HRESULT Initialize();

public:
	virtual DEFORM_TYPE Get_DeformType() override;

	virtual void Enter_Deform(CKirby* pKirby) override;
	virtual void Exit_Deform(CKirby* pKirby) override;

	virtual void Enter_AttackState(CKirby* pKirby, _int iFlag) override;
	virtual void Update_AttackState(CKirby* pKirby, _float fTimeDelta) override;
	virtual void Exit_AttackState(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

	virtual _bool Enter_Attack_KeyDown(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyPress(CKirby* pKirby) override;
	virtual _bool Enter_Attack_KeyUp(CKirby* pKirby) override;

public:
	virtual void On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo) override;

private:
	_float m_fMaxBoostTime{};
	_float m_fAccBoostTime{};

	DEFORM_CAR_STATE m_eDeformCar_State{};
	BOOST_JUMP_STATE m_eBoostJumpState{};

	_float3 m_vRotDir{};

	CEffect_Container* m_pBoostGas1{};
	CEffect_Container* m_pBoostGas2{};
	CEffect_Container* m_pBoostWind{};

private:
	void Change_DeformCarState(CKirby* pKirby, DEFORM_CAR_STATE eNext);
	void Enter_DeformCarState(CKirby* pKirby, DEFORM_CAR_STATE eState);
	void Update_DeformCarState(CKirby* pKirby, _float fTimeDelta);
	void Exit_DeformCarState(CKirby* pKirby, DEFORM_CAR_STATE eStaten);

	void Change_BoostJumpState(CKirby* pKirby, BOOST_JUMP_STATE eNext);
	void Enter_BoostJumpState(CKirby* pKirby, BOOST_JUMP_STATE eState);
	void Update_BoostJumpState(CKirby* pKirby, _float fTimeDelta);
	void Exit_BoostJumpState(CKirby* pKirby, BOOST_JUMP_STATE eStaten);

	void BoostEffectStart(CKirby* pKirby, CEffect_Container*& pContainer1, CEffect_Container*& pContainer2, const _tchar* EffectTag);

public:
	static CKirby_Deform_Car* Create();
private:
	virtual void Free() override;
};

NS_END