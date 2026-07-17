#include "Dekabu_Body.h"
#include "GameInstance.h"

CDekabu_Body::CDekabu_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonsterPart{ pDevice, pContext }
{
}

CDekabu_Body::CDekabu_Body(const CDekabu_Body& Prototype)
	: CMonsterPart (Prototype)
{
}

HRESULT CDekabu_Body::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CDekabu_Body::Initialize(void* pArg)
{
	auto pDesc = static_cast<DEKABU_BODY_DESC*>(pArg);
	pDesc->fSpeedPerSec = 1.f;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pAnimatorCom->Play("Wait", true, true);

	return S_OK;
}

void CDekabu_Body::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

HRESULT CDekabu_Body::Render()
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

        if (FAILED(m_pShaderCom->Begin(1)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CDekabu_Body::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Monster;
    t.szModelProtoTag = TEXT("Prototype_Component_Model_Dekabu_Body");
    t.szAnimEventFile = TEXT("");   

    if (FAILED(Ready_MeshPart(t)))
        return E_FAIL;

	return S_OK;
}

CDekabu_Body* CDekabu_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CDekabu_Body* pInstance = new CDekabu_Body(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CDekabu_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CDekabu_Body::Clone(void* pArg)
{
    CDekabu_Body* pInstance = new CDekabu_Body(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CDekabu_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CDekabu_Body::Free()
{
    __super::Free();
}
