#pragma once

#include "Kirby_State.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_Body;

class CLIENT_DLL CKirby_QTE_Grabbed final : public CKirby_State
{
private:
	CKirby_QTE_Grabbed();
	virtual ~CKirby_QTE_Grabbed() = default;

	enum QTE_GRABBED_STATE
	{
		START, ESCAPE,
		SLAM_SPIN, FAINT, WAKEUP,
		GRABBED_STATE_END
	};

private:
	HRESULT Initialize();

public:
	virtual KIRBY_STATE_TYPE Get_StateType() override;

public:
	virtual void Enter(CKirby* pKirby, _int iFlag) override;
	virtual void Update(CKirby* pKirby, const _float fTimeDelta) override;
	virtual void Exit(CKirby* pKirby) override;

public:
	virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

public:
	virtual void Request_Attachment_End(CKirby* pKirby, const KIRBY_ATTACHMENT_END_DESC* pDesc) override;

private:
	void Change_QTEGrabbedState(CKirby* pKirby, QTE_GRABBED_STATE eNext);
	void Enter_QTEGrabbedState(CKirby* pKirby, QTE_GRABBED_STATE eState);
	void Update_QTEGrabbedState(CKirby* pKirby, _float fTimeDelta);
	void Exit_QTEGrabbedState(CKirby* pKirby, QTE_GRABBED_STATE eState);

private:
	QTE_GRABBED_STATE m_eQTEGrabbedState{ QTE_GRABBED_STATE:: GRABBED_STATE_END };

	_bool m_bPublishedEvent{};
	_uint m_iQTE_InputCount{};
	_uint m_iQTE_RequiredCount{};

	_float m_fSpinAccTime{};

	_bool m_bDidSmallJump{};

	_bool m_bQTEStart = { false };

	SUBHANDLE m_tEventHandle{};

public:
	static CKirby_QTE_Grabbed* Create();
private:
	virtual void Free() override;
};

NS_END