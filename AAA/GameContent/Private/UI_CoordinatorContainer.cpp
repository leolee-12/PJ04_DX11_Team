#include "UI_CoordinatorContainer.h"
#include "GameInstance.h"

CUICoordinatorContainer::CUICoordinatorContainer(ID3D11Device* d, ID3D11DeviceContext* c)
    : CUIMovableContainer{ d, c } {
}

// 자식은 프로토 복제 대상 아님(런타임 Deserialize/Add_Child 로 구성) -> 복사 안 함
CUICoordinatorContainer::CUICoordinatorContainer(const CUICoordinatorContainer& p)
    : CUIMovableContainer(p) {
}

HRESULT CUICoordinatorContainer::Initialize_Prototype() { return __super::Initialize_Prototype(); }

HRESULT CUICoordinatorContainer::Initialize(void* pArg)
{
    CGameObject::GAMEOBJECT_DESC Default{};
    void* pDesc = pArg ? pArg : &Default;
    if (FAILED(__super::Initialize(pDesc))) return E_FAIL;
    return S_OK;
}

HRESULT CUICoordinatorContainer::Add_Child(_uint iProtoLevel, const _wstring& strProtoTag, const _wstring& strChildTag)
{
    if (m_Children.find(strChildTag) != m_Children.end()) return E_FAIL;

    auto pChild = dynamic_cast<CUIMovableContainer*>(
        m_pGameInstance_Proxy->Clone_Prototype(PROTOTYPE::GAMEOBJECT, iProtoLevel, strProtoTag, nullptr));
    if (!pChild) return E_FAIL;

    pChild->Set_GroupParentMatrix(Get_TiltedParentMatrix());   // 그룹행렬 물려줌(통째 이동)

    m_Children.emplace(strChildTag, pChild);
    m_ChildProto.emplace(strChildTag, CHILD_PROTO{ iProtoLevel, strProtoTag });
    m_ChildOrder.push_back(strChildTag);
    return S_OK;
}

CUIMovableContainer* CUICoordinatorContainer::Find_Child(const _wstring& strChildTag) const
{
    auto it = m_Children.find(strChildTag);
    return it != m_Children.end() ? it->second : nullptr;
}

HRESULT CUICoordinatorContainer::Rename_Child(const _wstring& strOldTag, const _wstring& strNewTag)
{
    if (strOldTag.empty() || strNewTag.empty()) return E_FAIL;
    if (strOldTag == strNewTag)                  return S_OK;
    if (m_Children.find(strNewTag) != m_Children.end()) return E_FAIL;   // 중복 방지

    auto itChild = m_Children.find(strOldTag);
    auto itProto = m_ChildProto.find(strOldTag);
    if (itChild == m_Children.end() || itProto == m_ChildProto.end()) return E_FAIL;

    CUIMovableContainer* pChild = itChild->second;
    CHILD_PROTO          proto = itProto->second;

    m_Children.erase(itChild);
    m_ChildProto.erase(itProto);
    m_Children.emplace(strNewTag, pChild);
    m_ChildProto.emplace(strNewTag, proto);

    for (auto& t : m_ChildOrder)
        if (t == strOldTag) { t = strNewTag; break; }

    return S_OK;
}

HRESULT CUICoordinatorContainer::Remove_Child(const _wstring& strChildTag)
{
    auto it = m_Children.find(strChildTag);
    if (it == m_Children.end())
        return E_FAIL;

    Safe_Release(it->second);
    m_Children.erase(it);
    m_ChildProto.erase(strChildTag);
    m_ChildOrder.erase(
        remove(m_ChildOrder.begin(), m_ChildOrder.end(), strChildTag),
        m_ChildOrder.end());
    return S_OK;
}

void CUICoordinatorContainer::Priority_Update(_float dt)
{
    if (!m_bActive) return;
    __super::Priority_Update(dt);
    for (auto& t : m_ChildOrder)
        if (auto it = m_Children.find(t); it != m_Children.end() && it->second->Is_Active())
            it->second->Priority_Update(dt);
}

void CUICoordinatorContainer::Update(_float dt)
{
    if (!m_bActive) return;
    __super::Update(dt);
    for (auto& t : m_ChildOrder)
        if (auto it = m_Children.find(t); it != m_Children.end() && it->second->Is_Active())
            it->second->Update(dt);
}

void CUICoordinatorContainer::Late_Update(_float dt)
{
    if (!m_bActive) return;
    __super::Late_Update(dt);        // 자기 행렬(m_TiltedWorldMatrix) 확정 + 자기 파츠
    for (auto& t : m_ChildOrder)      // 자식은 그 뒤 -> 최신 부모행렬 읽음
        if (auto it = m_Children.find(t); it != m_Children.end() && it->second->Is_Active())
            it->second->Late_Update(dt);
}

json CUICoordinatorContainer::Serialize() const
{
    json j = __super::Serialize();
    j["Children"] = json::object();
    j["ChildOrder"] = json::array();
    for (auto& tag : m_ChildOrder)
    {
        auto it = m_Children.find(tag);
        auto pit = m_ChildProto.find(tag);
        if (it == m_Children.end() || pit == m_ChildProto.end()) continue;

        json jc = it->second->Serialize();      // 자식 = 컨테이너+파츠 완전 재귀
        jc["ProtoTag"] = WstrToStr(pit->second.strProtoTag);
        jc["ProtoLevel"] = pit->second.iLevel;
        j["ChildOrder"].push_back(WstrToStr(tag));
        j["Children"][WstrToStr(tag)] = jc;
    }
    return j;
}

void CUICoordinatorContainer::Deserialize_Internal(const json& j)
{
    __super::Deserialize_Internal(j);
    if (!j.contains("Children") || !j["Children"].is_object())
        return;

    Clear_Children();

    vector<string> order;
    if (j.contains("ChildOrder") && j["ChildOrder"].is_array())
        for (auto& t : j["ChildOrder"]) if (t.is_string()) order.push_back(t.get<string>());
        else
            for (auto& [k, v] : j["Children"].items()) order.push_back(k);

    for (auto& tagS : order)
    {
        auto itj = j["Children"].find(tagS);
        if (itj == j["Children"].end()) continue;
        const json& jc = itj.value();
        if (!jc.contains("ProtoTag") || !jc.contains("ProtoLevel")) continue;

        _wstring tag = StrToWstr(tagS);
        _wstring proto = StrToWstr(jc["ProtoTag"].get<string>());
        _uint    lv = jc["ProtoLevel"].get<_uint>();

        if (SUCCEEDED(Add_Child(lv, proto, tag)))
            m_Children[tag]->Deserialize(jc);   // 자식 재귀 복원(파츠까지)
    }
}

void CUICoordinatorContainer::Clear_Children()
{
    for (auto& p : m_Children) Safe_Release(p.second);
    m_Children.clear(); m_ChildProto.clear(); m_ChildOrder.clear();
}

CUICoordinatorContainer* CUICoordinatorContainer::Create(ID3D11Device* d, ID3D11DeviceContext* c)
{
    auto* p = new CUICoordinatorContainer(d, c);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUICoordinatorContainer"); Safe_Release(p); }
    return p;
}

CGameObject* CUICoordinatorContainer::Clone(void* pArg)
{
    auto* p = new CUICoordinatorContainer(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUICoordinatorContainer"); Safe_Release(p); }
    return p;
}

void CUICoordinatorContainer::Free()
{
    Clear_Children();
    __super::Free();
}