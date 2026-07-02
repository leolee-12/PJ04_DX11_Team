#include "Panel_Inspector.h"
#include "imgui.h"
#include "Preview_Actor.h"
#include "Model.h"
#include <functional>
#include "Panel_Manager.h"
#include "Transform.h"
#include "Property.h"
#include "Preview_Kirby.h"
#include "Preview_DeformCar_Main.h"
#include "Preview_DeformCar_Demo.h"
#include "Kirby_States.h"
#include "UIPartObject.h"
#include "UIContainerObject.h"
#include "Level_Tool.h"
#include "UI_SpriteAnim.h"
#include "UI_Fonts.h"
#include "UI_Text.h"
#include "UI_Image.h"
#include "UI_Effect.h"
#include "UI_GaugeFill.h"
#include "UIAnimatorCom.h"
#include "UI_Curtain.h"
#include "UI_CurtainAnimBase.h"

namespace
{
    template<typename TPair, size_t N>
    void Draw_StateCombo(const char* szLabel, int& iValue,
        const TPair(&names)[N])
    {
        const char* szCur = "?";
        for (auto& [v, n] : names) if ((int)v == iValue) szCur = n;
        if (ImGui::BeginCombo(szLabel, szCur))
        {
            for (auto& [v, n] : names)
                if (ImGui::Selectable(n, (int)v == iValue)) iValue = (int)v;
            ImGui::EndCombo();
        }
    }

    const std::pair<int, const char*> g_TextAlignNames[] = {
            { 0, "Left" }, { 1, "Center" }, { 2, "Right" },
    };

    struct UI_EFFECT_PASS_ITEM
    {
        int iPass;
        const char* szName;
        const char* szDesc;
        bool bSourceTexture;
        bool bMaskTexture;
        bool bUV;
        bool bReveal;
    };

    const UI_EFFECT_PASS_ITEM g_UIEffectPassItems[] =
    {
        { 2, "2 MaskedTexture", "Source texture clipped by mask", true, true, true, false },
        { 3, "3 MaskedColor",   "Color clipped by mask", false, true, false, false },
        { 4, "4 MaskedAdd",     "Source texture additively blended through mask", true, true, true, false },
        { 5, "5 BrushReveal",   "Source texture revealed by brush mask", true, true, false, true },
    };

    const UI_EFFECT_PASS_ITEM* Find_EffectPassItem(int iPass)
    {
        for (const auto& item : g_UIEffectPassItems)
        {
            if (item.iPass == iPass)
                return &item;
        }

        return nullptr;
    }

    bool Is_EffectInspectorProperty(const wstring& strName)
    {
        return strName == L"ShaderPass" ||
            strName == L"TextureIndex" ||
            strName == L"MaskTextureIndex" ||
            strName == L"EffectTiling" ||
            strName == L"EffectOffset" ||
            strName == L"MaskPower" ||
            strName == L"EffectIntensity" ||
            strName == L"RevealProgress" ||
            strName == L"RevealSoftness" ||
            strName == L"MaskChannel" ||
            strName == L"InvertMask";
    }

    void* Find_PropertyData(IReflectable* pHolder, const wchar_t* szName,
        PROP_TYPE eType)
    {
        if (!pHolder)
            return nullptr;

        for (auto& prop : pHolder->Get_Properties())
        {
            if (prop.strName == szName && prop.eType == eType)
                return pHolder->Get_PropertyPtr(prop.uOffset);
        }

        return nullptr;
    }

