#include "ImGui_Manager.h"

#include "GameInstance.h"
#include "GameObject_Factory.h"
#include "Level_Edit.h"
#include "Transform.h"
#include "GameObject.h"
#include "imgui_internal.h"
#include "Model.h"
#include "ContainerObject.h"
#include "PartObject.h"
#include "UIObject.h"
#include "NavMesh_Editor.h"
#include "UIContainerObject.h"
#include "UIPartObject.h"
#include "Animator.h"
#include "GameContent_AnimEvents.h"
#include "Effect_Container.h"
#include "Effect_Part.h"
#include "Map_EditHelper.h"
#include "MapStage.h"
#include "MapObject.h"
#include "EnvObject.h"
#include "Map_PreviewSession.h"

IMPLEMENT_SINGLETON(CImGui_Manager)

static const char* TexTypeName(_uint t)
{
    static const char* names[MTEX_TYPE_MAX] = {
        "None","Diffuse","Specular","Ambient","Emissive","Height","Normals","Shininess",
        "Opacity","Displacement","Lightmap","Reflection","BaseColor","NormalCamera",
        "EmissionColor","Metalness(MRA)","Roughness","AO","Unknown(Mask)","Sheen","Clearcoat",
        "Transmission","MayaBase","MayaSpecular","MayaSpecColor","MayaSpecRough","Anisotropy"
    };
    return (t < MTEX_TYPE_MAX) ? names[t] : "?";
}

static int ToMapRenderGroupIndex(RENDERID eRenderID)
{
    return (eRenderID == RENDERID::BLEND) ? 1 : 0;
}

static RENDERID FromMapRenderGroupIndex(int iIndex)
{
    return (iIndex == 1) ? RENDERID::BLEND : RENDERID::NONBLEND;
}

static int FindExactMeshNameIndex(CModel* pModel, const string& strMeshName)
{
    if (nullptr == pModel || strMeshName.empty())
        return -1;

    const size_t iNumMeshes = pModel->Get_NumMeshes();
    for (size_t i = 0; i < iNumMeshes; ++i)
    {
        if (pModel->Get_MeshName(static_cast<_uint>(i)) == strMeshName)
            return static_cast<int>(i);
    }

    return -1;
}

CImGui_Manager::CImGui_Manager()
    : m_pGameInstance_Proxy(CGameInstance::GetProxy())
{
}

CImGui_Manager::~CImGui_Manager()
{
}

HRESULT CImGui_Manager::ImGui_Initialize(ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext, CLevel_Edit* pLevelEdit, ID3D11ShaderResourceView** ppSRV)
{
    if (pLevelEdit == nullptr)
    {
        MSG_BOX("ImGui_Initialize Failed : Null Level");
        return E_FAIL;
    }

    if (ppDevice == nullptr ||
        ppContext == nullptr)
    {
        MSG_BOX("ImGui_Initialize Failed : Null Device");
		return E_FAIL;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    /*io.ConfigFlags &= ~ImGuiConfigFlags_DpiEnableScaleViewports;
    io.ConfigFlags &= ~ImGuiConfigFlags_DpiEnableScaleFonts;*/
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    io.IniFilename = nullptr;

    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 15.0f, nullptr, io.Fonts->GetGlyphRangesKorean());

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;  // 배경 불투명하게
    }

    ImGui_ImplWin32_Init(g_hWndEditor);
    ImGui_ImplDX11_Init(*ppDevice, *ppContext);

    m_pLevel_Edit = pLevelEdit;
    Safe_AddRef(m_pLevel_Edit);

    m_pSRV = *ppSRV;
    Safe_AddRef(m_pSRV);

    m_pContext = *ppContext;
    Safe_AddRef(m_pContext);

    D3D11_BLEND_DESC bd{};
    bd.RenderTarget[0].BlendEnable = FALSE;                                  // 블렌딩 OFF = RT의 RGB 그대로
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    if (FAILED((*ppDevice)->CreateBlendState(&bd, &m_pOpaqueBlend)))
        return E_FAIL;

    m_ViewportDraw = { m_pContext, m_pOpaqueBlend };

    m_eGizmoOp = ImGuizmo::TRANSLATE;

    return S_OK;
}


