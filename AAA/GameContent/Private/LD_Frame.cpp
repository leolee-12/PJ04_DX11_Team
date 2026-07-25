#include "LD_Frame.h"
#include "LevelDesign_Registry.h"
#include "MeshLayer_Binder.h"
#include "Parsing_Utils.h"
#include "GameContent_const.h"

#include "Model.h"
#include "GameInstance.h"

NS_BEGIN(Client)

CLD_Frame::CLD_Frame(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLD_Frame::CLD_Frame(const CLD_Frame& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tStaticModelDesc(Prototype.m_tStaticModelDesc)
{
}

HRESULT CLD_Frame::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLD_Frame::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	m_tStaticModelDesc = *static_cast<const LD_STATIC_MODEL_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_RenderComponents()))
		return E_FAIL;

	if (FAILED(Ready_CullingState(m_pModelCom)))
		return E_FAIL;

	return Validate_Initialized();
}

HRESULT CLD_Frame::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, m_tStaticModelDesc.strObjectName.c_str()))
		return E_FAIL;

	if (LD_CATEGORY::GIMMICK != m_tStaticModelDesc.eCategory)
		return E_FAIL;

	if (m_tStaticModelDesc.wstrModelProtoTag != MODEL_PROTO_TAG)
		return E_FAIL;

	if (m_tStaticModelDesc.bUseCollMesh)
		return E_FAIL;

	if (nullptr == m_pShaderCom || nullptr == m_pModelCom || 0u == m_pModelCom->Get_NumMeshes())
		return E_FAIL;

	return S_OK;
}

void CLD_Frame::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (!m_bActive || Is_Dead())
		return;

	Check_Visible();
	Submit_RenderGroups();
}

HRESULT CLD_Frame::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	return Render_Model();
}

void CLD_Frame::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_Frame::Register_LevelDesignSpecs()
{
	LD_SPAWN_SPEC Spec{};
	Spec.strObjectName = OBJECT_NAME;
	Spec.strPrototypeTag = PROTOTYPE_TAG;
	Spec.strLayerTag = LAYER_TAG;
	Spec.eCategory = LD_CATEGORY::GIMMICK;
	Spec.wstrModelProtoTag = MODEL_PROTO_TAG;
	Spec.eModelType = MODEL::NONANIM;
	Spec.pPrototypeFactory = &Create_Prototype;
	Spec.pBuildDesc = &Build_Desc;
	Spec.ModelRequirements =
	{
			{ MODEL_PROTO_TAG, MODEL_PATH, MODEL::NONANIM, false },
	};

	CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

_bool CLD_Frame::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY*
	pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, CommonDesc.strObjectName.c_str()))
		return false;

	if (Spec.strPrototypeTag != PROTOTYPE_TAG || Spec.strLayerTag != LAYER_TAG)
		return false;

	if (Spec.eCategory != LD_CATEGORY::GIMMICK
		|| Spec.eModelType != MODEL::NONANIM
		|| Spec.wstrModelProtoTag != MODEL_PROTO_TAG)
	{
		return false;
	}

	LD_STATIC_MODEL_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;
	Desc.bUseCollMesh = false;

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLD_Frame::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_Frame::Create(pDevice, pContext);
}

HRESULT CLD_Frame::Ready_RenderComponents()
{
	if (m_tStaticModelDesc.wstrModelProtoTag.empty())
		return E_FAIL;

	m_pShaderCom = Add_Component<CShader>(Shader_World_NonAnim.iLevelID, Shader_World_NonAnim.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(
		m_tStaticModelDesc.iModelProtoLevel,
		m_tStaticModelDesc.wstrModelProtoTag,
		TEXT("Com_Model"));

	if (nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_Frame::Bind_ShaderResources()
{
	if (FAILED(MeshLayerBinder::Bind_WorldViewProj(m_pShaderCom, m_pTransformCom, m_pGameInstance_Proxy, m_eProjType)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_Frame::Render_Model()
{
	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);
		const _bool bUseUnknownPass =
			0u == m_pModelCom->Get_MeshTextureCount(i, MTEX_TYPE::DIFFUSE)
			&& 0u < m_pModelCom->Get_MeshTextureCount(i, MTEX_TYPE::UNKNOWN);

		MESH_LAYER_BIND_CONTEXT Ctx{};
		Ctx.Set_Renderer(m_pShaderCom, m_pModelCom, m_pGameInstance_Proxy, m_pCullingState);
		Ctx.iMesh = i;
		Ctx.pLayer = &Layer;
		Ctx.eProfile = MESH_LAYER_PROFILE::WORLD_NONANIM;
		Ctx.eKind = MESH_LAYER_RENDER_KIND::MAIN;
		Ctx.iFallbackPass = bUseUnknownPass ? ETOUI(WORLD_PASS::UKWN) : ETOUI(WORLD_PASS::DMN);

		_uint iPass = 0u;
		const HRESULT hrBind = MeshLayerBinder::Bind_OrSkip(Ctx, &iPass);
		if (FAILED(hrBind))
			return E_FAIL;
		if (S_FALSE == hrBind)
			continue;

		if (FAILED(m_pShaderCom->Begin(iPass)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

CLD_Frame* CLD_Frame::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_Frame* pInstance = new CLD_Frame(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_Frame");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_Frame::Clone(void* pArg)
{
	CLD_Frame* pInstance = new CLD_Frame(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_Frame");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_Frame::Free()
{
	__super::Free();
}

NS_END