    template<typename T>
    T* Find_PropertyDataT(IReflectable* pHolder, const wchar_t* szName,
        PROP_TYPE eType)
    {
        return static_cast<T*>(Find_PropertyData(pHolder, szName, eType));
    }
}

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

    const bool bEffectPart =
        dynamic_cast<Client::CUI_Effect*>(pHolder) != nullptr;

    for (auto& prop : props)
    {
        if (prop.strName == L"FontTag" || prop.strName == L"Align")
            continue;

        if (bEffectPart && Is_EffectInspectorProperty(prop.strName))
            continue;

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
            std::wstring* pW = static_cast<std::wstring*>(pData);

            char buf[256] = {};
            strcpy_s(buf, ToUtf8(*pW).c_str());

            if (ImGui::InputText(name.c_str(), buf, sizeof(buf)))
            {
                *pW = StrToWstr(buf);
                m_pPanel_Manager->Get_UIContext().bDirty = true;
            }

            const bool bTextureProperty =
                prop.strName == L"TextureProtoTag" ||
                prop.strName == L"MaskTextureProtoTag";

            if (bTextureProperty && ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* pPayload =
                    ImGui::AcceptDragDropPayload(DND_FILE_PATH))
                {
                    std::string strPath(static_cast<const char*>(pPayload->Data));
                    std::string strExt = std::filesystem::path(strPath).extension().string();

                    for (auto& c : strExt)
                        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

                    if (strExt == ".png" || strExt == ".dds")
                    {
                        CLevel_Tool* pLevel = m_pPanel_Manager->Get_Level();

                        if (pLevel)
                        {
                            _wstring strProtoTag =
                                pLevel->Register_TextureProto(StrToWstr(strPath));

                            if (!strProtoTag.empty())
                            {
                                *pW = strProtoTag;
                                m_pPanel_Manager->Get_UIContext().bDirty = true;

                                UI_SELECTION& sel =
                                    m_pPanel_Manager->Get_UIContext().Selection;

                                if (sel.pPart)
                                {
                                    if (auto* pImage =
                                        dynamic_cast<Client::CUI_Image*>(sel.pPart))
                                    {
                                        if (prop.strName == L"TextureProtoTag")
                                            pImage->Set_Texture(
                                                ETOUI(TOOL_LEVEL::EDIT),
                                                strProtoTag);
                                    }
                                    else if (auto* pAnim =
                                        dynamic_cast<Client::CUI_SpriteAnim*>(sel.pPart))
                                    {
                                        if (prop.strName == L"TextureProtoTag")
                                            pAnim->Set_Texture(
                                                ETOUI(TOOL_LEVEL::EDIT),
                                                strProtoTag);
                                    }
                                    else if (auto* pEffect =
                                        dynamic_cast<Client::CUI_Effect*>(sel.pPart))
                                    {
                                        if (prop.strName == L"TextureProtoTag")
                                            pEffect->Set_Texture(
                                                ETOUI(TOOL_LEVEL::EDIT),
                                                strProtoTag);
                                        else if (prop.strName == L"MaskTextureProtoTag")
                                            pEffect->Set_MaskTexture(
                                                ETOUI(TOOL_LEVEL::EDIT),
                                                strProtoTag);
                                    }
                                    else if (auto* pGauge =
                                        dynamic_cast<Client::CUI_GaugeFill*>(sel.pPart))
                                    {
                                        if (prop.strName == L"TextureProtoTag")
                                            pGauge->Set_Texture(
                                                ETOUI(TOOL_LEVEL::EDIT),
                                                strProtoTag);
                                    }
                                    else if (auto* pCurtain =
                                        dynamic_cast<Client::CUI_Curtain*>(sel.pPart))
                                    {
                                        if (prop.strName == L"TextureProtoTag")
                                            pCurtain->Set_Texture(
                                                ETOUI(TOOL_LEVEL::EDIT), strProtoTag);
                                    }
                                    else if (auto* pEraser =
                                        dynamic_cast<Client::CUI_CurtainAnimBase*>(sel.pPart))
                                    {
                                        if (prop.strName == L"TextureProtoTag")
                                            pEraser->Set_Texture(
                                                ETOUI(TOOL_LEVEL::EDIT), strProtoTag);
                                    }
                                }
                            }
                        }
                    }
                }

                ImGui::EndDragDropTarget();
            }
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
    CPreview_DeformCar_Main* pDeformCarMain = dynamic_cast<CPreview_DeformCar_Main*>(ctx.pOwner);
    CPreview_DeformCar_Demo* pDeformCarDemo = dynamic_cast<CPreview_DeformCar_Demo*>(ctx.pOwner);
    CModel* pModel = ctx.pModel;

    if ((!pActor && !pDeformCarMain && !pDeformCarDemo) || !pModel)
        return;

    auto IsMeshVisible = [pActor, pDeformCarMain, pDeformCarDemo](_uint iMesh)
        {
            if (pActor)
                return pActor->Is_MeshVisible(iMesh);
            if (pDeformCarMain)
                return pDeformCarMain->Is_MeshVisible(iMesh);
            return pDeformCarDemo->Is_MeshVisible(iMesh);
        };

    auto SetMeshVisible = [pActor, pDeformCarMain, pDeformCarDemo](_uint iMesh, _bool bVisible)
        {
            if (pActor)
                pActor->Set_MeshVisible(iMesh, bVisible);
            else if (pDeformCarMain)
                pDeformCarMain->Set_MeshVisible(iMesh, bVisible);
            else
                pDeformCarDemo->Set_MeshVisible(iMesh, bVisible);
        };

    auto SetAllMeshVisible = [pActor, pDeformCarMain, pDeformCarDemo](_bool bVisible)
        {
            if (pActor)
                pActor->Set_AllMeshVisible(bVisible);
            else if (pDeformCarMain)
                pDeformCarMain->Set_AllMeshVisible(bVisible);
            else
                pDeformCarDemo->Set_AllMeshVisible(bVisible);
        };

    auto SetSoloMesh = [pActor, pDeformCarMain, pDeformCarDemo](_uint iMesh)
        {
            if (pActor)
                pActor->Set_SoloMesh(iMesh);
            else if (pDeformCarMain)
                pDeformCarMain->Set_SoloMesh(iMesh);
            else
                pDeformCarDemo->Set_SoloMesh(iMesh);
        };

    if (!ImGui::CollapsingHeader("Meshes", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    const _uint iNumMeshes = (_uint)pModel->Get_NumMeshes();

    ImGui::Text("Mesh Count: %u", iNumMeshes);

    if (ImGui::Button("All On"))
        SetAllMeshVisible(true);

    ImGui::SameLine();

    if (ImGui::Button("All Off"))
        SetAllMeshVisible(false);

    ImGui::Separator();

    ImGui::BeginChild("MeshVisibilityList", ImVec2(0, 180.f), true);

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        ImGui::PushID((int)i);

        bool bVisible = IsMeshVisible(i);
        if (ImGui::Checkbox("##visible", &bVisible))
            SetMeshVisible(i, bVisible);

        ImGui::SameLine();

        if (ImGui::SmallButton("Solo"))
            SetSoloMesh(i);

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

void CPanel_Inspector::Render_UIContainerTransform(CUIContainerObject* pContainer, _bool bDefaultOpen)
{
    if (!pContainer)
        return;

    ImGuiTreeNodeFlags iFlags = {};
    if (bDefaultOpen)
        iFlags |= ImGuiTreeNodeFlags_DefaultOpen;

    if (!ImGui::CollapsingHeader("Container Transform", iFlags))
        return;

    CTransform* pT = pContainer->Get_Transform();
    if (!pT)
        return;

    UI_CONTEXT& uictx = m_pPanel_Manager->Get_UIContext();

    ImGui::PushID("ContainerTransform");

    _vector vPos = pT->Get_State(STATE::POSITION);
    float pos[2] =
    {
        XMVectorGetX(vPos),
        XMVectorGetY(vPos)
    };

    if (ImGui::DragFloat2("Position (x,y)", pos, 1.f))
    {
        pT->Set_State(
            STATE::POSITION,
            XMVectorSet(
                pos[0],
                pos[1],
                XMVectorGetZ(vPos),
                1.f));

        uictx.bDirty = true;
    }

    _float3 vScale = pT->Get_Scaled();
    float scale[2] =
    {
        vScale.x,
        vScale.y
    };

    if (ImGui::DragFloat2("Scale (x,y)", scale, 0.01f, 0.01f, 100.f))
    {
        pT->Set_Scale(scale[0], scale[1], vScale.z);
        uictx.bDirty = true;
    }

    float fRotationDeg = Get_UIRotationZDeg(pT);

    if (ImGui::DragFloat(
        "Rotation Z",
        &fRotationDeg,
        0.5f,
        -180.f,
        180.f,
        "%.1f deg"))
    {
        Set_UIRotationZDeg(pT, fRotationDeg);
        uictx.bDirty = true;
    }

    ImGui::PopID();
}

void CPanel_Inspector::Render_UIPartTransform(CUIPartObject* pPart)
{

    if (!ImGui::CollapsingHeader("Part Transform (UI)", ImGuiTreeNodeFlags_DefaultOpen))
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

    _float fRotationDeg = Get_UIRotationZDeg(pT);

    if (ImGui::DragFloat("Rotation Z", &fRotationDeg, 0.5f, -180.f, 180.f, "%.1f deg"))
    {
        Set_UIRotationZDeg(pT, fRotationDeg);
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

    _float fTiltX = sel.pContainer->Get_TiltX();
    _float fTiltY = sel.pContainer->Get_TiltY();
    _float fDist = sel.pContainer->Get_PerspDistance();
    bool bDirty = false;
    bDirty |= ImGui::DragFloat("Tilt X", &fTiltX, 0.5f, -60.f, 60.f, "%.1f deg");
    bDirty |= ImGui::DragFloat("Tilt Y", &fTiltY, 0.5f, -60.f, 60.f, "%.1f deg");
    bDirty |= ImGui::DragFloat("Persp Dist", &fDist, 1.f, 0.f, 5000.f, "%.0f (0=off)");
    if (bDirty) {
        sel.pContainer->Set_Tilt(fTiltX, fTiltY);
        sel.pContainer->Set_PerspDistance(fDist);
        uictx.bDirty = true;
    }

    if (nullptr == sel.pPart)
    {
        std::string strCName = ToUtf8(sel.pContainer->Get_ObjectTag());
        ImGui::Text("[UI Container] %s",
            strCName.empty() ? "-" : strCName.c_str());
        _bool bActive = sel.pContainer->Is_Active() != false;
        if (ImGui::Checkbox("Active", &bActive))
        {
            sel.pContainer->Set_Active(bActive);
        }
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

        ImGui::Separator();
        Render_Properties(sel.pContainer);
        return;
    }

    CUIPartObject* pPart = sel.pPart;

    std::string strName = ToUtf8(sel.strPartTag);
    ImGui::Text("[UI Part]");
    ImGui::SameLine();
    ImGui::TextDisabled("%s", strName.empty() ? "-" : strName.c_str());

    static _wstring s_EditSourceTag;
    static char s_szPartName[128] = {};

    if (s_EditSourceTag != sel.strPartTag)
    {
        s_EditSourceTag = sel.strPartTag;
        strncpy_s(s_szPartName, ToUtf8(sel.strPartTag).c_str(), sizeof(s_szPartName) - 1);
    }

    ImGui::SetNextItemWidth(-1.f);
    const bool bEnter =
        ImGui::InputText("Part Name", s_szPartName, sizeof(s_szPartName),
            ImGuiInputTextFlags_EnterReturnsTrue);

    const bool bCommit = bEnter || ImGui::IsItemDeactivatedAfterEdit();

    if (bCommit)
    {
        std::string strNewName = s_szPartName;

        if (!strNewName.empty())
        {
            _wstring strNewTag = StrToWstr(strNewName);

            CLevel_Tool* pLevel = m_pPanel_Manager->Get_Level();
            if (pLevel && SUCCEEDED(pLevel->Rename_UIPart(
                sel.pContainer,
                sel.strPartTag,
                strNewTag)))
            {
                sel.strPartTag = strNewTag;
                s_EditSourceTag = strNewTag;
                uictx.bDirty = true;
            }
            else
            {
                strncpy_s(s_szPartName, ToUtf8(sel.strPartTag).c_str(), sizeof(s_szPartName) - 1);
            }
        }
        else
        {
            strncpy_s(s_szPartName, ToUtf8(sel.strPartTag).c_str(), sizeof(s_szPartName) - 1);
        }
    }

    ImGui::Separator();

    struct { RENDERUIID v; const char* n; } layers[] = {
        { RENDERUIID::BACK, "BACK" }, { RENDERUIID::MIDDLE, "MIDDLE" }, { RENDERUIID::FRONT, "FRONT" },
        { RENDERUIID::CURTAIN, "CURTAIN" }, {RENDERUIID::FLASH, "FLASH"}
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
    Render_UIContainerTransform(sel.pContainer, false);
    ImGui::Separator();
    Render_UIPartTransform(pPart);
    Render_EffectInspector(pPart);
    Render_Properties(pPart);
    Render_TextInspector(pPart);
    Render_SpriteAnimControl(pPart);
    Render_UIPartBounceInspector(sel.pContainer, pPart, sel.strPartTag);
}

void CPanel_Inspector::Render_EffectInspector(CUIPartObject* pPart)
{
    auto* pEffect = dynamic_cast<Client::CUI_Effect*>(pPart);
    if (!pEffect)
        return;

    if (!ImGui::CollapsingHeader("Effect Shader",
        ImGuiTreeNodeFlags_DefaultOpen))
        return;

    UI_CONTEXT& uictx = m_pPanel_Manager->Get_UIContext();
    IReflectable* pReflect = pEffect;

    int* pShaderPass = Find_PropertyDataT<int>(
        pReflect, L"ShaderPass", PROP_TYPE::INT);
    if (!pShaderPass)
        return;

    const UI_EFFECT_PASS_ITEM* pCurItem = Find_EffectPassItem(*pShaderPass);
    const char* szCurPass = pCurItem ? pCurItem->szName : "Unsupported";

    if (ImGui::BeginCombo("Shader Pass", szCurPass))
    {
        for (const auto& item : g_UIEffectPassItems)
        {
            const bool bSelected = *pShaderPass == item.iPass;
            if (ImGui::Selectable(item.szName, bSelected))
            {
                *pShaderPass = item.iPass;
                pCurItem = &item;
                uictx.bDirty = true;
            }

            if (bSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    pCurItem = Find_EffectPassItem(*pShaderPass);
    if (!pCurItem)
    {
        ImGui::TextColored(ImVec4(1.f, 0.45f, 0.35f, 1.f),
            "Unsupported CUI_Effect pass: %d", *pShaderPass);
        return;
    }

    ImGui::TextDisabled("%s", pCurItem->szDesc);

    auto DrawInt = [&](const char* szLabel, const wchar_t* szPropName,
        int iMin, int iMax)
    {
        int* pValue = Find_PropertyDataT<int>(
            pReflect, szPropName, PROP_TYPE::INT);
        if (!pValue)
            return;

        int iValue = *pValue;
        if (ImGui::DragInt(szLabel, &iValue, 1.f, iMin, iMax))
        {
            if (iValue < iMin)
                iValue = iMin;
            if (iValue > iMax)
                iValue = iMax;

            *pValue = iValue;
            uictx.bDirty = true;
        }
    };

    auto DrawFloat = [&](const char* szLabel, const wchar_t* szPropName,
        float fSpeed, float fMin, float fMax, const char* szFmt)
    {
        float* pValue = Find_PropertyDataT<float>(
            pReflect, szPropName, PROP_TYPE::FLOAT);
        if (!pValue)
            return;

        if (ImGui::DragFloat(szLabel, pValue, fSpeed, fMin, fMax, szFmt))
            uictx.bDirty = true;
    };

    auto DrawFloat2 = [&](const char* szLabel, const wchar_t* szPropName,
        float fSpeed)
    {
        _float2* pValue = Find_PropertyDataT<_float2>(
            pReflect, szPropName, PROP_TYPE::FLOAT2);
        if (!pValue)
            return;

        if (ImGui::DragFloat2(szLabel, reinterpret_cast<float*>(pValue), fSpeed))
            uictx.bDirty = true;
    };

    std::wstring* pTextureTag = Find_PropertyDataT<std::wstring>(
        pReflect, L"TextureProtoTag", PROP_TYPE::WSTRING);
    std::wstring* pMaskTextureTag = Find_PropertyDataT<std::wstring>(
        pReflect, L"MaskTextureProtoTag", PROP_TYPE::WSTRING);

    if (pCurItem->bSourceTexture)
    {
        DrawInt("TextureIndex", L"TextureIndex", 0, 9999);
        if (!pTextureTag || pTextureTag->empty())
            ImGui::TextColored(ImVec4(1.f, 0.45f, 0.35f, 1.f),
                "TextureProtoTag required");
    }
    else
    {
        ImGui::TextDisabled("TextureIndex: not used by this pass");
    }

    if (pCurItem->bMaskTexture)
    {
        DrawInt("MaskTextureIndex", L"MaskTextureIndex", 0, 9999);
        if (!pMaskTextureTag || pMaskTextureTag->empty())
            ImGui::TextColored(ImVec4(1.f, 0.45f, 0.35f, 1.f),
                "MaskTextureProtoTag required");
    }

    if (pCurItem->bMaskTexture)
    {
        int* pMaskChannel = Find_PropertyDataT<int>(
            pReflect, L"MaskChannel", PROP_TYPE::INT);
        if (pMaskChannel)
        {
            const char* szChannels[] = { "R", "G", "B", "A" };
            int iChannel = *pMaskChannel;
            if (iChannel < 0 || iChannel > 3)
                iChannel = 3;

            if (ImGui::Combo("MaskChannel", &iChannel,
                szChannels, IM_ARRAYSIZE(szChannels)))
            {
                *pMaskChannel = iChannel;
                uictx.bDirty = true;
            }
        }

        int* pInvertMask = Find_PropertyDataT<int>(
            pReflect, L"InvertMask", PROP_TYPE::INT);
        if (pInvertMask)
        {
            bool bInvert = *pInvertMask != 0;
            if (ImGui::Checkbox("InvertMask", &bInvert))
            {
                *pInvertMask = bInvert ? 1 : 0;
                uictx.bDirty = true;
            }
        }

        DrawFloat("MaskPower", L"MaskPower", 0.05f, 0.001f, 32.f, "%.3f");
    }

    DrawFloat("EffectIntensity", L"EffectIntensity", 0.05f, 0.f, 100.f, "%.3f");

    if (pCurItem->bUV)
    {
        DrawFloat2("EffectTiling", L"EffectTiling", 0.01f);
        DrawFloat2("EffectOffset", L"EffectOffset", 0.01f);
    }

    if (pCurItem->bReveal)
    {
        float* pProgress = Find_PropertyDataT<float>(
            pReflect, L"RevealProgress", PROP_TYPE::FLOAT);
        if (pProgress)
        {
            if (ImGui::SliderFloat("RevealProgress", pProgress, 0.f, 1.f, "%.3f"))
                uictx.bDirty = true;
        }

        DrawFloat("RevealSoftness", L"RevealSoftness", 0.001f, 0.0001f, 1.f, "%.4f");
    }
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

void CPanel_Inspector::Render_TextInspector(CUIPartObject* pPart)
{
    auto* pText = dynamic_cast<Client::CUI_Text*>(pPart);
    if (!pText)
        return;

    if (!ImGui::CollapsingHeader("Text Properties",
        ImGuiTreeNodeFlags_DefaultOpen))
        return;

    UI_CONTEXT& uictx = m_pPanel_Manager->Get_UIContext();

    std::string strCur = ToUtf8(pText->Get_FontTag());
    if (ImGui::BeginCombo("Font", strCur.c_str()))
    {
        for (auto& f : Client::g_UIFonts)
        {
            std::string strTag = ToUtf8(f.szTag);
            if (ImGui::Selectable(strTag.c_str(), strCur == strTag))
            {
                pText->Set_FontTag(f.szTag);
                uictx.bDirty = true;
            }
        }
        ImGui::EndCombo();
    }

    int iAlign = pText->Get_Align();
    Draw_StateCombo("Align", iAlign, g_TextAlignNames);
    if (iAlign != pText->Get_Align())
    {
        pText->Set_Align(iAlign);
        uictx.bDirty = true;
    }
}

void CPanel_Inspector::Render_UIPartBounceInspector(CUIContainerObject* pContainer, CUIPartObject* pPart, const _wstring& strPartTag)
{
    if (!pPart || !pContainer)
        return;

    if (!ImGui::CollapsingHeader("Bounce Test"))
        return;

    static _float2 s_vDirection = { 0.f, 1.f };
    static _float  s_fDistance = 12.f;
    static _float  s_fDuration = 0.28f;
    static _float  s_fWaveCount = 1.5f;
    static _float  s_fDamping = 1.2f;

    ImGui::DragFloat2("Direction", (float*)&s_vDirection, 0.05f);
    ImGui::DragFloat("Distance", &s_fDistance, 0.5f, 0.f, 200.f);
    ImGui::DragFloat("Duration", &s_fDuration, 0.01f, 0.01f, 5.f);
    ImGui::DragFloat("Wave Count", &s_fWaveCount, 0.05f, 0.1f, 10.f);
    ImGui::DragFloat("Damping", &s_fDamping, 0.05f, 0.f, 8.f);

    auto* pOwner = dynamic_cast<Client::IUIAnimatorOwner*>(pContainer);
    Client::CUIAnimatorCom* pAnimator = pOwner ? pOwner->Get_UIAnimatorCom() : nullptr;

    if (!pAnimator)
    {
        ImGui::TextDisabled("UIAnimatorCom is not available.");
        return;
    }

    if (ImGui::Button("Bounce Play"))
    {
        Client::CUIAnimatorCom::UI_BOUNCE_DESC Desc{};
        Desc.vDirection = s_vDirection;
        Desc.fDistance = s_fDistance;
        Desc.fDuration = s_fDuration;
        Desc.fWaveCount = s_fWaveCount;
        Desc.fDamping = s_fDamping;
        pAnimator->Play_Bounce(strPartTag, Desc);
    }

    ImGui::SameLine();

    if (ImGui::Button("Bounce Stop"))
        pAnimator->Stop_Bounce(strPartTag, true);

    ImGui::TextDisabled(pAnimator->Is_Bouncing(strPartTag) ? "Playing" : "Idle");
}

CPanel_Inspector* CPanel_Inspector::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CPanel_Inspector(pDevice, pContext);
}

float CPanel_Inspector::Get_UIRotationZDeg(CTransform* pT)
{
    _vector vRight = pT->Get_State(STATE::RIGHT);
    return XMConvertToDegrees(atan2f(XMVectorGetY(vRight), XMVectorGetX(vRight)));
}

float CPanel_Inspector::Get_UIRotationYDeg(CTransform* pT)
{
    _vector vLook = pT->Get_State(STATE::LOOK);
    return XMConvertToDegrees(atan2f(XMVectorGetX(vLook), XMVectorGetZ(vLook)));
}

float CPanel_Inspector::Get_UIRotationXDeg(CTransform* pT)
{
    _vector vUp = pT->Get_State(STATE::UP);
    return XMConvertToDegrees(atan2f(XMVectorGetZ(vUp), XMVectorGetY(vUp)));
}

void CPanel_Inspector::Set_UIRotationZDeg(CTransform* pT, float fDeg)
{
    _float3 vScale = pT->Get_Scaled();
    _vector vPos = pT->Get_State(STATE::POSITION);

    _matrix matRot = XMMatrixRotationZ(XMConvertToRadians(fDeg));

    pT->Set_State(STATE::RIGHT,
        XMVector3TransformNormal(XMVectorSet(vScale.x, 0.f, 0.f, 0.f), matRot));

    pT->Set_State(STATE::UP,
        XMVector3TransformNormal(XMVectorSet(0.f, vScale.y, 0.f, 0.f), matRot));

    pT->Set_State(STATE::LOOK, XMVectorSet(0.f, 0.f, vScale.z, 0.f));
    pT->Set_State(STATE::POSITION, vPos);
}

void	CPanel_Inspector::Set_UIRotationYDeg(CTransform* pT, float fDeg)
{
    _float3 vScale = pT->Get_Scaled();
    _vector vPos = pT->Get_State(STATE::POSITION);

    _matrix matRot = XMMatrixRotationY(XMConvertToRadians(fDeg));

    pT->Set_State(STATE::RIGHT,
        XMVector3TransformNormal(XMVectorSet(vScale.x, 0.f, 0.f, 0.f), matRot));

    pT->Set_State(STATE::UP,
        XMVector3TransformNormal(XMVectorSet(0.f, vScale.y, 0.f, 0.f), matRot));

    pT->Set_State(STATE::LOOK,
        XMVector3TransformNormal(XMVectorSet(0.f, 0.f, vScale.z, 0.f), matRot));

    pT->Set_State(STATE::POSITION, vPos);
}

void	CPanel_Inspector::Set_UIRotationXDeg(CTransform* pT, float fDeg)
{
    _float3 vScale = pT->Get_Scaled();
    _vector vPos = pT->Get_State(STATE::POSITION);

    _matrix matRot = XMMatrixRotationX(XMConvertToRadians(fDeg));

    pT->Set_State(STATE::RIGHT,
        XMVector3TransformNormal(XMVectorSet(vScale.x, 0.f, 0.f, 0.f), matRot));

    pT->Set_State(STATE::UP,
        XMVector3TransformNormal(XMVectorSet(0.f, vScale.y, 0.f, 0.f), matRot));

    pT->Set_State(STATE::LOOK,
        XMVector3TransformNormal(XMVectorSet(0.f, 0.f, vScale.z, 0.f), matRot));

    pT->Set_State(STATE::POSITION, vPos);
}

void CPanel_Inspector::Free()
{
    __super::Free();
}
