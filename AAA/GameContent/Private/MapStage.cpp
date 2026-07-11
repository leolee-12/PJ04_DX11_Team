#include "MapStage.h"
#include "MapEvent_BreakWall.h"

#include "GameInstance_Proxy.h"
#include "Math_Utils.h"

NS_BEGIN(Client)

namespace
{
	constexpr const _tchar* STAGE12_BREAK_WALL_BASE_SECTION_NAME = L"GsDefault_2";
}

CMapStage::CMapStage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CMapStage::CMapStage(const CMapStage& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CMapStage::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const MAP_STAGE_DESC* pDesc = static_cast<const MAP_STAGE_DESC*>(pArg);
	m_strStageName = pDesc->strStageName;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Sections(pDesc)))
		return E_FAIL;

	m_bSnapshotValid = false;
	Refresh_SectionTransforms();

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapStage::Validate_Initialized()
{
	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom)
		return E_FAIL;
	if (m_strStageName.empty())
		return E_FAIL;
	if (!m_bSnapshotValid)
		return E_FAIL;

	_bool bHasStage12BreakWallBaseSection = false;

	for (CMapSection* pSection : m_Sections)
	{
		if (nullptr == pSection)
			return E_FAIL;

		if (STAGE12_BREAK_WALL_BASE_SECTION_NAME == pSection->Get_SectionName())
			bHasStage12BreakWallBaseSection = true;
	}

	if (CMapEvent_BreakWall::STAGE12_STAGE_NAME == m_strStageName && !bHasStage12BreakWallBaseSection)
		return E_FAIL;

	return S_OK;
}

void CMapStage::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	Refresh_SectionTransforms();
	Submit_VisibleSections();
}

void CMapStage::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
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
		j["Sections"].push_back(pSection->Serialize_SectionState());
	}

	return j;
}

void CMapStage::Deserialize_Internal(const json& j)
{
	const auto IterStageName = j.find("StageName");
	if (IterStageName != j.end())
	{
		if (!IterStageName->is_string())
			return;

		if (StrToWstr(IterStageName->get<string>()) != m_strStageName)
			return;
	}

	__super::Deserialize_Internal(j);

	if (j.contains("Sections") && j["Sections"].is_array())
	{
		unordered_map<wstring, CMapSection*> SectionByName;
		for (CMapSection* pSection : m_Sections)
		{
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
	if (CMapEvent_BreakWall::STAGE12_STAGE_NAME == m_strStageName)
	{
		Subscribe_Event(EventTag::Stage1_Step2_CarBreakMap,
			[this](void* pData)
			{
				UNREFERENCED_PARAMETER(pData);
				m_pGameInstance_Proxy->Play_SFX(L"GimmickWallStake_Strike.wav", 1.0f, ESoundBus::SFX);
				On_Stage12CarBreakWall();
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

		pSection->Set_StageName(m_strStageName);
		pSection->Set_ParentMatrix(m_pTransformCom->Get_WorldMatrixPtr());
		m_Sections.push_back(pSection);
	}

	return S_OK;
}

void CMapStage::Refresh_SectionTransforms()
{
	const _float4x4* pCurrentWorld = m_pTransformCom->Get_WorldMatrixPtr();

	if (m_bSnapshotValid && MathUtils::Is_NearlyEqualFloat4x4(m_LastWorldMatrix, *pCurrentWorld))
		return;

	for (CMapSection* pSection : m_Sections)
		pSection->Refresh_CombinedWorldMatrix();

	m_LastWorldMatrix = *pCurrentWorld;
	m_bSnapshotValid = true;
}

void CMapStage::Submit_VisibleSections()
{
	for (CMapSection* pSection : m_Sections)
	{
#ifdef _DEBUG
		if (!Should_RenderSection(pSection))
			continue;
#endif

		if (!pSection->Is_Renderable())
			continue;

		const _bool bVisibleMain =
			!pSection->Is_Culling()
			|| !m_pGameInstance_Proxy->Should_CullAABB(
				CULLING_VIEW::MAIN_CAMERA,
				pSection->Get_WorldBounds());

		if (bVisibleMain)
		{
			const RENDERID eRenderID = pSection->Get_RenderID();
			m_pGameInstance_Proxy->Add_RenderGroup(eRenderID, pSection);
		}

		// ESM policy:
		// Map sections must always submit their complete depth to the shadow pass.
		// Main-camera culling must not remove shadow casters, because off-camera sections can still affect visible shadow results.
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, pSection);
	}
}

void CMapStage::On_Stage12CarBreakWall()
{
	for (CMapSection* pSection : m_Sections)
	{
		if (STAGE12_BREAK_WALL_BASE_SECTION_NAME != pSection->Get_SectionName())
			continue;

		pSection->Set_Renderable(false);
		pSection->Set_UseCollMesh(false);
		return;
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
