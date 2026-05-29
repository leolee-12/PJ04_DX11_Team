#include "Level_Loading.h"
#include "Loader.h"

#include "GameInstance.h"
#include "GameInstance_Proxy.h"
#include "GameObject_Factory.h"
#include "GameObject.h"
#include "Level_Logo.h"
#include "Level_GamePlay.h"
#include "DataLoader.h"
#include "Loader_Prototype.h"
#include "Level_Lobby.h"

CLevel_Loading::CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_Loading::Initialize(LEVEL eNextLevelID)
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_eNextLevelID = eNextLevelID;

    /*if (FAILED(Load_Level(m_pGameInstance_Proxy, m_pDevice, m_pContext, 
        L"../../Resources/LevelData/Loading.JSON", ETOUI(LEVEL::LOADING))))
        return E_FAIL;*/

    m_pLoader = CLoader::Create(m_pDevice, m_pContext, eNextLevelID);
    if (nullptr == m_pLoader)
        return E_FAIL;

    m_pGameInstance_Proxy->Stop_Sound(ETOUI(SOUND_CHANNEL::BGM));
    m_pGameInstance_Proxy->Clear_Lights();

    return S_OK;
}

void CLevel_Loading::Update(_float fTimeDelta)
{
    _float fRatio = m_pLoader->Get_Progress();
    m_pGameInstance_Proxy->Publish(TEXT("Loading_Progress"), &fRatio);

    if (true == m_pLoader->isFinished())
    {
        CLevel* pNextLevel = { nullptr };

        switch (m_eNextLevelID)
        {
        case LEVEL::LOGO:
            pNextLevel = CLevel_Logo::Create(m_pDevice, m_pContext);
            break;
        case LEVEL::LOBBY:
            pNextLevel = CLevel_Lobby::Create(m_pDevice, m_pContext);
            break;
        case LEVEL::GAMEPLAY:
            pNextLevel = CLevel_GamePlay::Create(m_pDevice, m_pContext);
            break;
        }

        if (nullptr == pNextLevel)
        {
            MSG_BOX("Failed to Changed");
            return;
        }

        if (SUCCEEDED(m_pGameInstance_Proxy->Change_Level(ETOI(m_eNextLevelID), pNextLevel)))
            return;
    }
}

HRESULT CLevel_Loading::Render()
{
#ifdef _DEBUG
    m_pLoader->Show();
#endif
    return S_OK;
}

HRESULT CLevel_Loading::Load_LoadingLevel()
{
    string strContent = {};
    if (FAILED(CDataLoader::Read_Json(L"../../Resources/LevelData/Loading.JSON", &strContent)))
        return E_FAIL;

    json jLevel = json::parse(strContent);

    for (auto& jObj : jLevel["Objects"])
    {
        wstring wProto = StrToWstr(jObj["Prototype_Tag"].get<string>());
        wstring wLayer = StrToWstr(jObj["Layer_Tag"].get<string>());

        auto* pReg = CGameObject_Factory::GetInstance()->Get_Registration(wProto);
        if (!pReg) continue;

        if (!m_pGameInstance_Proxy->Has_Prototype(ETOUI(LEVEL::LOADING), wProto))
        {
            pReg->ResourceLoader(m_pGameInstance_Proxy, m_pDevice, m_pContext);
            m_pGameInstance_Proxy->Add_Prototype(ETOUI(LEVEL::LOADING), wProto.c_str(),
                pReg->CreatorFunc(m_pDevice, m_pContext));
        }

        wstring wObjectName = StrToWstr(jObj["Object_Tag"].get<string>());

        CGameObject* pObj = nullptr;
        m_pGameInstance_Proxy->Add_GameObject_Return(
            &pObj,
            ETOUI(LEVEL::LOADING), wProto.c_str(),
            ETOUI(LEVEL::LOADING), wLayer.c_str(), wObjectName, nullptr);

        if (pObj)
            pObj->Deserialize(jObj);
    }
    return S_OK;
}

CLevel_Loading* CLevel_Loading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID)
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
