#pragma once
#include "BTNode.h"

NS_BEGIN(Engine)
class CBlackboard;

// 자식 1개 감싸는 베이스 (소유권 인수)
class ENGINE_DLL CBTDecorator abstract : public CBTNode
{
protected:
	CBTDecorator(CBTNode* pChild);
	virtual ~CBTDecorator() = default;

public:
	virtual void Reset() override;

protected:
	CBTNode* m_pChild = nullptr;

public:
	virtual void Free() override;
};

// 성공 <-> 실패 반전 (RUNNING은 그대로)
class ENGINE_DLL CBTInverter final : public CBTDecorator
{
private:
	CBTInverter(CBTNode* pChild) : CBTDecorator(pChild) {}
	virtual ~CBTInverter() = default;

public:
	virtual BT_STATUS Tick(CBlackboard* pBB, _float fDt) override;

public:
	static CBTInverter* Create(CBTNode* pChild);
	virtual void Free() override;
};

// 쿨다운: 마지막 성공 이후 fColldown 동안은 실패
class ENGINE_DLL CBTCooldown final : public CBTDecorator
{
private:
	CBTCooldown(CBTNode* pChild, _float fCooldown) 
		: CBTDecorator(pChild), m_fCooldown(fCooldown) {}
	virtual ~CBTCooldown() = default;

public:
	virtual BT_STATUS Tick(CBlackboard* pBB, _float fDt) override;
	virtual void Reset() override;

private:
	_float m_fCooldown = { 0.f };
	_float m_fElapsed  = { 0.f };

public:
	static CBTCooldown* Create(CBTNode* pChild, _float fCooldown);
	virtual void Free() override;
};

NS_END
