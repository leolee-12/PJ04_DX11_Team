#include "Kirby_DeformCylinder_Demo.h"

#include "GameInstance.h"

CKirby_DeformCylinder_Demo::CKirby_DeformCylinder_Demo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_Deform_Model(pDevice, pContext)
{
}

CKirby_DeformCylinder_Demo::CKirby_DeformCylinder_Demo(const CKirby_DeformCylinder_Demo& Prototype)
    : CKirby_Deform_Model(Prototype)
{
}

HRESULT CKirby_DeformCylinder_Demo::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;

    return S_OK;
}

HRESULT CKirby_DeformCylinder_Demo::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_bBodyAOn = true;
    m_bBodyBOn = false;

    m_bActive = false;

    return S_OK;
}

HRESULT CKirby_DeformCylinder_Demo::Render()
{
    if (m_pModelCom->Get_NumMeshes() < DEFORMCYLINDER_DEMO_MESH_END)
        return E_FAIL;

    if (FAILED(Bind_CommonShaderResources(m_pKirbyShaderCom)))
        return E_FAIL;

    if (FAILED(m_pEyeTextureCom->Bind_ShaderResource(m_pKirbyShaderCom, "g_EyeTexture", ETOUI(m_eEye))))
        return E_FAIL;
    if (FAILED(m_pEyeMaskTextureCom->Bind_ShaderResource(m_pKirbyShaderCom, "g_EyeMaskTexture", ETOUI(m_eEye))))
        return E_FAIL;

    if (FAILED(m_pKirbyShaderCom->Bind_RawValue("g_vBodyColor", &s_vBodyColor, sizeof(s_vBodyColor))))
        return E_FAIL;
    if (FAILED(m_pKirbyShaderCom->Bind_RawValue("g_vFootColor", &s_vFootColor, sizeof(s_vFootColor))))
        return E_FAIL;
    if (FAILED(m_pKirbyShaderCom->Bind_RawValue("g_vBlushColor", &s_vBlushColor, sizeof(s_vBlushColor))))
        return E_FAIL;

    const auto RenderKirbyMesh = [this](_uint iMeshIndex) -> HRESULT
        {
            if (FAILED(m_pModelCom->Bind_Material(m_pKirbyShaderCom, "g_SkinTexture", iMeshIndex, MTEX_TYPE::UNKNOWN, 1)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pKirbyShaderCom, "g_MouthTexture", iMeshIndex, MTEX_TYPE::METALNESS, 0)))
                return E_FAIL;

            if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pKirbyShaderCom, "g_BoneMatrices", iMeshIndex)))
                return E_FAIL;

            if (FAILED(m_pKirbyShaderCom->Begin(ETOUI(KIRBY_SHADER_PASS::KIRBY))))
                return E_FAIL;

            return m_pModelCom->Render(iMeshIndex);
        };

    if (FAILED(RenderKirbyMesh(LIMBS)))
        return E_FAIL;

    if (m_bBodyAOn && FAILED(RenderKirbyMesh(BODY_A)))
        return E_FAIL;

    if (m_bBodyBOn && FAILED(RenderKirbyMesh(BODY_B)))
        return E_FAIL;

    return S_OK;
}

_bool CKirby_DeformCylinder_Demo::Should_RenderShadowMesh(_uint iMeshIndex)
{
    switch (iMeshIndex)
    {
        case LIMBS:  return true;
        case BODY_A: return m_bBodyAOn;
        case BODY_B: return m_bBodyBOn;
    }

    return false;
}

HRESULT CKirby_DeformCylinder_Demo::Ready_AnimEvents(CKirby* pKirby)
{
    m_pAnimatorCom->Set_EventCallback(
        [this, pKirby](const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
        {
            if (Handle_AnimEventParent(pKirby, e, ePhase) == true)
                return;

            switch (static_cast<EANIM_EVENT>(e.iEventType))
            {
                case EANIM_EVENT::OnOffMesh:
                    if (e.iIntParam == 0)
                    {
                        m_bBodyAOn = true;
                        m_bBodyBOn = false;
                    }
                    else if (e.iIntParam == 1)
                    {
                        m_bBodyAOn = false;
                        m_bBodyBOn = true;
                    }
                    break;
            }
        }
    );

    return S_OK;
}

HRESULT CKirby_DeformCylinder_Demo::Ready_Components()
{
    /* For.Com_Shader */
    m_pKirbyShaderCom = Add_Component<CShader>(Shader_Kirby.iLevelID, Shader_Kirby.szProtoTag, TEXT("Com_Shader_Kirby"));
    if (m_pKirbyShaderCom == nullptr)
        return E_FAIL;

    /* For.Com_Model */
    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, TEXT("Prototype_Component_Model_Kirby_DeformCylinder_Demo"), TEXT("Com_Model"));
    if (m_pModelCom == nullptr)
        return E_FAIL;

    /* For.Com_EyeTexture */
    m_pEyeTextureCom = Add_Component<CTexture>(TEXT("Com_EyeTexture"),
        CTexture::Create(m_pDevice,m_pContext, L"../../Resources/YSE/DeformCylinder/Demo/KirbyEye.%02d.dds", ETOUI(KIRBY_EYE_STATE::END)));
    if (m_pEyeTextureCom == nullptr)
        return E_FAIL;

    /* For.Com_EyeMaskTexture */
    m_pEyeMaskTextureCom = Add_Component<CTexture>(TEXT("Com_EyeMaskTexture"),
        CTexture::Create( m_pDevice, m_pContext, L"../../Resources/YSE/DeformCylinder/Demo/KirbyEyeMask.%02d.dds", ETOUI(KIRBY_EYE_STATE::END)));
    if (m_pEyeMaskTextureCom == nullptr)
        return E_FAIL;

    /* For.Com_Animator */
    CAnimator::ANIMATOR_DESC AnimDesc{};
    AnimDesc.pModel = m_pModelCom;
    AnimDesc.strDataFile = TEXT("../../Resources/YSE/DeformCylinder/Demo/Demo_AnimEvents.json");

    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
    if (m_pAnimatorCom == nullptr || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
        return E_FAIL;

    return S_OK;
}

CKirby_DeformCylinder_Demo* CKirby_DeformCylinder_Demo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_DeformCylinder_Demo* pInstance = new CKirby_DeformCylinder_Demo(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_DeformCylinder_Demo");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_DeformCylinder_Demo::Clone(void* pArg)
{
    CKirby_DeformCylinder_Demo* pInstance = new CKirby_DeformCylinder_Demo(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_DeformCylinder_Demo");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_DeformCylinder_Demo::Free()
{
    __super::Free();
}
