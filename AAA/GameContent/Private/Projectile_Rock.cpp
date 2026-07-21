#include "Projectile_Rock.h"
#include "GameContent_const.h"

CProjectile_Rock::CProjectile_Rock(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
: CProjectile{ pDevice, pContext }
{
}

CProjectile_Rock::CProjectile_Rock(const CProjectile_Rock & Prototype)
    : CProjectile(Prototype)
{
}

HRESULT CProjectile_Rock::Initialize(void* pArg)
{
    m_fHitRadius = 1.2f;
    m_fHitHeight = 0.4f;
    m_fDamage = 12.f;
    m_fKnockback = 8.f;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_pAnimatorCom->Play("Wait", true, false, 0.0f, 1.f);

    return S_OK;
}

HRESULT CProjectile_Rock::Ready_Visual()
{
    m_pShaderCom = Add_Component<CShader>(
        Shader_AnimMesh_PBR.iLevelID, Shader_AnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    const wchar_t* szModelTag = (rand() & 1) ? MODEL_PROTO_TAG_A : MODEL_PROTO_TAG_B;
    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, szModelTag, TEXT("Com_Model"));
    if (nullptr == m_pModelCom)
        return E_FAIL;

    CAnimator::ANIMATOR_DESC AnimDesc{};
    AnimDesc.pModel = m_pModelCom;
    AnimDesc.strDataFile = TEXT("");

    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
    if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CProjectile_Rock::Ready_HitBox()
{
    CCollider::COLLIDER_DESC d{};
    d.pOwner = this;
    d.vCenter = m_vCenterOffset;    // 기본 {0,0,0}, 필요시 Initialize에서 조절
    d.vCenter.y = 0.3f;
    d.fRadius = m_fHitRadius;

    m_pHitBox = Add_Component<CCollider>(
        Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag, TEXT("Com_HitBox"), &d);
    if (nullptr == m_pHitBox)
        return E_FAIL;

    m_pHitBox->Set_OnEnter([this](CCollider* pOther) {
        if (m_eState != STATE::BREAKING) return;
        if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup()) return;

        if (auto* pVictim = dynamic_cast<IDamageable*>(pOther->Get_Owner()))
        {
            ATTACK_INFO atk{};
            atk.fDamage = m_fDamage;
            atk.fKnockback = m_fKnockback;
            atk.pAttacker = this;
            XMStoreFloat3(&atk.vAttackerPos, m_pTransformCom->Get_State(Engine::STATE::POSITION));
            pVictim->Damaged(atk);
        }
        });

    m_pHitBox->Set_Enabled(false);
    m_pGameInstance_Proxy->Register_Collider(m_pHitBox, ETOUI(COLLISION_LAYER::MONSTER_PROJECTILE));
    return S_OK;
}

void CProjectile_Rock::On_Activated()   // 풀에서 재사용될 때 리셋
{
    m_eState = STATE::FALLING;
    m_fHitTimer = 0.f;
    if (m_pHitBox) m_pHitBox->Set_Enabled(false);
}

void CProjectile_Rock::Drop(const _float3& vTile, _float fSpawnHeight)
{
    m_fTargetY = vTile.y;
    m_pTransformCom->Set_State(Engine::STATE::POSITION,
        XMVectorSet(vTile.x, vTile.y + fSpawnHeight, vTile.z, 1.f));

    m_eState = STATE::FALLING;
    m_fHitTimer = 0.f;
    m_bAlive = true;                       // 베이스 Update/Late_Update 활성
    if (m_pHitBox) m_pHitBox->Set_Enabled(false);
    // TODO: 낙하 애님 있으면 m_pAnimatorCom->Play("Fall", ...)
}

void CProjectile_Rock::Update(_float fTimeDelta)
{
    if (!m_bAlive) return;

    if (m_eState == STATE::FALLING)
    {
        _vector p = m_pTransformCom->Get_State(Engine::STATE::POSITION);
        _float  y = XMVectorGetY(p) - FALL_SPEED * fTimeDelta;

        if (y <= m_fTargetY)   // 착지
        {
            m_pTransformCom->Set_State(Engine::STATE::POSITION, XMVectorSetY(p, m_fTargetY));
            m_eState = STATE::BREAKING;
            m_fHitTimer = HIT_WINDOW;
            m_pAnimatorCom->Play(ANIM_BREAK, false, true, 0.1f, 1.f);
            if (m_pHitBox) m_pHitBox->Set_Enabled(true);     // 착지 순간 타격
            // TODO: 착지 이펙트(RockBurst) Spawn
        }
        else
            m_pTransformCom->Set_State(Engine::STATE::POSITION, XMVectorSetY(p, y));
    }
    else // BREAKING
    {
        if (m_fHitTimer > 0.f)
        {
            m_fHitTimer -= fTimeDelta;
            if (m_fHitTimer <= 0.f && m_pHitBox)
                m_pHitBox->Set_Enabled(false);
        }
        if (m_pAnimatorCom->Is_Finished())
        {
            Kill();     // 풀 반환 (베이스가 처리)
            return;
        }
    }

    m_pAnimatorCom->Update(fTimeDelta);

    if (m_pHitBox && m_pHitBox->Is_Enabled())
        m_pHitBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

HRESULT CProjectile_Rock::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
        return E_FAIL;
    return S_OK;
}

HRESULT CProjectile_Rock::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(1)))     
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }
    return S_OK;
}

CProjectile_Rock* CProjectile_Rock::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CProjectile_Rock* pInstance = new CProjectile_Rock(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CProjectile_Rock");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CProjectile_Rock::Clone(void* pArg)
{
    CProjectile_Rock* pInstance = new CProjectile_Rock(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CProjectile_Rock");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CProjectile_Rock::Free()
{
    __super::Free();
}