#include "LevelDesign_Bush.h"
#include "Shader_PassMeta.h"
#include "Parsing_Utils.h"

#include "GameInstance.h"

NS_BEGIN(Client)

CLevelDesign_Bush::CLevelDesign_Bush(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_Bush::CLevelDesign_Bush(const CLevelDesign_Bush& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tBushDesc(Prototype.m_tBushDesc)
	, m_iModelProtoLevel(Prototype.m_iModelProtoLevel)
{
}

HRESULT CLevelDesign_Bush::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLevelDesign_Bush::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_tBushDesc = *static_cast<const LD_BUSH_DESC*>(pArg);

	if (FAILED(Validate_Desc()))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_Bush::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (nullptr != m_pModelComs[m_eRenderSlot])
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CLevelDesign_Bush::Render()
{
	if (nullptr == m_pModelComs[m_eRenderSlot] || nullptr == m_pShaderComs[m_eRenderSlot])
		return S_OK;

	if (FAILED(Bind_ShaderResources(m_eRenderSlot)))
		return E_FAIL;

	return Render_Model(m_eRenderSlot);
}

void CLevelDesign_Bush::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

LD_BUSH_TYPE CLevelDesign_Bush::Resolve_BushType(const _wstring& wstrObjName)
{
	static const pair<const _tchar*, LD_BUSH_TYPE> Catalog[] =
	{
			{ L"Bush2BasicS", LD_BUSH_TYPE::BUSH_S },
			{ L"Bush2BasicM", LD_BUSH_TYPE::BUSH_M },
			{ L"Bush2BasicL", LD_BUSH_TYPE::BUSH_L }
	};

	for (const auto& [pName, eType] : Catalog)
	{
		if (JsonUtils::Equals_NoCase(pName, wstrObjName.c_str()))
			return eType;
	}

	return LD_BUSH_TYPE::UNKNOWN;
}

HRESULT CLevelDesign_Bush::Validate_Desc()
{
	if (LD_BUSH_TYPE::UNKNOWN == m_tBushDesc.eType)
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Bush::Ready_Components()
{
	for (_uint i = 0; i < MODEL_SLOT::_COUNT; ++i)
	{
		const MODEL_SLOT eSlot = static_cast<MODEL_SLOT>(i);
		const _tchar* pModelProtoTag = Resolve_ModelProtoTag(eSlot);
		if (nullptr == pModelProtoTag)
			return E_FAIL;

		const MODEL eModelType = Resolve_ModelType(eSlot);
		const auto& ShaderDesc = MODEL::ANIM == eModelType
			? Shader_AnimMesh_PBR
			: Shader_NonAnimMesh_PBR;

		_tchar szShaderTag[32] = {};
		_tchar szModelTag[32] = {};

		if (MODEL_SLOT::BASIC == eSlot)
		{
			lstrcpy(szShaderTag, TEXT("Com_Shader_Basic"));
			lstrcpy(szModelTag, TEXT("Com_Model_Basic"));
		}
		else
		{
			lstrcpy(szShaderTag, TEXT("Com_Shader_Cut"));
			lstrcpy(szModelTag, TEXT("Com_Model_Cut"));
		}

		m_pShaderComs[eSlot] = Add_Component<CShader>(ShaderDesc.iLevelID, ShaderDesc.szProtoTag, szShaderTag);
		if (nullptr == m_pShaderComs[eSlot])
			return E_FAIL;

		m_pModelComs[eSlot] = Add_Component<CModel>(m_iModelProtoLevel, pModelProtoTag, szModelTag);
		if (nullptr == m_pModelComs[eSlot])
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevelDesign_Bush::Bind_ShaderResources(MODEL_SLOT eSlot)
{
	CShader* pShader = m_pShaderComs[eSlot];
	if (nullptr == pShader || nullptr == m_pTransformCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(pShader, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Bush::Render_Model(MODEL_SLOT eSlot)
{
	CModel* pModel = m_pModelComs[eSlot];
	CShader* pShader = m_pShaderComs[eSlot];
	const MODEL eModelType = Resolve_ModelType(eSlot);

	const _uint iNumMeshes = static_cast<_uint>(pModel->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX& Layer = pModel->Get_MeshLayer(i);

		auto BindMaterial = [&](const _char* pConstantName, MTEX_TYPE eType, DEFAULT_TEXTURE eDefaultKind) -> HRESULT
			{
				const _uint iLayerIndex = Layer.idx[ETOUI(eType)];
				const _uint iTextureCount = pModel->Get_MeshTextureCount(i, eType);

				if (0u < iTextureCount)
				{
					const _uint iSafeIndex = (iLayerIndex < iTextureCount) ? iLayerIndex : (iTextureCount - 1u);

					if (SUCCEEDED(pModel->Bind_Material(pShader, pConstantName, i, eType, iSafeIndex)))
						return S_OK;
				}

				return m_pGameInstance_Proxy->Bind_DefaultTextureFromHub(pShader, pConstantName, eDefaultKind);
			};

		if (FAILED(BindMaterial("g_DiffuseTexture", MTEX_TYPE::DIFFUSE, DEFAULT_TEXTURE::MAGENTA)))             return E_FAIL;
		if (FAILED(BindMaterial("g_NormalTexture", MTEX_TYPE::NORMALS, DEFAULT_TEXTURE::FLAT_NORMAL)))  return E_FAIL;
		if (FAILED(BindMaterial("g_MRATexture", MTEX_TYPE::METALNESS, DEFAULT_TEXTURE::MRA)))                   return E_FAIL;
		if (FAILED(BindMaterial("g_UnknownTexture", MTEX_TYPE::UNKNOWN, DEFAULT_TEXTURE::BLACK)))               return E_FAIL;

		if (MODEL::ANIM == eModelType)
		{
			if (FAILED(pModel->Bind_BoneMatrices(pShader, "g_BoneMatrices", i)))
				return E_FAIL;

			if (FAILED(pShader->Begin(0u)))
				return E_FAIL;
		}
		else
		{
			const _uint iUVIndex = (Layer.iUVIndex <= 3u) ? Layer.iUVIndex : 0u;
			_uint iFlags = Layer.iFlags;
			_float fDissolve = 0.f;

			if (FAILED(pShader->Bind_RawValue("g_iUVIndex", &iUVIndex, sizeof(_uint))))                     return E_FAIL;
			if (FAILED(pShader->Bind_RawValue("g_iEnvInstanceFlags", &iFlags, sizeof(_uint))))      return E_FAIL;
			if (FAILED(pShader->Bind_RawValue("g_fDissolve", &fDissolve, sizeof(_float))))          return E_FAIL;

			if (FAILED(pShader->Begin(ShaderPass::NonAnimPBR::DMN)))
				return E_FAIL;
		}

		if (FAILED(pModel->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

const _tchar* CLevelDesign_Bush::Resolve_ModelProtoTag(MODEL_SLOT eSlot) const
{
	switch (m_tBushDesc.eType)
	{
	case LD_BUSH_TYPE::BUSH_S:	return MODEL_SLOT::BASIC == eSlot ? BUSH_S_MODEL_PROTO_TAG : CUT_S_MODEL_PROTO_TAG;
	case LD_BUSH_TYPE::BUSH_M:	return MODEL_SLOT::BASIC == eSlot ? BUSH_M_MODEL_PROTO_TAG : CUT_M_MODEL_PROTO_TAG;
	case LD_BUSH_TYPE::BUSH_L:	return MODEL_SLOT::BASIC == eSlot ? BUSH_L_MODEL_PROTO_TAG : CUT_L_MODEL_PROTO_TAG;
	default:					return nullptr;
	}
}

MODEL CLevelDesign_Bush::Resolve_ModelType(MODEL_SLOT eSlot) const
{
	return MODEL_SLOT::BASIC == eSlot ? MODEL::ANIM : MODEL::NONANIM;
}

CLevelDesign_Bush* CLevelDesign_Bush::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelDesign_Bush* pInstance = new CLevelDesign_Bush(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLevelDesign_Bush");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelDesign_Bush::Clone(void* pArg)
{
	CLevelDesign_Bush* pInstance = new CLevelDesign_Bush(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLevelDesign_Bush");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelDesign_Bush::Free()
{
	__super::Free();
}

NS_END