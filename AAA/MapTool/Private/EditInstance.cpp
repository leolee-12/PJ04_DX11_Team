#include "EditInstance.h"
#include "ImGui_Manager.h"
#include "Level_Edit.h"
#include "Panel_Manager.h"

IMPLEMENT_SINGLETON(CEditInstance)

HRESULT CEditInstance::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	if (nullptr == m_pImGui_Manager)
	{
		m_pImGui_Manager = CImGui_Manager::Create(pDevice, pContext);
		if (nullptr == m_pImGui_Manager)
			return E_FAIL;
	}

	if (nullptr == m_pPanelManager)
	{
		m_pPanelManager = CPanel_Manager::Create(pDevice, pContext);
		if (nullptr == m_pPanelManager)
			return E_FAIL;
	}

	return S_OK;
}

void CEditInstance::ImGui_BeginFrame()
{
	if (nullptr == m_pImGui_Manager)
		return;

	m_pImGui_Manager->Begin_Frame();
}

void CEditInstance::Render_UI()
{
	if (m_bLoading)
	{
		if (nullptr != m_pImGui_Manager)
			m_pImGui_Manager->Draw_LoadingOverlay(m_fLoadProgress);
		return;
	}

	Render_Panels();
}

void CEditInstance::ImGui_Render()
{
	if (nullptr == m_pImGui_Manager)
		return;

	m_pImGui_Manager->Render();
}

void CEditInstance::Update_Panels(_float fTimeDelta)
{
	if (nullptr == m_pPanelManager)
		return;

	m_pPanelManager->Update(fTimeDelta);
}

void CEditInstance::Render_Panels()
{
	if (nullptr == m_pPanelManager)
		return;

	m_pPanelManager->Render();
}

CPanel* CEditInstance::Get_Panel(const _wstring& strPanelTag)
{
	return m_pPanelManager ? m_pPanelManager->Get_Panel(strPanelTag) : nullptr;
}

CGameObject* CEditInstance::Get_Selected() const
{
	return m_pLevel ? m_pLevel->Get_Selected() : nullptr;
}

void CEditInstance::Free()
{
	Safe_Release(m_pPanelManager);
	Safe_Release(m_pImGui_Manager);

	// Level and scene SRV are weak references owned by Engine/App.
	m_pLevel = nullptr;
	m_pSceneSRV = nullptr;

	__super::Free();
}
