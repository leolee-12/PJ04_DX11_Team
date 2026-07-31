#include "MeteorRock.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "Shader.h"
#include "Model.h"
#include "Collider.h"
#include "Effect_Container.h"

CMeteorRock::CMeteorRock(
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext)
    : CProjectile{ pDevice, pContext }
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    m_fSpeed = 10.f;
    m_fLifeTime = 12.f;
    m_fDamage = 6.f;
    m_fHitRadius = 3.f;
    m_fHitHeight = 0.1f;
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
    Release_AuraFx();

    m_eState = METEOR_STATE::FALLING;
    m_fLingerTimer = 0.f;

    m_FallSound.Stop();   // 풀 재사용 대비
    m_FallSound = m_pGameInstance_Proxy->Play_SFX3D_Loop(SND_FALLING, XMLoadFloat3(&m_vTargetPos), FALL_SOUND_VOLUME);
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

        if (m_FallSound.Is_Valid())
        {
            const _float4* pCam =  m_pGameInstance_Proxy->Get_CamPosition();
            if (nullptr != pCam)
            {
                const _float fSoundDist = XMVectorGetX(XMVector3Length(XMLoadFloat3(&vNow) - XMLoadFloat4(pCam)));

                _float fAtten = 1.f - (fSoundDist / SOUND_FALLOFF_RANGE);
                Helper::FloatClamp(fAtten, 0.f, 1.f);

                m_FallSound.Set_Volume(FALL_SOUND_VOLUME * fAtten);
            }
        }

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
        m_fVibrationElapsed += fTimeDelta;

        const _float fX = sinf(m_fVibrationElapsed * VIBRATION_FREQ_X) * VIBRATION_AMPLITUDE;
        const _float fZ = sinf(m_fVibrationElapsed * VIBRATION_FREQ_Z + 1.3f) * VIBRATION_AMPLITUDE;

        m_pTransformCom->Set_State(STATE::POSITION,
            XMLoadFloat3(&m_vTargetPos) + XMVectorSet(fX, 0.f, fZ, 0.f));

        m_fLingerTimer -= fTimeDelta;
        if (m_fLingerTimer <= 0.f)
        {
            m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat3(&m_vTargetPos));
            Spawn_ExplosionFx();
            Kill();
        }
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
        //On_Impact();
        });

    m_pHitBox->Set_Enabled(false);
    m_pGameInstance_Proxy->Register_Collider(m_pHitBox, ETOUI(COLLISION_LAYER::ENV_PROJECTILE));
    return S_OK;
}

void CMeteorRock::Kill()
{
    Release_AuraFx();
    m_FallSound.Stop();

    __super::Kill();
}

void CMeteorRock::Enter_Landed()
{
    m_eState = METEOR_STATE::LANDED;
    m_vVelocity = { 0.f, 0.f, 0.f };

    if (m_pHitBox)
        m_pHitBox->Set_Enabled(false);

    On_Land();
}

void CMeteorRock::Spawn_ExplosionFx()
{
    if (!m_bBreakOnLand)
        return;

    const _float3 vRockScale = m_pTransformCom->Get_Scaled();
    const _float fWorldRadius = m_fHitRadius * max(vRockScale.x, vRockScale.z);

    _vector vBackward = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    const _vector vDir = XMLoadFloat3(&m_vDir);
    if (XMVectorGetX(XMVector3LengthSq(vDir)) > 1e-6f)
        vBackward = -XMVector3Normalize(vDir);

    _float3 vPos{};
    XMStoreFloat3(&vPos,
        XMLoadFloat3(&m_vTargetPos) + vBackward * (fWorldRadius * 0.5f));

    CEffect_Container* pEffect = nullptr;
    if (SUCCEEDED(CEffect_Loader::GetInstance()->Spawn(
        TEXT("MeteorExplosion"), Get_LevelIndex(), vPos,
        _float3{}, _float3{}, nullptr, &pEffect)) && nullptr != pEffect)
    {
        pEffect->Get_Transform()->Set_Scale(vRockScale.x, vRockScale.y, vRockScale.z);
    }
}

