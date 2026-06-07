#include "Panel_Inspector.h"
#include "imgui.h"
#include "Preview_Actor.h"
#include "Model.h"
#include <functional>
#include "Panel_Manager.h"
#include "Transform.h"
#include "Property.h"
#include "Preview_Kirby.h"
#include "Kirby_States.h"
#include "UIPartObject.h"
#include "UIContainerObject.h"
#include "Level_Tool.h"
#include "UI_SpriteAnim.h"

CPanel_Inspector::CPanel_Inspector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "Inspector");
}

void CPanel_Inspector::Render()
{
    ImGui::Begin(m_szName);

    if (m_pPanel_Manager->Get_WorkMode() == TOOL_MODE::UI)
    {
        Render_UIInspector();
        ImGui::End();
        return;
    }

    ANIM_CONTEXT& ctx = m_pPanel_Manager->Get_Context();
    if (!ctx.pOwner || !ctx.pModel)
    {
        ImGui::TextDisabled("(no model loaded)");
        ImGui::End();
        return;
    }

    Render_Model();
    ImGui::Separator();
    Render_KirbyFace(ctx.pOwner);
    Render_RenderDebug();
    ImGui::Separator();
    Render_Transform(ctx.pOwner);
    Render_Properties(ctx.pOwner);
    ImGui::Separator();
    Render_Meshs();
    ImGui::Separator();
    Render_Bones();

    ImGui::End();
}

void CPanel_Inspector::Render_Model()
{
    ANIM_CONTEXT& ctx = m_pPanel_Manager->Get_Context();

    ImGui::Text("[Model]");
    std::string name(ctx.strName.begin(), ctx.strName.end());
    ImGui::Text("Name: %s", name.empty() ? "-" : name.c_str());
    if (auto* pv = dynamic_cast<CPreview_Actor*>(ctx.pOwner))
        ImGui::Text("Type: %s", pv->Get_Type() == MODEL::ANIM ? "ANIM" : "NONANIM");
    ImGui::Text("Meshes: %zu", ctx.pModel->Get_NumMeshes());
    // (NonAnim/Anim는 .ysh에 박힌 값. 변경/저장은 추후 .AnimClips/bake 단계와 연동)
}

void CPanel_Inspector::Render_Transform(CGameObject* pObject)
{
    if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    CTransform* pTransform = pObject->Get_Transform();

    _float4 vPos = {};
    XMStoreFloat4(&vPos, pTransform->Get_State(STATE::POSITION));
    if (ImGui::DragFloat3("Position", (float*)&vPos, 0.1f))
        pTransform->Set_State(STATE::POSITION, XMLoadFloat4(&vPos));

    _float3& vEuler = m_RotEditEuler[pObject];
    _float3  vBefore = vEuler;
    if (ImGui::DragFloat3("Rotate", (float*)&vEuler, 0.5f))
    {
        _float3 d = { vEuler.x - vBefore.x, vEuler.y - vBefore.y, vEuler.z - vBefore.z };
        auto Apply = [pTransform](_fvector vAxis, _float deg) {
            if (deg == 0.f) return;
            pTransform->Rotate(XMQuaternionRotationNormal(vAxis, XMConvertToRadians(deg)));
            };
        Apply(XMVector3Normalize(pTransform->Get_State(STATE::RIGHT)), d.x);
        Apply(XMVector3Normalize(pTransform->Get_State(STATE::UP)), d.y);
        Apply(XMVector3Normalize(pTransform->Get_State(STATE::LOOK)), d.z);
    }

    _float3 vScale = pTransform->Get_Scaled();
    _float  fUniform = vScale.x;
    if (ImGui::DragFloat("Scale", &fUniform, 0.1f))
        pTransform->Set_Scale(fUniform, fUniform, fUniform);
}

