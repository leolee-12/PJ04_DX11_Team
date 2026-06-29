#pragma once
#include "Boss.h"

NS_BEGIN(Client)

class CBoss_Gorilla_Body;
class CProjectile;

class CBoss_Gorilla final : public CBoss
{
    GENERATED_BODY(CBoss_Gorilla)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Gorilla";

    static constexpr _float s_fCCT_Radius = 4.f;
    static constexpr _float s_fCCT_Height = 8.f;

    static constexpr const _char* GRAB_BONE = "RHaveL";
    static constexpr const _char* THROW_BONE = "RHaveL";
    static constexpr _float DEATH_PAUSE_SEC = 0.7f;
    static constexpr _float DEATH_SHAKE_SEC = 0.7f;
    static constexpr const _char* s_Intro[] = {
          "CatchSuccessB", "CatchSuccessWait", "CatchRelease", "Roar"
    };

private:
    CBoss_Gorilla(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss_Gorilla(const CBoss_Gorilla& Prototype);
    virtual ~CBoss_Gorilla() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

public:
    virtual CAnimator* Get_BodyAnimator() const override;
    virtual CMultiHitBoxPart* Get_HitBoxPart() const override;

protected:
    virtual CMonsterBrain* Create_Brain() override;
    virtual void           Play_Intro() override;
    virtual _bool          Is_Intro_Finished() const override;
    virtual void           Play_Death() override;
    virtual _bool          Is_Death_Finished() const override;
    virtual _float         Get_CorpseLinger() const override { return 0.f; }

    virtual const vector<_float>& Get_PhaseThresholds() const override { return s_Thresholds; }
    virtual void           Play_PhaseTransition(_int iNewPhase) override;
    virtual _bool          Is_PhaseTransition_Finished() const override;
    virtual void           On_PhaseChanged(_int iOldPhase, _int iNewPhase) override;

    virtual _float Get_CapsuleRadius() const override { return s_fCCT_Radius; }
    virtual _float Get_CapsuleHeight() const override { return s_fCCT_Height; }
    virtual _float Get_InteractRadius() const override { return 0.f; }
    virtual _bool  Get_HurtBoxDesc(CAPSULE_DESC& Out) const override;
    virtual _bool  Is_Touch_Harmful() const override { return false; }

    virtual const _tchar*  Get_AppearEventTag() const override { return EventTag::Cutscene_GorillaHandoff; }
    virtual HRESULT        Ready_AnimEvents() override;                       

    virtual HRESULT Ready_PartObjects() override;

private:
    CBoss_Gorilla_Body* m_pBody = { nullptr };
    CProjectile* m_pHeldRock = { nullptr };
    static const vector<_float> s_Thresholds;

    _int  m_iIntroStep = { -1 };
    _bool m_bIntroDone = { false };
    _float m_fFreezeTimer = { 0.f };

    enum class EDEATH { POSE_WAIT, PAUSING, PLAYING };

    _bool   m_bDeathSeq = { false };
    EDEATH  m_eDeathStep = { EDEATH::POSE_WAIT };
    _int    m_iDeathPoseDelay = { 0 };
    _float  m_fDeathPauseTimer = { 0.f };

private:
    void Tick_OpeningCatch();                    
    void Fire_CatchCamera(const _tchar* szTrack);
    void Begin_AnimFreeze(_float fSeconds);
    void Fire_Grab();
    void Tick_DeathSequence(_float fTimeDelta);

public:
    static CBoss_Gorilla* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CBoss_Gorilla* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END