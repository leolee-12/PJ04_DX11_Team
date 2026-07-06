#include "Character.h"

#include "GameInstance.h"

#include "PartObject.h"

CCharacter::CCharacter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CContainerObject{ pDevice, pContext }
{
}

CCharacter::CCharacter(const CCharacter& Prototype)
    : CContainerObject(Prototype)
{
}

HRESULT CCharacter::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCharacter::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void CCharacter::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CCharacter::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    // 기본(몬스터) = 원샷 1회 페이드. 커비는 이 값을 자기 Update에서 덮어씀.
    if (m_fHitFlashTime > 0.f)
    {
        m_fHitFlashTime = max(0.f, m_fHitFlashTime - fTimeDelta);
        m_fHitFlashCur = m_fHitFlashTime / m_fHitFlashDuration; // 1 -> 0
    }
    else
        m_fHitFlashCur = 0.f;
}

void CCharacter::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CCharacter::Render()
{
    return S_OK;
}

void CCharacter::Damaged(const ATTACK_INFO& tInfo)
{
    if (!Is_Active()) 
        return;

    if (Block_Hit(tInfo)) 
        return;

    m_pGameInstance_Proxy->Play_SFX(
        TEXT("CharaBasic_DamageReact_Normal.wav"), 0.5f);

    m_fHitFlashTime = m_fHitFlashDuration;

    m_fCurHP -= tInfo.fDamage;
    if (m_fCurHP <= 0.f) 
    {
        m_fCurHP = 0.f;
        On_Death(tInfo);
        return;
    }

    On_Damaged(tInfo);
}


void CCharacter::Free()
{
    __super::Free();
}