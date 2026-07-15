#include "Level_Test.h"

#include "Level_Defines.h"

CLevel_Test::CLevel_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel (pDevice, pContext)
{
}

HRESULT CLevel_Test::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    _uint iLevel = ETOUI(LEVEL::TEST);

    LEVEL_MANIFEST Manifest{};
    if (FAILED(Load_LevelManifest(LAUNCHER_LEVEL_PROFILES::LEVEL_TEST, &Manifest)))
        return E_FAIL;

    MAP_LOAD_RESULT MapReport{};
    CMapStage* pMapStage = nullptr;

    if (FAILED(CMap_Loader::Spawn_Map(
        m_pDevice,
        m_pContext,
        Manifest.strMapManifest,
        Manifest.strObjectsFile,
        iLevel,
        &MapReport,
        &pMapStage)))
    {
        return E_FAIL;
    }

    if (FAILED(Load_Level(m_pGameInstance_Proxy, m_pDevice, m_pContext,
        Manifest.strObjectsFile.c_str(), iLevel)))
        return E_FAIL;

    if (!Manifest.strUIFile.empty())
    {
        if (FAILED(Load_Level_UI(m_pGameInstance_Proxy, m_pDevice, m_pContext,
            Manifest.strUIFile.c_str(), iLevel)))
            return E_FAIL;
    }

    if (FAILED(Ready_Lights()))
        return E_FAIL;

    return S_OK;
}

void CLevel_Test::Update(_float fTimeDelta)
{
#ifdef  _DEBUG
    if (m_pGameInstance_Proxy->Key_Down(DIK_F2))
        m_pGameInstance_Proxy->Publish(TEXT("FadeOut_Start"), nullptr);

    if (m_bTestLevelChange)
    {
        CLevel_Loading* pLoadingLevel = CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::STAGE0_STEP1);
        if (pLoadingLevel)
        {
            m_pGameInstance_Proxy->Change_Level(ETOUI(LEVEL::LOADING), pLoadingLevel);
            return;
        }
    }
#endif //  _DEBUG
}

HRESULT CLevel_Test::Render()
{
    return S_OK;
}

HRESULT CLevel_Test::Ready_Events()
{
    m_pGameInstance_Proxy->Subscribe(TEXT("FadeOut_Done"), [this](void* p) {
        m_bTestLevelChange = true;
        });
    return S_OK;
}

HRESULT CLevel_Test::Ready_Lights()
{
    LIGHT_DESC      LightDesc{};

    LightDesc.eType = LIGHT::DIRECTIONAL;
    LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vAmbient = _float4(0.5f, 0.5f, 0.5f, 1.f);
    LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vDirection = _float4(0.25f, -1.f, 0.25f, 0.f);

    if (FAILED(m_pGameInstance_Proxy->Add_Light(LightDesc)))
        return E_FAIL;

    return S_OK;
}

CLevel_Test* CLevel_Test::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Test* pInstance = new CLevel_Test(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CLevel_Test");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Test::Free()
{
    __super::Free();
}
