#include "LevelDesignObject.h"
#include "MeshLayer_Binder.h"

#include "GameInstance.h"
#include "Geometry_Utils.h"

NS_BEGIN(Client)

namespace
{
	constexpr _bool		ENABLE_LD_OBJECT_SHADOW = true;
	constexpr _float	LD_DISTANCE_CULL_START = 175.f;
	constexpr _float	LD_SHADOW_DISTANCE_CULL_START = 175.f;
	constexpr _uint		LD_CULL_CHECK_INTERVAL = 2u;

	static_assert(0u < LD_CULL_CHECK_INTERVAL);
}

CLevelDesignObject::CLevelDesignObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}

CLevelDesignObject::CLevelDesignObject(const CLevelDesignObject& Prototype)
	: CGameObject(Prototype)
	, m_tLevelDesignDesc(Prototype.m_tLevelDesignDesc)
	, m_bUseShadow(Prototype.m_bUseShadow)
	, m_bUseCullDistance(Prototype.m_bUseCullDistance)
	, m_bUseCullFrustum(Prototype.m_bUseCullFrustum)
{
}

HRESULT CLevelDesignObject::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CLevelDesignObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const LD_OBJECT_DESC* pDesc = static_cast<const LD_OBJECT_DESC*>(pArg);

	m_tLevelDesignDesc = *pDesc;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_iMaterialID = 0;

	return S_OK;
}

HRESULT CLevelDesignObject::Validate_Initialized()
{
	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom)
		return E_FAIL;
	if (m_tLevelDesignDesc.strObjectName.empty())
		return E_FAIL;
	if (ETOUI(m_tLevelDesignDesc.eCategory) >= ETOUI(LD_CATEGORY::END))
		return E_FAIL;

	return S_OK;
}

_wstring CLevelDesignObject::Make_LevelDesignObjectKey() const
{
	if (m_tLevelDesignDesc.iUid != 0)
		return m_tLevelDesignDesc.strSection + L":" + to_wstring(m_tLevelDesignDesc.iUid);

	return m_tLevelDesignDesc.strSection
		+ L":"
		+ m_tLevelDesignDesc.strEntryKey
		+ L":"
		+ m_tLevelDesignDesc.strObjectName;
}

#pragma region Editable
void CLevelDesignObject::Add_EditModelSlot(vector<EDITABLE_MODEL_SLOT>* pOutSlots, const _tchar* pLabel, EDITABLE_MODEL_KIND eKind, CModel* pModel) const
{
	if (nullptr == pOutSlots || nullptr == pModel)
		return;

	EDITABLE_MODEL_SLOT Slot{};
	Slot.strLabel = nullptr != pLabel ? pLabel : L"Model";
	Slot.eKind = eKind;
	Slot.pModel = pModel;
	Slot.iMeshCount = static_cast<_uint>(pModel->Get_NumMeshes());

	pOutSlots->push_back(Slot);
}
void CLevelDesignObject::Build_EditCapabilities(_uint* pOutCaps, EDIT_OBJECT_POLICY* pOutPolicy) const
{
	if (nullptr != pOutCaps)
		*pOutCaps = 0u;

	if (nullptr != pOutPolicy)
		*pOutPolicy = {};
}

void CLevelDesignObject::Collect_EditModelSlots(vector<EDITABLE_MODEL_SLOT>* pOutSlots) const
{
	if (nullptr == pOutSlots)
		return;

	const auto& Components = Get_Components();
	const auto Iter = Components.find(TEXT("Com_Model"));
	if (Iter == Components.end())
		return;

	CModel* pModel = dynamic_cast<CModel*>(Iter->second);
	if (nullptr == pModel)
		return;

	const EDITABLE_MODEL_KIND eKind = 0u < pModel->Get_NumAnimations()
		? EDITABLE_MODEL_KIND::ANIM
		: EDITABLE_MODEL_KIND::NONANIM;

	Add_EditModelSlot(pOutSlots, TEXT("Model"), eKind, pModel);
}

HRESULT CLevelDesignObject::On_ApplyEditPolicy(const EDIT_OBJECT_POLICY& Policy)
{
	UNREFERENCED_PARAMETER(Policy);
	return S_OK;
}

_bool CLevelDesignObject::Get_EditDesc(EDITABLE_DESC* pOutDesc) const
{
	if (nullptr == pOutDesc)
		return false;

	*pOutDesc = {};
	pOutDesc->eKind = EDITABLE_OBJECT_KIND::LEVEL_DESIGN_OBJECT;
	pOutDesc->strStableKey = m_tLevelDesignDesc.strSourceFile + L"|" + m_tLevelDesignDesc.strSection + L"|" + m_tLevelDesignDesc.strEntryKey + L"|"
		+ to_wstring(m_tLevelDesignDesc.iUid);

	Build_EditCapabilities(&pOutDesc->iCapabilities, &pOutDesc->Policy);
	Collect_EditModelSlots(&pOutDesc->ModelSlots);

	if (!pOutDesc->ModelSlots.empty())
	{
		pOutDesc->iCapabilities |= EDIT_CAP_MESH_LAYER;

		for (const EDITABLE_MODEL_SLOT& Slot : pOutDesc->ModelSlots)
		{
			if (EDITABLE_MODEL_KIND::ANIM == Slot.eKind)
			{
				pOutDesc->iCapabilities |= EDIT_CAP_ANIMATION;
				break;
			}
		}
	}

	return true;
}

