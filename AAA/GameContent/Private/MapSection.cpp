#include "MapSection.h"
#include "Shader_PassMeta.h"
#include "Map_EditFile.h"

#include "GameInstance.h"
#include "Geometry_Utils.h"

NS_BEGIN(Client)

CMapSection::CMapSection(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMapObject { pDevice, pContext }
{
}

CMapSection::CMapSection(const CMapSection& Prototype)
	: CMapObject(Prototype)
{
}

HRESULT CMapSection::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const MAP_SECTION_DESC* pDesc = static_cast<const MAP_SECTION_DESC*>(pArg);
	m_tDesc = *pDesc;

	m_strSectionName = pDesc->strSectionName;
	m_strModelProtoTag = pDesc->wstrModelProtoTag;
	m_iModelProtoLevel = pDesc->iModelProtoLevel;
	m_eSectionType = pDesc->eSectionType;
	m_eRenderID = pDesc->eRenderID;
	m_bEnableCulling = pDesc->bEnableCulling;
	m_bRenderable = pDesc->bRenderable;
	m_bUseCollMesh = pDesc->bUseCollMesh;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_bHasCollMesh = nullptr != m_pModelCom->Get_CollisionMesh();
	m_bUseCollMesh = m_bUseCollMesh && m_bHasCollMesh;

	m_CombinedWorldMatrix = *m_pTransformCom->Get_WorldMatrixPtr();
	Update_LocalBounds();
	Refresh_WorldBounds();
	Rebuild_ColliderActor();

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapSection::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;
	if (m_strSectionName.empty() || m_strModelProtoTag.empty())
		return E_FAIL;
	if (ETOUI(m_eSectionType) >= MAP_SECTION_TYPE_COUNT)
		return E_FAIL;
	if (ETOUI(m_eRenderID) >= ETOUI(RENDERID::END))
		return E_FAIL;
	if (m_bUseCollMesh && (!m_bHasCollMesh || nullptr == m_pColliderActor))
		return E_FAIL;
	return S_OK;
}

void CMapSection::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	// MapStage owns render-group submission for sections.
	// CMapObject::Late_Update() is intentionally skipped here.
}

