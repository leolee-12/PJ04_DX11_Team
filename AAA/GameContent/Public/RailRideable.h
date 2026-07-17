#pragma once
#include "LevelDesign_LoadTypes.h"

/* ---------- RailRideable ---------- */
// - Rail을 타는 몬스터, 코스터가 상속
// - 사용법
// -- Bind_Rail 내부에서 검증 후 Set_RailDesc
/* -------------------------------------- */

NS_BEGIN(Client)
class CRailTrack;

struct RAIL_BIND_CONTEXT
{
    const LD_RAIL_DESC* pRailDesc = nullptr;
    const CRailTrack* pRailTrack = nullptr;

    _uint iRailUid = 0u;
    _uint iStartNodeIndex = 0u;
};

class CLIENT_DLL IRailRideable
{
public:
	virtual ~IRailRideable() = default;
	virtual void Set_RailDesc(const LD_RAIL_DESC& RailDesc) = 0;
    virtual HRESULT Bind_Rail(const RAIL_BIND_CONTEXT& Context) = 0;
};

NS_END