#include "Prototype_Manager.h"
#include "GameObject.h"
#include "Component.h"


CPrototype_Manager::CPrototype_Manager()
{
}

HRESULT CPrototype_Manager::Initialize(_uint iNumLevels)
{
	if (nullptr != m_pPrototypes)
		return E_FAIL;

	m_iNumLevels = iNumLevels;

	m_pPrototypes = new PROTOTYPES[iNumLevels];

	return S_OK;
}


HRESULT CPrototype_Manager::Add_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag, CBase* pPrototype)
{
	if (nullptr == m_pPrototypes) 
	{
		MSG_BOX("Not Initialized map_Prototypes");
		return E_FAIL;
	}

	if (iLevelIndex >= m_iNumLevels)
	{
		MSG_BOX("Index out of range");
		return E_FAIL;
	}

	unique_lock<shared_mutex> lock(m_Mutex);
	auto [iter, inserted] = m_pPrototypes[iLevelIndex].try_emplace(strPrototypeTag, pPrototype);
	lock.unlock();

	if (!inserted)
	{
		Safe_Release(pPrototype);
		return S_OK;
	}

	return S_OK;
}

CBase* CPrototype_Manager::Clone_Prototype(PROTOTYPE eType, _uint iLevelIndex, const _wstring& strPrototypeTag, void* pArg)
{
	if (iLevelIndex >= m_iNumLevels)
	{
		MSG_BOX("Index out of range");
		return nullptr;
	}

	CBase* pPrototype = Find_Prototype(iLevelIndex, strPrototypeTag);
	if (nullptr == pPrototype)
		return nullptr;

	CBase* pInstance = { nullptr };

	if (PROTOTYPE::GAMEOBJECT == eType)
		pInstance = dynamic_cast<CGameObject*>(pPrototype)->Clone(pArg);
	else
		pInstance = dynamic_cast<CComponent*>(pPrototype)->Clone(pArg);

	Safe_Release(pPrototype);

	if (nullptr == pInstance)
		return nullptr;

	return pInstance;
}

void CPrototype_Manager::Clear(_uint iLevelIndex)
{
	unique_lock<shared_mutex> lock(m_Mutex);
	for (auto& Pair : m_pPrototypes[iLevelIndex])
		Safe_Release(Pair.second);

	m_pPrototypes[iLevelIndex].clear();
	lock.unlock();
}

_bool CPrototype_Manager::Has_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag)
{
	CBase* pProto = Find_Prototype(iLevelIndex, strPrototypeTag);
	_bool bResult = pProto != nullptr;
	Safe_Release(pProto);
	return bResult;
}

CBase* CPrototype_Manager::Find_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag)
{
	shared_lock<shared_mutex> lock(m_Mutex);
	auto	iter = m_pPrototypes[iLevelIndex].find(strPrototypeTag);

	if (iter == m_pPrototypes[iLevelIndex].end())
		return nullptr;

	Safe_AddRef(iter->second);
	return iter->second;
}

CPrototype_Manager* CPrototype_Manager::Create(_uint iNumLevels)
{
	CPrototype_Manager* pInstance = new CPrototype_Manager();

	if (FAILED(pInstance->Initialize(iNumLevels)))
	{
		MSG_BOX("Failed to Created : CPrototype_Manager");
		Safe_Release(pInstance);
	}

	return pInstance;

}

void CPrototype_Manager::Free()
{
	__super::Free();

	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pPrototypes[i])
			Safe_Release(Pair.second);
		m_pPrototypes[i].clear();
	}

	Safe_Delete_Array(m_pPrototypes);
}
