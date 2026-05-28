#include "Level_Manager.h"
#include "GameInstance.h"

#include "Level.h"

CLevel_Manager::CLevel_Manager()
    : m_pGameInstance_Proxy { CGameInstance::GetProxy() }
{
    /* 해당 매니저는 피소유자, 소유자는 GameInstance
        피소유자는 소유자보다 오래 살 수 없음 고로 참조카운트 올리지않음 (순환 참조 일어남 예외)*/
    //Safe_AddRef(m_pGameInstance);
}

HRESULT CLevel_Manager::Change_Level(_int iNewLevelIndex, CLevel* pNewLevel)
{
    if (nullptr == pNewLevel)
        return E_FAIL;

    /* 기존 레벨용 자원을 정리한다. */
    m_pGameInstance_Proxy->Clear_Resources(m_iCurrentLevelIndex);

    Safe_Release(m_pCurrentLevel);

    m_pCurrentLevel = pNewLevel;

    m_iCurrentLevelIndex = iNewLevelIndex;

    return S_OK;
}

void CLevel_Manager::Update(_float fTimeDelta)
{
    if (nullptr == m_pCurrentLevel)
        return;

    m_pCurrentLevel->Update(fTimeDelta);
}

HRESULT CLevel_Manager::Render()
{
    if (nullptr == m_pCurrentLevel)
        return E_FAIL;

    return m_pCurrentLevel->Render();
}

CLevel_Manager* CLevel_Manager::Create()
{
    return new CLevel_Manager;
}

void CLevel_Manager::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance_Proxy);
    Safe_Release(m_pCurrentLevel);
}
