#include "Boss_Cage_Body.h"
#include "Animator.h"
#include "Effect_Loader.h"
#include "GameInstance.h"

CBoss_Cage_Body::CBoss_Cage_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterPart{ pDevice, pContext }
{
}

CBoss_Cage_Body::CBoss_Cage_Body(const CBoss_Cage_Body& Prototype)
    : CMonsterPart(Prototype)
{
}

HRESULT CBoss_Cage_Body::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CBoss_Cage_Body::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    Ready_AnimEvent();

    m_pAnimatorCom->Play("Wait", true, true);
    //m_pAnimatorCom->Pause();

    return S_OK;
}

HRESULT CBoss_Cage_Body::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW,
        m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ,
        m_eProjType))))
        return E_FAIL;
    return S_OK;
}

void CBoss_Cage_Body::Ready_AnimEvent()
{
    m_pAnimatorCom->Set_EventCallback([this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase)
        {
            if (e.iEventType == ETOI(EANIM_EVENT::OnOffMesh))
            {
                m_bRender = false;
                CEffect_Loader::GetInstance()->Spawn(L"Split_Cage", Get_LevelIndex(), _float3(0.f, 3.f, 0.f),
                    _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f), &m_CombinedWorldMatrix);
                m_pGameInstance_Proxy->Play_SFX(L"DemoStageClear_CageBreak.wav", 0.45f);
            }
        }
    );
}

HRESULT CBoss_Cage_Body::Render()
{
    if (!m_bRender)
        return S_OK;

    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes =
        static_cast<_uint>(m_pModelCom->Get_NumMeshes());

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (i == 0 && !m_bRenderBird)
            continue;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;

        if (m_pAnimatorCom)
            if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
                return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(9)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CBoss_Cage_Body::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_AnimMesh_PBR;
    t.szModelProtoTag = MODEL_PROTO_TAG;
    t.bAnimated = true;
    t.szAnimEventFile = L"../../Resources/YSH/Gimmick/CaptiveCage/CageL/CageL_Anim_TopL_AnimEvents.json";

    if (FAILED(Ready_MeshPart(t)))
        return E_FAIL;

    return S_OK;
}

CBoss_Cage_Body* CBoss_Cage_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Cage_Body* pInstance = new CBoss_Cage_Body(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CBoss_Cage_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CBoss_Cage_Body::Clone(void* pArg)
{
    CBoss_Cage_Body* pInstance = new CBoss_Cage_Body(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CBoss_Cage_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Cage_Body::Free()
{
    __super::Free();
}
