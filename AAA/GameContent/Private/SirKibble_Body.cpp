#include "SirKibble_Body.h"
#include "GameInstance.h"

CSirKibble_Body::CSirKibble_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterPart{ pDevice, pContext }
{
}

CSirKibble_Body::CSirKibble_Body(const CSirKibble_Body& Prototype)
    : CMonsterPart(Prototype)
{
}

HRESULT CSirKibble_Body::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CSirKibble_Body::Initialize(void* pArg)
{
    auto pDesc = static_cast<SIRKIBBLE_BODY_DESC*>(pArg);
    pDesc->fSpeedPerSec = 1.f;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pAnimatorCom->Play("Wait", true, true);

    return S_OK;
}

HRESULT CSirKibble_Body::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes =
        static_cast<_uint>(m_pModelCom->Get_NumMeshes());

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;

        if (m_pAnimatorCom)
        {
            if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
                return E_FAIL;
        }

        if (FAILED(m_pShaderCom->Begin(8)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CSirKibble_Body::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Monster;
    t.szModelProtoTag = TEXT("Prototype_Component_Model_SirKibble_Body");
    t.szAnimEventFile = L"";

    if (FAILED(Ready_MeshPart(t)))
        return E_FAIL;

    return S_OK;
}

CSirKibble_Body* CSirKibble_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSirKibble_Body* pInstance = new CSirKibble_Body(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSirKibble_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CSirKibble_Body::Clone(void* pArg)
{
    CSirKibble_Body* pInstance = new CSirKibble_Body(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSirKibble_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CSirKibble_Body::Free()
{
    __super::Free();
}
