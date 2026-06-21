#include "LevelDesign_Breakable.h"
#include "Shader_PassMeta.h"
#include "Parsing_Utils.h"

#include "GameInstance.h"

namespace
{
	void Build_DefaultBreakableDesc(LD_BREAKABLE_OBJECT_DESC* pOutDesc)
	{
		if (nullptr == pOutDesc)
			return;

		*pOutDesc = {};

		pOutDesc->wstrSourcePath = L"Palette";
		pOutDesc->strSourceFile = L"H1W1.ysh";
		pOutDesc->strSection = L"Palette";
		pOutDesc->strEntryKey = L"StarBlock_Default";
		pOutDesc->strObjectName = L"StarBlock";
		pOutDesc->strKind = L"Palette";

		pOutDesc->eCategory = LD_CATEGORY::BREAKABLE;
		pOutDesc->eType = LD_BREAKABLE_TYPE::STAR_BLOCK;
		pOutDesc->wstrModelProtoTag = CLevelDesign_Breakable::STARBLOCK_H1W1_MODEL_PROTO_TAG;

		pOutDesc->fScale = 1.f;
		pOutDesc->vRight = { 1.f, 0.f, 0.f, 0.f };
		pOutDesc->vUp = { 0.f, 1.f, 0.f, 0.f };
		pOutDesc->vLook = { 0.f, 0.f, 1.f, 0.f };
		pOutDesc->vPosition = { 0.f, 0.f, 0.f, 1.f };

		pOutDesc->vParsedPosition = { 0.f, 0.f, 0.f };
		pOutDesc->qParsedRotation = { 0.f, 0.f, 0.f, 1.f };
		pOutDesc->vParsedScale = { 1.f, 1.f, 1.f };
	}
}

NS_BEGIN(Client)

CLevelDesign_Breakable::CLevelDesign_Breakable(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_Breakable::CLevelDesign_Breakable(const CLevelDesign_Breakable& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tBreakableDesc(Prototype.m_tBreakableDesc)
{
}

HRESULT CLevelDesign_Breakable::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLevelDesign_Breakable::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (nullptr == pArg)
	{
		LD_BREAKABLE_OBJECT_DESC DefaultDesc{};
		Build_DefaultBreakableDesc(&DefaultDesc);
		m_tBreakableDesc = DefaultDesc;
	}
	else
	{
		m_tBreakableDesc = *static_cast<const LD_BREAKABLE_OBJECT_DESC*>(pArg);
		if (FAILED(Validate_Desc()))
			return E_FAIL;
	}

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_PhysicsActor_Box()))
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_Breakable::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (nullptr != m_pModelCom)
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CLevelDesign_Breakable::Render()
{
	if (nullptr == m_pModelCom || nullptr == m_pShaderCom)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);

		auto BindMaterial = [&](const _char* pConstantName, MTEX_TYPE eType, DEFAULT_TEXTURE eDefaultKind) -> HRESULT
			{
				const _uint iLayerIndex = Layer.idx[ETOUI(eType)];
				const _uint iTextureCount = m_pModelCom->Get_MeshTextureCount(i, eType);

				if (0u < iTextureCount)
				{
					const _uint iSafeIndex = (iLayerIndex < iTextureCount) ? iLayerIndex : (iTextureCount - 1u);

					if (SUCCEEDED(m_pModelCom->Bind_Material(m_pShaderCom, pConstantName, i, eType, iSafeIndex)))
						return S_OK;
				}

				return m_pGameInstance_Proxy->Bind_DefaultTextureFromHub(m_pShaderCom, pConstantName, eDefaultKind);
			};

		if (FAILED(BindMaterial("g_DiffuseTexture", MTEX_TYPE::DIFFUSE, DEFAULT_TEXTURE::MAGENTA)))		return E_FAIL;
		if (FAILED(BindMaterial("g_NormalTexture", MTEX_TYPE::NORMALS, DEFAULT_TEXTURE::FLAT_NORMAL)))	return E_FAIL;
		if (FAILED(BindMaterial("g_MRATexture", MTEX_TYPE::METALNESS, DEFAULT_TEXTURE::MRA)))			return E_FAIL;
		if (FAILED(BindMaterial("g_UnknownTexture", MTEX_TYPE::UNKNOWN, DEFAULT_TEXTURE::BLACK)))		return E_FAIL;

		if (MODEL::ANIM == m_tBreakableDesc.eModelType)
		{
			if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Begin(0u)))
				return E_FAIL;
		}
		else
		{
			const _uint iUVIndex = (Layer.iUVIndex <= 3u) ? Layer.iUVIndex : 0u;
			_uint iFlags = Layer.iFlags;
			_float fDissolve = 0.f;

			if (FAILED(m_pShaderCom->Bind_RawValue(
				"g_iUVIndex", &iUVIndex, sizeof(_uint))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Bind_RawValue(
				"g_iEnvInstanceFlags", &iFlags, sizeof(_uint))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Bind_RawValue(
				"g_fDissolve", &fDissolve, sizeof(_float))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Begin(ShaderPass::NonAnimPBR::DMN)))
				return E_FAIL;
		}

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

