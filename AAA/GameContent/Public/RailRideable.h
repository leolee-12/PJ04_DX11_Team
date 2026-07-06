#pragma once
#include "LevelDesign_LoadTypes.h"

/* ---------- RailRideable ---------- */
// - Rail을 타는 몬스터만 상속
// - 사용법
// -- class CSomeMonster final : public CMonster, public IRailRideable
// -- pReceiver->Set_RailDesc(pRail->Get_RailDesc());
/* -------------------------------------- */

NS_BEGIN(Client)

class CLIENT_DLL IRailRideable
{
public:
	virtual ~IRailRideable() = default;
	virtual void Set_RailDesc(const LD_RAIL_DESC& RailDesc) = 0;
};

NS_END