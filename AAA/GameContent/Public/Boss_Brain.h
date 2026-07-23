#pragma once
#include "Monster_Brain.h"

NS_BEGIN(Engine)
class CBehaviorTree;
class CBTNode;
NS_END

NS_BEGIN(Client)
class CBoss;

class CBoss_Brain abstract : public CMonsterBrain
{
protected:
    CBoss_Brain() = default;
    virtual ~CBoss_Brain() = default;

    HRESULT Initialize_Trees(CMonster* pOwner);

public:
    virtual void Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;
    void         Reset_Tree();
    CBoss*          Owner() const;

protected:
    CAnimator* Anim() const;
    CBTNode* Clip(const string& name, _float speed, _float blend = 0.2f);   
    CBTNode* Loop(const string& name, _float hold, _float speed);           
    CBTNode* FaceWindup(const string& clip, _float degPerSec, _float speed);
    CBTNode* SetHitBox(_int idx, _bool on);
    void     Enable_Hit(_int idx, _bool on);

    _vector Dir_ToTargetXZ() const;
    _bool   RotateYawTo(_fvector vDirXZ, _float degPerSec, _float dt);
    _bool   IsFacing(_fvector vDirXZ, _float dot) const;

protected:
    virtual _int     Get_PhaseCount() const = 0;
    virtual CBTNode* Build_PhaseTree(_int iPhase) = 0;

protected:
    vector<CBehaviorTree*> m_PhaseBTs;

    virtual void Free() override;
};
NS_END