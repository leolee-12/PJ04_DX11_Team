#include "Kirby_DeformCar_Demo.h"

#include "GameInstance.h"

#include "GameContent_const.h"

#include "Kirby.h"

CKirby_DeformCar_Demo::CKirby_DeformCar_Demo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_Deform_Model(pDevice, pContext)
{
}

CKirby_DeformCar_Demo::CKirby_DeformCar_Demo(const CKirby_DeformCar_Demo& Prototype)
    : CKirby_Deform_Model(Prototype) {
}

HRESULT CKirby_DeformCar_Demo::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_DeformCar_Demo::Initialize(void* pArg)
{
    KIRBY_DEFORMCAR_DEMO_DESC* pDesc = static_cast<KIRBY_DEFORMCAR_DEMO_DESC*>(pArg);

    pDesc->fSpeedPerSec = 1.f;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_bActive = false;

    return S_OK;
}

void CKirby_DeformCar_Demo::Priority_Update(_float fTimeDelta)
{
}

void CKirby_DeformCar_Demo::Update(_float fTimeDelta)
{
    if (m_bActive == false)
        return;

    if (m_pGameInstance_Proxy->Is_EditMode())
        return;

    m_pAnimatorCom->Update(fTimeDelta);
}

void CKirby_DeformCar_Demo::Late_Update(_float fTimeDelta)
{
    if (m_bActive == false)
        return;

    CPartObject::Compute_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
}

HRESULT CKirby_DeformCar_Demo::Render()
{
    if (m_pModelCom->Get_NumMeshes() < DEFORMCAR_DEMO_MESH::DEFORMCAR_DEMO_MESH_END)
        return E_FAIL;

    if (FAILED(Bind_ShaderResources(m_pKirbyShaderCom)))
        return E_FAIL;

    if (FAILED(Render_KirbyMesh(DEFORMCAR_DEMO_MESH::LIMBS)))
        return E_FAIL;

    if(m_bBodyAOn)
    {
        if (FAILED(Render_KirbyMesh(DEFORMCAR_DEMO_MESH::BODY_A)))
            return E_FAIL;
    }

    if (m_bBodyBOn)
    {
        if (FAILED(Render_KirbyMesh(DEFORMCAR_DEMO_MESH::BODY_B)))
            return E_FAIL;
    }


    return S_OK;
}

HRESULT CKirby_DeformCar_Demo::Ready_AnimEvents(CKirby* pKirby)
{
    m_pAnimatorCom->Set_EventCallback(
        [this](const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
        {
            if (Handle_AnimEventEye(e, ePhase) == true)
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

HRESULT CKirby_DeformCar_Demo::Ready_Components()
{
    /* For.Com_Shader */
    m_pKirbyShaderCom = Add_Component<CShader>(Shader_Kirby.iLevelID, Shader_Kirby.szProtoTag, TEXT("Com_Shader_Kirby"));
    if (m_pKirbyShaderCom == nullptr)
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
    AnimDesc.strDataFile = TEXT("../../Resources/YSE/DeformCar/Demo_AnimEvents.json");

    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));

    if (m_pAnimatorCom == nullptr || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby_DeformCar_Demo::Render_KirbyMesh(_uint iMeshIndex)
{
    if (FAILED(m_pEyeTextureCom->Bind_ShaderResource(m_pKirbyShaderCom, "g_EyeTexture", ETOUI(m_eEye))))
        return E_FAIL;

    if (FAILED(m_pEyeMaskTextureCom->Bind_ShaderResource(m_pKirbyShaderCom, "g_EyeMaskTexture", ETOUI(m_eEye))))
        return E_FAIL;

    if (FAILED(m_pModelCom->Bind_Material(m_pKirbyShaderCom, "g_SkinTexture", iMeshIndex, MTEX_TYPE::UNKNOWN, 1)))
        return E_FAIL;

    if (FAILED(m_pModelCom->Bind_Material(m_pKirbyShaderCom, "g_MouthTexture", iMeshIndex, MTEX_TYPE::METALNESS, 0)))
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

    if(FAILED(m_pModelCom->Render(iMeshIndex)))
        return E_FAIL;

    return S_OK;
}

CKirby_DeformCar_Demo* CKirby_DeformCar_Demo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_DeformCar_Demo* pInstance = new CKirby_DeformCar_Demo(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_DeformCar_Demo");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_DeformCar_Demo::Clone(void* pArg)
{
    CKirby_DeformCar_Demo* pInstance = new CKirby_DeformCar_Demo(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_DeformCar_Demo");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_DeformCar_Demo::Free()
{
    __super::Free();
}