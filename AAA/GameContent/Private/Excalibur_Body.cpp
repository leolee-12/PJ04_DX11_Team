#include "Excalibur_Body.h"
#include "GameInstance.h"

CExcalibur_Body::CExcalibur_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterPart(pDevice, pContext) {
}
CExcalibur_Body::CExcalibur_Body(const CExcalibur_Body& Prototype)
    : CMonsterPart(Prototype) {
}

HRESULT CExcalibur_Body::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))  return E_FAIL;
    if (FAILED(Ready_Components()))         return E_FAIL;

    m_iShadowPassIdx = 2;

    return S_OK;
}

HRESULT CExcalibur_Body::Render()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW,
        m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ,
        m_eProjType))))
        return E_FAIL;

    const size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

    static constexpr _uint iJewelMeshIndex = 1;

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPassIndex{};

        if (i != iJewelMeshIndex) {

            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
                return E_FAIL;

            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
                return E_FAIL;

            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
                return E_FAIL;

            iPassIndex = 17;
        }
        else
        {
            static constexpr _float4 vConstantDiffuse = { 1.f, 0.72f, 0.08f, 1.f };
            if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantDiffuse", &vConstantDiffuse, sizeof(_float4))))
                return E_FAIL;

            static constexpr _float3 vConstantMRA = { 0.25f, 0.18f, 1.f };
            if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantMRA", &vConstantMRA, sizeof(_float3))))
                return E_FAIL;

            static constexpr _float4 vConstantEmissive = { 0.05f, 0.025f, 0.f, 1.f };
            if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantEmissive", &vConstantEmissive, sizeof(_float4))))
                return E_FAIL;

            iPassIndex = 18;
        }

        if (FAILED(m_pShaderCom->Begin(iPassIndex)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }
    return S_OK;
}

HRESULT CExcalibur_Body::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_NonAnimMesh_PBR;
    t.szModelProtoTag = MODEL_PROTO_TAG;
    t.bAnimated = false;
    return Ready_MeshPart(t);
}

CExcalibur_Body* CExcalibur_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CExcalibur_Body* pInstance = new CExcalibur_Body(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CExcalibur_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CExcalibur_Body* CExcalibur_Body::Clone(void* pArg)
{
    CExcalibur_Body* pInstance = new CExcalibur_Body(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CExcalibur_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CExcalibur_Body::Free() { __super::Free(); }