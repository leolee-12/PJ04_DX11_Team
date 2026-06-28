#include "LevelDesign_Bush.h"
#include "LevelDesign_Registry.h"
#include "MeshLayer_Binder.h"
#include "Parsing_Utils.h"

#include "GameInstance.h"

namespace
{
	struct LD_BUSH_CATALOG
	{
		const _tchar* pObjectName;

		const _tchar* pBasicModelProtoTag;
		const _char* pBasicModelPath;
		MODEL eBasicModelType;

		const _tchar* pCutModelProtoTag;
		const _char* pCutModelPath;
		MODEL eCutModelType;
	};

	static const LD_BUSH_CATALOG g_EventObjectCatalog[] =
	{
		{ L"Bush2BasicS", CLevelDesign_Bush::BUSH_S_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/BushS.ysh", MODEL::ANIM,
		CLevelDesign_Bush::CUT_S_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/CutS.ysh", MODEL::NONANIM },
		{ L"Bush2BasicM", CLevelDesign_Bush::BUSH_M_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/BushM.ysh", MODEL::ANIM,
		CLevelDesign_Bush::CUT_M_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/CutM.ysh", MODEL::NONANIM },
		{ L"Bush2BasicL", CLevelDesign_Bush::BUSH_L_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/BushL.ysh", MODEL::ANIM,
		CLevelDesign_Bush::CUT_L_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/CutL.ysh", MODEL::NONANIM }
	};

	static const LD_BUSH_CATALOG* Find_BushCatalog(const _wstring& wstrObjName)
	{
		for (const LD_BUSH_CATALOG& Entry : g_EventObjectCatalog)
		{
			if (JsonUtils::Equals_NoCase(Entry.pObjectName, wstrObjName.c_str()))
				return &Entry;
		}

		return nullptr;
	}

	void Normalize_BushUnknownTextureSlot(CModel* pModel)
	{
		if (nullptr == pModel)
			return;

		const _uint iNumMeshes = static_cast<_uint>(pModel->Get_NumMeshes());
		const _uint iUnknownType = ETOUI(MTEX_TYPE::UNKNOWN);

		for (_uint i = 0; i < iNumMeshes; ++i)
		{
			MESH_LAYER_IDX Layer = pModel->Get_MeshLayer(i);
			if (0u != Layer.idx[iUnknownType])
				continue;

			const _uint iTextureCount = pModel->Get_MeshTextureCount(i, MTEX_TYPE::UNKNOWN);
			if (iTextureCount <= 1u)
				continue;

			Layer.idx[iUnknownType] = (3u < iTextureCount) ? 3u : (iTextureCount - 1u);
			pModel->Set_MeshLayer(i, Layer);
		}
	}
}

NS_BEGIN(Client)

CLevelDesign_Bush::CLevelDesign_Bush(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_Bush::CLevelDesign_Bush(const CLevelDesign_Bush& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tBushDesc(Prototype.m_tBushDesc)
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

	m_pAnimatorCom->Play("Wait", true, true);
	return S_OK;
}

void CLevelDesign_Bush::Update(_float fTimeDelta)
{
	if (BUSH_STATE::BASIC == m_eState)
	{
		m_pAnimatorCom->Update(fTimeDelta);
	}
}

void CLevelDesign_Bush::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (nullptr != m_pModelComs[m_eState])
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CLevelDesign_Bush::Render()
{
	if (FAILED(Bind_ShaderResources(m_eState)))
		return E_FAIL;

	return Render_Model(m_eState);
}

void CLevelDesign_Bush::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLevelDesign_Bush::Register_LevelDesignSpecs()
{
	for (const LD_BUSH_CATALOG& Entry : g_EventObjectCatalog)
	{
		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = Entry.pObjectName;
		Spec.strPrototypeTag = PROTOTYPE_TAG;
		Spec.strLayerTag = L"Layer_LevelDesign_Gimmick";
		Spec.eCategory = LD_CATEGORY::FOLIAGE;
		Spec.pPrototypeFactory = &Create_Prototype;
		Spec.pBuildDesc = &Build_Desc;
		Spec.ModelRequirements =
		{
			{ Entry.pBasicModelProtoTag, Entry.pBasicModelPath, Entry.eBasicModelType },
			{ Entry.pCutModelProtoTag, Entry.pCutModelPath, Entry.eCutModelType }
		};

		CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
	}
}

_bool CLevelDesign_Bush::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	if (nullptr == pOutEntry)
		return false;

	const LD_BUSH_CATALOG* pCatalog = Find_BushCatalog(CommonDesc.strObjectName);
	if (nullptr == pCatalog)
		return false;

	LD_BUSH_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = LD_CATEGORY::FOLIAGE;
	Desc.wstrBasicProtoTag = pCatalog->pBasicModelProtoTag;
	Desc.wstrCutProtoTag = pCatalog->pCutModelProtoTag;
	Desc.eBasicType = pCatalog->eBasicModelType;
	Desc.eCutType = pCatalog->eCutModelType;
	JsonUtils::Try_ReadBoolFromNumeric(jEntry, "Gimmick.Bush2.MainComponent.IsGenerateItem", &Desc.bGenerateItem);

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLevelDesign_Bush::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLevelDesign_Bush::Create(pDevice, pContext);
}


HRESULT CLevelDesign_Bush::Validate_Desc()
{
	if (m_tBushDesc.eCategory != LD_CATEGORY::FOLIAGE)
		return E_FAIL;
	if (m_tBushDesc.wstrBasicProtoTag.empty())
		return E_FAIL;
	if (m_tBushDesc.wstrCutProtoTag.empty())
		return E_FAIL;
	if (MODEL::ANIM != m_tBushDesc.eBasicType)
		return E_FAIL;
	if (MODEL::NONANIM != m_tBushDesc.eCutType)
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Bush::Ready_Components()
{
	for (_uint i = 0; i < BUSH_STATE::_COUNT; ++i)
	{
		const BUSH_STATE eSlot = static_cast<BUSH_STATE>(i);
		const _tchar* pModelProtoTag = Resolve_ModelProtoTag(eSlot);
		if (nullptr == pModelProtoTag)
			return E_FAIL;

		const MODEL eModelType = Resolve_ModelType(eSlot);
		const auto& ShaderDesc = MODEL::ANIM == eModelType
			? Shader_AnimMesh_PBR
			: Shader_NonAnimMesh_PBR;

		_tchar szShaderTag[32] = {};
		_tchar szModelTag[32] = {};

		if (BUSH_STATE::BASIC == eSlot)
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

		m_pModelComs[eSlot] = Add_Component<CModel>(m_tBushDesc.iModelProtoLevel, pModelProtoTag, szModelTag);
		if (nullptr == m_pModelComs[eSlot])
			return E_FAIL;

		Normalize_BushUnknownTextureSlot(m_pModelComs[eSlot]);
	}

	CAnimator::ANIMATOR_DESC AnimDesc{};
	AnimDesc.pModel = m_pModelComs[BASIC];

	m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
	if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Bush::Bind_ShaderResources(BUSH_STATE eSlot)
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

HRESULT CLevelDesign_Bush::Render_Model(BUSH_STATE eSlot)
{
	CModel* pModel = m_pModelComs[eSlot];
	CShader* pShader = m_pShaderComs[eSlot];
	const MODEL eModelType = Resolve_ModelType(eSlot);

	const _uint iNumMeshes = static_cast<_uint>(pModel->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX& Layer = pModel->Get_MeshLayer(i);

		if (MODEL::ANIM == eModelType)
		{
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

			if (FAILED(BindMaterial("g_NormalTexture", MTEX_TYPE::NORMALS, DEFAULT_TEXTURE::FLAT_NORMAL)))	return E_FAIL;
			if (FAILED(BindMaterial("g_UnknownTexture", MTEX_TYPE::UNKNOWN, DEFAULT_TEXTURE::BLACK)))		return E_FAIL;

				if (FAILED(pModel->Bind_BoneMatrices(pShader, "g_BoneMatrices", i)))
					return E_FAIL;

			const _uint iBushPass = 4u;

			if (FAILED(pShader->Begin(iBushPass)))
				return E_FAIL;
		}
		else
		{
			MESH_LAYER_BIND_CONTEXT Ctx{};
			Ctx.pShader = pShader;
			Ctx.pModel = pModel;
			Ctx.pGI_Proxy = m_pGameInstance_Proxy;
			Ctx.iMesh = i;
			Ctx.pLayer = &Layer;
			Ctx.eProfile = MESH_LAYER_PROFILE::NONANIM_PBR;
			Ctx.eKind = MESH_LAYER_RENDER_KIND::MAIN;
			Ctx.iFallbackPass = ShaderPass::NonAnimPBR::UMN;
			Ctx.fDissolve = 0.f;

			MESH_LAYER_BIND_RESULT Result{};
			if (FAILED(MeshLayerBinder::Bind(Ctx, &Result)))
				return E_FAIL;

			if (Result.bSkipMesh)
				continue;

			if (FAILED(pShader->Begin(Result.iPass)))
				return E_FAIL;
		}

		if (FAILED(pModel->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

const _tchar* CLevelDesign_Bush::Resolve_ModelProtoTag(BUSH_STATE eSlot) const
{
	if (BUSH_STATE::BASIC == eSlot)
		return m_tBushDesc.wstrBasicProtoTag.empty() ? nullptr : m_tBushDesc.wstrBasicProtoTag.c_str();

	if (BUSH_STATE::CUT == eSlot)
		return m_tBushDesc.wstrCutProtoTag.empty() ? nullptr : m_tBushDesc.wstrCutProtoTag.c_str();

	return nullptr;
}

MODEL CLevelDesign_Bush::Resolve_ModelType(BUSH_STATE eSlot) const
{
	return BUSH_STATE::BASIC == eSlot ? m_tBushDesc.eBasicType : m_tBushDesc.eCutType;
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