#pragma once
#include "GameContent_Defines.h"
#include "BossBase.h"

NS_BEGIN(Client)

class CMultiHitBoxPart;

class CBoss abstract : public CBossBase
{
    GENERATED_BODY_ABSTRACT(CBoss)

protected:
    CBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss(const CBoss& Prototype);
    virtual ~CBoss() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual CMultiHitBoxPart* Get_HitBoxPart() const { return nullptr; }

    _int  Get_Phase() const { return m_iPhase; }
    _bool Is_PhaseTransitioning() const { return m_bPhaseTransition; }

protected:
    virtual void Update_AI(_float fTimeDelta) override;

    // 페이즈 정의 (구체 보스가 채움). 임계값은 HP비율 내림차순: 3페이즈면 {0.66f, 0.33f}
    virtual const vector<_float>& Get_PhaseThresholds() const = 0;
    virtual void  Play_PhaseTransition(_int iNewPhase) {}
    virtual _bool Is_PhaseTransition_Finished() const { return true; }
    virtual void  On_PhaseChanged(_int iOldPhase, _int iNewPhase) {}

    virtual void  On_Enter_Corpse() override;     // 보스 사망(흡입화 X) + Boss_Died
    void          Publish_Boss_Died();

private:
    void Check_PhaseTransition();

    _int  m_iPhase = { 0 };
    _bool m_bPhaseTransition = { false };

protected:
    virtual void Free() override;
};

NS_END