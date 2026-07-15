#include "DropStar_Body.h"
#include "GameInstance.h"

CDropStar_Body::CDropStar_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPartObject {pDevice, pContext}
{
}

CDropStar_Body::CDropStar_Body(const CDropStar_Body& Prototype)
	: CPartObject(Prototype)
{
}

HRESULT CDropStar_Body::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CDropStar_Body::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (m_pAnimatorCom)
		m_pAnimatorCom->Play("NormalPosition", true, true);
		//m_pAnimatorCom->Play("BoundlPosition", true, true);


	return S_OK;
}

void CDropStar_Body::Update(_float fTimeDelta)
{
	if (m_pGameInstance_Proxy->Is_EditMode())
		return;

	if (m_pAnimatorCom)
		m_pAnimatorCom->Update(fTimeDelta);
}

void CDropStar_Body::Late_Update(_float fTimeDelta)
{
	_matrix matWorld =
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	Compute_CombinedWorldMatrix(matWorld);

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
}

HRESULT CDropStar_Body::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantDiffuse", &m_vDiffuse, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantMRA", &m_vMRA, sizeof(_float3))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantEmissive", &m_vEmissive, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID",	&m_iMaterialID, sizeof(_uint))))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(3)))     
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CDropStar_Body::Render_Shadow()
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
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom,	"g_BoneMatrices", i)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Begin(m_iShadowPassIdx)))  
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

const _float4x4* CDropStar_Body::Get_BoneMatrixPtr(const _string& strBone) const
{
	if (nullptr == m_pModelCom)
		return nullptr;

	return m_pModelCom->Get_BoneMatrixPtr(strBone);
}

HRESULT CDropStar_Body::Ready_Components()
{
	m_pShaderCom = Add_Component<CShader>(Shader_AnimMesh_PBR.iLevelID,	Shader_AnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, MODEL_PROTO_TAG, TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	CAnimator::ANIMATOR_DESC AnimDesc{};
	AnimDesc.pModel = m_pModelCom;
	AnimDesc.strDataFile = TEXT("");

	m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"),	CAnimator::Create(m_pDevice, m_pContext));
	if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CDropStar_Body::Bind_ShaderResources()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	return S_OK;
}

CDropStar_Body* CDropStar_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDropStar_Body* pInstance = new CDropStar_Body(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CDropStar_Body");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CDropStar_Body::Clone(void* pArg)
{
	CDropStar_Body* pInstance = new CDropStar_Body(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CDropStar_Body");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CDropStar_Body::Free()
{
	__super::Free();
}
