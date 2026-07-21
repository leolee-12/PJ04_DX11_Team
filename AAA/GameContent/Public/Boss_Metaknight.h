#pragma once
#include "Boss.h"

NS_BEGIN(Client)

class CBoss_Metaknight_Body;
class CBoss_Metaknight_Sword;
class CBoss_Metaknight_ReplicaSword;
class CBoss_Metaknight_Mant;
class CAttackDecal;

class CBoss_Metaknight final : public CBoss
{
    GENERATED_BODY(CBoss_Metaknight)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Metaknight";

    static constexpr const _tchar* APPEAR_TAG = TEXT("Metaknight_Appear");
    static constexpr _float APPEAR_DELAY = 1.5f;
    static constexpr _float INTRO_HOLD_TIME = 2.f;

    static constexpr _float s_fCCT_Radius = 0.75f;
    static constexpr _float s_fCCT_Height = 0.1f;
    static constexpr _float s_fDefaultAnimSpeed = 1.5f;

    static constexpr _float s_fDodgeCooldown = 2.f;

    static constexpr const _char* WEAPON_BONE = "RHaveL";

    static constexpr _uint  GIGA_POINT_COUNT = 4;
    static const _float3    s_vGigaPoints[GIGA_POINT_COUNT];
    static constexpr _float s_fGigaCooldown = 12.f;

    static constexpr int    ROCK_SAFE_COUNT = 5;
    static constexpr int    ROCK_TILE_COUNT = 23;
    static constexpr _float ROCK_DECAL_RADIUS = 5.f;
    static constexpr _float ROCK_SLIDE_TIME = 1.f;
    static constexpr _float ROCK_DROP_HEIGHT = 20.f;
    static constexpr _float s_fRockCooldown = 60.f;

    static constexpr _float TOPVIEW_HEIGHT = 40.f;

    static constexpr _float PHASE_HOP_HEIGHT = 2.f;
    static constexpr _float PHASE_HOP_GRAVITY = 9.f;
    static constexpr _float PHASE_WAIT_BLEND = 0.6f;


    enum class EMK_SWORD { GALAXIA, REPLICA, NONE };

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

public:
    void      Set_ActiveSword(EMK_SWORD eSword);
    EMK_SWORD Get_ActiveSword() const { return m_eActiveSword; }
    void      Enable_SwordHit(_bool bOn);

    void      Show_Mant(_bool bOn);
    void      Play_MantSync(const _char* szClip, _bool bLoop, _float fBland = 0.2f, _float fSpeed = 1.5f);

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
    void  Start_GigaCooldown() { m_fGigaCooldown = s_fGigaCooldown; }
    void  Fire_GigaMoonShot();

    void  Begin_RockDecalSlide();

    void  Drop_Rocks();
    _bool Is_RockReady() const { return m_fRockCooldown <= 0.f; }
    void  Start_RockCooldown() { m_fRockCooldown = s_fRockCooldown; }

    void  Set_TopViewCam(_bool bOn);

private:
    CBoss_Metaknight_Body* m_pBody = { nullptr };
    static const vector<_float> s_Thresholds;

    CBoss_Metaknight_Sword* m_pSword = { nullptr };          
    CBoss_Metaknight_ReplicaSword* m_pReplica = { nullptr }; 
    CBoss_Metaknight_Mant* m_pMant = { nullptr };
    EMK_SWORD m_eActiveSword = { EMK_SWORD::GALAXIA };

    _bool      m_bDodgeInvuln = { false };

    _bool  m_bDodgeRequested = { false };
    _bool  m_bAttackBusy = { false };    
    _float m_fDodgeCooldown = { 0.f };

    _float m_fGigaCooldown = { s_fGigaCooldown };

    _bool  m_bAppearPending = { false };
    _float m_fAppearTimer = { 0.f };
    _float m_fIntroHoldTimer = { 0.f };

    _float3       m_RockTiles[ROCK_TILE_COUNT];
    CAttackDecal* m_pRockDecals[ROCK_TILE_COUNT] = {};
    _bool         m_bSafeTile[ROCK_TILE_COUNT] = {};
    _float        m_fRockCooldown = { 0.f };

    enum class EPhaseTrans { NONE, HOP, LANDING, WAIT, DONE };
    EPhaseTrans m_ePhaseTrans = { EPhaseTrans::NONE };
    _float m_fPhaseBaseY = { 0.f };
    _float m_fPhaseVelY = { 0.f };

    // µð¹ö±×
    static constexpr _bool s_bSkipIntro = true;

private:
    void Fire_CutsceneCamera(const _tchar* szTrack);
    void Hide_AllParts();
    void Build_RockTilePositions(const _float3 fCornersIn[4], _float3 fOutPos[23]);
    void Select_SafeTiles();
    void Update_PhaseTransition(_float fTimeDelta);

#ifdef _DEBUG
    void Debug_TriggerPhaseTransition();
#endif

public:
    static CBoss_Metaknight* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CBoss_Metaknight* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END