#include "Projectile_Nail.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "Effect_Loader.h"

CProjectile_Nail::CProjectile_Nail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CProjectile{ pDevice, pContext }
{
    m_fSpeed = 100.f; m_fLifeTime = 8.f;      // 8s = 안전망(지형 못 맞고 날아가면 소멸)
    m_fDamage = 10.f; m_fKnockback = 6.f;
    m_fHitRadius = 1.f; m_fHitHeight = 0.4f;
    m_vCenterOffset = { 0.f, 0.f, 0.8f };
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
    Start_Meteo();
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

void CProjectile_Nail::Kill()
{
    Stop_Meteo();

    _float3 vPos{};
    XMStoreFloat3(&vPos, m_pTransformCom->Get_State(Engine::STATE::POSITION));
    CEffect_Loader::GetInstance()->Spawn(TEXT("Nail_Smoke"), m_iLevelIndex, vPos);

    __super::Kill();
}

void CProjectile_Nail::Update(_float fTimeDelta)
{
    if (!m_bAlive) return;

    if (m_bCarried)
    {
        Update_Socket();
        return;
    }

    _float3 vPrev;
    XMStoreFloat3(&vPrev, m_pTransformCom->Get_State(Engine::STATE::POSITION));

    __super::Update(fTimeDelta);
    if (!m_bAlive) return;

    if (m_eState == STATE::FLYING)
    {
        _float3 vNow;
        XMStoreFloat3(&vNow, m_pTransformCom->Get_State(Engine::STATE::POSITION));

        _vector vDelta = XMLoadFloat3(&vNow) - XMLoadFloat3(&vPrev);
        _float  fMoved = XMVectorGetX(XMVector3Length(vDelta));

        if (fMoved > 1e-6f)
        {
            _float3 vDir; XMStoreFloat3(&vDir, XMVector3Normalize(vDelta));
            _float3 vN{};
            _float  fHitDist = 0.f;
            if (m_pGameInstance_Proxy->Sweep_Sphere(vPrev, 0.4f, vDir, fMoved, &vN, &fHitDist))
            {
                const _float fStop = (std::max)(0.f, fHitDist - STUCK_PULLBACK);

                m_pTransformCom->Set_State(Engine::STATE::POSITION,
                    XMVectorSetW(XMLoadFloat3(&vPrev) + XMLoadFloat3(&vDir) * fStop, 1.f));

                Enter_Stuck(vN);
            }
        }
    }
    else // STUCK
    {
        m_fStuckTimer -= fTimeDelta;
        if (m_fStuckTimer <= 0.f)
            Kill();
    }
}

void CProjectile_Nail::Enter_Stuck(const _float3& vNormal)
{
    if (m_eState == STATE::STUCK) return;
    m_eState = STATE::STUCK;
    m_vVelocity = _float3(0.f, 0.f, 0.f);          // 정지
    if (m_pHitBox) m_pHitBox->Set_Enabled(false);  // 꽂힌 뒤엔 데미지 X
    m_fStuckTimer = STUCK_LINGER;
    Stop_Meteo();

    UNREFERENCED_PARAMETER(vNormal);   // TODO(선택): 표면 노멀에 맞춰 정렬(꽂힌 각도)
}

HRESULT CProjectile_Nail::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix"))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType)))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType)))) return E_FAIL;
    return S_OK;
}

void CProjectile_Nail::Start_Meteo()
{
    if (m_pMeteo)           
        return;

    if (m_pMeteo)
        return;

    const _float4x4* pParent = m_pTransformCom->Get_WorldMatrixPtr();

    CEffect_Loader::GetInstance()->Spawn(L"Leopard_Meteo", Get_LevelIndex(),
        _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f),
        pParent, &m_pMeteo);

    CEffect_Loader::GetInstance()->Spawn(L"Nail_Trail", Get_LevelIndex(),
        _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f),
        pParent, &m_pTrail);
}

void CProjectile_Nail::Stop_Meteo()
{
    if (!m_pMeteo)
        return;

    m_pMeteo->EffectContainer_StopAfterEmission();
    m_pMeteo = nullptr;

    m_pTrail->EffectContainer_StopAfterEmission();
    m_pTrail = nullptr;
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