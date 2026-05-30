#pragma once
#include "AnimUITool_Defines.h"
#include "Level.h"

NS_BEGIN(AnimUITool)

class CEditCamera;
class CEdit_Grid;

class CLevel_Tool final : public Engine::CLevel
{
private:
    CLevel_Tool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~CLevel_Tool() = default;

public:
    virtual HRESULT     Initialize() override;
    virtual void        Update(_float fTimeDelta) override;
    virtual HRESULT     Render() override;

    void                Set_CameraActive(_bool b);

private:
    CEditCamera*        m_pCamera = { nullptr };  
    CEdit_Grid*         m_pGrid = { nullptr };   

private:
    virtual HRESULT     Ready_Events() override { return S_OK; }
    HRESULT             Ready_Lights();
    HRESULT             Ready_Camera();
    HRESULT             Ready_Grid();

public:
    static CLevel_Tool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual void        Free() override;
};

NS_END