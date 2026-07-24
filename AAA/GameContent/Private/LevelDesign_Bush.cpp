#include "LevelDesign_Bush.h"
#include "LevelDesign_Registry.h"
#include "MeshLayer_Binder.h"
#include "Parsing_Utils.h"
#include "Effect_Loader.h"

#include "GameInstance.h"
#include "Geometry_Utils.h"

namespace
{
	static constexpr const _char* ANIM_WAIT = "Wait";
	static constexpr const _char* ANIM_SHAKE_ONCE = "ShakeOnce";
	static constexpr const _char* ANIM_SHAKE_LOOP = "ShakeLoop";
	static constexpr _float BUSH_CULL_MARGIN = 0.5f;
	static constexpr _float DESTROY_EFFECT_HEIGHT_RATIO = 0.55f;

	struct LD_BUSH_CATALOG
	{
		const _tchar*	pObjectName;

		const _tchar*	pBasicModelProtoTag;
		const _char*	pBasicModelPath;
		MODEL			eBasicModelType;

		const _tchar*	pCutModelProtoTag;
		const _char*	pCutModelPath;
		MODEL			eCutModelType;
	};

	static const LD_BUSH_CATALOG g_BushCatalog[] =
	{
		{ L"Bush2BasicS", CLevelDesign_Bush::BUSH_S_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/BushS.ysh", MODEL::ANIM,	CLevelDesign_Bush::CUT_S_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/CutS.ysh", MODEL::NONANIM },
		{ L"Bush2BasicM", CLevelDesign_Bush::BUSH_M_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/BushM.ysh", MODEL::ANIM,	CLevelDesign_Bush::CUT_M_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/CutM.ysh", MODEL::NONANIM },
		{ L"Bush2BasicL", CLevelDesign_Bush::BUSH_L_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/BushL.ysh", MODEL::ANIM,	CLevelDesign_Bush::CUT_L_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/CutL.ysh", MODEL::NONANIM }
	};

	static const LD_BUSH_CATALOG* Find_BushCatalog(const _wstring& wstrObjName)
	{
		for (const LD_BUSH_CATALOG& Entry : g_BushCatalog)
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

	if (FAILED(Ready_BushCullBounds()))
		return E_FAIL;

	m_bUseShadow = true;

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

	if (nullptr == m_pAnimatorCom || nullptr == m_pHurtBox)
		return E_FAIL;

	_bool bHasWaitAnim = false;
	_bool bHasShakeOnceAnim = false;
	_bool bHasShakeLoopAnim = false;
	const _uint iNumAnimations = m_pModelComs[BUSH_STATE::BASIC]->Get_NumAnimations();
	for (_uint i = 0; i < iNumAnimations; ++i)
	{
		const _string& strAnimName = m_pModelComs[BUSH_STATE::BASIC]->Get_AnimationName(i);
		if (strAnimName == ANIM_WAIT)
			bHasWaitAnim = true;
		else if (strAnimName == ANIM_SHAKE_ONCE)
			bHasShakeOnceAnim = true;
		else if (strAnimName == ANIM_SHAKE_LOOP)
			bHasShakeLoopAnim = true;

		if (bHasWaitAnim && bHasShakeOnceAnim && bHasShakeLoopAnim)
			break;
	}

	if (!bHasWaitAnim || !bHasShakeOnceAnim || !bHasShakeLoopAnim)
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_Bush::Update(_float fTimeDelta)
{
	if (BUSH_STATE::BASIC != m_eState)
		return;

	m_pAnimatorCom->Update(fTimeDelta);

	const _string& strCurrentAnimName = m_pAnimatorCom->Get_CurrentAnimName();
	if (strCurrentAnimName == ANIM_SHAKE_ONCE)
	{
		if (!m_pAnimatorCom->Is_Finished())
			return;

		if (m_bInhaleOverlapping)
			m_pAnimatorCom->Play(ANIM_SHAKE_LOOP, false, true);
		else
			m_pAnimatorCom->Play(ANIM_WAIT, true, true);

		return;
	}

	if (strCurrentAnimName == ANIM_SHAKE_LOOP)
	{
		if (!m_pAnimatorCom->Is_Finished())
			return;

		if (m_bInhaleOverlapping)
			m_pAnimatorCom->Play(ANIM_SHAKE_LOOP, false, true);
		else
			m_pAnimatorCom->Play(ANIM_WAIT, true, true);
	}
}

void CLevelDesign_Bush::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (BUSH_STATE::BASIC == m_eState && m_pHurtBox->Is_Enabled())
	{
		m_pHurtBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pHurtBox);
#endif
	}

