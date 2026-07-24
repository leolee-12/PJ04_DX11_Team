#include "WaddleDee_Hat.h"
#include "Shader.h"
#include "Model.h"

CWaddleDee_Hat::CWaddleDee_Hat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonsterPart{ pDevice, pContext }
{
}

CWaddleDee_Hat::CWaddleDee_Hat(const CWaddleDee_Hat& Prototype)
	: CMonsterPart(Prototype)
{
}

HRESULT CWaddleDee_Hat::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CWaddleDee_Hat::Initialize(void* pArg)
{
	auto pDesc = static_cast<WADDLEDEE_HAT_DESC*>(pArg);
	if (nullptr == pDesc || nullptr == pDesc->szModelProtoTag)
		return E_FAIL;

	m_szModelProtoTag = pDesc->szModelProtoTag;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_iShadowPassIdx = 2;

	return Ready_Components();
}

HRESULT CWaddleDee_Hat::Render()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
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

		if (FAILED(m_pShaderCom->Begin(17)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CWaddleDee_Hat::Ready_Components()
{
	PART_SETUP t{};
	t.tShader = Shader_NonAnimMesh_PBR;
	t.szModelProtoTag = m_szModelProtoTag;
	t.bAnimated = false;
	return Ready_MeshPart(t);
}

CWaddleDee_Hat* CWaddleDee_Hat::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWaddleDee_Hat* pInstance = new CWaddleDee_Hat(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CWaddleDee_Hat");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CWaddleDee_Hat::Clone(void* pArg)
{
	CWaddleDee_Hat* pInstance = new CWaddleDee_Hat(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CWaddleDee_Hat");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CWaddleDee_Hat::Free()
{
	__super::Free();
}