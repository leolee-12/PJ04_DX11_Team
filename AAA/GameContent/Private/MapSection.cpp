#include "MapSection.h"

#include <exception>
#include <Windows.h>

#include "Engine_Function.h"
#include "GameInstance_Proxy.h"
#include "Model.h"

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
	: CMapObject { Prototype }
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
	m_eSectionType = pDesc->eSectionType;
	m_eRenderID = pDesc->eRenderID;
	m_bCastShadow = pDesc->bCastShadow;
	m_bEnableCulling = pDesc->bEnableCulling;
	m_bRenderable = pDesc->bRenderable;

	if (m_bRenderable)
	{
		if (FAILED(Ready_ModelPrototype(pDesc)))
			return E_FAIL;

		if (FAILED(__super::Initialize(pArg)))
			return E_FAIL;

		m_CombinedWorldMatrix = *m_pTransformCom->Get_WorldMatrixPtr();
		Update_LocalBounds();
		Refresh_WorldBounds();
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
	return S_OK;
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

const _tchar* CMapSection::Get_ModelProtoTag() const
{
	return m_strModelProtoTag.c_str();
}

HRESULT CMapSection::Bind_WorldMatrix()
{
	return m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix);
}

HRESULT CMapSection::Ready_ModelPrototype(const MAP_SECTION_DESC* pDesc)
{
	if (nullptr == pDesc)
		return E_FAIL;

	if (!m_pGameInstance_Proxy->Has_Prototype(pDesc->iModelProtoLevel, pDesc->strModelProtoTag))
	{
		if (pDesc->strModelPath.empty())
			return E_FAIL;

		const string strModelPath = WstrToStr(pDesc->strModelPath);
		CModel* pModelPrototype = nullptr;
		try
		{
			pModelPrototype = CModel::Create(
				m_pDevice,
				m_pContext,
				MODEL::MAP,
				strModelPath.c_str(),
				XMMatrixRotationY(XMConvertToRadians(180.f)));
		}
		catch (const std::exception& e)
		{
			Log_MapSectionWarning(
				"MapSection model creation exception: section=" + WstrToStr(pDesc->strSectionName)
				+ " path=" + strModelPath
				+ " reason=" + e.what());
			return E_FAIL;
		}

		if (nullptr == pModelPrototype)
			return E_FAIL;

		if (FAILED(m_pGameInstance_Proxy->Add_Prototype(
			pDesc->iModelProtoLevel,
			pDesc->strModelProtoTag.c_str(),
			pModelPrototype)))
		{
			Safe_Release(pModelPrototype);
			return E_FAIL;
		}
	}

	return S_OK;
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

void CMapSection::Free()
{
	__super::Free();
}

NS_END
