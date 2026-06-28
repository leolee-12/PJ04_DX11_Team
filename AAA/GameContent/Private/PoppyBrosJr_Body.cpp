#include "PoppyBrosJr_Body.h"
#include "Animator.h"
#include "GameInstance.h"

CPoppyBrosJr_Body::CPoppyBrosJr_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonsterPart{ pDevice, pContext }
{
}

CPoppyBrosJr_Body::CPoppyBrosJr_Body(const CPoppyBrosJr_Body& Prototype)
	: CMonsterPart (Prototype)
{
}

HRESULT CPoppyBrosJr_Body::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CPoppyBrosJr_Body::Initialize(void* pArg)
{
	auto pDesc = static_cast<POPPYBROSJR_BODY_DESC*>(pArg);
	pDesc->fSpeedPerSec = 1.f;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	// 애니메이션 지정
	m_pAnimatorCom->Play("Wait", true, true);
	
	return S_OK;
}

void CPoppyBrosJr_Body::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

HRESULT CPoppyBrosJr_Body::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	for (_uint i = 0; i < iNumMeshes; ++i)
	{
	 	const _bool bIsFace = (i == s_iFaceMeshIndex);

		if (bIsFace)
		{
			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
				return E_FAIL;

			if (FAILED(m_pEyeTextureCom->Bind_ShaderResource(m_pShaderCom, "g_UnknownTexture", m_iEyeIndex)))
				return E_FAIL;

			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
				return E_FAIL;
		}
		else
		{
			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
				return E_FAIL;

			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
				return E_FAIL;

			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
				return E_FAIL;
		}

		if (m_pAnimatorCom)
		{
			if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
				return E_FAIL;
		}

		if (FAILED(m_pShaderCom->Begin(bIsFace ? 6 : 1)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;		
	}

	return S_OK;
}

HRESULT CPoppyBrosJr_Body::Ready_Components()
{
	PART_SETUP t{};
	t.tShader = Shader_AnimMesh_PBR;
	t.szModelProtoTag = TEXT("Prototype_Component_Model_PoppyBrosJr_Body");

	if (FAILED(Ready_MeshPart(t)))
		return E_FAIL;

	// 눈 표정 텍스처 배열
	m_pEyeTextureCom = Add_Component<CTexture>(TEXT("Com_EyeTexture"),
		CTexture::Create(m_pDevice, m_pContext,
			L"../../Resources/CHJ/Monster/PoppyBrosJr/Model/FaceTexturePattern.%02d.dds", EYE_COUNT));
	if (nullptr == m_pEyeTextureCom)
		return E_FAIL;

	return S_OK;
}

CPoppyBrosJr_Body* CPoppyBrosJr_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPoppyBrosJr_Body* pInstance = new CPoppyBrosJr_Body(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype())) {
		MSG_BOX("Failed to Created: CPoppyBrosJr_Body");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CPoppyBrosJr_Body::Clone(void* pArg)
{
	CPoppyBrosJr_Body* pInstance = new CPoppyBrosJr_Body(*this);
	if (FAILED(pInstance->Initialize(pArg))) {
		MSG_BOX("Failed to Cloned: CPoppyBrosJr_Body");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPoppyBrosJr_Body::Free()
{
	__super::Free();
}
