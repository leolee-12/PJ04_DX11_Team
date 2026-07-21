#pragma once

#include "Kirby_Deform.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Deform_Cylinder final : public CKirby_Deform
{
private:
	static constexpr _float s_fCylinderMaxHorizontalSpeed = 35.f;
	static constexpr _float s_fCylinderCarFallVelocity = -42.f;

	// Controller
	static constexpr _float s_fCylinder_CCT_Radius = 1.4f;
	static constexpr _float s_fCylinder_CCT_Height = 0.05f;

private:
	enum DEFORM_CYLINDER_STATE
	{
		ROT_MOVEDIR, ROLL, 
		CLASH, CLASH_SLIP_WALL, CLASHED_REMOVE_WALL, CLASHED_LANDING, CLASHED_WAIT,
		CYLINDER_STATE_END
	};

	enum ROLL_STATE { MOVE, JUMP, FALL, LANDING, ROLL_STATE_END };

private:
	CKirby_Deform_Cylinder();
	virtual ~CKirby_Deform_Cylinder() = default;

private:
	HRESULT Initialize();

public:
	virtual DEFORM_TYPE Get_DeformType() override;

	// Deform 진입
	virtual void Enter_Deform(CKirby* pKirby) override;
	virtual void Exit_Deform(CKirby* pKirby) override;

	// Attack 상태 진입
	virtual void Enter_AttackState(CKirby* pKirby, _int iFlag) override;
	virtual void Update_AttackState(CKirby* pKirby, _float fTimeDelta) override;
	virtual void Exit_AttackState(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

	// GetDeform 변신 추가 로직
	virtual void Enter_DeformState_Deform(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext) override;
	virtual _bool Update_DeformState_Deform(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext, _float fTimeDelta) override;
	virtual void Exit_DeformState_Deform(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext) override;

public:
	virtual _bool Should_ForceEnterAttackState() override { return true; }

private:
	void Change_DeformCylinderState(CKirby* pKirby, DEFORM_CYLINDER_STATE eNext);
	void Enter_DeformCylinderState(CKirby* pKirby, DEFORM_CYLINDER_STATE eState);
	void Update_DeformCylinderState(CKirby* pKirby, _float fTimeDelta);
	void Exit_DeformCylinderState(CKirby* pKirby, DEFORM_CYLINDER_STATE eState);

	void Rot_MoveDir(CKirby* pKirby, _float fTimeDelta);
	void Roll(CKirby* pKirby, _float fTimeDelta);
	void Roll_RotL(CKirby* pKirby, _float fTimeDelta);

	_bool Check_FrontCollision(CKirby* pKirby);

private:
	void Change_RollState(CKirby* pKirby, ROLL_STATE eNext);
	void Enter_RollState(CKirby* pKirby, ROLL_STATE eState);
	void Update_RollState(CKirby* pKirby, _float fTimeDelta);
	void Exit_RollState(CKirby* pKirby, ROLL_STATE eState);

private:
	_float3 m_fMoveDir{};
	DEFORM_CYLINDER_STATE m_eCylinderState{};

private:
	ROLL_STATE m_eRollState{};
	_bool m_bTryJump{};
	_float m_fRotL_Degree{};

	CSound_Handle m_RollSound{};

public:
	static CKirby_Deform_Cylinder* Create();

private:
	virtual void Free() override;
};

NS_END
