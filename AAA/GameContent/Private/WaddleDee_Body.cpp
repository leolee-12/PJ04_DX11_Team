#include "WaddleDee_Body.h"
#include "GameInstance.h"
#include "GameContent_const.h"

#include "Cage_WaddleDee.h"

namespace
{
    constexpr _uint s_iShadowPass = 2;
}

CWaddleDee_Body::CWaddleDee_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject(pDevice, pContext)
{
}

CWaddleDee_Body::CWaddleDee_Body(const CWaddleDee_Body& Prototype)
    : CPartObject(Prototype)
{
}

HRESULT CWaddleDee_Body::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CWaddleDee_Body::Initialize(void* pArg)
{
    if (nullptr == pArg)
        return E_FAIL;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (nullptr == m_pParentMatrix)
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CWaddleDee_Body::Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    m_pAnimatorCom->Update(fTimeDelta);
}

void CWaddleDee_Body::Late_Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    Compute_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
}

HRESULT CWaddleDee_Body::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPass = 0;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;

        if (0 == i)
        {
            if (FAILED(m_pEyeTextureCom->Bind_ShaderResource(m_pShaderCom, "g_EyeTexture", m_iEyeIndex)))
                return E_FAIL;
            if (FAILED(m_pEyeMaskTextureCom->Bind_ShaderResource(m_pShaderCom, "g_EyeMaskTexture", m_iEyeIndex)))
                return E_FAIL;
        }
        else
            iPass = 1;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(iPass)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CWaddleDee_Body::Render_Shadow()
{
    if (!m_bActive)
        return S_OK;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(s_iShadowPass)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

_bool CWaddleDee_Body::Has_Animation(const _char* pAnimName) const
{
    if (nullptr == m_pModelCom || nullptr == pAnimName)
        return false;

    return 0 <= m_pModelCom->Get_AnimationIndex(pAnimName);
}

const _float4x4* CWaddleDee_Body::Get_BoneMatrixPtr(const _char* pBoneName) const
{
    if (nullptr == m_pModelCom || nullptr == pBoneName)
        return nullptr;

    return m_pModelCom->Get_BoneMatrixPtr(pBoneName);
}

HRESULT CWaddleDee_Body::Ready_Components()
{
    m_pShaderCom = Add_Component<CShader>(Shader_WaddleDee.iLevelID, Shader_WaddleDee.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, CCage_WaddleDee::MODEL_PROTO_TAG, TEXT("Com_Model"));
    if (nullptr == m_pModelCom)
        return E_FAIL;

    CAnimator::ANIMATOR_DESC AnimDesc{};
    AnimDesc.pModel = m_pModelCom;
    AnimDesc.strDataFile = TEXT("../../Resources/YSH/WaddleDee/Body/Model_Anim_AnimEvents.json");

    m_pEyeTextureCom = Add_Component<CTexture>(TEXT("Com_EyeTexture"),
        CTexture::Create(m_pDevice, m_pContext,
            L"../../Resources/YSH/WaddleDee/Body/DeeEye.%02d.dds", EYE_COUNT));
    if (nullptr == m_pEyeTextureCom)
        return E_FAIL;

    m_pEyeMaskTextureCom = Add_Component<CTexture>(TEXT("Com_EyeMaskTexture"),
        CTexture::Create(m_pDevice, m_pContext,
            L"../../Resources/YSH/WaddleDee/Body/DeeEyeMask.%02d.dds", EYE_COUNT));
    if (nullptr == m_pEyeMaskTextureCom)
        return E_FAIL;

    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
    if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CWaddleDee_Body::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

CWaddleDee_Body* CWaddleDee_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CWaddleDee_Body* pInstance = new CWaddleDee_Body(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CWaddleDee_Body");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CWaddleDee_Body::Clone(void* pArg)
{
    CWaddleDee_Body* pInstance = new CWaddleDee_Body(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CWaddleDee_Body");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CWaddleDee_Body::Free()
{
    __super::Free();
}