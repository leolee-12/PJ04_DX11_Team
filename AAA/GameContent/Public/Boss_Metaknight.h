#pragma once
#include "Boss.h"

NS_BEGIN(Client)

class CBoss_Metaknight_Body;
class CBoss_Metaknight_Sword;
class CBoss_Metaknight_ReplicaSword;
class CBoss_Metaknight_Mant;
class CBoss_Metaknight_EscapeMant;
class CAttackDecal;

class CBoss_Metaknight final : public CBoss
{
    GENERATED_BODY(CBoss_Metaknight)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Metaknight";

    static constexpr const _tchar* APPEAR_TAG = TEXT("Metaknight_Appear");

    // --- 공통 ---
    static constexpr _float s_fDefaultAnimSpeed = 1.5f;
    static constexpr _float s_fCCT_Radius = 0.75f;
    static constexpr _float s_fCCT_Height = 0.1f;
    static constexpr const _char* WEAPON_BONE = "RHaveL";

    // --- 데모 컷씬 ---
    static constexpr _float DEMO_ORIGIN_Y = 7.23f;
    static constexpr const _char* DEMO_ROOT_BONE = "TopL";

    // --- 락킹 QTE ---
    static constexpr _float LOCK_CAM_DELAY = 0.5f;
    static constexpr _float LOCK_TIMEOUT = 10.f;
    static constexpr _float LOCK_GAUGE_START = 0.2f;
    static constexpr _float LOCK_GAUGE_WIN = 0.9f;
    static constexpr _float LOCK_GAUGE_LOSE = 0.005f;
    static constexpr _float ATTACH_YAW_OFFSET = 180.f;
    static constexpr const _tchar* LOCK_CAM_TRACK = TEXT("Metaknight_LockingSword");

    // 인트로
    static constexpr _float APPEAR_DELAY = 1.5f;
    static constexpr _float INTRO_HOLD_TIME = 2.f;

    // 잡기/패링
    static constexpr _float PARRY_HURT_RADIUS = 2.2f;

    // 기가문샷
    static constexpr _uint  GIGA_POINT_COUNT = 4;
    static const _float3    s_vGigaPoints[GIGA_POINT_COUNT];

    // 낙석
    static constexpr _float ROCK_DECAL_RADIUS = 5.f;
    static constexpr _float ROCK_SLIDE_TIME = 1.f;
    static constexpr _float ROCK_DROP_HEIGHT = 20.f;

    static constexpr _float TOPVIEW_HEIGHT = 40.f;

    // 페이즈 전환
    static constexpr _float PHASE_HOP_HEIGHT = 2.f;
    static constexpr _float PHASE_HOP_GRAVITY = 9.f;
    static constexpr _float PHASE_WAIT_BLEND = 0.4f;

    // 쿨다운
    static constexpr _float s_fDodgeCooldown = 4.f;
    static constexpr _float s_fGigaCooldown = 30.f;
    static constexpr _float s_fRockCooldown = 60.f;
    //static constexpr _float s_fUpperCooldown = 90.f;
    static constexpr _float s_fUpperCooldown = 3.f;

    // 사망
    static constexpr _float DEATH_PAUSE_SEC = 0.7f;
    static constexpr _float DEATH_SHAKE_SEC = 0.7f;
    static constexpr _float DEATH_MASK_HOLD = 0.5f;

    enum class EMK_SWORD { GALAXIA, REPLICA, NONE };
    enum class ELockOutcome { NONE, MK_WIN, MK_LOSE };
    enum class EDemo { UPPER_CALIBUR, LOCK_LOSE };

private:
    CBoss_Metaknight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss_Metaknight(const CBoss_Metaknight& Prototype);
    virtual ~CBoss_Metaknight() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

public:
    virtual CAnimator* Get_BodyAnimator() const override;
    virtual CMultiHitBoxPart* Get_HitBoxPart() const override;

protected:
    virtual CMonsterBrain* Create_Brain() override;
    virtual void           Play_Intro() override;
    virtual _bool          Is_Intro_Finished() const override;
    virtual void           On_Intro_End() override;
    virtual void           Play_Death() override;
    virtual _bool          Is_Death_Finished() const override;
    virtual void           On_Enter_Corpse() override;
    virtual _float         Get_CorpseLinger() const override { return 0.f; }
    virtual void           Play_PhaseTransition(_int iNewPhase) override;
    virtual _bool          Is_PhaseTransition_Finished() const override;
    virtual void           On_PhaseChanged(_int iOldPhase, _int iNewPhase) override;

