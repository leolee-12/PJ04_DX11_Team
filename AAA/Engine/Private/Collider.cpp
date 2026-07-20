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

    case COLLIDER::CAPSULE:
    {
        CBounding_Capsule::BOUNDING_CAPSULE_DESC d{};
        d.vCenter = pColliderDesc->vCenter;
        d.fRadius = pColliderDesc->fRadius;
        d.fHeight = pColliderDesc->fHeight;
        d.vRadians = pColliderDesc->vRadians;
        m_pBounding = CBounding_Capsule::Create(m_pDevice, m_pContext, &d);
        break;
    }

    case COLLIDER::TORUS:
    {
        CBounding_Torus::BOUNDING_TORUS_DESC d{};
        d.vCenter = pColliderDesc->vCenter;
        d.fRingRadius = pColliderDesc->fRadius;   // fRadius = 링 반지름
        d.fTubeRadius = pColliderDesc->fHeight;   // fHeight = 튜브 반지름으로 재활용
        d.vRadians = pColliderDesc->vRadians;
        d.fArcDeg = (pColliderDesc->vSize.x > 1.f) ? pColliderDesc->vSize.x : 360.f;
        m_pBounding = CBounding_Torus::Create(m_pDevice, m_pContext, &d);
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

void CCollider::Reset_Bounding(const COLLIDER_DESC& Desc)
{
    switch (m_eType)
    {
        case COLLIDER::AABB:
        {
            CBounding_AABB::BOUNDING_AABB_DESC BoundingDesc{};
            BoundingDesc.vCenter = Desc.vCenter;
            BoundingDesc.vSize = Desc.vSize;
            m_pBounding->Reset_Desc(&BoundingDesc);
            break;
        }

        case COLLIDER::OBB:
        {
            CBounding_OBB::BOUNDING_OBB_DESC BoundingDesc{};
            BoundingDesc.vCenter = Desc.vCenter; 
            BoundingDesc.vSize = Desc.vSize;
            BoundingDesc.vRadians = Desc.vRadians;
            m_pBounding->Reset_Desc(&BoundingDesc);
            break;
        }

        case COLLIDER::SPHERE:
        {
            CBounding_Sphere::BOUNDING_SPHERE_DESC BoundingDesc{};
            BoundingDesc.vCenter = Desc.vCenter;
            BoundingDesc.fRadius = Desc.fRadius;
            m_pBounding->Reset_Desc(&BoundingDesc);
            break;
        }

        case COLLIDER::CAPSULE:
        {
            CBounding_Capsule::BOUNDING_CAPSULE_DESC BoundingDesc{};
            BoundingDesc.vCenter = Desc.vCenter;
            BoundingDesc.fRadius = Desc.fRadius;
            BoundingDesc.fHeight = Desc.fHeight;
            BoundingDesc.vRadians = Desc.vRadians;
            m_pBounding->Reset_Desc(&BoundingDesc);
            break;
        }

        case COLLIDER::TORUS:
        {
            CBounding_Torus::BOUNDING_TORUS_DESC d{};
            d.vCenter = Desc.vCenter;
            d.fRingRadius = Desc.fRadius;   // fRadius = 링 반지름
            d.fTubeRadius = Desc.fHeight;   // fHeight = 튜브 반지름으로 재활용
            d.vRadians = Desc.vRadians;
            d.fArcDeg = (Desc.vSize.x > 1.f) ? Desc.vSize.x : 360.f;
            m_pBounding->Reset_Desc(&d);
            break;
        }
    }
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
void CCollider::Set_DebugText(const _wstring& strFontTag, const _wstring& strText, const _float4& vColor, const _float2&
    vScale, TEXT_ALIGN eAlign)
{
    m_bUseDebugText = true;
    m_strDebugFontTag = strFontTag;
    m_strDebugText = strText;
    m_vDebugTextColor = vColor;
    m_vDebugTextScale = vScale;
    m_eDebugTextAlign = eAlign;
}

void CCollider::Set_DebugRenderColor(const _float4& vColor)
{
    if (nullptr == m_pBounding)
        return;

    m_pBounding->Set_DebugColor(vColor);
}

void CCollider::Clear_DebugText()
{
    m_bUseDebugText = false;
    m_strDebugFontTag.clear();
    m_strDebugText.clear();
}

HRESULT CCollider::Render()
{
    m_pEffect->SetWorld(XMMatrixIdentity());
    m_pEffect->SetView(XMLoadFloat4x4(m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC)));
    m_pEffect->SetProjection(XMLoadFloat4x4(m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC)));
    m_pContext->IASetInputLayout(m_pInputLayout);

    m_pEffect->Apply(m_pContext);

    m_pBatch->Begin();

    m_pBounding->Set_Colliding(!m_CurrContacts.empty());

    if (FAILED(m_pBounding->Render(m_pBatch)))
        return E_FAIL;

    m_pBatch->End();

    return S_OK;
}

