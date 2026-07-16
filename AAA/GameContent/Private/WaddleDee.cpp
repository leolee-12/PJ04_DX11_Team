#include "WaddleDee.h"
#include "WaddleDee_Body.h"
#include "LevelDesign_LoadTypes.h"

#include "Animator.h"
#include "Parsing_Utils.h"

CWaddleDee::CWaddleDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCharacter(pDevice, pContext)
{
    m_strFixedAnim = L"";
}

CWaddleDee::CWaddleDee(const CWaddleDee& Prototype)
    : CCharacter(Prototype)
    , m_strFixedAnim(Prototype.m_strFixedAnim)
{
}

HRESULT CWaddleDee::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (nullptr != pArg)
    {
        const LD_OBJECT_DESC* pDesc = static_cast<const LD_OBJECT_DESC*>(pArg);
        if (!pDesc->strAIVariation.empty())
            m_strFixedAnim = pDesc->strAIVariation;
    }

    if (FAILED(Ready_PartObjects()))
        return E_FAIL;

    Change_State(WADDLEDEE_STATE::IDLE);
    return S_OK;
}

void CWaddleDee::Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Update(fTimeDelta);

    switch (m_eState)
    {
    case WADDLEDEE_STATE::IDLE:
        Update_Idle();
        break;
    }
}

void CWaddleDee::Late_Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Late_Update(fTimeDelta);
}

void CWaddleDee::On_Deserialized()
{
    __super::On_Deserialized();

    m_strAppliedFixedAnim.clear();
    Change_State(WADDLEDEE_STATE::IDLE);
}

HRESULT CWaddleDee::Ready_PartObjects()
{
    CWaddleDee_Body::WADDLEDEE_BODY_DESC BodyDesc{};
    BodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CWaddleDee_Body::PROTOTYPE_TAG, CWaddleDee_Body::PART_TAG, &BodyDesc)))
        return E_FAIL;

    auto iter = m_PartObjects.find(CWaddleDee_Body::PART_TAG);
    if (iter == m_PartObjects.end())
        return E_FAIL;

    m_pBody = dynamic_cast<CWaddleDee_Body*>(iter->second);
    if (nullptr == m_pBody)
        return E_FAIL;

    return S_OK;
}

void CWaddleDee::Change_State(WADDLEDEE_STATE eState)
{
    m_eState = eState;

    switch (m_eState)
    {
    case WADDLEDEE_STATE::IDLE:
        Play_Idle();
        break;
    }
}

void CWaddleDee::Update_Idle()
{
    if (m_strAppliedFixedAnim != m_strFixedAnim)
        Play_Idle();
}

void CWaddleDee::Play_Idle()
{
    const _string strClip = m_strFixedAnim.empty() ? "Wait" : WstrToStr(m_strFixedAnim);

    m_pBody->Get_Animator()->Play(strClip, true, true, 0.f);
    m_strAppliedFixedAnim = m_strFixedAnim;
}

CWaddleDee* CWaddleDee::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CWaddleDee* pInstance = new CWaddleDee(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CWaddleDee");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CWaddleDee::Clone(void* pArg)
{
    CWaddleDee* pInstance = new CWaddleDee(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CWaddleDee");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CWaddleDee::Free()
{
    __super::Free();
}