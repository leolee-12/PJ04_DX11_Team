#include "Kirby_OnOffPart.h"

#include "GameInstance.h"

CKirby_OnOffPart::CKirby_OnOffPart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject(pDevice, pContext)
{
}

CKirby_OnOffPart::CKirby_OnOffPart(const CKirby_OnOffPart& Prototype)
    : CPartObject(Prototype)
{
}

HRESULT CKirby_OnOffPart::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_OnOffPart::Initialize(void* pArg)
{
    if (pArg == nullptr)
    {
        MSG_BOX("pArg is nullptr: Kirby_OnOffPart");
        return E_FAIL;
    }

    KIRBY_ONONFFPART_DESC* pDesc = static_cast<KIRBY_ONONFFPART_DESC*>(pArg);

    m_pSocketBoneMatrix = pDesc->pSocketBoneMatrix;
    m_pHitFlashIntensity = pDesc->pHitFlashIntensity;
    m_pHitFlashColor = pDesc->pHitFlashColor;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
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

    _matrix matLocalWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

    if (m_pSocketBoneMatrix)                          
        matLocalWorld = matLocalWorld * XMLoadFloat4x4(m_pSocketBoneMatrix);

    Compute_CombinedWorldMatrix(matLocalWorld);

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
}

HRESULT CKirby_OnOffPart::Render_Shadow()
{
    if(FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;

    if(FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
        return E_FAIL;

    const size_t iNumMeshes = m_pModelCom->Get_NumMeshes();
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (m_pAnimatorCom)
        {
            if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
                return E_FAIL;
        }
        else
        {
            // Animator가 없으면 Shadow Render에 문제 생길 수 있음. 뼈 행렬
            MSG_BOX("m_pAnimatorCom is nullptr: Kirby_OnOffPart");
            return E_FAIL;
        }

        if (FAILED(m_pShaderCom->Begin(ETOUI(KIRBY_SHADER_PASS::ANIM_SHADOW))))
            return E_FAIL;

        m_pModelCom->Render(i);
    }

    return S_OK;
}

HRESULT CKirby_OnOffPart::Ready_PartComponents(const KIRBY_PART_COMPONENT_DESC가& tDesc)
{
    m_pShaderCom = Add_Component<CShader>(tDesc.tShaderDesc.iLevelID, tDesc.tShaderDesc.szProtoTag, TEXT("Com_Shader"));
    if (m_pShaderCom == nullptr)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, tDesc.szModelProtoTag, TEXT("Com_Model"));
    if (m_pModelCom == nullptr)  
        return E_FAIL;

    if (tDesc.bCreateAnimator)
    {
        CAnimator::ANIMATOR_DESC AnimDesc{};
        AnimDesc.pModel = m_pModelCom;
        AnimDesc.strDataFile = tDesc.szAnimEventFile ? tDesc.szAnimEventFile : TEXT("");

        m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
        if (m_pAnimatorCom == nullptr || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
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

    const _float fIntensity = m_pHitFlashIntensity ? *m_pHitFlashIntensity : 0.f;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fHitFlash", &fIntensity, sizeof(fIntensity))))
        return E_FAIL;

    const _float3 vColor = m_pHitFlashColor ? *m_pHitFlashColor : _float3(1.f, 1.f, 1.f);
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vHitFlashColor", &vColor, sizeof(vColor))))
        return E_FAIL;

    return S_OK;
}

void CKirby_OnOffPart::Free()
{
    __super::Free();
}