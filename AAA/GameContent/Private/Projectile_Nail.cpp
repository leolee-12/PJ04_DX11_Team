#include "Projectile_Nail.h"
#include "GameInstance.h"
#include "GameContent_const.h"

CProjectile_Nail::CProjectile_Nail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CProjectile{ pDevice, pContext }
{
    m_fSpeed = 26.f; m_fLifeTime = 8.f;      // 8s = 안전망(지형 못 맞고 날아가면 소멸)
    m_fDamage = 4.f; m_fKnockback = 6.f;
    m_fHitRadius = 0.4f; m_fHitHeight = 0.4f;
    m_vCenterOffset = { 0.f, 0.5f, 0.f };
}
CProjectile_Nail::CProjectile_Nail(const CProjectile_Nail& Prototype)
    : CProjectile(Prototype) {
}

void CProjectile_Nail::Launch_At(const _float3& vTargetPos)
{
    _vector vPos = m_pTransformCom->Get_State(Engine::STATE::POSITION);   

    _vector vDir = XMLoadFloat3(&vTargetPos) - vPos;
    if (XMVectorGetX(XMVector3LengthSq(vDir)) < 1e-6f)
        vDir = m_pTransformCom->Get_State(Engine::STATE::LOOK);           
    vDir = XMVector3Normalize(vDir);

    _float3 vP, vD;
    XMStoreFloat3(&vP, vPos);
    XMStoreFloat3(&vD, vDir);

    Launch(vP, vD);
    _vector vAt = vPos - vDir;
    m_pTransformCom->LookAt(vAt);
    m_eState = STATE::FLYING;
}

HRESULT CProjectile_Nail::Ready_Visual()
{
    m_pShaderCom = Add_Component<CShader>(Shader_NonAnimMesh_PBR.iLevelID,
        Shader_NonAnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
    if (!m_pShaderCom) return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, MODEL_PROTO_TAG, TEXT("Com_Model"));
    if (!m_pModelCom) return E_FAIL;
    return S_OK;
}

void CProjectile_Nail::Update(_float fTimeDelta)
{
    if (!m_bAlive) return;

    if (m_bCarried)               
    {
        Update_Socket();
        return;
    }

    __super::Update(fTimeDelta);
    if (!m_bAlive) return;

    if (m_eState == STATE::FLYING)
    {
        _vector vVel = XMLoadFloat3(&m_vVelocity);
        if (XMVectorGetX(XMVector3LengthSq(vVel)) > 1e-6f)
        {
            _float3 vDir; XMStoreFloat3(&vDir, XMVector3Normalize(vVel));
            _float3 vPos; XMStoreFloat3(&vPos, m_pTransformCom->Get_State(Engine::STATE::POSITION));
            _float3 vN{};
            if (m_pGameInstance_Proxy->Sweep_Sphere(vPos, m_fHitRadius, vDir, m_fHitRadius + 0.3f, &vN))
                Enter_Stuck(vN);
        }
    }
    else // STUCK
    {
        m_fStuckTimer -= fTimeDelta;
        if (m_fStuckTimer <= 0.f)
            Kill();
    }
}

void CProjectile_Nail::Spin(_float dt)
{
    _vector vAxis = XMLoadFloat3(&m_vVelocity);
    if (XMVectorGetX(XMVector3LengthSq(vAxis)) < 1e-6f) return;
    vAxis = XMVector3Normalize(vAxis);
    m_pTransformCom->Rotate(XMQuaternionRotationAxis(vAxis, SPIN_SPEED * dt));  // 진행축 기준 회전
}

void CProjectile_Nail::Enter_Stuck(const _float3& vNormal)
{
    if (m_eState == STATE::STUCK) return;
    m_eState = STATE::STUCK;
    m_vVelocity = _float3(0.f, 0.f, 0.f);          // 정지
    if (m_pHitBox) m_pHitBox->Set_Enabled(false);  // 꽂힌 뒤엔 데미지 X
    m_fStuckTimer = STUCK_LINGER;

    UNREFERENCED_PARAMETER(vNormal);   // TODO(선택): 표면 노멀에 맞춰 정렬(꽂힌 각도)
}

HRESULT CProjectile_Nail::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix"))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType)))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType)))) return E_FAIL;
    return S_OK;
}

HRESULT CProjectile_Nail::Render()
{
    if (!m_bAlive) return S_OK;
    if (!m_pModelCom || !m_pShaderCom) return S_OK;
    if (FAILED(Bind_ShaderResources())) return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))   return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))   return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0))) return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(0))) return E_FAIL;
        if (FAILED(m_pModelCom->Render(i))) return E_FAIL;
    }
    return S_OK;
}

CProjectile_Nail* CProjectile_Nail::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CProjectile_Nail* p = new CProjectile_Nail(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CProjectile_Nail"); Safe_Release(p); }
    return p;
}
CGameObject* CProjectile_Nail::Clone(void* pArg)
{
    CProjectile_Nail* p = new CProjectile_Nail(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CProjectile_Nail"); Safe_Release(p); }
    return p;
}
void CProjectile_Nail::Free() { __super::Free(); }