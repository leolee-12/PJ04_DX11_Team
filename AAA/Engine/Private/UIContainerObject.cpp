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

    XMStoreFloat4x4(&m_TiltedWorldMatrix, XMMatrixIdentity());

    return S_OK;
}

void CUIContainerObject::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive) return;

    for (const auto& tag : m_UIPartOrder)
    {
        auto iter = m_UIPartObjects.find(tag);
        if (iter == m_UIPartObjects.end())
            continue;

        CUIPartObject* pPart = iter->second;
        if (!pPart || !pPart->Is_Active())
            continue;

        pPart->Priority_Update(fTimeDelta);
    }

}

void CUIContainerObject::Update(_float fTimeDelta)
{
    if (!m_bActive) 
        return;

    for (const auto& tag : m_UIPartOrder)
    {
        auto iter = m_UIPartObjects.find(tag);
        if (iter == m_UIPartObjects.end())
            continue;

        CUIPartObject* pPart = iter->second;
        if (!pPart || !pPart->Is_Active())
            continue;

        pPart->Update(fTimeDelta);
    }
}

void CUIContainerObject::Late_Update(_float fTimeDelta)
{
    if (!m_bActive) 
        return;

    Update_TiltedWorldMatrix();

    for (const auto& tag : m_UIPartOrder)
    {
        auto iter = m_UIPartObjects.find(tag);
        if (iter == m_UIPartObjects.end())
            continue;

        CUIPartObject* pPart = iter->second;
        if (!pPart || !pPart->Is_Active())
            continue;

        pPart->Late_Update(fTimeDelta);
    }
}

HRESULT CUIContainerObject::Render()
{
    return S_OK;
}

void CUIContainerObject::Get_UIPartObjectsInOrder(vector<pair<_wstring, CUIPartObject*>>* pOutParts) const
{
    if (!pOutParts)
        return;

    pOutParts->clear();
    pOutParts->reserve(m_UIPartObjects.size());

    unordered_set<_wstring> Visited;

    for (const _wstring& strTag : m_UIPartOrder)
    {
        auto iter = m_UIPartObjects.find(strTag);
        if (iter == m_UIPartObjects.end())
            continue;

        if (!Visited.insert(strTag).second)
            continue;

        pOutParts->emplace_back(strTag, iter->second);
    }

    for (const auto& Pair : m_UIPartObjects)
    {
        if (!Visited.insert(Pair.first).second)
            continue;

        pOutParts->emplace_back(Pair.first, Pair.second);
    }
}

_int CUIContainerObject::Get_UIPartOrderIndex(const _wstring& strPartTag) const
{
    for (_uint i = 0; i < m_UIPartOrder.size(); ++i)
    {
        if (m_UIPartOrder[i] == strPartTag)
            return static_cast<_int>(i);
    }

    return -1;
}

void CUIContainerObject::Clear_UIPartObjects()
{
    for (auto& Pair : m_UIPartObjects)
        Safe_Release(Pair.second);

    m_UIPartObjects.clear();
    m_UIPartPrototypeInfos.clear();
    m_UIPartOrder.clear();
}

json CUIContainerObject::Serialize() const
{
    json j = __super::Serialize();
    j["TiltX"] = m_fTiltXDeg;
    j["TiltY"] = m_fTiltYDeg;
    j["PerspDist"] = m_fPerspDist;
    j["UIPartObjects"] = json::object();
    j["UIPartOrder"] = json::array();

    vector<pair<_wstring, CUIPartObject*>> OrderedParts;
    Get_UIPartObjectsInOrder(&OrderedParts);

    for (auto& [tag, pPart] : OrderedParts)
    {
        auto iter = m_UIPartPrototypeInfos.find(tag);
        if (iter == m_UIPartPrototypeInfos.end())
            continue;

        json jPart = pPart->Serialize();
        jPart["ProtoTag"] = WstrToStr(iter->second.strPrototypeTag);
        jPart["ProtoLevel"] = iter->second.iPrototypeLevel;

        j["UIPartOrder"].push_back(WstrToStr(tag));
        j["UIPartObjects"][WstrToStr(tag)] = jPart;
    }

    return j;
}

