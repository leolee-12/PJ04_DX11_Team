#include "Panel_Viewport.h"

#include "EditInstance.h"
#include "Level_Edit.h"

#include "GameInstance.h"
#include "GameObject.h"
#include "Transform.h"
#include "GameContrnt_Events.h"     // UI_RBTN_PROBE / WORLD_RBTN_DOWN

#include "imgui.h"
#include "ImGuizmo.h"

namespace
{
    ImGuizmo::OPERATION ToImGuizmoOp(MapTool::GIZMO_OP eOp)
    {
        switch (eOp)
        {
        case MapTool::GIZMO_OP::ROTATE: return ImGuizmo::ROTATE;
        case MapTool::GIZMO_OP::SCALE:  return ImGuizmo::SCALE;
        default:                        return ImGuizmo::TRANSLATE;
        }
    }

    _bool Compute_ViewportPickingRay(
        Engine::CGameInstance_Proxy* pProxy,
        _float fNdcX,
        _float fNdcY,
        _vector* pOutOrigin,
        _vector* pOutDir,
        PROJ_TYPE eProjType = PROJ_TYPE::PERSPEC)
    {
        if (nullptr == pProxy || nullptr == pOutOrigin || nullptr == pOutDir)
            return false;

        const _float4x4* pView = pProxy->Get_Matrix(D3DTS::VIEW, eProjType);
        const _float4x4* pProj = pProxy->Get_Matrix(D3DTS::PROJ, eProjType);
        if (nullptr == pView || nullptr == pProj)
            return false;

        const _matrix matInvViewProj =
            XMMatrixInverse(nullptr, XMLoadFloat4x4(pView) * XMLoadFloat4x4(pProj));

        _vector vNear = XMVector4Transform(
            XMVectorSet(fNdcX, fNdcY, 0.f, 1.f),
            matInvViewProj);
        _vector vFar = XMVector4Transform(
            XMVectorSet(fNdcX, fNdcY, 1.f, 1.f),
            matInvViewProj);

        const _float fNearW = XMVectorGetW(vNear);
        const _float fFarW = XMVectorGetW(vFar);

        if ((fNearW > -1e-6f && fNearW < 1e-6f)
            || (fFarW > -1e-6f && fFarW < 1e-6f))
        {
            return false;
        }

        vNear = XMVectorScale(vNear, 1.f / fNearW);
        vFar = XMVectorScale(vFar, 1.f / fFarW);

        *pOutOrigin = XMVectorSetW(vNear, 1.f);
        *pOutDir = XMVector3Normalize(XMVectorSubtract(vFar, vNear));
        return true;
    }
}

CPanel_Viewport::CPanel_Viewport(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "Viewport");
}

HRESULT CPanel_Viewport::Initialize()
{
    D3D11_BLEND_DESC bd{};
    bd.RenderTarget[0].BlendEnable = FALSE;                                  // 블렌딩 OFF = RT의 RGB 그대로
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    //bd.AlphaToCoverageEnable = FALSE;
    //bd.IndependentBlendEnable = FALSE;
    //bd.RenderTarget[0].BlendEnable = FALSE;
    //bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    if (FAILED(m_pDevice->CreateBlendState(&bd, &m_pViewportOpaqueBlend)))
        return E_FAIL;

    m_ViewportDrawDesc = { m_pContext, m_pViewportOpaqueBlend };
    return S_OK;
}

