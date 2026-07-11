#include "DialogueDee.h"
#include "GameInstance.h"
#include "Shader.h"
#include "Model.h"
#include "Animator.h"
#include "Cage_WaddleDee.h"

CDialogueDee::CDialogueDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCutsceneActor(pDevice, pContext) {
}
CDialogueDee::CDialogueDee(const CDialogueDee& Prototype)
    : CCutsceneActor(Prototype) {
}

HRESULT CDialogueDee::Ready_Components()
{
    VISUAL_SETUP t{};
    t.tShader = Shader_WaddleDee;
    t.szModelProtoTag = CCage_WaddleDee::MODEL_PROTO_TAG;
    if (FAILED(Ready_Visual(t)))
        return E_FAIL;

    Play("Wait", true);
    return S_OK;
}

HRESULT CDialogueDee::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (!m_pGameInstance_Proxy->Is_EditMode())
        Set_Active(false);

    return S_OK;
}

HRESULT CDialogueDee::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPass = 0;
        if (i == 0)
        {
            m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0);
            m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0);
            m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0);
            m_pModelCom->Bind_Material(m_pShaderCom, "g_EyeTexture", i, MTEX_TYPE::UNKNOWN, 0);
            m_pModelCom->Bind_Material(m_pShaderCom, "g_EyeMaskTexture", i, MTEX_TYPE::UNKNOWN, 2);
        }
        else
        {
            m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0);
            m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0);
            m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0);
            iPass = 1;
        }

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(iPass)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }
    return S_OK;
}

CDialogueDee* CDialogueDee::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CDialogueDee* pInstance = new CDialogueDee(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CDialogueDee");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CDialogueDee::Clone(void* pArg)
{
    CDialogueDee* pInstance = new CDialogueDee(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CDialogueDee");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CDialogueDee::Free()
{
    __super::Free();
}