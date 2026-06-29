#include "Kirby_DeformCar_Main.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CKirby_DeformCar_Main::CKirby_DeformCar_Main(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_Form(pDevice, pContext)
{
}

CKirby_DeformCar_Main::CKirby_DeformCar_Main(const CKirby_DeformCar_Main& Prototype)
    : CKirby_Form(Prototype) {
}

HRESULT CKirby_DeformCar_Main::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_DeformCar_Main::Initialize(void* pArg)
{
    KIRBY_DEFORMCAR_MAIN_DESC* pDesc = static_cast<KIRBY_DEFORMCAR_MAIN_DESC*>(pArg);

    pDesc->fSpeedPerSec = 1.f;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pAnimatorCom->Play("Deform", true, true);

    return S_OK;
}

void CKirby_DeformCar_Main::Priority_Update(_float fTimeDelta)
{
}

void CKirby_DeformCar_Main::Update(_float fTimeDelta)
{
    if (m_pGameInstance_Proxy->Is_EditMode())
        return;

    m_pAnimatorCom->Update(fTimeDelta);
}

void CKirby_DeformCar_Main::Late_Update(_float fTimeDelta)
{
    CPartObject::Compute_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CKirby_DeformCar_Main::Render()
{
    if (m_pModelCom->Get_NumMeshes() < MESH_END)
        return E_FAIL;

    if (FAILED(Bind_ShaderResources(m_pPBRShaderCom)))
        return E_FAIL;

    if (FAILED(Bind_ShaderResources(m_pKirbyShaderCom)))
        return E_FAIL;

    if (FAILED(Render_PBRMesh(DEFORMCAR_MAIN_MESH::MESH_CAR)))
        return E_FAIL;

    if (FAILED(Render_KirbyMesh(DEFORMCAR_MAIN_MESH::MESH_KIRBY)))
        return E_FAIL;

    if (FAILED(Render_PBRMesh(DEFORMCAR_MAIN_MESH::MESH_TIRES)))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby_DeformCar_Main::Ready_Components()
{
    /* For.Com_Shader */
    m_pKirbyShaderCom = Add_Component<CShader>(Shader_Kirby.iLevelID, Shader_Kirby.szProtoTag, TEXT("Com_Shader_Kirby"));
    if (m_pKirbyShaderCom == nullptr)
        return E_FAIL;

    m_pPBRShaderCom = Add_Component<CShader>(Shader_VtxAnimMesh.iLevelID, Shader_VtxAnimMesh.szProtoTag, TEXT("Com_Shader_PBR"));
    if (m_pPBRShaderCom == nullptr)
        return E_FAIL;

    /* For.Com_Model */
    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, TEXT("Prototype_Component_Model_Kirby_DeformCar_Demo"), TEXT("Com_Model"));
    if (m_pModelCom == nullptr)
        return E_FAIL;

    m_pEyeTextureCom = Add_Component<CTexture>(TEXT("Com_EyeTexture"),
        CTexture::Create(m_pDevice, m_pContext, L"../../Resources/YSE/DeformCar/KirbyEye.%02d.dds", ETOUI(KIRBY_EYE_STATE::END)));
    if (m_pEyeTextureCom == nullptr)
        return E_FAIL;

    m_pEyeMaskTextureCom = Add_Component<CTexture>(TEXT("Com_EyeMaskTexture"),
        CTexture::Create(m_pDevice, m_pContext, L"../../Resources/YSE/DeformCar/KirbyEyeMask.%02d.dds", ETOUI(KIRBY_EYE_STATE::END)));
    if (m_pEyeMaskTextureCom == nullptr)
        return E_FAIL;

    /* For.Com_Animator */
    CAnimator::ANIMATOR_DESC AnimDesc{};
    AnimDesc.pModel = m_pModelCom;

    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));

    if (m_pAnimatorCom == nullptr || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby_DeformCar_Main::Render_KirbyMesh(_uint iMeshIndex)
{
    if (FAILED(m_pEyeTextureCom->Bind_ShaderResource(m_pKirbyShaderCom, "g_EyeTexture", ETOUI(m_eEye))))
        return E_FAIL;

    if (FAILED(m_pEyeMaskTextureCom->Bind_ShaderResource(m_pKirbyShaderCom, "g_EyeMaskTexture", ETOUI(m_eEye))))
        return E_FAIL;

    if (FAILED(m_pModelCom->Bind_Material(m_pKirbyShaderCom, "g_SkinTexture", iMeshIndex, MTEX_TYPE::UNKNOWN, 1)))
        return E_FAIL;

    if (FAILED(m_pModelCom->Bind_Material(m_pKirbyShaderCom, "g_MouthTexture", iMeshIndex, MTEX_TYPE::UNKNOWN, 2)))
        return E_FAIL;

    if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pKirbyShaderCom, "g_BoneMatrices", iMeshIndex)))
        return E_FAIL;

    if (FAILED(m_pKirbyShaderCom->Bind_RawValue("g_vBodyColor", &m_vBodyColor, sizeof(_float4))))
        return E_FAIL;

    if (FAILED(m_pKirbyShaderCom->Bind_RawValue("g_vFootColor", &m_vFootColor, sizeof(_float4))))
        return E_FAIL;

    if (FAILED(m_pKirbyShaderCom->Bind_RawValue("g_vBlushColor", &m_vBlushColor, sizeof(_float4))))
        return E_FAIL;

    if (FAILED(m_pKirbyShaderCom->Begin(0)))
        return E_FAIL;

    if (FAILED(m_pModelCom->Render(iMeshIndex)))
        return E_FAIL;

    return E_FAIL;
}

CKirby_DeformCar_Main* CKirby_DeformCar_Main::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_DeformCar_Main* pInstance = new CKirby_DeformCar_Main(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_DeformCar_Main");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_DeformCar_Main::Clone(void* pArg)
{
    CKirby_DeformCar_Main* pInstance = new CKirby_DeformCar_Main(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_DeformCar_Main");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_DeformCar_Main::Free()
{
    __super::Free();
}