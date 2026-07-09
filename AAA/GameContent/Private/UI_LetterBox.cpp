#include "UI_LetterBox.h"
#include "GameInstance.h"
#include "GameContrnt_Events.h"

CUI_LetterBox::CUI_LetterBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_GenericContainer{ pDevice, pContext }
{
    m_fSlideDist = { 200.f };
    m_fSlideDur = { 0.35f };
}

CUI_LetterBox::CUI_LetterBox(const CUI_LetterBox& Prototype)
    : CUI_GenericContainer(Prototype)
    , m_fSlideDist(Prototype.m_fSlideDist)
    , m_fSlideDur(Prototype.m_fSlideDur)
{
}

HRESULT CUI_LetterBox::Initialize(void* pArg)
{
    return __super::Initialize(pArg);
}

void CUI_LetterBox::Update(_float fTimeDelta)
{
    _float fRawDelta = m_pGameInstance_Proxy->Get_RawTimeDelta(TEXT("Timer_60"));
    __super::Update(fRawDelta);
}

HRESULT CUI_LetterBox::Ready_Events()
{
    Subscribe_Event(EventTag::Letterbox_Begin, [this](void*) { Begin_LetterBox(); });
    Subscribe_Event(EventTag::Letterbox_End, [this](void*) { End_LetterBox(); });
    return S_OK;
}

void CUI_LetterBox::On_Deserialized()
{
    __super::On_Deserialized();

    XMStoreFloat3(&m_vBasePos, m_pTransformCom->Get_State(STATE::POSITION));
    m_bBaseCaptured = true;

    if (m_pGameInstance_Proxy && !m_pGameInstance_Proxy->Is_EditMode())
        Set_Active(false);
}

void CUI_LetterBox::Begin_LetterBox()
{
    if (m_bActive)
        return;

    if (!m_bBaseCaptured)
    {
        XMStoreFloat3(&m_vBasePos, m_pTransformCom->Get_State(STATE::POSITION));
        m_bBaseCaptured = true;
    }

    Set_Active(true);

    _float fDir = (m_vBasePos.y >= 0.f) ? 1.f : -1.f;
    Play_SlideIn(_float3{ fDir * m_fSlideDist, 0.f, 0.f }, m_fSlideDur);
}

void CUI_LetterBox::End_LetterBox()
{
    if (!m_bActive)
        return;

    Stop_Move();
    m_pTransformCom->Set_State(STATE::POSITION,
        XMVectorSet(m_vBasePos.x, m_vBasePos.y, m_vBasePos.z, 1.f));

    Set_Active(false);
}

CUI_LetterBox* CUI_LetterBox::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_LetterBox* pInstance = new CUI_LetterBox(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_LetterBox");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUI_LetterBox::Clone(void* pArg)
{
    CUI_LetterBox* pInstance = new CUI_LetterBox(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_LetterBox");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUI_LetterBox::Free()
{
    __super::Free();
}