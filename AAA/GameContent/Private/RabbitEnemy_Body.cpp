#include "RabbitEnemy_Body.h"
#include "GameInstance.h"

CRabbitEnemy_Body::CRabbitEnemy_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterPart{ pDevice, pContext }
{
}

CRabbitEnemy_Body::CRabbitEnemy_Body(const CRabbitEnemy_Body& Prototype)
    : CMonsterPart(Prototype)
{
}

HRESULT CRabbitEnemy_Body::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CRabbitEnemy_Body::Initialize(void* pArg)
{
    auto pDesc = static_cast<RABBITENEMY_BODY_DESC*>(pArg);
    pDesc->fSpeedPerSec = 1.f;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pAnimatorCom->Play("Wait", true, true);

    return S_OK;
}

HRESULT CRabbitEnemy_Body::Render()
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

HRESULT CRabbitEnemy_Body::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Monster;
    t.szModelProtoTag = TEXT("Prototype_Component_Model_RabbitEnemy_Body");
    t.szAnimEventFile = L"../../Resources/CHJ/Monster/RabbitEnemy/Body/RabbitEnemy_AnimEvents.json";

    if (FAILED(Ready_MeshPart(t)))
        return E_FAIL;

    m_pFaceTextureCom = Add_Component<CTexture>(TEXT("Com_FaceTexture"), CTexture::Create(m_pDevice, m_pContext, L"../../Resources/CHJ/Monster/RabbitEnemy/Body/RabbitEnemyEye.%02d.dds", FACE_COUNT));
    if (nullptr == m_pFaceTextureCom)
        return E_FAIL;

    return S_OK;
}

CRabbitEnemy_Body* CRabbitEnemy_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CRabbitEnemy_Body* pInstance = new CRabbitEnemy_Body(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CRabbitEnemy_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CRabbitEnemy_Body::Clone(void* pArg)
{
    CRabbitEnemy_Body* pInstance = new CRabbitEnemy_Body(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CRabbitEnemy_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CRabbitEnemy_Body::Free()
{
    __super::Free();
}
