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
}

void CCharacter::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CCharacter::Render()
{
    return S_OK;
}

void CCharacter::On_Hit(_fvector vAttackerPos, _float fDamage)
{
    if (!Is_Active()) return;
    if (Block_Hit(vAttackerPos)) return;     // 무적/가드면 무효

    m_fCurHP -= fDamage;
    if (m_fCurHP <= 0.f) { m_fCurHP = 0.f; On_Death(vAttackerPos); return; }

    On_Damaged(vAttackerPos, fDamage);
}

void CCharacter::Free()
{
    __super::Free();
}