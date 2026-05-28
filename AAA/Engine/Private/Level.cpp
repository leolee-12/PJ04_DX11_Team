#include "Level.h"
#include "GameInstance.h"

CLevel::CLevel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice { pDevice }
    , m_pContext { pContext }
    , m_pGameInstance_Proxy{ CGameInstance::GetProxy()}
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

HRESULT CLevel::Initialize()
{
    if (FAILED(Ready_Events()))
        return E_FAIL;

    return S_OK;
}

void CLevel::Update(_float fTimeDelta)
{
}

HRESULT CLevel::Render()
{
    return S_OK;
}

void CLevel::Subscribe_Event(const wstring& strEventTag, function<void(void*)> Handler)
{
    SUBHANDLE handle = m_pGameInstance_Proxy->Subscribe(strEventTag, Handler);
    auto [iter, inserted] = m_Subhandles.try_emplace(strEventTag, handle);

    if (inserted == false)
    {
#ifdef _DEBUG
        MSG_BOX("SubHandle Insert Fail");
#endif // _DEBUG
        m_pGameInstance_Proxy->UnSubscribe(handle);
    }
}

void CLevel::UnSubscribe_Event(const wstring& strEventTag)
{
    auto iter = m_Subhandles.find(strEventTag);
    if (iter == m_Subhandles.end())
    {
#ifdef _DEBUG
        MSG_BOX("UnSubscribe Fail : Not Found HandleTag");
#endif // _DEBUG
        return;
    }

    m_pGameInstance_Proxy->UnSubscribe(iter->second);
    m_Subhandles.erase(iter);
}

void CLevel::Free()
{
    __super::Free();
    for (auto& Pair : m_Subhandles)
        m_pGameInstance_Proxy->UnSubscribe(Pair.second);
    m_Subhandles.clear();

    Safe_Release(m_pGameInstance_Proxy);
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
}
