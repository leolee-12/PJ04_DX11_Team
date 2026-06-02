#include "MapStage.h"

#ifdef _DEBUG
#include <Windows.h>
#endif

#include "CullingContext.h"
#include "GameInstance_Proxy.h"

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

CMapStage::CMapStage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CMapStage::CMapStage(const CMapStage& Prototype)
	: CGameObject { Prototype }
	, m_strProtoTag { Prototype.m_strProtoTag }
{
}

HRESULT CMapStage::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CMapStage::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const MAP_STAGE_DESC* pDesc = static_cast<const MAP_STAGE_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strStageName = pDesc->strStageName;
	m_iSectionProtoLevel = pDesc->iSectionProtoLevel;

	if (nullptr == m_pMainViewCullingContext)
	{
		m_pMainViewCullingContext = ::MapTool::CCullingContext::Create();
		if (nullptr == m_pMainViewCullingContext)
			return E_FAIL;
	}

	if (FAILED(Ready_Sections(pDesc)))
		return E_FAIL;

	return S_OK;
}

void CMapStage::Late_Update(_float fTimeDelta)
{
#ifdef _DEBUG
	const double dStart = Now_PerfCounter();

	Reset_ProfileFrame();
#endif
	Submit_VisibleSections();

#ifdef _DEBUG
	m_Profile.dStageLateUpdateCpuMs = PerfCounter_ToMs(dStart, Now_PerfCounter());
#endif
}

CGameObject* CMapStage::Clone(void* pArg)
{
	CMapStage* pInstance = new CMapStage(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMapStage");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMapStage::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = m_strProtoTag;
}

HRESULT CMapStage::Ready_Sections(const MAP_STAGE_DESC* pDesc)
{
	if (nullptr == pDesc)
		return E_FAIL;

	for (const MAP_SECTION_DESC& SectionDesc : pDesc->SectionDescs)
	{
		CBase* pBase = m_pGameInstance_Proxy->Clone_Prototype(
			PROTOTYPE::GAMEOBJECT,
			pDesc->iSectionProtoLevel,
			CMapSection::PROTOTYPE_TAG,
			const_cast<MAP_SECTION_DESC*>(&SectionDesc));

		CMapSection* pSection = dynamic_cast<CMapSection*>(pBase);
		if (nullptr == pSection)
		{
			Safe_Release(pBase);
			return E_FAIL;
		}

		m_Sections.push_back(pSection);
	}

	return S_OK;
}

#ifdef _DEBUG
void CMapStage::Reset_ProfileFrame()
{
	const _uint iNextFrame = m_Profile.iFrameIndex + 1;
	m_Profile = {};
	m_Profile.iFrameIndex = iNextFrame;
	m_Profile.iTotalSections = static_cast<_uint>(m_Sections.size());

	for (CMapSection* pSection : m_Sections)
	{
		if (nullptr != pSection)
			pSection->Reset_FrameProfile();
	}
}
#endif

void CMapStage::Submit_VisibleSections()
{
#ifdef _DEBUG
	const double dStart = Now_PerfCounter();
#endif

	const _float4x4* pViewMatrix = m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC);
	const _float4x4* pProjMatrix = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);
	const _bool bHasFrustum = (nullptr != m_pMainViewCullingContext) ?
		m_pMainViewCullingContext->Update_FromViewProj(pViewMatrix, pProjMatrix) :
		false;

	for (CMapSection* pSection : m_Sections)
	{
		if (nullptr == pSection)
			continue;

#ifdef _DEBUG
		const MAP_SECTION_TYPE eType = pSection->Get_SectionType();
		const _uint iTypeIndex = static_cast<_uint>(eType);
		if (iTypeIndex < MAP_SECTION_TYPE_COUNT)
			++m_Profile.iSectionTypeCount[iTypeIndex];
#endif

		if (bHasFrustum &&
			nullptr != m_pMainViewCullingContext &&
			m_pMainViewCullingContext->Should_CullAABB(pSection->Is_Culling(), pSection->Get_WorldBounds()))
		{
#ifdef _DEBUG
			++m_Profile.iCulledSections;
#endif
			continue;
		}

#ifdef _DEBUG
		++m_Profile.iVisibleSections;
#endif

		if (!pSection->Is_Renderable())
			continue;

		const RENDERID eRenderID = pSection->Get_RenderID();
		m_pGameInstance_Proxy->Add_RenderGroup(eRenderID, pSection);
#ifdef _DEBUG
		Count_Submitted(eRenderID);
#endif

		if (pSection->Is_ShadowCaster())
		{
			m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, pSection);
#ifdef _DEBUG
			Count_Submitted(RENDERID::SHADOW);
#endif
		}
	}

#ifdef _DEBUG
	m_Profile.dCullingCpuMs = PerfCounter_ToMs(dStart, Now_PerfCounter());
#endif
}

#ifdef _DEBUG
void CMapStage::Count_Submitted(RENDERID eRenderID)
{
	switch (eRenderID)
	{
	case RENDERID::NONBLEND:
		++m_Profile.iSubmittedNonBlend;
		break;
	case RENDERID::BLEND:
		++m_Profile.iSubmittedBlend;
		break;
	case RENDERID::SHADOW:
		++m_Profile.iSubmittedShadow;
		break;
	default:
		break;
	}
}
#endif

CMapStage* CMapStage::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapStage* pInstance = new CMapStage(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMapStage");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMapStage::Free()
{
	for (CMapSection*& pSection : m_Sections)
		Safe_Release(pSection);
	m_Sections.clear();

	Safe_Release(m_pMainViewCullingContext);

	__super::Free();
}

NS_END
