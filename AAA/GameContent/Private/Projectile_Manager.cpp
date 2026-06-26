#include "Projectile_Manager.h"
#include "GameInstance.h"
#include "Projectile.h"

IMPLEMENT_SINGLETON(CProjectile_Manager)     // m_pInstance / GetInstance / DestroyInstance 정의

CProjectile_Manager::CProjectile_Manager()
    : m_pGameInstance_Proxy{ CGameInstance::GetProxy() }   // Effect_Manager 와 동일하게 ctor 에서 확보
{
    Safe_AddRef(m_pGameInstance_Proxy);
}

HRESULT CProjectile_Manager::Spawn(_uint iLevel, const _wstring& strKey,
    const _wstring& strProtoTag, CProjectile** ppOut)
{
    POOL_KEY key{ iLevel, strKey };

    auto it = m_Dormant.find(key);                        // 1) 재사용
    if (it != m_Dormant.end() && !it->second.empty())
    {
        CProjectile* p = it->second.back(); it->second.pop_back();
        if (ppOut) *ppOut = p;
        return S_OK;
    }

    _wstring strObjTag = strKey + L"#" + std::to_wstring(m_iSpawnCounter++);
    CGameObject* pObj = nullptr;                          // 2) STATIC 프로토에서 Clone
    if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(
        &pObj, m_iProtoLevel, strProtoTag,
        iLevel, PROJECTILE_LAYER_TAG, strObjTag, nullptr)))
        return E_FAIL;

    CProjectile* pProj = dynamic_cast<CProjectile*>(pObj);
    if (!pProj) { m_pGameInstance_Proxy->Destroy_GameObject(pObj); return E_FAIL; }

    pProj->Set_Pool(this, iLevel, strKey);
    if (ppOut) *ppOut = pProj;
    return S_OK;
}

void CProjectile_Manager::Return(_uint iLevel, const _wstring& strKey, CProjectile* p)
{
    m_Dormant[POOL_KEY{ iLevel, strKey }].push_back(p);
}

void CProjectile_Manager::Clear_Level(_uint iLevel)
{
    for (auto it = m_Dormant.begin(); it != m_Dormant.end(); )
        it = (it->first.iLevel == iLevel) ? m_Dormant.erase(it) : std::next(it);
}

void CProjectile_Manager::Free()
{
    m_Dormant.clear();                       // 휴면은 레이어가 소유, clear 만(Effect_Manager 와 동일)
    Safe_Release(m_pGameInstance_Proxy);
    __super::Free();
}