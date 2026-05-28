#include "Layer.h"
#include "GameObject.h"

CLayer::CLayer()
{
}

HRESULT CLayer::Add_GameObject(CGameObject* pGameObject)
{
	if (nullptr == pGameObject) return E_FAIL;

    m_GameObjects.push_back(pGameObject);
    return S_OK;
}

void CLayer::Priority_Update(_float fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (nullptr != pGameObject)
			pGameObject->Priority_Update(fTimeDelta);
	}
}

void CLayer::Update(_float fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (nullptr != pGameObject)
			pGameObject->Update(fTimeDelta);
	}
}

void CLayer::Late_Update(_float fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (nullptr != pGameObject)
			pGameObject->Late_Update(fTimeDelta);
	}
}

CGameObject* CLayer::Find_GameObject(const wstring& strObjectTag)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (pGameObject->Get_ObjectTag() == strObjectTag)
			return pGameObject;
	}
	return nullptr;
}

void CLayer::Remove_GameObject(CGameObject* pGameObject)
{
	for (auto iter = m_GameObjects.begin(); iter != m_GameObjects.end(); ++iter)
	{
		if ((*iter) == pGameObject)
		{
			Safe_Release(*iter);
			m_GameObjects.erase(iter);
			return;
		}
	}
}


CLayer* CLayer::Create()
{
	return new CLayer();
}

void CLayer::Free()
{
	__super::Free();

	for (auto& pGameObject : m_GameObjects)
		Safe_Release(pGameObject);

	m_GameObjects.clear();
}
