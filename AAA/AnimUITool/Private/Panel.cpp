#include "Panel.h"
#include "GameInstance.h"

CPanel::CPanel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance_Proxy { CGameInstance::GetProxy()}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CPanel::Initialize(CPanel_Manager* pPanelManager)
{
	m_pPanel_Manager = pPanelManager;
	return S_OK;
}

void CPanel::Free()
{
	__super::Free();


	Safe_Release(m_pGameInstance_Proxy);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
