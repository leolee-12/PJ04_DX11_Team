#include "Panel_Manager.h"
#include "Panel.h"
#include "GameObject.h"
#include "Model.h"
#include "Animator.h"
#include "UIContainerObject.h"
#include "ContainerObject.h"
#include "PartObject.h"
#include "GameInstance.h"

#include "Panel_Hierarchy.h"
#include "Panel_Viewport.h"
#include "Panel_Inspector.h"
#include "Panel_Console.h"
#include "Panel_Animation.h"
#include "Panel_UICanvas.h"
#include "Panel_Preview.h"

#include "Panel_Browser.h"
#include "Panel_Palette.h"

#include "Level_Tool.h"


CPanel_Manager::CPanel_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice(pDevice), m_pContext(pContext)
    , m_pGameInstance_Proxy(CGameInstance::GetProxy())
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

HRESULT CPanel_Manager::Initialize()
{
    if (FAILED(Add_Panel(L"Hierarchy", CPanel_Hierarchy::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(Add_Panel(L"Viewport", CPanel_Viewport::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(Add_Panel(L"UICanvas", CPanel_UICanvas::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(Add_Panel(L"Inspector", CPanel_Inspector::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(Add_Panel(L"Console", CPanel_Console::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(Add_Panel(L"Animation", CPanel_Animation::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(Add_Panel(L"Browser", CPanel_Browser::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(Add_Panel(L"Preview", CPanel_Preview::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(Add_Panel(L"Palette", CPanel_Palette::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    return S_OK;
}

HRESULT CPanel_Manager::Add_Panel(const _wstring& strPanelTag, CPanel* pPanel)
{
    if (nullptr == pPanel)
        return E_FAIL;

    if (m_Panels.find(strPanelTag) != m_Panels.end())
        return E_FAIL;

    if (FAILED(pPanel->Initialize(this)))
        return E_FAIL;

    m_Panels.emplace(strPanelTag, pPanel);
    return S_OK;
}

CPanel* CPanel_Manager::Get_Panel(const _wstring& strPanelTag)
{
    auto it = m_Panels.find(strPanelTag);
    return (it == m_Panels.end()) ? nullptr : it->second;
}

void CPanel_Manager::Update(_float fTimeDelta)
{
    for (auto& [tag, pPanel] : m_Panels)
    {
        if (!pPanel->Is_Open())
            continue;

        if (!Is_PanelAllowedInCurrentMode(tag))
            continue;

        pPanel->Update(fTimeDelta);
    }
}

void CPanel_Manager::Render()
{
    Render_DockSpace();
    Render_ModeBar();

    for (auto& [tag, pPanel] : m_Panels)
    {
        if (!pPanel->Is_Open())
            continue;

        if (!Is_PanelAllowedInCurrentMode(tag))
            continue;

        pPanel->Render();
    }
}

void CPanel_Manager::Set_Selected(Engine::CGameObject* pObject)
{
    if (m_pSelected == pObject)
        return;

    Safe_Release(m_pSelected);
    m_pSelected = pObject;
    Safe_AddRef(m_pSelected);
}

void CPanel_Manager::Clear_Selected()
{
    m_Context.pOwner = nullptr;
    m_Context.pModel = nullptr;
    m_Context.pAnimator = nullptr;
    m_Context.strName.clear();
    m_Context.strModelPath.clear();
    m_Context.strAnimEventPath.clear();
    m_Context.iClip = 0;
    m_Context.fProgress = 0.f;
    m_Context.iRootBone = -1;
    m_Context.iSelBone = -1;

    Safe_Release(m_pSelected);
    m_pSelected = nullptr;
}

void CPanel_Manager::Clear_Spawned()
{
    if (nullptr == m_pLevel)
        return;

    if (Is_Spawned_Object(m_pSelected))
        Clear_Selected();

    m_pLevel->Clear_Spawned();
}

_bool CPanel_Manager::Is_Spawned_Object(CGameObject* pObject) const
{
    if (nullptr == pObject || nullptr == m_pLevel)
        return false;

    for (auto* pSpawned : m_pLevel->Get_SpawnedObjects())
    {
        if (nullptr == pSpawned)
            continue;

        if (pSpawned == pObject)          
            return true;

        if (auto* pContainer = dynamic_cast<CContainerObject*>(pSpawned))
        {
            for (const auto& Pair : pContainer->Get_PartObjects())
                if (Pair.second == pObject)   
                    return true;
        }
    }
    return false;
}

void CPanel_Manager::Validate_AnimSelection()
{
    if (nullptr == m_pSelected)
        return;

    if (m_pLevel && m_pSelected == m_pLevel->Get_Preview())
        return;

    if (Is_Spawned_Object(m_pSelected))
        return;

    Clear_Selected();
}

void CPanel_Manager::Bind_Preview(CGameObject* pOwner)
{
    m_Context.pOwner = pOwner;
    m_Context.pModel = pOwner ? pOwner->Get_Component<CModel>(L"Com_Model") : nullptr;
    m_Context.pAnimator = pOwner ? pOwner->Get_Component<CAnimator>(L"Com_Animator") : nullptr;
    m_Context.iClip = 0;
    m_Context.fProgress = 0.f;
    m_Context.iRootBone = -1;
    if (!pOwner)
    {
        m_Context.strName.clear();
        m_Context.strModelPath.clear();
    }
    Set_Selected(pOwner);
}

void CPanel_Manager::Set_UISelected(CUIContainerObject* pContainer, CUIPartObject* pPart, const _wstring& strPartTag)
{
    m_UIContext.Selection.pContainer = pContainer;
    m_UIContext.Selection.pPart = pPart;
    m_UIContext.Selection.strPartTag = strPartTag;
}

void CPanel_Manager::Load_Preview(const std::wstring& strYshPath)
{
    if (!m_pLevel)
        return;
    CGameObject* p = m_pLevel->Load_Preview(strYshPath);
    Bind_Preview(p);
    if (p)
    {
        m_Context.strName = filesystem::path(strYshPath).stem().wstring();
        m_Context.strModelPath = strYshPath;
    }
}

void CPanel_Manager::Clear_Preview()
{
    if (m_pLevel)
        m_pLevel->Clear_Preview();
    Bind_Preview(nullptr);
}

void CPanel_Manager::Clear_UISelected()
{
    m_UIContext.Selection.Clear();
}

_bool CPanel_Manager::Validate_UISelection()
{
    UI_SELECTION& Selection = m_UIContext.Selection;

    if (nullptr == Selection.pContainer)
        return false;

    // 컨테이너가 추적 목록에 없으면(삭제됨) 선택 해제
    if (m_pLevel)
    {
        const auto& Entries = m_pLevel->Get_UIContainerEntries();        
        auto it = std::find_if(Entries.begin(), Entries.end(),
            [&Selection](const UI_CONTAINER_ENTRY& Entry)
            {
                return Entry.pContainer == Selection.pContainer;
            });

        if (it == Entries.end())
        {
            Clear_UISelected();
            return false;
        }
    }

    // 컨테이너만 선택된 상태는 유효
    if (nullptr == Selection.pPart)
        return true;

    // 파트 선택: 컨테이너에 실제 존재하는지 검증, 없으면 파트만 해제
    const auto& Parts = Selection.pContainer->Get_UIPartObjects();
    auto iter = Parts.find(Selection.strPartTag);
    if (iter == Parts.end() || iter->second != Selection.pPart)
    {
        Selection.pPart = nullptr;
        Selection.strPartTag.clear();
    }

    return true;
}

void CPanel_Manager::Load_UI_ByPath(const _wstring& strFullPath)
{
    if (!m_pLevel) 
        return;
    _float2 ds = m_UIContext.vDesignSize;
    if (m_pLevel->Load_UIContainerByPath(strFullPath, ds))
        m_UIContext.vDesignSize = ds;
}

void CPanel_Manager::Bind_ForAnim(CGameObject* pObj)
{
    if (!pObj)
    {
        Bind_Preview(nullptr);
        return;
    }

    // 일반 Object는 자기 자신을 Animation Source로 사용.
    // Container는 Hierarchy에서 Part를 직접 선택하게 하고,
    // Container 자체 클릭은 선택만 되게 둔다.
    Bind_ForAnimSource(pObj, pObj, pObj->Get_ObjectTag());
}

void CPanel_Manager::Bind_ForAnimSource(CGameObject* pOwner, CGameObject* pAnimSource, const _wstring& strDisplayName)
{
    if (!pOwner)
    {
        Bind_Preview(nullptr);
        return;
    }

    CGameObject* pSelectedObject = pAnimSource ? pAnimSource : pOwner;

    m_Context.pOwner = pSelectedObject;
    m_Context.pModel = pAnimSource ? pAnimSource->Get_Component<CModel>(L"Com_Model") : nullptr;
    m_Context.pAnimator = pAnimSource ? pAnimSource->Get_Component<CAnimator>(L"Com_Animator") :
        nullptr;

    m_Context.iClip = 0;
    m_Context.fProgress = 0.f;
    m_Context.iRootBone = -1;
    m_Context.iSelBone = -1;

    m_Context.strName = strDisplayName;
    m_Context.strModelPath.clear();
    m_Context.strAnimEventPath.clear();

    if (m_Context.pAnimator)
        m_Context.strAnimEventPath = m_Context.pAnimator->Get_DataFilePath();

    Set_Selected(pSelectedObject);
}

void CPanel_Manager::Render_DockSpace()
{
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpaceHost", nullptr, flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
    {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, vp->WorkSize);

        ImGuiID center = dockspace_id, left, bottom;
        ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.30f, &bottom, &center);
        ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.24f, &left, &center);

        ImGui::DockBuilderDockWindow("Inspector", left);
        ImGui::DockBuilderDockWindow("Hierarchy", left);
        ImGui::DockBuilderDockWindow("Console", left);
        ImGui::DockBuilderDockWindow("Palette", left);
        ImGui::DockBuilderDockWindow("Viewport", center);
        ImGui::DockBuilderDockWindow("UICanvas", center);
        ImGui::DockBuilderDockWindow("Animation", bottom);
        ImGui::DockBuilderDockWindow("Browser", bottom);
        ImGui::DockBuilderDockWindow("Preview", bottom);
        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_None);
    ImGui::End();
}

void CPanel_Manager::Render_ModeBar()
{
    if (!ImGui::BeginMainMenuBar())
        return;

    if (ImGui::BeginMenu("Panels"))
    {
        for (auto& [tag, pPanel] : m_Panels)
        {
            _bool bOpen = pPanel->Is_Open();
            if (ImGui::Checkbox(pPanel->Get_Name(), &bOpen))
                pPanel->Set_Open(bOpen);
        }

        ImGui::EndMenu();
    }

    ImGui::SameLine();

    _int iMode = static_cast<_int>(m_eWorkMode);

    if (ImGui::RadioButton("Animation", iMode == ETOI(TOOL_MODE::ANIMATION)))
        m_eWorkMode = TOOL_MODE::ANIMATION;

    ImGui::SameLine();

    if (ImGui::RadioButton("UI", iMode == ETOI(TOOL_MODE::UI)))
        m_eWorkMode = TOOL_MODE::UI;

    ImGui::SameLine();
    if (ImGui::Button("Load Kirby (Test)"))
    {
        if (m_pLevel)
        {
            Bind_Preview(m_pLevel->Load_Kirby());
            //m_pLevel->Load_Kirby();
            m_Context.strName = L"Kirby";
            //m_Context.strModelPath = L"../../Resources/CHJ/AnimModel/Kirby/Kirby.ysh";
            m_Context.strModelPath = L"../../Resources/CHJ/AnimModel/Kirby/Kirby_AllAbilities.ysh";
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Load DeformCar_Main (Test)"))
    {
        if (m_pLevel)
        {
            Bind_Preview(m_pLevel->Load_DeformCar());
            m_Context.strName = L"DeformCa_Main";
            m_Context.strModelPath = L"../../Resources/YSE/DeformCar/Main.ysh";
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Load DeformCar_Demo (Test)"))
    {
        if (m_pLevel)
        {
            Bind_Preview(m_pLevel->Load_DeformCar_Demo());
            m_Context.strName = L"DeformCar_Demo";
            m_Context.strModelPath = L"../../Resources/YSE/DeformCar/Demo.ysh";
        }
    }

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

    _bool bEditMode = m_pGameInstance_Proxy->Is_EditMode();
    if (ImGui::Checkbox("EditMode", &bEditMode))
    {
        m_pGameInstance_Proxy->Set_EditMode(bEditMode);
    }


    ImGui::EndMainMenuBar();
}

_bool CPanel_Manager::Is_PanelAllowedInCurrentMode(const _wstring& strPanelTag) const
{
    if (m_eWorkMode == TOOL_MODE::ANIMATION)
    {
        if (strPanelTag == L"UICanvas")
            return false;

        return true;
    }

    if (m_eWorkMode == TOOL_MODE::UI)
    {
        if (strPanelTag == L"Animation")
            return false;

        if (strPanelTag == L"Viewport")
            return false;

        if (strPanelTag == L"Palette")   
            return false;

        return true;
    }

    return true;
}

CPanel_Manager* CPanel_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CPanel_Manager* pInstance = new CPanel_Manager(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CPanel_Manager");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CPanel_Manager::Free()
{
    __super::Free();

    Clear_UISelected();


    Safe_Release(m_pSelected);

    for (auto& [tag, pPanel] : m_Panels)
        Safe_Release(pPanel);
    m_Panels.clear();

    Safe_Release(m_pGameInstance_Proxy);

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
}
