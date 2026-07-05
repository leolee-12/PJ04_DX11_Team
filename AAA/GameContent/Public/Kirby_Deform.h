#pragma once

#include "Kirby_AttackMode.h"

#include "Animator.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;
struct ATTACK_INFO;

enum class DEFORM_ANI
{
	WAIT, RUN, FALL, LANDING,
	JUMP_START, JUMP,
	DAMAGE,
	END
};

class CLIENT_DLL CKirby_Deform abstract : public CKirby_AttackMode
{
protected:
	enum DEFORM_ANI_PLAY_TYPE { FULL_BODY, OVERLAY };

	struct DEFORM_ANI_DESC
	{
		CAnimator::ANI_PLAY_INFO tBaseAniInfo;
		DEFORM_ANI_PLAY_TYPE ePlayType = DEFORM_ANI_PLAY_TYPE::FULL_BODY;
		CAnimator::LAYER_PLAY_INFO tLayerAniInfo;
	};

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
	vector<DEFORM_ANI_DESC> m_tDeformAniInfos;

protected:
	void Set_FullBodyAni(DEFORM_ANI eAni, const _string& strAniName, _bool bLoop = true, _bool bRestart = false, _float fBlend = 0.1f, _float fSpeed = 1.f);

protected:
	virtual void Free() override;
};

NS_END