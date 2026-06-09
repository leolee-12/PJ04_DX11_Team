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
    {
        if (!pPart->Is_Active())
            continue;

        pPart->Priority_Update(fTimeDelta);
    }

}

void CUIContainerObject::Update(_float fTimeDelta)
{
    if (!m_bActive) 
        return;

    for (auto& [tag, pPart] : m_UIPartObjects)
    {
        if (!pPart->Is_Active())
            continue;

        pPart->Update(fTimeDelta);
    }
}

void CUIContainerObject::Late_Update(_float fTimeDelta)
{
    if (!m_bActive) 
        return;

    for (auto& [tag, pPart] : m_UIPartObjects)
    {
        if (!pPart->Is_Active())
            continue;

        pPart->Late_Update(fTimeDelta);
    }
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
    m_UIPartPrototypeInfos.clear();
}

json CUIContainerObject::Serialize() const
{
    json j = __super::Serialize();
    j["UIPartObjects"] = json::object();

    for (auto& [tag, pPart] : m_UIPartObjects)
    {
        auto iter = m_UIPartPrototypeInfos.find(tag);
        if (iter == m_UIPartPrototypeInfos.end())
            continue;

        json jPart = pPart->Serialize();
        jPart["ProtoTag"] = WstrToStr(iter->second.strPrototypeTag);
        jPart["ProtoLevel"] = iter->second.iPrototypeLevel;

        j["UIPartObjects"][WstrToStr(tag)] = jPart;
    }

    return j;
}

void CUIContainerObject::Deserialize_Internal(const json& j)
{
    __super::Deserialize_Internal(j);

    Clear_UIPartObjects();

    if (!j.contains("UIPartObjects"))
        return;

    for (auto& [strPartTag, jPart] : j["UIPartObjects"].items())
    {
        if (!jPart.contains("ProtoTag") || !jPart.contains("ProtoLevel"))
            continue;

        _wstring strPartTagW = StrToWstr(strPartTag);
        _wstring strProtoTag = StrToWstr(jPart["ProtoTag"].get<string>());
        _uint iProtoLevel = jPart["ProtoLevel"].get<_uint>();

        if (FAILED(Add_UIPartObject(iProtoLevel, strProtoTag, strPartTagW, nullptr)))
            continue;

        auto iter = m_UIPartObjects.find(strPartTagW);
        if (iter != m_UIPartObjects.end())
            iter->second->Deserialize(jPart);
    }
}
HRESULT CUIContainerObject::Add_Part(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strPartTag, void* pArg)
{
    HRESULT hr = Add_UIPartObject(iPrototypeLevelIndex, strPrototypeTag, strPartTag, pArg);

    if (SUCCEEDED(hr))
        On_UIPartsChanged();

    return hr;
}


HRESULT CUIContainerObject::Remove_Part(const _wstring& strPartTag)
{
    auto it = m_UIPartObjects.find(strPartTag);
    if (it == m_UIPartObjects.end())
        return E_FAIL;

    Safe_Release(it->second);
    m_UIPartObjects.erase(it);
    m_UIPartPrototypeInfos.erase(strPartTag);

    On_UIPartsChanged();

    return S_OK;
}

HRESULT CUIContainerObject::Rename_Part(const _wstring& strOldTag, const _wstring& strNewTag)
{
    if (strOldTag.empty() || strNewTag.empty())
        return E_FAIL;

    if (strOldTag == strNewTag)
        return S_OK;

    if (m_UIPartObjects.find(strNewTag) != m_UIPartObjects.end())
        return E_FAIL;

    auto partIt = m_UIPartObjects.find(strOldTag);
    if (partIt == m_UIPartObjects.end())
        return E_FAIL;

    auto protoIt = m_UIPartPrototypeInfos.find(strOldTag);
    if (protoIt == m_UIPartPrototypeInfos.end())
        return E_FAIL;

    CUIPartObject* pPart = partIt->second;
    UI_PART_PROTOTYPE_INFO protoInfo = protoIt->second;

    m_UIPartObjects.erase(partIt);
    m_UIPartPrototypeInfos.erase(protoIt);

    m_UIPartObjects.emplace(strNewTag, pPart);
    m_UIPartPrototypeInfos.emplace(strNewTag, protoInfo);

    On_UIPartsChanged();

    return S_OK;
}

HRESULT CUIContainerObject::Add_UIPartObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strPartTag, void* pArg)
{
    if (m_UIPartObjects.find(strPartTag) != m_UIPartObjects.end())
        return E_FAIL;

    if (pArg)
    {
        auto pDesc = static_cast<CUIPartObject::UI_PARTOBJECT_DESC*>(pArg);
        pDesc->pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    }

    auto pUIPartObject = dynamic_cast<CUIPartObject*>(
        m_pGameInstance_Proxy->Clone_Prototype(
            PROTOTYPE::GAMEOBJECT,
            iPrototypeLevelIndex,
            strPrototypeTag,
            pArg));

    if (nullptr == pUIPartObject)
        return E_FAIL;

    pUIPartObject->Set_ParentMatrix(m_pTransformCom->Get_WorldMatrixPtr());

    m_UIPartObjects.emplace(strPartTag, pUIPartObject);
    m_UIPartPrototypeInfos.emplace(strPartTag,
        UI_PART_PROTOTYPE_INFO{ iPrototypeLevelIndex, strPrototypeTag });

    return S_OK;
}

void CUIContainerObject::Free()
{
    __super::Free();

    for (auto& Pair : m_UIPartObjects)
        Safe_Release(Pair.second);

    m_UIPartObjects.clear();
    m_UIPartPrototypeInfos.clear();
}
