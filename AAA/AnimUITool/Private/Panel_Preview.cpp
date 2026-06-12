#include "Panel_Preview.h"
#include "Panel_Manager.h"
#include "Panel_Browser.h"
#include "Texture.h"
#include "imgui.h"

using namespace AnimUITool;

static std::string LowerExt(const fs::path& path)
{
    std::string ext = path.extension().string();
    for (auto& c : ext)
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return ext;
}

CPanel_Preview::CPanel_Preview(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "Preview");
}

HRESULT CPanel_Preview::Initialize(CPanel_Manager* pPanelManager)
{
    return __super::Initialize(pPanelManager);
}

_bool CPanel_Preview::Is_TextureFile(const fs::path& path) const
{
    const std::string ext = LowerExt(path);
    return ext == ".png" || ext == ".dds";
}

void CPanel_Preview::Load_Texture(const fs::path& path)
{
    Clear_Texture();

    if (!Is_TextureFile(path))
        return;

    m_PreviewPath = path;
    m_pTexture = CTexture::Create(m_pDevice, m_pContext, path.wstring().c_str(), 1);
    if (!m_pTexture)
        return;

    m_pSRV = m_pTexture->Get_SRV(0);
    m_pTexture->Get_TextureSize(0, &m_iWidth, &m_iHeight);

    D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
    if (m_pSRV)
    {
        m_pSRV->GetDesc(&desc);
        m_bUnsupported = desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D;
    }
}

void CPanel_Preview::Render()
{
    ImGui::Begin(m_szName);

    auto* pBrowser =
        dynamic_cast<CPanel_Browser*>(m_pPanel_Manager->Get_Panel(L"Browser"));

    const fs::path selected = pBrowser ? pBrowser->Get_SelectedPath() : fs::path{};
    if (selected != m_PreviewPath)
        Load_Texture(selected);

    if (!m_pTexture)
    {
        ImGui::TextDisabled("No texture selected.");
        ImGui::End();
        return;
    }

    ImGui::Text("%s", m_PreviewPath.filename().string().c_str());
    ImGui::Text("%u x %u", m_iWidth, m_iHeight);

    if (m_bUnsupported)
    {
        ImGui::TextDisabled("Texture2DArray DDS preview is not supported yet.");
        ImGui::End();
        return;
    }

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float scale = min(avail.x / max(1.f, static_cast<float>(m_iWidth)),
        avail.y / max(1.f, static_cast<float>(m_iHeight)));
    scale = min(scale, 1.f);

    ImVec2 size(static_cast<float>(m_iWidth) * scale,
        static_cast<float>(m_iHeight) * scale);

    ImGui::Image((ImTextureID)m_pSRV, size);

    ImGui::End();
}

void CPanel_Preview::Clear_Texture()
{
    Safe_Release(m_pTexture);
    m_pSRV = nullptr;
    m_iWidth = 0;
}

CPanel_Preview* CPanel_Preview::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CPanel_Preview(pDevice, pContext);
}

void CPanel_Preview::Free()
{
    Clear_Texture();
    __super::Free();
}