#pragma once
#include "Base.h"
#include "LevelDesign_LoadTypes.h"

NS_BEGIN(Client)

class CRailTrack final : public CBase
{
private:
	CRailTrack() = default;
	virtual ~CRailTrack() = default;

public:
	void					Build(const LD_RAIL_DESC& Desc);
	_bool					Is_Valid() const {	return m_Table.size() >= 2;	}
	_float					Get_Length() const { return m_fLength; }
	_bool					Is_Loop() const { return m_bLoop; }
	_bool					Sample(_float fDist, _float3* pOutPos, _float3* pOutTarget = nullptr) const;

private:
	struct ENTRY 
	{ 
		_float fDist = {};
		_uint iSeg = {};
		_float fT = {};
	};

	LD_RAIL_DESC			m_Desc = {};
	vector<ENTRY>			m_Table;
	_float					m_fLength = { 0.f };
	_bool					m_bLoop = { false };
	mutable _uint			m_iHint = { 0 };

	static constexpr _uint	STEPS = { 16 };

public:
	static CRailTrack*		Create();
	virtual void			Free() override;
};

NS_END