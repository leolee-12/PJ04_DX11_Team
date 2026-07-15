#include "Boss_Armadillo_Cage.h"
#include "Animator.h"
#include "Model.h"
#include "Shader.h"
#include "GameContent_const.h"   // Shader_AnimMesh_PBR

CBoss_Armadillo_Cage::CBoss_Armadillo_Cage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterPart(pDevice, pContext) {
}
CBoss_Armadillo_Cage::CBoss_Armadillo_Cage(const CBoss_Armadillo_Cage& Prototype)
    : CMonsterPart(Prototype) {
}

HRESULT CBoss_Armadillo_Cage::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg))) return E_FAIL;
    if (FAILED(Ready_Components()))        return E_FAIL;

    m_iShadowPassIdx = 7;    

    m_pModelCom->Set_BindPose();
    if (m_pAnimatorCom) m_pAnimatorCom->Pause();

    Set_Active(false);             
    return S_OK;
}

void CBoss_Armadillo_Cage::Update(_float /*fTimeDelta*/)
{
}

HRESULT CBoss_Armadillo_Cage::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_AnimMesh_PBR;
    t.szModelProtoTag = MODEL_PROTO_TAG;
    t.bAnimated = true;
    return Ready_MeshPart(t);
}

CBoss_Armadillo_Cage* CBoss_Armadillo_Cage::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Armadillo_Cage* pInstance = new CBoss_Armadillo_Cage(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CBoss_Armadillo_Cage");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBoss_Armadillo_Cage* CBoss_Armadillo_Cage::Clone(void* pArg)
{
    CBoss_Armadillo_Cage* pInstance = new CBoss_Armadillo_Cage(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CBoss_Armadillo_Cage");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Armadillo_Cage::Free() { __super::Free(); }