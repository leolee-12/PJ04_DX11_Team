#include "GameObject.h"
#include "GameInstance.h"

CGameObject::CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance_Proxy { CGameInstance::GetProxy() }
    , m_eProjType{ PROJ_TYPE::PERSPEC }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

CGameObject::CGameObject(const CGameObject& Prototype)
    : m_pDevice{ Prototype.m_pDevice }
    , m_pContext{ Prototype.m_pContext }
    , m_pGameInstance_Proxy{ CGameInstance::GetProxy() }
    , m_eProjType{ Prototype.m_eProjType }
    , m_bActive{ Prototype.m_bActive }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

HRESULT CGameObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CGameObject::Initialize(void* pArg)
{
    if (nullptr != pArg)
    {
        auto        pDesc = static_cast<GAMEOBJECT_DESC*>(pArg);
        m_iFlag = pDesc->iFlag;
    }

    /* 객체당 부여되어야할 트랜스폼 컴포넌트를 생성한다. */
    //m_pTransformCom = CTransform::Create(m_pDevice, m_pContext);
	m_pTransformCom = Add_Component<CTransform>(L"Com_Transform", CTransform::Create(m_pDevice, m_pContext));
    if (nullptr == m_pTransformCom)
        return E_FAIL;

    /* 객체에게 부여된 초기 월드 상태를 트래스폼에게 동기화시킨다.  */
    if (FAILED(m_pTransformCom->Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Events()))
        return E_FAIL;

    return S_OK;
}

void CGameObject::Priority_Update(_float fTimeDelta)
{

}

void CGameObject::Update(_float fTimeDelta)
{

}

void CGameObject::Late_Update(_float fTimeDelta)
{

}

HRESULT CGameObject::Render()
{
    return S_OK;
}

void CGameObject::Copy_ObjectData(ENGINE_OBJECT_DATA* pOutData)
{
	if (pOutData == nullptr)
		return;

    Copy_PrototypeName(pOutData);

	if (m_pTransformCom)
	{
		XMStoreFloat4(&pOutData->Desc.vLook, m_pTransformCom->Get_State(STATE::LOOK));
		XMStoreFloat4(&pOutData->Desc.vRight, m_pTransformCom->Get_State(STATE::RIGHT));
		XMStoreFloat4(&pOutData->Desc.vUp, m_pTransformCom->Get_State(STATE::UP));
		XMStoreFloat4(&pOutData->Desc.vPosition, m_pTransformCom->Get_State(STATE::POSITION));
	}
	return;
}

json CGameObject::Serialize() const
{
    json j = IReflectable::Serialize();

    const _float4x4* pWorld = m_pTransformCom->Get_WorldMatrixPtr();
    j["Transform"]["vRight"] = { pWorld->m[0][0], pWorld->m[0][1], pWorld->m[0][2], pWorld->m[0][3] };
    j["Transform"]["vUp"] = { pWorld->m[1][0], pWorld->m[1][1], pWorld->m[1][2], pWorld->m[1][3] };
    j["Transform"]["vLook"] = { pWorld->m[2][0], pWorld->m[2][1], pWorld->m[2][2], pWorld->m[2][3] };
    j["Transform"]["vPosition"] = { pWorld->m[3][0], pWorld->m[3][1], pWorld->m[3][2], pWorld->m[3][3] };

    return j;
}

void CGameObject::Deserialize(const json& j)
{
    IReflectable::Deserialize(j);

    if (j.contains("Transform"))
    {
        _float4x4 matWorld = {};
        auto& jT = j["Transform"];

        for (int i = 0; i < 4; ++i)
            matWorld.m[0][i] = jT["vRight"][i].get<float>();

        for (int i = 0; i < 4; ++i)
            matWorld.m[1][i] = jT["vUp"][i].get<float>();

        for (int i = 0; i < 4; ++i)
            matWorld.m[2][i] = jT["vLook"][i].get<float>();

        for (int i = 0; i < 4; ++i)
            matWorld.m[3][i] = jT["vPosition"][i].get<float>();

        m_pTransformCom->Set_WorldMatrix(matWorld);
    }

    Initialize_NaviPlacement();
}


HRESULT CGameObject::Add_Component_Internal(const wstring& strComTag, CComponent* pComponent)
{
    if (pComponent == nullptr)
        return E_FAIL;
    auto [iter, inserted] = m_Components.try_emplace(strComTag, pComponent);
    return inserted ? S_OK : E_FAIL;
}

CComponent* CGameObject::Add_Component_Internal(_uint iProtoLevel, const wstring& strPrototypeTag, const wstring& strComTag, void* pArg)
{
    CComponent* pComponent = static_cast<CComponent*>(m_pGameInstance_Proxy->Clone_Prototype(PROTOTYPE::COMPONENT, iProtoLevel, strPrototypeTag, pArg));

    if (pComponent == nullptr)
        return nullptr;

    auto [iter, inserted] = m_Components.try_emplace(strComTag, pComponent);

    return inserted ? pComponent : nullptr;
}

void CGameObject::Subscribe_Event(const wstring& strEventTag, function<void(void*)> Handler)
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

void CGameObject::UnSubscribe_Event(const wstring& strEventTag)
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

void CGameObject::Free()
{
    __super::Free();
    for (auto& Pair : m_Subhandles)
        m_pGameInstance_Proxy->UnSubscribe(Pair.second);
    m_Subhandles.clear();

    for (auto& Pair : m_Components)
		Safe_Release(Pair.second);

	m_Components.clear();

    Safe_Release(m_pGameInstance_Proxy);
    Safe_Release(m_pContext);
    Safe_Release(m_pDevice);
}