void CPanel_Inspector::Render_RenderDebug()
{
    ANIM_CONTEXT& ctx = m_pPanel_Manager->Get_Context();
    CPreview_Actor* pActor = dynamic_cast<CPreview_Actor*>(ctx.pOwner);
    if (!pActor)
        return;

    if (!ImGui::CollapsingHeader("Render Debug", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    using MODE = PREVIEW_SHADER_MODE;

    int shaderMode = (int)pActor->Get_PreviewShaderMode();
    const char* shaderItems[] =
    {
        "Auto",
        "AnimMesh",
        "NonAnimMesh"
    };

    if (ImGui::Combo("Shader", &shaderMode, shaderItems, IM_ARRAYSIZE(shaderItems)))
        pActor->Set_PreviewShaderMode((MODE)shaderMode);

    int passMode = pActor->Get_PreviewPassOverride() + 1;
    const char* passItems[] =
    {
        "Auto",
        "Pass 0",
        "Pass 1",
        "Pass 2"
    };

    if (ImGui::Combo("Pass", &passMode, passItems, IM_ARRAYSIZE(passItems)))
        pActor->Set_PreviewPassOverride(passMode - 1);

    ImGui::Text("Resolved Shader: %s", pActor->Get_ResolvedShaderName());
    ImGui::Text("Resolved Pass: %u", pActor->Get_ResolvedPassIndex());

    ImGui::TextDisabled("AnimMesh pass 0: Eye/UV1");
    ImGui::TextDisabled("AnimMesh pass 1: NonEye/UV0");
    ImGui::TextDisabled("NonAnim pass 0: Eye/UV1");
    ImGui::TextDisabled("NonAnim pass 1: Diffuse/UV0");
}

void CPanel_Inspector::Render_Properties(IReflectable* pHolder)
{
    auto& props = pHolder->Get_Properties();
    if (props.empty()) return;
    if (!ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen)) return;

    for (auto& prop : props)
    {
        void* pData = pHolder->Get_PropertyPtr(prop.uOffset);
        if (!pData) continue;
        std::string name(prop.strName.begin(), prop.strName.end());

        switch (prop.eType)
        {
        case PROP_TYPE::INT:    ImGui::InputInt(name.c_str(), (int*)pData); break;
        case PROP_TYPE::UINT: { int v = (int)*(_uint*)pData; if (ImGui::InputInt(name.c_str(), &v)) *(_uint*)pData = (_uint)(v < 0 ? 0 : v); } break;
        case PROP_TYPE::FLOAT:  ImGui::DragFloat(name.c_str(), (float*)pData, 0.1f); break;
        case PROP_TYPE::BOOL:   ImGui::Checkbox(name.c_str(), (bool*)pData); break;
        case PROP_TYPE::FLOAT2: ImGui::DragFloat2(name.c_str(), (float*)pData, 0.1f); break;
        case PROP_TYPE::FLOAT3: ImGui::DragFloat3(name.c_str(), (float*)pData, 0.1f); break;
        case PROP_TYPE::FLOAT4: ImGui::DragFloat4(name.c_str(), (float*)pData, 0.1f); break;
        case PROP_TYPE::ANIM_INDEX:
        {
            ANIM_CONTEXT& ctx = m_pPanel_Manager->Get_Context();
            CModel* pModel = ctx.pModel;
            if (!pModel)
                break;

            const _uint iNumAnims = pModel->Get_NumAnimations();
            if (iNumAnims == 0)
            {
                ImGui::TextDisabled("%s: no animation clips", name.c_str());
                break;
            }

            if (ctx.iClip < 0 || ctx.iClip >= (_int)iNumAnims)
                ctx.iClip = 0;

            int iVal = ctx.iClip;

            std::string items;
            for (_uint i = 0; i < iNumAnims; ++i)
                items += std::to_string(i) + ": " + pModel->Get_AnimationName(i) + '\0';
            items += '\0';

            ImGui::PushID(name.c_str());
            if (ImGui::Combo(name.c_str(), &iVal, items.c_str()))
                ctx.iClip = iVal;
            ImGui::PopID();

            break;
        }
        case PROP_TYPE::WSTRING:
        {
            std::wstring* pW = (std::wstring*)pData;
            std::string s(pW->begin(), pW->end());
            char buf[256] = {}; strcpy_s(buf, s.c_str());
            if (ImGui::InputText(name.c_str(), buf, sizeof(buf)))
                *pW = std::wstring(buf, buf + strlen(buf));
            break;
        }
        }
    }
}

void CPanel_Inspector::Render_Bones()
{
    if (!ImGui::CollapsingHeader("Bones", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    ANIM_CONTEXT& ctx = m_pPanel_Manager->Get_Context();
    CModel* pModel = ctx.pModel;

    const _uint iNumBones = pModel->Get_NumBones();
    ImGui::Text("[Bones] %u", iNumBones);

    // Root Bone (자동 + 선택)
    if (ctx.iRootBone < 0) ctx.iRootBone = pModel->Get_RootBoneIndex();
    std::string rootName = (ctx.iRootBone >= 0 && (_uint)ctx.iRootBone < iNumBones)
        ? pModel->Get_BoneName((_uint)ctx.iRootBone) : "(none)";
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::BeginCombo("Root", rootName.c_str()))
    {
        for (_uint i = 0; i < iNumBones; ++i)
            if (ImGui::Selectable(pModel->Get_BoneName(i).c_str(), (_uint)ctx.iRootBone == i))
                ctx.iRootBone = (_int)i;
        ImGui::EndCombo();
    }

    // 현재 클립이 건드리는 본 집합
    std::vector<_uint> clipBones;

    const _uint iNumAnims = pModel->Get_NumAnimations();
    if (iNumAnims > 0)
    {
        if (ctx.iClip < 0 || ctx.iClip >= (_int)iNumAnims)
            ctx.iClip = 0;

        pModel->Get_AnimChannelBoneIndices((_uint)ctx.iClip, clipBones);
    }

    auto isAnimated = [&](_uint i)
        {
            return std::find(clipBones.begin(), clipBones.end(), i) != clipBones.end();
        };

    // 부모 → 자식 인접 리스트 구성 (매 프레임, ~수백 본까지 가벼움)
    std::vector<std::vector<_uint>> children(iNumBones);
    std::vector<_uint> roots;
    for (_uint i = 0; i < iNumBones; ++i)
    {
        _int p = pModel->Get_BoneParentIndex(i);
        if (p >= 0 && (_uint)p < iNumBones) children[p].push_back(i);
        else                                roots.push_back(i);
    }

    const ImVec4 vGreen(0.35f, 1.f, 0.35f, 1.f);

    // 스켈레톤 트리
    ImGui::TextDisabled(
        iNumAnims > 0
        ? "Skeleton (green = animated by current clip)"
        : "Skeleton (no animation clips)");
    ImGui::BeginChild("BoneTree", ImVec2(0, 230.f), true);

    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 12.f);

    function<void(_uint)> drawNode = [&](_uint i)
        {
            const bool leaf = children[i].empty();
            const bool animated = isAnimated(i);

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                | ImGuiTreeNodeFlags_DefaultOpen
                | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (leaf)
                flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            if ((_int)i == ctx.iSelBone)
                flags |= ImGuiTreeNodeFlags_Selected;

            if (animated) ImGui::PushStyleColor(ImGuiCol_Text, vGreen);
            ImGui::PushID((int)i);
            const bool open = ImGui::TreeNodeEx("##bone", flags, "%u: %s", i, pModel->Get_BoneName(i).c_str());
            if (animated) ImGui::PopStyleColor();           // 라벨 그린 뒤 즉시 복원

            if (ImGui::IsItemClicked()) ctx.iSelBone = (_int)i;

            if (open && !leaf)
            {
                for (_uint c : children[i]) drawNode(c);
                ImGui::TreePop();
            }
            ImGui::PopID();
        };

    for (_uint r : roots)
        drawNode(r);

    ImGui::PopStyleVar();

    ImGui::EndChild();

    // 현재 클립 본 목록 (부분집합 → 평면)
    ImGui::TextDisabled("Clip Bones (%zu)", clipBones.size());
    ImGui::BeginChild("ClipBones", ImVec2(0, 90.f), true);
    for (_uint idx : clipBones)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, vGreen);
        ImGui::Text("%u: %s", idx, pModel->Get_BoneName(idx).c_str());
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();
}

