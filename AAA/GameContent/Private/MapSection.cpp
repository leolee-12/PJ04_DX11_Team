#include "MapSection.h"
#include "Shader_PassMeta.h"
#include "Map_EditFile.h"
#include "GameContent_Log.h"

#include "GameInstance.h"

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
	m_bRenderable = pDesc->bRenderable;
	m_bUseCollMesh = pDesc->bUseCollMesh;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_bHasCollMesh = nullptr != m_pModelCom->Get_CollisionMesh();

	m_CombinedWorldMatrix = *m_pTransformCom->Get_WorldMatrixPtr();

	if (FAILED(Ready_RigidStatic()))
		return E_FAIL;

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapSection::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
	{
		Log_GameContentWarning("MapSection component invalid: " + WstrToStr(m_strSectionName));
		return E_FAIL;
	}
	if (m_strSectionName.empty() || m_strModelProtoTag.empty())
	{
		Log_GameContentWarning("MapSection descriptor invalid: " + WstrToStr(m_strSectionName));
		return E_FAIL;
	}
	if (m_bUseCollMesh && (!m_bHasCollMesh || nullptr == m_pRigidStatic))
	{
		Log_GameContentError("MapSection rigid static invalid: " + WstrToStr(m_strSectionName));
		return E_FAIL;
	}
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
	Refresh_RigidStaticPose();
}

#pragma region Editable
_bool CMapSection::Get_EditDesc(EDITABLE_DESC* pOutDesc) const
{
	if (nullptr == pOutDesc)
		return false;

	pOutDesc->eKind = EDITABLE_OBJECT_KIND::MAP_SECTION;
	pOutDesc->strStableKey = CMap_EditFile::Make_SectionKey(m_strStageName, m_strSectionName);
	pOutDesc->iCapabilities = EDIT_CAP_RENDERABLE | EDIT_CAP_MESH_LAYER;

	if (m_bHasCollMesh)	pOutDesc->iCapabilities |= EDIT_CAP_COLLISION_MESH;

	pOutDesc->Policy.bRenderable = m_bRenderable;
	pOutDesc->Policy.bUseCullDistance = false;
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
	if (FAILED(Set_UseCollMesh(Policy.bUseCollMesh)))
		return E_FAIL;

	m_bRenderable = Policy.bRenderable;
	return S_OK;
}

HRESULT CMapSection::On_EditTransformChanged()
{
	Refresh_CombinedWorldMatrix();
	return S_OK;
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

HRESULT CMapSection::Set_UseCollMesh(_bool bUseCollMesh)
{
	if (m_bUseCollMesh == bUseCollMesh)
		return S_OK;

	if (!bUseCollMesh)
	{
		m_bUseCollMesh = false;
		Release_RigidStatic();
		return S_OK;
	}

	m_bUseCollMesh = true;

	if (FAILED(Ready_RigidStatic()))
	{
		m_bUseCollMesh = false;
		return E_FAIL;
	}

	return S_OK;
}

void CMapSection::Deactivate()
{
	Set_Active(false);
	m_bRenderable = false;
	m_bUseCollMesh = false;
	Release_RigidStatic();
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

HRESULT CMapSection::Ready_RigidStatic()
{
	if (!m_bUseCollMesh || nullptr != m_pRigidStatic)
		return S_OK;

	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pModelCom)
		return E_FAIL;

	auto* pCollisionMesh = m_pModelCom->Get_CollisionMesh();
	if (nullptr == pCollisionMesh)
	{
		Log_GameContentError("MapSection collision mesh missing: " + WstrToStr(m_strSectionName)
			+ ", model=" + WstrToStr(m_strModelProtoTag));
		return E_FAIL;
	}

	m_pRigidStatic = m_pGameInstance_Proxy->Create_StaticActor(
		pCollisionMesh,
		XMLoadFloat4x4(&m_CombinedWorldMatrix));

	if (nullptr == m_pRigidStatic)
	{
		Log_GameContentError("MapSection rigid static creation failed: " + WstrToStr(m_strSectionName));
		return E_FAIL;
	}

	return S_OK;
}

void CMapSection::Refresh_RigidStaticPose()
{
	if (nullptr == m_pRigidStatic)
		return;

	m_pGameInstance_Proxy->Refresh_StaticActorPose(m_pRigidStatic, XMLoadFloat4x4(&m_CombinedWorldMatrix));
}

void CMapSection::Release_RigidStatic()
{
	if (nullptr == m_pRigidStatic)
		return;

	if (nullptr != m_pGameInstance_Proxy)
		m_pGameInstance_Proxy->Remove_StaticActor(m_pRigidStatic);

	m_pRigidStatic = nullptr;
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
	Release_RigidStatic();
	__super::Free();
}

NS_END