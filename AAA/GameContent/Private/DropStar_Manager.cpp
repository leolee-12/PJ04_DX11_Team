#include "DropStar_Manager.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "DropStar.h"
#include "GameObject_Factory.h"

IMPLEMENT_SINGLETON(CDropStar_Manager)

CDropStar_Manager::CDropStar_Manager()
	: m_pGameInstance_Proxy { CGameInstance::GetProxy() }
{
}

HRESULT CDropStar_Manager::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	if (FAILED(Register_At_Static(CDropStar::PROTOTYPE_TAG, pDevice, pContext)))
		return E_FAIL;

	return S_OK;
}

HRESULT CDropStar_Manager::Register_At_Static(const _tchar* szProtoTag, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	const _uint iStatic = ETOUI(LEVEL::STATIC);
	if (m_pGameInstance_Proxy->Has_Prototype(iStatic, szProtoTag))
		return S_OK;

	auto* pReg = CGameObject_Factory::GetInstance()->Get_Registration(szProtoTag);
	if (nullptr == pReg)
		return E_FAIL;

	CGameObject_Factory::GetInstance()->LoadResource(szProtoTag, m_pGameInstance_Proxy, pDevice, pContext, iStatic);

	return m_pGameInstance_Proxy->Add_Prototype(iStatic, szProtoTag, pReg->CreatorFunc(pDevice, pContext));
}

HRESULT CDropStar_Manager::Spawn(_uint iTargetLevel, _fvector vPos, _float fDelay, CDropStar** ppOut)
{
    const _wstring strKey = CDropStar::PROTOTYPE_TAG;

    _float3 vPosition{};
    XMStoreFloat3(&vPosition, vPos);

    auto it = m_Dormant.find(POOL_KEY{ iTargetLevel, strKey });
    if (it != m_Dormant.end() && !it->second.empty())
    {
        CDropStar* p = it->second.back();
        it->second.pop_back();
        p->Activate(vPosition, fDelay);
        if (ppOut) *ppOut = p;
        return S_OK;
    }

    const _wstring strObjTag =
        strKey + L"#" + std::to_wstring(m_iSpawnCounter++);

    CGameObject* pObj = nullptr;
    if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(
        &pObj, ETOUI(LEVEL::STATIC), CDropStar::PROTOTYPE_TAG,
        iTargetLevel, DROPSTAR_LAYER_TAG, strObjTag, nullptr)))
        return E_FAIL;

    CDropStar* pStar = dynamic_cast<CDropStar*>(pObj);
    if (!pStar)
    {
        m_pGameInstance_Proxy->Destroy_GameObject(pObj);
        return E_FAIL;
    }

    pStar->Set_Pool(this, iTargetLevel, strKey);
    pStar->Activate(vPosition, fDelay);
    if (ppOut) 
        *ppOut = pStar;

    return S_OK;
}

void CDropStar_Manager::Return(_uint iLevel, const _wstring& strKey, CDropStar* pStar)
{
	m_Dormant[POOL_KEY{ iLevel, strKey }].push_back(pStar);
}

void CDropStar_Manager::Clear_Level(_uint iLevel)
{
	for (auto it = m_Dormant.begin(); it != m_Dormant.end(); )
		it = (it->first.iLevel == iLevel) ? m_Dormant.erase(it)  : std::next(it);
}

void CDropStar_Manager::Free()
{
	m_Dormant.clear();
	Safe_Release(m_pGameInstance_Proxy);
	__super::Free();
}
