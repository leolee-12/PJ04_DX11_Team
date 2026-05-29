#include "Level_Logo.h"

#include "GameInstance.h"
#include "GameInstance_Proxy.h"
#include "Level_Loading.h"
#include "Level_GamePlay.h"
#include "Loader_Prototype.h"

CLevel_Logo::CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel (pDevice, pContext)
{
}

HRESULT CLevel_Logo::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    /*if (FAILED(Load_Level(m_pGameInstance_Proxy, m_pDevice, m_pContext,
        L"../../Resources/LevelData/Title.JSON", ETOUI(LEVEL::LOGO))))
        return E_FAIL;*/

    return S_OK;
}

void CLevel_Logo::Update(_float fTimeDelta)
{
}

HRESULT CLevel_Logo::Render()
{
#ifdef _DEBUG

#endif
    return S_OK;
}

CLevel_Logo* CLevel_Logo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Logo* pInstance = new CLevel_Logo(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CLevel_Logo");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Logo::Free()
{
    __super::Free();
}
