#include "UI_FadeOut.h"
#include "UI_CurtainAnimBase.h"
#include "ICurtainPart.h"

CUI_FadeOut::CUI_FadeOut(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIContainerObject(pDevice, pContext) 
    , m_bStartHidden{ true }
    , m_bAutoStart{ false }
{
}
CUI_FadeOut::CUI_FadeOut(const CUI_FadeOut& Prototype)
    : CUIContainerObject(Prototype)
    , m_bStartHidden{ Prototype.m_bStartHidden }
    , m_bAutoStart{ Prototype.m_bAutoStart } {
}

HRESULT CUI_FadeOut::Initialize_Prototype() { return __super::Initialize_Prototype(); }

HRESULT CUI_FadeOut::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_bStarted = false; m_bDone = false;
    if (m_bStartHidden && !m_bAutoStart)
        Set_Active(false);                              // 신호 전까지 숨김
    return S_OK;
}

void CUI_FadeOut::Reset_Containor()
{
    if (!m_bCollected) Collect_Parts();
    for (auto* fp : m_Parts)
        fp->Reset();
}

void CUI_FadeOut::Collect_Parts()
{
    m_Parts.clear();
    std::vector<std::pair<_wstring, CUIPartObject*>> ordered;
    Get_UIPartObjectsInOrder(&ordered);
    for (auto& [tag, pPart] : ordered)
        if (auto* fp = dynamic_cast<ICurtainPart*>(pPart)) // 종류 무관
            m_Parts.push_back(fp);
    m_bCollected = true;
}

void CUI_FadeOut::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive) return;
    __super::Priority_Update(fTimeDelta);
}

void CUI_FadeOut::Play()
{
    Set_Active(true);
    if (!m_bCollected) Collect_Parts();
    m_bStarted = true; m_bDone = false;
    for (auto* fp : m_Parts)
        fp->Begin_Delayed();
}

void CUI_FadeOut::Stop()
{
    m_bStarted = false;
    for (auto* fp : m_Parts) fp->Anim_Stop();
}

void CUI_FadeOut::Update(_float fTimeDelta)
{
    if (!m_bActive) return;

    if (!m_bCollected && !m_UIPartObjects.empty())
        Collect_Parts();

    if (m_bAutoStart && !m_bStarted && m_bCollected)
        Play();

    __super::Update(fTimeDelta);            // 파트들 각자 딜레이/재생/끝 처리

    if (m_bStarted && !m_bDone)
    {
        _bool bAny = false, bAll = true;
        for (auto* fp : m_Parts)
        {
            if (fp->Is_Loop()) continue;
            bAny = true;
            if (!fp->Is_Finished()) { bAll = false; break; }
        }
        if (bAny && bAll) 
        { 
            m_bDone = true;
            /* Publish */
        }
    }

    if (m_bDone && m_bLoop)
    {
        Reset_Containor();
        Play();
        m_bDone = false;
    }
}

void CUI_FadeOut::Late_Update(_float fTimeDelta)
{
    if (!m_bActive) return;
    __super::Late_Update(fTimeDelta);
}

CUI_FadeOut* CUI_FadeOut::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_FadeOut* p = new CUI_FadeOut(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_FadeOut"); Safe_Release(p); }
    return p;
}
CGameObject* CUI_FadeOut::Clone(void* pArg)
{
    CUI_FadeOut* p = new CUI_FadeOut(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_FadeOut"); Safe_Release(p); }
    return p;
}
void CUI_FadeOut::Free() { __super::Free(); }