HRESULT CLevelDesignObject::Apply_EditPolicy(const EDIT_OBJECT_POLICY& Policy)
{
	return On_ApplyEditPolicy(Policy);
}

HRESULT CLevelDesignObject::On_EditTransformChanged()
{
	if (!m_bHasCullBounds)
		return S_OK;

	Refresh_WorldBounds();
	m_iCullCheckCounter = 0u;
	return S_OK;
}

const MESH_LAYER_IDX* CLevelDesignObject::Get_EditMeshLayer(_uint iModelSlot, _uint iMesh) const
{
	vector<EDITABLE_MODEL_SLOT> Slots;
	Collect_EditModelSlots(&Slots);

	if (iModelSlot >= Slots.size())
		return nullptr;

	CModel* pModel = Slots[iModelSlot].pModel;
	if (nullptr == pModel || iMesh >= pModel->Get_NumMeshes())
		return nullptr;

	return &pModel->Get_MeshLayer(iMesh);
}

HRESULT CLevelDesignObject::Apply_EditMeshLayer(_uint iModelSlot, _uint iMesh, const MESH_LAYER_IDX& Layer)
{
	vector<EDITABLE_MODEL_SLOT> Slots;
	Collect_EditModelSlots(&Slots);

	if (iModelSlot >= Slots.size())
		return E_FAIL;

	CModel* pModel = Slots[iModelSlot].pModel;
	if (nullptr == pModel || iMesh >= pModel->Get_NumMeshes())
		return E_FAIL;

	pModel->Set_MeshLayer(iMesh, Layer);
	return S_OK;
}
#pragma endregion

HRESULT CLevelDesignObject::Ready_CullBounds(CModel* pModel, _float fBoundsMargin)
{
	if (nullptr == pModel || nullptr == m_pTransformCom)
		return E_FAIL;

	_float3 vMin{}, vMax{};
	pModel->Get_ModelAABB(&vMin, &vMax);

	BoundingBox LocalBounds = GeometryUtils::Is_ValidAABB(vMin, vMax)
		? GeometryUtils::Make_AABB_FromMinMax(vMin, vMax)
		: GeometryUtils::Make_DefaultAABB(0.5f);

	if (!GeometryUtils::Expand_AABB(&LocalBounds, fBoundsMargin))
		return E_FAIL;

	Set_CullLocalBounds(LocalBounds);
	return S_OK;
}

HRESULT CLevelDesignObject::Ready_CullBounds_RotationInvariant(CModel* pModel, _float fBoundsMargin)
{
	if (FAILED(Ready_CullBounds(pModel, fBoundsMargin)))
		return E_FAIL;

	m_bUseRotationInvariantCullBounds = true;
	Refresh_WorldBounds();
	return S_OK;
}

void CLevelDesignObject::Set_CullLocalBounds(const BoundingBox& LocalBounds)
{
	m_LocalBounds = GeometryUtils::Is_ValidAABB(LocalBounds)
		? LocalBounds
		: GeometryUtils::Make_DefaultAABB(0.5f);

	m_bHasCullBounds = true;
	m_bUseRotationInvariantCullBounds = false;
	m_iCullCheckCounter = m_tLevelDesignDesc.iUid % LD_CULL_CHECK_INTERVAL;

	Refresh_WorldBounds();
}

void CLevelDesignObject::Refresh_WorldBounds()
{
	if (!m_bHasCullBounds || nullptr == m_pTransformCom)
		return;

	if (m_bUseRotationInvariantCullBounds)
	{
		const _float3 vScale = m_pTransformCom->Get_Scaled();
		const _float fMaxScale = max(vScale.x, max(vScale.y, vScale.z));
		const _float fRadius = (XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_LocalBounds.Center)))
			+ XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_LocalBounds.Extents)))) * fMaxScale;

		_float3 vPosition{};
		XMStoreFloat3(&vPosition, m_pTransformCom->Get_State(STATE::POSITION));
		m_WorldBounds = BoundingBox(vPosition, _float3(fRadius, fRadius, fRadius));
		return;
	}

	GeometryUtils::Transform_AABB(m_LocalBounds, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()), &m_WorldBounds);
}

