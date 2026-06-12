#pragma once

#include "AnimUITool_Defines.h"
#include "Panel.h"

NS_BEGIN(Engine)
class CTexture;
NS_END

NS_BEGIN(AnimUITool)
namespace fs = filesystem;

class CPanel_Preview final : public CPanel
{
private:
    CPanel_Preview(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~CPanel_Preview() = default;

public:
    virtual HRESULT             Initialize(CPanel_Manager* pPanelManager) override;
    virtual void                Render() override;

private:
    void                        Load_Texture(const fs::path& path);
    void                        Clear_Texture();
    _bool                       Is_TextureFile(const fs::path& path) const;

private:
    fs::path                    m_PreviewPath;
    CTexture*                   m_pTexture = { nullptr };
    ID3D11ShaderResourceView*   m_pSRV = { nullptr };
    _uint                       m_iWidth = {};
    _uint                       m_iHeight = {};
    _bool                       m_bUnsupported = { false };

public:
    static CPanel_Preview*      Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
    virtual void                Free() override;
};

NS_END