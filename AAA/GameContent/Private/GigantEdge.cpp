#include "GigantEdge.h"
#include "GameInstance.h"

#include "GigantEdge_Body.h"
#include "GigantEdge_Sword.h"
#include "GigantEdge_Shield.h"
#include "GigantEdge_Brain.h"
#include "Animator.h"

CGigantEdge::CGigantEdge(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMiniBoss(pDevice, pContext) {
}
CGigantEdge::CGigantEdge(const CGigantEdge& Prototype)
    : CMiniBoss(Prototype) {
}

HRESULT CGigantEdge::Initialize_Prototype() { return S_OK; }

void CGigantEdge::Priority_Update(_float fTimeDelta) { __super::Priority_Update(fTimeDelta); }
void CGigantEdge::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }  
void CGigantEdge::Late_Update(_float fTimeDelta) { __super::Late_Update(fTimeDelta); }
HRESULT CGigantEdge::Render() { return S_OK; }
void CGigantEdge::On_Deserialized() { __super::On_Deserialized(); }


CMonsterBrain* CGigantEdge::Create_Brain()
{
    return CGigantEdge_Brain::Create();
}

void CGigantEdge::Play_Intro()
{
    m_pBody->Get_Animator()->Play("Intro", false, true);
}
_bool CGigantEdge::Is_Intro_Finished() const
{
    return m_pBody->Get_Animator()->Is_Finished();
}

CGigantEdge* CGigantEdge::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CGigantEdge* pInstance = new CGigantEdge(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CGigantEdge");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGigantEdge* CGigantEdge::Clone(void* pArg)
{
    CGigantEdge* pInstance = new CGigantEdge(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CGigantEdge");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CGigantEdge::Free()
{
    __super::Free();
}

void CGigantEdge::On_Hit(_fvector vAttackerPos, _float fDamage)
{
    if (!Is_Active()) return;           

    m_fCurHP -= fDamage;
    if (m_fCurHP <= 0.f) { m_fCurHP = 0.f; /* TODO: »ç¸Á Ã³¸® */ return; }

    if (m_bGuarding)
    {
        _vector vForward = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
        _vector vToAtk = XMVector3Normalize(vAttackerPos - m_pTransformCom->Get_State(STATE::POSITION));
        if (XMVectorGetX(XMVector3Dot(vForward, vToAtk)) < 0.f)   
        {
            m_bGuarding = false;
            m_bGroggyRequested = true;
        }
    }
}

HRESULT CGigantEdge::Ready_Parts()
{
    CGigantEdge_Body::GIGANTEDGE_BODY_DESC BodyDesc{};
    BodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    if (FAILED(Add_PartObject(m_iPrototypeLevel, CGigantEdge_Body::PROTOTYPE_TAG, CGigantEdge_Body::PART_TAG, &BodyDesc)))
        return E_FAIL;
    m_pBody = dynamic_cast<CGigantEdge_Body*>(m_PartObjects[CGigantEdge_Body::PART_TAG]);
    if (!m_pBody) return E_FAIL;

    CGigantEdge_Sword::GIGANTEDGE_SWORD_DESC SwordDesc{};
    SwordDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    SwordDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("RHaveL");
    if (FAILED(Add_PartObject(m_iPrototypeLevel, CGigantEdge_Sword::PROTOTYPE_TAG, CGigantEdge_Sword::PART_TAG, &SwordDesc)))
        return E_FAIL;
    m_pSword = dynamic_cast<CGigantEdge_Sword*>(m_PartObjects[CGigantEdge_Sword::PART_TAG]);

    CGigantEdge_Shield::GIGANTEDGE_SHIELD_DESC ShieldDesc{};
    ShieldDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    ShieldDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("LHaveL");
    if (FAILED(Add_PartObject(m_iPrototypeLevel, CGigantEdge_Shield::PROTOTYPE_TAG, CGigantEdge_Shield::PART_TAG, &ShieldDesc)))
        return E_FAIL;
    m_pShield = dynamic_cast<CGigantEdge_Shield*>(m_PartObjects[CGigantEdge_Shield::PART_TAG]);

    return S_OK;
}