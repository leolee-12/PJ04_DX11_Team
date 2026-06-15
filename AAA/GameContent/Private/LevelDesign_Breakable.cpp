#include "LevelDesign_Breakable.h"
#include "GameContent_const.h"

#include "GameInstance_Proxy.h"
#include "Model.h"

namespace
{
	LD_BREAKABLE_TYPE Resolve_BreakableType(const _wstring& strObjectName)
	{
		if (0 == _wcsicmp(strObjectName.c_str(), L"StarBlock"))		return Client::LD_BREAKABLE_TYPE::STAR_BLOCK;
		if (0 == _wcsicmp(strObjectName.c_str(), L"StarBlockBig"))	return Client::LD_BREAKABLE_TYPE::STAR_BLOCK_BIG;
		if (0 == _wcsicmp(strObjectName.c_str(), L"WoodBox"))		return Client::LD_BREAKABLE_TYPE::WOOD_BOX;

		return Client::LD_BREAKABLE_TYPE::UNKNOWN;
	}

	void Build_DefaultBreakableDesc(Client::LD_BREAKABLE_OBJECT_DESC* pOutDesc)
	{
		if (nullptr == pOutDesc)
			return;

		*pOutDesc = {};

		pOutDesc->strSourcePath = L"Palette";
		pOutDesc->strSourceFile = L"H1W1.ysh";
		pOutDesc->strSection = L"Palette";
		pOutDesc->strEntryKey = L"StarBlock_Default";
		pOutDesc->strObjectName = L"StarBlock";
		pOutDesc->strKind = L"Palette";

		pOutDesc->eCategory = LD_CATEGORY::BREAKABLE;
		pOutDesc->eBreakableType = LD_BREAKABLE_TYPE::STAR_BLOCK;

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
	LD_BREAKABLE_OBJECT_DESC DefaultDesc{};

	if (nullptr != pArg)
	{
		m_tBreakableDesc = *static_cast<const LD_BREAKABLE_OBJECT_DESC*>(pArg);
	}
	else
	{
		Build_DefaultBreakableDesc(&DefaultDesc);
		m_tBreakableDesc = DefaultDesc;
	}

	m_tBreakableDesc.eCategory = LD_CATEGORY::BREAKABLE;

	if (m_tBreakableDesc.eBreakableType == LD_BREAKABLE_TYPE::UNKNOWN)
		m_tBreakableDesc.eBreakableType = Resolve_BreakableType(m_tBreakableDesc.strObjectName);

	if (FAILED(__super::Initialize(&m_tBreakableDesc)))
		return E_FAIL;

	Desc().eCategory = LD_CATEGORY::BREAKABLE;

	if (FAILED(Ready_Components()))
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

		const _uint iUVIndex = (Layer.iUVIndex <= 3u) ? Layer.iUVIndex : 0u;
		_uint iFlags = Layer.iFlags;
		_float fDissolve = 0.f;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_iUVIndex", &iUVIndex, sizeof(_uint))))			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_iEnvInstanceFlags", &iFlags, sizeof(_uint))))		return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolve", &fDissolve, sizeof(_float))))			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(ShaderPass::NonAnimPBR::DMN)))
			return E_FAIL;
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

HRESULT CLevelDesign_Breakable::Ready_Components()
{
	const _tchar* pModelProtoTag = Resolve_ModelProtoTag();
	if (nullptr == pModelProtoTag)
		return S_OK;

	m_pShaderCom = Add_Component<CShader>(Shader_NonAnimMesh_PBR.iLevelID, Shader_NonAnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(ETOUI(LEVEL::GAMEPLAY), pModelProtoTag, TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
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
	switch (m_tBreakableDesc.eBreakableType)
	{
	case LD_BREAKABLE_TYPE::STAR_BLOCK:	return STARBLOCK_MODEL_PROTO_TAG;

	default:	return nullptr;
	}
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
	__super::Free();
}

NS_END