void CPanel_Viewport::Render()
{
    if (!Begin_Panel())
    {
        End_Panel();
        return;
    }

    CEditInstance* pEI = CEditInstance::GetInstance();

    ImVec2 vAvail = ImGui::GetContentRegionAvail();
    if (vAvail.x < 1.f || vAvail.y < 1.f)
    {
        End_Panel();
        return;
    }

    // 16:9 종횡비 유지 레터박스
    _float fAvailAspect = vAvail.x / vAvail.y;
    ImVec2 vSize = {};
    if (fAvailAspect > m_fTargetAspect)
    {
        vSize.y = vAvail.y;
        vSize.x = vAvail.y * m_fTargetAspect;
    }
    else
    {
        vSize.x = vAvail.x;
        vSize.y = vAvail.x / m_fTargetAspect;
    }

    ImVec2 vOffset((vAvail.x - vSize.x) * 0.5f, (vAvail.y - vSize.y) * 0.5f);
    ImVec2 vCursor = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(vCursor.x + vOffset.x, vCursor.y + vOffset.y));

    ImVec2 vPos = ImGui::GetCursorScreenPos();

    ID3D11ShaderResourceView* pSRV = pEI->Get_SceneSRV();

    if (pSRV)
    {
        ImDrawList* pDrawList = ImGui::GetWindowDrawList();
        pDrawList->AddCallback(Viewport_DisableBlend, &m_ViewportDrawDesc);
        ImGui::Image((ImTextureID)pSRV, vSize);
        pDrawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
    }
    else
        ImGui::Dummy(vSize);

    // 뷰포트 스크린 사각형 공유(오버레이/피킹용)
    pEI->Set_ViewportRect(_float2(vPos.x, vPos.y), _float2(vSize.x, vSize.y));

    CLevel_Edit* pLevel = pEI->Get_Level();
    if (nullptr == pLevel)
    {
        End_Panel();
        return;
    }

    _bool bRightHeld = ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right);
    _bool bImageHovered = ImGui::IsItemHovered();

    if (bImageHovered && !bRightHeld && !ImGuizmo::IsOver())
    {
        ImVec2 mouse = ImGui::GetMousePos();

        // 이미지 로컬 좌표 → NDC
        _float ndcX = ((mouse.x - vPos.x) / vSize.x) * 2.f - 1.f;
        _float ndcY = 1.f - ((mouse.y - vPos.y) / vSize.y) * 2.f;

        XMVECTOR origin = XMVectorZero();
        XMVECTOR dir = XMVectorSet(0.f, 0.f, 1.f, 0.f);

        if (!Compute_ViewportPickingRay(
            m_pGI_Proxy,
            ndcX,
            ndcY,
            &origin,
            &dir,
            PROJ_TYPE::PERSPEC))
        {
            // 예외 시 기존 공용 경로 fallback
            m_pGI_Proxy->Compute_PickingRay(ndcX, ndcY, &origin, &dir);
        }

        // 좌클릭: 배치 모드면 기존 배치, 일반 모드면 EnvObject 선택
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsUsing())
        {
            if (pLevel->Is_PlaceMode())
                pLevel->Pick_And_Place(origin, dir);
            else
                pLevel->Pick_And_SelectEnvObject(origin, dir);
        }

        //if (m_pGI_Proxy->Mouse_Down(DIMB::WHEEL))
        //{
        //    _float2 vNDC = { ndcX, ndcY };
        //    UI_RBTN_PROBE eProbe = { vNDC, false };
        //    m_pGI_Proxy->Publish(TEXT("UI_RButton_Probe"), &eProbe);
        //
        //    if (eProbe.bConsumed)
        //    {
        //        End_Panel();
        //        return;
        //    }
        //
        //    WORLD_RBTN_DOWN eEvent = { vNDC };
        //    m_pGI_Proxy->Publish(TEXT("World_RButton_Click"), &eEvent);
        //}

        // ESC : 배치 모드 취소
        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            pLevel->End_PlaceMode();
    }

    pLevel->Set_CameraActive(bRightHeld);

    CGameObject* pSelected = pLevel->Get_Selected();
    if (pSelected)
        Draw_Gizmo(pSelected, vPos, vSize);

    End_Panel();
}

void CPanel_Viewport::Viewport_DisableBlend(const ImDrawList*, const ImDrawCmd* cmd)
{
    auto* pDesc = static_cast<VIEWPORT_DRAW_DESC*>(cmd->UserCallbackData);
    const float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
    pDesc->pContext->OMSetBlendState(pDesc->pBlendState, blendFactor, 0xffffffff);
}

void CPanel_Viewport::Draw_Gizmo(CGameObject* pSelected, const ImVec2& vImagePos, const ImVec2& vImageSize)
{
    ImGuizmo::BeginFrame();
    ImGuizmo::Enable(true);

    // 이 패널의 DrawList에 기즈모를 그린다.
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(vImagePos.x, vImagePos.y, vImageSize.x, vImageSize.y);

    PROJ_TYPE eProjType = pSelected->Get_ProjType();
    ImGuizmo::SetOrthographic(eProjType == PROJ_TYPE::ORTHO);

    const _float4x4* pView = m_pGI_Proxy->Get_Matrix(D3DTS::VIEW, eProjType);
    const _float4x4* pProj = m_pGI_Proxy->Get_Matrix(D3DTS::PROJ, eProjType);
    if (!pView || !pProj)
        return;

    CTransform* pTransform = pSelected->Get_Transform();
    _float4x4 matWorld = *pTransform->Get_WorldMatrixPtr();

    ImGuizmo::OPERATION eOp = ToImGuizmoOp(CEditInstance::GetInstance()->Get_GizmoOp());

    _float snap[3] = { 0, 0, 0 };
    if (eOp == ImGuizmo::TRANSLATE && eProjType == PROJ_TYPE::ORTHO)
        snap[0] = snap[1] = snap[2] = 1.f;

    ImGuizmo::Manipulate(
        (float*)pView,
        (float*)pProj,
        eOp,
        ImGuizmo::LOCAL,
        (float*)&matWorld,
        nullptr, snap);

    if (ImGuizmo::IsUsing())
    {
        if (eOp == ImGuizmo::TRANSLATE && eProjType == PROJ_TYPE::ORTHO)
        {
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

CPanel_Viewport* CPanel_Viewport::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CPanel_Viewport* pInstance = new CPanel_Viewport(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CToolApp");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CPanel_Viewport::Free()
{
    Safe_Release(m_pViewportOpaqueBlend);

    __super::Free();
}
