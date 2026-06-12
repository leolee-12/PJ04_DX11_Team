#pragma once
#include "AnimUITool_Defines.h"
#include "Panel.h"

NS_BEGIN(AnimUITool)
namespace fs = filesystem;

class CPanel_Browser final : public CPanel
{
private:
    CPanel_Browser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~CPanel_Browser() = default;

public:
    virtual HRESULT Initialize(CPanel_Manager* pPanelManager) override;   
    virtual void    Render() override;

    const fs::path& Get_SelectedPath() const { return m_SelectedPath; }

private:
    void            Refresh();
    void            Render_Breadcrumb();
    void            Render_Contents();
    const char*     Get_FileIcon(const fs::path& ext) const;

private:
    fs::path                        m_RootPath;
    fs::path                        m_CurrentPath;
    fs::path                        m_SelectedPath;
    std::vector<fs::directory_entry> m_Directories;
    std::vector<fs::directory_entry> m_Files;
    _bool                           m_bNeedRefresh = { true };

public:
    static CPanel_Browser* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
protected:
    virtual void    Free() override;
};

NS_END