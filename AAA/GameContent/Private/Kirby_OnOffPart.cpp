#include "Kirby_OnOffPart.h"

#include "GameInstance.h"

#include "GameContent_const.h"

#include "Animator.h"

CKirby_OnOffPart::CKirby_OnOffPart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject(pDevice, pContext)
{
}

CKirby_OnOffPart::CKirby_OnOffPart(const CKirby_OnOffPart& Prototype)
    : CPartObject(Prototype) {
}

HRESULT CKirby_OnOffPart::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_OnOffPart::Initialize(void* pArg)
{
    if (auto pDesc = static_cast<KIRBY_ONONFFPART_DESC*>(pArg))
    {
        m_pSocketBoneMatrix = pDesc->pSocketBoneMatrix;
        m_pHitFlash = pDesc->pHitFlash;
        m_pHitFlashColor = pDesc->pHitFlashColor;
    }

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_iShadowPass = 3;

    return S_OK;
}

void CKirby_OnOffPart::Priority_Update(_float fTimeDelta)
{
    if (!m_bOn)
        return;
}

void CKirby_OnOffPart::Update(_float fTimeDelta)
{
    if (!m_bOn || m_pGameInstance_Proxy->Is_EditMode())
        return;

    if (m_pAnimatorCom)
        m_pAnimatorCom->Update(fTimeDelta);
}

void CKirby_OnOffPart::Late_Update(_float fTimeDelta)
{
    if (!m_bOn)
        return;

    _matrix LocalWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
    if (m_pSocketBoneMatrix)                          
        LocalWorld = LocalWorld * XMLoadFloat4x4(m_pSocketBoneMatrix);

    Compute_CombinedWorldMatrix(LocalWorld);          
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
}

HRESULT CKirby_OnOffPart::Render()
{
    return S_OK;
}

HRESULT CKirby_OnOffPart::Render_Shadow()
{
    if (!m_bOn || m_iShadowPass < 0 || nullptr == m_pModelCom)
        return S_OK;

    m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix);
    m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW));
    m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ));

    const _uint iNumMeshes = (_uint)m_pModelCom->Get_NumMeshes();
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (m_pAnimatorCom)
            m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
        if (FAILED(m_pShaderCom->Begin(m_iShadowPass)))
            return E_FAIL;
        m_pModelCom->Render(i);
    }
    return S_OK;
}

HRESULT CKirby_OnOffPart::Ready_MeshPart(const PART_SETUP& t)
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

HRESULT CKirby_OnOffPart::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    // 피격 플래시
    _float  fFlash = m_pHitFlash ? *m_pHitFlash : 0.f;
    _float3 vFlashCol = m_pHitFlashColor ? *m_pHitFlashColor : _float3(1.f, 1.f, 1.f);
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fHitFlash", &fFlash, sizeof(_float))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vHitFlashColor", &vFlashCol, sizeof(_float3))))
        return E_FAIL;
    return S_OK;
}

void CKirby_OnOffPart::Free()
{
    __super::Free();
}