void CImGui_Manager::ImGui_Render()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    //ImGuizmo::BeginFrame();

    ImGuiViewport* pViewport = ImGui::GetMainViewport();
    ImVec2 vPos = pViewport->Pos;
    ImVec2 vSize = pViewport->Size;

    // 비율 정의
    float fLeftWidth = vSize.x * 0.15f;      // 왼쪽 15%
    float fRightWidth = vSize.x * 0.2f;      // 오른쪽 20%
    float fCenterWidth = vSize.x - fLeftWidth - fRightWidth;
    float fTopHeight = vSize.y * 0.1f;      // 상단 10%
    float fCenterHeight = vSize.y - fTopHeight;

    // Toolbar (상단 전체)
    ImGui::SetNextWindowPos(vPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(vSize.x, fTopHeight), ImGuiCond_Always);
    Draw_Toolbar();

    // Hierarchy (왼쪽)
    ImGui::SetNextWindowPos(ImVec2(vPos.x, vPos.y + fTopHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(fLeftWidth, (vSize.y - fTopHeight) * 0.5f), ImGuiCond_Always);
    Draw_Hierarchy();

    // Palette (왼쪽 아래)
    ImGui::SetNextWindowPos(ImVec2(vPos.x, vPos.y + fTopHeight + (vSize.y - fTopHeight) * 0.5f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(fLeftWidth, (vSize.y - fTopHeight) * 0.5f), ImGuiCond_Always);
    Draw_Palette();

    // Viewport (중앙 상단 75%)
    ImGui::SetNextWindowPos(ImVec2(vPos.x + fLeftWidth, vPos.y + fTopHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(fCenterWidth, fCenterHeight * 0.75f), ImGuiCond_Always);
    Draw_Viewport();

    // Shader Globals (중앙 하단 25%)
    ImGui::SetNextWindowPos(ImVec2(vPos.x + fLeftWidth, vPos.y + fTopHeight + fCenterHeight * 0.75f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(fCenterWidth, fCenterHeight * 0.25f), ImGuiCond_Always);
    Draw_ShaderGlobals();

    // Inspector (오른쪽)
    ImGui::SetNextWindowPos(ImVec2(vPos.x + fLeftWidth + fCenterWidth, vPos.y + fTopHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(fRightWidth, vSize.y - fTopHeight), ImGuiCond_Always);
    Draw_Inspector();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void CImGui_Manager::Draw_Toolbar()
{
    ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    constexpr _uint iBufferSize = 64;

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    static char s_ObjSaveBuf[iBufferSize] = {};
    if (ImGui::Button("Object Save")) {
        memset(s_ObjSaveBuf, 0, sizeof(s_ObjSaveBuf));
        ImGui::OpenPopup("Save LiveObjects Name");
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Save LiveObjects Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Name:");
        ImGui::InputText("##objname", s_ObjSaveBuf, iBufferSize);

        if (ImGui::Button("OK")) {
            wstring strLevelName(s_ObjSaveBuf, s_ObjSaveBuf + strlen(s_ObjSaveBuf));
            wstring strFilePath(g_strLiveobjectPath + strLevelName + L".json");
            m_pLevel_Edit->Save_LiveObjects(strFilePath, strLevelName);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    ImGui::SameLine();

    static char s_ObjLoadBuf[iBufferSize] = {};
    if (ImGui::Button("Object Load")) {
        memset(s_ObjLoadBuf, 0, sizeof(s_ObjLoadBuf));
        ImGui::OpenPopup("Load LiveObjects Name");
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Load LiveObjects Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Name:");
        ImGui::InputText("##objloadname", s_ObjLoadBuf, iBufferSize);

        if (ImGui::Button("OK")) {
            wstring strLevelName(s_ObjLoadBuf, s_ObjLoadBuf + strlen(s_ObjLoadBuf));
            wstring strFilePath(g_strLiveobjectPath + strLevelName + L".json");
            m_pLevel_Edit->Load_LiveObjects(strFilePath);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    ImGui::SameLine();

    static char    s_LayerNameBuf[iBufferSize] = {};
    if (ImGui::Button("Add Layer")) {
        memset(s_LayerNameBuf, 0, sizeof(s_LayerNameBuf));
        ImGui::OpenPopup("Add Layer Name");
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Add Layer Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Name:");
        ImGui::InputText("##name", s_LayerNameBuf, iBufferSize);

        if (ImGui::Button("OK")) {
            wstring strLayerName(s_LayerNameBuf, s_LayerNameBuf + strlen(s_LayerNameBuf));
            m_pLevel_Edit->Add_Layer(strLayerName);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    ImGui::SameLine();

    const _uint iMapPreviewPresetCount = CMap_EditHelper::Get_MapPresetCount();
    static _int s_iMapPreviewPreset = 0;
    if (0 < iMapPreviewPresetCount)
    {
        if (s_iMapPreviewPreset < 0 || static_cast<_uint>(s_iMapPreviewPreset) >=
            iMapPreviewPresetCount)
            s_iMapPreviewPreset = 0;

        ImGui::SetNextItemWidth(100.f);
        if (ImGui::BeginCombo("##MapPreviewPreset",
            CMap_EditHelper::Get_MapPresetLabel(static_cast<_uint>(s_iMapPreviewPreset))))
        {
            for (_uint i = 0; i < iMapPreviewPresetCount; ++i)
            {
                const _bool bSelected = (static_cast<_uint>(s_iMapPreviewPreset) == i);
                if (ImGui::Selectable(CMap_EditHelper::Get_MapPresetLabel(i), bSelected))
                    s_iMapPreviewPreset = static_cast<_int>(i);
                if (bSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        if (ImGui::Button("Load All"))
            m_pLevel_Edit->Load_MapPreview(static_cast<_uint>(s_iMapPreviewPreset));

        ImGui::SameLine();
        if (ImGui::Button("Load Stage"))
            m_pLevel_Edit->Load_MapPreviewStage(static_cast<_uint>(s_iMapPreviewPreset));

        ImGui::SameLine();
        if (ImGui::Button("Load Env"))
            m_pLevel_Edit->Load_MapPreviewEnv(static_cast<_uint>(s_iMapPreviewPreset));

        ImGui::SameLine();
        if (ImGui::Button("Clear All"))
            m_pLevel_Edit->Clear_MapPreview();

        ImGui::SameLine();
        if (ImGui::Button("Clear Stage"))
            m_pLevel_Edit->Clear_MapPreviewStage();

        ImGui::SameLine();
        if (ImGui::Button("Clear Env"))
            m_pLevel_Edit->Clear_MapPreviewEnv();

        ImGui::SameLine();
        if (ImGui::Button("Map Override Save"))
        {
            if (FAILED(m_pLevel_Edit->Save_MapOverride()))
                MSG_BOX("MAP OVERRIDE SAVE FAILED");
        }

        if (ImGui::IsItemHovered())
        {
            _wstring strOverridePath;
            if (SUCCEEDED(CMap_EditHelper::Get_MapPresetOverrideAssetPath(
                static_cast<_uint>(s_iMapPreviewPreset),
                L"",
                &strOverridePath)))
            {
                const string strTooltip = WstrToStr(strOverridePath);
                ImGui::SetTooltip("%s", strTooltip.c_str());
            }
        }

        string strMapPreviewStatus = WstrToStr(m_pLevel_Edit->Get_MapPreviewStatus());
        const string strFullMapPreviewStatus = strMapPreviewStatus;
        if (strMapPreviewStatus.size() > 48)
            strMapPreviewStatus = strMapPreviewStatus.substr(0, 45) + "...";

        ImGui::TextDisabled("%s", strMapPreviewStatus.c_str());
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", strFullMapPreviewStatus.c_str());

        ImGui::SameLine();
    }

    auto OpButton = [this](const char* label, ImGuizmo::OPERATION op) {
        bool bActive = (m_eGizmoOp == op);
        if (bActive) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
        if (ImGui::Button(label)) m_eGizmoOp = op;
        if (bActive) ImGui::PopStyleColor();
        };

    OpButton("Move", ImGuizmo::TRANSLATE);
    ImGui::SameLine();
    OpButton("Rotate", ImGuizmo::ROTATE);
    ImGui::SameLine();
    OpButton("Scale", ImGuizmo::SCALE);

    ImGui::Separator();
    ImGui::SameLine();

    // --- Key Input Toggle ---
    if (m_bKeyInputEnabled)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.f));
        if (ImGui::Button("KeyInput [ON]"))
        {
            m_bKeyInputEnabled = false;
            m_pGameInstance_Proxy->Disable_InputDeveice();
        }
        ImGui::PopStyleColor();
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.f));
        if (ImGui::Button("KeyInput [OFF]"))
        {
            m_bKeyInputEnabled = true;
            m_pGameInstance_Proxy->Enable_InputDeveice();
        }
        ImGui::PopStyleColor();
    } 

    ImGui::SameLine();

    // --- Play / Stop Toggle ---
    if (m_pGameInstance_Proxy->Is_EditMode())
    {
        // 편집 중 → Play
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.65f, 0.15f, 1.f));
        if (ImGui::Button("Play"))
        {
            m_pGameInstance_Proxy->Set_EditMode(false);    // CCT/게임플레이 ON
            m_pGameInstance_Proxy->Enable_InputDeveice();  // 키입력 ON (WASD)
            m_bKeyInputEnabled = true;
        }
        ImGui::PopStyleColor();
    }
    else
    {
        // 플레이 중 → Stop
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.2f, 0.2f, 1.f));
        if (ImGui::Button("Stop"))
        {
            m_pGameInstance_Proxy->Set_EditMode(true);     // 편집 모드 복귀
            m_pGameInstance_Proxy->Disable_InputDeveice(); // 키입력 OFF
            m_bKeyInputEnabled = false;
        }
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    // --- Physics Debug Toggle ---
    {
        bool bOn = m_pGameInstance_Proxy->Is_PhysXDebug();
        if (bOn) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
        if (ImGui::Button("Physics Debug"))
            m_pGameInstance_Proxy->Toggle_PhysXDebug();
        if (bOn) ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    // --- Effect Save ---
    static char s_EffectSaveBuf[iBufferSize] = {};
    if (ImGui::Button("Effect Save"))
    {
        CGameObject* pSelected = m_pLevel_Edit->Get_Selected();
        CEffect_Container* pSelectedEffect = dynamic_cast<CEffect_Container*>(pSelected);

        if (nullptr == pSelectedEffect)
        {
            MSG_BOX("Select Effect Object First");
        }
        else
        {
            memset(s_EffectSaveBuf, 0, sizeof(s_EffectSaveBuf));

            string strDefaultName = WstrToStr(pSelected->Get_ObjectTag());
            strncpy_s(s_EffectSaveBuf, strDefaultName.c_str(), _TRUNCATE);

            ImGui::OpenPopup("Effect Save");
        }
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Effect Save", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("File name (.JSON):");
        ImGui::InputText("##effectsave", s_EffectSaveBuf, iBufferSize);

        if (ImGui::Button("OK"))
        {
            CGameObject* pSelected = m_pLevel_Edit->Get_Selected();
            CEffect_Container* pSelectedEffect = dynamic_cast<CEffect_Container*>(pSelected);

            if (nullptr == pSelectedEffect)
            {
                MSG_BOX("Select Effect Object First");
            }
            else if (strlen(s_EffectSaveBuf) == 0)
            {
                MSG_BOX("Input Effect File Name");
            }
            else
            {
                wstring strFileName(s_EffectSaveBuf, s_EffectSaveBuf + strlen(s_EffectSaveBuf));
                if (FAILED(m_pLevel_Edit->Save_Selected_Effect(g_strEditPath + strFileName +
                    L".JSON")))
                    MSG_BOX("Effect Save Failed");
                else
                    ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    ImGui::SameLine();

    // --- Effect Load ---
    static char s_EffectLoadBuf[iBufferSize] = {};
    if (ImGui::Button("Effect Load"))
    {
        memset(s_EffectLoadBuf, 0, sizeof(s_EffectLoadBuf));
        ImGui::OpenPopup("Effect Load");
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Effect Load", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("File name (.JSON):");
        ImGui::InputText("##effectload", s_EffectLoadBuf, iBufferSize);

        if (ImGui::Button("OK"))
        {
            if (strlen(s_EffectLoadBuf) == 0)
            {
                MSG_BOX("Input Effect File Name");
            }
            else
            {
                wstring strFileName(s_EffectLoadBuf, s_EffectLoadBuf + strlen(s_EffectLoadBuf));
                if (FAILED(m_pLevel_Edit->Load_Selected_Effect(g_strEditPath + strFileName +
                    L".JSON")))
                    MSG_BOX("Effect Load Failed");
                else
                    ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();

    if (ImGui::Button("Back to Edit"))
        m_pLevel_Edit->Back_To_Edit();

    ImGui::SameLine();

    if (ImGui::Button("Cameras"))
        ImGui::OpenPopup("CamerasPopup");

    if (ImGui::BeginPopup("CamerasPopup"))
    {
        const auto* pLayer = m_pLevel_Edit->Get_CameraLayer();

        if (pLayer && !pLayer->empty())
        {
            for (const auto& handle : *pLayer)
            {
                string strName(handle.strName.begin(), handle.strName.end());

                if (ImGui::Selectable(strName.c_str()))
                {
                    m_pLevel_Edit->Preview_Camera(handle.pObject);
                    ImGui::CloseCurrentPopup();
                }
            }
        }
        else
        {
            ImGui::TextDisabled("(No cameras)");
        }

        ImGui::EndPopup();
    }
   
    ImGui::End();

    return;
}

void CImGui_Manager::Draw_Hierarchy()
{
    ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    for (auto& [LayerTag, Objects] : m_pLevel_Edit->Get_Layers())
    {
        string strLayerName = WstrToStr(LayerTag);
        //_bool bOpen = ImGui::TreeNode(strLayerName.c_str());

        if (ImGui::TreeNode(strLayerName.c_str()))
        {
            // 레이어에 드롭 타겟 설정
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OBJECT_DND"))
                {
                    CGameObject* pDropped = *(CGameObject**)payload->Data;
                    m_pLevel_Edit->Change_ObjectLayer(pDropped, LayerTag);
                }
                ImGui::EndDragDropTarget();
            }


            for (auto& Handle : Objects)
            {
                string strName = WstrToStr(Handle.strName);
                _bool bSelected = (Handle.pObject == m_pLevel_Edit->Get_Selected());

                float fAvailWidth = ImGui::GetContentRegionAvail().x;

                if (ImGui::Selectable(strName.c_str(), bSelected, 0, ImVec2(fAvailWidth - 25.f, 0)))
                    m_pLevel_Edit->Set_Selected(Handle.pObject);

                // 드래그 소스 설정
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {
                    ImGui::SetDragDropPayload("OBJECT_DND", &Handle.pObject, sizeof(CGameObject*));
                    ImGui::Text("%s", strName.c_str());
                    ImGui::EndDragDropSource();
                }

                ImGui::SameLine();

                ImGui::PushID(Handle.pObject);
                if (ImGui::SmallButton("X"))
                {
                    m_RotEditEuler.erase(Handle.pObject);
                    m_pLevel_Edit->Delete_Object(Handle.pObject);
                    ImGui::PopID();
                    break;
                }
				ImGui::PopID();
            }
            ImGui::TreePop();
        }
    }

    ImGui::Separator();
    Draw_MapPreviewDeletedOverrides();
    ImGui::Separator();
    Draw_MapPreviewAddedOverrides();

    ImGui::End();
    return;
}

void CImGui_Manager::Draw_Gizmo()
{
    CGameObject* pSelected = m_pLevel_Edit->Get_Selected();
    if (!pSelected) return;

    ImGuizmo::BeginFrame();
    ImGuizmo::Enable(true);

    // 클라이언트 전체 영역 기준 
    ImGuiViewport* pMainViewport = ImGui::GetMainViewport();
    ImGuizmo::SetRect(pMainViewport->Pos.x, pMainViewport->Pos.y, pMainViewport->Size.x, pMainViewport->Size.y);

    // 메인 DrawList에 그림
    //ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
    ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList(pMainViewport));

    PROJ_TYPE eProjType = pSelected->Get_ProjType();

    ImGuizmo::SetOrthographic(eProjType == PROJ_TYPE::ORTHO);

    const _float4x4* pView = m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, eProjType);
    const _float4x4* pProj = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, eProjType);

    if (!pView || !pProj) return;

    CTransform* pTransform = pSelected->Get_Transform();
    _float4x4 matWorld = *pTransform->Get_WorldMatrixPtr();

    _float snap[3] = { 0,0,0 };
    if (eProjType == PROJ_TYPE::ORTHO)
        snap[0] = snap[1] = snap[2] = 1.f;

    ImGuizmo::Manipulate(
        (float*)pView,
        (float*)pProj,
        m_eGizmoOp,
        ImGuizmo::LOCAL,
        (float*)&matWorld,
        nullptr, snap
    );

    if (ImGuizmo::IsUsing()) {
        if (m_eGizmoOp == ImGuizmo::TRANSLATE && eProjType == PROJ_TYPE::ORTHO) {
            _float4 OriginPos = {};
            XMStoreFloat4(&OriginPos, pTransform->Get_State(STATE::POSITION));

            _float3 MovePos = {};
            memcpy(&MovePos, matWorld.m[3], sizeof(_float3));

            _float3 FixedPos = MovePos;
            FixedPos.z = OriginPos.z;

            memcpy(matWorld.m[3], &FixedPos, sizeof(_float3));
        }
        pTransform->Set_WorldMatrix(XMLoadFloat4x4(&matWorld));
    }
}

void CImGui_Manager::Draw_Inspector()
{
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    CGameObject* pSelected = m_pLevel_Edit->Get_Selected();
    if (!pSelected) { ImGui::End(); return; }

    Draw_Transform(pSelected);

    ImGui::Separator();

    Draw_MeshLayerPanel(pSelected);

    ImGui::Separator();
    ImGui::Separator();

    Draw_Properties(pSelected);

    auto pModel = pSelected->Get_Component<CModel>(TEXT("Com_Model"));
    auto pAnimator = pSelected->Get_Component<CAnimator>(TEXT("Com_Animator"));
    if (pModel && pAnimator)
        Draw_AnimatorEditor(pModel, pAnimator);

    ImGui::Separator();

    for (auto& [tag, pComponent] : pSelected->Get_Components())
    {
        if (!pComponent) continue;
        if (pComponent->Get_Properties().empty()) continue;

        string strTag = WstrToStr(tag);
        if (ImGui::CollapsingHeader(strTag.c_str()))
        {
            ImGui::PushID(pComponent);
            Draw_Properties(pComponent);
            ImGui::PopID();
        }
    }

    ImGui::Separator();

    auto pContainer = dynamic_cast<CContainerObject*>(pSelected);
    if (pContainer)
    {
        auto& PartObjects = pContainer->Get_PartObjects();
        vector<pair<wstring, CPartObject*>> vecSorted(PartObjects.begin(), PartObjects.end());
        sort(vecSorted.begin(), vecSorted.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
            });

        for (auto& [tag, pPart] : vecSorted)
        {

            string strTag = WstrToStr(tag);
            if (ImGui::CollapsingHeader(strTag.c_str()))
            {
                ImGui::PushID(pPart);
                Draw_Transform(pPart, strTag);
                ImGui::Separator();
                Draw_Properties(pPart);

                // 파트 오브젝트의 애니메이터 노출
                auto pPartModel = pPart->Get_Component<CModel>(TEXT("Com_Model"));
                auto pPartAnimator = pPart->Get_Component<CAnimator>(TEXT("Com_Animator"));
                if (pPartModel && pPartAnimator)
                    Draw_AnimatorEditor(pPartModel, pPartAnimator);

                ImGui::PopID();
            }
        }
    }

    auto pUIContainer = dynamic_cast<CUIContainerObject*>(pSelected);
    if (pUIContainer)
    {
        auto& UIPartObjects = pUIContainer->Get_UIPartObjects();
        vector<pair<wstring, CUIPartObject*>> vecSorted(UIPartObjects.begin(), UIPartObjects.end());
        sort(vecSorted.begin(), vecSorted.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
            });

        for (auto& [tag, pPart] : vecSorted)
        {
            string strTag = WstrToStr(tag);
            if (ImGui::CollapsingHeader(strTag.c_str()))
            {
                ImGui::PushID(pPart);
                Draw_Transform(pPart, strTag);
                ImGui::Separator();
                Draw_Properties(pPart);
                ImGui::PopID();
            }
        }
    }

    auto pEffectContainer = dynamic_cast<CEffect_Container*>(pSelected);
    if (pEffectContainer)
    {      
        auto& pEffectPart = pEffectContainer->Get_EffectPartObject();
        vector<pair<wstring, CEffect_Part*>> vecSorted(pEffectPart.begin(), pEffectPart.end());
        sort(vecSorted.begin(), vecSorted.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
            });
        
        for (auto& [tag, pPart] : vecSorted)
        {
            ImGui::Separator();
            ImGui::Separator();

            string strTag = WstrToStr(tag);
            if (ImGui::CollapsingHeader(strTag.c_str()))
            {
                ImGui::PushID(pPart);
                Draw_Transform(pPart, strTag);
                ImGui::Separator();
                Draw_Properties(pPart);
                ImGui::PopID();
            }
        }
    }

    ImGui::Separator();

    // ===== Map Stage → Sections (컨테이너와 동일한 시각적 표현) =====
    auto pMapStage = dynamic_cast<CMapStage*>(pSelected);
    if (pMapStage)
    {
        const auto& Sections = pMapStage->Get_Sections();   // vector<CMapSection*> (이미 정렬된 순서)

        ImGui::Separator();
        ImGui::Text("Sections (%d)", (int)Sections.size());

        for (CMapSection* pSection : Sections)
        {
            if (!pSection) continue;

            // 섹션 이름을 헤더 라벨 + 고유 ID로 사용
            string strName = WstrToStr(pSection->Get_SectionName());
            string strHeader = strName + "##Section_" + to_string((uintptr_t)pSection);

            if (ImGui::CollapsingHeader(strHeader.c_str()))
            {
                ImGui::PushID(pSection);

                // 섹션의 로컬 트랜스폼 (스테이지 부모행렬과 Late_Update에서 합성됨)
                Draw_Transform(pSection, strName);
                ImGui::Separator();
                Draw_Properties(pSection);
                ImGui::Separator();
                Draw_MeshLayerPanel(pSection);
                ImGui::Separator();
                Draw_MapSectionRenderOptions(pSection);
                ImGui::PopID();
            }
            ImGui::Separator();
            ImGui::Separator();
        }
    }

    ImGui::End();
    return;
}

void CImGui_Manager::Draw_Properties(IReflectable* pHolder)
{
    // 카테고리별로 프로퍼티를 모음 (최초 등장 순서 유지)
    vector<string>                                  vecOrder;
    unordered_map<string, vector<const FPROPERTY*>> mapByCategory;

    for (auto& prop : pHolder->Get_Properties())
    {
        const string strCat = WstrToStr(prop.strCategory);
        if (mapByCategory.find(strCat) == mapByCategory.end())
            vecOrder.push_back(strCat);
        mapByCategory[strCat].push_back(&prop);
    }

    for (auto& strCategory : vecOrder)
    {
        // 카테고리 이름이 비어 있으면 헤더 없이 그대로 출력
        if (strCategory.empty())
        {
            for (auto* pProp : mapByCategory[strCategory])
                Draw_Property(pHolder, *pProp);
            continue;
        }

        // 폴더처럼 접히는 카테고리 헤더 (기본 펼침)
        if (ImGui::CollapsingHeader(strCategory.c_str()))
        {
            ImGui::Indent();
            for (auto* pProp : mapByCategory[strCategory])
                Draw_Property(pHolder, *pProp);
            ImGui::Unindent();
        }
    }
}

void CImGui_Manager::Draw_Property(IReflectable* pHolder, const FPROPERTY& prop)
{
    void* pData = pHolder->Get_PropertyPtr(prop.uOffset);
    if (!pData) return;

    string strPropName = WstrToStr(prop.strName);
    string strID = "##" + to_string(prop.uOffset);   // 고유 ID

    // 이름이 10글자를 넘으면 기존처럼 위에, 아니면 위젯 오른쪽에 표시
    bool bLabelAbove = prop.strName.length() > 10;
    if (bLabelAbove)
        ImGui::Text(strPropName.c_str());

    // 위에 띄웠으면 숨김 라벨(ID만), 아니면 이름+ID
    string strLabel = bLabelAbove ? strID : (strPropName + strID);

    switch (prop.eType)
    {
        case PROP_TYPE::INT:
            ImGui::InputInt(strLabel.c_str(), (int*)pData);
            break;
        case PROP_TYPE::UINT:
        {
            int v = (int)(*(_uint*)pData);
            if (ImGui::InputInt(strLabel.c_str(), &v))
                *(_uint*)pData = (_uint)(v < 0 ? 0 : v);
            break;
        }
        case PROP_TYPE::FLOAT:
            ImGui::DragFloat(strLabel.c_str(), (float*)pData, 0.1f);
            break;
        case PROP_TYPE::BOOL:
            ImGui::Checkbox(strLabel.c_str(), (bool*)pData);
            break;
        case PROP_TYPE::FLOAT2:
            ImGui::DragFloat2(strLabel.c_str(), (float*)pData, 0.1f);
            break;
        case PROP_TYPE::FLOAT3:
            ImGui::DragFloat3(strLabel.c_str(), (float*)pData, 0.1f);
            break;
        case PROP_TYPE::FLOAT4:
            ImGui::DragFloat4(strLabel.c_str(), (float*)pData, 0.1f);
            break;
        case PROP_TYPE::ANIM_INDEX:
        {
            int iVal = (int)*((_uint*)pData);
            CGameObject* pObject = dynamic_cast<CGameObject*>(pHolder);
            CModel* pModel = pObject ? pObject->Get_Component<CModel>(L"Com_Model") : nullptr;
            if (!pModel) break;
            _uint iNumAnims = pModel->Get_MaxAnimationIndex() + 1;
            string strItems;
            for (_uint i = 0; i < iNumAnims; ++i)
                strItems += to_string(i) + ": " + pModel->Get_AnimationName(i) + '\0';
            strItems += '\0';

            if (ImGui::Combo(strLabel.c_str(), &iVal, strItems.c_str()))
                *(_uint*)pData = (_uint)iVal;
            break;
        }
        case PROP_TYPE::WSTRING:
        {
            wstring* pWstr = (wstring*)pData;
            string str = WstrToStr(*pWstr);
            char buf[256] = {};
            strcpy_s(buf, str.c_str());
            if (ImGui::InputText(strLabel.c_str(), buf, sizeof(buf)))
                *pWstr = StrToWstr(buf);
            break;
        }
    }
    ImGui::Separator();
}

void CImGui_Manager::Draw_Palette()
{
    ImGui::Begin("Palette", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    map<wstring, vector<wstring>> TagsByCategory;
    CGameObject_Factory::GetInstance()->Copy_TagsByCategory(&TagsByCategory);

    static wstring s_PendingTag;
    constexpr _uint iBufferSize = 64;
    static char    s_NameBuf[iBufferSize] = {};
    static _bool   s_isOpenPopup = { false };

    for (auto& [category, tags] : TagsByCategory)
    {
        string strCatLabel = WstrToStr(category);
        if (ImGui::TreeNode(strCatLabel.c_str()))
        {
            for (auto& strTag : tags)
            {
                string strLabel = WstrToStr(strTag);
                if (ImGui::Button(strLabel.c_str())) {
                    if (category == L"UI_OBJECT" || category == L"UI_CONTAINER_OBJECT")
                    {
                        s_PendingTag = strTag;
                        memset(s_NameBuf, 0, sizeof(s_NameBuf));
                        s_isOpenPopup = true;
                    }
                    else {
                        m_pLevel_Edit->Begin_PlaceMode(strTag, CLevel_Edit::OBJECT_LAYER_TAG);
                    }
                }
            }
            ImGui::TreePop();
        }
    }

    if (m_pLevel_Edit->Is_PlaceMode())
    {
        string strProto = WstrToStr(m_pLevel_Edit->Get_PendingProto());
        ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Placing : %s", strProto.c_str());
        ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "[Esc : Cancel]");
    }

    if (s_isOpenPopup)
    {
        ImGui::OpenPopup("Input Name");
        s_isOpenPopup = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Input Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Name:");
        ImGui::InputText("##name", s_NameBuf, iBufferSize);

        if(ImGui::Button("OK")) {
            wstring strName(s_NameBuf, s_NameBuf + strlen(s_NameBuf));
            m_pLevel_Edit->Spawn_Object(s_PendingTag, TEXT("Default_Layer"), strName);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    ImGui::End();
}

void CImGui_Manager::Draw_Viewport()
{
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImVec2 vAvail = ImGui::GetContentRegionAvail();

    _float fTargetAspect = 1600.f / 900.f;
    _float fAvailAspect = vAvail.x / vAvail.y;

    ImVec2 vSize = {};
    if (fAvailAspect > fTargetAspect)
    {
        vSize.y = vAvail.y;
        vSize.x = vAvail.y * fTargetAspect;
    }
    else
    {
        vSize.x = vAvail.x;
        vSize.y = vAvail.x / fTargetAspect;
    }

    ImVec2 vOffset((vAvail.x - vSize.x) * 0.5f, (vAvail.y - vSize.y) * 0.5f);
    ImVec2 vCursor = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(vCursor.x + vOffset.x, vCursor.y + vOffset.y));

    // 패널 크기와 위치
    ImVec2 vPos = ImGui::GetCursorScreenPos();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddCallback(Viewport_DisableBlend, &m_ViewportDraw);  
    ImGui::Image((ImTextureID)m_pSRV, vSize);
    dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

    const _float4x4* pView = m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC);
    const _float4x4* pProj = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);

    auto WorldToScreen = [&](const _float3& vWorld) -> ImVec2
        {
            XMVECTOR v = XMVectorSet(vWorld.x, vWorld.y, vWorld.z, 1.f);
            XMMATRIX mVP = XMLoadFloat4x4(pView) * XMLoadFloat4x4(pProj);
            XMVECTOR clip = XMVector4Transform(v, mVP);

            float w = XMVectorGetW(clip);
            if (w < 1e-5f)  // w <= 0 : 카메라 뒤쪽 or near plane 이전
                return ImVec2(-9999.f, -9999.f);

            float ndcX = XMVectorGetX(clip) / w;
            float ndcY = XMVectorGetY(clip) / w;

            return ImVec2(
                vPos.x + (ndcX + 1.f) * 0.5f * vSize.x,
                vPos.y + (1.f - ndcY) * 0.5f * vSize.y
            );
        };

    

    bool bRightHeld = ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right);
    bool bImageHovered = ImGui::IsItemHovered();

    if (bImageHovered && !bRightHeld && !ImGuizmo::IsOver())
    {
        ImVec2 mouse = ImGui::GetMousePos();

        // 뷰포트 로컬 좌표 → NDC
        float ndcX = ((mouse.x - vPos.x) / vSize.x) * 2.f - 1.f;
        float ndcY = 1.f - ((mouse.y - vPos.y) / vSize.y) * 2.f;

        XMVECTOR origin, dir;
        m_pGameInstance_Proxy->Compute_PickingRay(ndcX, ndcY, &origin, &dir);

        // 좌클릭 → 땅바닥 배치
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            m_pLevel_Edit->Pick_And_Place(origin, dir);

        if (m_pGameInstance_Proxy->Mouse_Down(DIMB::WHEEL))
        {
            _float2 vNDC = { ndcX, ndcY };
            UI_RBTN_PROBE eProbe = { vNDC, false };
            m_pGameInstance_Proxy->Publish(TEXT("UI_RButton_Probe"), &eProbe);

            if (eProbe.bConsumed) return;

            WORLD_RBTN_DOWN eEvent = { vNDC };
            m_pGameInstance_Proxy->Publish(TEXT("World_RButton_Click"), &eEvent);
        }

        // ESC → 배치 모드 취소
        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            m_pLevel_Edit->End_PlaceMode();
    }

    m_pLevel_Edit->Set_CameraActive(bRightHeld);



    // 기즈모 그리기
    CGameObject* pSelected = m_pLevel_Edit->Get_Selected();
    if (pSelected)
    {
        ImGuizmo::BeginFrame();
        ImGuizmo::Enable(true);

        // 이 패널 영역에 기즈모 그리기
        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        ImGuizmo::SetRect(vPos.x, vPos.y, vSize.x, vSize.y);

        PROJ_TYPE eProjType = pSelected->Get_ProjType();

        ImGuizmo::SetOrthographic(eProjType == PROJ_TYPE::ORTHO);

        const _float4x4* pView = m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, eProjType);
        const _float4x4* pProj = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, eProjType);

        if (!pView || !pProj) return;

        CTransform* pTransform = pSelected->Get_Transform();
        _float4x4 matWorld = *pTransform->Get_WorldMatrixPtr();

        _float snap[3] = { 0,0,0 };
        if (m_eGizmoOp == ImGuizmo::TRANSLATE && eProjType == PROJ_TYPE::ORTHO)
            snap[0] = snap[1] = snap[2] = 1.f;

        ImGuizmo::Manipulate(
            (float*)pView,
            (float*)pProj,
            m_eGizmoOp,
            ImGuizmo::LOCAL,
            (float*)&matWorld,
            nullptr, snap
        );

        if (ImGuizmo::IsUsing()) {
            if (m_eGizmoOp == ImGuizmo::TRANSLATE && eProjType == PROJ_TYPE::ORTHO) {
                _float4 OriginPos = {};
                XMStoreFloat4(&OriginPos, pTransform->Get_State(STATE::POSITION));

                _float3 MovePos = {};
                memcpy(&MovePos, matWorld.m[3], sizeof(_float3));

                _float3 FixedPos = MovePos;
                FixedPos.z = OriginPos.z;

                memcpy(matWorld.m[3], &FixedPos, sizeof(_float3));
            }
            pTransform->Set_WorldMatrix(XMLoadFloat4x4(&matWorld));
        }
    }

    ImGui::End();
}

void CImGui_Manager::Draw_Transform(CGameObject* pObject, const string& strSuffix)
{
    CTransform* pTransform = pObject->Get_Transform();

    _float4 vPos = {};
    XMStoreFloat4(&vPos, pTransform->Get_State(STATE::POSITION));
    if (ImGui::DragFloat3(("Position##" + strSuffix).c_str(), (float*)&vPos, 0.1f))
        pTransform->Set_State(STATE::POSITION, XMLoadFloat4(&vPos));

    _float3& vEuler = m_RotEditEuler[pObject];
    _float3 vBefore = vEuler;
    if (ImGui::DragFloat3(("Rotate##" + strSuffix).c_str(), (float*)&vEuler, 0.5f))
    {
        _float3 vDelta = {
            vEuler.x - vBefore.x,
            vEuler.y - vBefore.y,
            vEuler.z - vBefore.z
        };

        auto ApplyDelta = [pTransform](_fvector vAxis, _float fDegree) {
            if (fDegree == 0.f) return;
            _vector vQuat = XMQuaternionRotationNormal(vAxis, XMConvertToRadians(fDegree));
            pTransform->Rotate(vQuat);
            };

        _vector vR = XMVector3Normalize(pTransform->Get_State(STATE::RIGHT));
        _vector vU = XMVector3Normalize(pTransform->Get_State(STATE::UP));
        _vector vL = XMVector3Normalize(pTransform->Get_State(STATE::LOOK));

        ApplyDelta(vR, vDelta.x);
        ApplyDelta(vU, vDelta.y);
        ApplyDelta(vL, vDelta.z);
    }

    _float3 vScale = pTransform->Get_Scaled();

    if (dynamic_cast<CUIObject*>(pObject))
    {
        static CGameObject* pPrevObj = nullptr;
        static _float fRatioScale = 1.f;
        static _float fBaseScaleX{}, fBaseScaleY{};

        if (pPrevObj != pObject)
        {
            fRatioScale = 1.f;
            fBaseScaleX = vScale.x;
            fBaseScaleY = vScale.y;
            pPrevObj = pObject;
        }

        _float2 vUIScale = { vScale.x, vScale.y };
        if (ImGui::DragFloat2(("Scale##" + strSuffix).c_str(), (float*)&vUIScale, 0.1f))
            pTransform->Set_Scale(vUIScale.x, vUIScale.y, 1.f);

        if (ImGui::DragFloat(("Uniform Scale##" + strSuffix).c_str(), &fRatioScale, 0.01f, 0.01f, 100.f))
            pTransform->Set_Scale(fBaseScaleX * fRatioScale, fBaseScaleY * fRatioScale, 1.f);
    }
    else
    {
        _float fUniformScale = vScale.x;
        if (ImGui::DragFloat(("Scale##" + strSuffix).c_str(), &fUniformScale, 0.1f))
            pTransform->Set_Scale(fUniformScale, fUniformScale, fUniformScale);
    }
}

void CImGui_Manager::Draw_AnimatorEditor(CModel* pModel, CAnimator* pAnimator)
{
    if (!pModel || !pAnimator) return;
    if (!ImGui::CollapsingHeader("Animator", ImGuiTreeNodeFlags_DefaultOpen)) return;

    // 애니메이터(파트 포함)마다 독립 상태 유지
    struct ANIM_UI_STATE 
    { 
        int iSel = 0,iPrevSel = -1; 
        bool bPlay = true, bLoop = true;
        float fPreview = 0.f;
        float fPlaySpeed = 1.f;
        float fBlendDuration = 0.2f;
    };
    static unordered_map<CAnimator*, ANIM_UI_STATE> s_States;
    ANIM_UI_STATE& st = s_States[pAnimator];

    _uint iMax = pModel->Get_MaxAnimationIndex();
    if (st.iSel > (int)iMax) st.iSel = 0;
    string strName = pModel->Get_AnimationName((_uint)st.iSel);

    if (ImGui::BeginCombo("Animation", strName.c_str()))
    {
        for (_uint i = 0; i <= iMax; ++i)
            if (ImGui::Selectable(pModel->Get_AnimationName(i).c_str(), (_uint)st.iSel == i))
                st.iSel = (int)i;
        ImGui::EndCombo();
    }

    ImGui::Checkbox("Play", &st.bPlay); ImGui::SameLine();
    ImGui::Checkbox("Loop", &st.bLoop);
    ImGui::SetNextItemWidth(120.f);
    ImGui::DragFloat("Speed", &st.fPlaySpeed, 0.01f, 0.05f, 8.f, "%.2fx");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.f);
    ImGui::DragFloat("Blend", &st.fBlendDuration, 0.01f, 0.f, 2.f, "%.2fs");

    if (st.iSel != st.iPrevSel)
    {
        pAnimator->Play(strName, st.bLoop, true, st.fBlendDuration, st.fPlaySpeed);
        st.iPrevSel = st.iSel; st.fPreview = 0.f;
    }

    if (st.bPlay)
    {
        pAnimator->Resume();
        pAnimator->Play(strName, st.bLoop, false, st.fBlendDuration, st.fPlaySpeed);
        pAnimator->Update(ImGui::GetIO().DeltaTime);
        st.fPreview = pAnimator->Get_Progress();
        ImGui::SliderFloat("Preview", &st.fPreview, 0.f, 1.f);
    }
    else
    {
        pAnimator->Pause();
        if (ImGui::SliderFloat("Preview", &st.fPreview, 0.f, 1.f))
            pAnimator->Seek(st.fPreview);
    }

    ANIM_EVENT_TRACK& track = pAnimator->Get_Track(strName);
    if (ImGui::Button("+ Add Event"))
        track.Events.push_back({ (int)EANIM_EVENT::Fx, st.fPreview });

    for (int i = 0; i < (int)track.Events.size(); ++i)
    {
        ImGui::PushID(i);
        ANIM_EVENT& e = track.Events[i];

        const char* szCur = "None";
        for (auto& [v, n] : g_AnimEventNames) if ((int)v == e.iEventType) szCur = n;
        if (ImGui::BeginCombo("Type", szCur))
        {
            for (auto& [v, n] : g_AnimEventNames)
                if (ImGui::Selectable(n, (int)v == e.iEventType)) e.iEventType = (int)v;
            ImGui::EndCombo();
        }

        ImGui::SliderFloat("Start", &e.fTriggerProgress, 0.f, 1.f);
        ImGui::Checkbox("Range", &e.bIsRange);
        if (e.bIsRange) ImGui::SliderFloat("End", &e.fEndProgress, 0.f, 1.f);

        char pbuf[128]; strcpy_s(pbuf, e.strParam.c_str());
        if (ImGui::InputText("Param", pbuf, sizeof(pbuf))) e.strParam = pbuf;
        ImGui::InputInt("IntParam", &e.iIntParam);
        ImGui::DragFloat3("Offset", &e.vOffset.x, 0.1f);

        if (ImGui::Button("X Delete")) track.Events.erase(track.Events.begin() + i--);
        ImGui::Separator();
        ImGui::PopID();
    }

    if (ImGui::Button("Save"))
    {
        pAnimator->Sort_Track(strName);
        pAnimator->Save_ToFile(TEXT("../../Resources/Models/Test/Marb1e/Marb1e_animevents.json"));
    }
}

void CImGui_Manager::Draw_ShaderGlobals()
{
    ImGui::Begin("Shader Globals", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    if (nullptr != m_pGameInstance_Proxy)
    {
        auto& Globals = m_pGameInstance_Proxy->Get_ShaderGlobals();

        _int iID = 0;
        for (auto& g : Globals)
        {
            ImGui::PushID(iID++);   // 라벨 중복 대비 고유 ID

            switch (g.eType)
            {
                case GVAL::FLOAT:
                    ImGui::SliderFloat(g.strLabel.c_str(), &g.vValue.x, g.vRange.x, g.vRange.y);
                    break;
                case GVAL::FLOAT2:
                    ImGui::SliderFloat2(g.strLabel.c_str(), &g.vValue.x, g.vRange.x, g.vRange.y);
                    break;
                case GVAL::FLOAT3:
                    ImGui::SliderFloat3(g.strLabel.c_str(), &g.vValue.x, g.vRange.x, g.vRange.y);
                    break;
                case GVAL::FLOAT4:
                    ImGui::SliderFloat4(g.strLabel.c_str(), &g.vValue.x, g.vRange.x, g.vRange.y);
                    break;
                case GVAL::BOOL:
                {
                    bool b = (g.vValue.x > 0.5f);
                    if (ImGui::Checkbox(g.strLabel.c_str(), &b))
                        g.vValue.x = b ? 1.f : 0.f;
                    break;
                }
            }

            ImGui::PopID();
        }
    }

    ImGui::End();
}

void CImGui_Manager::Draw_MeshLayerPanel(CGameObject* pObj)
{
    if (nullptr == pObj) return;
    CModel* pModel = pObj->Get_Component<CModel>(L"Com_Model");
    if (nullptr == pModel) return;
    if (!ImGui::CollapsingHeader("Mesh Render Settings (per Model)")) return;

    size_t n = pModel->Get_NumMeshes();
    for (size_t i = 0; i < n; ++i)
    {
        MESH_LAYER_IDX L = pModel->Get_MeshLayer((_uint)i);
        ImGui::PushID((int)i);
        ImGui::Text("%zu: %s", i, pModel->Get_MeshName((_uint)i).c_str());

        bool changed = false;
        bool bAnyField = false;

        int iPass = L.iPass;
        ImGui::SetNextItemWidth(120.f);
        if (ImGui::InputInt("Pass", &iPass))
        {
            if (iPass < -1)
                iPass = -1;

            L.iPass = iPass;
            changed = true;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(-1 = default)");

        for (_uint t = 0; t < MTEX_TYPE_MAX; ++t)
        {
            int count = (int)pModel->Get_MeshTextureCount((_uint)i, (MTEX_TYPE)t);
            if (count <= 1) continue;            // 고를 게 없는 타입은 숨김

            bAnyField = true;
            int iv = (int)L.idx[t];
            ImGui::SetNextItemWidth(120.f);
            if (ImGui::InputInt(TexTypeName(t), &iv))
            {
                if (iv < 0)      iv = 0;
                if (iv >= count) iv = count - 1; // 사용 가능 개수로 클램프 → E_FAIL 방지
                L.idx[t] = (_uint)iv;
                changed = true;
            }
            ImGui::SameLine(); ImGui::Text("/ %d", count);
        }
        if (!bAnyField)
            ImGui::TextDisabled("  (no texture slot override)");

        if (changed) pModel->Set_MeshLayer((_uint)i, L);
        ImGui::Separator();
        ImGui::PopID();
    }

    if (ImGui::Button("Bake (Save sidecar)"))
        pModel->Save_MeshLayers();
}

void CImGui_Manager::Draw_MapSectionRenderOptions(CMapSection* pSection)
{
    if (nullptr == pSection)
        return;

    if (!ImGui::CollapsingHeader("Render Group (per Section)", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    int iRenderGroup = ToMapRenderGroupIndex(pSection->Get_RenderID());
    const char* RenderGroups[] = { "NONBLEND", "BLEND" };

    if (ImGui::Combo("Render Group", &iRenderGroup, RenderGroups, IM_ARRAYSIZE(RenderGroups)))
        pSection->Set_RenderID(FromMapRenderGroupIndex(iRenderGroup));
}

void CImGui_Manager::Draw_MapPreviewDeletedOverrides()
{
    ImGui::TextUnformatted("Deleted Env Overrides");

    const CMap_PreviewSession* pSession = m_pLevel_Edit->Get_MapPreviewSession();
    if (nullptr == pSession || 0 == pSession->Get_DeletedEnvCount())
    {
        ImGui::TextDisabled("No deleted env overrides.");
        return;
    }

    _wstring strRestoreKey;
    _bool bRestoreAll = false;

    if (ImGui::Button("Restore All"))
        bRestoreAll = true;

    ImGui::BeginChild("DeletedEnvOverrides", ImVec2(0.f, 140.f), true);

    for (const auto& strKey : pSession->Get_DeletedEnvOrder())
    {
        CMap_PreviewSession::MAP_PREVIEW_ENV_ITEM Item{};
        if (!pSession->Try_GetDeletedEnvItem(strKey, &Item))
            continue;

        const string strKeyUtf8 = WstrToStr(strKey);
        const string strDisplay = WstrToStr(Item.strDisplayName.empty() ? strKey : Item.strDisplayName);

        ImGui::PushID(strKeyUtf8.c_str());
        ImGui::TextWrapped("%s", strDisplay.c_str());
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", strKeyUtf8.c_str());

        if (ImGui::SmallButton("Restore"))
            strRestoreKey = strKey;

        ImGui::Separator();
        ImGui::PopID();
    }

    ImGui::EndChild();

    if (bRestoreAll)
        m_pLevel_Edit->Restore_AllDeletedMapPreviewEnv();
    else if (!strRestoreKey.empty())
        m_pLevel_Edit->Restore_DeletedMapPreviewEnv(strRestoreKey);
}

void CImGui_Manager::Draw_MapPreviewAddedOverrides()
{
    ImGui::TextUnformatted("Added Map Overrides");

    const CMap_PreviewSession* pSession = m_pLevel_Edit->Get_MapPreviewSession();
    if (nullptr == pSession || 0 == pSession->Get_AddedMapObjectCount())
    {
        ImGui::TextDisabled("No added map overrides.");
        return;
    }

    CGameObject* pSelectObject = nullptr;
    CGameObject* pDeleteObject = nullptr;

    ImGui::BeginChild("AddedMapOverrides", ImVec2(0.f, 140.f), true);

    for (CGameObject* pObject : pSession->Get_AddedMapObjectOrder())
    {
        if (nullptr == pObject)
            continue;

        CMap_PreviewSession::MAP_PREVIEW_ADDED_ITEM Item{};
        if (!pSession->Try_GetAddedMapObjectItem(pObject, &Item))
            continue;

        const _wstring strDisplayName =
            Item.strDisplayName.empty()
            ? (!Item.strObjectTag.empty() ? Item.strObjectTag : Item.strPrototypeTag)
            : Item.strDisplayName;

        const string strDisplayUtf8 = WstrToStr(strDisplayName);
        const string strProtoUtf8 = WstrToStr(Item.strPrototypeTag);
        const string strLayerUtf8 = WstrToStr(Item.strLayerTag);
        const string strObjectTagUtf8 = WstrToStr(Item.strObjectTag);

        ImGui::PushID(pObject);

        ImGui::TextWrapped("%s", strDisplayUtf8.c_str());
        if (!strProtoUtf8.empty() || !strLayerUtf8.empty())
        {
            ImGui::TextDisabled(
                "Proto=%s / Layer=%s / Tag=%s",
                strProtoUtf8.c_str(),
                strLayerUtf8.c_str(),
                strObjectTagUtf8.c_str());
        }

        if (ImGui::Button("Select"))
            pSelectObject = pObject;

        ImGui::SameLine();
        if (ImGui::SmallButton("Remove"))
            pDeleteObject = pObject;

        ImGui::Separator();
        ImGui::PopID();
    }

    ImGui::EndChild();
    {
        m_RotEditEuler.erase(pDeleteObject);
        m_pLevel_Edit->Delete_Object(pDeleteObject);
    }
}

void CImGui_Manager::Free()
{
    Safe_Release(m_pSRV);
    Safe_Release(m_pOpaqueBlend);   
    Safe_Release(m_pContext);
    Safe_Release(m_pLevel_Edit);
    Safe_Release(m_pGameInstance_Proxy);

    ImGui::DestroyPlatformWindows();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void CImGui_Manager::Viewport_DisableBlend(const ImDrawList*, const ImDrawCmd* cmd)
{
    auto* p = static_cast<CImGui_Manager::VIEWPORT_DRAW*>(cmd->UserCallbackData);
    const float bf[4] = { 0.f, 0.f, 0.f, 0.f };
    p->pContext->OMSetBlendState(p->pBlend, bf, 0xffffffff);
}
