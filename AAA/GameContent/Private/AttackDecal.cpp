#include "AttackDecal.h"
#include "MeshLayer_Binder.h"

#include "CullingState.h"


namespace
{
    _float Clamp01(_float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }
    _float SmoothStep01(_float e0, _float e1, _float x)
    {
        _float t = Clamp01((x - e0) / max(e1 - e0, 1e-4f));
        return t * t * (3.f - 2.f * t);
    }
}

CAttackDecal::CAttackDecal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CAttackDecal::CAttackDecal(const CAttackDecal& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CAttackDecal::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    m_iMaterialID = WORLD_STATIC_ID;
    return S_OK;
}

HRESULT CAttackDecal::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

HRESULT CAttackDecal::Ready_Components()
{
    m_pShaderCom = Add_Component<CShader>(
        Shader_AttackDecal.iLevelID, Shader_AttackDecal.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(
        m_iPrototypeLevel, MODEL_PROTO_TAG, TEXT("Com_Model"));
    if (nullptr == m_pModelCom)
        return E_FAIL;

    m_pCullingState = Add_Component<CCullingState>(
        L"Com_CullingState", CCullingState::Create(m_pDevice, m_pContext));
    if (nullptr == m_pCullingState ||
        FAILED(m_pCullingState->Set_LocalBoundsFromModel(m_pModelCom)))
        return E_FAIL;

    return S_OK;
}

void CAttackDecal::Place(const _float3& vGroundPos, _float fRadius, _float fLifeTime)
{
    m_fAge = 0.f;
    m_fLifeTime = fLifeTime;
    m_bExpired = false;
    m_fDecalAlpha = 0.f;

    const _float fSize = fRadius * 2.f;
    _matrix matWorld =
        XMMatrixScaling(fSize, fSize, fSize) *
        XMMatrixTranslation(vGroundPos.x, vGroundPos.y, vGroundPos.z);

    m_pTransformCom->Set_WorldMatrix(matWorld);
}

void CAttackDecal::Update(_float fTimeDelta)
{
    if (m_pGameInstance_Proxy->Is_EditMode())
    {
        m_fDecalAlpha = 1.f;
        m_bExpired = false;
        return;
    }

    if (m_bExpired)
        return;

    m_fAge += fTimeDelta;
    if (m_fAge >= m_fLifeTime)
    {
        m_bExpired = true;
        Set_Active(false);
        return;
    }

    _float t = (m_fLifeTime > 0.f) ? (m_fAge / m_fLifeTime) : 1.f;
    m_fDecalAlpha = SmoothStep01(0.f, 0.15f, t);
}

void CAttackDecal::Late_Update(_float fTimeDelta)
{
    if (m_bExpired)
        return;

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::DECAL, this);
}

HRESULT CAttackDecal::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix"))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType)))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType)))) return E_FAIL;
    return S_OK;
}

HRESULT CAttackDecal::Render_Decal()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    // 월드 -> 데칼 로컬 복원용 역행렬들
    _float4x4 WorldInv{};
    XMStoreFloat4x4(&WorldInv, XMMatrixInverse(nullptr, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())));

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrixInverse", &WorldInv)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrixInverse", m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrixInverse", m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::PROJ))))
        return E_FAIL;

    const BoundingBox& lb = m_pCullingState->Get_LocalBounds();
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vDecalBoundsCenter", &lb.Center, sizeof(_float3)))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vDecalBoundsExtents", &lb.Extents, sizeof(_float3)))) return E_FAIL;

    // G버퍼 깊이 / 머티리얼ID (수신면 판정)
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShaderCom, "g_DepthTexture")))
        return E_FAIL;
    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_MaterialID"), m_pShaderCom, "g_MaterialIDTexture")))
        return E_FAIL;

    // 정적 바닥에만 투영(커비/동적물엔 안 묻음)
    const _int iMaskMode = 1;
    const _int iMaskID = WORLD_STATIC_ID;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_iDecalMaskMode", &iMaskMode, sizeof(_int))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_iDecalMaskID", &iMaskID, sizeof(_int))))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }
    return S_OK;
}

CAttackDecal* CAttackDecal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAttackDecal* pInstance = new CAttackDecal(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CAttackDecal");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CAttackDecal::Clone(void* pArg)
{
    CAttackDecal* pInstance = new CAttackDecal(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CAttackDecal");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CAttackDecal::Free()
{
    __super::Free();
}