    virtual const vector<_float>& Get_PhaseThresholds() const override { return s_Thresholds; }
    virtual const _tchar* Get_AppearEventTag() const override { return nullptr; }

    virtual _float Get_CapsuleRadius() const override { return s_fCCT_Radius; }
    virtual _float Get_CapsuleHeight() const override { return s_fCCT_Height; }
    virtual _float Get_InteractRadius() const override { return 0.f; }
    virtual _bool  Get_HurtBoxDesc(CAPSULE_DESC& Out) const override;
    virtual _bool  Is_Touch_Harmful() const override { return false; }

    virtual HRESULT Ready_AnimEvents() override;
    virtual HRESULT Ready_PartObjects() override;
    virtual const _float4x4* Get_FxParentMatrix(const _wstring& strFx) const override;

    virtual void Damaged(const ATTACK_INFO& tInfo) override;
    virtual void Update_AI(_float fTimeDelta) override;

public:
    void      Set_ActiveSword(EMK_SWORD eSword);
    EMK_SWORD Get_ActiveSword() const { return m_eActiveSword; }
    void      Enable_SwordHit(_bool bOn);
    void      Hide_Sword(EMK_SWORD eSword);
    void      Drop_Galaxia();
    void      Retire_Galaxia();
    void      Hold_BossCam(_bool bOn);

    void      Show_Mant(_bool bOn);
    void      Play_MantSync(const _char* szClip, _bool bLoop, _float fBland = 0.2f, _float fSpeed = 1.5f);
    void      Show_EscapeMant(_bool bOn);
    void      Play_EscapeMantSequence();

    void    Set_DodgeInvincible(_bool bOn) { m_bDodgeInvuln = bOn; }
    void    Set_AttackBusy(_bool bOn) { m_bAttackBusy = bOn; }
    _bool Consume_DodgeRequest()
    {
        if (!m_bDodgeRequested) return false;
        m_bDodgeRequested = false;
        m_fDodgeCooldown = s_fDodgeCooldown;
        return true;
    }

    _bool Is_GigaReady() const { return m_fGigaCooldown <= 0.f; }
    void  Fire_GigaMoonShot();

    void  Begin_RockDecalSlide();
    void  Drop_Rocks();
    _bool Is_RockReady() const { return m_fRockCooldown <= 0.f; }

    void  Set_TopViewCam(_bool bOn);

    void  Start_PatternCooldowns(_float fUsedCooldown);

    void  Enable_CatchBox(_bool bOn);
    void  Reset_CatchHit() { m_bCatchHit = false; }
    _bool Is_CatchHit() const { return m_bCatchHit; }
    _bool Is_UpperReady() const { return m_fUpperCooldown <= 0.f; }

    void Begin_Demo(EDemo eDemo);
    void End_Demo(EDemo eDemo);

    void Set_ParryWindow(_bool bOn);

    void Begin_LockingSync();
    void Sync_LockingProgress(_float fProgress01);
    void End_LockingSync();

    ELockOutcome Consume_LockOutcome()
    {
        ELockOutcome e = m_Lock.eOutcome;
        m_Lock.eOutcome = ELockOutcome::NONE;
        return e;
    }

private:
    struct DEMO_DESC
    {
        KIRBY_POSITION_SYNC_CONTEXT     eSyncContext;
        KIRBY_POSITION_SYNC_END_REASON  eEndReason;
        const _char* const*             pClips;
        _uint                           iClipCount;
    };

    struct LOCK_STATE
    {
        // 진행
        _bool  bActive = { false };
        _float fTimer = { 0.f };
        _bool  bCamFired = { false };

