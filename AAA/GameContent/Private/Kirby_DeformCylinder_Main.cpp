#include "Kirby_DeformCylinder_Main.h"

#include "GameInstance.h"

CKirby_DeformCylinder_Main::CKirby_DeformCylinder_Main(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_HitBox_Model(pDevice, pContext)
{
}

CKirby_DeformCylinder_Main::CKirby_DeformCylinder_Main(const CKirby_DeformCylinder_Main& Prototype)
    : CKirby_HitBox_Model(Prototype)
{
}

HRESULT CKirby_DeformCylinder_Main::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;

    return S_OK;
}

HRESULT CKirby_DeformCylinder_Main::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_bActive = false;

    return S_OK;
}

HRESULT CKirby_DeformCylinder_Main::Render()
{
    if (m_pModelCom->Get_NumMeshes() < MESH_END)
        return E_FAIL;

    if (FAILED(Bind_CommonShaderResources(m_pKirbyShaderCom)))
        return E_FAIL;

    const auto RenderPartMesh = [this](_uint iMeshIndex) -> HRESULT
        {
            if (FAILED(m_pModelCom->Bind_Material(m_pKirbyShaderCom, "g_DiffuseTexture", iMeshIndex, MTEX_TYPE::DIFFUSE, 0)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pKirbyShaderCom, "g_NormalTexture", iMeshIndex, MTEX_TYPE::NORMALS, 0)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pKirbyShaderCom, "g_MRATexture", iMeshIndex, MTEX_TYPE::METALNESS, 0)))
                return E_FAIL;

            if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pKirbyShaderCom, "g_BoneMatrices", iMeshIndex)))
                return E_FAIL;

            if (FAILED(m_pKirbyShaderCom->Begin(ETOUI(KIRBY_SHADER_PASS::ANIM_TEXTURED_PBR))))
                return E_FAIL;

            return m_pModelCom->Render(iMeshIndex);
        };

    // Mesh 0: Kirby
    if (FAILED(m_pEyeTextureCom->Bind_ShaderResource(m_pKirbyShaderCom, "g_EyeTexture", ETOUI(m_eEye))))
        return E_FAIL;
    if (FAILED(m_pEyeMaskTextureCom->Bind_ShaderResource(m_pKirbyShaderCom, "g_EyeMaskTexture", ETOUI(m_eEye))))
        return E_FAIL;

    if (FAILED(m_pModelCom->Bind_Material(m_pKirbyShaderCom, "g_SkinTexture", DEFORM_CYLINDER_MESH::MESH_KIRBY, MTEX_TYPE::UNKNOWN, 1)))
        return E_FAIL;
    if (FAILED(m_pModelCom->Bind_Material(m_pKirbyShaderCom, "g_MouthTexture", DEFORM_CYLINDER_MESH::MESH_KIRBY, MTEX_TYPE::UNKNOWN, 2)))
        return E_FAIL;

    if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pKirbyShaderCom, "g_BoneMatrices", DEFORM_CYLINDER_MESH::MESH_KIRBY)))
        return E_FAIL;

    if (FAILED(m_pKirbyShaderCom->Bind_RawValue("g_vBodyColor", &s_vBodyColor, sizeof(s_vBodyColor))))
        return E_FAIL;
    if (FAILED(m_pKirbyShaderCom->Bind_RawValue("g_vFootColor", &s_vFootColor, sizeof(s_vFootColor))))
        return E_FAIL;
    if (FAILED(m_pKirbyShaderCom->Bind_RawValue("g_vBlushColor", &s_vBlushColor, sizeof(s_vBlushColor))))
        return E_FAIL;

    if (FAILED(m_pKirbyShaderCom->Begin(ETOUI(KIRBY_SHADER_PASS::KIRBY))))
        return E_FAIL;

    if (FAILED(m_pModelCom->Render(DEFORM_CYLINDER_MESH::MESH_KIRBY)))
        return E_FAIL;

    // Mesh 1: Cylinder
    if (FAILED(RenderPartMesh(DEFORM_CYLINDER_MESH::MESH_CYLINDER)))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby_DeformCylinder_Main::Ready_AnimEvents(CKirby* pKirby)
{
    m_pAnimatorCom->Set_EventCallback(
        [this, pKirby](const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
        {
            if (Handle_AnimEventParent(pKirby, e, ePhase) == true)
                return;
        }
    );

    return S_OK;
}

HRESULT CKirby_DeformCylinder_Main::Ready_Components()
{
    /* For.Com_Shader */
    m_pKirbyShaderCom = Add_Component<CShader>(Shader_Kirby.iLevelID, Shader_Kirby.szProtoTag, TEXT("Com_Shader_Kirby"));
    if (m_pKirbyShaderCom == nullptr)
        return E_FAIL;

    /* For.Com_Model */
    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, TEXT("Prototype_Component_Model_Kirby_DeformCylinder_Main"), TEXT("Com_Model"));
    if (m_pModelCom == nullptr)
        return E_FAIL;

    /* For.Com_EyeTexture */
    m_pEyeTextureCom = Add_Component<CTexture>(TEXT("Com_EyeTexture"),
        CTexture::Create(m_pDevice, m_pContext, L"../../Resources/YSE/DeformCylinder/Model/KirbyEye.%02d.dds", ETOUI(KIRBY_EYE_STATE::END)));
    if (m_pEyeTextureCom == nullptr)
        return E_FAIL;

    /* For.Com_EyeMaskTexture */
    m_pEyeMaskTextureCom = Add_Component<CTexture>(TEXT("Com_EyeMaskTexture"),
        CTexture::Create(m_pDevice, m_pContext, L"../../Resources/YSE/DeformCylinder/Model/KirbyEyeMask.%02d.dds", ETOUI(KIRBY_EYE_STATE::END)));
    if (m_pEyeMaskTextureCom == nullptr)
        return E_FAIL;

    /* For.Com_Animator */
    CAnimator::ANIMATOR_DESC AnimDesc{};
    AnimDesc.pModel = m_pModelCom;
    AnimDesc.strDataFile = TEXT("../../Resources/YSE/DeformCylinder/Model/Main_AnimEvents.json");

    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"),
        CAnimator::Create(m_pDevice, m_pContext));
    if (m_pAnimatorCom == nullptr || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
        return E_FAIL;

    // юс╫ц
    CCollider::COLLIDER_DESC WallBreakerDesc{};
    WallBreakerDesc.pOwner = this;
    WallBreakerDesc.vCenter = _float3(0.f, 1.5f, 1.5f);
    WallBreakerDesc.fRadius = 2.5f;

    m_pHitBox = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
        TEXT("WallBreakerCollider_Com"), &WallBreakerDesc);
    if (m_pHitBox == nullptr)
        return E_FAIL;

    m_pHitBox->Set_Enabled(false);
    //m_pGameInstance_Proxy->Register_Collider(m_pHitBox, ETOUI(COLLISION_LAYER::CAR_BOOST));

    return S_OK;
}

CKirby_DeformCylinder_Main* CKirby_DeformCylinder_Main::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_DeformCylinder_Main* pInstance = new CKirby_DeformCylinder_Main(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_DeformCylinder_Main");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_DeformCylinder_Main::Clone(void* pArg)
{
    CKirby_DeformCylinder_Main* pInstance = new CKirby_DeformCylinder_Main(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_DeformCylinder_Main");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_DeformCylinder_Main::Free()
{
    __super::Free();
}
