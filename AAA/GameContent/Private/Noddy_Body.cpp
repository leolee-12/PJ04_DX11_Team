#include "Noddy_Body.h"
#include "GameInstance.h"

CNoddy_Body::CNoddy_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterPart{ pDevice, pContext }
{
}

CNoddy_Body::CNoddy_Body(const CNoddy_Body& Prototype)
    : CMonsterPart(Prototype)
{
}

HRESULT CNoddy_Body::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CNoddy_Body::Initialize(void* pArg)
{
    auto pDesc = static_cast<NODDY_BODY_DESC*>(pArg);
    pDesc->fSpeedPerSec = 1.f;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pAnimatorCom->Play("Sleep", true, true);

    return S_OK;
}

HRESULT CNoddy_Body::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes =
        static_cast<_uint>(m_pModelCom->Get_NumMeshes());

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom,"g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;

        if (FAILED(m_pFaceTextureCom->Bind_ShaderResource(m_pShaderCom,"g_UnknownTexture", m_iFaceIndex)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom,"g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom,"g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;

        if (m_pAnimatorCom)
        {
            if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom,"g_BoneMatrices", i)))
                return E_FAIL;
        }

        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CNoddy_Body::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Monster;
    t.szModelProtoTag = TEXT("Prototype_Component_Model_Noddy_Body");
    t.szAnimEventFile = L"../../Resources/CHJ/Monster/Noddy/Body/Noddy_AnimEvents.json";

    if (FAILED(Ready_MeshPart(t)))
        return E_FAIL;

    m_pFaceTextureCom = Add_Component<CTexture>(TEXT("Com_FaceTexture"), CTexture::Create(m_pDevice, m_pContext, L"../../Resources/CHJ/Monster/Noddy/Body/ChNoddy.%02d.dds", FACE_COUNT));
    if (nullptr == m_pFaceTextureCom)
        return E_FAIL;

    return S_OK;
}

CNoddy_Body* CNoddy_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CNoddy_Body* pInstance = new CNoddy_Body(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CNoddy_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CNoddy_Body::Clone(void* pArg)
{
    CNoddy_Body* pInstance = new CNoddy_Body(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CNoddy_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CNoddy_Body::Free()
{
    __super::Free();
}
