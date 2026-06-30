#include "Projectile_Bomb.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "Animator.h"
#include "Projectile_Movement.h"

CProjectile_Bomb::CProjectile_Bomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysicsProjectile{ pDevice, pContext }
{
}

CProjectile_Bomb::CProjectile_Bomb(const CProjectile_Bomb& Prototype)
	: CPhysicsProjectile ( Prototype )
{
}

HRESULT CProjectile_Bomb::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (m_pMovement)
        m_pMovement->Set_Physics(-45.f, 0.3f, 0.90f);

    return S_OK;
}

void CProjectile_Bomb::Update(_float fTimeDelta)
{
    if (!m_bAlive)
        return;

    if (m_bCarried)
    {
        //Update_Socket();
        Tick_Visual(fTimeDelta);
        return;
    }

    if (m_bFlying)
    {
        if (m_pMovement && m_pMovement->Tick(fTimeDelta))
            On_Bounce(++m_iBounceCount);
    }
    else
        Update_Terminal(fTimeDelta);

    Tick_Visual(fTimeDelta);

    if (m_pHitBox && m_pHitBox->Is_Enabled())
        m_pHitBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CProjectile_Bomb::Late_Update(_float fTimeDelta)
{
    if (m_bAlive && m_bCarried)
        Update_Socket();

    __super::Late_Update(fTimeDelta);
}

HRESULT CProjectile_Bomb::Render()
{
    if (!m_bAlive) 
        return S_OK;

    if (nullptr == m_pModelCom || nullptr == m_pShaderCom)
        return S_OK;

    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPass = { 0 };

        if (i == 1)
        {
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
                return E_FAIL;

            iPass = 1;
        }
        else
        {
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 1)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
                return E_FAIL;

            iPass = 0;
        }
        
 
        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(iPass)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

void CProjectile_Bomb::Despawn()
{
    // 사라질 때 발생하는 이펙트 여기에 작성

    Kill();
}

void CProjectile_Bomb::Bomb_Explode()
{
    if (!m_bAlive)
        return;

    On_Explode();

    Kill();
}

HRESULT CProjectile_Bomb::Ready_Visual()
{
    m_pShaderCom = Add_Component<CShader>(Shader_Bomb.iLevelID, Shader_Bomb.szProtoTag, TEXT("Com_Shader"));

    if (nullptr == m_pShaderCom)
        return E_FAIL;

    return S_OK;
}

void CProjectile_Bomb::Tick_Visual(_float fTimeDelta)
{
    Roll_ByMovement(fTimeDelta);

    if (m_pAnimatorCom)
        m_pAnimatorCom->Update(fTimeDelta);

    if (m_pAnimatorCom && m_pAnimatorCom->Is_Overlay_Finished(1))        // FuseBurning 애니메이션을 Base에 깔기
        Bomb_Explode();
}

void CProjectile_Bomb::On_Impact()
{
    Bomb_Explode();
}

void CProjectile_Bomb::Roll_ByMovement(_float fTimeDelta)
{
    if (m_bCarried || !m_bFlying || nullptr == m_pAnimatorCom || nullptr == m_pMovement)
        return;

    _float3 v = m_pMovement->Get_Velocity();
    _vector vFwd = XMVectorSet(v.x, 0.f, v.z, 0.f);     // 수평 진행방향
    _float fHoriz = XMVectorGetX(XMVector3Length(vFwd));

    static const _matrix matInv = XMMatrixInverse(nullptr, 
                                    XMMatrixRotationX(XMConvertToRadians(90.f)) *
                                    XMMatrixRotationY(XMConvertToRadians(180.f)));

    if (fHoriz > 0.01f)
    {
        vFwd = XMVector3Normalize(vFwd);
        _vector vAxisWorld = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vFwd); // 방향 뒤집히면 vUp, vFwd 순서 바꾸기
        _vector vAxisLocal = XMVector3TransformNormal(vAxisWorld, matInv);
        XMStoreFloat3(&m_vRollAxis, vAxisLocal);
    }

    m_fRollAngle += fHoriz * ROLL_DEG_PER_SPEED * fTimeDelta;
    m_fRollAngle = fmodf(m_fRollAngle, 360.f);

    m_pAnimatorCom->SetBoneRotation("RotL", m_fRollAngle, XMLoadFloat3(&m_vRollAxis));      // KirbyBomb의 회전 본 이름 동일 해야 함 
}

HRESULT	CProjectile_Bomb::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}


void CProjectile_Bomb::Free()
{
    __super::Free();
}
