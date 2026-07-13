#include "Boss_Armadillo.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "Boss_Armadillo_Brain.h"
#include "Boss_Armadillo_Body.h"
#include "Animator.h"
#include "Projectile_Partner.h"
#include "Projectile_Manager.h"

// 보스러쉬용: 페이즈 구분 없음 (Brain의 Get_PhaseCount = 1과 일치)
const vector<_float> CBoss_Armadillo::s_Thresholds = {};

CBoss_Armadillo::CBoss_Armadillo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CBoss(pDevice, pContext) {
}
CBoss_Armadillo::CBoss_Armadillo(const CBoss_Armadillo& Prototype)
    : CBoss(Prototype) {
}

HRESULT CBoss_Armadillo::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBoss_Armadillo::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_strBossName = L"아르마파라파";
    m_fMaxHP = 1000.f;
    m_fCurHP = m_fMaxHP;

    if (m_pMovement)
    {
        m_pMovement->Set_MoveSpeed(4.f);
        m_pMovement->Set_RotSpeed(120.f);
    }

    return S_OK;
}

void CBoss_Armadillo::Update(_float fTimeDelta)
{
#ifdef _DEBUG
    if (m_pGameInstance_Proxy->Is_EditMode())
    {
        if (m_pMovement) m_pMovement->Sync_To_Controller();
        return;
    }
    if (m_pGameInstance_Proxy->Key_Down(DIK_0))
        Appear();
    if (m_pGameInstance_Proxy->Key_Down(DIK_9))
        m_bDebugWallHit = true;
#endif
    __super::Update(fTimeDelta);
    Update_BodyOffset(fTimeDelta);
}

void CBoss_Armadillo::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

CMonsterBrain* CBoss_Armadillo::Create_Brain()
{
    return CBoss_Armadillo_Brain::Create(this);
}

void CBoss_Armadillo::Play_Intro()
{
    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("Angry", false, true, 0.f, 1.5f);
}

_bool CBoss_Armadillo::Is_Intro_Finished() const
{
    CAnimator* pAnim = Get_BodyAnimator();
    return pAnim ? pAnim->Is_Finished() : true;
}

void CBoss_Armadillo::Play_Death()
{
    Enable_Colliders(false);              // 메인보스: 즉시 콜라이더 off
    if (auto* p = Get_HitBoxPart())
        p->Enable_AllHitBoxes(false);

    if (m_pPartner) { m_pPartner->Despawn(); m_pPartner = nullptr; }

    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("DeathDamage", false, true, 0.f, 1.f);   // DeathWait/Death 클립도 있음. 연출 늘릴 때 사용
}

_bool CBoss_Armadillo::Is_Death_Finished() const
{
    CAnimator* pAnim = Get_BodyAnimator();
    return pAnim ? pAnim->Is_Finished() : true;
}

_bool CBoss_Armadillo::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
    Out.fRadius = s_fCCT_Radius + 0.1f;
    Out.fHeight = s_fCCT_Height + 0.1f;
    return true;
}

CAnimator* CBoss_Armadillo::Get_BodyAnimator() const
{
    return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

CMultiHitBoxPart* CBoss_Armadillo::Get_HitBoxPart() const
{
    return m_pBody;
}

_bool CBoss_Armadillo::Sweep_Wall(const _float3& vDir, _float fDist, _float3* pOutNormal) const
{
#ifdef _DEBUG
    if (m_bDebugWallHit)
    {
        m_bDebugWallHit = false;  
        if (pOutNormal)
        {
            _vector d = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(&vDir), 0.f));
            _float fJitter = XMConvertToRadians(((rand() % 81) - 40) * 1.f);
            XMStoreFloat3(pOutNormal, XMVector3Transform(-d, XMMatrixRotationY(fJitter)));
        }
        return true;
    }
#endif

    _float3 vPos;
    XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));
    vPos.y += s_fCCT_Height * 0.5f;   // 몸통 중심 높이에서 쏨

    _float3 vNormal{};
    if (!m_pGameInstance_Proxy->Sweep_Sphere(vPos, 2.5f, vDir, fDist, &vNormal))
        return false;

    if (fabsf(vNormal.y) >= 0.5f)     // 바닥/경사는 벽으로 안 침
        return false;

    if (pOutNormal) *pOutNormal = vNormal;
    return true;
}

