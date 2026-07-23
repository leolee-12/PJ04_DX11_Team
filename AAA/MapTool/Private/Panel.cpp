#include "Panel.h"
#include "GameInstance.h"

CPanel::CPanel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGI_Proxy { CGameInstance::GetProxy() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CPanel::Initialize(CPanel_Manager* pPanelManager)
{
	m_pPanel_Manager = pPanelManager;
	return S_OK;
}

_bool CPanel::Begin_Panel()
{
	return ImGui::Begin(m_szName, &m_bOpen, m_iWindowFlags);
}

void CPanel::End_Panel()
{
	ImGui::End();
}

void CPanel::Free()
{
	__super::Free();

	Safe_Release(m_pGI_Proxy);
	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
}
