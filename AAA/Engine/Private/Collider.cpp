#include "Collider.h"
#include "GameInstance.h"
#include "GameObject.h"

CCollider::CCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }
{
}

CCollider::CCollider(const CCollider& Prototype)
	: CComponent(Prototype)
    , m_eType { Prototype.m_eType }
#ifdef _DEBUG
    , m_pBatch { Prototype.m_pBatch } 
    , m_pEffect { Prototype.m_pEffect }
    , m_pInputLayout { Prototype.m_pInputLayout }
#endif
{

#ifdef _DEBUG
    Safe_AddRef(m_pInputLayout);
#endif
}

HRESULT CCollider::Initialize_Prototype(COLLIDER eType)
{
    m_eType = eType;

#ifdef _DEBUG
    m_pBatch = new PrimitiveBatch<VertexPositionColor>(m_pContext);
    m_pEffect = new BasicEffect(m_pDevice);
    m_pEffect->SetVertexColorEnabled(true);

    const void* pShaderByteCode = { nullptr };
    size_t      iShaderByteCodeLength = {};

    m_pEffect->GetVertexShaderBytecode(&pShaderByteCode, &iShaderByteCodeLength);

    if (FAILED(m_pDevice->CreateInputLayout(VertexPositionColor::InputElements, VertexPositionColor::InputElementCount, pShaderByteCode, iShaderByteCodeLength, &m_pInputLayout)))
        return E_FAIL;
#endif

  
	return S_OK;
}

HRESULT CCollider::Initialize(void* pArg)
{
    auto pColliderDesc = static_cast<COLLIDER_DESC*>(pArg);
    if (pColliderDesc == nullptr)
        return E_FAIL;

    m_pOwner = pColliderDesc->pOwner;

    switch (m_eType)
    {
    case COLLIDER::AABB:
    {
        CBounding_AABB::BOUNDING_AABB_DESC BoundingDesc{};
        BoundingDesc.vCenter = pColliderDesc->vCenter;
        BoundingDesc.vSize = pColliderDesc->vSize;
        m_pBounding = CBounding_AABB::Create(m_pDevice, m_pContext, &BoundingDesc);
        break;
    }

    case COLLIDER::OBB:
    {
        CBounding_OBB::BOUNDING_OBB_DESC BoundingDesc{};
        BoundingDesc.vCenter = pColliderDesc->vCenter;
        BoundingDesc.vSize = pColliderDesc->vSize;
        BoundingDesc.vRadians = pColliderDesc->vRadians;
        m_pBounding = CBounding_OBB::Create(m_pDevice, m_pContext, &BoundingDesc);
        break;
    }

    case COLLIDER::SPHERE:
    {
        CBounding_Sphere::BOUNDING_SPHERE_DESC BoundingDesc{};
        BoundingDesc.vCenter = pColliderDesc->vCenter;
        BoundingDesc.fRadius = pColliderDesc->fRadius;
        m_pBounding = CBounding_Sphere::Create(m_pDevice, m_pContext, &BoundingDesc);
        break;
    }

    default:
        return E_FAIL;
    }

    if (nullptr == m_pBounding)
        return E_FAIL;

    return S_OK;
}

void CCollider::Update(_fmatrix TransformMatrix)
{
    if (!m_bEnabled) return;
    m_pBounding->Update(TransformMatrix);
}

_bool CCollider::Intersect(CCollider* pTarget)
{
    m_isColl = false;
    
    m_isColl = m_pBounding->Intersect(pTarget->m_eType, pTarget->m_pBounding);

    return m_isColl;
}

void CCollider::Notify_Enter(CCollider* pOther)
{
    if (m_OnEnter) 
        m_OnEnter(pOther);
}

void CCollider::Notify_Stay(CCollider* pOther)
{
    if (m_OnStay)  
        m_OnStay(pOther);
}

void CCollider::Notify_Exit(CCollider* pOther)
{
    if (m_OnExit)  
        m_OnExit(pOther);
}

void CCollider::Swap_ContactFrame()
{
    m_PrevContacts = m_CurrContacts;
    m_CurrContacts.clear();
}

void CCollider::Add_Contact(CCollider* pOther)
{
    m_CurrContacts.insert(pOther);
}

void CCollider::Dispatch_Callbacks()
{
    unordered_set<CCollider*> CurrCopy = m_CurrContacts;
    unordered_set<CCollider*> PrevCopy = m_PrevContacts;

    for (auto& Other : CurrCopy)
    {
        if (PrevCopy.find(Other) == PrevCopy.end())
            Notify_Enter(Other);
        else
            Notify_Stay(Other);
    }
    for (auto& Other : PrevCopy)
    {
        if (CurrCopy.find(Other) == CurrCopy.end())
            Notify_Exit(Other);
    }
}

void CCollider::Remove_FromContacts(CCollider* pOther)
{
    m_PrevContacts.erase(pOther);
    m_CurrContacts.erase(pOther);
}

#ifdef _DEBUG

HRESULT CCollider::Render()
{
    m_pEffect->SetWorld(XMMatrixIdentity());
    m_pEffect->SetView(XMLoadFloat4x4(m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC)));
    m_pEffect->SetProjection(XMLoadFloat4x4(m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC)));
    m_pContext->IASetInputLayout(m_pInputLayout);

    m_pEffect->Apply(m_pContext);

    m_pBatch->Begin();

    if (FAILED(m_pBounding->Render(m_pBatch)))
        return E_FAIL;

    m_pBatch->End();

    return S_OK;
}
#endif

CCollider* CCollider::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, COLLIDER eType)
{
    CCollider* pInstance = new CCollider(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(eType)))
    {
        MSG_BOX("Failed to Created : CCollider");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CCollider::Clone(void* pArg)
{
    CCollider* pInstance = new CCollider(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CCollider");
        Safe_Release(pInstance);
    }

    return pInstance;
}
void CCollider::Free()
{
	__super::Free();

    if (m_bRegistered) {
        m_pGameInstance_Proxy->Immediate_Unregister(this, m_RegisteredGroup);
    }

    Safe_Release(m_pBounding);
#ifdef _DEBUG

    if (false == m_isCloned)
    {
        Safe_Delete(m_pEffect);
        Safe_Delete(m_pBatch);
    }

    Safe_Release(m_pInputLayout);
#endif

}
