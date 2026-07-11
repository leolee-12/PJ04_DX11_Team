#pragma once

#include "Kirby_AttackMode.h"

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
protected:
	CKirby_Deform();
	virtual ~CKirby_Deform() = default;

protected:
	HRESULT Initialize();

public:
	virtual DEFORM_TYPE Get_DeformType() = 0;

	virtual void Enter_Deform(CKirby* pKirby) = 0;
	virtual void Exit_Deform(CKirby* pKirby) = 0;

public:
	void Play_DeformAni(CKirby* pKirby, DEFORM_ANI eDeformAni);

protected:
	void Set_FullBodyAni(DEFORM_ANI eAni, const _string& strAniName, _bool bLoop = true, _bool bRestart = false, _float fBlend = 0.1f, _float fSpeed = 1.f);

protected:
	virtual void Free() override;
};

NS_END