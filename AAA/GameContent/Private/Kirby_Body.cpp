#include "Kirby_Body.h"

#include "GameInstance.h"

#include "GameContent_const.h"

#include "Animator.h"

CKirby_Body::CKirby_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject(pDevice, pContext)
{
}

CKirby_Body::CKirby_Body(const CKirby_Body& Prototype)
    : CPartObject(Prototype){
}

HRESULT CKirby_Body::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_Body::Initialize(void* pArg)
{
    KIRBY_BODY_DESC* pDesc = static_cast<KIRBY_BODY_DESC*>(pArg);

    pDesc->fSpeedPerSec = 1.f;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;
    
    m_pAnimatorCom->Play("Wait", true, true);

    return S_OK;
}

void CKirby_Body::Priority_Update(_float fTimeDelta)
{
}

void CKirby_Body::Update(_float fTimeDelta)
{
    if (m_pGameInstance_Proxy->Is_EditMode())
        return;

    m_pAnimatorCom->Update(fTimeDelta);
}

void CKirby_Body::Late_Update(_float fTimeDelta)
{
    __super::Compute_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CKirby_Body::Render()
{
    if (FAILED(Set_VisibleMeshes()))
        return E_FAIL;

    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = (_uint)m_pModelCom->Get_NumMeshes();

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (i >= m_VisibleMeshes.size() || m_VisibleMeshes[i] == false)
            continue;

        if (FAILED(m_pEyeTextureCom->Bind_ShaderResource(m_pShaderCom, "g_EyeTexture", ETOUI(m_eEye))))
            return E_FAIL;
        if (FAILED(m_pEyeMaskTextureCom->Bind_ShaderResource(m_pShaderCom, "g_EyeMaskTexture", ETOUI(m_eEye))))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_SkinTexture", i, MTEX_TYPE::UNKNOWN, 1)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MouthTexture", i, MTEX_TYPE::UNKNOWN, 2)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Bind_RawValue("g_vBodyColor", &m_vBodyColor, sizeof(_float4))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vFootColor", &m_vFootColor, sizeof(_float4))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vBlushColor", &m_vBlushColor, sizeof(_float4))))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }
    return S_OK;
}

const _float4x4* CKirby_Body::Get_BoneMatrixPtr(const _char* pBoneName) const
{
    return m_pModelCom->Get_BoneMatrixPtr(pBoneName);
}

HRESULT CKirby_Body::Ready_Components()
{
    /* For.Com_Shader */
    m_pShaderCom = Add_Component<CShader>(Shader_Kirby.iLevelID, Shader_Kirby.szProtoTag, TEXT("Com_Shader"));
    if (m_pShaderCom == nullptr)
        return E_FAIL;

    /* For.Com_Model */
    m_pModelCom = Add_Component<CModel>(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Kirby_Body"), TEXT("Com_Model"));
    if (m_pModelCom == nullptr)
        return E_FAIL;

    m_pEyeTextureCom = Add_Component<CTexture>(TEXT("Com_EyeTexture"),
        CTexture::Create(m_pDevice, m_pContext, L"../../Resources/YSE/Kirby/KirbyEye.%02d.png", ETOUI(KIRBY_EYE_STATE::END)));
    if (nullptr == m_pEyeTextureCom)
        return E_FAIL;

    m_pEyeMaskTextureCom = Add_Component<CTexture>(TEXT("Com_EyeMaskTexture"),
        CTexture::Create(m_pDevice, m_pContext, L"../../Resources/YSE/Kirby/KirbyEyeMask.%02d.png", ETOUI(KIRBY_EYE_STATE::END)));
    if (nullptr == m_pEyeMaskTextureCom)
        return E_FAIL;

    m_VisibleMeshes.resize(m_pModelCom->Get_NumMeshes(), true);

    /* For.Com_Animator */
    CAnimator::ANIMATOR_DESC AnimDesc{};
    AnimDesc.pModel = m_pModelCom;
    //AnimDesc.strDataFile = TEXT("../Bin/Resources/Test/Test/Marb1e/Marb1e_animevents.json");

    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));

    if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby_Body::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    _uint iID = KIRBY_SILHOUETTE_ID;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &iID, sizeof(_uint)))) // ¡ç Ãß°¡
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby_Body::Set_VisibleMeshes()
{
    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

    m_VisibleMeshes.assign(iNumMeshes, false);

    auto ShowMesh = [&](KIRBY_MESH eKirbyMesh)
        {
            _uint iIndex = ETOUI(eKirbyMesh);
            if (iIndex >= 0 && iIndex < iNumMeshes)
                m_VisibleMeshes[iIndex] = true;
        };

    switch (m_eBody)
    {
        case KIRBY_BODY_STATE::NORMAL:  ShowMesh(KIRBY_MESH::BODY);         break;
        case KIRBY_BODY_STATE::STUFFED: ShowMesh(KIRBY_MESH::BODY_BIG);     break;
        case KIRBY_BODY_STATE::INHALE:  ShowMesh(KIRBY_MESH::BODY_VACUUM);  break;
    }

    if (m_eBody == KIRBY_BODY_STATE::NORMAL)
    {
        switch (m_eMouth)
        {
            case KIRBY_MOUTH_STATE::IDLE:           ShowMesh(KIRBY_MESH::MOUTH_NORMAL);         break;
            case KIRBY_MOUTH_STATE::OPEN:           ShowMesh(KIRBY_MESH::MOUTH_OPEN);           break;
            case KIRBY_MOUTH_STATE::ANGRY:          ShowMesh(KIRBY_MESH::MOUTH_ANGRY);          break;
            case KIRBY_MOUTH_STATE::SMILE_OPEN:     ShowMesh(KIRBY_MESH::MOUTH_SMILE_OPEN);     break;
            case KIRBY_MOUTH_STATE::SMILE_CLOSE:    ShowMesh(KIRBY_MESH::MOUTH_SMILE_CLOSE);    break;
        }
    }

    ShowMesh(KIRBY_MESH::LIMBS);

    return S_OK;
}

CKirby_Body* CKirby_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_Body* pInstance = new CKirby_Body(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_Body");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_Body::Clone(void* pArg)
{
    CKirby_Body* pInstance = new CKirby_Body(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_Body");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Body::Free()
{
    __super::Free();
}