HRESULT CMapSection::Render_Shadow()
{
	if (FAILED(Bind_WorldMatrix()))
		return E_FAIL;
	if (FAILED(Bind_ShadowTransforms()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (!Should_RenderMesh(i))
			continue;

		if (FAILED(Render_ShadowMesh(i)))
			return E_FAIL;
	}

	return S_OK;
}

void CMapSection::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CMapSection::Refresh_WorldBounds()
{
	m_LocalBounds.Transform(
		m_WorldBounds,
		XMLoadFloat4x4(&m_CombinedWorldMatrix));
}

void CMapSection::Set_ParentMatrix(const _float4x4* pParentMatrix)
{
	m_pParentMatrix = pParentMatrix;
}

void CMapSection::Refresh_CombinedWorldMatrix()
{
	_matrix CombinedWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	if (nullptr != m_pParentMatrix)
		CombinedWorld *= XMLoadFloat4x4(m_pParentMatrix);

	XMStoreFloat4x4(&m_CombinedWorldMatrix, CombinedWorld);
	Refresh_WorldBounds();
	Refresh_ColliderPose();
}

#pragma region Editable
_bool CMapSection::Get_EditDesc(EDITABLE_DESC* pOutDesc) const
{
	if (nullptr == pOutDesc)
		return false;

	pOutDesc->eKind = EDITABLE_OBJECT_KIND::MAP_SECTION;
	pOutDesc->strStableKey = CMap_EditFile::Make_SectionKey(m_strStageName, m_strSectionName);
	pOutDesc->iCapabilities = EDIT_CAP_RENDERABLE | EDIT_CAP_CULL_FRUSTUM | EDIT_CAP_MESH_LAYER;

	if (m_bHasCollMesh)	pOutDesc->iCapabilities |= EDIT_CAP_COLLISION_MESH;

	pOutDesc->Policy.bRenderable = m_bRenderable;
	pOutDesc->Policy.bUseCullDistance = false;
	pOutDesc->Policy.bUseCullFrustum = m_bEnableCulling;
	pOutDesc->Policy.bUseCollMesh = m_bHasCollMesh && m_bUseCollMesh;
	pOutDesc->Policy.bUseShadow = false;

	pOutDesc->ModelSlots.clear();
	if (nullptr != m_pModelCom)
	{
		EDITABLE_MODEL_SLOT Slot{};
		Slot.strLabel = L"Model";
		Slot.eKind = EDITABLE_MODEL_KIND::NONANIM;
		Slot.pModel = m_pModelCom;
		Slot.iMeshCount = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
		pOutDesc->ModelSlots.push_back(Slot);
	}

	return true;
}

HRESULT CMapSection::Apply_EditPolicy(const EDIT_OBJECT_POLICY& Policy)
{
	m_bRenderable = Policy.bRenderable;
	m_bEnableCulling = Policy.bUseCullFrustum;
	Set_UseCollMesh(m_bHasCollMesh ? Policy.bUseCollMesh : false);
	return S_OK;
}

HRESULT CMapSection::On_EditTransformChanged()
{
	Refresh_CombinedWorldMatrix();
	return S_OK;
}

const MESH_LAYER_IDX* CMapSection::Get_EditMeshLayer(_uint iModelSlot, _uint iMesh) const
{
	if (0u != iModelSlot)
		return nullptr;

	if (nullptr == m_pModelCom || iMesh >= m_pModelCom->Get_NumMeshes())
		return nullptr;

	return &m_pModelCom->Get_MeshLayer(iMesh);
}

HRESULT CMapSection::Apply_EditMeshLayer(_uint iModelSlot, _uint iMesh, const MESH_LAYER_IDX& Layer)
{
	if (0u != iModelSlot)
		return E_FAIL;

	if (nullptr == m_pModelCom || iMesh >= m_pModelCom->Get_NumMeshes())
		return E_FAIL;

	m_pModelCom->Set_MeshLayer(iMesh, Layer);
	return S_OK;
}
#pragma endregion

#ifdef _DEBUG
void CMapSection::Set_EditorSoloMeshIndex(_int iMeshIndex)
{
	const _int iNumMeshes = static_cast<_int>(m_pModelCom->Get_NumMeshes());

	if (iMeshIndex < 0 || iMeshIndex >= iNumMeshes)
	{
		m_iEditorSoloMeshIndex = -1;
		return;
	}

	m_iEditorSoloMeshIndex = iMeshIndex;
}

void CMapSection::Clear_EditorSoloMesh()
{
	m_iEditorSoloMeshIndex = -1;
}

_bool CMapSection::Should_RenderMesh(_uint iMesh) const
{
	if (m_iEditorSoloMeshIndex < 0)
		return true;

	return iMesh == static_cast<_uint>(m_iEditorSoloMeshIndex);
}
#endif

json CMapSection::Serialize_SectionState() const
{
	json j = IReflectable::Serialize();

	j["SectionName"] = WstrToStr(m_strSectionName);
	j["SectionRender"]["RenderID"] = static_cast<_int>(m_eRenderID);

	return j;
}

void CMapSection::Deserialize_SectionState(const json& j)
{
	IReflectable::Deserialize_Internal(j);

	if (j.contains("SectionRender") && j["SectionRender"].is_object())
	{
		const json& jRender = j["SectionRender"];

		if (jRender.contains("RenderID") && jRender["RenderID"].is_number_integer())
			Set_RenderID(static_cast<RENDERID>(jRender["RenderID"].get<_int>()));
	}
}

void CMapSection::Set_RenderID(RENDERID eRenderID)
{
	if (ETOUI(eRenderID) >= ETOUI(RENDERID::END))
		return;

	m_eRenderID = eRenderID;
}

void CMapSection::Set_UseCollMesh(_bool bUseCollMesh)
{
	if (m_bUseCollMesh == bUseCollMesh)
		return;

	m_bUseCollMesh = bUseCollMesh;
	Rebuild_ColliderActor();
}

const _tchar* CMapSection::Get_ModelProtoTag() const
{
	return m_strModelProtoTag.c_str();
}

_uint CMapSection::Get_ModelProtoLevel() const
{
	return m_iModelProtoLevel;
}

HRESULT CMapSection::Bind_WorldMatrix()
{
	return m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix);
}

void CMapSection::Update_LocalBounds()
{
	_float3 vMin{}, vMax{};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	if (!GeometryUtils::Is_ValidAABB(vMin, vMax))
	{
		m_LocalBounds = GeometryUtils::Make_DefaultAABB();
		return;
	}

	m_LocalBounds = GeometryUtils::Make_AABB_FromMinMax(vMin, vMax);
}

void CMapSection::Refresh_ColliderPose()
{
	if (nullptr == m_pColliderActor)
		return;

	m_pGameInstance_Proxy->Refresh_StaticActorPose(m_pColliderActor, XMLoadFloat4x4(&m_CombinedWorldMatrix));
}

void CMapSection::Rebuild_ColliderActor()
{
	if (nullptr != m_pColliderActor)
	{
		m_pGameInstance_Proxy->Remove_StaticActor(m_pColliderActor);
		m_pColliderActor = nullptr;
	}

	if (!m_bUseCollMesh)
		return;

	m_pColliderActor = m_pGameInstance_Proxy->Create_StaticActor(
		m_pModelCom->Get_CollisionMesh(),
		XMLoadFloat4x4(&m_CombinedWorldMatrix));
}

CMapSection* CMapSection::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapSection* pInstance = new CMapSection(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMapSection");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapSection::Clone(void* pArg)
{
	CMapSection* pInstance = new CMapSection(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMapSection");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMapSection::Free()
{
	if (m_pColliderActor)
	{ 
		m_pGameInstance_Proxy->Remove_StaticActor(m_pColliderActor);
		m_pColliderActor = nullptr;
	}

	__super::Free();
}

NS_END