void CLevelDesignObject::Check_Visible()
{
	const _bool bShadowPolicy = ENABLE_LD_OBJECT_SHADOW && m_bUseShadow;

	if (!m_bHasCullBounds)
	{
		m_bVisible = true;
		m_bVisibleShadow = bShadowPolicy;
		return;
	}

	if (0u < m_iCullCheckCounter)
	{
		--m_iCullCheckCounter;
		m_bVisibleShadow = m_bVisibleShadow && bShadowPolicy;
		return;
	}
	m_iCullCheckCounter = LD_CULL_CHECK_INTERVAL - 1u;

	m_bVisible = true;
	m_bVisibleShadow = bShadowPolicy;

	if (m_bUseCullDistance && m_pGameInstance_Proxy->Should_CullByDistance(m_WorldBounds, LD_DISTANCE_CULL_START))
		m_bVisible = false;
	else if (m_bUseCullFrustum && m_pGameInstance_Proxy->Should_CullAABB(CULLING_VIEW::MAIN_CAMERA, m_WorldBounds))
		m_bVisible = false;

	if (m_bVisibleShadow)
	{
		if (m_bUseCullDistance && m_pGameInstance_Proxy->Should_CullByDistance(m_WorldBounds, LD_SHADOW_DISTANCE_CULL_START))
			m_bVisibleShadow = false;
		else if (m_bUseCullFrustum && m_pGameInstance_Proxy->Should_CullAABB(CULLING_VIEW::SHADOW_DIR, m_WorldBounds))
			m_bVisibleShadow = false;
	}
}

void CLevelDesignObject::Submit_RenderGroups(RENDERID eMainID)
{
	if (m_bVisible)
		m_pGameInstance_Proxy->Add_RenderGroup(eMainID, this);

	if (m_bVisibleShadow)
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
}

HRESULT CLevelDesignObject::Bind_ShadowTransforms(CShader* pShader, const _float4x4* pWorldOverride) const
{
	if (nullptr == pShader || nullptr == m_pGameInstance_Proxy)
		return E_FAIL;

	if (nullptr != pWorldOverride)
	{
		if (FAILED(pShader->Bind_Matrix("g_WorldMatrix", pWorldOverride)))
			return E_FAIL;
	}
	else
	{
		if (nullptr == m_pTransformCom)
			return E_FAIL;

		if (FAILED(m_pTransformCom->Bind_ShaderResource(pShader, "g_WorldMatrix")))
			return E_FAIL;
	}

	if (FAILED(pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesignObject::Render_ShadowMesh(CShader* pShader, CModel* pModel, _uint iMeshIndex, MESH_LAYER_PROFILE eProfile) const
{
	if (nullptr == pShader || nullptr == pModel || nullptr == m_pGameInstance_Proxy)
		return E_FAIL;
	if (iMeshIndex >= static_cast<_uint>(pModel->Get_NumMeshes()))
		return E_FAIL;

	const MESH_LAYER_IDX& Layer = pModel->Get_MeshLayer(iMeshIndex);

	MESH_LAYER_BIND_CONTEXT Ctx{};
	Ctx.pShader = pShader;
	Ctx.pModel = pModel;
	Ctx.pGI_Proxy = m_pGameInstance_Proxy;
	Ctx.iMesh = iMeshIndex;
	Ctx.pLayer = &Layer;
	Ctx.eProfile = eProfile;
	Ctx.eKind = MESH_LAYER_RENDER_KIND::SHADOW;
	Ctx.iFallbackPass = ETOUI(WORLD_PASS::SHADOW);

	MESH_LAYER_BIND_RESULT Result{};
	if (FAILED(MeshLayerBinder::Bind(Ctx, &Result)))
		return E_FAIL;
	if (Result.bSkipMesh)
		return S_OK;

	if (FAILED(pShader->Begin(Result.iPass)))
		return E_FAIL;
	if (FAILED(pModel->Render(iMeshIndex)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesignObject::Render_ShadowModel(CShader* pShader, CModel* pModel, MESH_LAYER_PROFILE eProfile, const _float4x4* pWorldOverride) const
{
	if (nullptr == pShader || nullptr == pModel)
		return E_FAIL;

	if (FAILED(Bind_ShadowTransforms(pShader, pWorldOverride)))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(pModel->Get_NumMeshes());
	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(Render_ShadowMesh(pShader, pModel, i, eProfile)))
			return E_FAIL;
	}

	return S_OK;
}

_bool CLevelDesignObject::Compute_EffectSpawnPosition(CModel* pModel, _float fHeightRatio, _float3* pOutPosition) const
{
	if (nullptr == pModel || nullptr == m_pTransformCom || nullptr == pOutPosition)
		return false;

	_float3 vMin{}, vMax{};
	pModel->Get_ModelAABB(&vMin, &vMax);

	if (!GeometryUtils::Is_ValidAABB(vMin, vMax))
		return false;

	const _float fClampedHeightRatio = max(0.f, min(1.f, fHeightRatio));
	const _float3 vLocalPosition = {
			(vMin.x + vMax.x) * 0.5f,
			vMin.y + (vMax.y - vMin.y) * fClampedHeightRatio,
			(vMin.z + vMax.z) * 0.5f
	};

	XMStoreFloat3(pOutPosition, XMVector3TransformCoord(
		XMLoadFloat3(&vLocalPosition),
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())));

	return true;
}

void CLevelDesignObject::Free()
{
	__super::Free();
}

NS_END