	Check_Visible();
	Submit_RenderGroups();
}

HRESULT CLevelDesign_Bush::Render()
{
	if (FAILED(Bind_ShaderResources(m_eState)))
		return E_FAIL;

	return Render_Model(m_eState);
}

HRESULT CLevelDesign_Bush::Render_Shadow()
{
	const BUSH_STATE eSlot = m_eState;
	CShader* pShader = m_pShaderComs[eSlot];
	CModel* pModel = m_pModelComs[eSlot];
	const MODEL eModelType = Resolve_ModelType(eSlot);

	if (MODEL::NONANIM == eModelType)
		return Render_ShadowModel(pShader, pModel, MESH_LAYER_PROFILE::WORLD_NONANIM);

	if (MODEL::ANIM != eModelType)
		return E_FAIL;

	if (FAILED(Bind_ShadowTransforms(pShader)))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(pModel->Get_NumMeshes());
	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(pModel->Bind_BoneMatrices(pShader, "g_BoneMatrices", i)))
			return E_FAIL;

		if (FAILED(Render_ShadowMesh(pShader, pModel, i, MESH_LAYER_PROFILE::WORLD_ANIM)))
			return E_FAIL;
	}

	return S_OK;
}

void CLevelDesign_Bush::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLevelDesign_Bush::Damaged(const ATTACK_INFO& tInfo)
{
	if (BUSH_STATE::CUT == m_eState)
		return;

	if (!Is_CutHit(tInfo.eHitType))
		return;

	_float3 vPos{};
	if (!Compute_EffectSpawnPosition(m_pModelComs[BUSH_STATE::BASIC], DESTROY_EFFECT_HEIGHT_RATIO, &vPos))
		XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));

	CEffect_Loader::GetInstance()->Spawn(L"Split_Bush", Get_LevelIndex(), vPos);

	_int	iIdx = m_pGameInstance_Proxy->RandomInt(1, 4);
	_tchar	szSoundKey[MAX_PATH] = {};

	swprintf_s(szSoundKey, L"GimmickBush_Cut%d.wav", iIdx);
	m_pGameInstance_Proxy->Play_SFX(szSoundKey, 0.3f);

	m_eState = BUSH_STATE::CUT;
	m_pHurtBox->Set_Enabled(false);
	Publish_LDEvent();
}

void CLevelDesign_Bush::Register_LevelDesignSpecs()
{
	for (const LD_BUSH_CATALOG& Entry : g_BushCatalog)
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
	UNREFERENCED_PARAMETER(jEntry);

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
			? Shader_World_Anim
			: Shader_World_NonAnim;

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

	m_pAnimatorCom->Play(ANIM_WAIT, true, true);

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

	m_pHurtBox = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag, TEXT("Com_HurtBox"),
		&ColliderDesc);
	if (nullptr == m_pHurtBox)
		return E_FAIL;

	SetUp_Collider_Callback();

	m_pGameInstance_Proxy->Register_Collider(m_pHurtBox, ETOUI(COLLISION_LAYER::ENV_FOLIAGE));

	return S_OK;
}

HRESULT CLevelDesign_Bush::Ready_BushCullBounds()
{
	BoundingBox CullBounds{};
	_bool bHasBounds = false;

	for (_uint i = 0; i < BUSH_STATE::_COUNT; ++i)
	{
		_float3 vMin{}, vMax{};
		m_pModelComs[i]->Get_ModelAABB(&vMin, &vMax);

		const BoundingBox ModelBounds = GeometryUtils::Is_ValidAABB(vMin, vMax)
			? GeometryUtils::Make_AABB_FromMinMax(vMin, vMax)
			: GeometryUtils::Make_DefaultAABB(0.5f);

		CullBounds = bHasBounds
			? GeometryUtils::Merge_AABB(CullBounds, ModelBounds)
			: ModelBounds;
		bHasBounds = true;
	}

	if (!bHasBounds || !GeometryUtils::Expand_AABB(&CullBounds, BUSH_CULL_MARGIN))
		return E_FAIL;

	return Ready_CullingState(CullBounds);
}


