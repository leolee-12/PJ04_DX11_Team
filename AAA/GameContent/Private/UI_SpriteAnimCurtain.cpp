#include "UI_SpriteAnimCurtain.h"

CUI_SpriteAnimCurtain::CUI_SpriteAnimCurtain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_SpriteAnim{ pDevice, pContext } 
    , m_fStartDelay{0.f}
    , m_bDisableOnFinish{true}
    , m_bShowWhileWaiting{false}
{
}
CUI_SpriteAnimCurtain::CUI_SpriteAnimCurtain(const CUI_SpriteAnimCurtain& Prototype)
    : CUI_SpriteAnim(Prototype)
    , m_fStartDelay{ Prototype.m_fStartDelay }
    , m_bDisableOnFinish{ Prototype.m_bDisableOnFinish } 
    , m_bShowWhileWaiting{ Prototype.m_bShowWhileWaiting }
{
}

void CUI_SpriteAnimCurtain::Update(_float fTimeDelta)
{
    if (m_bArmed)                                                   // 딜레이 대기: 프레임 정지 + 렌더 스킵
    {
        m_fDelayAcc += fTimeDelta;
        if (m_fDelayAcc >= m_fStartDelay)
        {
            m_bArmed = false;
            Play(Is_Loop());                                // 딜레이 끝 → 프레임 재생 시작
        }
    }
    __super::Update(fTimeDelta);                    // 재생 중이면 프레임 진행
}

void CUI_SpriteAnimCurtain::Late_Update(_float fTimeDelta)
{
    if (m_bArmed && !m_bShowWhileWaiting) return;
    if (Is_Finished() && m_bDisableOnFinish) return;
    __super::Late_Update(fTimeDelta);
}

CUI_SpriteAnimCurtain* CUI_SpriteAnimCurtain::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_SpriteAnimCurtain* p = new CUI_SpriteAnimCurtain(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_SpriteAnimCurtain"); Safe_Release(p); }
    return p;
}
CGameObject* CUI_SpriteAnimCurtain::Clone(void* pArg)
{
    CUI_SpriteAnimCurtain* p = new CUI_SpriteAnimCurtain(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_SpriteAnimCurtain"); Safe_Release(p); }
    return p;
}
void CUI_SpriteAnimCurtain::Free() { __super::Free(); }