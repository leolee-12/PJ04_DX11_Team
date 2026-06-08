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

void CPanel::Draw_OpaqueImage(ID3D11ShaderResourceView* pSRV, const ImVec2& vSize)
{
    if (nullptr == pSRV)
    {
        ImGui::Dummy(vSize);
        return;
    }

    if (nullptr == m_pOpaqueImageBlend)
    {
        D3D11_BLEND_DESC bd{};
        bd.RenderTarget[0].BlendEnable = FALSE;
        bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        if (SUCCEEDED(m_pDevice->CreateBlendState(&bd, &m_pOpaqueImageBlend)))
            m_OpaqueImageDraw = { m_pContext, m_pOpaqueImageBlend };
    }

    ImDrawList* pDrawList = ImGui::GetWindowDrawList();

    if (m_pOpaqueImageBlend)
        pDrawList->AddCallback(Disable_ImageBlend, &m_OpaqueImageDraw);

    ImGui::Image((ImTextureID)pSRV, vSize);

    if (m_pOpaqueImageBlend)
        pDrawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
}

void CPanel::Disable_ImageBlend(const ImDrawList*, const ImDrawCmd* cmd)
{
	auto* p = static_cast<VIEWPORT_DRAW*>(cmd->UserCallbackData);
	const float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };

	p->pContext->OMSetBlendState(p->pBlend, blendFactor, 0xffffffff);
}

void CPanel::Free()
{
	__super::Free();

    Safe_Release(m_pOpaqueImageBlend);

    Safe_Release(m_pGameInstance_Proxy);
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
}
