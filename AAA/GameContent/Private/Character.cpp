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

    if (m_fHitStopTime > 0.f)
        m_fHitStopTime = max(0.f, m_fHitStopTime - fTimeDelta);

    if (m_fHitFlashTime > 0.f)
    {
        m_fHitFlashTime = max(0.f, m_fHitFlashTime - fTimeDelta);
        m_fHitFlashCur = m_fHitFlashTime / m_fHitFlashDuration; // 1 -> 0
    }
    else
        m_fHitFlashCur = 0.f;

    if (m_fShakeTime > 0.f)
    {
        m_fShakeTime = max(0.f, m_fShakeTime - fTimeDelta);
        m_fShakeElapsed += fTimeDelta;
    }

    if (m_fInvincibleTime > 0.f)
    {
        m_fInvincibleTime -= fTimeDelta;
        if (m_fInvincibleTime <= 0.f)
        {
            m_fInvincibleTime = 0.f;
            On_InvincibleEnd();             // 만료 시점에 1회 호출
        }
    }
}

void CCharacter::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CCharacter::Late_Update(_float fTimeDelta)
{
    // 셰이크: 파트들이 Late_Update에서 부모행렬과 합성하기 직전에만
      // 위치를 흔들고, 합성이 끝나면 원위치. 렌더에만 반영되고
      // 로직/콜라이더(Update 단계)는 영향 없음.
    const _bool isShaking = (m_fShakeTime > 0.f) && (m_pTransformCom != nullptr);
    _vector vOriginPos = {};

    if (isShaking)
    {
        vOriginPos = m_pTransformCom->Get_State(STATE::POSITION);

        _float fFalloff = m_fShakeTime / m_fShakeDuration;              // 1 -> 0 감쇠
        _float fX = sinf(m_fShakeElapsed * 120.f) * m_fShakeAmp * fFalloff;
        _float fZ = sinf(m_fShakeElapsed * 97.f + 1.3f) * m_fShakeAmp * fFalloff;

        // 월드 기준 X/Z축 지터 (앞뒤양옆)
        m_pTransformCom->Set_State(STATE::POSITION,
            vOriginPos + XMVectorSet(fX, 0.f, fZ, 0.f));
    }

    __super::Late_Update(fTimeDelta);

    if (isShaking)
        m_pTransformCom->Set_State(STATE::POSITION, vOriginPos);
}

HRESULT CCharacter::Render()
{
    return S_OK;
}

void CCharacter::Damaged(const ATTACK_INFO& tInfo)
{
    if (!Is_Active()) 
        return;

    if (Is_Invincible())
        return;

    if (Block_Hit(tInfo)) 
        return;

    m_pGameInstance_Proxy->Play_SFX(
        TEXT("CharaBasic_DamageReact_Normal.wav"), 0.5f);

    m_fHitFlashTime = m_fHitFlashDuration;

    m_fHitStopTime = m_fHitStopDuration;
    m_fShakeTime = m_fShakeDuration;
    m_fShakeElapsed = 0.f;
    Start_Invincibility();

    m_fCurHP -= tInfo.fDamage;
    if (m_fCurHP <= 0.f) 
    {
        m_fCurHP = 0.f;
        On_Death(tInfo);
        return;
    }

    On_Damaged(tInfo);
}

void CCharacter::Start_Invincibility(_float fDuration)
{
    if (!Is_Invincible())
        On_InvincibleBegin();

    m_fInvincibleTime = fDuration;
}


void CCharacter::Free()
{
    __super::Free();
}