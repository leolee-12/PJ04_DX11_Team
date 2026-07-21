#include "MonsterPart.h"
#include "GameInstance.h"
#include "Animator.h"
#include "Ability_Model.h"

CMonsterPart::CMonsterPart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject{ pDevice, pContext } {
}

CMonsterPart::CMonsterPart(const CMonsterPart& Prototype)
    : CPartObject(Prototype) {
}

HRESULT CMonsterPart::Initialize(void* pArg)
{
    if (auto pDesc = static_cast<MONSTERPART_DESC*>(pArg))
    {
        m_pSocketBoneMatrix = pDesc->pSocketBoneMatrix;
        m_pHitFlash = pDesc->pHitFlash;        
        m_pHitFlashColor = pDesc->pHitFlashColor;  
        m_pCullState = pDesc->pCullState;
    }

    m_iShadowPassIdx = 7;

    return __super::Initialize(pArg);
}

void CMonsterPart::Update(_float fTimeDelta)
{
    if (!m_bActive || m_pGameInstance_Proxy->Is_EditMode())
        return;

    if (m_pAnimatorCom == nullptr)
        return;

    if (nullptr == m_pCullState)
        m_pAnimatorCom->Update(fTimeDelta);
    else if (m_pCullState->bAnimTick)
        m_pAnimatorCom->Update(m_pCullState->fAnimDt);
}

void CMonsterPart::Late_Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    _matrix LocalWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
    if (m_pSocketBoneMatrix)
    {
        _matrix boneMat = XMLoadFloat4x4(m_pSocketBoneMatrix);
        if (m_bIgnoreSocketScale)
        {
            boneMat.r[0] = XMVector3Normalize(boneMat.r[0]);
            boneMat.r[1] = XMVector3Normalize(boneMat.r[1]);
            boneMat.r[2] = XMVector3Normalize(boneMat.r[2]);
        }
        LocalWorld = LocalWorld * boneMat;
    }
    Compute_CombinedWorldMatrix(LocalWorld);

    if (m_pCullState && m_pCullState->bRenderCull)
        return;

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
}

HRESULT CMonsterPart::Render()
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

        if (m_pAnimatorCom)                            // 애니 파츠만 본 바인딩
        {
            if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
                return E_FAIL;
        }

        if (FAILED(m_pShaderCom->Begin(1)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CMonsterPart::Render_Shadow()
{
    if (!m_bActive || nullptr == m_pModelCom)
        return S_OK;

    if(FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;
    if(FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (m_pAnimatorCom)
            m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
        if (FAILED(m_pShaderCom->Begin(m_iShadowPassIdx)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }
    return S_OK;
}

const _float4x4* CMonsterPart::Get_BoneMatrixPtr(const _char* pBoneName) const
{
    if (nullptr == m_pModelCom || nullptr == pBoneName)
        return nullptr;

    return m_pModelCom->Get_BoneMatrixPtr(pBoneName);
}

HRESULT CMonsterPart::Ready_MeshPart(const PART_SETUP& t)
{
    m_pShaderCom = Add_Component<CShader>(t.tShader.iLevelID, t.tShader.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom) return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, t.szModelProtoTag, TEXT("Com_Model"));
    if (nullptr == m_pModelCom)  return E_FAIL;

    // 애니메이터 생성 여부는 오직 bAnimated 로만 결정 (경로와 무관)
    if (t.bAnimated)
    {
        CAnimator::ANIMATOR_DESC AnimDesc{};
        AnimDesc.pModel = m_pModelCom;
        AnimDesc.strDataFile = t.szAnimEventFile ? t.szAnimEventFile : TEXT(""); // 경로는 옵션

        m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
        if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CMonsterPart::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW,
        m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ,
        m_eProjType))))
        return E_FAIL;

    // 피격 플래시
    _float  fFlash = m_pHitFlash ? *m_pHitFlash : 0.f;
    _float3 vFlashCol = m_pHitFlashColor ? *m_pHitFlashColor : _float3(1.f, 1.f, 1.f);
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fHitFlash", &fFlash, sizeof(_float))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vHitFlashColor", &vFlashCol, sizeof(_float3))))
        return E_FAIL;

    const _float fDissolve = m_pCullState ? m_pCullState->fDissolve : 0.f;
    m_pShaderCom->Bind_RawValue("g_fDissolve", &fDissolve, sizeof(_float));

    return S_OK;
}

void CMonsterPart::Free()
{
    __super::Free();
}