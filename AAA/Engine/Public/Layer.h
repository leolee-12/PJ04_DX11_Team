#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CGameObject;

class CLayer final : public CBase
{
private:
	CLayer();
	virtual ~CLayer() = default;

public:
	HRESULT Add_GameObject(CGameObject* pGameObject);
	void Priority_Update(_float fTimeDelta);
	void Update(_float fTimeDelta);
	void Late_Update(_float fTimeDelta);

	CGameObject* Find_GameObject(const wstring& strObjectTag);

public:
	void Remove_GameObject(CGameObject* pGameObject);

private:
	list<CGameObject*>			m_GameObjects;
	//unordered_map<_wstring, CGameObject*> m_GameObjects;

public:
	static CLayer* Create();
	virtual void Free() override;
};

NS_END