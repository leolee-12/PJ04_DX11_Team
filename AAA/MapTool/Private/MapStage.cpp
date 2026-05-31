#include "MapStage.h"

#include <Windows.h>

#include "GameInstance_Proxy.h"

NS_BEGIN(Client)

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
		LARGE_INTEGER freq{};
		QueryPerformanceFrequency(&freq);
		if (0 == freq.QuadPart)
			return 0.0;

		return (dEnd - dStart) * 1000.0 / static_cast<double>(freq.QuadPart);
	}
}

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

	if (FAILED(Ready_Sections(pDesc)))
		return E_FAIL;

	return S_OK;
}

void CMapStage::Late_Update(_float fTimeDelta)
{
	const double dStart = Now_PerfCounter();

	Reset_ProfileFrame();
	Submit_VisibleSections();
	Collect_SectionRenderProfile();

	m_Profile.dStageLateUpdateCpuMs = PerfCounter_ToMs(dStart, Now_PerfCounter());
}

HRESULT CMapStage::Render()
{
	return S_OK;
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

void CMapStage::Reset_ProfileFrame()
{
	const _uint iNextFrame = m_Profile.iFrameIndex + 1;
	m_Profile = {};
	m_Profile.iFrameIndex = iNextFrame;
	m_Profile.iTotalSections = static_cast<_uint>(m_Sections.size());
}

void CMapStage::Submit_VisibleSections()
{
	const double dStart = Now_PerfCounter();

	BoundingFrustum WorldFrustum{};
	const _bool bHasFrustum = Build_WorldFrustum(&WorldFrustum);

	for (CMapSection* pSection : m_Sections)
	{
		if (nullptr == pSection)
			continue;

		pSection->Refresh_WorldBounds();

		const MAP_SECTION_TYPE eType = pSection->Get_SectionType();
		const _uint iTypeIndex = static_cast<_uint>(eType);
		if (iTypeIndex < MAP_SECTION_TYPE_COUNT)
			++m_Profile.iSectionTypeCount[iTypeIndex];

		if (bHasFrustum && pSection->Is_Culling() && !WorldFrustum.Intersects(pSection->Get_WorldBounds()))
		{
			++m_Profile.iCulledSections;
			continue;
		}

		++m_Profile.iVisibleSections;

		const RENDERID eRenderID = pSection->Get_RenderID();
		m_pGameInstance_Proxy->Add_RenderGroup(eRenderID, pSection);
		Count_Submitted(eRenderID);

		if (pSection->Is_ShadowCaster())
		{
			m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, pSection);
			Count_Submitted(RENDERID::SHADOW);
		}
	}

	m_Profile.dCullingCpuMs = PerfCounter_ToMs(dStart, Now_PerfCounter());
}

void CMapStage::Collect_SectionRenderProfile()
{
	for (CMapSection* pSection : m_Sections)
	{
		if (nullptr == pSection)
			continue;

		const MAP_SECTION_PROFILE& SectionProfile = pSection->Get_Profile();
		m_Profile.dSectionRenderCpuMs += SectionProfile.dRenderCpuMs;
		m_Profile.iEstimatedDrawCalls += SectionProfile.iEstimatedDrawCalls;
		m_Profile.iEstimatedTriangles += SectionProfile.iEstimatedTriangles;
	}
}

_bool CMapStage::Build_WorldFrustum(BoundingFrustum* pOutFrustum) const
{
	if (nullptr == pOutFrustum || nullptr == m_pGameInstance_Proxy)
		return false;

	const _float4x4* pView = m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC);
	const _float4x4* pProj = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);
	if (nullptr == pView || nullptr == pProj)
		return false;

	BoundingFrustum LocalFrustum{};
	BoundingFrustum::CreateFromMatrix(LocalFrustum, XMLoadFloat4x4(pProj));
	LocalFrustum.Transform(*pOutFrustum, XMMatrixInverse(nullptr, XMLoadFloat4x4(pView)));

	return true;
}

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

	__super::Free();
}

NS_END
