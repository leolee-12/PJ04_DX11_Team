#pragma once

#include "Base.h"

#include "Animator.h"

#include "GameContent_const.h"

#include "Kirby_Command.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
class CEffect_Container;
NS_END

NS_BEGIN(Client)

class CKirby;
struct ATTACK_INFO;

enum class KIRBY_ATTACK_LOCATION { GROUND, AIR };

class CLIENT_DLL CKirby_AttackMode abstract : public CBase
{
protected:
	enum ANI_PLAY_TYPE { FULL_BODY, OVERLAY };

	struct KIRBY_TYPE_ANI_DESC
	{
		CAnimator::ANI_PLAY_INFO tBaseAniInfo;
		ANI_PLAY_TYPE ePlayType = ANI_PLAY_TYPE::FULL_BODY;
		CAnimator::LAYER_PLAY_INFO tLayerAniInfo;
	};

protected:
	CKirby_AttackMode();
	virtual ~CKirby_AttackMode() = default;

protected:
	HRESULT Initialize();

public:
	virtual void Enter_AttackState(CKirby* pKirby, _int iFlag) = 0;
	virtual void Update_AttackState(CKirby* pKirby, _float fTimeDelta) = 0;
	virtual void Exit_AttackState(CKirby* pKirby) = 0;

	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) = 0;

	virtual _bool Enter_Attack_KeyDown(CKirby* pKirby) = 0;
	virtual _bool Enter_Attack_KeyPress(CKirby* pKirby) = 0;
	virtual _bool Enter_Attack_KeyUp(CKirby* pKirby) = 0;

public:
	virtual _bool Can_Attack(KIRBY_ATTACK_LOCATION eAttackLocation);
	_bool Get_ReqEndAttackState() { return m_bReqEndAttackState; }

public:
	virtual void On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo);

public:
	void Clear_Overlay(CKirby* pKirby, _uint iSlot = 1, _float fOverlayBlendTime = 0.1f);

	void Effect_Stop(CEffect_Container*& pContainer1);

	const _wstring& Get_AttackModeName() { return m_wstrAttackModeName; }

protected:
	CGameInstance_Proxy* m_pGameInstance_Proxy{};

	_bool m_bReqEndAttackState{ true };

	vector<KIRBY_TYPE_ANI_DESC> m_AniInfos;

	_wstring m_wstrAttackModeName = L"NULL";

protected:
	virtual void Free() override;
};

NS_END