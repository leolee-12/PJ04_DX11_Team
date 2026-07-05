#pragma once

#include "Kirby_AttackMode.h"

#include "Animator.h"

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
	GET_ABILITY, ABILITY_DUMP,
	FLIGHT_START, FLIGHT, FLIGHT_FALL, FLIGHT_LANDING, AIR_BALL,
	STUFFED_START, STUFFED_WAIT, STUFFED_RUN, STUFFED_JUMP, STUFFED_FALL, STUFFED_LANDING, STUFFED_SPIT,
	DAMAGED,
	GUARD,
	END
};

class CLIENT_DLL CKirby_Ability abstract : public CKirby_AttackMode
{
protected:
	enum ABILITY_ANI_PLAY_TYPE { FULL_BODY, OVERLAY };

	struct ABILITY_ANI_DESC
	{
		CAnimator::ANI_PLAY_INFO tBaseAniInfo;

		ABILITY_ANI_PLAY_TYPE ePlayType = ABILITY_ANI_PLAY_TYPE::FULL_BODY;

		CAnimator::ANI_PLAY_INFO tOverlayAniInfo;
		_string strOverlayRootBone;
		_float fOverlayWeight = 1.f;
		_float fOverlayBlend = 0.1f;
	};

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
	void Clear_Overlay(CKirby* pKirby, _uint iSlot = 1, _float fOverlayBlendTime = 0.1f);

protected:
	vector<ABILITY_ANI_DESC> m_tAniInfos;

protected:
	void Set_FullBodyAni(ABILITY_ANI eAni, const _string& strAniName, _bool bLoop = true, _bool bRestart = false, _float fBlend = 0.1f, _float fSpeed = 1.f);

	void Set_OverlayAni(ABILITY_ANI eAni, const _string& strBaseAniName, const _string& strOverlayAniName, const _string& strRootBone,
		_bool bBaseLoop = true, _bool bBaseRestart = true, _float fBaseSpeed = 1.f, _float fBaseBlend = 0.1f, _bool bBaseClearMask = true,
		_bool bOverlayLoop = true, _bool bOverlayRestart = true, _float fOverlaySpeed = 1.f, _float fOverlayBlend = 0.1f);

protected:
	virtual void Free() override;
};

NS_END