void CBoss_Armadillo::Set_TwinDanceOffset(_bool bOn)
{
    if (!m_pBody) return;

    if (bOn)
    {
        const _float4x4* pBone = m_pBody->Get_BoneMatrixPtr("Partner1L");
        if (!pBone) return;
        m_vBodyOffsetTarget = _float3(-pBone->_41 * 0.5f, 0.f, -pBone->_43 * 0.5f);
    }
    else
        m_vBodyOffsetTarget = _float3(0.f, 0.f, 0.f);

    XMStoreFloat3(&m_vBodyOffsetStart, m_pBody->Get_Transform()->Get_State(STATE::POSITION));
    m_fBodyOffsetT = 0.f;
}

void CBoss_Armadillo::Summon_Partner()
{
    if (m_pPartner) return;

    CProjectile* p = nullptr;
    CProjectile_Manager::GetInstance()->Spawn(Get_PrototypeLevelIndex(), Get_LevelIndex(),
        CProjectile_Partner::POOL_KEY, CProjectile_Partner::PROTOTYPE_TAG, &p);
    if (!p) return;

    p->Attach_To_Socket(m_pBody->Get_BoneMatrixPtr("Partner1L"),
        m_pBody->Get_CombinedWorldMatrixPtr(), XMMatrixIdentity());
    m_pPartner = static_cast<CProjectile_Partner*>(p);
}

void CBoss_Armadillo::Fire_PartnerThrow()
{
    if (!m_pPartner) return;

    _vector vSelf = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vKirby = XMLoadFloat3(&Get_BlackBoard().vTargetPos);

    _vector vDir = XMVectorSetY(vKirby - vSelf, 0.f);
    if (XMVectorGetX(XMVector3LengthSq(vDir)) > 1e-6f)
        vDir = XMVector3Normalize(vDir);
    else
        vDir = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f));

    _vector vStart = vSelf + vDir * (s_fCCT_Radius + 1.5f);

    _float3 vS, vD;
    XMStoreFloat3(&vS, vStart);
    XMStoreFloat3(&vD, vDir);
    m_pPartner->Launch(vS, vD);
    m_pPartner = nullptr;
}

void CBoss_Armadillo::Enable_PartnerSpinHit(_bool b)
{
    if (m_pPartner) m_pPartner->Enable_SpinHitBox(b);
}

void CBoss_Armadillo::Play_PartnerAnim(const _char* szClip, _bool bLoop)
{
    if (m_pPartner) m_pPartner->Play_Anim(szClip, bLoop);
}

HRESULT CBoss_Armadillo::Ready_AnimEvents()
{
    CAnimator* pAnim = Get_BodyAnimator();
    if (!pAnim) return E_FAIL;

    pAnim->Set_EventCallback([this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase) {
        if (Handle_SharedAnimEvent(e, phase))
            return;

        switch (static_cast<EANIM_EVENT>(e.iEventType))
        {
            case EANIM_EVENT::Projectile:
            {
                if (phase != ANIM_EVENT_PHASE::POINT) break;

                if (e.iIntParam == 0) 
                    Summon_Partner();
                else                  
                    Fire_PartnerThrow();
                break;
            }
        }
        });
    return S_OK;
}

HRESULT CBoss_Armadillo::Ready_PartObjects()
{
    m_pBody = Add_MonsterPart<CBoss_Armadillo_Body>(
        CBoss_Armadillo_Body::PROTOTYPE_TAG, CBoss_Armadillo_Body::PART_TAG);
    if (!m_pBody) return E_FAIL;

    // 잡기 명중 판정
    m_pBody->Set_HitBox_OnEnter(CBoss_Armadillo_Body::AHB_CATCH,
        [this](CCollider*) { m_bCatchHit = true; });

    return S_OK;
}

void CBoss_Armadillo::Update_BodyOffset(_float fTimeDelta)
{
    if (!m_pBody || m_fBodyOffsetT >= 1.f)
        return;

    m_fBodyOffsetT = min(m_fBodyOffsetT + fTimeDelta / s_fBodyOffsetBlendTime, 1.f);

    _float t = m_fBodyOffsetT * m_fBodyOffsetT * (3.f - 2.f * m_fBodyOffsetT);

    _vector vPos = XMVectorLerp(XMLoadFloat3(&m_vBodyOffsetStart),
        XMLoadFloat3(&m_vBodyOffsetTarget), t);
    m_pBody->Get_Transform()->Set_State(STATE::POSITION, XMVectorSetW(vPos, 1.f));
}

CBoss_Armadillo* CBoss_Armadillo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Armadillo* pInstance = new CBoss_Armadillo(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CBoss_Armadillo");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBoss_Armadillo* CBoss_Armadillo::Clone(void* pArg)
{
    CBoss_Armadillo* pInstance = new CBoss_Armadillo(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CBoss_Armadillo");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Armadillo::Free()
{
    __super::Free();
}