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

	m_tBushDesc = *static_cast<const LD_BUSH_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Bush::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

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

	const LD_BUSH_CATALOG* pCatalog = Find_BushCatalog(m_tBushDesc.strObjectName);
	if (nullptr == pCatalog)
		return E_FAIL;
	if (nullptr == pCatalog->pBasicModelProtoTag || m_tBushDesc.wstrBasicProtoTag !=
		pCatalog->pBasicModelProtoTag)
		return E_FAIL;
	if (nullptr == pCatalog->pCutModelProtoTag || m_tBushDesc.wstrCutProtoTag !=
		pCatalog->pCutModelProtoTag)
		return E_FAIL;
	if (m_tBushDesc.eBasicType != pCatalog->eBasicModelType || m_tBushDesc.eCutType !=
		pCatalog->eCutModelType)
		return E_FAIL;

	const _uint iState = static_cast<_uint>(m_eState);
	if (iState >= BUSH_STATE::_COUNT)
		return E_FAIL;

	for (_uint i = 0; i < BUSH_STATE::_COUNT; ++i)
	{
		if (nullptr == m_pShaderComs[i] || nullptr == m_pModelComs[i])
			return E_FAIL;
	}

	if (nullptr == m_pAnimatorCom || nullptr == m_pHurtBoxCom)
		return E_FAIL;

	_bool bHasWaitAnim = false;
	const _uint iNumAnimations = m_pModelComs[BUSH_STATE::BASIC]->Get_NumAnimations();
	for (_uint i = 0; i < iNumAnimations; ++i)
	{
		if (m_pModelComs[BUSH_STATE::BASIC]->Get_AnimationName(i) == "Wait")
		{
			bHasWaitAnim = true;
			break;
		}
	}

	if (!bHasWaitAnim)
		return E_FAIL;

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

	if (BUSH_STATE::BASIC == m_eState && m_pHurtBoxCom->Is_Enabled())
	{
		m_pHurtBoxCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pHurtBoxCom);
#endif
	}

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

void CLevelDesign_Bush::Damaged(const ATTACK_INFO& tInfo)
{
	UNREFERENCED_PARAMETER(tInfo);

	if (BUSH_STATE::CUT == m_eState)
		return;

	m_eState = BUSH_STATE::CUT;
	m_pHurtBoxCom->Set_Enabled(false);
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

void CLevelDesign_Bush::Collect_EditModelSlots(vector<EDITABLE_MODEL_SLOT>* pOutSlots) const
{
	auto AddSlot = [&](BUSH_STATE eSlot, const _tchar* pLabel)
		{
			const MODEL eModelType = Resolve_ModelType(eSlot);
			const EDITABLE_MODEL_KIND eKind = MODEL::ANIM == eModelType
				? EDITABLE_MODEL_KIND::ANIM
				: EDITABLE_MODEL_KIND::NONANIM;
			Add_EditModelSlot(pOutSlots, pLabel, eKind, m_pModelComs[eSlot]);
		};

	AddSlot(BUSH_STATE::BASIC, TEXT("Basic"));
	AddSlot(BUSH_STATE::CUT, TEXT("Cut"));
}

HRESULT CLevelDesign_Bush::Ready_Components()
{
	if (FAILED(Ready_RenderComponents()))
		return E_FAIL;

	if (FAILED(Ready_HurtBox()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Bush::Ready_RenderComponents()
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
	}

	CAnimator::ANIMATOR_DESC AnimDesc{};
	AnimDesc.pModel = m_pModelComs[BASIC];

	m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice,
		m_pContext));
	if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
		return E_FAIL;

	m_pAnimatorCom->Play("Wait", true, true);

	return S_OK;
}

HRESULT CLevelDesign_Bush::Ready_HurtBox()
{
	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pModelComs[BUSH_STATE::BASIC])
		return E_FAIL;

	_float3 vMin{};
	_float3 vMax{};
	m_pModelComs[BUSH_STATE::BASIC]->Get_ModelAABB(&vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
		return E_FAIL;

	const _float3 vSize = { vMax.x - vMin.x, vMax.y - vMin.y, vMax.z - vMin.z };

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = { (vMin.x + vMax.x) * 0.5f, (vMin.y + vMax.y) * 0.5f, (vMin.z + vMax.z) * 0.5f };
	ColliderDesc.fRadius = max(max(vSize.x, vSize.y), vSize.z) * 0.5f;

	m_pHurtBoxCom = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag, TEXT("Com_HurtBox"),
		&ColliderDesc);
	if (nullptr == m_pHurtBoxCom)
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pHurtBoxCom, ETOUI(COLLISION_LAYER::ENV_HURT));

	return S_OK;
}

HRESULT CLevelDesign_Bush::Bind_ShaderResources(BUSH_STATE eSlot)
{
	CShader* pShader = m_pShaderComs[eSlot];

	if (FAILED(m_pTransformCom->Bind_ShaderResource(pShader, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	if (FAILED(pShader->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;
	
	const _float4 vEmissiveColor = { 0.f, 0.f, 0.f, 0.f };
	if (FAILED(pShader->Bind_RawValue("g_vEmissiveColor", &vEmissiveColor, sizeof(_float4))))
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