#include "UI_AbilityDiscard.h"
#include "GameContent_Events.h"
#include "UI_GaugeFill.h"
#include "UI_Text.h"

CUI_AbilityDiscard::CUI_AbilityDiscard(ID3D11Device* d, ID3D11DeviceContext* c)
    : CUI_GenericContainer{ d, c }
{
    m_fWorldYOffset = 0.f;   
    m_fScreenYOffset = 60.f;  
    m_fFadeOutDur = 0.5f;
}
CUI_AbilityDiscard::CUI_AbilityDiscard(const CUI_AbilityDiscard& p)
    : CUI_GenericContainer(p)
    , m_fWorldYOffset(p.m_fWorldYOffset)
    , m_fScreenYOffset(p.m_fScreenYOffset)
    , m_fFadeOutDur(p.m_fFadeOutDur)
{
}

HRESULT CUI_AbilityDiscard::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;
    Set_Active(false);   // 능력 있을 때만 Show()
    return S_OK;
}

void CUI_AbilityDiscard::On_Deserialized()
{
    __super::On_Deserialized();

    m_pGauge = nullptr;
    if (auto it = m_UIPartObjects.find(L"Gauge"); it != m_UIPartObjects.end())
        m_pGauge = dynamic_cast<CUI_GaugeFill*>(it->second);

    m_pText = nullptr;
    if (auto it = m_UIPartObjects.find(L"Text"); it != m_UIPartObjects.end())
        m_pText = dynamic_cast<CUI_Text*>(it->second);

    if (m_pGameInstance_Proxy && !m_pGameInstance_Proxy->Is_EditMode())
        Set_Active(false);
}

HRESULT CUI_AbilityDiscard::Ready_Events()
{
    Subscribe_Event(EventTag::AbilityDiscardUI_Bind, [this](void* p) {
        auto* d = static_cast<ABILITY_DISCARD_BIND_DESC*>(p);
        if (!d || !d->pCoolTime)
            return;
        m_pCoolTime = d->pCoolTime;
        m_fMaxCoolTime = (d->fMaxCoolTime > 0.f) ? d->fMaxCoolTime : 1.f;
        m_pTarget = nullptr;

        if (m_pText)
            m_pText->Set_Text(d->bIsAbility ? L"능력 버리기" : L"뱉기");

        m_bFadingOut = false;
        if (auto* pAnim = Get_UIAnimatorCom())
            pAnim->Stop_AllFades(true);

        Set_Active(true);
        });
    return S_OK;
}

_bool CUI_AbilityDiscard::Project_TargetToUI(_float2* pOutUI)
{
    if (!m_pTarget)
        return false;

    // 앵커 = 커비 월드좌표 + 월드 Y 오프셋
    _vector vWorld = m_pTarget->Get_Transform()->Get_State(STATE::POSITION);
    vWorld = XMVectorAdd(vWorld, XMVectorSet(0.f, m_fWorldYOffset, 0.f, 0.f));
    vWorld = XMVectorSetW(vWorld, 1.f);

    // 월드 -> 클립 (원근 view*proj)  ※ LensFlare 투영과 동일
    const _matrix matView = XMLoadFloat4x4(m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC));
    const _matrix matProj = XMLoadFloat4x4(m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC));

    _vector vClip = XMVector4Transform(vWorld, matView * matProj);
    _float  fW = XMVectorGetW(vClip);
    if (fW <= 1e-4f)
        return false;   // 카메라 뒤 -> 숨김

    _float fInvW = 1.f / fW;
    _float ndcX = XMVectorGetX(vClip) * fInvW;   // -1..1
    _float ndcY = XMVectorGetY(vClip) * fInvW;   // -1..1 (+가 위)

    // NDC -> UI 직교좌표(중앙원점). ORTHO 프로젝션에서 half-extent 역산 -> 창 크기 하드코딩 불필요
    const _float4x4* pOrtho = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::ORTHO);
    _float fHalfW = (pOrtho && pOrtho->_11 != 0.f) ? (1.f / pOrtho->_11) : 800.f;
    _float fHalfH = (pOrtho && pOrtho->_22 != 0.f) ? (1.f / pOrtho->_22) : 450.f;

    pOutUI->x = ndcX * fHalfW;
    pOutUI->y = ndcY * fHalfH - m_fScreenYOffset;   // +y가 위 -> 아래로 내리려면 빼기
    return true;
}

void  CUI_AbilityDiscard::Begin_FadeOut()
{
    m_bFadingOut = true;
    if (auto* pAnim = Get_UIAnimatorCom())
    {
        UI_FADE_DESC d{};
        d.fFromAlpha = -1.f;          
        d.fToAlpha = 0.f;        
        d.fDuration = m_fFadeOutDur;
        d.fDelay = 0.f;
        d.bRestoreOnFinish = true;
        pAnim->Play_FadeAll(d);
    }
}


void CUI_AbilityDiscard::Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    _float fRatio = m_pCoolTime ? (1.f - (*m_pCoolTime) / m_fMaxCoolTime) : 0.f;
    fRatio = max(0.f, min(1.f, fRatio));

    if (fRatio <= 0.f)
    {
        if (!m_bFadingOut)
            Begin_FadeOut();
    }
    else
    {
        if (m_pGauge)
            m_pGauge->Set_FillRatio(fRatio);
    }

    if (!m_pTarget)
    {
        PLAYER_QUERY q{};
        m_pGameInstance_Proxy->Publish(EventTag::Query_Player, &q);
        m_pTarget = q.pPlayer;
    }

    _float2 vUI{};
    if (Project_TargetToUI(&vUI))
    {
        _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
        _float  z = XMVectorGetZ(vPos);
        m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(vUI.x, vUI.y, z, 1.f));
    }

    __super::Update(fTimeDelta);

    if (m_bFadingOut)
    {
        CUIAnimatorCom* pAnim = Get_UIAnimatorCom();
        if (!pAnim || !pAnim->Is_FadingAny())
        {
            m_bFadingOut = false;
            m_pCoolTime = nullptr;
            Set_Active(false);
        }
    }
}

CUI_AbilityDiscard* CUI_AbilityDiscard::Create(ID3D11Device* d, ID3D11DeviceContext* c)
{
    auto* p = new CUI_AbilityDiscard(d, c);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_AbilityDiscard"); Safe_Release(p); }
    return p;
}
CGameObject* CUI_AbilityDiscard::Clone(void* pArg)
{
    auto* p = new CUI_AbilityDiscard(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_AbilityDiscard"); Safe_Release(p); }
    return p;
}
void CUI_AbilityDiscard::Free() { __super::Free(); }