void CMeteorRock::Spawn_AuraFx()
{
    Release_AuraFx();

    FX_HANDLE hAura{};

    const HRESULT hr = CEffect_Loader::GetInstance()->Spawn(
        FX_AURA, Get_LevelIndex(),
        _float3{}, _float3{}, _float3{},
        m_pTransformCom->Get_WorldMatrixPtr(),
        nullptr, &hAura);

    if (FAILED(hr) || nullptr == hAura.p)
        return;

    m_hAura = hAura;
}

void CMeteorRock::Release_AuraFx()
{
    if (nullptr == m_hAura.p)
        return;

    if (CEffect_Loader::GetInstance()->Is_Current(m_hAura))
        m_hAura.p->EffectContainer_Stop();

    m_hAura = {};
}

void CMeteorRock::On_Land()
{
    m_FallSound.Stop();

    if (m_bBreakOnLand)
        m_pGameInstance_Proxy->Play_SFX3D(Get_BreakSoundKey(), m_pTransformCom->Get_State(STATE::POSITION), BREAK_SOUND_VOLUME);

    m_fLingerTimer = LINGER_SEC;
    m_fVibrationElapsed = 0.f;

    const _float4* pCamPosition = m_pGameInstance_Proxy->Get_CamPosition();
    if (nullptr == pCamPosition)
        return;

    const _vector vCamPosition = XMLoadFloat4(pCamPosition);
    if (XMVector3IsNaN(vCamPosition) || XMVector3IsInfinite(vCamPosition))
        return;

    const _float fDistance = XMVectorGetX(XMVector3Length(
        XMLoadFloat3(&m_vTargetPos) - vCamPosition));

    _float fT = (fDistance - SHAKE_FULL_RADIUS) / (SHAKE_ZERO_RADIUS - SHAKE_FULL_RADIUS);
    Helper::FloatClamp(fT, 0.f, 1.f);

    const _float fFactor = 1.f - (fT * fT * (3.f - 2.f * fT));
    if (fFactor <= Helper::fEpsilon)
        return;

    CAMERA_SHAKE_DESC desc{};
    desc.fTrauma = SHAKE_BASE_TRAUMA * fFactor;
    m_pGameInstance_Proxy->Publish(EventTag::Camera_Shake, &desc);
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

void CMeteorRock::Launch(const _float3& vPos, const _float3& vDir)
{
    __super::Launch(vPos, vDir);
    m_vDir = vDir;

    const _vector vInput = XMLoadFloat3(&vDir);

    if (XMVectorGetX(XMVector3LengthSq(vInput)) > FX_DIRECTION_EPSILON_SQ)
    {
        const _vector vFlight = XMVector3Normalize(vInput);
        const _vector vUp = -vFlight;

        _vector vReference = XMVectorSet(0.f, 0.f, 1.f, 0.f);
        if (fabsf(XMVectorGetX(XMVector3Dot(vUp, vReference)))
            > FX_REFERENCE_DOT_LIMIT)
        {
            vReference = XMVectorSet(1.f, 0.f, 0.f, 0.f);
        }

        const _vector vRight = XMVector3Normalize(
            XMVector3Cross(vUp, vReference));
        const _vector vLook = XMVector3Normalize(
            XMVector3Cross(vRight, vUp));

        const _float3 vScale = m_pTransformCom->Get_Scaled();

        m_pTransformCom->Set_State(STATE::RIGHT,
            XMVectorSetW(vRight * vScale.x, 0.f));
        m_pTransformCom->Set_State(STATE::UP,
            XMVectorSetW(vUp * vScale.y, 0.f));
        m_pTransformCom->Set_State(STATE::LOOK,
            XMVectorSetW(vLook * vScale.z, 0.f));

        XMStoreFloat3(&m_vDir, vFlight);
    }

    Spawn_AuraFx();
}

void CMeteorRock::Free()
{
    m_hAura = {};

    if (m_FallSound.Is_Valid())
        m_FallSound.Stop();

    __super::Free();
}