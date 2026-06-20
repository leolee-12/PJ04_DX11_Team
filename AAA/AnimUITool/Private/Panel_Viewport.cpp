#include "Panel_Viewport.h"
#include "imgui.h"
#include "Model.h"
#include "Preview_Actor.h"
#include "GameInstance_Proxy.h"
#include "Panel_Manager.h"
#include "PartObject.h"

using namespace AnimUITool;

CPanel_Viewport::CPanel_Viewport(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "Viewport");
}

void CPanel_Viewport::Render()
{
    ImGui::Begin(m_szName);

    ImVec2 vAvail = ImGui::GetContentRegionAvail();
    if (vAvail.x >= 1.f && vAvail.y >= 1.f)
    {
        const float fAvailAspect = vAvail.x / vAvail.y;
        ImVec2 vSize;
        if (fAvailAspect > m_fTargetAspect) { vSize.y = vAvail.y; vSize.x = vAvail.y * m_fTargetAspect; }
        else { vSize.x = vAvail.x; vSize.y = vAvail.x / m_fTargetAspect; }

        ImVec2 vOffset((vAvail.x - vSize.x) * 0.5f, (vAvail.y - vSize.y) * 0.5f);
        ImVec2 vCursor = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(vCursor.x + vOffset.x, vCursor.y + vOffset.y));

        ImVec2 vImagePos = ImGui::GetCursorScreenPos();

        if (m_pSRV) 
            ImGui::Image((ImTextureID)m_pSRV, vSize);
        else        
            ImGui::Dummy(vSize);

        m_bHovered = ImGui::IsItemHovered();

        // ── 본 위치 점 오버레이 ──
        ANIM_CONTEXT& ctx = m_pPanel_Manager->Get_Context();
        if (ctx.pOwner && ctx.pModel)
        {
            const _float4x4* pView = m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC);
            const _float4x4* pProj = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);
            const _float4x4* pWorld = nullptr;

            if (auto* pPart = dynamic_cast<CPartObject*>(ctx.pOwner))
            {
                pWorld = pPart->Get_CombinedWorldMatrixPtr();
            }
            else if (ctx.pOwner->Get_Transform())
            {
                pWorld = ctx.pOwner->Get_Transform()->Get_WorldMatrixPtr();
            }

            if (pView && pProj && pWorld)
            {
                XMMATRIX mWorld = XMLoadFloat4x4(pWorld);
                XMMATRIX mVP = XMLoadFloat4x4(pView) * XMLoadFloat4x4(pProj);
                ImDrawList* dl = ImGui::GetWindowDrawList();

                const _uint iNum = ctx.pModel->Get_NumBones();
                for (_uint i = 0; i < iNum; ++i)
                {
                    const _float4x4* pBone = ctx.pModel->Get_BoneMatrixPtr(ctx.pModel->Get_BoneName(i));
                    if (!pBone) continue;

                    // Combined의 위치(행4) = 모델공간 본 위치 → 월드 → 클립
                    XMVECTOR posModel = XMVectorSet(pBone->_41, pBone->_42, pBone->_43, 1.f);
                    XMVECTOR posWorld = XMVector3TransformCoord(posModel, mWorld);
                    XMVECTOR clip = XMVector4Transform(XMVectorSetW(posWorld, 1.f), mVP);

                    float w = XMVectorGetW(clip);
                    if (w < 1e-5f) continue;                       // 카메라 뒤
                    float ndcX = XMVectorGetX(clip) / w;
                    float ndcY = XMVectorGetY(clip) / w;
                    ImVec2 sp(vImagePos.x + (ndcX + 1.f) * 0.5f * vSize.x,
                        vImagePos.y + (1.f - ndcY) * 0.5f * vSize.y);

                    _bool bSel = ((_int)i == ctx.iSelBone);
                    dl->AddCircleFilled(sp, bSel ? 6.f : 3.f,
                        bSel ? IM_COL32(255, 80, 80, 255)      // 선택: 빨강+큼
                        : IM_COL32(255, 230, 0, 170));     // 일반: 노랑
                }
            }
        }
    }

    ImGui::End();
}

void CPanel_Viewport::Set_SRV(ID3D11ShaderResourceView* pSRV)
{
    if (m_pSRV == pSRV)
        return;

    Safe_Release(m_pSRV);

    m_pSRV = pSRV;

    Safe_AddRef(m_pSRV);
}

CPanel_Viewport* CPanel_Viewport::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CPanel_Viewport(pDevice, pContext);
}

void CPanel_Viewport::Free()
{
    Safe_Release(m_pSRV);

    __super::Free();
}