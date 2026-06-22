#include "Boss_Gorilla_Body.h"
#include "Animator.h"
#include "Model.h"
#include "Shader.h"

CBoss_Gorilla_Body::CBoss_Gorilla_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMultiHitBoxPart(pDevice, pContext) {
}
CBoss_Gorilla_Body::CBoss_Gorilla_Body(const CBoss_Gorilla_Body& Prototype)
    : CMultiHitBoxPart(Prototype) {
}

HRESULT CBoss_Gorilla_Body::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg))) return E_FAIL;   // -> CMonsterPart
    if (FAILED(Ready_Components()))        return E_FAIL;   // 모델/애님/셰이더 (본 준비됨)

    // 히트박스 1개 or 2개: 한 줄씩 추가. 단일이면 한 줄만 남기면 됨.
    // TODO: 실제 본 이름 / 크기 / 데미지.
    Add_HitBox("RHaveL", COLLIDER::SPHERE, 3.f, 0.f, 15.f, 10.f);
    Add_HitBox("LHaveL", COLLIDER::SPHERE, 3.f, 0.f, 15.f, 10.f);

    m_pAnimatorCom->Play("Wait", false);

    return S_OK;
}

HRESULT CBoss_Gorilla_Body::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (i == 4)
        {
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::UNKNOWN, 0)))
                return E_FAIL;
        }
        else
        {
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
                return E_FAIL;
        }

        if (i == 2)
            if(FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_BodyMaskTexture", i, MTEX_TYPE::DIFFUSE, 1)))
                return E_FAIL;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        switch (i)
        {
            case 0:
            {
                if (FAILED(m_pShaderCom->Begin(3)))
                    return E_FAIL;
                break;
            }
            case 1:
            {
                if (FAILED(m_pShaderCom->Begin(2)))
                    return E_FAIL;
                break;
            }
            case 2:
            {
                if (FAILED(m_pShaderCom->Begin(0)))
                    return E_FAIL;
                break;
            }
            case 3:
            {
                if (FAILED(m_pShaderCom->Begin(2)))
                    return E_FAIL;
                break;
            }
            case 4:
            {
                if (FAILED(m_pShaderCom->Begin(1)))
                    return E_FAIL;
                break;
            }
            case 5:
            {
                if (FAILED(m_pShaderCom->Begin(0)))
                    return E_FAIL;
                break;
            }
            case 6:
            {
                if (FAILED(m_pShaderCom->Begin(3)))
                    return E_FAIL;
                break;
            }
        }
        
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CBoss_Gorilla_Body::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Gorilla;
    t.szModelProtoTag = TEXT("Prototype_Component_Model_Boss_Gorilla_Body");
    t.szAnimEventFile = nullptr;        
    return Ready_MeshPart(t);
}

CBoss_Gorilla_Body* CBoss_Gorilla_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Gorilla_Body* pInstance = new CBoss_Gorilla_Body(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CBoss_Gorilla_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBoss_Gorilla_Body* CBoss_Gorilla_Body::Clone(void* pArg)
{
    CBoss_Gorilla_Body* pInstance = new CBoss_Gorilla_Body(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CBoss_Gorilla_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Gorilla_Body::Free() { __super::Free(); }