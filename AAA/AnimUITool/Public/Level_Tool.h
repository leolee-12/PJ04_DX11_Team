#pragma once
#include "AnimUITool_Defines.h"
#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
class CUIContainerObject;
NS_END

NS_BEGIN(AnimUITool)

class CEditCamera;
class CEdit_Grid;

class CLevel_Tool final : public Engine::CLevel
{
private:
    CLevel_Tool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~CLevel_Tool() = default;

public:
    virtual HRESULT             Initialize() override;
    virtual void                Update(_float fTimeDelta) override;
    virtual HRESULT             Render() override;

    const vector<CUIContainerObject*>& Get_UIContainers() const { return m_UIContainers; }

    void                        Set_CameraActive(_bool b);
    void                        Set_GridVisible(_bool bVisible) { m_bGridVisible = bVisible; }
    void                        Set_PreviewVisible(_bool bVisible);

    CGameObject*                Load_Preview(const _wstring& strYshPath);
    void                        Clear_Preview();
    void                        Recalc_CameraProj();

private:
    CEditCamera*                m_pCamera = { nullptr };  
    CEdit_Grid*                 m_pGrid = { nullptr };   
    CGameObject*                m_pPreview = { nullptr };
    map<_wstring, _wstring>     m_ModelTags;
    _uint                       m_iTagCounter = { 0 };
    _bool                       m_bGridVisible = { true };
    vector<CUIContainerObject*> m_UIContainers;

private:
    virtual HRESULT             Ready_Events() override { return S_OK; }
    HRESULT                     Ready_Lights();
    HRESULT                     Ready_Camera();
    HRESULT                     Ready_Grid();
    HRESULT                     Ready_PreviewShaders();
    HRESULT                     Ready_TestUI();

    void                        Track_UIContainer(CGameObject* pObject);

public:
    static CLevel_Tool*         Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual void                Free() override;
};

NS_END