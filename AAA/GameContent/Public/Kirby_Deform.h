#pragma once

#include "Kirby_AttackMode.h"

#include "Kirby_DeformContext.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;
struct ATTACK_INFO;

enum class DEFORM_ANI
{
	WAIT, RUN, FALL, LANDING,
	JUMP_START, JUMP,
	DAMAGED,
	SPIT_START,
	END
};

class CLIENT_DLL CKirby_Deform abstract : public CKirby_AttackMode
{
public:
	struct POST_DEFORM_END_INFO
	{
		
	};

protected:
	CKirby_Deform();
	virtual ~CKirby_Deform() = default;

protected:
	HRESULT Initialize();

public:
	virtual DEFORM_TYPE Get_DeformType() = 0;

	// Deform 진입
	virtual void Enter_Deform(CKirby* pKirby) = 0;
	virtual void Exit_Deform(CKirby* pKirby) = 0;

	// Get_Deform 변신 추가 로직
	virtual void Enter_DeformState_Deform(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext);
	virtual _bool Update_DeformState_Deform(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext, _float fTimeDelta) { return true; }
	virtual void Exit_DeformState_Deform(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext) {}

	virtual _bool HasDemoModel() { return true; }

	// DeformDump용 함수
	virtual void On_DumpSpitStart(CKirby* pKirby);
	virtual void On_DumpSpitDeform(CKirby* pKirby) {}

	void Play_DeformAni(CKirby* pKirby, DEFORM_ANI eDeformAni);

protected:
	void Set_FullBodyAni(DEFORM_ANI eAni, const _string& strAniName, _bool bLoop = true, _bool bRestart = false, _float fBlend = 0.1f, _float fSpeed = 1.f);

protected:
	virtual void Free() override;
};

NS_END