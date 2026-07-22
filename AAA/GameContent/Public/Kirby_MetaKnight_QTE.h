#pragma once
#include "Kirby_State.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;
class CKirby_Body;
class CBoss_Metaknight;

class CLIENT_DLL CKirby_MetaKnight_QTE final : public CKirby_State
{
private:
    enum class METAKNIGHT_QTE_STATE { QTE, SUCCESS, FAIL, METANIGHT_QTE_STATE_END };

private:
    CKirby_MetaKnight_QTE();
    virtual ~CKirby_MetaKnight_QTE() = default;

private:
    HRESULT Initialize();

public:
    virtual KIRBY_STATE_TYPE Get_StateType() override;

    virtual void Enter(CKirby* pKirby, _int iFlag) override;
    virtual void Update(CKirby* pKirby, const _float fTimeDelta) override;
    virtual void Exit(CKirby* pKirby) override;

    virtual _bool Handle_Command(CKirby* pKirby, CKirby_Command* pCommand) override;

    virtual void On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo) override;

    virtual void Request_PositionSync(CKirby* pKirby, const KIRBY_POSITION_SYNC_BEGIN_DESC* pDesc) override;
    virtual void Request_PositionSync_End(CKirby* pKirby, const KIRBY_POSITION_SYNC_END_DESC* pDesc) override;

    // MetaKnight
    virtual void Request_MetaKnight_ParryBegin(CKirby* pKirby) override;

private:
    void Change_MetaKnightState(CKirby* pKirby, METAKNIGHT_QTE_STATE eNext);
    void Enter_MetaKnightState(CKirby* pKirby, METAKNIGHT_QTE_STATE eState);
    void Update_MetaKnightState(CKirby* pKirby, _float fTimeDelta);
    void Exit_MetaKnightState(CKirby* pKirby, METAKNIGHT_QTE_STATE eState);

private:
    METAKNIGHT_QTE_STATE m_eMetaKnightQTEState{ METAKNIGHT_QTE_STATE::METANIGHT_QTE_STATE_END };

    _float m_fQTEProgress{};
    _float m_fQTEAnimationProgress{};
    _bool m_bQTEStarted{};

    CBoss_Metaknight* m_pMetaKnight{};
    SUBHANDLE m_tQTEShowHandle{};

public:
    static CKirby_MetaKnight_QTE* Create();
private:
    virtual void Free() override;
};

NS_END