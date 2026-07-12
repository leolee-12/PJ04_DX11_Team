#include "UI_Dialogue.h"
#include "UI_Text.h"
#include "GameInstance.h"

namespace
{
    constexpr const _tchar* PART_CURSOR = TEXT("Cursor");
    constexpr const _tchar* PART_NAME = TEXT("Text_Name");
    constexpr const _tchar* PART_LINES[3] =
    { TEXT("Text_Top"), TEXT("Text_Middle"), TEXT("Text_Bottom") };
}

CUI_Dialogue::CUI_Dialogue(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIContainerObject(pDevice, pContext)
{
    m_fCharInterval = { 0.1f };
    m_fCursorBlink = { 0.4f };
}

CUI_Dialogue::CUI_Dialogue(const CUI_Dialogue& Prototype)
    : CUIContainerObject(Prototype)
    , m_fCharInterval(Prototype.m_fCharInterval)
    , m_fCursorBlink(Prototype.m_fCursorBlink)
{
}

HRESULT CUI_Dialogue::Initialize_Prototype()
{
    return __super::Initialize_Prototype();
}

HRESULT CUI_Dialogue::Initialize(void* pArg)
{
    return __super::Initialize(pArg);
}

void CUI_Dialogue::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Priority_Update(fTimeDelta);
}

void CUI_Dialogue::Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Update(fTimeDelta);

    switch (m_eBox)
    {
        case EBOX::TYPING:
            if (m_pGameInstance_Proxy->Mouse_Down(DIMB::LBUTTON))
            {
                Reveal_All();
                m_eBox = EBOX::WAIT_INPUT;
                Set_CursorActive(true);
                break;
            }
            Tick_Typing(fTimeDelta);
            break;

        case EBOX::WAIT_INPUT:
            m_fCursorTimer += fTimeDelta;
            if (m_fCursorTimer >= m_fCursorBlink)
            {
                m_fCursorTimer -= m_fCursorBlink;
                m_bCursorShown = !m_bCursorShown;

                auto it = m_UIPartObjects.find(PART_CURSOR);
                if (it != m_UIPartObjects.end())
                    it->second->Set_Active(m_bCursorShown);
            }

            if (m_pGameInstance_Proxy->Mouse_Down(DIMB::LBUTTON))
            {
                Set_CursorActive(false);
                m_eBox = EBOX::ADVANCED;
                m_pGameInstance_Proxy->Publish(EventTag::Dialogue_SayDone, nullptr);
            }
            break;

        default:
            break;
    }
}

void CUI_Dialogue::Late_Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Late_Update(fTimeDelta);
}

HRESULT CUI_Dialogue::Ready_Events()
{
    Subscribe_Event(EventTag::Dialogue_Say, [this](void* pData)
        {
            const auto* pDesc = static_cast<DIALOGUE_SAY_DESC*>(pData);
            if (pDesc)
                Begin_Say(pDesc->strSpeaker, pDesc->strLines);
        });

    Subscribe_Event(EventTag::Dialogue_Finished, [this](void*)
        {
            m_eBox = EBOX::HIDDEN;
            Set_Active(false);
        });

    return S_OK;
}

void CUI_Dialogue::Begin_Say(const _wstring& strSpeaker, const _wstring* pLines)
{
    Set_Active(true);
    Set_CursorActive(false);

    auto itName = m_UIPartObjects.find(PART_NAME);
    if (itName != m_UIPartObjects.end())
        static_cast<CUI_Text*>(itName->second)->Set_Text(strSpeaker);

    m_iTotalChars = 0;
    for (_uint i = 0; i < 3; ++i)
    {
        m_FullLines[i] = pLines[i];
        m_iTotalChars += m_FullLines[i].length();
        m_LineShown[i] = 0;
        Set_LineText(i, L"");           // 이전 대사 잔상 제거
    }

    m_iRevealed = 0;
    m_fCharTimer = 0.f;
    m_eBox = EBOX::TYPING;
}

void CUI_Dialogue::Tick_Typing(_float fTimeDelta)
{
    m_fCharTimer += fTimeDelta;

    const _float fInterval = max(0.001f, m_fCharInterval);
    while (m_fCharTimer >= fInterval && m_iRevealed < m_iTotalChars)
    {
        m_fCharTimer -= fInterval;
        ++m_iRevealed;
    }

    Apply_Reveal();

    if (m_iRevealed >= m_iTotalChars)
        m_eBox = EBOX::WAIT_INPUT;
}

void CUI_Dialogue::Apply_Reveal()
{
    // 노출 총량을 위에서부터 라인별로 배분. 글자 수가 바뀐 라인만 리베이크
    size_t iRemain = m_iRevealed;
    for (_uint i = 0; i < 3; ++i)
    {
        const size_t iShow = min(iRemain, m_FullLines[i].length());
        iRemain -= iShow;

        if (iShow != m_LineShown[i])
        {
            m_LineShown[i] = iShow;
            Set_LineText(i, m_FullLines[i].substr(0, iShow));
        }
    }
}

void CUI_Dialogue::Reveal_All()
{
    m_iRevealed = m_iTotalChars;
    Apply_Reveal();
}

void CUI_Dialogue::Set_LineText(_uint iLine, const _wstring& strText)
{
    auto it = m_UIPartObjects.find(PART_LINES[iLine]);
    if (it != m_UIPartObjects.end())
        static_cast<CUI_Text*>(it->second)->Set_Text(strText);
}

void CUI_Dialogue::Set_CursorActive(_bool bOn)
{
    m_bCursorShown = bOn;
    m_fCursorTimer = 0.f;

    auto it = m_UIPartObjects.find(PART_CURSOR);
    if (it != m_UIPartObjects.end())
        it->second->Set_Active(bOn);
}

CUI_Dialogue* CUI_Dialogue::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Dialogue* pInstance = new CUI_Dialogue(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_Dialogue");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUI_Dialogue::Clone(void* pArg)
{
    CUI_Dialogue* pInstance = new CUI_Dialogue(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_Dialogue");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUI_Dialogue::Free()
{
    __super::Free();
}