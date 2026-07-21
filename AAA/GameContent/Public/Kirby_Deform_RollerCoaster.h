#pragma once

#include "Kirby_Deform.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

class CLevelDesign_Rail;
class CRailTrack;

class CLIENT_DLL CKirby_Deform_RollerCoaster final : public CKirby_Deform
{
private:
	enum DEFORM_ROLLERCOASTER_STATE { RUNNING, WAIT, ROLLERCOASTER_STATE_END };

private:
	CKirby_Deform_RollerCoaster();
	virtual ~CKirby_Deform_RollerCoaster() = default;

private:
	HRESULT Initialize();

public:
	virtual DEFORM_TYPE Get_DeformType() override;

	// Deform 진입
	virtual void Enter_Deform(CKirby* pKirby) override;
	virtual void Exit_Deform(CKirby* pKirby) override;

	virtual void Enter_AttackState(CKirby* pKirby, _int iFlag) override;
	virtual void Update_AttackState(CKirby* pKirby, _float fTimeDelta) override;
	virtual void Exit_AttackState(CKirby* pKirby) override;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

	virtual _bool HasDemoModel() { return false; }

	// GetDeform 변신 추가 로직
	virtual void Enter_DeformState_Deform_End(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& tDeformContext) override;
	virtual _bool Update_DeformState_Deform_End(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& tDeformContext, _float fTimeDelta) override;
	virtual void Exit_DeformState_Deform_End(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& tDeformContext) override;

	// DeformDump용 함수
	virtual _bool Has_SpitStartAni() { return false; }

public:
	virtual _bool Should_ForceEnterAttackState() override { return true; }

private:
	void Change_CoasterState(CKirby* pKirby, DEFORM_ROLLERCOASTER_STATE eNext);
	void Enter_CoasterState(CKirby* pKirby, DEFORM_ROLLERCOASTER_STATE eState);
	void Update_CoasterState(CKirby* pKirby, _float fTimeDelta);
	void Exit_CoasterState(CKirby* pKirby, DEFORM_ROLLERCOASTER_STATE eState);

private:
	_bool Update_OnRail(CKirby* pKirby, _float fTimeDelta);
	_bool Set_OnRail(CKirby* pKirby, _float fRailDist);

	void Set_RotLeftRight(_float fTimeDelta);

private:
	DEFORM_ROLLERCOASTER_STATE m_eRollerCoasterState{};

	_uint m_iRailUid{};
	_uint m_iStartNodeIndex{};

	const CLevelDesign_Rail* m_pRail{};
	const CRailTrack* m_pRailTrack{};

	_float m_fRailLength{};
	_float m_fCurRailDist{};
	_float m_fCurFrameMoveDist{};
	_float m_fSlopeRatio{};

	_float m_fAccRailSpeed{};

	CSound_Handle m_RunningSound{};

private:
	_float m_fLeftRightDegree{};
	_int m_iLeftRight{};

public:
	static CKirby_Deform_RollerCoaster* Create();
private:
	virtual void Free() override;
};

NS_END
