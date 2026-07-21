#include "Projectile_MoonShot.h"
#include "GameContent_const.h"

CProjectile_MoonShot::CProjectile_MoonShot(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CProjectile(pDevice, pContext) {
}
CProjectile_MoonShot::CProjectile_MoonShot(const CProjectile_MoonShot& Prototype)
    : CProjectile(Prototype) {
}

HRESULT CProjectile_MoonShot::Initialize(void* pArg)
{
    m_fHitRadius = 1.5f;
    m_fHitHeight = 0.45f;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_fSpeed = 22.f;
    m_fLifeTime = 3.f;
    m_fDamage = 15.f;
    m_fKnockback = 10.f;
    return S_OK;
}

void CProjectile_MoonShot::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
    if (!m_bAlive) return;

    m_fCurRadius += WAVE_SPEED * fTimeDelta;
    if (m_fCurRadius >= WAVE_MAX_R)
    {
        Kill();
        return;
    }

    CCollider::COLLIDER_DESC d{};
    d.pOwner = this;
    d.vCenter = { 0.f, MOON_H_OFFSET, 0.f };
    d.fRadius = m_fCurRadius;
    d.fHeight = m_fHitHeight;
    d.vSize.x = ARC_DEG;
    m_pHitBox->Reset_Bounding(d);
    m_pHitBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CProjectile_MoonShot::Launch(const _float3& vPos, const _float3& vDir)
{
    __super::Launch(vPos, vDir);

    _vector vD = XMLoadFloat3(&vDir);
    if (XMVectorGetX(XMVector3LengthSq(vD)) > 1e-6f)
    {
        _vector vP = m_pTransformCom->Get_State(STATE::POSITION);
        m_pTransformCom->LookAt(vP + XMVector3Normalize(vD));
    }

    if (m_pHitBox)
        m_pHitBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

HRESULT CProjectile_MoonShot::Ready_Visual()
{
    return S_OK;
}

HRESULT CProjectile_MoonShot::Ready_HitBox()
{
    CCollider::COLLIDER_DESC desc{};
    desc.pOwner = this;
    desc.vCenter = m_vCenterOffset;
    desc.fRadius = WAVE_START_R;
    desc.fHeight = m_fHitHeight;    
    desc.vSize.x = ARC_DEG;
    desc.vCenter = { 0.f, MOON_H_OFFSET, 0.f };

    m_pHitBox = Add_Component<CCollider>(Collider_Torus.iLevelID, Collider_Torus.szProtoTag,
        TEXT("Com_HitBox"), &desc);
    if (nullptr == m_pHitBox)
        return E_FAIL;

    m_pHitBox->Set_OnEnter([this](CCollider* pOther) {
        if (!m_bAlive) return;
        if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup()) return;
        if (auto* pVictim = dynamic_cast<IDamageable*>(pOther->Get_Owner()))
        {
            ATTACK_INFO atk{};
            atk.fDamage = m_fDamage; atk.fKnockback = m_fKnockback;
            XMStoreFloat3(&atk.vAttackerPos, m_pTransformCom->Get_State(STATE::POSITION));
            atk.pAttacker = this;
            pVictim->Damaged(atk);
        }
        On_Impact();
        });

    m_pHitBox->Set_Enabled(false);
    m_pGameInstance_Proxy->Register_Collider(m_pHitBox, ETOUI(COLLISION_LAYER::MONSTER_PROJECTILE));
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