void CPanel_Inspector::Render_Meshs()
{
    ANIM_CONTEXT& ctx = m_pPanel_Manager->Get_Context();

    CPreview_Actor* pActor = dynamic_cast<CPreview_Actor*>(ctx.pOwner);
    CModel* pModel = ctx.pModel;

    if (!pActor || !pModel)
        return;

    if (!ImGui::CollapsingHeader("Meshes", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    const _uint iNumMeshes = (_uint)pModel->Get_NumMeshes();

    ImGui::Text("Mesh Count: %u", iNumMeshes);

    if (ImGui::Button("All On"))
        pActor->Set_AllMeshVisible(true);

    ImGui::SameLine();

    if (ImGui::Button("All Off"))
        pActor->Set_AllMeshVisible(false);

    ImGui::Separator();

    ImGui::BeginChild("MeshVisibilityList", ImVec2(0, 180.f), true);

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        ImGui::PushID((int)i);

        bool bVisible = pActor->Is_MeshVisible(i);
        if (ImGui::Checkbox("##visible", &bVisible))
            pActor->Set_MeshVisible(i, bVisible);

        ImGui::SameLine();

        if (ImGui::SmallButton("Solo"))
            pActor->Set_SoloMesh(i);

        ImGui::SameLine();

        const string& strMeshName = pModel->Get_MeshName(i);
        ImGui::Text("%u: %s", i, strMeshName.c_str());

        ImGui::PopID();
    }

    ImGui::EndChild();
}

void CPanel_Inspector::Render_KirbyFace(CGameObject* pObject)
{
    CPreview_Kirby* pKirby = dynamic_cast<CPreview_Kirby*>(pObject);
    if (!pKirby)
        return;

    if (!ImGui::CollapsingHeader("Kirby Face", ImGuiTreeNodeFlags_DefaultOpen) || pObject == nullptr)
        return;

    {
        const char* szCur = "?";
        for (auto& [v, n] : g_KirbyBodyNames) if (v == pKirby->Get_Body()) szCur = n;
        if (ImGui::BeginCombo("Body", szCur))
        {
            for (auto& [v, n] : g_KirbyBodyNames)
                if (ImGui::Selectable(n, v == pKirby->Get_Body())) pKirby->Set_Body(v);
            ImGui::EndCombo();
        }
    }
    // Mouth
    {
        const char* szCur = "?";
        for (auto& [v, n] : g_KirbyMouthNames) if (v == pKirby->Get_Mouth()) szCur = n;
        if (ImGui::BeginCombo("Mouth", szCur))
        {
            for (auto& [v, n] : g_KirbyMouthNames)
                if (ImGui::Selectable(n, v == pKirby->Get_Mouth())) pKirby->Set_Mouth(v);
            ImGui::EndCombo();
        }
    }
    // Eye
    {
        const char* szCur = "?";
        for (auto& [v, n] : g_KirbyEyeNames) if (v == pKirby->Get_Eye()) szCur = n;
        if (ImGui::BeginCombo("Eye", szCur))
        {
            for (auto& [v, n] : g_KirbyEyeNames)
                if (ImGui::Selectable(n, v == pKirby->Get_Eye())) pKirby->Set_Eye(v);
            ImGui::EndCombo();
        }
    }

}

void CPanel_Inspector::Render_UITransform(CUIPartObject* pPart)
{

    if (!ImGui::CollapsingHeader("Transform (UI)", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    CTransform* pT = pPart->Get_Transform();
    UI_CONTEXT& uictx = m_pPanel_Manager->Get_UIContext();

    // Position: x,y = UI 중심좌표 / z = z-order
    _vector vPos = pT->Get_State(STATE::POSITION);
    float pos[3] = { XMVectorGetX(vPos), XMVectorGetY(vPos), XMVectorGetZ(vPos) };
    bool bMoved = false;
    bMoved |= ImGui::DragFloat2("Position (x,y)", pos, 1.f);
    bMoved |= ImGui::DragFloat("Z-Order", &pos[2], 0.01f, 0.1f, 2.0f);   // ortho z 범위
    if (bMoved)
    {
        pT->Set_State(STATE::POSITION, XMVectorSet(pos[0], pos[1], pos[2], 1.f));
        uictx.bDirty = true;
    }

    // Size: scale.x = 가로, scale.y = 세로 (개별)
    _float3 sc = pT->Get_Scaled();
    float size[2] = { sc.x, sc.y };
    if (ImGui::DragFloat2("Size (w,h)", size, 1.f, 1.f, 99999.f))
    {
        pT->Set_Scale(size[0], size[1], 1.f);
        uictx.bDirty = true;
    }
}

void CPanel_Inspector::Render_UIInspector()
{
    UI_CONTEXT& uictx = m_pPanel_Manager->Get_UIContext();
    UI_SELECTION& sel = uictx.Selection;

    if (nullptr == sel.pContainer)
    {
        ImGui::TextDisabled("(no UI selected)");
        return;
    }

    if (nullptr == sel.pPart)
    {
        std::string strCName = ToUtf8(sel.pContainer->Get_ObjectTag());
        ImGui::Text("[UI Container] %s",
            strCName.empty() ? "-" : strCName.c_str());
        ImGui::Separator();

        CLevel_Tool* pLevel = m_pPanel_Manager->Get_Level();
        if (pLevel)
        {
            _wstring wTag = pLevel->Get_AuthoredProtoTag(sel.pContainer);
            char szTag[128] = {};
            strncpy_s(szTag, ToUtf8(wTag).c_str(), sizeof(szTag) - 1);

            if (ImGui::InputText("Runtime ProtoTag",
                szTag, sizeof(szTag)))
            {
                pLevel->Set_AuthoredProtoTag(
                    sel.pContainer, StrToWstr(szTag));
                uictx.bDirty = true;
            }
            ImGui::TextDisabled(
                "RunTime Spawn Class Tag : Save -> json in in !!");
        }
        return;
    }

    CUIPartObject* pPart = sel.pPart;

    std::string strName = ToUtf8(sel.strPartTag);
    ImGui::Text("[UI Part] %s", strName.empty() ? "-" : strName.c_str());
    ImGui::Separator();

    struct { RENDERUIID v; const char* n; } layers[] = {
        { RENDERUIID::BACK, "BACK" }, { RENDERUIID::MIDDLE, "MIDDLE" }, { RENDERUIID::FRONT, "FRONT" }
    };
    const char* szCur = "?";
    for (auto& L : layers) if (L.v == pPart->Get_RenderLayer()) szCur = L.n;
    if (ImGui::BeginCombo("RenderLayer", szCur))
    {
        for (auto& L : layers)
            if (ImGui::Selectable(L.n, L.v == pPart->Get_RenderLayer()))
            {
                pPart->Set_RenderLayer(L.v);
                uictx.bDirty = true;
            }
        ImGui::EndCombo();
    }
    ImGui::Text("Z (Transform z): %.3f", pPart->Get_ZOrder());

    ImGui::Separator();
    Render_UITransform(pPart);
    Render_Properties(pPart);
    Render_SpriteAnimControl(pPart);
}

void CPanel_Inspector::Render_SpriteAnimControl(CUIPartObject* pPart)
{
    auto* pAnim = dynamic_cast<Client::CUI_SpriteAnim*>(pPart);
    if (!pAnim)
        return;

    if (!ImGui::CollapsingHeader("SpriteAnim",
        ImGuiTreeNodeFlags_DefaultOpen))
        return;

    const _int iFrameCount = pAnim->Get_FrameCount();
    const _int iLast = (iFrameCount > 0) ? iFrameCount - 1 : 0;

    const char* szState =
        pAnim->Is_Finished() ? "Finished" :
        (pAnim->Is_Playing() ? "Playing" : "Paused");
    ImGui::Text("State: %s   Frame: %d / %d",
        szState, pAnim->Get_Frame(), iLast);

    // 진행도 바
    ImGui::ProgressBar(pAnim->Get_Progress(), ImVec2(-1.f, 0.f));

    float fProgress = pAnim->Get_Progress();
    if (ImGui::SliderFloat("Seek", &fProgress, 0.f, 1.f, "%.3f"))
    {
        pAnim->Pause();
        pAnim->Seek(fProgress);
    }

    // 프레임 단위 스크럽
    if (iFrameCount > 1)
    {
        int iFrame = pAnim->Get_Frame();
        if (ImGui::SliderInt("Frame", &iFrame, 0, iLast))
        {
            pAnim->Pause();
            pAnim->Set_Frame(iFrame);
        }
    }

    // 트랜스포트
    if (ImGui::Button("Restart")) pAnim->Play();
    ImGui::SameLine();
    if (ImGui::Button("Pause"))   pAnim->Pause();
    ImGui::SameLine();
    if (ImGui::Button("Resume"))  pAnim->Resume();
    ImGui::SameLine();
    if (ImGui::Button("Stop"))    pAnim->Stop();
}

CPanel_Inspector* CPanel_Inspector::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CPanel_Inspector(pDevice, pContext);
}

void CPanel_Inspector::Free()
{
    __super::Free();
}
