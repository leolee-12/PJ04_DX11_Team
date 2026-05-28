#include "UIObject.h"
#include "GameInstance.h"

CUIObject::CUIObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject(pDevice, pContext)
    , m_iTexIndex{ 0 }
{
}

CUIObject::CUIObject(const CUIObject& Prototype)
    : CGameObject(Prototype)
    , m_iTexIndex{ Prototype.m_iTexIndex }
{
}

HRESULT CUIObject::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::ORTHO;
    return S_OK;
}

HRESULT CUIObject::Initialize(void* pArg)
{
    if (pArg != nullptr)
    {
        UIOBJECT_DESC* UIDesc = static_cast<UIOBJECT_DESC*>(pArg);
    }


    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;


    Update_UIState(_float2(100.f, 100.f));

    return S_OK;
}

void CUIObject::Priority_Update(_float fTimeDelta)
{
}

void CUIObject::Update(_float fTimeDelta)
{
}

void CUIObject::Late_Update(_float fTimeDelta)
{
}

HRESULT CUIObject::Render()
{
    return S_OK;
}

_float CUIObject::Get_ZOrder() const
{
    return XMVectorGetZ(m_pTransformCom->Get_State(STATE::POSITION));
}

void CUIObject::Update_UIState(const _float2& fScale)
{
    m_pTransformCom->Set_Scale(fScale.x, fScale.y, 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, 0.f, 0.1f, 1.f));
}

HRESULT CUIObject::Bind_ShaderResource(CShader* pShader, const _char* pConstantName, D3DTS eState, PROJ_TYPE eType)
{
    return pShader->Bind_Matrix(pConstantName, m_pGameInstance_Proxy->Get_Matrix(eState, eType));
}

void CUIObject::Picking_Update(const _float4x4& WorldMatrix, CTexture* pTex, _uint iTexIdx)
{
    const _float4x4* pProj = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::ORTHO);
    _float designW = 2.f / pProj->_11;
    _float designH = 2.f / pProj->_22;

    _float fActualW = m_pGameInstance_Proxy->Get_WindowWidth();
    _float fActualH = m_pGameInstance_Proxy->Get_WindowHeight();

    // 마우스 픽셀 → 오쏘 월드 좌표
    POINT pt = m_pGameInstance_Proxy->Get_MousePos();
    _float orthoX = (pt.x / fActualW) * designW - designW * 0.5f;
    _float orthoY = designH * 0.5f - (pt.y / fActualH) * designH;

    // 바운딩 박스 (Transform에서 중심/스케일 추출)
    _float3 scaled = _float3(
        XMVectorGetX(XMVector3Length(XMVectorSet(WorldMatrix._11, WorldMatrix._12, WorldMatrix._13, WorldMatrix._14))),
        XMVectorGetX(XMVector3Length(XMVectorSet(WorldMatrix._21, WorldMatrix._22, WorldMatrix._23, WorldMatrix._24))),
        XMVectorGetX(XMVector3Length(XMVectorSet(WorldMatrix._31, WorldMatrix._32, WorldMatrix._33, WorldMatrix._34)))
    );

    _vector vPos = XMVectorSet(WorldMatrix._41, WorldMatrix._42, WorldMatrix._43, WorldMatrix._44);
    _float cx = XMVectorGetX(vPos);
    _float cy = XMVectorGetY(vPos);
    _float hw = scaled.x * 0.5f;
    _float hh = scaled.y * 0.5f;

    _bool bInBounds = orthoX >= cx - hw && orthoX <= cx + hw
        && orthoY >= cy - hh && orthoY <= cy + hh;

    if (!bInBounds) {
        m_bHovered = false;
        m_bClicked = false;
        return;
    }

    // 알파 체크 (텍스처 있을 때만)
    if (pTex)
    {
        _float u = (orthoX - (cx - hw)) / (hw * 2.f);
        _float v = ((cy + hh) - orthoY) / (hh * 2.f);
        if (pTex->Sample_Alpha(iTexIdx, u, v) < 10)
        {
            m_bHovered = false;
            return;
        }
    }

    if (!m_bHovered) On_Hovered();
    m_bHovered = true;

    if (m_pGameInstance_Proxy->Mouse_Down(DIMB::LBUTTON)) {
        m_bClicked = true;
        On_Click();
    }
    if (m_bClicked && m_pGameInstance_Proxy->Mouse_Up(DIMB::LBUTTON)) {
        m_bClicked = false;
        On_Release();
    }
}

void CUIObject::Free()
{
    __super::Free();
}
