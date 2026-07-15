#include "Bubble_Manager.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "Ability_Bubble.h"

#include "EssenceBubble.h"
#include "DroppedBubble.h"

#include "GameObject_Factory.h"

IMPLEMENT_SINGLETON(CBubble_Manager)

CBubble_Manager::CBubble_Manager()
    : m_pGameInstance_Proxy{ CGameInstance::GetProxy() }
{
}

const _tchar* CBubble_Manager::Proto_Of(BUBBLE_KIND eKind) const
{
    switch (eKind)
    {
    case BUBBLE_KIND::ESSENCE: return CEssenceBubble::PROTOTYPE_TAG;
    case BUBBLE_KIND::DROPPED: return CDroppedBubble::PROTOTYPE_TAG;
    }
    return nullptr;
}

const _tchar* CBubble_Manager::Model_Of(COPY_ABILITY_TYPE eAbility) const
{
    switch (eAbility)
    {
    case COPY_ABILITY_TYPE::SWORD: return L"Prototype_Component_Ability_Model_Sword";
    case COPY_ABILITY_TYPE::BOMB:  return L"Prototype_Component_Ability_Model_Bomb";
    case COPY_ABILITY_TYPE::ICE:   return L"Prototype_Component_Ability_Model_Ice";
    }
    return nullptr;
}

_wstring CBubble_Manager::Make_Key(BUBBLE_KIND eKind, COPY_ABILITY_TYPE eAbility)
const
{
    return _wstring(Proto_Of(eKind)) + L"#"
        + std::to_wstring(static_cast<_int>(eAbility));
}

HRESULT CBubble_Manager::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    if (FAILED(Register_At_Static(CEssenceBubble::PROTOTYPE_TAG, pDevice, pContext)))
        return E_FAIL;
    if (FAILED(Register_At_Static(CDroppedBubble::PROTOTYPE_TAG, pDevice, pContext)))
        return E_FAIL;

    return S_OK;
}

HRESULT CBubble_Manager::Register_At_Static(const _tchar* szProtoTag, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    const _uint iStatic = ETOUI(LEVEL::STATIC);
    if (m_pGameInstance_Proxy->Has_Prototype(iStatic, szProtoTag))
        return S_OK;

    auto* pReg = CGameObject_Factory::GetInstance()->Get_Registration(szProtoTag);
    if (nullptr == pReg)
        return E_FAIL;

    CGameObject_Factory::GetInstance()->LoadResource(
        szProtoTag, m_pGameInstance_Proxy, pDevice, pContext, iStatic);

    return m_pGameInstance_Proxy->Add_Prototype(iStatic, szProtoTag,
        pReg->CreatorFunc(pDevice, pContext));

    return S_OK;
}

HRESULT CBubble_Manager::Spawn(_uint iTargetLevel, BUBBLE_KIND eKind,  COPY_ABILITY_TYPE eAbility, const _float3& vPos, CAbility_Bubble** ppOut)
{
    return Spawn(iTargetLevel, eKind, eAbility, vPos, _float3(0.f, 0.f, 0.f), ppOut);
}

HRESULT CBubble_Manager::Spawn(_uint iTargetLevel, BUBBLE_KIND eKind, COPY_ABILITY_TYPE eAbility, _fvector vPos, CAbility_Bubble** ppOut)
{
    _float3 vPosition{};
    XMStoreFloat3(&vPosition, vPos);

    return Spawn(iTargetLevel, eKind, eAbility, vPosition, ppOut);
}

HRESULT CBubble_Manager::Spawn(_uint iTargetLevel, BUBBLE_KIND eKind, COPY_ABILITY_TYPE eAbility, const _float3& vPos, const _float3& vDir, CAbility_Bubble** ppOut)
{
    if (nullptr == Model_Of(eAbility))
    {
#ifdef _DEBUG
        OutputDebugStringA("[Bubble_Manager] Ability Bubble Spawn Failed : UnSupported Ability\n"); 
#endif
        return E_FAIL;
    }

    const _wstring strKey = Make_Key(eKind, eAbility);

    auto it = m_Dormant.find(POOL_KEY{ iTargetLevel, strKey });
    if (it != m_Dormant.end() && !it->second.empty())
    {
        CAbility_Bubble* p = it->second.back();
        it->second.pop_back();
        p->Activate(vPos);
        p->Launch(vDir);
        if (ppOut) *ppOut = p;
        return S_OK;
    }

    CAbility_Bubble::ABILITY_BUBBLE_DESC desc{};
    desc.eAbility = eAbility;
    desc.szModelProtoTag = Model_Of(eAbility);
    desc.vPosition = _float4(vPos.x, vPos.y, vPos.z, 1.f);

    const _wstring strTag = strKey + L"#" + std::to_wstring(m_iSpawnCounter++);

    CGameObject* pObj = nullptr;
    if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(&pObj, ETOUI(LEVEL::STATIC), Proto_Of(eKind), iTargetLevel, BUBBLE_LAYER_TAG, strTag, &desc)))
        return E_FAIL;

    CAbility_Bubble* pBubble = dynamic_cast<CAbility_Bubble*>(pObj);
    if (nullptr == pBubble)
    {
        m_pGameInstance_Proxy->Destroy_GameObject(pObj);
        return E_FAIL;
    }

    pBubble->Set_Pool(this, iTargetLevel, strKey);
    pBubble->Activate(vPos);
    pBubble->Launch(vDir);
    if (ppOut) *ppOut = pBubble;
    return S_OK;
}

HRESULT CBubble_Manager::Spawn(_uint iTargetLevel, BUBBLE_KIND eKind, COPY_ABILITY_TYPE eAbility, _fvector vPos, _fvector vDir, CAbility_Bubble** ppOut)
{
    _float3 vPosition{};
    _float3 vDirection{};
    XMStoreFloat3(&vPosition, vPos);
    XMStoreFloat3(&vDirection, vDir);

    return Spawn(iTargetLevel, eKind, eAbility, vPosition, vDirection, ppOut);
}

void CBubble_Manager::Return(_uint iLevel, const _wstring& strKey, CAbility_Bubble* p)
{
    m_Dormant[POOL_KEY{ iLevel, strKey }].push_back(p);
}

void CBubble_Manager::Clear_Level(_uint iLevel)
{
    for (auto it = m_Dormant.begin(); it != m_Dormant.end(); )
        it = (it->first.iLevel == iLevel) ? m_Dormant.erase(it) : std::next(it);
}

void CBubble_Manager::Free()
{
    m_Dormant.clear();
    Safe_Release(m_pGameInstance_Proxy);
    __super::Free();
}