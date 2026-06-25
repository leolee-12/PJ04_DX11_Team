#include "BladeKnight_Sword.h"
#include "GameInstance.h"

CBladeKnight_Sword::CBladeKnight_Sword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterHitPart{ pDevice, pContext } {
}

CBladeKnight_Sword::CBladeKnight_Sword(const CBladeKnight_Sword& Prototype)
    : CMonsterHitPart(Prototype) {
}

HRESULT CBladeKnight_Sword::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CBladeKnight_Sword::Initialize(void* pArg)
{
    if (nullptr == pArg)                   
        return E_FAIL;

    if (FAILED(__super::Initialize(pArg))) 
        return E_FAIL;   // 소켓행렬 수령

    if (nullptr == m_pSocketBoneMatrix)    
        return E_FAIL;   // 소켓 필수

    if (FAILED(Ready_Components()))        
        return E_FAIL;

    CAPSULE_DESC CapsuleDesc{};
    CapsuleDesc.vCenter = { 0.f, 0.f, -0.5f };
    CapsuleDesc.fHeight = { 0.8f };

    CapsuleDesc.fRadius = 0.25f;        
    CapsuleDesc.vRadians = { XMConvertToRadians(-90.f), 0.f, 0.f };

    if (FAILED(Ready_HitBox(CapsuleDesc, 5.f, 8.f)))
        return E_FAIL;

    m_pAnimatorCom->Play("Thrust", false, false);
    return S_OK;
}

HRESULT CBladeKnight_Sword::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_AnimMesh_PBR;
    t.szModelProtoTag = TEXT("Prototype_Component_Model_BladeKnight_Sword");
    return Ready_MeshPart(t);
}

// 메쉬 1번만 상수머티리얼 + pass 3, 나머지는 텍스처 + pass 1
HRESULT CBladeKnight_Sword::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPassIdx = 1;
        if (i != 1)
        {
            m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0);
            m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0);
            m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0);
        }
        else
        {
            m_pShaderCom->Bind_RawValue("g_vConstantDiffuse", &m_vConstantDiffuse, sizeof(_float4));
            m_pShaderCom->Bind_RawValue("g_vConstantMRA", &m_vConstantMRA, sizeof(_float3));
            m_pShaderCom->Bind_RawValue("g_vConstantEmissive", &m_vConstantEmissive, sizeof(_float4));
            iPassIdx = 3;
        }

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(iPassIdx)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }
    return S_OK;
}

CBladeKnight_Sword* CBladeKnight_Sword::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBladeKnight_Sword* pInstance = new CBladeKnight_Sword(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CBladeKnight_Sword");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CBladeKnight_Sword::Clone(void* pArg)
{
    CBladeKnight_Sword* pInstance = new CBladeKnight_Sword(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CBladeKnight_Sword");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBladeKnight_Sword::Free() { __super::Free(); }