#include "Panel_Inspector.h"
#include "imgui.h"
#include "Preview_Actor.h"
#include "Model.h"
#include <functional>
#include "Panel_Manager.h"
#include "Transform.h"
#include "Property.h"

CPanel_Inspector::CPanel_Inspector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "Inspector");
}

void CPanel_Inspector::Render()
{
    ImGui::Begin(m_szName);

    ANIM_CONTEXT& ctx = m_pPanel_Manager->Get_Context();
    if (!ctx.pActor || !ctx.pModel)
    {
        ImGui::TextDisabled("(no model loaded)");
        ImGui::End();
        return;
    }

    Render_Model();
    ImGui::Separator();
    Render_RenderDebug();
    ImGui::Separator();
    Render_Transform(ctx.pActor);
    Render_Properties(ctx.pActor);
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
    ImGui::Text("Type: %s", ctx.pActor->Get_Type() == MODEL::ANIM ? "ANIM" :
        "NONANIM");
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
    CPreview_Actor* pActor = ctx.pActor;
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

    CPreview_Actor* pActor = ctx.pActor;
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

CPanel_Inspector* CPanel_Inspector::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CPanel_Inspector(pDevice, pContext);
}

void CPanel_Inspector::Free() 
{
    __super::Free();
}