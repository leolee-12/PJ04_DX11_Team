#pragma once
#include "Editor_Defines.h"
#include "Base.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "ImGuizmo.h"

#include "Camera_AreaData.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Editor)
class CLevel_Edit;

class CImGui_Manager final : public CBase
{
    DECLARE_SINGLETON(CImGui_Manager)
private:
    CImGui_Manager();
    virtual ~CImGui_Manager();

public:
    HRESULT ImGui_Initialize(ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext, CLevel_Edit* pLevelEdit,
        ID3D11ShaderResourceView** ppSRV);
    void ImGui_Render();

private:
    void Draw_Toolbar();
    void Draw_List();
    void Draw_Inspector();
    void Draw_Viewport();

    void Draw_AreaInspector(Client::CAM_AREA& A, _int idx);
    void Draw_NodeInspector();
    void Draw_KirbyInspector();
    void Draw_OffsetEditor(const _char* label, Client::CAM_OFFSET& off);

public:
    virtual void Free() override;

private:
    CLevel_Edit* m_pLevel_Edit = { nullptr };
    CGameInstance_Proxy* m_pGameInstance_Proxy = { nullptr };
    ImGuizmo::OPERATION       m_eGizmoOp = { ImGuizmo::TRANSLATE };
    ID3D11ShaderResourceView* m_pSRV = { nullptr };
    ID3D11DeviceContext* m_pContext = { nullptr };
    ID3D11BlendState* m_pOpaqueBlend = { nullptr };

    struct VIEWPORT_DRAW { ID3D11DeviceContext* pContext; ID3D11BlendState* pBlend; };
    VIEWPORT_DRAW m_ViewportDraw{};

    char m_szDocPath[260] = "../../Tools/Level0_Stage1_Step01_cam.json";

private:
    static void Viewport_DisableBlend(const ImDrawList*, const ImDrawCmd* cmd);
};

NS_END