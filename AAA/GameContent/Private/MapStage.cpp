#include <chrono>

#include "MapStage.h"

#include "GameInstance_Proxy.h"

NS_BEGIN(Client)

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
	m_pTransformCom->Rotation(
		XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f)));

	if (FAILED(Ready_Sections(pDesc)))
		return E_FAIL;

	Refresh_SectionTransforms();
	return S_OK;
}

void CMapStage::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

#ifdef _DEBUG
	Reset_ProfileFrame();
#endif

	Refresh_SectionTransforms();

#ifdef _DEBUG
	const auto CullingBegin = std::chrono::steady_clock::now();
#endif

	Submit_VisibleSections();

#ifdef _DEBUG
	const auto CullingEnd = std::chrono::steady_clock::now();
	m_Profile.dCullingCpuMs =
		std::chrono::duration<double, std::milli>(CullingEnd - CullingBegin).count();
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

		pSection->Set_ParentMatrix(m_pTransformCom->Get_WorldMatrixPtr());
		pSection->Refresh_CombinedWorldMatrix();
		m_Sections.push_back(pSection);
	}

	return S_OK;
}

void CMapStage::Refresh_SectionTransforms() const
{
	for (CMapSection* pSection : m_Sections)
	{
		if (nullptr == pSection)
			continue;

		pSection->Refresh_CombinedWorldMatrix();
	}
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
		if (nullptr == pSection)
			continue;

		if (!pSection->Is_Renderable())
			continue;

		++m_Profile.iMainCandidateSections;

		if (pSection->Is_ShadowCaster())
			++m_Profile.iShadowCandidateSections;

		pSection->Reset_FrameProfile();
	}
}
#endif

void CMapStage::Submit_VisibleSections()
{
	for (CMapSection* pSection : m_Sections)
	{
		if (nullptr == pSection)
			continue;

		if (!pSection->Is_Renderable())
			continue;

		const _bool bVisibleMain =
			!m_pGameInstance_Proxy->Should_CullAABB(
				CULLING_VIEW::MAIN_CAMERA,
				pSection->Is_Culling(),
				pSection->Get_WorldBounds());

		if (bVisibleMain)
		{
			const RENDERID eRenderID = pSection->Get_RenderID();
			m_pGameInstance_Proxy->Add_RenderGroup(eRenderID, pSection);

#ifdef _DEBUG
			++m_Profile.iMainVisibleSections;
			++m_Profile.iMainSubmittedSections;
			Count_MainSubmitted(eRenderID);
#endif
		}
		else
		{
#ifdef _DEBUG
			++m_Profile.iMainCulledSections;
#endif
		}

		if (!pSection->Is_ShadowCaster())
			continue;

		const _bool bVisibleShadow =
			!m_pGameInstance_Proxy->Should_CullAABB(
				CULLING_VIEW::SHADOW_DIR,
				pSection->Is_Culling(),
				pSection->Get_WorldBounds());

		if (bVisibleShadow)
		{
			m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, pSection);

#ifdef _DEBUG
			++m_Profile.iShadowVisibleSections;
			++m_Profile.iShadowSubmittedSections;
#endif
		}
		else
		{
#ifdef _DEBUG
			++m_Profile.iShadowCulledSections;
#endif
		}
	}
}

#ifdef _DEBUG
void CMapStage::Count_MainSubmitted(RENDERID eRenderID)
{
	switch (eRenderID)
	{
	case RENDERID::NONBLEND:
		++m_Profile.iMainSubmittedNonBlend;
		break;

	case RENDERID::BLEND:
		++m_Profile.iMainSubmittedBlend;
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

	__super::Free();
}

NS_END
