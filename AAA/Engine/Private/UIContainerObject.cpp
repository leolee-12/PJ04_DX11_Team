#include "UIContainerObject.h"
#include "GameInstance.h"

#include "UIPartObject.h"

CUIContainerObject::CUIContainerObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject(pDevice, pContext)
{
}

CUIContainerObject::CUIContainerObject(const CUIContainerObject& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CUIContainerObject::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::ORTHO;
    if (FAILED(__super::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CUIContainerObject::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void CUIContainerObject::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive) return;

    for (auto& [tag, pPart] : m_UIPartObjects)
        pPart->Priority_Update(fTimeDelta);
}

void CUIContainerObject::Update(_float fTimeDelta)
{
    if (!m_bActive) return;

    for (auto& [tag, pPart] : m_UIPartObjects)
        pPart->Update(fTimeDelta);
}

void CUIContainerObject::Late_Update(_float fTimeDelta)
{
    if (!m_bActive) return;

    for (auto& [tag, pPart] : m_UIPartObjects)
        pPart->Late_Update(fTimeDelta);
}

HRESULT CUIContainerObject::Render()
{
    return S_OK;
}

void CUIContainerObject::Clear_UIPartObjects()
{
    for (auto& Pair : m_UIPartObjects)
        Safe_Release(Pair.second);
    m_UIPartObjects.clear();
}

json CUIContainerObject::Serialize() const
{
    json j = __super::Serialize();

    for (auto& [tag, pPart] : m_UIPartObjects)
    {
        string strTag = WstrToStr(tag);
        j["UIPartObjects"][strTag] = pPart->Serialize();
    }

    return j;
}

void CUIContainerObject::Deserialize(const json& j)
{
    __super::Deserialize(j);

    if (!j.contains("UIPartObjects")) return;

    for (auto& [tag, pPart] : m_UIPartObjects)
    {
        string strTag = WstrToStr(tag);
        if (j["UIPartObjects"].contains(strTag))
            pPart->Deserialize(j["UIPartObjects"][strTag]);
    }
}

HRESULT CUIContainerObject::Add_UIPartObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strPartTag, void* pArg)
{
    auto        pUIPartObject = dynamic_cast<CUIPartObject*>(m_pGameInstance_Proxy->Clone_Prototype(PROTOTYPE::GAMEOBJECT, iPrototypeLevelIndex, strPrototypeTag, pArg));
    if (nullptr == pUIPartObject)
        return E_FAIL;

    m_UIPartObjects.emplace(strPartTag, pUIPartObject);

    return S_OK;
}

void CUIContainerObject::Free()
{
    __super::Free();

    for (auto& Pair : m_UIPartObjects)
        Safe_Release(Pair.second);

    m_UIPartObjects.clear();

}