void CLevelDesign_Breakable::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

LD_BREAKABLE_TYPE CLevelDesign_Breakable::Resolve_BreakableType(const _wstring& wstrObjName)
{
	static const pair<const _tchar*, LD_BREAKABLE_TYPE> Catalog[] =
	{
		{ L"StarBlock",					LD_BREAKABLE_TYPE::STAR_BLOCK },
		{ L"StarBlockBig",				LD_BREAKABLE_TYPE::STAR_BLOCK_BIG },
		{ L"WoodBox",					LD_BREAKABLE_TYPE::WOOD_BOX },
		{ L"BoxPlastic",				LD_BREAKABLE_TYPE::PLASTIC_BOX },
		{ L"BreakableRockS",			LD_BREAKABLE_TYPE::BREAKABLE_ROCK },
		{ L"BreakableRockM",			LD_BREAKABLE_TYPE::BREAKABLE_ROCK_BIG },
		{ L"BreakableRockMForBridge",	LD_BREAKABLE_TYPE::BREAKABLE_ROCK_BIG }
	};

	for (const auto& [pName, eType] : Catalog)
	{
		if (JsonUtils::Equals_NoCase(pName, wstrObjName.c_str()))
			return eType;
	}

	return LD_BREAKABLE_TYPE::UNKNOWN;
}

HRESULT CLevelDesign_Breakable::Validate_Desc()
{
	if (m_tBreakableDesc.eCategory != LD_CATEGORY::BREAKABLE)
		return E_FAIL;
	if (m_tBreakableDesc.eType == LD_BREAKABLE_TYPE::UNKNOWN)
		return E_FAIL;
	if (m_tBreakableDesc.wstrModelProtoTag.empty())
		return E_FAIL;

	return S_OK;
}
HRESULT CLevelDesign_Breakable::Ready_Components()
{
	const _tchar* pModelProtoTag = Resolve_ModelProtoTag();
	if (nullptr == pModelProtoTag)
		return E_FAIL;

	const auto& ShaderDesc = MODEL::ANIM == m_tBreakableDesc.eModelType
		? Shader_AnimMesh_PBR
		: Shader_NonAnimMesh_PBR;

	m_pShaderCom = Add_Component<CShader>(ShaderDesc.iLevelID, ShaderDesc.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(ETOUI(LEVEL::GAMEPLAY), pModelProtoTag, TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Breakable::Ready_PhysicsActor_Box()
{
	Release_PhysicsActor();

	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom || nullptr == m_pModelCom)
		return E_FAIL;

	_float3 vMin{};
	_float3 vMax{};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
		return E_FAIL;

	const _float3 vLocalCenter = {
			(vMin.x + vMax.x) * 0.5f,
			(vMin.y + vMax.y) * 0.5f,
			(vMin.z + vMax.z) * 0.5f
	};

	const _float3 vLocalHalfExtents = {
			(vMax.x - vMin.x) * 0.5f,
			(vMax.y - vMin.y) * 0.5f,
			(vMax.z - vMin.z) * 0.5f
	};

	m_pPhysicsActor = m_pGameInstance_Proxy->Create_StaticBox(
		vLocalCenter,
		vLocalHalfExtents,
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	if (nullptr == m_pPhysicsActor)
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_Breakable::Release_PhysicsActor()
{
	if (nullptr == m_pPhysicsActor)
		return;

	if (nullptr != m_pGameInstance_Proxy)
		m_pGameInstance_Proxy->Remove_StaticActor(m_pPhysicsActor);

	m_pPhysicsActor = nullptr;
}

HRESULT CLevelDesign_Breakable::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom || nullptr == m_pTransformCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	return S_OK;
}

const _tchar* CLevelDesign_Breakable::Resolve_ModelProtoTag() const
{
	if (m_tBreakableDesc.wstrModelProtoTag.empty())
		return nullptr;

	return m_tBreakableDesc.wstrModelProtoTag.c_str();
}

CLevelDesign_Breakable* CLevelDesign_Breakable::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelDesign_Breakable* pInstance = new CLevelDesign_Breakable(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLevelDesign_Breakable");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelDesign_Breakable::Clone(void* pArg)
{
	CLevelDesign_Breakable* pInstance = new CLevelDesign_Breakable(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLevelDesign_Breakable");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelDesign_Breakable::Free()
{
	Release_PhysicsActor();

	__super::Free();
}

NS_END