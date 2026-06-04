#include "Level_Loading.h"
#include "Loader.h"
#include "Level_Edit.h"
#include "EditInstance.h"

#include "GameInstance.h"
#include "GameInstance_Proxy.h"

CLevel_Loading::CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_Loading::Initialize(TOOL_LEVEL eNextLevelID)
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_eNextLevelID = eNextLevelID;

    m_pLoader = CLoader::Create(m_pDevice, m_pContext, eNextLevelID);
    if (nullptr == m_pLoader)
        return E_FAIL;

    CEditInstance::GetInstance()->Set_Loading(true);

    return S_OK;
}

void CLevel_Loading::Update(_float fTimeDelta)
{
    CEditInstance* pEI = CEditInstance::GetInstance();
    pEI->Set_LoadProgress(m_pLoader->Get_Progress());

    if (false == m_pLoader->isFinished())
        return;

    CLevel* pNextLevel = { nullptr };
    switch (m_eNextLevelID)
    {
    case TOOL_LEVEL::EDIT:
        pNextLevel = CLevel_Edit::Create(m_pDevice, m_pContext);
        break;
    default:
        break;
    }

    if (nullptr == pNextLevel)
    {
        MSG_BOX("Failed to Changed");
        return;
    }

    // 공유 허브 상태를 먼저 갱신(Change_Level 이 이 로딩 레벨을 해제할 수 있으므로
    // this 를 건드리지 않고 싱글톤만 갱신한 뒤 전환한다).
    pEI->Set_Level(static_cast<CLevel_Edit*>(pNextLevel));
    pEI->Set_Loading(false);

    if (SUCCEEDED(m_pGameInstance_Proxy->Change_Level(ETOI(m_eNextLevelID), pNextLevel)))
        return;

    // 전환 실패 시 롤백
    pEI->Set_Level(nullptr);
    pEI->Set_Loading(true);
}

HRESULT CLevel_Loading::Render()
{
#ifdef _DEBUG
    m_pLoader->Show();
#endif
    return S_OK;
}

CLevel_Loading* CLevel_Loading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TOOL_LEVEL eNextLevelID)
{
    CLevel_Loading* pInstance = new CLevel_Loading(pDevice, pContext);

    if (FAILED(pInstance->Initialize(eNextLevelID)))
    {
        MSG_BOX("Failed to Created : CLevel_Loading");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Loading::Free()
{
    __super::Free();

    Safe_Release(m_pLoader);
}
