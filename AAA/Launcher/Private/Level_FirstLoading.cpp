#include "Level_FirstLoading.h"
#include "GameContent_const.h"

#include "GameInstance.h"
#include "Loader_Prototype.h"
#include "Launcher_LevelProfiles.h"
#include "DataLoader.h"
#include "UI_SpriteAnimCurtain.h"
#include "UI_Curtain.h"
#include "UI_CurtainTexture.h"
#include "Loader.h"
#include "Stage0_Step1.h"


CLevel_FirstLoading::CLevel_FirstLoading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel(pDevice, pContext)
{
}

HRESULT CLevel_FirstLoading::Initialize(LEVEL eNextLevelID)
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    if (FAILED(Ready_Proto()))
        return E_FAIL;

    if (FAILED(Ready_MyResources()))
        return E_FAIL;

    m_eNextLevelID = eNextLevelID;

    m_pLoader = CLoader::Create(m_pDevice, m_pContext, m_eNextLevelID, false);
    if (nullptr == m_pLoader)
        return E_FAIL;

    return S_OK;
}

void CLevel_FirstLoading::Update(_float fTimeDelta)
{
    _float fRatio = m_pLoader->Get_Progress();

    if (true == m_pLoader->isFinished())
    {
        CLevel* pNextLevel = { nullptr };

        switch (m_eNextLevelID)
        {
            case LEVEL::STAGE0_STEP1:
                pNextLevel = CStage0_Step1::Create(m_pDevice, m_pContext);
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

HRESULT CLevel_FirstLoading::Render()
{
#ifdef _DEBUG
    //m_pLoader->Show();
#endif
    return S_OK;
}

HRESULT CLevel_FirstLoading::Ready_Proto()
{
    const _uint iLevel = ETOUI(LEVEL::STATIC);

    if (FAILED(m_pGameInstance_Proxy->Add_Prototype(VI_Rect.iLevelID, VI_Rect.szProtoTag,
        CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Add_Prototype(Shader_UI.iLevelID, Shader_UI.szProtoTag,
        CShader::Create(m_pDevice, m_pContext, Shader_UI.szFileTag, VTXTEX::Elements, VTXTEX::iNumElements))))
        return E_FAIL;

    if (!m_pGameInstance_Proxy->Has_Prototype(iLevel, CUI_SpriteAnimCurtain::PROTOTYPE_TAG))
    {
        if (FAILED(m_pGameInstance_Proxy->Add_Prototype(iLevel, CUI_SpriteAnimCurtain::PROTOTYPE_TAG,
            CUI_SpriteAnimCurtain::Create(m_pDevice, m_pContext))))
            return E_FAIL;
    }

    if (!m_pGameInstance_Proxy->Has_Prototype(iLevel, CUI_Curtain::PROTOTYPE_TAG))
    {
        if (FAILED(m_pGameInstance_Proxy->Add_Prototype(iLevel, CUI_Curtain::PROTOTYPE_TAG, CUI_Curtain::Create(m_pDevice, m_pContext))))
            return E_FAIL;
    }

    if (!m_pGameInstance_Proxy->Has_Prototype(iLevel, CUI_CurtainTexture::PROTOTYPE_TAG))
    {
        if (FAILED(m_pGameInstance_Proxy->Add_Prototype(iLevel, CUI_CurtainTexture::PROTOTYPE_TAG, CUI_CurtainTexture::Create(m_pDevice, m_pContext))))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CLevel_FirstLoading::Ready_MyResources()
{
    LEVEL_MANIFEST Manifest{};
    if (FAILED(Load_LevelManifest(LAUNCHER_LEVEL_PROFILES::LEVEL_FIRSTLOADING, &Manifest)))
        return E_FAIL;

    LEVEL eLevel = LEVEL::FIRST_LOADING;

    if (!Manifest.strUIFile.empty())
    {
        wstring strUIFile = Manifest.strUIFile;
        if (FAILED(Ready_Level_UIResources(m_pGameInstance_Proxy, m_pDevice, m_pContext, strUIFile.c_str(), ETOUI(eLevel))))
            return E_FAIL;

        if (FAILED(Load_Level_UI(m_pGameInstance_Proxy, m_pDevice, m_pContext,
            Manifest.strUIFile.c_str(), ETOUI(eLevel))))
            return E_FAIL;
    }

    return S_OK;
}

CLevel_FirstLoading* CLevel_FirstLoading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID)
{
    CLevel_FirstLoading* pInstance = new CLevel_FirstLoading(pDevice, pContext);

    if (FAILED(pInstance->Initialize(eNextLevelID)))
    {
        MSG_BOX("Failed to Created : CLevel_FirstLoading");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_FirstLoading::Free()
{
    Safe_Release(m_pLoader);
    __super::Free();
}
