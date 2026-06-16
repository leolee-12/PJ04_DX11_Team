#include "BladeKnight.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "Animator.h"

#include "BladeKnight_Body.h"
#include "BladeKnight_Sword.h"

CBladeKnight::CBladeKnight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonster{ pDevice, pContext }
{
}

CBladeKnight::CBladeKnight(const CBladeKnight& Prototype)
    : CMonster( Prototype )
{

}

HRESULT CBladeKnight::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CBladeKnight::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_PartObjects()))
        return E_FAIL;

    if (FAILED(Ready_Movement()))
        return E_FAIL;

    if (FAILED(Ready_AI()))
        return E_FAIL;

    return S_OK;
}

void CBladeKnight::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Priority_Update(fTimeDelta);
}

void CBladeKnight::Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Update(fTimeDelta);

#ifdef _DEBUG
    if (nullptr != m_pGameInstance_Proxy)
    {
        if (m_pGameInstance_Proxy->Key_Down(DIK_1))
            Change_State(MONSTER_STATE_TYPE::IDLE);

        if (m_pGameInstance_Proxy->Key_Down(DIK_2))
            Change_State(MONSTER_STATE_TYPE::CHASE);

        if (m_pGameInstance_Proxy->Key_Down(DIK_3))
            Change_State(MONSTER_STATE_TYPE::RETREAT);

        if (m_pGameInstance_Proxy->Key_Down(DIK_4))
            Change_State(MONSTER_STATE_TYPE::ATTACK);
    }
#endif
}

void CBladeKnight::Late_Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Late_Update(fTimeDelta);
}

HRESULT CBladeKnight::Render()
{
    return S_OK;
}

_float CBladeKnight::Get_CapsuleRadius() const
{
    return 0.5f;
}

_float CBladeKnight::Get_CapsuleHeight() const
{
    return 1.0f;
}

void CBladeKnight::Play_StateAnimation(MONSTER_STATE_TYPE eState)
{
    if (nullptr == m_pBody || nullptr == m_pSword || nullptr == m_pMovement)
        return;

    CAnimator* pBodyAnimator = m_pBody->Get_Animator();
    if (pBodyAnimator == nullptr)
        return;

    CAnimator* pSwordAnimator = m_pSword->Get_Animator();
    if (pSwordAnimator == nullptr)
        return;

    CAnimator::ANI_PLAY_INFO AnimInfo{};

    switch (eState)
    {
    case MONSTER_STATE_TYPE::IDLE:
    {
        // IDLE :: FindWait 모션
        AnimInfo.strAniName = "FindWait";
        AnimInfo.bLoop = true;
        pBodyAnimator->Play(&AnimInfo);
        //pSwordAnimator->Play("Thrust", false, false);
        break;
    }
    case MONSTER_STATE_TYPE::CHASE:
    {
        m_pMovement->Set_MoveSpeed(2.f);
        pBodyAnimator->Play("Move", true, false);
        //pSwordAnimator->Play("Thrust", false, false);
        break;
    }

    case MONSTER_STATE_TYPE::ATTACK:
    {
        // 칼 애니메이션 상태 세팅
        //pSwordAnimator->Play("Thrust", false, false);

        AnimInfo.strAniName = "AttackStart";
        AnimInfo.bLoop = false;
        pBodyAnimator->Play(&AnimInfo);

        AnimInfo.strAniName = "Attack";
        AnimInfo.bLoop = false;
        pBodyAnimator->Enqueue(AnimInfo);
        break;
    }
    case MONSTER_STATE_TYPE::RETREAT:
    {
        m_pMovement->Set_MoveSpeed(2.f);
        AnimInfo.strAniName = "Retreat";
        AnimInfo.bLoop = false;
        pBodyAnimator->Play(&AnimInfo);
        break;
    }

    default:
    {
        m_pBody->Get_Animator()->Play("Wait", true, false);
        pSwordAnimator->Play("Thrust", false, false);
    }
        break;
    }
}

_bool CBladeKnight::Is_StateAnimationFinished() const
{
    if (nullptr == m_pBody)
        return true;      // 확인 불가면 끝난 걸로 간주해서 true를 반환 ( false로 반환 하게 해두면 해당 상태 못빠져나감 )

    CAnimator* pAnim = m_pBody->Get_Animator();

    return (nullptr == pAnim) ? true : pAnim->Is_Finished();
}

HRESULT CBladeKnight::Ready_PartObjects()
{
    // Body
    CBladeKnight_Body::BLADEKNIGHT_BODY_DESC BodyDesc{};
    BodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CBladeKnight_Body::PROTOTYPE_TAG, TEXT("Body"), &BodyDesc)))
        return E_FAIL;

    m_pBody = dynamic_cast<CBladeKnight_Body*>(m_PartObjects[TEXT("Body")]);
    if (nullptr == m_pBody)
        return E_FAIL;

    // Sword
    CBladeKnight_Sword::BLADEKNIGHT_SWORD_DESC SwordDesc{};
    SwordDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    SwordDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("RHaveL");

    if (nullptr == SwordDesc.pSocketBoneMatrix)
        return E_FAIL;

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CBladeKnight_Sword::PROTOTYPE_TAG, CBladeKnight_Sword::PART_TAG, &SwordDesc)))
        return E_FAIL;

    m_pSword = dynamic_cast<CBladeKnight_Sword*>(m_PartObjects[CBladeKnight_Sword::PART_TAG]);
    if (nullptr == m_pSword)
        return E_FAIL;

    return S_OK;
}

HRESULT	CBladeKnight::Bind_ShaderResources()
{
    return S_OK;
}


void CBladeKnight::On_Deserialized()
{
    __super::On_Deserialized();
}

CBladeKnight* CBladeKnight::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBladeKnight* pInstance = new CBladeKnight(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CBladeKnight");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CBladeKnight::Clone(void* pArg)
{
    CBladeKnight* pInstance = new CBladeKnight(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CBladeKnight");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CBladeKnight::Free()
{
	__super::Free();
}