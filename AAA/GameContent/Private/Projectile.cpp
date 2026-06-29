#include "Projectile.h"
#include "GameInstance.h"
#include "GameContent_const.h"      
#include "Projectile_Manager.h"
#include "Collider.h"

CProjectile::CProjectile(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext } {
}
CProjectile::CProjectile(const CProjectile& Prototype)
    : CGameObject(Prototype)
    , m_fSpeed{ Prototype.m_fSpeed }, m_fLifeTime{ Prototype.m_fLifeTime }
    , m_fDamage{ Prototype.m_fDamage }, m_fKnockback{ Prototype.m_fKnockback }
    , m_fHitRadius{ Prototype.m_fHitRadius } {
}

HRESULT CProjectile::Initialize_Prototype() { return __super::Initialize_Prototype(); }

HRESULT CProjectile::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))        // desc 로 트랜스폼 생성(GameObject.cpp:43)
        return E_FAIL;

    if (FAILED(Ready_HitBox()))
        return E_FAIL;
    if (FAILED(Ready_Movement())) 
        return E_FAIL;
    if (FAILED(Ready_Visual()))     
        return E_FAIL;

    m_bAlive = false;                            
    return S_OK;
}

HRESULT CProjectile::Ready_HitBox()
{
    CCollider::COLLIDER_DESC desc{};
    desc.pOwner = this;
    desc.fRadius = m_fHitRadius;

    m_pHitBox = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
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

void CProjectile::Launch(const _float3& vPos, const _float3& vDir)
{
    const _bool bWasCarried = m_bCarried;
    Detach();                    
    m_bAlive = true; m_fAccLife = 0.f;
    On_Activated();

    if (bWasCarried)
    {
        _vector vRight = m_pTransformCom->Get_State(STATE::RIGHT);
        _vector vUp = m_pTransformCom->Get_State(STATE::UP);
        _vector vLook = m_pTransformCom->Get_State(STATE::LOOK);

        const _float fScaleX = XMVectorGetX(XMVector3Length(vRight));
        const _float fScaleY = XMVectorGetX(XMVector3Length(vUp));
        const _float fScaleZ = XMVectorGetX(XMVector3Length(vLook));

        _vector vFlatLook = XMVectorSetY(vLook, 0.f);
        if (XMVectorGetX(XMVector3LengthSq(vFlatLook)) < 1e-6f)
            vFlatLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);
        vFlatLook = XMVector3Normalize(vFlatLook);

        const _vector vNewUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
        const _vector vNewRight = XMVector3Normalize(XMVector3Cross(vNewUp, vFlatLook));

        m_pTransformCom->Set_State(STATE::RIGHT, vNewRight * fScaleX);
        m_pTransformCom->Set_State(STATE::UP, vNewUp * fScaleY);
        m_pTransformCom->Set_State(STATE::LOOK, vFlatLook * fScaleZ);
    }
    else
    {
        m_pTransformCom->Set_State(STATE::RIGHT, XMVectorSet(1.f, 0.f, 0.f, 0.f));
        m_pTransformCom->Set_State(STATE::UP, XMVectorSet(0.f, 1.f, 0.f, 0.f));
        m_pTransformCom->Set_State(STATE::LOOK, XMVectorSet(0.f, 0.f, 1.f, 0.f));
    }

    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&vPos), 1.f));

    _vector v = XMLoadFloat3(&vDir);
    if (XMVectorGetX(XMVector3LengthSq(v)) > 1e-6f) v = XMVector3Normalize(v);
    XMStoreFloat3(&m_vVelocity, v * m_fSpeed);

    if (m_pHitBox)
    {
        m_pHitBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
        m_pHitBox->Set_Enabled(true);
    }
}

void CProjectile::Update(_float fTimeDelta)
{
    if (!m_bAlive) return;                        // 휴면 early-out (m_bIsPlay 대응)

    m_fAccLife += fTimeDelta;
    if (m_fAccLife >= m_fLifeTime) 
    {
        Kill(); 
        return; 
    }   // 수명 만료

    _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
    vPos += XMLoadFloat3(&m_vVelocity) * fTimeDelta;
    m_pTransformCom->Set_State(STATE::POSITION, vPos);

    if (m_pHitBox && m_pHitBox->Is_Enabled())
        m_pHitBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CProjectile::Late_Update(_float fTimeDelta)
{
    UNREFERENCED_PARAMETER(fTimeDelta);
    if (!m_bAlive) return;

#ifdef _DEBUG
    if (m_pHitBox && m_pHitBox->Is_Enabled())
        m_pGameInstance_Proxy->Add_DebugComponent(m_pHitBox);
#endif

    // 불투명 메시 기본 등록. concrete 가 모델 들고 Render() 처리.
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

void CProjectile::Attach_To_Socket(const _float4x4* pBone, const _float4x4* pOwnerWorld, _fmatrix matOffset)
{
    m_pSocketBone = pBone; m_pSocketOwnerWorld = pOwnerWorld;
    XMStoreFloat4x4(&m_SocketOffset, matOffset);
    m_bCarried = true; m_bAlive = true;
    if (m_pHitBox) m_pHitBox->Set_Enabled(false);   
    On_Activated();
}

void CProjectile::Kill()
{
    if (!m_bAlive) return;                        // 중복 회수 방지(이펙트와 동일)
    m_bAlive = false;
    if (m_pHitBox) m_pHitBox->Set_Enabled(false); // lifecycle-no-destroy 정리

    if (m_pPool) m_pPool->Return(m_iPoolLevel, m_strPoolKey, this); // 자기 반납
}

void CProjectile::Free() { __super::Free(); }