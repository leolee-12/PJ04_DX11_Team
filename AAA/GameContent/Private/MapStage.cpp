#include "MapStage.h"
#include "MapGimmick_Defines.h"
#include "GameContent_Log.h"

#include "GameInstance_Proxy.h"
#include "Math_Utils.h"

NS_BEGIN(Client)

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

	for (CMapSection* pSection : m_Sections)
	{
		if (nullptr == pSection)
			return E_FAIL;
	}

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

HRESULT CMapStage::Ready_Events()
{
	Subscribe_Event(EventTag::MapGimmick_SectionBreak,
		[this](void* pData)
		{
			const MAP_GIMMICK_BREAK_EVENT* pEvent = static_cast<const MAP_GIMMICK_BREAK_EVENT*>(pData);
			if (nullptr == pEvent || nullptr == pEvent->pEntry)
				return;
			if (m_strStageName != pEvent->pEntry->pStageName)
				return;

			On_GimmickSectionBreak(pEvent->pEntry->pShellSectionName);
		});

	return S_OK;
}


HRESULT CMapStage::Ready_Sections(const MAP_STAGE_DESC* pDesc)
{
	if (nullptr == pDesc)
		return E_FAIL;

	unordered_set<_wstring> SectionNames;

	for (const MAP_SECTION_DESC& SectionDesc : pDesc->SectionDescs)
	{
		if (!SectionNames.emplace(SectionDesc.strSectionName).second)
		{
			Log_GameContentWarning("MapStage duplicated section name: "
				+ WstrToStr(m_strStageName)
				+ "/" + WstrToStr(SectionDesc.strSectionName));
		}

		CBase* pBase = m_pGameInstance_Proxy->Clone_Prototype(
			PROTOTYPE::GAMEOBJECT,
			pDesc->iSectionProtoLevel,
			CMapSection::PROTOTYPE_TAG,
			const_cast<MAP_SECTION_DESC*>(&SectionDesc));

		CMapSection* pSection = dynamic_cast<CMapSection*>(pBase);
		if (nullptr == pSection)
		{
			Log_GameContentWarning("MapStage section clone failed: " + WstrToStr(m_strStageName)
				+ "/" + WstrToStr(SectionDesc.strSectionName));
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

		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, pSection);
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, pSection);
	}
}

void CMapStage::On_GimmickSectionBreak(const _tchar* pShellSectionName)
{
	if (nullptr == pShellSectionName)
		return;

	for (CMapSection* pSection : m_Sections)
	{
		if (pSection->Get_SectionName() != pShellSectionName)
			continue;

		pSection->Deactivate();
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
