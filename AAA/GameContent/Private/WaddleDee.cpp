#include "WaddleDee.h"
#include "WaddleDee_Body.h"
#include "LevelDesign_LoadTypes.h"
#include "GameContrnt_Events.h"

#include "GameInstance.h"
#include "Parsing_Utils.h"

namespace
{
    constexpr const _char* s_szGreetClip = "WaveHand";
    constexpr _ubyte s_byInteractKey = DIK_F;
    constexpr _float s_fGreetCooldown = 1.5f;
}

CWaddleDee::CWaddleDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCharacter(pDevice, pContext)
{
    m_strFixedAnim = L"";
    m_fInteractRadius = 2.f;
}

CWaddleDee::CWaddleDee(const CWaddleDee& Prototype)
    : CCharacter(Prototype)
    , m_strFixedAnim(Prototype.m_strFixedAnim)
    , m_fInteractRadius(Prototype.m_fInteractRadius)
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

    const _bool bEditMode = m_pGameInstance_Proxy->Is_EditMode();

    if (!bEditMode && m_fGreetCooldown > 0.f)
        m_fGreetCooldown = max(0.f, m_fGreetCooldown - fTimeDelta);

    switch (m_eState)
    {
    case WADDLEDEE_STATE::IDLE:
        Update_Idle();
        if (!bEditMode)
            Check_Interact();
        break;

    case WADDLEDEE_STATE::GREET:
        if (!bEditMode)
            Update_Greet();
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

    m_fGreetCooldown = 0.f;
    m_pPlayer = nullptr;
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

    case WADDLEDEE_STATE::GREET:
        m_pBody->Get_Animator()->Play(s_szGreetClip, false, true);
        break;
    }
}

void CWaddleDee::Check_Interact()
{
    if (m_fGreetCooldown > 0.f || m_fInteractRadius <= 0.f || !Find_Player())
        return;

    _vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vPlayerPosition = m_pPlayer->Get_Transform()->Get_State(STATE::POSITION);
    _vector vToPlayer = XMVectorSetY(vPlayerPosition - vPosition, 0.f);
    const _float fDistanceSq = XMVectorGetX(XMVector3LengthSq(vToPlayer));

    if (fDistanceSq > m_fInteractRadius * m_fInteractRadius)
        return;

    if (!m_pGameInstance_Proxy->Key_Down(s_byInteractKey))
        return;

    if (fDistanceSq > FLT_EPSILON)
        m_pTransformCom->LookAt(vPosition + vToPlayer);

    Change_State(WADDLEDEE_STATE::GREET);
}

_bool CWaddleDee::Find_Player()
{
    if (nullptr != m_pPlayer)
        return true;

    PLAYER_QUERY PlayerQuery{};
    m_pGameInstance_Proxy->Publish(EventTag::Query_Player, &PlayerQuery);
    m_pPlayer = PlayerQuery.pPlayer;

    return nullptr != m_pPlayer;
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

void CWaddleDee::Update_Greet()
{
    if (!m_pBody->Get_Animator()->Is_Finished())
        return;

    m_fGreetCooldown = s_fGreetCooldown;
    Change_State(WADDLEDEE_STATE::IDLE);
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