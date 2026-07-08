#include "Ability_Model.h"
#include "GameInstance.h"

CAbility_Model::CAbility_Model(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPartObject{ pDevice, pContext }
{
}

CAbility_Model::CAbility_Model(const CAbility_Model& Prototype)
	: CPartObject(Prototype)
{
}

HRESULT CAbility_Model::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CAbility_Model::Initialize(void* pArg)
{
	auto pDesc = static_cast<ABILITY_MODEL_DESC*>(pArg);
	if (nullptr == pDesc)
		return E_FAIL;

	m_eAbility			= pDesc->eAbility;
	m_szModelProtoTag	= pDesc->szModelProtoTag;
	m_pSocketBoneMatrix = pDesc->pSocketBoneMatrix;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (m_pAnimatorCom)
		m_pAnimatorCom->Play("AbilityItem", true, true);

	return S_OK;
}

void CAbility_Model::Update(_float fTimeDelta)
{
	if (m_pGameInstance_Proxy->Is_EditMode())
		return;
	if (m_pAnimatorCom)
		m_pAnimatorCom->Update(fTimeDelta);
}

void CAbility_Model::Late_Update(_float fTimeDelta)
{
	_matrix matWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	if (m_pSocketBoneMatrix)
		matWorld = matWorld * XMLoadFloat4x4(m_pSocketBoneMatrix);

	Compute_CombinedWorldMatrix(matWorld);
	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
}

HRESULT CAbility_Model::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	// TODO : 능력 별로 모델, Shader  다르게 설정 & Render에서도 분기하기. 지금은 AnimMesh PBR 공통으로 처리

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (m_pAnimatorCom)
			m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i);

		// diffuse 없으면 무텍스처 -> 상수색 패스(3)
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom,
			"g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
		{
			if (FAILED(m_pShaderCom->Begin(3)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Render(i)))
				return E_FAIL;
			continue;
		}

		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0);

		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CAbility_Model::Render_Shadow()
{
	if (nullptr == m_pModelCom || nullptr == m_pShaderCom)
		return S_OK;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (m_pAnimatorCom)
			m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		if (FAILED(m_pShaderCom->Begin(m_iShadowPassIdx)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

const _float4x4* CAbility_Model::Get_BoneMatrixPtr(const _char* pBoneName) const
{
	if (nullptr == m_pModelCom || nullptr == pBoneName)
		return nullptr;

	return m_pModelCom->Get_BoneMatrixPtr(pBoneName);
}

HRESULT CAbility_Model::Ready_MeshPart(const PART_SETUP& tSetup)
{
	m_pShaderCom = Add_Component<CShader>(tSetup.tShader.iLevelID, tSetup.tShader.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom) return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, tSetup.szModelProtoTag, TEXT("Com_Model"));
	if (nullptr == m_pModelCom) return E_FAIL;

	if (tSetup.bAnimated)
	{
		CAnimator::ANIMATOR_DESC AnimDesc{};
		AnimDesc.pModel = m_pModelCom;
		AnimDesc.strDataFile = tSetup.szAnimEventFile ? tSetup.szAnimEventFile : TEXT("");

		m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"),	CAnimator::Create(m_pDevice, m_pContext));

		if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CAbility_Model::Ready_Components()
{
	PART_SETUP t{};
	// TODO : 능력별 모델 전용 SHADER 세팅 ( 지금은 PBR 공통 )
	t.tShader = Shader_AnimMesh_PBR;
	t.szModelProtoTag = m_szModelProtoTag;
	t.bAnimated = true;
	t.szAnimEventFile = nullptr;

	if (FAILED(Ready_MeshPart(t)))
		return E_FAIL;

	return S_OK;
}

HRESULT CAbility_Model::Bind_ShaderResources()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	return S_OK;
}

CAbility_Model* CAbility_Model::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAbility_Model* pInstance = new CAbility_Model(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAbility_Model");
		Safe_Release(pInstance);
	}
	return pInstance;;
}

CGameObject* CAbility_Model::Clone(void* pArg)
{
	CAbility_Model* pInstance = new CAbility_Model(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CAbility_Model");
		Safe_Release(pInstance);
	}
	return pInstance;;
}

void CAbility_Model::Free()
{
	__super::Free();
}
