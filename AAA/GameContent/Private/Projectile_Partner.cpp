#include "Projectile_Partner.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "Shader.h"
#include "Model.h"
#include "Animator.h"
#include "Controller.h"
#include "Collider.h"
#include "Projectile_Movement.h"
#include "Effect_Loader.h"

CProjectile_Partner::CProjectile_Partner(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPhysicsProjectile{ pDevice, pContext }
{
    m_fSpeed = 18.f; m_fLifeTime = 4.f; m_fDamage = 5.f; m_fKnockback = 12.f;
    m_fHitRadius = 1.2f; m_fHitHeight = 1.5f;
    m_vCenterOffset = { 0.f, 1.f, 0.f };
}
CProjectile_Partner::CProjectile_Partner(const CProjectile_Partner& Prototype)
    : CPhysicsProjectile(Prototype) {
}

HRESULT CProjectile_Partner::Ready_Visual()
{
    m_pShaderCom = Add_Component<CShader>(Shader_AnimMesh_PBR.iLevelID,
        Shader_AnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom) return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, MODEL_PROTO_TAG, TEXT("Com_Model"));
    if (nullptr == m_pModelCom) return E_FAIL;

    CAnimator::ANIMATOR_DESC ad{};
    ad.pModel = m_pModelCom;
    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
    if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&ad))) return E_FAIL;

    if (m_pMovement) m_pMovement->Set_Physics(0.f, 0.f, 1.f);
    return S_OK;
}

void CProjectile_Partner::On_Activated()
{
    if (m_bCarried)
    {
        m_eState = STATE::CARRIED;
        Update_Socket();
        if (m_pAnimatorCom)
        {
            m_pAnimatorCom->Play("AppearShort2", false, true, 0.f, 1.f);
            m_pAnimatorCom->Update(0.f);
        }
    }
}

void CProjectile_Partner::On_Launched()
{
    m_eState = STATE::FLYING;
    if (m_pAnimatorCom)
        m_pAnimatorCom->Play("TwinRolling", true, true);
}

void CProjectile_Partner::On_Impact()
{
    if (m_bCarried) return;
    Enter_Break();
}

void CProjectile_Partner::On_Bounce(_int iCount)
{
    UNREFERENCED_PARAMETER(iCount);
    Enter_Break();
}

void CProjectile_Partner::Enter_Break()
{
    if (m_eState == STATE::BREAKING) return;
    m_eState = STATE::BREAKING;
    Stop_Flying();

    m_pTransformCom->Rotation(XMVectorSet(1.f, 0.f, 0.f, 0.f), 0.f);

    if (m_pController) m_pController->Set_Enabled(false);
    if (m_pMovement)   m_pMovement->Stop();
    if (m_pHitBox)     m_pHitBox->Set_Enabled(false);

    if (m_pAnimatorCom)
        m_pAnimatorCom->Play("Break", false, true);
    // TODO: ÂøÅº ÀÌÆåÆ® / SFX (ÀÌÆåÆ® Å° È®Á¤µÇ¸é CEffect_Loader::Spawn)
}

void CProjectile_Partner::Update_Terminal(_float dt)
{
    UNREFERENCED_PARAMETER(dt);
    if (m_pAnimatorCom && m_pAnimatorCom->Is_Finished())
        Kill();
}

void CProjectile_Partner::Tick_Visual(_float dt)
{
    if (m_pAnimatorCom)
        m_pAnimatorCom->Update(dt);
}

void CProjectile_Partner::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
    if (!m_bAlive) return;

    if (m_bCarried && m_pHitBox && m_pHitBox->Is_Enabled())
        m_pHitBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    if (m_eState == STATE::FLYING && !m_bCarried)
    {
        _vector vVel = XMVectorSetY(XMLoadFloat3(&m_vVelocity), 0.f);
        if (XMVectorGetX(XMVector3LengthSq(vVel)) > 1e-6f)
        {
            _float3 vDir; XMStoreFloat3(&vDir, XMVector3Normalize(vVel));
            _float3 vPos; XMStoreFloat3(&vPos, m_pTransformCom->Get_State(Engine::STATE::POSITION));
            vPos.y += 1.f;

            _float3 vN{};
            if (m_pGameInstance_Proxy->Sweep_Sphere(vPos, 1.f, vDir, 1.5f, &vN)
                && fabsf(vN.y) < 0.5f)
                Enter_Break();
        }
    }
}

void CProjectile_Partner::Play_Anim(const _char* szClip, _bool bLoop)
{
    if (m_pAnimatorCom)
        m_pAnimatorCom->Play(szClip, bLoop, true, 0.1f, 1.f);
}

void CProjectile_Partner::Enable_SpinHitBox(_bool b)
{
    if (!m_pHitBox) return;
    m_pHitBox->Set_Enabled(b);
    if (b)
        m_pHitBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

HRESULT CProjectile_Partner::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix"))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType)))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType)))) return E_FAIL;
    return S_OK;
}

HRESULT CProjectile_Partner::Render()
{
    if (!m_bAlive) return S_OK;
    if (nullptr == m_pModelCom || nullptr == m_pShaderCom) return S_OK;
    if (FAILED(Bind_ShaderResources())) return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))   return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))    return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))     return E_FAIL;
        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i))) return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(1))) return E_FAIL;
        if (FAILED(m_pModelCom->Render(i))) return E_FAIL;
    }
    return S_OK;
}

CProjectile_Partner* CProjectile_Partner::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CProjectile_Partner* p = new CProjectile_Partner(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CProjectile_Partner"); Safe_Release(p); }
    return p;
}
CGameObject* CProjectile_Partner::Clone(void* pArg)
{
    CProjectile_Partner* p = new CProjectile_Partner(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CProjectile_Partner"); Safe_Release(p); }
    return p;
}
void CProjectile_Partner::Free() { __super::Free(); }