void CLevelDesign_Bush::SetUp_Collider_Callback()
{
	if (nullptr == m_pHurtBox)
		return;

	m_pHurtBox->Set_OnEnter([this](CCollider* pOther) { Handle_HurtBoxEnter(pOther); });
	m_pHurtBox->Set_OnExit([this](CCollider* pOther) { Handle_HurtBoxExit(pOther); });
}

void CLevelDesign_Bush::Handle_HurtBoxEnter(CCollider* pOther)
{
	if (BUSH_STATE::BASIC != m_eState || nullptr == pOther)
		return;

	const _uint iGroup = pOther->Get_RegisteredGroup();
	if (ETOUI(COLLISION_LAYER::PLAYER_INHALE) == iGroup)
	{
		m_bInhaleOverlapping = true;
		if (m_pAnimatorCom->Get_CurrentAnimName() != ANIM_SHAKE_LOOP || m_pAnimatorCom->Is_Finished())
			m_pAnimatorCom->Play(ANIM_SHAKE_LOOP, false, true);
		return;
	}

	if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != iGroup && ETOUI(COLLISION_LAYER::PLAYER_PROJECTILE) != iGroup)
		return;

	if (m_pAnimatorCom->Get_CurrentAnimName() == ANIM_SHAKE_LOOP)
		return;
	if (m_pAnimatorCom->Get_CurrentAnimName() == ANIM_SHAKE_ONCE && !m_pAnimatorCom->Is_Finished())
		return;

	m_pAnimatorCom->Play(ANIM_SHAKE_ONCE, false, true);
}

void CLevelDesign_Bush::Handle_HurtBoxExit(CCollider* pOther)
{
	if (BUSH_STATE::BASIC != m_eState || nullptr == pOther)
		return;

	if (ETOUI(COLLISION_LAYER::PLAYER_INHALE) != pOther->Get_RegisteredGroup())
		return;

	m_bInhaleOverlapping = false;
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
			if (FAILED(pModel->Bind_BoneMatrices(pShader, "g_BoneMatrices", i)))
				return E_FAIL;

			MESH_LAYER_BIND_CONTEXT Ctx{};
			Ctx.Set_Renderer(pShader, pModel, m_pGameInstance_Proxy, m_pCullingState);
			Ctx.iMesh = i;
			Ctx.pLayer = &Layer;
			Ctx.eProfile = MESH_LAYER_PROFILE::WORLD_ANIM;
			Ctx.eKind = MESH_LAYER_RENDER_KIND::MAIN;
			Ctx.iFallbackPass = ETOUI(WORLD_PASS::UMN);

			_uint iPass = 0u;
			const HRESULT hrBind = MeshLayerBinder::Bind_OrSkip(Ctx, &iPass);
			if (FAILED(hrBind))     return E_FAIL;
			if (S_FALSE == hrBind)  continue;

			if (FAILED(pShader->Begin(iPass)))
				return E_FAIL;
		}
		else
		{
			MESH_LAYER_BIND_CONTEXT Ctx{};
			Ctx.Set_Renderer(pShader, pModel, m_pGameInstance_Proxy, m_pCullingState);
			Ctx.iMesh = i;
			Ctx.pLayer = &Layer;
			Ctx.eProfile = MESH_LAYER_PROFILE::WORLD_NONANIM;
			Ctx.eKind = MESH_LAYER_RENDER_KIND::MAIN;
			Ctx.iFallbackPass = ETOUI(WORLD_PASS::UMN);

			_uint iPass = 0u;
			const HRESULT hrBind = MeshLayerBinder::Bind_OrSkip(Ctx, &iPass);
			if (FAILED(hrBind))     return E_FAIL;
			if (S_FALSE == hrBind)  continue;

			if (FAILED(pShader->Begin(iPass)))
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
