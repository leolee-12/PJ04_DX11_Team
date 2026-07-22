#include "Boss_Brain.h"
#include "Boss.h"
#include "GameInstance.h"
#include "Transform.h"
#include "BT.h"
#include "MultiHitBoxPart.h"

HRESULT CBoss_Brain::Initialize_Trees(CMonster* pOwner)
{
    if (pOwner == nullptr)
        return E_FAIL;

    m_pOwner = pOwner;

    for (_int i = 0; i < Get_PhaseCount(); ++i)
    {
        CBTNode* pRoot = Build_PhaseTree(i);
        if (nullptr == pRoot)
            return E_FAIL;

        CBehaviorTree* pBT = CBehaviorTree::Create(pRoot);
        if (nullptr == pBT)
            return E_FAIL;

        m_PhaseBTs.push_back(pBT);
    }
    return m_PhaseBTs.empty() ? E_FAIL : S_OK;
}

void CBoss_Brain::Decide(const MONSTER_BLACKBOARD& BB, _float fDt)
{
    if (m_pOwner == nullptr)
        return;

    _int iPhase = Owner()->Get_Phase();
    if (iPhase < 0 || iPhase >= static_cast<_int>(m_PhaseBTs.size()))
        return;

    CBehaviorTree* pBT = m_PhaseBTs[iPhase];
    auto* pBB = pBT->Get_Blackboard();

    pBB->Set<_float>("DistToTarget", BB.fDistToTarget);

    _vector vMyPos = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);
    _vector vTgt = XMLoadFloat3(&BB.vTargetPos);
    _vector vDiff = XMVectorSetY(vTgt - vMyPos, 0.f);
    _float3 vDir;  XMStoreFloat3(&vDir, XMVector3Normalize(vDiff));
    pBB->Set<_float3>("DirToTarget", vDir);

    pBT->Tick(fDt);
}

CBoss* CBoss_Brain::Owner() const
{
    return (m_pOwner != nullptr) ? static_cast<CBoss*>(m_pOwner) : nullptr;
}

CAnimator* CBoss_Brain::Anim() const { return m_pOwner->Get_BodyAnimator(); }

CBTNode* CBoss_Brain::Clip(const string& name, _float speed, _float blend)
{
    return CBTPlayClip::Create([this] { return Anim(); }, { name, false, 0.f, speed, blend });
}

CBTNode* CBoss_Brain::Loop(const string& name, _float hold, _float speed)
{
    return CBTPlayClip::Create([this] { return Anim(); }, { name, true, hold, speed });
}

_vector CBoss_Brain::Dir_ToTargetXZ() const
{
    _vector self = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);
    _vector tgt = XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos);
    _vector d = XMVectorSetY(XMVectorSubtract(tgt, self), 0.f);
    return (XMVectorGetX(XMVector3LengthSq(d)) < 1e-6f) ? XMVectorZero() : XMVector3Normalize(d);
}

_bool CBoss_Brain::IsFacing(_fvector dir, _float dot) const
{
    if (XMVector3Equal(dir, XMVectorZero())) return true;
    _vector look = XMVector3Normalize(XMVectorSetY(m_pOwner->Get_Transform()->Get_State(STATE::LOOK), 0.f));
    return XMVectorGetX(XMVector3Dot(look, dir)) >= dot;
}

void CBoss_Brain::Reset_Tree()
{
    _int iPhase = Owner()->Get_Phase();
    if (iPhase >= 0 && iPhase < static_cast<_int>(m_PhaseBTs.size()))
        m_PhaseBTs[iPhase]->Reset();
}

_bool CBoss_Brain::RotateYawTo(_fvector dir, _float degPerSec, _float dt)
{
    if (XMVector3Equal(dir, XMVectorZero())) return true;
    CTransform* tf = m_pOwner->Get_Transform();
    _vector look = XMVector3Normalize(XMVectorSetY(tf->Get_State(STATE::LOOK), 0.f));
    _float d = XMVectorGetX(XMVector3Dot(look, dir));
    _float c = XMVectorGetZ(look) * XMVectorGetX(dir) - XMVectorGetX(look) * XMVectorGetZ(dir);
    _float yaw = atan2f(c, d), step = XMConvertToRadians(degPerSec) * dt;
    _float ap = (fabsf(yaw) <= step) ? yaw : (yaw > 0.f ? step : -step);
    tf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), ap));
    return fabsf(yaw) <= step;
}

CBTNode* CBoss_Brain::FaceWindup(const string& clip, _float deg, _float speed)
{
    auto on = make_shared<bool>(false);
    return CBTAction::Create(
        [this, on, clip, deg, speed](CBlackboard*, _float dt) -> BT_STATUS {
            if (!*on) { Anim()->Play(clip, false, true, 0.2f, speed); *on = true; }
            RotateYawTo(Dir_ToTargetXZ(), deg, dt);
            if (Anim()->Is_Finished()) { *on = false; return BT_STATUS::SUCCESS; }
            return BT_STATUS::RUNNING;
        }, [on] { *on = false; });
}

CBTNode* CBoss_Brain::SetHitBox(_int idx, _bool on)
{
    return CBTAction::Create([this, idx, on](CBlackboard*, _float) {
        if (auto* p = Owner()->Get_HitBoxPart()) p->Enable_HitBox(idx, on);
        return BT_STATUS::SUCCESS; });
}

void CBoss_Brain::Enable_Hit(_int idx, _bool on)
{
    if (auto* p = Owner()->Get_HitBoxPart()) p->Enable_HitBox(idx, on);
}

void CBoss_Brain::Free()
{
    for (auto* pBT : m_PhaseBTs)
        Safe_Release(pBT);
    m_PhaseBTs.clear();

    __super::Free();
}