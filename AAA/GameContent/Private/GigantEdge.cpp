#include "GigantEdge.h"
#include "GameInstance.h"

#include "GigantEdge_Body.h"
#include "GigantEdge_Sword.h"
#include "GigantEdge_Shield.h"
#include "GigantEdge_Brain.h"
#include "Animator.h"
#include "Monster_Movement.h"

CGigantEdge::CGigantEdge(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMiniBoss(pDevice, pContext) {
}
CGigantEdge::CGigantEdge(const CGigantEdge& Prototype)
    : CMiniBoss(Prototype) {
}

HRESULT CGigantEdge::Initialize_Prototype() 
{ 
    return S_OK;
}

HRESULT CGigantEdge::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_eCopyAbility = COPY_ABILITY_TYPE::SWORD;
    m_pMovement->Set_MoveSpeed(4.f);

    return S_OK;
}

void CGigantEdge::Update(_float fTimeDelta) 
{ 
#ifdef _DEBUG
    if (m_pGameInstance_Proxy->Is_EditMode())
    {
        m_pMovement->Sync_To_Controller();
        return;
    }

    Debug_KeyInput();
#endif

    __super::Update(fTimeDelta); 
}  

void CGigantEdge::On_Deserialized() 
{ 
    __super::On_Deserialized(); 
}

CMonsterBrain* CGigantEdge::Create_Brain()
{
    return CGigantEdge_Brain::Create();
}

void CGigantEdge::Play_Intro()
{
    CAnimator* pAni = m_pBody->Get_Animator();

    CAnimator::ANI_PLAY_INFO info{};
    info.strAniName = "DemoAppear1";
    info.bLoop = false;
    pAni->Play(&info);

    info.strAniName = "DemoAppear2";
    info.bLoop = false;
    pAni->Enqueue(info);
}
_bool CGigantEdge::Is_Intro_Finished() const
{
    return m_pBody->Get_Animator()->Is_Finished();
}

void CGigantEdge::Play_Death()
{
    m_pBody->Get_Animator()->Play("DeathStart", false, true, 0.2f, 2.f);
}

void CGigantEdge::Play_DeathLoop()
{
    m_pBody->Get_Animator()->Play("DeathEndWait", true, true);
}

_bool CGigantEdge::Is_Death_Finished() const
{
    return m_pBody->Get_Animator()->Is_Finished();
}

_bool CGigantEdge::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
    Out.fRadius = { s_fCCT_Radius + 0.1f };
    Out.fHeight = { s_fCCT_Height + 0.1f };
    return true;
}

CGameObject* CMiniBoss::Find_Player() const
{
    PLAYER_QUERY q;
    m_pGameInstance_Proxy->Publish(EVT_QUERY_PLAYER, &q);
    return q.pPlayer;
}

_bool CGigantEdge::Block_Hit(const ATTACK_INFO& tInfo)
{
    if (!m_bGuarding) return false;

    _vector vForward = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
    _vector vToAtk = XMVector3Normalize(XMLoadFloat3(&tInfo.vAttackerPos) - m_pTransformCom->Get_State(STATE::POSITION));
    _bool   bFromBack = (XMVectorGetX(XMVector3Dot(vForward, vToAtk)) < 0.f);

    if (bFromBack) {              
        m_bGuarding = false;
        m_bGroggyRequested = true;
        return false;
    }
    return true;
}

#ifdef _DEBUG
void CGigantEdge::Debug_KeyInput()
{
    if (nullptr == m_pGameInstance_Proxy)
        return;

    if (m_pGameInstance_Proxy->Key_Down(DIK_H) && m_pSword)
    {
        m_bDbgSwordDrawn = !m_bDbgSwordDrawn;
        m_pSword->Set_Drawn(m_bDbgSwordDrawn);
    }
    if (m_pGameInstance_Proxy->Key_Down(DIK_0)) Appear();                          // HIDDEN→INTRO→ACTIVE
    //if (m_pGameInstance_Proxy->Key_Down(DIK_G)) m_bGroggyRequested = true;         // 그로기 분기 진입
    //if (m_pGameInstance_Proxy->Key_Down(DIK_R)) m_bDbgInRange = !m_bDbgInRange;    // 사거리 토글 (공격↔추격)
    //if (m_pGameInstance_Proxy->Key_Down(DIK_M)) m_bDbgWalkInPlace = !m_bDbgWalkInPlace;
    if (m_pGameInstance_Proxy->Key_Down(DIK_X)) Die();
    //if (m_pGameInstance_Proxy->Key_Down(DIK_1)) m_iDbgAttack = 0;                  // Slam 고정
    //if (m_pGameInstance_Proxy->Key_Down(DIK_2)) m_iDbgAttack = 1;                  // Charge 고정
    //if (m_pGameInstance_Proxy->Key_Down(DIK_3)) m_iDbgAttack = 2;                  // Swing 고정
    //if (m_pGameInstance_Proxy->Key_Down(DIK_4)) m_iDbgAttack = -1;                 // 랜덤 복귀
}
#endif

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

HRESULT CGigantEdge::Ready_Parts()
{
    m_pBody = Add_MonsterPart<CGigantEdge_Body>(
        CGigantEdge_Body::PROTOTYPE_TAG, CGigantEdge_Body::PART_TAG);
    if (!m_pBody) return E_FAIL;

    m_pSword = Add_MonsterPart<CGigantEdge_Sword>(
        CGigantEdge_Sword::PROTOTYPE_TAG, CGigantEdge_Sword::PART_TAG,
        m_pBody->Get_BoneMatrixPtr("RHaveL"));
    if (!m_pSword) return E_FAIL;

    m_pShield = Add_MonsterPart<CGigantEdge_Shield>(
        CGigantEdge_Shield::PROTOTYPE_TAG, CGigantEdge_Shield::PART_TAG,
        m_pBody->Get_BoneMatrixPtr("LHaveL"));
    if (!m_pShield) return E_FAIL;

    return S_OK;
}