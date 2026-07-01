#include "NormalEnemy_Body.h"
#include "Animator.h"
#include "GameInstance.h"

CNormalEnemy_Body::CNormalEnemy_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonsterPart { pDevice, pContext }
{
}

CNormalEnemy_Body::CNormalEnemy_Body(const CNormalEnemy_Body& Prototype)
	: CMonsterPart (Prototype)
{
}

HRESULT CNormalEnemy_Body::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CNormalEnemy_Body::Initialize(void* pArg)
{
	auto pDesc = static_cast<NORMALENEMY_BODY_DESC*>(pArg);
	pDesc->fSpeedPerSec = 1.f;

	if (FAILED(__super::Initialize(pDesc))) 
		return E_FAIL;

	if (FAILED(Ready_Components()))         
		return E_FAIL;

	// 애니메이션 지정
	m_pAnimatorCom->Play("Wait", true, true);

	return S_OK;
}

HRESULT CNormalEnemy_Body::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		// 바디 알베도 (Pass0에서 UV1으로 샘플)
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
			return E_FAIL;

		// 눈 텍스처 -> g_UnknownTexture 
		if (FAILED(m_pEyeTextureCom->Bind_ShaderResource(m_pShaderCom, "g_UnknownTexture", m_iEyeIndex)))
			return E_FAIL;

		// 실제 노멀맵은 NORMALS 슬롯 1번
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 1)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
			return E_FAIL;

		if (m_pAnimatorCom)
		{
			if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
				return E_FAIL;
		}

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CNormalEnemy_Body::Ready_Components()
{
	PART_SETUP t{};
	t.tShader = Shader_AnimMesh_PBR;
	t.szModelProtoTag = TEXT("Prototype_Component_Model_NormalEnemy_Body");
	t.szAnimEventFile =	TEXT("../../Resources/CHJ/Monster/NormalEnemy/NormalEnemy_AnimEvents.json");
	if (FAILED(Ready_MeshPart(t)))
		return E_FAIL;

	// 눈 표정 텍스처 배열
	m_pEyeTextureCom = Add_Component<CTexture>(TEXT("Com_EyeTexture"),
		CTexture::Create(m_pDevice, m_pContext,
			L"../../Resources/CHJ/Monster/NormalEnemy/NormalEnemyEye.%02d.dds", EYE_COUNT));
	if (nullptr == m_pEyeTextureCom)
		return E_FAIL;

	return S_OK;
}

CNormalEnemy_Body* CNormalEnemy_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNormalEnemy_Body* pInstance = new CNormalEnemy_Body(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype())) {
		MSG_BOX("Failed to Created: CNormalEnemy_Body");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CNormalEnemy_Body::Clone(void* pArg)
{
	CNormalEnemy_Body* pInstance = new CNormalEnemy_Body(*this);
	if (FAILED(pInstance->Initialize(pArg))) {
		MSG_BOX("Failed to Cloned: CNormalEnemy_Body");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CNormalEnemy_Body::Free()
{
	__super::Free();
}
