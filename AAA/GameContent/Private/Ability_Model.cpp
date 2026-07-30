#include "Ability_Model.h"
#include "GameInstance.h"

namespace
{
	inline constexpr _float ABILITY_SPIN_SPEED = 240.f;
}

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

	if (m_pAnimatorCom )
	{
		m_fSpinAngle = fmodf(m_fSpinAngle + ABILITY_SPIN_SPEED * fTimeDelta, 360.f);
		m_pAnimatorCom->SetBoneRotation("TopL", m_fSpinAngle, XMVectorSet(0.f, 1.f, 0.f, 0.f));

		m_pAnimatorCom->Update(fTimeDelta);
	}


}

void CAbility_Model::Late_Update(_float fTimeDelta)
{
	_matrix matWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	if (m_pSocketBoneMatrix)
		matWorld = matWorld * XMLoadFloat4x4(m_pSocketBoneMatrix);

	Compute_CombinedWorldMatrix(matWorld);

	if (!m_bRenderActive) return;
	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
}

HRESULT CAbility_Model::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	switch (m_eAbility)
	{
	case COPY_ABILITY_TYPE::SWORD:
		return Render_Sword();
	case COPY_ABILITY_TYPE::BOMB:
		return Render_Bomb();
	case COPY_ABILITY_TYPE::ICE:
		return Render_Ice();
	case COPY_ABILITY_TYPE::TOY_HAMMER:
		return Render_Hammer();
	case COPY_ABILITY_TYPE::CRASH:
		return Render_Crash();
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
	
	switch (m_eAbility)
	{
	case COPY_ABILITY_TYPE::BOMB:
	{
		t.tShader = Shader_Bomb;
		m_iShadowPassIdx = 2;
		break;
	}
	default:
	{
		t.tShader = Shader_AbillityModel;
		m_iShadowPassIdx = 0;
		break;
	}
	}
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

HRESULT CAbility_Model::Render_Sword()
{
	static constexpr _uint   iJewelMesh = 1u;
	static constexpr _float4 vJewelDiffuse = { 1.f, 0.72f, 0.08f, 1.f };
	static constexpr _float3 vJewelMRA = { 0.25f, 0.18f, 1.f };
	static constexpr _float4 vJewelEmissive = { 0.05f, 0.025f, 0.f, 1.f };

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		_uint iPass = 1u;

		if (iJewelMesh != i)
		{
			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom,	"g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom,	"g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
				return E_FAIL;
		}
		else
		{
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantDiffuse", &vJewelDiffuse, sizeof(_float4))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantMRA", &vJewelMRA, sizeof(_float3))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantEmissive", &vJewelEmissive, sizeof(_float4))))
				return E_FAIL;

			iPass = 2u;
		}

		if (m_pAnimatorCom)
		{
			if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
				return E_FAIL;
		}

		if (FAILED(m_pShaderCom->Begin(iPass)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CAbility_Model::Render_Bomb()
{
	static constexpr _float  fBurnRatio = 0.f;
	static constexpr _float3 vGlow = { 0.f, 0.f, 0.f };

	if (FAILED(m_pShaderCom->Bind_RawValue("g_fBurnRatio", &fBurnRatio, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vGlow", &vGlow, sizeof(_float3))))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		_uint iPass = 0u;

		if (1u == i)   // FuseM__FuseC (심지)
		{
			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom,	"g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom,	"g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom,	"g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom,	"g_FuseMaskTexture", i, MTEX_TYPE::UNKNOWN, 0)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom,	"g_FuseBurntTexture", i, MTEX_TYPE::UNKNOWN, 1)))
				return E_FAIL;
			iPass = 1u;
		}
		else   // BombM / MetalM (몸통)
		{
			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom,	"g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 1)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom,	"g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom,	"g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
				return E_FAIL;

			iPass = 0u;
		}

		if (m_pAnimatorCom)
		{
			if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
				return E_FAIL;
		}

		if (FAILED(m_pShaderCom->Begin(iPass)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CAbility_Model::Render_Ice()
{
	static constexpr _float4 vIceDiffuse =	{ 0.70f, 0.84f, 0.95f, 1.f };
	static constexpr _float3 vIceMRA =	{ 0.f, 0.15f, 1.f };
	static constexpr _float4 vIceEmissive =	{ 0.f, 0.f, 0.f, 1.f };

	static constexpr _float4 vJuelDiffuse =	{ 0.45f, 0.78f, 0.95f, 1.f };
	static constexpr _float3 vJuelMRA =	{ 0.10f, 0.10f, 1.f };
	static constexpr _float4 vJuelEmissive = { 0.03f, 0.06f, 0.09f, 1.f };

	const _uint iNumMeshes =
		static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		_uint iPass = 1u;

		if (0u == i)   
		{
			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture",	i, MTEX_TYPE::DIFFUSE, 1)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture",	i, MTEX_TYPE::NORMALS, 0)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture",	i, MTEX_TYPE::METALNESS, 0)))
				return E_FAIL;
			iPass = 1u;
		}
		else   // Ice(1,2) / Juel(3) = 상수 Pass3
		{
			const _bool bJuel = (3u == i);

			const _float4* pDiff = bJuel ? &vJuelDiffuse : &vIceDiffuse;
			const _float3* pMRA = bJuel ? &vJuelMRA : &vIceMRA;
			const _float4* pEmi = bJuel ? &vJuelEmissive : &vIceEmissive;

			if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantDiffuse", pDiff, sizeof(_float4))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantMRA", pMRA, sizeof(_float3))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantEmissive", pEmi, sizeof(_float4))))
				return E_FAIL;

			iPass = 2u;
		}

		if (m_pAnimatorCom)
		{
			if (FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)))
				return E_FAIL;
		}

		if (FAILED(m_pShaderCom->Begin(iPass)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CAbility_Model::Render_Hammer()
{
	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(3u)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CAbility_Model::Render_Crash()
{
	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(1u)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

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
