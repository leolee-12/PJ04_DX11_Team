#include "Object_Manager.h"
#include "GameObject.h"
#include "Layer.h"

#include "GameInstance.h"

CObject_Manager::CObject_Manager()
	: m_pGameInstance_Proxy{ CGameInstance::GetProxy() }
{
}

HRESULT CObject_Manager::Initialize(_uint iNumLevels)
{
	if (nullptr != m_pLayers ||
		m_pGameInstance_Proxy == nullptr)
		return E_FAIL;

	m_iNumLevels = iNumLevels;

	m_pLayers = new LAYERS[iNumLevels];

	return S_OK;
}


HRESULT CObject_Manager::Add_GameObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLayerLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag, void* pArg)
{
	if (iLayerLevelIndex >= m_iNumLevels ||
		nullptr == m_pLayers)
		return E_FAIL;

	/* 레이어에 추가해야할 사본을 준비했다. */
	CGameObject* pGameObject = dynamic_cast<CGameObject*>(m_pGameInstance_Proxy->Clone_Prototype(PROTOTYPE::GAMEOBJECT, iPrototypeLevelIndex, strPrototypeTag, pArg));
	if (nullptr == pGameObject)
		return E_FAIL;

	pGameObject->m_iLevelIndex = iLayerLevelIndex;
	pGameObject->m_strLayerTag = strLayerTag;
	pGameObject->m_strObjectTag = strObjectTag;

	CLayer* pLayer = Find_Layer(iLayerLevelIndex, strLayerTag);
	if (nullptr == pLayer)
	{
		pLayer = CLayer::Create();
		pLayer->Add_GameObject(pGameObject);
		m_pLayers[iLayerLevelIndex][strLayerTag] = pLayer;
	}
	else
		pLayer->Add_GameObject(pGameObject);

	return S_OK;
}

HRESULT CObject_Manager::Add_GameObject_Return(CGameObject** ppOut, _uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLayerLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag, void* pArg)
{
	if (iLayerLevelIndex >= m_iNumLevels ||
		nullptr == m_pLayers)
		return E_FAIL;

	/* 레이어에 추가해야할 사본을 준비했다. */
	CGameObject* pGameObject = dynamic_cast<CGameObject*>(m_pGameInstance_Proxy->Clone_Prototype(PROTOTYPE::GAMEOBJECT, iPrototypeLevelIndex, strPrototypeTag, pArg));
	if (nullptr == pGameObject)
		return E_FAIL;

	pGameObject->m_iLevelIndex = iLayerLevelIndex;
	pGameObject->m_strLayerTag = strLayerTag;
	pGameObject->m_strObjectTag = strObjectTag;

	CLayer* pLayer = Find_Layer(iLayerLevelIndex, strLayerTag);
	if (nullptr == pLayer)
	{
		pLayer = CLayer::Create();
		pLayer->Add_GameObject(pGameObject);
		m_pLayers[iLayerLevelIndex][strLayerTag] = pLayer;
	}
	else
		pLayer->Add_GameObject(pGameObject);

	*ppOut = pGameObject;
	return S_OK;
}

void CObject_Manager::Priority_Update(_float fTimeDelta)
{
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pLayers[i])
			Pair.second->Priority_Update(fTimeDelta);
	}

}

void CObject_Manager::Update(_float fTimeDelta)
{
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pLayers[i])
			Pair.second->Update(fTimeDelta);
	}
}

void CObject_Manager::Late_Update(_float fTimeDelta)
{
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pLayers[i])
			Pair.second->Late_Update(fTimeDelta);
	}
}

void CObject_Manager::Clear(_uint iLevelIndex)
{
	for (auto& Pair : m_pLayers[iLevelIndex])
		Safe_Release(Pair.second);

	m_pLayers[iLevelIndex].clear();
}

CGameObject* CObject_Manager::Find_GameObject(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag)
{
	CLayer* pLayer = Find_Layer(iLevelIndex, strLayerTag);
	if (nullptr == pLayer) return nullptr;
	return pLayer->Find_GameObject(strObjectTag);
}

void CObject_Manager::Request_Destroy(CGameObject* pGameObject)
{
	if (nullptr == pGameObject)
		return;

	if (pGameObject->Is_Dead())
		return;

	pGameObject->Set_Dead();
	m_PendingDestroyObjects.push_back(pGameObject);
}

void CObject_Manager::Flush_DeadObjects()
{
	for (auto& pObj : m_PendingDestroyObjects)
	{
		if (pObj == nullptr)
			continue;

		auto iter = m_pLayers[pObj->Get_LevelIndex()].find(pObj->Get_LayerTag());
		if (iter != m_pLayers[pObj->Get_LevelIndex()].end())
			iter->second->Remove_GameObject(pObj);
	}
	m_PendingDestroyObjects.clear();
}

CLayer* CObject_Manager::Find_Layer(_uint iLayerLevelIndex, const _wstring& strLayerTag)
{
	auto	iter = m_pLayers[iLayerLevelIndex].find(strLayerTag);
	if (iter == m_pLayers[iLayerLevelIndex].end())
		return nullptr;

	return iter->second;
}

CObject_Manager* CObject_Manager::Create(_uint iNumLevels)
{
	CObject_Manager* pInstance = new CObject_Manager();

	if (FAILED(pInstance->Initialize(iNumLevels)))
	{
		MSG_BOX("Failed to Created : CObject_Manager");
		Safe_Release(pInstance);
	}

	return pInstance;

}

void CObject_Manager::Free()
{
	__super::Free();

	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pLayers[i])
			Safe_Release(Pair.second);

		m_pLayers[i].clear();
	}

	Safe_Release(m_pGameInstance_Proxy);
	Safe_Delete_Array(m_pLayers);
}
