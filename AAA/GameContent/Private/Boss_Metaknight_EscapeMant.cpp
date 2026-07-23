#include "Boss_Metaknight_EscapeMant.h"
#include "Animator.h"
#include "Model.h"
#include "Shader.h"

CBoss_Metaknight_EscapeMant::CBoss_Metaknight_EscapeMant(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterPart(pDevice, pContext) {
}
CBoss_Metaknight_EscapeMant::CBoss_Metaknight_EscapeMant(const CBoss_Metaknight_EscapeMant& Prototype)
    : CMonsterPart(Prototype) {
}

HRESULT CBoss_Metaknight_EscapeMant::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg))) return E_FAIL;
    if (FAILED(Ready_Components()))        return E_FAIL;

    m_pAnimatorCom->Play("EscapeStart", true);

    m_iShadowPassIdx = 0;

    return S_OK;
}

HRESULT CBoss_Metaknight_EscapeMant::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::UNKNOWN, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(9)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CBoss_Metaknight_EscapeMant::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Metaknight;
    t.szModelProtoTag = MODEL_PROTO_TAG;
    return Ready_MeshPart(t);
}

CBoss_Metaknight_EscapeMant* CBoss_Metaknight_EscapeMant::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Metaknight_EscapeMant* pInstance = new CBoss_Metaknight_EscapeMant(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CBoss_Metaknight_EscapeMant");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBoss_Metaknight_EscapeMant* CBoss_Metaknight_EscapeMant::Clone(void* pArg)
{
    CBoss_Metaknight_EscapeMant* pInstance = new CBoss_Metaknight_EscapeMant(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CBoss_Metaknight_EscapeMant");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Metaknight_EscapeMant::Free() { __super::Free(); }