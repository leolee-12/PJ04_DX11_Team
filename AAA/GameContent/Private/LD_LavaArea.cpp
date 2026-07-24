#include "LD_LavaArea.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"
#include "MeshLayer_Binder.h"

#include "GameInstance.h"

NS_BEGIN(Client)

namespace
{
	constexpr const _char* LAVA_MODEL_PATH = "../../Resources/Map/Gimmick/NonAnim/Lava/Lava.ysh";
}

CLD_LavaArea::CLD_LavaArea(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLD_LavaArea::CLD_LavaArea(const CLD_LavaArea& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tSurfaceAreaDesc(Prototype.m_tSurfaceAreaDesc)
{
}

HRESULT CLD_LavaArea::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLD_LavaArea::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	m_tSurfaceAreaDesc = *static_cast<const LD_SURFACE_AREA_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_RenderComponents()))
		return E_FAIL;

	if (FAILED(Ready_CullingState(m_pModelCom)))
		return E_FAIL;

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_LavaArea::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, m_tSurfaceAreaDesc.strObjectName.c_str()))
		return E_FAIL;

	if (LD_CATEGORY::VOLUME != m_tSurfaceAreaDesc.eCategory)
		return E_FAIL;

	if (MODEL::NONANIM != m_tSurfaceAreaDesc.eModelType)
		return E_FAIL;

	if (m_tSurfaceAreaDesc.wstrModelProtoTag != MODEL_PROTO_TAG)
		return E_FAIL;

	if (nullptr == m_pShaderCom || nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
}

void CLD_LavaArea::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (!m_bActive || Is_Dead())
		return;

	Check_Visible();
	Submit_RenderGroups();
}

HRESULT CLD_LavaArea::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	return Render_Model();
}

void CLD_LavaArea::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_LavaArea::Register_LevelDesignSpecs()
{
	LD_SPAWN_SPEC Spec{};
	Spec.strObjectName = OBJECT_NAME;
	Spec.strPrototypeTag = PROTOTYPE_TAG;
	Spec.strLayerTag = LAYER_TAG;
	Spec.eCategory = LD_CATEGORY::VOLUME;
	Spec.wstrModelProtoTag = MODEL_PROTO_TAG;
	Spec.eModelType = MODEL::NONANIM;
	Spec.pPrototypeFactory = &Create_Prototype;
	Spec.pBuildDesc = &Build_Desc;
	Spec.ModelRequirements =
	{
			{ MODEL_PROTO_TAG, LAVA_MODEL_PATH, MODEL::NONANIM, false }
	};

	CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

_bool CLD_LavaArea::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, CommonDesc.strObjectName.c_str()))
		return false;

	if (!JsonUtils::Equals_NoCase(Spec.strObjectName.c_str(), CommonDesc.strObjectName.c_str()))
		return false;

	if (Spec.strPrototypeTag != PROTOTYPE_TAG || Spec.strLayerTag != LAYER_TAG)
		return false;

	if (LD_CATEGORY::VOLUME != Spec.eCategory || MODEL::NONANIM != Spec.eModelType || Spec.wstrModelProtoTag != MODEL_PROTO_TAG)
		return false;

	LD_SURFACE_AREA_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.eModelType = Spec.eModelType;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;

	*pOutEntry = std::move(Desc);
	return true;
}

CGameObject* CLD_LavaArea::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_LavaArea::Create(pDevice, pContext);
}

HRESULT CLD_LavaArea::Ready_RenderComponents()
{
	if (m_tSurfaceAreaDesc.wstrModelProtoTag.empty())
		return E_FAIL;

	m_pShaderCom = Add_Component<CShader>(Shader_World_NonAnim.iLevelID, Shader_World_NonAnim.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_tSurfaceAreaDesc.iModelProtoLevel, m_tSurfaceAreaDesc.wstrModelProtoTag.c_str(), TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_LavaArea::Bind_ShaderResources()
{
	if (FAILED(MeshLayerBinder::Bind_WorldViewProj(m_pShaderCom, m_pTransformCom, m_pGameInstance_Proxy, m_eProjType)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_LavaArea::Render_Model()
{
	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);

		MESH_LAYER_BIND_CONTEXT Ctx{};
		Ctx.Set_Renderer(m_pShaderCom, m_pModelCom, m_pGameInstance_Proxy, m_pCullingState);
		Ctx.iMesh = i;
		Ctx.pLayer = &Layer;
		Ctx.eProfile = MESH_LAYER_PROFILE::WORLD_NONANIM;
		Ctx.eKind = MESH_LAYER_RENDER_KIND::MAIN;
		Ctx.iFallbackPass = ETOUI(WORLD_PASS::DMN);

		_uint iPass = 0u;
		const HRESULT hrBind = MeshLayerBinder::Bind_OrSkip(Ctx, &iPass);
		if (FAILED(hrBind))     return E_FAIL;
		if (S_FALSE == hrBind)  continue;

		if (FAILED(m_pShaderCom->Begin(iPass)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

CLD_LavaArea* CLD_LavaArea::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_LavaArea* pInstance = new CLD_LavaArea(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_LavaArea");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_LavaArea::Clone(void* pArg)
{
	CLD_LavaArea* pInstance = new CLD_LavaArea(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_LavaArea");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_LavaArea::Free()
{
	__super::Free();
}

NS_END