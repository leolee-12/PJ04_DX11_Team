#include "Cappy_Body.h"
#include "GameInstance.h"

CCappy_Body::CCappy_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonsterPart{ pDevice, pContext }
{
}

CCappy_Body::CCappy_Body(const CCappy_Body& Prototype)
	: CMonsterPart (Prototype)
{
}

HRESULT CCappy_Body::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CCappy_Body::Initialize(void* pArg)
{
	auto pDesc = static_cast<CAPPY_BODY_DESC*>(pArg);
	pDesc->fSpeedPerSec = 1.f;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pAnimatorCom->Play("Wait", true, true);

	return S_OK;
}

HRESULT CCappy_Body::Render()
{
	if (FAILED(Bind_ShaderResources()))
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

		if (m_pAnimatorCom)                            
		{
			if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
				return E_FAIL;
		}

		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;

		// 숨어 있는 상태 + 모자 쓰고 있을 때 팔을 렌더 하지 않음
		if (m_bIsHidden && m_bHatAttached && i == 1)
			continue;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CCappy_Body::Ready_Components()
{
	PART_SETUP t{};
	t.tShader = Shader_Monster;
	t.szModelProtoTag = TEXT("Prototype_Component_Model_Cappy_Body");
	t.szAnimEventFile = TEXT("../../Resources/CHJ/Monster/Cappy/Body/Cappy_Body_AnimEvents.json");
	if (FAILED(Ready_MeshPart(t)))
		return E_FAIL;

	return S_OK;
}

CCappy_Body* CCappy_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCappy_Body* pInstance = new CCappy_Body(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CCappy_Body");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCappy_Body::Clone(void* pArg)
{
	CCappy_Body* pInstance = new CCappy_Body(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CCappy_Body");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCappy_Body::Free()
{
	__super::Free();
}