void CUIContainerObject::Deserialize_Internal(const json& j)
{
    __super::Deserialize_Internal(j);

    if (j.contains("TiltX"))     m_fTiltXDeg = j["TiltX"].get<_float>();
    if (j.contains("TiltY"))     m_fTiltYDeg = j["TiltY"].get<_float>();
    if (j.contains("PerspDist")) m_fPerspDist = j["PerspDist"].get<_float>();

    Clear_UIPartObjects();

    if (!j.contains("UIPartObjects") || !j["UIPartObjects"].is_object())
        return;

    vector<string> OrderedTags;

    if (j.contains("UIPartOrder") && j["UIPartOrder"].is_array())
    {
        for (const auto& jTag : j["UIPartOrder"])
        {
            if (jTag.is_string())
                OrderedTags.push_back(jTag.get<string>());
        }
    }
    else
    {
        for (auto& [strPartTag, jPart] : j["UIPartObjects"].items())
            OrderedTags.push_back(strPartTag);
    }

    auto LoadPart = [this, &j](const string& strPartTag) -> void
        {
            auto iterJson = j["UIPartObjects"].find(strPartTag);
            if (iterJson == j["UIPartObjects"].end())
                return;

            const json& jPart = iterJson.value();

            if (!jPart.contains("ProtoTag") || !jPart.contains("ProtoLevel"))
                return;

            _wstring strPartTagW = StrToWstr(strPartTag);
            _wstring strProtoTag = StrToWstr(jPart["ProtoTag"].get<string>());
            _uint iProtoLevel = jPart["ProtoLevel"].get<_uint>();

            if (FAILED(Add_UIPartObject(iProtoLevel, strProtoTag, strPartTagW, nullptr)))
                return;

            auto iterPart = m_UIPartObjects.find(strPartTagW);
            if (iterPart != m_UIPartObjects.end())
                iterPart->second->Deserialize(jPart);
        };

    unordered_set<string> Visited;

    for (const string& strPartTag : OrderedTags)
    {
        if (!Visited.insert(strPartTag).second)
            continue;

        LoadPart(strPartTag);
    }

    for (auto& [strPartTag, jPart] : j["UIPartObjects"].items())
    {
        if (!Visited.insert(strPartTag).second)
            continue;

        LoadPart(strPartTag);
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
    m_UIPartOrder.erase(
        remove(m_UIPartOrder.begin(), m_UIPartOrder.end(), strPartTag),
        m_UIPartOrder.end());
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

    for (_wstring& strTag : m_UIPartOrder)
    {
        if (strTag == strOldTag)
        {
            strTag = strNewTag;
            break;
        }
    }

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
        pDesc->pParentMatrix = &m_TiltedWorldMatrix;
    }

    auto pUIPartObject = dynamic_cast<CUIPartObject*>(
        m_pGameInstance_Proxy->Clone_Prototype(
            PROTOTYPE::GAMEOBJECT,
            iPrototypeLevelIndex,
            strPrototypeTag,
            pArg));

    if (nullptr == pUIPartObject)
        return E_FAIL;

    pUIPartObject->Set_ParentMatrix(&m_TiltedWorldMatrix);

    m_UIPartObjects.emplace(strPartTag, pUIPartObject);
    m_UIPartPrototypeInfos.emplace(strPartTag,
        UI_PART_PROTOTYPE_INFO{ iPrototypeLevelIndex, strPrototypeTag });
    m_UIPartOrder.push_back(strPartTag);

    return S_OK;
}

void CUIContainerObject::Update_TiltedWorldMatrix()
{
    _matrix matWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

    if (m_fPerspDist <= 0.f && m_fTiltXDeg == 0.f && m_fTiltYDeg == 0.f)
    {
        XMStoreFloat4x4(&m_TiltedWorldMatrix, matWorld);
        return;
    }

    _vector vPos = matWorld.r[3];
    _matrix matLocal = matWorld;
    matLocal.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);

    _matrix matTilt = XMMatrixRotationX(XMConvertToRadians(m_fTiltXDeg))
        * XMMatrixRotationY(XMConvertToRadians(m_fTiltYDeg));

    _matrix matPersp = XMMatrixIdentity();
    if (m_fPerspDist > 0.f)
        matPersp.r[2] = XMVectorSet(0.f, 0.f, 1.f, 1.f / m_fPerspDist);  // _33=1 (ฟ๘บน), _34=1/d

    _matrix matTilted = matLocal * matTilt * matPersp
        * XMMatrixTranslationFromVector(vPos);

    matTilted.r[0] = XMVectorSetZ(matTilted.r[0], XMVectorGetW(matTilted.r[0]));
    matTilted.r[1] = XMVectorSetZ(matTilted.r[1], XMVectorGetW(matTilted.r[1]));
    matTilted.r[2] = XMVectorSetZ(matTilted.r[2], XMVectorGetW(matTilted.r[2]));
    matTilted.r[3] = XMVectorSetZ(matTilted.r[3], XMVectorGetW(matTilted.r[3]));

    XMStoreFloat4x4(&m_TiltedWorldMatrix, matTilted);
}

void CUIContainerObject::Free()
{
    __super::Free();

    for (auto& Pair : m_UIPartObjects)
        Safe_Release(Pair.second);

    m_UIPartObjects.clear();
    m_UIPartPrototypeInfos.clear();
    m_UIPartOrder.clear();
}
