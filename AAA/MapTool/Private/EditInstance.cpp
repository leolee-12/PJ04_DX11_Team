#include "EditInstance.h"
#include "Level_Edit.h"
#include "Panel_Manager.h"

IMPLEMENT_SINGLETON(CEditInstance)

HRESULT CEditInstance::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	if (nullptr != m_pPanelManager)
		return S_OK;

	m_pPanelManager = CPanel_Manager::Create(pDevice, pContext);
	if (nullptr == m_pPanelManager)
		return E_FAIL;

	return S_OK;
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

	// 약참조뿐이므로 Release 하지 않는다. (레벨=엔진, SRV=App 소유)
	m_pLevel = nullptr;
	m_pSceneSRV = nullptr;

	__super::Free();
}
