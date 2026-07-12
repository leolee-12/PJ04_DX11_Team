#include "GigantEdge_Sword.h"
#include "GameInstance.h"
#include "Damageable.h"
#include "Shader_PassMeta.h"

CGigantEdge_Sword::CGigantEdge_Sword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterHitPart(pDevice, pContext) {
}
CGigantEdge_Sword::CGigantEdge_Sword(const CGigantEdge_Sword& Prototype)
    : CMonsterHitPart(Prototype) {
}

HRESULT CGigantEdge_Sword::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))            return E_FAIL;
    if (FAILED(Ready_Components()))                   return E_FAIL;
    CAPSULE_DESC CapsuleDesc{};
    CapsuleDesc.vCenter = { 0.f, 0.f, 0.f };
    CapsuleDesc.fHeight = 1.5f;
    CapsuleDesc.fRadius = 0.6f;
    CapsuleDesc.vRadians = { XMConvertToRadians(-90.f), 0.f, 0.f};
    if (FAILED(Ready_HitBox(CapsuleDesc)))
        return E_FAIL;

    m_iShadowPassIdx = 2;

    Set_Drawn(false);                               
    return S_OK;
}

HRESULT CGigantEdge_Sword::Render()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW,
        m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ,
        m_eProjType))))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return E_NOTIMPL;
}

HRESULT CGigantEdge_Sword::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_NonAnimMesh_PBR;
    t.szModelProtoTag = TEXT("Prototype_Component_Model_GigantEdge_Sword");
    t.bAnimated = false;
    return Ready_MeshPart(t);
}

CGigantEdge_Sword* CGigantEdge_Sword::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CGigantEdge_Sword* pInstance = new CGigantEdge_Sword(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CGigantEdge_Sword");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGigantEdge_Sword* CGigantEdge_Sword::Clone(void* pArg)
{
    CGigantEdge_Sword* pInstance = new CGigantEdge_Sword(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CGigantEdge_Sword");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CGigantEdge_Sword::Free() { __super::Free(); }