        // 판정
        _float       fGauge = { LOCK_GAUGE_START };
        _bool        bJudged = { false };
        ELockOutcome eOutcome = { ELockOutcome::NONE };

        // 애니메이션 동기화
        _bool bSyncing = { false };

        // 패링
        _bool bParryWindow = { false };

        // 커비 본 부착
        _bool            bAttached = { false };
        const _float4x4* pAttachBone = { nullptr };
        const _float4x4* pAttachAnchor = { nullptr };
        _float3          vSaveScale = { 1.f, 1.f, 1.f };
    };

    struct ROCK_FIELD
    {
        static constexpr int TILE_COUNT = 23;
        static constexpr int SAFE_COUNT = 5;

        _float3       vTiles[TILE_COUNT] = {};
        CAttackDecal* pDecals[TILE_COUNT] = {};
        _bool         bSafe[TILE_COUNT] = {};

        void Build(const _float3 fCornersIn[4]);
        void Select_Safe(CGameInstance_Proxy* pProxy);
    };

    enum class EPhaseTrans { NONE, HOP, LANDING, WAIT, DONE };
    enum class EDEATH { NONE, POSE_WAIT, PAUSING, MASK, MASK_HOLD, ESCAPE, DONE };

private:
    CBoss_Metaknight_Body* m_pBody = { nullptr };
    static const vector<_float> s_Thresholds;

    CBoss_Metaknight_Sword* m_pSword = { nullptr };          
    CBoss_Metaknight_ReplicaSword* m_pReplica = { nullptr }; 
    CBoss_Metaknight_Mant* m_pMant = { nullptr };
    CBoss_Metaknight_EscapeMant* m_pEscapeMant = { nullptr };

    EMK_SWORD m_eActiveSword = { EMK_SWORD::GALAXIA };

    _bool      m_bDodgeInvuln = { false };

    _bool  m_bDodgeRequested = { false };
    _bool  m_bAttackBusy = { false };    

    _bool  m_bAppearPending = { false };
    _float m_fAppearTimer = { 0.f };
    _float m_fIntroHoldTimer = { 0.f };

    
    _bool  m_bCatchHit = { false };

    ROCK_FIELD m_Rock = {};
    LOCK_STATE m_Lock = {};

    // 쿨타임
    _float        m_fDodgeCooldown = { 0.f };
    _float        m_fGigaCooldown = { s_fGigaCooldown };
    _float        m_fRockCooldown = { 0.f };
    _float        m_fUpperCooldown = { s_fUpperCooldown };
    
    EPhaseTrans m_ePhaseTrans = { EPhaseTrans::NONE };
    _float m_fPhaseBaseY = { 0.f };
    _float m_fPhaseVelY = { 0.f };

    _bool  m_bDeathSeq = { false };
    EDEATH m_eDeathStep = { EDEATH::NONE };
    _int   m_iDeathPoseDelay = { 0 };
    _float m_fDeathTimer = { 0.f };

    // 디버그
    static constexpr _bool s_bSkipIntro = true;

private:
    HRESULT Ready_MetaEvents();
    void Fire_CutsceneCamera(const _tchar* szTrack);
    void Hide_AllParts();
    void Update_PhaseTransition(_float fTimeDelta);
    void Update_Attachment();
    void Enter_Locking();
    void Exit_Locking();
    void Detach_FromKirby();
    void Judge_Locking(_bool bPlayerWin);

    void Tick_Cooldowns(_float fTimeDelta);
    void Tick_Locking(_float fTimeDelta);
    void Tick_Appear(_float fTimeDelta);
    void Tick_DeathSequence(_float fTimeDelta);

    void Snap_ToDemoOrigin();
    void Reanchor_FromRootBone();
    static const DEMO_DESC& Get_DemoDesc(EDemo eDemo);

    void Queue_DeathClips();

#ifdef _DEBUG
    void Debug_TriggerPhaseTransition();
#endif

public:
    static CBoss_Metaknight* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CBoss_Metaknight* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END