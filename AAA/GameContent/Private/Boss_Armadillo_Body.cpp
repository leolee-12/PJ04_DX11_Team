#include "Boss_Armadillo_Body.h"
#include "Animator.h"
#include "Model.h"
#include "Shader.h"

CBoss_Armadillo_Body::CBoss_Armadillo_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMultiHitBoxPart(pDevice, pContext) {
}
CBoss_Armadillo_Body::CBoss_Armadillo_Body(const CBoss_Armadillo_Body& Prototype)
    : CMultiHitBoxPart(Prototype) {
}

HRESULT CBoss_Armadillo_Body::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg))) return E_FAIL;
    if (FAILED(Ready_Components()))        return E_FAIL;

    //// TODO: 본 이름은 실제 아르마딜로 모델 스켈레톤 확정 후 교체 (지금은 고릴라식 임시값)
    //Add_HitBox(AHB_RHAND, "RHaveL", COLLIDER::SPHERE, 2.5f, 0.f, 10.f, 8.f);
    //Add_HitBox(AHB_LHAND, "LHaveL", COLLIDER::SPHERE, 2.5f, 0.f, 10.f, 8.f);
    //Add_HitBox(AHB_ROLL, "RotL", COLLIDER::SPHERE, 5.f, 0.f, 15.f, 10.f);   // 구르기 전신 판정

    m_pAnimatorCom->Play("Wait", false);

    m_iShadowPassIdx = PASS_SHADOW;

    return S_OK;
}

HRESULT CBoss_Armadillo_Body::Render()
{
    if(FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPass = 0;
        switch (i)
        {
            case 1: case 4: iPass = PASS_BODY;  break;   // 몸통: 마스크 곱 알베도
            case 0: case 3: iPass = PASS_SHELL; break;   // 등딱지: UV/머티리얼 2세트
            case 2:         iPass = PASS_EYE;   break;   // 눈: 마스크 기반 팔레트
            default:        continue;                    // 나머지 메쉬는 스킵
        }

        if (FAILED(Bind_MeshMaterials(i, iPass)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(iPass)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CBoss_Armadillo_Body::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Armadillo;
    t.szModelProtoTag = MODEL_PROTO_TAG;
    t.szAnimEventFile = TEXT("../../Resources/YSH/Boss/Armadillo/Body/Armadillo_anim_events.json");  // 파일 없으면 무시됨
    return Ready_MeshPart(t);
}

HRESULT CBoss_Armadillo_Body::Bind_MeshMaterials(_uint iMeshIndex, _uint iPass)
{
    switch (iPass)
    {
        case PASS_BODY:
            if (FAILED(Bind_PBRSet(iMeshIndex, 0)))
                return E_FAIL;
            return m_pModelCom->Bind_Material(m_pShaderCom, "g_BodyMaskTexture", iMeshIndex, MTEX_TYPE::UNKNOWN, 0);

        case PASS_SHELL:
            if (FAILED(Bind_PBRSet(iMeshIndex, 0)))
                return E_FAIL;
            return Bind_PBRSet(iMeshIndex, 1);

        case PASS_EYE:
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", iMeshIndex, MTEX_TYPE::UNKNOWN, 0)))
                return E_FAIL;
            return m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", iMeshIndex, MTEX_TYPE::NORMALS, 0);
    }
    return S_OK;
}

HRESULT CBoss_Armadillo_Body::Bind_PBRSet(_uint iMeshIndex, _uint iSetIndex)
{
    static constexpr const _char* Diffuse[] = { "g_DiffuseTexture", "g_DiffuseTexture1" };
    static constexpr const _char* Normal[] = { "g_NormalTexture",  "g_NormalTexture1" };
    static constexpr const _char* MRA[] = { "g_MRATexture",     "g_MRATexture1" };

    if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, Diffuse[iSetIndex], iMeshIndex, MTEX_TYPE::DIFFUSE, iSetIndex)))
        return E_FAIL;
    if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, Normal[iSetIndex], iMeshIndex, MTEX_TYPE::NORMALS, iSetIndex)))
        return E_FAIL;
    return m_pModelCom->Bind_Material(m_pShaderCom, MRA[iSetIndex], iMeshIndex, MTEX_TYPE::METALNESS, iSetIndex);
}

CBoss_Armadillo_Body* CBoss_Armadillo_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Armadillo_Body* pInstance = new CBoss_Armadillo_Body(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CBoss_Armadillo_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBoss_Armadillo_Body* CBoss_Armadillo_Body::Clone(void* pArg)
{
    CBoss_Armadillo_Body* pInstance = new CBoss_Armadillo_Body(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CBoss_Armadillo_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Armadillo_Body::Free() { __super::Free(); }