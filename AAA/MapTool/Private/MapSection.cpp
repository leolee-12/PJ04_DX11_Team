#include "MapSection.h"

#ifdef _DEBUG
#include <Windows.h>
#endif

#include "CullingContext.h"
#include "GameContent_const.h"
#include "GameInstance_Proxy.h"
#include "Model.h"
#include "Shader.h"

NS_BEGIN(Client)

#ifdef _DEBUG
namespace
{
	double Now_PerfCounter()
	{
		LARGE_INTEGER counter{};
		QueryPerformanceCounter(&counter);
		return static_cast<double>(counter.QuadPart);
	}

	double PerfCounter_ToMs(double dStart, double dEnd)
	{
		static const double dFrequency = []() -> double
			{
				LARGE_INTEGER freq{};
				QueryPerformanceFrequency(&freq);
				return static_cast<double>(freq.QuadPart);
			}();

		if (0.0 == dFrequency)
			return 0.0;

		return (dEnd - dStart) * 1000.0 / dFrequency;
	}
}
#endif

CMapSection::CMapSection(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CMapSection::CMapSection(const CMapSection& Prototype)
	: CGameObject { Prototype }
	, m_strProtoTag { Prototype.m_strProtoTag }
{
}

HRESULT CMapSection::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CMapSection::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const MAP_SECTION_DESC* pDesc = static_cast<const MAP_SECTION_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strSectionName = pDesc->strSectionName;
	m_eSectionType = pDesc->eSectionType;
	m_eRenderID = pDesc->eRenderID;
	m_bCastShadow = pDesc->bCastShadow;
	m_bEnableCulling = pDesc->bEnableCulling;
	m_bRenderable = pDesc->bRenderable;

	if (FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	Update_LocalBounds();
	Refresh_WorldBounds();

	return S_OK;
}

HRESULT CMapSection::Render()
{
#ifdef _DEBUG
	const double dStart = Now_PerfCounter();

	m_Profile = {};
#endif

	if (FAILED(Bind_ShaderResources()))
	{
#ifdef _DEBUG
		m_Profile.dRenderCpuMs = PerfCounter_ToMs(dStart, Now_PerfCounter());
#endif
		return E_FAIL;
	}

	const size_t iNumMeshes = m_pModelCom ? m_pModelCom->Get_NumMeshes() : 0;
	const _uint iPassIndex = Get_RenderPassIndex();

	for (size_t i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", static_cast<_uint>(i), MTEX_TYPE::DIFFUSE, 0)))
			continue;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", static_cast<_uint>(i), MTEX_TYPE::NORMALS, 0)))
			continue;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", static_cast<_uint>(i), MTEX_TYPE::METALNESS, 0)))
			continue;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_UnkownTexture", static_cast<_uint>(i), MTEX_TYPE::UNKNOWN, 0)))
			continue;

		if (FAILED(m_pShaderCom->Begin(iPassIndex)))
		{
#ifdef _DEBUG
			m_Profile.dRenderCpuMs = PerfCounter_ToMs(dStart, Now_PerfCounter());
#endif
			return E_FAIL;
		}

		if (FAILED(m_pModelCom->Render(static_cast<_uint>(i))))
		{
#ifdef _DEBUG
			m_Profile.dRenderCpuMs = PerfCounter_ToMs(dStart, Now_PerfCounter());
#endif
			return E_FAIL;
		}

#ifdef _DEBUG
		++m_Profile.iEstimatedDrawCalls;
#endif
	}

#ifdef _DEBUG
	m_Profile.dRenderCpuMs = PerfCounter_ToMs(dStart, Now_PerfCounter());
#endif
	return S_OK;
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
	if (nullptr == m_pTransformCom)
		return;

	::MapTool::CCullingContext::Transform_AABB(
		m_LocalBounds,
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()),
		&m_WorldBounds);
}

#ifdef _DEBUG
void CMapSection::Reset_FrameProfile()
{
	m_Profile = {};
}
#endif

HRESULT CMapSection::Ready_Components(const MAP_SECTION_DESC* pDesc)
{
	if (nullptr == pDesc)
		return E_FAIL;

	m_pShaderCom = Add_Component<CShader>(
		Shader_NonAnimMesh_PBR.iLevelID,
		Shader_NonAnimMesh_PBR.szProtoTag,
		TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(
		pDesc->iModelProtoLevel,
		pDesc->strModelProtoTag,
		TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CMapSection::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom || nullptr == m_pTransformCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	return S_OK;
}

void CMapSection::Update_LocalBounds()
{
	if (nullptr == m_pModelCom)
	{
		m_LocalBounds = ::MapTool::CCullingContext::Make_DefaultAABB();
		return;
	}

	_float3 vMin{}, vMax{};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	if (!::MapTool::CCullingContext::Is_ValidAABB(vMin, vMax))
	{
		m_LocalBounds = ::MapTool::CCullingContext::Make_DefaultAABB();
		return;
	}

	m_LocalBounds = ::MapTool::CCullingContext::Make_AABB_FromMinMax(vMin, vMax);
}

_uint CMapSection::Get_RenderPassIndex() const
{
	return 1;
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
