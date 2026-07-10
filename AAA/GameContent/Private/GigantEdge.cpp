#include "GigantEdge.h"
#include "GameInstance.h"

#include "GigantEdge_Body.h"
#include "GigantEdge_Sword.h"
#include "GigantEdge_Shield.h"
#include "GigantEdge_Brain.h"
#include "Animator.h"
#include "Monster_Movement.h"
#include "GameContrnt_Events.h"

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

    m_strBossName = L"±â°£Æ® ¿§Áö";

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

CMonsterBrain* CGigantEdge::Create_Brain()
{
    return CGigantEdge_Brain::Create(this);
}

CAnimator* CGigantEdge::Get_BodyAnimator() const
{
    return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

void CGigantEdge::Play_Intro()
{
    CAnimator* pAni = m_pBody->Get_Animator();

    CAnimator::ANI_PLAY_INFO info{};
    info.strAniName = "DemoAppear1";
    info.bLoop = false;
    info.fBlend = 0.f;
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

void CGigantEdge::On_Corpse_End()
{
    __super::On_Corpse_End();
    m_pGameInstance_Proxy->Publish(EventTag::Cage_Descend, nullptr);
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

void CGigantEdge::On_Death_Reaction(const ATTACK_INFO& tInfo)
{
    if (m_pSword)
        m_pSword->Set_HitBox(false);
}

#ifdef _DEBUG
void CGigantEdge::Debug_KeyInput()
{
    if (nullptr == m_pGameInstance_Proxy)
        return;

    if (m_pGameInstance_Proxy->Key_Down(DIK_0)) 
        Appear();                         
    if (m_pGameInstance_Proxy->Key_Down(DIK_9)) Die();                            
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

void CGigantEdge::On_Swallowed()
{
    __super::On_Swallowed();
    m_pGameInstance_Proxy->Publish(EventTag::Cage_Descend, nullptr);
}

HRESULT CGigantEdge::Ready_PartObjects()
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

HRESULT CGigantEdge::Ready_AnimEvents()
{
    CAnimator* pAnim = m_pBody->Get_Animator();
    if (!pAnim)
        return E_FAIL;

    CAnimator::EventCallback CallBack = [this](const ANIM_EVENT& Event, ANIM_EVENT_PHASE Phase)
        {
            OutputDebugStringA(("[AnimEvt] clip=" + m_pBody->Get_Animator()->Get_CurrentAnimName()
                + " type=" + std::to_string(Event.iEventType)
                + " p=" + std::to_string(m_pBody->Get_Animator()->Get_Progress()) + "\n").c_str());

            EANIM_EVENT eType = static_cast<EANIM_EVENT>(Event.iEventType);
            switch (eType)
            {
                case EANIM_EVENT::OnOffPart:
                {
                    switch (Phase)
                    {
                        case ANIM_EVENT_PHASE::POINT:
                        {
                            switch (Event.iIntParam)
                            {
                                case 0:
                                    m_pSword->Set_Drawn(true);
                                    break;
                                case 1:
                                {
                                    m_pSword->Set_Drawn(false);
                                    m_pShield->Set_Drawn(true);
                                    break;
                                }
                                case 2:
                                {
                                    m_pShield->Set_Drawn(false);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        };
    pAnim->Set_EventCallback(CallBack);

    return S_OK;
}
