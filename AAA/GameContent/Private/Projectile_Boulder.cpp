#include "Projectile_Boulder.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "Shader.h"
#include "Model.h"
#include "Animator.h"
#include "Controller.h"
#include "Collider.h"
#include "Projectile_Movement.h"
#include "Effect_Loader.h"

CProjectile_Boulder::CProjectile_Boulder(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPhysicsProjectile{ pDevice, pContext }
{
    m_fSpeed = 16.f; m_fLifeTime = 6.f; m_fDamage = 4.f; m_fKnockback = 11.f; m_fHitRadius = 5.f;
}
CProjectile_Boulder::CProjectile_Boulder(const CProjectile_Boulder& Prototype)
    : CPhysicsProjectile(Prototype) {
}

HRESULT CProjectile_Boulder::Ready_Visual()
{
    m_pShaderCom = Add_Component<CShader>(Shader_AnimMesh_PBR.iLevelID,
        Shader_AnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom) return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, MODEL_PROTO_TAG, TEXT("Com_Model"));  // ANIM
    if (nullptr == m_pModelCom) return E_FAIL;

    CAnimator::ANIMATOR_DESC ad{};
    ad.pModel = m_pModelCom;
    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
    if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&ad))) return E_FAIL;

    if (m_pMovement) m_pMovement->Set_Physics(-45.f, RESTITUTION, HORIZ_DAMP);
    return S_OK;
}

void CProjectile_Boulder::On_Launched()
{
    m_eState = STATE::FLYING;

    _vector vAxis = XMVector3Cross(XMLoadFloat3(&m_vVelocity), XMVectorSet(0.f, 1.f, 0.f, 0.f));
    if (XMVectorGetX(XMVector3LengthSq(vAxis)) < 1e-6f)
        vAxis = XMVectorSet(1.f, 0.f, 0.f, 0.f);
    XMStoreFloat3(&m_vSpinAxis, XMVector3Normalize(vAxis));

    m_fSpinAngle = 0.f;
}

void CProjectile_Boulder::On_Bounce(_int iCount)
{
    CAMERA_SHAKE_DESC shake{ 0.5f, 0.f };
    m_pGameInstance_Proxy->Publish(EventTag::Camera_Shake, &shake);
    _float3 vPos{};
    XMStoreFloat3(&vPos, m_pTransformCom->Get_State(Engine::STATE::POSITION));
    CEffect_Loader::GetInstance()->Spawn(TEXT("RockBounce"), m_iLevelIndex, vPos);
    if (iCount >= 3)
        Enter_Break();
    else
    {
       m_pGameInstance_Proxy->Play_SFX(L"CharaBossGorilla_RockBound.wav");
    }
}

void CProjectile_Boulder::Enter_Break()
{
    m_eState = STATE::BREAKING;
    Stop_Flying();

    m_pTransformCom->Rotation(XMVectorSet(1.f, 0.f, 0.f, 0.f), 0.f);

    if (m_pController) m_pController->Set_Enabled(false);
    if (m_pMovement)   m_pMovement->Stop();
    if (m_pHitBox)     m_pHitBox->Set_Enabled(false);

    if (m_pAnimatorCom)
        m_pAnimatorCom->Play("Broken", false, true);

    m_pGameInstance_Proxy->Play_SFX(L"CharaBossGorilla_RockBreak.wav");
}

void CProjectile_Boulder::Update_Terminal(_float dt)
{
    UNREFERENCED_PARAMETER(dt);
    if (m_pAnimatorCom && m_pAnimatorCom->Is_Finished())
        Kill();                                       
}

void CProjectile_Boulder::Tick_Visual(_float dt)
{
    if (!m_pAnimatorCom) return;

    if (m_bFlying)
    {
        m_fSpinAngle += SPIN_SPEED_DEG * dt;
        if (m_fSpinAngle >= 360.f) m_fSpinAngle = fmodf(m_fSpinAngle, 360.f);
        m_pAnimatorCom->SetBoneRotation("LowM", m_fSpinAngle, XMLoadFloat3(&m_vSpinAxis));
    }

    m_pAnimatorCom->Update(dt);   
}

void CProjectile_Boulder::On_Activated()
{
    m_eState = STATE::FLYING;
    m_fSpinAngle = 0.f;             
    if (m_pAnimatorCom)
        m_pAnimatorCom->Play("Wait", true, true);
}

HRESULT CProjectile_Boulder::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix"))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType)))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType)))) return E_FAIL;
    return S_OK;
}

HRESULT CProjectile_Boulder::Render()
{
    if (!m_bAlive) return S_OK;
    if (nullptr == m_pModelCom || nullptr == m_pShaderCom) return S_OK;
    if (FAILED(Bind_ShaderResources())) return E_FAIL;

    const _bool bBreaking = (m_eState == STATE::BREAKING);
    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        const _bool bThisMesh = bBreaking ? (i != 0) : (i == 0);
        if (!bThisMesh) continue;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))   return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))   return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0))) return E_FAIL;
        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i))) return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(1))) return E_FAIL;
        if (FAILED(m_pModelCom->Render(i))) return E_FAIL;
    }
    return S_OK;
}

HRESULT CProjectile_Boulder::Render_Shadow()
{
    if (!m_bAlive) return S_OK;
    if (nullptr == m_pModelCom || nullptr == m_pShaderCom) 
        return S_OK;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrixPtr()))) 
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
        return E_FAIL;

    const _bool bBreaking = (m_eState == STATE::BREAKING);
    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        const _bool bThisMesh = bBreaking ? (i != 0) : (i == 0);
        if (!bThisMesh) continue;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i))) 
            return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(7))) 
            return E_FAIL;   
        if (FAILED(m_pModelCom->Render(i))) 
            return E_FAIL;
    }
    return S_OK;
}

CProjectile_Boulder* CProjectile_Boulder::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CProjectile_Boulder* p = new CProjectile_Boulder(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CProjectile_Boulder"); Safe_Release(p); }
    return p;
}
CGameObject* CProjectile_Boulder::Clone(void* pArg)
{
    CProjectile_Boulder* p = new CProjectile_Boulder(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CProjectile_Boulder"); Safe_Release(p); }
    return p;
}
void CProjectile_Boulder::Free() { __super::Free(); }