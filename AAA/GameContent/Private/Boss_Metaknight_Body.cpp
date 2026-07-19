#include "Boss_Metaknight_Body.h"
#include "Animator.h"
#include "Model.h"
#include "Shader.h"

CBoss_Metaknight_Body::CBoss_Metaknight_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMultiHitBoxPart(pDevice, pContext) {
}
CBoss_Metaknight_Body::CBoss_Metaknight_Body(const CBoss_Metaknight_Body& Prototype)
    : CMultiHitBoxPart(Prototype) {
}

HRESULT CBoss_Metaknight_Body::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg))) return E_FAIL;
    if (FAILED(Ready_Components()))        return E_FAIL;

    // TODO: 검 파트 생기면 검 쪽으로 이동 검토. 지금은 오른손 소켓 기준 임시 판정
    Add_HitBox(MKHB_SLASH, "RHaveL", COLLIDER::SPHERE, 2.f, 0.f, 0.f, 0.f);

    m_pAnimatorCom->Play("Wait", false);

    m_iShadowPassIdx = PASS_SHADOW;

    return S_OK;
}

HRESULT CBoss_Metaknight_Body::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPass = 0;
        switch (i)
        {
            case 0:         iPass = PASS_FACE;     break;  
            case 4:         iPass = PASS_BODY;     break;  
            case 1: case 2: iPass = PASS_WING;     break;
            case 3:         iPass = PASS_SHOULDER; break;
            case 5: case 6: iPass = PASS_EYE;      break;
            default:        continue;
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

HRESULT CBoss_Metaknight_Body::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Metaknight;
    t.szModelProtoTag = MODEL_PROTO_TAG;
    // TODO: 애님 이벤트 json 만들면 경로 연결
    return Ready_MeshPart(t);
}

HRESULT CBoss_Metaknight_Body::Bind_MeshMaterials(_uint iMeshIndex, _uint iPass)
{
    switch (iPass)
    {
        case PASS_BODY:
        case PASS_WING:
            return Bind_PBRSet(iMeshIndex);

        case PASS_SHOULDER:
            if (FAILED(Bind_PBRSet(iMeshIndex)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MarkTexture", iMeshIndex, MTEX_TYPE::UNKNOWN, 0)))
                return E_FAIL;
            return m_pModelCom->Bind_Material(m_pShaderCom, "g_MarkNormalTexture", iMeshIndex, MTEX_TYPE::NORMALS, 1);

        case PASS_EYE:
            return m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", iMeshIndex, MTEX_TYPE::UNKNOWN, 0);

        case PASS_FACE:
            if (FAILED(Bind_PBRSet(iMeshIndex)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_FaceEyeTexture", iMeshIndex, MTEX_TYPE::UNKNOWN, 2)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MouthTexture", iMeshIndex, MTEX_TYPE::UNKNOWN, 1)))
                return E_FAIL;
            return m_pModelCom->Bind_Material(m_pShaderCom, "g_FaceEyeNormalTexture", iMeshIndex, MTEX_TYPE::NORMALS, 2);
    }
    return S_OK;
}

HRESULT CBoss_Metaknight_Body::Bind_PBRSet(_uint iMeshIndex)
{
    if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", iMeshIndex, MTEX_TYPE::DIFFUSE, 0)))
        return E_FAIL;
    if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", iMeshIndex, MTEX_TYPE::NORMALS, 0)))
        return E_FAIL;
    return m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", iMeshIndex, MTEX_TYPE::METALNESS, 0);
}

CBoss_Metaknight_Body* CBoss_Metaknight_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Metaknight_Body* pInstance = new CBoss_Metaknight_Body(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CBoss_Metaknight_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBoss_Metaknight_Body* CBoss_Metaknight_Body::Clone(void* pArg)
{
    CBoss_Metaknight_Body* pInstance = new CBoss_Metaknight_Body(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CBoss_Metaknight_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Metaknight_Body::Free() { __super::Free(); }