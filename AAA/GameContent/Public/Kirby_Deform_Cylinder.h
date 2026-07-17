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

private:
	enum DEFORM_CYLINDER_STATE { ROT_MOVEDIR, ROLL, CYLINDER_STATE_END };

private:
	CKirby_Deform_Cylinder();
	virtual ~CKirby_Deform_Cylinder() = default;

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
	virtual void Enter_Deform(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext) override;
	virtual _bool Update_Deform(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext, _float fTimeDelta) override;
	virtual void Exit_Deform(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext) override;

public:
	virtual _bool Should_ForceEnterAttackState() override { return true; }

private:
	void Change_DeformCylinderState(CKirby* pKirby, DEFORM_CYLINDER_STATE eNext);
	void Enter_DeformCylinderState(CKirby* pKirby, DEFORM_CYLINDER_STATE eState);
	void Update_DeformCylinderState(CKirby* pKirby, _float fTimeDelta);
	void Exit_DeformCylinderState(CKirby* pKirby, DEFORM_CYLINDER_STATE eState);

	void Rot_MoveDir(CKirby* pKirby, _float fTimeDelta);
	void Roll(CKirby* pKirby, _float fTimeDelta);

private:
	_float3 m_fMoveDir{};
	DEFORM_CYLINDER_STATE m_eCylinderState{};

public:
	static CKirby_Deform_Cylinder* Create();

private:
	virtual void Free() override;
};

NS_END
