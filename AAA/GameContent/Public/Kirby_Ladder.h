#pragma once

#include "Kirby_ControllableState.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;
class CLevelDesign_Ladder;

class CLIENT_DLL CKirby_Ladder final : public CKirby_ControllableState
{
private:
	enum LADDER_STATE { WAIT, MOVE, LADDER_TOP_JUMP, LADDER_END };

private:
	CKirby_Ladder();
	virtual ~CKirby_Ladder() = default;

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

private:
	LADDER_STATE m_eLadderState{};

	_int m_iCurLadderIndex{};
	_int m_iNextLadderIndex{};

	_int m_iCurMoveDir{};
	_int m_iPreMoveDir{};

	_bool m_bPlayAniLadderUp{};
	_int m_iRemainDownAniCells{};
	_int m_iMaxDownAniCells{};

	_float m_fLadderSpeed{};

private:
	void Change_LadderState(CKirby* pKirby, LADDER_STATE eNext);
	void Enter_LadderState(CKirby* pKirby, LADDER_STATE eState);
	void Update_LadderState(CKirby* pKirby, _float fTimeDelta);
	void Exit_LadderState(CKirby* pKirby, LADDER_STATE eState);

	void Set_NextCell();
	_bool Handle_LadderTopBottom(CKirby* pKirby, CLevelDesign_Ladder* pLadder);

	void Apply_LadderUp(CKirby* pKirby);
	void Apply_LadderDown(CKirby* pKirby);

public:
	static CKirby_Ladder* Create();
private:
	virtual void Free() override;
};

NS_END