#include "MapStage.h"

#include "GameInstance_Proxy.h"

#include <chrono>
#include <cstring>

NS_BEGIN(Client)

namespace
{
	constexpr _bool ENABLE_MAP_SECTION_SHADOW = false;

	_bool Is_SameMatrix(const _float4x4& lhs, const _float4x4& rhs)
	{
		return 0 == memcmp(&lhs, &rhs, sizeof(_float4x4));
	}
}

CMapStage::CMapStage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CMapStage::CMapStage(const CMapStage& Prototype)
	: CGameObject(Prototype)
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
	m_strStageName = pDesc->strStageName;
	m_iSectionProtoLevel = pDesc->iSectionProtoLevel;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Sections(pDesc)))
		return E_FAIL;

	m_bSnapshotValid = false;
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

void CMapStage::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = m_strProtoTag;
}

#ifdef _DEBUG
void CMapStage::Set_EditorSoloSection(CMapSection* pSection)
{
	m_pEditorSoloSection = pSection;
}

void CMapStage::Clear_EditorSoloSection()
{
	m_pEditorSoloSection = nullptr;
}

_bool CMapStage::Should_RenderSection(const CMapSection* pSection) const
{
	if (nullptr == m_pEditorSoloSection)
		return true;

	return pSection == m_pEditorSoloSection;
}
void CMapStage::Clear_EditorSoloMeshAllSections()
{
	for (CMapSection* pSection : m_Sections)
	{
		if (nullptr != pSection)
			pSection->Clear_EditorSoloMesh();
	}
}
#endif

json CMapStage::Serialize() const
{
	json j = __super::Serialize();
	j["StageName"] = WstrToStr(m_strStageName);
	j["Sections"] = json::array();

	for (const CMapSection* pSection : m_Sections)
	{
		if (nullptr == pSection)
			continue;

		j["Sections"].push_back(pSection->Serialize_SectionState());
	}

	return j;
}

void CMapStage::Deserialize_Internal(const json& j)
{
	__super::Deserialize_Internal(j);

	if (j.contains("StageName") && j["StageName"].is_string())
		m_strStageName = StrToWstr(j["StageName"].get<string>());

	if (j.contains("Sections") && j["Sections"].is_array())
	{
		unordered_map<wstring, CMapSection*> SectionByName;
		for (CMapSection* pSection : m_Sections)
		{
			if (nullptr == pSection)
				continue;

			SectionByName.emplace(pSection->Get_SectionName(), pSection);
		}

		for (const auto& jSection : j["Sections"])
		{
			if (!jSection.is_object())
				continue;
			if (!jSection.contains("SectionName") || !jSection["SectionName"].is_string())
				continue;

			const wstring strSectionName = StrToWstr(jSection["SectionName"].get<string>());
			auto iter = SectionByName.find(strSectionName);
			if (iter == SectionByName.end())
				continue;

			iter->second->Deserialize_SectionState(jSection);
		}
	}

	m_bSnapshotValid = false;
	Refresh_SectionTransforms();
}

HRESULT CMapStage::Ready_Events()
{
	if (L"Stage1-2_MapStage" == m_strStageName)
	{
		Subscribe_Event(EventTag::Stage12_CarBreakWall,
			[this](void* pData)
			{
				UNREFERENCED_PARAMETER(pData);
				Stage12_CarBreakWall();
			});
	}

	return S_OK;
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
		m_Sections.push_back(pSection);
	}

	return S_OK;
}

void CMapStage::Refresh_SectionTransforms()
{
	if(nullptr == m_pTransformCom)
		return;

	const _float4x4* pCurrentWorld = m_pTransformCom->Get_WorldMatrixPtr();
	if (nullptr == pCurrentWorld)
		return;

	if (m_bSnapshotValid && Is_SameMatrix(m_LastWorldMatrix, *pCurrentWorld))
		return;

	for (CMapSection* pSection : m_Sections)
	{
		if (nullptr == pSection)
			continue;

		pSection->Refresh_CombinedWorldMatrix();
	}

	m_LastWorldMatrix = *pCurrentWorld;
	m_bSnapshotValid = true;
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

#ifdef _DEBUG
		if (!Should_RenderSection(pSection))
			continue;
#endif

		if (!pSection->Is_Renderable())
			continue;

		const _bool bVisibleMain =
			!m_pGameInstance_Proxy->Should_CullAABB(CULLING_VIEW::MAIN_CAMERA, pSection->Get_WorldBounds());

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

		if (!ENABLE_MAP_SECTION_SHADOW)
			continue;

		if (!pSection->Is_ShadowCaster())
			continue;

		const _bool bVisibleShadow =
			!m_pGameInstance_Proxy->Should_CullAABB(
				CULLING_VIEW::SHADOW_DIR,
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

void CMapStage::Stage12_CarBreakWall()
{
	for (CMapSection* pSection : m_Sections)
	{
		if (nullptr == pSection)
			continue;

		if (L"GsDefault_2" != pSection->Get_SectionName())
			continue;

		pSection->Set_Renderable(false);
		pSection->Set_RuntimeCollisionActorEnabled(false);
		return;
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

void CMapStage::Free()
{
#ifdef _DEBUG
	m_pEditorSoloSection = nullptr;
#endif

	for (CMapSection*& pSection : m_Sections)
		Safe_Release(pSection);
	m_Sections.clear();

	__super::Free();
}

NS_END
