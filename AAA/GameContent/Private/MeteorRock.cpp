#include "MeteorRock.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "Shader.h"
#include "Model.h"
#include "Collider.h"

CMeteorRock::CMeteorRock(
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext)
    : CProjectile{ pDevice, pContext }
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    m_fSpeed = 10.f;
    m_fLifeTime = 12.f;
    m_fDamage = 6.f;
    m_fHitRadius = 2.f;
    m_fHitHeight = 0.5f;
}

CMeteorRock::CMeteorRock(const CMeteorRock& Prototype)
    : CProjectile(Prototype)
    , m_bBreakOnLand{ Prototype.m_bBreakOnLand }
{
    m_fHitHeight = Prototype.m_fHitHeight;
}

HRESULT CMeteorRock::Ready_Visual()
{
    m_pShaderCom = Add_Component<CShader>(Shader_NonAnimMesh_PBR.iLevelID, Shader_NonAnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, Get_ModelProtoTag(), TEXT("Com_Model"));
    if (nullptr == m_pModelCom)
        return E_FAIL;

    return S_OK;
}

void CMeteorRock::Configure(_float fSpeed, _float fLifeSec, _bool bBreakOnLand, _float fHitRadius)
{
    m_fSpeed = fSpeed;
    m_fLifeTime = fLifeSec;
    m_bBreakOnLand = bBreakOnLand;

    if (fHitRadius > 0.f)
    {
        m_fHitRadius = fHitRadius;          // Sweep_Sphere
            if (m_pHitBox)
            {
                CCollider::COLLIDER_DESC desc{};
                desc.pOwner = this;
                desc.fRadius = m_fHitRadius;
                desc.fHeight = m_fHitHeight;
                desc.vCenter = m_vCenterOffset;
                desc.vRadians = m_vRadians;
                m_pHitBox->Reset_Bounding(desc);   
            }
    }
}

void CMeteorRock::On_Activated()
{
    m_eState = METEOR_STATE::FALLING;
    m_fLingerTimer = 0.f;
}

HRESULT CMeteorRock::Initialize_Prototype()
{
    if (FAILED(__super::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CMeteorRock::Initialize(void* pArg)
{

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;


    return S_OK;
}

void CMeteorRock::Update(_float fTimeDelta)
{
    if (!m_bAlive)
        return;

    if (METEOR_STATE::FALLING == m_eState)
    {
        _float3 vPrev{};
        XMStoreFloat3(&vPrev, m_pTransformCom->Get_State(STATE::POSITION));

        __super::Update(fTimeDelta);

        if (!m_bAlive)
            return;

        const _vector vAxis = XMVector3Normalize(m_pTransformCom->Get_State(STATE::UP));
        const _matrix matSpin = XMMatrixRotationAxis(vAxis, XMConvertToRadians(SPIN_DEG) * fTimeDelta);

        m_pTransformCom->Set_State(STATE::RIGHT, 
            XMVector3TransformNormal(m_pTransformCom->Get_State(STATE::RIGHT), matSpin));
        m_pTransformCom->Set_State(STATE::LOOK,
            XMVector3TransformNormal(m_pTransformCom->Get_State(STATE::LOOK),  matSpin));

        _float3 vNow{};
        XMStoreFloat3(&vNow, m_pTransformCom->Get_State(STATE::POSITION));

        const _vector vStart = XMLoadFloat3(&vPrev);
        const _vector vTarget = XMLoadFloat3(&m_vTargetPos);

        const _float fMoved = XMVectorGetX(XMVector3Length(XMLoadFloat3(&vNow) - vStart));
        const _float fToTarget = XMVectorGetX(XMVector3Length(vTarget - vStart));

        if (fMoved >= fToTarget)
        {
            m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(vTarget, 1.f));
            Enter_Landed();
        }
    }
    else
    {
        m_fLingerTimer -= fTimeDelta;
        if (m_fLingerTimer <= 0.f)
            Kill();
    }
}

HRESULT CMeteorRock::Ready_HitBox()
{
    CCollider::COLLIDER_DESC desc{};
    desc.pOwner = this;
    desc.fHeight = m_fHitHeight;
    desc.fRadius = m_fHitRadius;
    desc.vCenter = m_vCenterOffset;
    desc.vRadians = m_vRadians;

    m_pHitBox = Add_Component<CCollider>(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag, TEXT("Com_HitBox"), &desc);
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
    m_pGameInstance_Proxy->Register_Collider(m_pHitBox, ETOUI(COLLISION_LAYER::ENV_PROJECTILE));
    return S_OK;
}

void CMeteorRock::Enter_Landed()
{
    m_eState = METEOR_STATE::LANDED;
    m_vVelocity = { 0.f, 0.f, 0.f };

    if (m_pHitBox)
        m_pHitBox->Set_Enabled(false);

    m_fLingerTimer = LINGER_SEC;
    On_Land();
}

void CMeteorRock::On_Land()
{
}

HRESULT CMeteorRock::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

HRESULT CMeteorRock::Render()
{
    if (!m_bAlive)
        return S_OK;

    if (nullptr == m_pModelCom || nullptr == m_pShaderCom)
        return S_OK;

    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes =  static_cast<_uint>(m_pModelCom->Get_NumMeshes());

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;

        if (FAILED(m_pGameInstance_Proxy->Bind_DefaultTextureFromHub(m_pShaderCom, "g_MRATexture", DEFAULT_TEXTURE::MRA)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }
    return S_OK;
}

HRESULT CMeteorRock::Render_Shadow()
{
    if (!m_bAlive || nullptr == m_pModelCom || nullptr == m_pShaderCom)
        return S_OK;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrixPtr())))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pShaderCom->Begin(2)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }
    return S_OK;
}

void CMeteorRock::Free()
{
    __super::Free();
}