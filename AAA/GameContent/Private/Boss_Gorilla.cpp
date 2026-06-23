#include "Boss_Gorilla.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "Boss_Gorilla_Brain.h"

#include "Boss_Gorilla_Body.h"
#include "Animator.h"

// 3페이즈: 66%, 33% 에서 전환 (PhaseCount = size()+1 = 3) Brain의 Get_PhaseCount와 일치!
const vector<_float> CBoss_Gorilla::s_Thresholds = { 0.5f };

CBoss_Gorilla::CBoss_Gorilla(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CBoss(pDevice, pContext) {
}
CBoss_Gorilla::CBoss_Gorilla(const CBoss_Gorilla& Prototype)
    : CBoss(Prototype) {
}

HRESULT CBoss_Gorilla::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBoss_Gorilla::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))   
        return E_FAIL;

    m_strBossName = L"고릴라 보스";
    m_fMaxHP = 1000.f;    
    m_fCurHP = m_fMaxHP;

    if (m_pMovement)
    {
        m_pMovement->Set_MoveSpeed(3.f);
        m_pMovement->Set_RotSpeed(90.f);
    }

    return S_OK;
}

void CBoss_Gorilla::Update(_float fTimeDelta)
{
#ifdef _DEBUG
    if (m_pGameInstance_Proxy->Is_EditMode())
    {
        if (m_pMovement) m_pMovement->Sync_To_Controller();
        return;
    }
#endif

    if (m_eLife == EBOSS_LIFE::HIDDEN)
        Appear();

    __super::Update(fTimeDelta);
}

void CBoss_Gorilla::On_Deserialized()
{
    __super::On_Deserialized();
    m_pTransformCom->Set_Scale(3.f, 3.f, 3.f);  // TODO: 튜닝
}

CMonsterBrain* CBoss_Gorilla::Create_Brain()
{
    return CBoss_Gorilla_Brain::Create();
}

void CBoss_Gorilla::Play_PhaseTransition(_int iNewPhase)
{
    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("Roar", false, true);
}

_bool CBoss_Gorilla::Is_PhaseTransition_Finished() const
{
    CAnimator* pAnim = Get_BodyAnimator();
    return pAnim ? pAnim->Is_Finished() : true;
}

void CBoss_Gorilla::On_PhaseChanged(_int iOldPhase, _int iNewPhase)
{
    OutputDebugStringA(("[Gorilla] Phase " + std::to_string(iOldPhase)
        + " -> " + std::to_string(iNewPhase) + "\n").c_str());
    // TODO: 전환 시 transient 상태 리셋(가드/속도 등) + 셰이더 파라미터 갱신
}

_bool CBoss_Gorilla::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
    Out.fRadius = s_fCCT_Radius + 0.1f;
    Out.fHeight = s_fCCT_Height + 0.1f;
    return true;
}

CAnimator* CBoss_Gorilla::Get_BodyAnimator() const
{
    return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

CMultiHitBoxPart* CBoss_Gorilla::Get_HitBoxPart() const
{
    return m_pBody;
}

HRESULT CBoss_Gorilla::Ready_PartObjects()
{
    m_pBody = Add_MonsterPart<CBoss_Gorilla_Body>(
        CBoss_Gorilla_Body::PROTOTYPE_TAG, CBoss_Gorilla_Body::PART_TAG);
    if (!m_pBody) return E_FAIL;
    return S_OK;
}

CBoss_Gorilla* CBoss_Gorilla::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Gorilla* pInstance = new CBoss_Gorilla(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CBoss_Gorilla");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBoss_Gorilla* CBoss_Gorilla::Clone(void* pArg)
{
    CBoss_Gorilla* pInstance = new CBoss_Gorilla(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CBoss_Gorilla");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Gorilla::Free()
{
    __super::Free();
}