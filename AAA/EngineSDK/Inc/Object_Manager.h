#pragma once

#include "Base.h"

/* 1.실제 동작을 할 객체(사본객체)를 보관한다. */
/* 1-1. 레벨별로 구분짓자. */
/* 1-2. 레이어별로 그룹짓는다. */

/* 2. 보관하고 있는 객체들의 갱신(Update, Tick) 을 수행해준다. */

NS_BEGIN(Engine)

class CGameInstance_Proxy;
class CLayer;
class CGameObject;

class CObject_Manager final : public CBase
{
private:
	CObject_Manager();
	virtual ~CObject_Manager() = default;

public:
	HRESULT Initialize(_uint iNumLevels);
	HRESULT Add_GameObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLayerLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag, void* pArg);
	HRESULT Add_GameObject_Return(CGameObject** ppOut, _uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLayerLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag, void* pArg);
	void Priority_Update(_float fTimeDelta);
	void Update(_float fTimeDelta);
	void Late_Update(_float fTimeDelta);
	void Clear(_uint iLevelIndex);

	CGameObject* Find_GameObject(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag);

public:
	void Request_Destroy(CGameObject* pGameObject);
	void Flush_DeadObjects();

private:
	size_t										m_iNumLevels = {};
	unordered_map<_wstring, CLayer*>*			m_pLayers = { nullptr };
	typedef unordered_map<_wstring, CLayer*>	LAYERS;

	CGameInstance_Proxy*						m_pGameInstance_Proxy = { nullptr };

	vector<CGameObject*>						m_PendingDestroyObjects;

private:
	CLayer* Find_Layer(_uint iLayerLevelIndex, const _wstring& strLayerTag);


public:
	static CObject_Manager* Create(_uint iNumLevels);
	virtual void Free() override;

};

NS_END