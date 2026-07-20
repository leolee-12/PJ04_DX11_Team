#include "Projectile_MoonShot.h"
#include "GameInstance.h"

CProjectile_MoonShot::CProjectile_MoonShot(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CProjectile(pDevice, pContext) {
}
CProjectile_MoonShot::CProjectile_MoonShot(const CProjectile_MoonShot& Prototype)
    : CProjectile(Prototype) {
}

HRESULT CProjectile_MoonShot::Initialize(void* pArg)
{
    m_fHitRadius = 1.2f;
    m_fHitHeight = 2.f;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_fSpeed = 22.f;
    m_fLifeTime = 3.f;
    m_fDamage = 15.f;
    m_fKnockback = 10.f;
    return S_OK;
}

HRESULT CProjectile_MoonShot::Ready_Visual()
{
    return S_OK;
}

CProjectile_MoonShot* CProjectile_MoonShot::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CProjectile_MoonShot* pInstance = new CProjectile_MoonShot(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CProjectile_MoonShot");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CProjectile_MoonShot::Clone(void* pArg)
{
    CProjectile_MoonShot* pInstance = new CProjectile_MoonShot(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CProjectile_MoonShot");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CProjectile_MoonShot::Free() { __super::Free(); }