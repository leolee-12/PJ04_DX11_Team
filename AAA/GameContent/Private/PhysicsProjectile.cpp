#include "PhysicsProjectile.h"
#include "GameInstance.h"
#include "Controller.h"
#include "Collider.h"
#include "Projectile_Movement.h"

CPhysicsProjectile::CPhysicsProjectile(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CProjectile{ pDevice, pContext } {
}
CPhysicsProjectile::CPhysicsProjectile(const CPhysicsProjectile& Prototype)
    : CProjectile(Prototype) {
}

HRESULT CPhysicsProjectile::Ready_Movement()
{
    CController::CONTROLLER_DESC cd{};
    cd.pOwner = this; cd.fRadius = m_fHitRadius; cd.fHeight = 0.1f; cd.vFootPos = { 0.f, 0.f, 0.f };
    m_pController = Add_Component<CController>(TEXT("Com_Controller"), CController::Create(m_pDevice, m_pContext));
    if (!m_pController || FAILED(m_pController->Initialize(&cd))) return E_FAIL;
    m_pController->Set_Enabled(false);

    m_pMovement = Add_Component<CProjectile_Movement>(TEXT("Com_Movement"),
        CProjectile_Movement::Create(m_pDevice, m_pContext));
    if (!m_pMovement) return E_FAIL;
    m_pMovement->Set_Refs(m_pTransformCom, m_pController);
    m_pMovement->Set_Physics(-45.f, 0.5f, 0.7f);     // 자식이 Ready_Visual 에서 덮어쓰기 가능
    return S_OK;
}

void CPhysicsProjectile::Launch(const _float3& vPos, const _float3& vDir)
{
    __super::Launch(vPos, vDir);    // alive + transform pos + m_vVelocity = dir*speed + 히트박스 on
    m_bFlying = true; m_iBounceCount = 0;

    if (m_pController) { m_pController->Set_Enabled(true); m_pController->Set_FootPosition(XMLoadFloat3(&vPos)); }
    if (m_pMovement)   m_pMovement->Launch(XMLoadFloat3(&m_vVelocity));   // 베이스가 만든 초기속도 인계
    On_Launched();
}

void CPhysicsProjectile::Launch_Arc(const _float3& vStart, const _float3& vTarget, _float fDur, _float fHeight)
{
    Detach();
    m_bAlive = true; m_fAccLife = 0.f;
    On_Activated();

    m_pTransformCom->Rotation(XMVectorSet(1.f, 0.f, 0.f, 0.f), 0.f);
    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&vStart), 1.f));

    _vector vDir = XMLoadFloat3(&vTarget) - XMLoadFloat3(&vStart);
    vDir = XMVectorSetY(vDir, 0.f);
    if (XMVectorGetX(XMVector3LengthSq(vDir)) > 1e-6f) vDir = XMVector3Normalize(vDir);
    XMStoreFloat3(&m_vVelocity, vDir * m_fSpeed);

    if (m_pHitBox)
    {
        m_pHitBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
        m_pHitBox->Set_Enabled(true);
    }

    m_bFlying = true; m_iBounceCount = 0;
    if (m_pController) { m_pController->Set_Enabled(true); m_pController->Set_FootPosition(XMLoadFloat3(&vStart)); }
    if (m_pMovement)   m_pMovement->Begin_Arc(XMLoadFloat3(&vStart), XMLoadFloat3(&vTarget), fDur, fHeight);
    On_Launched();
}

void CPhysicsProjectile::Update(_float fTimeDelta)
{
    if (!m_bAlive) return;

    if (m_bCarried) { Update_Socket(); Tick_Visual(fTimeDelta); return; }   // 들고 있을 땐 물리 X, 손 따라감

    if (m_bFlying)
    {
        m_fAccLife += fTimeDelta;
        if (m_fAccLife >= m_fLifeTime) { Kill(); return; }
        if (m_pMovement && m_pMovement->Tick(fTimeDelta))
        {
            if (m_pMovement->Is_Arc()) On_Impact();
            else                       On_Bounce(++m_iBounceCount);
        }
    }
    else Update_Terminal(fTimeDelta);

    Tick_Visual(fTimeDelta);
    if (m_pHitBox && m_pHitBox->Is_Enabled())
        m_pHitBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CPhysicsProjectile::Kill()
{
    if (m_pController) m_pController->Set_Enabled(false);
    if (m_pMovement)   m_pMovement->Stop();
    __super::Kill();    // m_bAlive=false + 히트박스 off + 풀 반환
}