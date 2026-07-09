#pragma once

#include "Kirby_AttackMode.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;
struct ATTACK_INFO;

enum class ABILITY_ANI
{
	WAIT, RUN, FALL,
	JUMP_L, JUMP_R, JUMP_END_L, JUMP_END_R,
	LANDING,
	GET_ABILITY, COPY, ABILITY_DUMP,
	FLIGHT_START, FLIGHT, FLIGHT_FALL, FLIGHT_LANDING, AIR_BALL,
	STUFFED_START, STUFFED_WAIT, STUFFED_RUN, STUFFED_JUMP, STUFFED_FALL, STUFFED_LANDING, STUFFED_SPIT,
	DAMAGED,
	GUARD,
	SLIDE_START, SLIDE, SLIDE_END,
	DODGE_START,
	END
};

class CLIENT_DLL CKirby_Ability abstract : public CKirby_AttackMode
{
protected:
	CKirby_Ability();
	virtual ~CKirby_Ability() = default;

protected:
	HRESULT Initialize();

public:
	virtual COPY_ABILITY_TYPE Get_AbilityType() = 0;

public:
	const CAnimator::ANI_PLAY_INFO* Get_AniInfo(ABILITY_ANI eAbilityAni);

	void Play_AbilityAni(CKirby* pKirby, ABILITY_ANI eAbilityAni);

protected:
	void Set_FullBodyAni(ABILITY_ANI eAni, const _string& strAniName, _bool bLoop = true, _bool bRestart = false, _float fBlend = 0.1f, _float fSpeed = 1.f);

	void Set_OverlayAni(ABILITY_ANI eAni, const _string& strBaseAniName, const _string& strOverlayAniName, const _string& strRootBone,
		_bool bBaseLoop = true, _bool bBaseRestart = true, _float fBaseSpeed = 1.f, _float fBaseBlend = 0.1f,
		_bool bOverlayLoop = true, _bool bOverlayRestart = true, _float fOverlaySpeed = 1.f, _float fTargetWeight = 1.f, _float fWeightBlendTime = 0.1f,
		_float fOverlayClipBlend = 0.1f);

protected:
	virtual void Free() override;
};

NS_END
