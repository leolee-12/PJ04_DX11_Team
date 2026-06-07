#include "MapSection.h"

#include "GameInstance.h"

NS_BEGIN(Client)

namespace
{
	BoundingBox Make_DefaultAABB()
	{
		return BoundingBox(_float3(0.f, 0.f, 0.f), _float3(1.f, 1.f, 1.f));
	}

	_bool Is_ValidAABB(const _float3& vMin, const _float3& vMax)
	{
		return vMin.x <= vMax.x
			&& vMin.y <= vMax.y
			&& vMin.z <= vMax.z;
	}

	BoundingBox Make_AABB_FromMinMax(const _float3& vMin, const _float3& vMax)
	{
		const _float3 vCenter(
			(vMin.x + vMax.x) * 0.5f,
			(vMin.y + vMax.y) * 0.5f,
			(vMin.z + vMax.z) * 0.5f);

		const _float3 vExtents(
			(vMax.x - vMin.x) * 0.5f,
			(vMax.y - vMin.y) * 0.5f,
			(vMax.z - vMin.z) * 0.5f);

		return BoundingBox(vCenter, vExtents);
	}

	void Log_MapSectionWarning(const string& strMessage)
	{
		OutputDebugStringA((strMessage + "\n").c_str());
	}
}

CMapSection::CMapSection(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMapObject { pDevice, pContext }
{
}

CMapSection::CMapSection(const CMapSection& Prototype)
	: CMapObject(Prototype)
	, m_strProtoTag { Prototype.m_strProtoTag }
{
}

HRESULT CMapSection::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CMapSection::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const MAP_SECTION_DESC* pDesc = static_cast<const MAP_SECTION_DESC*>(pArg);

	m_strSectionName = pDesc->strSectionName;
	m_strModelProtoTag = pDesc->strModelProtoTag;
	m_strModelPath = pDesc->strModelPath;
	m_iModelProtoLevel = pDesc->iModelProtoLevel;
	m_eSectionType = pDesc->eSectionType;
	m_eRenderID = pDesc->eRenderID;
	m_bCastShadow = pDesc->bCastShadow;
	m_bEnableCulling = pDesc->bEnableCulling;
	m_bRenderable = pDesc->bRenderable;

	if (m_bRenderable)
	{
		if (FAILED(__super::Initialize(pArg)))
			return E_FAIL;

		m_CombinedWorldMatrix = *m_pTransformCom->Get_WorldMatrixPtr();
		Update_LocalBounds();
		Refresh_WorldBounds();
		if (m_pModelCom && m_pModelCom->Get_CollisionMesh())
		{
			m_pColliderActor = m_pGameInstance_Proxy->Add_StaticActor(
				m_pModelCom->Get_CollisionMesh(), XMLoadFloat4x4(&m_CombinedWorldMatrix));
#ifdef _DEBUG
			_float4 p; XMStoreFloat4(&p, XMLoadFloat4x4(&m_CombinedWorldMatrix).r[3]);
			char buf[256];
			sprintf_s(buf, "[MapColl] %ls  mesh=%p  actor=%p  pos=(%.1f,%.1f,%.1f)\n",
				m_strSectionName.c_str(), (void*)m_pModelCom->Get_CollisionMesh(),
				(void*)m_pColliderActor, p.x, p.y, p.z);
			OutputDebugStringA(buf);
#endif
		}
#ifdef _DEBUG
		else
			OutputDebugStringA("[MapColl] !!! 충돌메쉬 없음 (cook 실패 or null) !!!\n");
#endif
	}
	else
	{
		if (FAILED(CGameObject::Initialize(pArg)))
			return E_FAIL;

		m_CombinedWorldMatrix = *m_pTransformCom->Get_WorldMatrixPtr();
		m_LocalBounds = Make_DefaultAABB();
		m_WorldBounds = m_LocalBounds;
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
	if (!m_bRenderable || !m_bCastShadow || nullptr == m_pModelCom || nullptr == m_pShaderCom)
		return S_OK;

	if (FAILED(Bind_WorldMatrix()))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
		return E_FAIL;

	size_t n = m_pModelCom->Get_NumMeshes();
	for (size_t i = 0; i < n; ++i)
	{
		if (FAILED(m_pShaderCom->Begin(3))) 
			return E_FAIL;
		if (FAILED(m_pModelCom->Render((_uint)i)))
			return E_FAIL;
	}
	return S_OK;
}

void CMapSection::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = m_strProtoTag;
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
	if (nullptr == m_pTransformCom)
		return;

	_matrix CombinedWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	if (nullptr != m_pParentMatrix)
		CombinedWorld *= XMLoadFloat4x4(m_pParentMatrix);

	XMStoreFloat4x4(&m_CombinedWorldMatrix, CombinedWorld);
	Refresh_WorldBounds();
}

#ifdef _DEBUG
void CMapSection::Reset_FrameProfile()
{
	// Profiling is temporarily disabled during the GameContent migration.
	m_Profile = {};
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
	IReflectable::Deserialize(j);

	if (j.contains("SectionRender") && j["SectionRender"].is_object())
	{
		const json& jRender = j["SectionRender"];

		if (jRender.contains("RenderID") && jRender["RenderID"].is_number_integer())
			m_eRenderID = static_cast<RENDERID>(jRender["RenderID"].get<_int>());
	}
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
	if (nullptr == m_pModelCom)
	{
		m_LocalBounds = Make_DefaultAABB();
		return;
	}

	_float3 vMin{}, vMax{};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	if (!Is_ValidAABB(vMin, vMax))
	{
		m_LocalBounds = Make_DefaultAABB();
		return;
	}

	m_LocalBounds = Make_AABB_FromMinMax(vMin, vMax);
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
	__super::Free();

	if (m_pColliderActor)
	{ 
		m_pGameInstance_Proxy->Remove_StaticActor(m_pColliderActor);
		m_pColliderActor = nullptr;
	}
}

NS_END