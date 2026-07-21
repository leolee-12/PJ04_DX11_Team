#include "Projectile_Bomb.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "Effect_Loader.h"
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

    if (FAILED(Ready_AnimEvents()))
        return E_FAIL;

    if (m_pMovement)
        m_pMovement->Set_Physics(-45.f, 0.3f, 0.45f);

    return S_OK;
}

void CProjectile_Bomb::Update(_float fTimeDelta)
{
    if (!m_bAlive)
        return;

    if (m_bCarried)
    {
        Tick_Visual(fTimeDelta);
        Update_FuseSocket();
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

    Update_FuseSocket();
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
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_FuseMaskTexture", i, MTEX_TYPE::UNKNOWN, 0)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_FuseBurntTexture", i, MTEX_TYPE::UNKNOWN, 1)))
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

HRESULT CProjectile_Bomb::Render_Shadow()
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
        if (m_pAnimatorCom)
            m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
        if (FAILED(m_pShaderCom->Begin(2)))
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

_matrix CProjectile_Bomb::Get_PreRotInverse() const
{
    return XMMatrixInverse(nullptr,
        XMMatrixRotationX(XMConvertToRadians(90.f)) *
        XMMatrixRotationY(XMConvertToRadians(180.f)));
}

void CProjectile_Bomb::Ignite()
{
    Start_Fuse();

    if (m_bCarried)
    {
        Pause_Fuse();
        Update_Socket();
    }

    Spawn_FuseFx();
}

void CProjectile_Bomb::Start_Fuse(_float fSpeed)
{
    if (nullptr == m_pAnimatorCom)
        return;

    m_pAnimatorCom->Clear_Overlay(FUSE_LAYER);

    CAnimator::LAYER_PLAY_INFO LayerInfo{};
    LayerInfo.iSlot = FUSE_LAYER;
    LayerInfo.tAnim.strAniName = ANIM_FUSE;
    LayerInfo.tAnim.bLoop = false;
    LayerInfo.tAnim.bRestart = true;
    LayerInfo.tAnim.fSpeed = fSpeed;
    LayerInfo.Roots = { Get_FuseBoneName() };

    m_pAnimatorCom->Apply_Overlay(LayerInfo);
}

void CProjectile_Bomb::Pause_Fuse()
{
    if (m_pAnimatorCom)
        m_pAnimatorCom->Pause_Mask(FUSE_LAYER);
}

void CProjectile_Bomb::Resume_Fuse()
{
    if (m_pAnimatorCom)
        m_pAnimatorCom->Resume_Mask(FUSE_LAYER);
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
    {
        m_pAnimatorCom->Update(fTimeDelta);

        constexpr _int iSlot = 1;
        m_fBurnRatio = m_pAnimatorCom->Get_LayerProgress(iSlot);

        if (m_fBurnRatio >= 1.f)
            Bomb_Explode();
    }
}

void CProjectile_Bomb::On_Explode()
{
    Play_ExplodeFx();
}

void CProjectile_Bomb::On_Impact()
{
    Bomb_Explode();
}

void CProjectile_Bomb::Kill()
{
    Stop_FuseFx();
    __super::Kill();
}

void CProjectile_Bomb::Spawn_FuseFx()
{
    if (nullptr == m_pFuseBone && m_pModelCom)
        m_pFuseBone = m_pModelCom->Get_BoneMatrixPtr(Get_FuseBoneName());

    Update_FuseSocket();

    if (nullptr == m_pFuseFx)
    {
        CEffect_Loader::GetInstance()->Spawn(L"BombFuseEffect", Get_LevelIndex(),
            _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f),
            &m_matFuseWorld, &m_pFuseFx);
    }
}

void CProjectile_Bomb::Stop_FuseFx()
{
    if (nullptr == m_pFuseFx)
        return;

    m_pFuseFx->EffectContainer_StopAfterEmission();
    m_pFuseFx = nullptr;
}

void CProjectile_Bomb::Update_FuseSocket()
{
    if (!m_pFuseBone)	
        return;
    _matrix matBoneWorld =
        XMLoadFloat4x4(m_pFuseBone) *
        XMLoadFloat4x4(Get_Transform()->Get_WorldMatrixPtr());

    _matrix matSocket = XMMatrixIdentity();   // 회전=월드 정렬
    matSocket.r[3] = matBoneWorld.r[3];

    XMStoreFloat4x4(&m_matFuseWorld, matSocket);
}

void CProjectile_Bomb::Play_BodyAnim(const _char* szClip, _bool bLoop, _float fSpeed, _bool bRestart)
{
    if (nullptr == m_pAnimatorCom)
        return;

    CAnimator::ANI_PLAY_INFO Info{};
    Info.strAniName = szClip;
    Info.bLoop = bLoop;
    Info.fSpeed = fSpeed;
    Info.bRestart = bRestart;

    m_pAnimatorCom->Play(&Info);
}

void CProjectile_Bomb::Play_DangerGlow(_float fSpeed)
{
    Play_BodyAnim(ANIM_DANGER, true, fSpeed, false);
}

void CProjectile_Bomb::Apply_RollPose()
{
    if (m_pAnimatorCom)
        m_pAnimatorCom->SetBoneRotation(Get_RollBoneName(),
            m_fRollAngle, XMLoadFloat3(&m_vRollAxis));
}

void CProjectile_Bomb::Play_ExplodeFx()
{
    _float3 vPos{};
    XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));

    m_pGameInstance_Proxy->Play_SFX3D(Get_ExplodeSFXKey(), XMLoadFloat3(&vPos));

    CEffect_Loader::GetInstance()->Spawn(L"BombExplosion", Get_LevelIndex(),
                                        vPos, _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f),  nullptr);
}

void CProjectile_Bomb::Reset_BombVisual()
{
    m_fRollAngle = 0.f;
    m_vGlow = { 0.f, 0.f, 0.f };
}

void CProjectile_Bomb::Roll_ByMovement(_float fTimeDelta)
{
    if (m_bCarried || !m_bFlying ||
        nullptr == m_pAnimatorCom || nullptr == m_pMovement)
        return;

    _float3 v = m_pMovement->Get_Velocity();
    _vector vFwd = XMVectorSet(v.x, 0.f, v.z, 0.f);
    _float fHoriz = XMVectorGetX(XMVector3Length(vFwd));

    if (fHoriz > 0.01f)
    {
        vFwd = XMVector3Normalize(vFwd);
        _vector vAxisWorld = XMVector3Cross(
            XMVectorSet(0.f, 1.f, 0.f, 0.f), vFwd);
        _vector vAxisLocal = XMVector3TransformNormal(
            vAxisWorld, Get_PreRotInverse());
        XMStoreFloat3(&m_vRollAxis, vAxisLocal);
    }

    m_fRollAngle += fHoriz * ROLL_DEG_PER_SPEED * fTimeDelta;
    m_fRollAngle = fmodf(m_fRollAngle, 360.f);

    Apply_RollPose();
}

HRESULT	CProjectile_Bomb::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fBurnRatio", &m_fBurnRatio, sizeof(_float))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vGlow", &m_vGlow, sizeof(_float3))))
        return E_FAIL;

    return S_OK;
}


void CProjectile_Bomb::Free()
{
    __super::Free();
}
