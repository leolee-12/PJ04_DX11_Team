
#include "TestMarb1e.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "Animator.h"
#include "GameContent_AnimEvents.h"

CTestMarb1e::CTestMarb1e(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
    , m_iAnimationIndex{ 0 }
{

}

CTestMarb1e::CTestMarb1e(const CTestMarb1e& Prototype)
    : CGameObject(Prototype)
    , m_iAnimationIndex{ Prototype.m_iAnimationIndex }
{

}

HRESULT CTestMarb1e::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CTestMarb1e::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CTestMarb1e::Priority_Update(_float fTimeDelta)
{

}

void CTestMarb1e::Update(_float fTimeDelta)
{
    /*if (m_pGameInstance_Proxy->Is_EditMode())
        return;*/
    /*m_pAnimatorCom->Play("Attack", true);

    m_pAnimatorCom->Update(fTimeDelta);*/
}

void CTestMarb1e::Late_Update(_float fTimeDelta)
{
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CTestMarb1e::Render()
{
    if (FAILED(Bind_ShaderResources()))

        return E_FAIL;

    size_t      iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
        //for (size_t i = 0; i < 1; i++)
    {
        _uint iPassIdx = 0;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", (_uint)i, MTEX_TYPE::DIFFUSE, 0)))
            iPassIdx = 0;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", (_uint)i, MTEX_TYPE::NORMALS, 0)))
            iPassIdx = 0;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", (_uint)i, MTEX_TYPE::METALNESS, 0)))
            iPassIdx = 0;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_UnkownTexture", (_uint)i, MTEX_TYPE::UNKNOWN, 0)))
            iPassIdx = 1;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", (_uint)i)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(1)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render((_uint)i)))
            return E_FAIL;
    }

    // debug
    //m_pNavigationCom->Render();

    return S_OK;
}

HRESULT CTestMarb1e::Ready_Components()
{
    m_pShaderCom = Add_Component<CShader>(Shader_AnimMesh_PBR.iLevelID, Shader_AnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Marb1e"), TEXT("Com_Model"));
    if (nullptr == m_pModelCom)
        return E_FAIL;

    CAnimator::ANIMATOR_DESC AnimDesc{};
    AnimDesc.pModel = m_pModelCom;
    AnimDesc.strDataFile = TEXT("../Bin/Resources/Models/Test/Marb1e/Marb1e_animevents.json");

    m_pAnimatorCom = Add_Component<CAnimator>(
        TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
    if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
        return E_FAIL;

    m_pAnimatorCom->Set_EventCallback(
        [this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase)
        {
            switch (static_cast<EANIM_EVENT>(e.iEventType))
            {
                case EANIM_EVENT::Fx:
                    if (phase == ANIM_EVENT_PHASE::POINT)
                    {
                        int TestBreak = 10;
                    }
                    break;
                case EANIM_EVENT::Hitbox:
                    if (phase == ANIM_EVENT_PHASE::BEGIN)
                        int TestBreak = 10;
                    if (phase == ANIM_EVENT_PHASE::END)
                        int TestBreak = 10;
                    break;
                default: break;
            }
        });


    return S_OK;
}

HRESULT CTestMarb1e::Bind_ShaderResources()
{

    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}


CTestMarb1e* CTestMarb1e::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTestMarb1e* pInstance = new CTestMarb1e(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CTestMarb1e");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTestMarb1e::Clone(void* pArg)
{
    CTestMarb1e* pInstance = new CTestMarb1e(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CTestMarb1e");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTestMarb1e::Free()
{
    __super::Free();
}