HRESULT CCollider::Render_DebugText()
{
    if (!m_bUseDebugText || m_strDebugFontTag.empty() || m_strDebugText.empty())
        return S_OK;

    _float3 vWorldPos{};
    if (false == Try_GetDebugTextWorldPos(&vWorldPos))
        return S_OK;

    const _float4x4* pView = m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC);
    const _float4x4* pProj = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);
    if (nullptr == pView || nullptr == pProj)
        return S_OK;

    const _vector vClipPos = XMVector4Transform(XMVector4Transform(XMVectorSet(vWorldPos.x, vWorldPos.y, vWorldPos.z, 1.f),
        XMLoadFloat4x4(pView)), XMLoadFloat4x4(pProj));
    const _float fW = XMVectorGetW(vClipPos);
    if (fW <= 0.001f)
        return S_OK;

    const _float fNdcX = XMVectorGetX(vClipPos) / fW;
    const _float fNdcY = XMVectorGetY(vClipPos) / fW;
    const _float fNdcZ = XMVectorGetZ(vClipPos) / fW;
    if (fNdcX < -1.f || fNdcX > 1.f || fNdcY < -1.f || fNdcY > 1.f || fNdcZ < 0.f || fNdcZ > 1.f)
        return S_OK;

    D3D11_VIEWPORT Viewport{};
    UINT iNumViewports = 1;
    m_pContext->RSGetViewports(&iNumViewports, &Viewport);
    if (0 == iNumViewports || Viewport.Width <= 0.f || Viewport.Height <= 0.f)
        return S_OK;

    _float2 vPixelPos{};
    vPixelPos.x = Viewport.TopLeftX + (fNdcX * 0.5f + 0.5f) * Viewport.Width;
    vPixelPos.y = Viewport.TopLeftY + (-fNdcY * 0.5f + 0.5f) * Viewport.Height;

    _float4 vColor = !m_CurrContacts.empty() ? _float4(1.f, 0.f, 0.f, 1.f) : m_vDebugTextColor;
    m_pGameInstance_Proxy->Draw_Text_Raw(m_strDebugFontTag, m_strDebugText.c_str(), vPixelPos, XMLoadFloat4(&vColor), m_vDebugTextScale, m_eDebugTextAlign);

    return S_OK;
}

_bool CCollider::Try_GetDebugTextWorldPos(_float3* pOut) const
{
    if (nullptr == pOut || nullptr == m_pBounding)
        return false;

    switch (m_eType)
    {
    case COLLIDER::AABB:
    {
        const BoundingBox* pDesc = static_cast<CBounding_AABB*>(m_pBounding)->Get_Desc();
        *pOut = pDesc->Center;
        pOut->y += pDesc->Extents.y + 0.35f;
        return true;
    }
    case COLLIDER::OBB:
    {
        const BoundingOrientedBox* pDesc = static_cast<CBounding_OBB*>(m_pBounding)->Get_Desc();
        const _float fExtent = max(pDesc->Extents.x, max(pDesc->Extents.y, pDesc->Extents.z));
        *pOut = pDesc->Center;
        pOut->y += fExtent + 0.35f;
        return true;
    }
    case COLLIDER::SPHERE:
    {
        const BoundingSphere* pDesc = static_cast<CBounding_Sphere*>(m_pBounding)->Get_Desc();
        *pOut = pDesc->Center;
        pOut->y += pDesc->Radius + 0.35f;
        return true;
    }
    case COLLIDER::CAPSULE:
    {
        _float3 vP0{}, vP1{};
        CBounding_Capsule* pCapsule = static_cast<CBounding_Capsule*>(m_pBounding);
        pCapsule->Get_Segment(vP0, vP1);

        pOut->x = (vP0.x + vP1.x) * 0.5f;
        pOut->y = max(vP0.y, vP1.y) + pCapsule->Get_Radius() + 0.35f;
        pOut->z = (vP0.z + vP1.z) * 0.5f;
        return true;
    }
    }

    return false;
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
