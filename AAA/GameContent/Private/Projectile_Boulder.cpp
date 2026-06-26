#include "Projectile_Boulder.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "Shader.h"
#include "Model.h"
#include "Animator.h"
#include "Controller.h"
#include "Collider.h"
#include "Projectile_Movement.h"

CProjectile_Boulder::CProjectile_Boulder(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPhysicsProjectile{ pDevice, pContext }
{
    m_fSpeed = 16.f; m_fLifeTime = 6.f; m_fDamage = 4.f; m_fKnockback = 11.f; m_fHitRadius = 0.9f;
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
    ad.strDataFile = TEXT("");
    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
    if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&ad))) return E_FAIL;

    if (m_pMovement) m_pMovement->Set_Physics(-45.f, RESTITUTION, HORIZ_DAMP);  // 부모 무브먼트 튜닝
    return S_OK;
}

void CProjectile_Boulder::On_Launched()
{
    m_eState = STATE::FLYING;
    if (m_pAnimatorCom)
        m_pAnimatorCom->Play("Wait", true, true);    // 항상 도는 베이스 클립(루프)
}

void CProjectile_Boulder::On_Bounce(_int iCount)
{
    if (iCount >= 2)                 // 바닥 2회 -> 파괴
        Enter_Break();
}

void CProjectile_Boulder::Enter_Break()
{
    m_eState = STATE::BREAKING;
    Stop_Flying();                                       // 부모: m_bFlying=false

    if (m_pController) m_pController->Set_Enabled(false);
    if (m_pMovement)   m_pMovement->Stop();
    if (m_pHitBox)     m_pHitBox->Set_Enabled(false);    // 데미지 종료

    if (m_pAnimatorCom)
        m_pAnimatorCom->Play("Break", false, true);      // 파괴 클립(파편 메쉬). 클립명은 모델에 맞춰
}

void CProjectile_Boulder::Update_Terminal(_float dt)
{
    UNREFERENCED_PARAMETER(dt);
    if (m_pAnimatorCom && m_pAnimatorCom->Is_Finished())
        Kill();                                          // 파괴 애님 끝 -> 풀 반환
}

void CProjectile_Boulder::Tick_Visual(_float dt)
{
    if (!m_pAnimatorCom) return;

    m_pAnimatorCom->Update(dt);                          // Wait/Break 클립 진행 (본 로컬 갱신)

    if (m_eState == STATE::FLYING && m_pModelCom)
    {
        // 비행 중: LowM 을 매 프레임 누적 회전 -> 돌이 구르는 것처럼.
        // (Wait 가 LowM 을 건드리지 않는다는 전제. 건드리면 m_fSpinAngle 누적 변수로 전환)
        m_pModelCom->RotateBone("LowM", SPIN_SPEED_DEG * dt, XMVectorSet(1.f, 0.f, 0.f, 0.f));
        m_pModelCom->Update_Combined();                  // 회전 반영해 합성행렬 재계산
    }
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

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))   return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))   return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0))) return E_FAIL;
        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i))) return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(1))) return E_FAIL;   // AnimMesh_PBR DMN 패스(번호 확인)
        if (FAILED(m_pModelCom->Render(i))) return E_FAIL;
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