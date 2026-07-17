#pragma once
#include "Boss.h"

NS_BEGIN(Client)

class CBoss_Armadillo_Body;
class CProjectile_Partner;
class CBoss_Armadillo_Cage;

class CBoss_Armadillo final : public CBoss
{
    GENERATED_BODY(CBoss_Armadillo)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Armadillo";

    static constexpr _float s_fCCT_Radius = 3.f;
    static constexpr _float s_fCCT_Height = 6.f;

    static constexpr const _char* GRAB_BONE = "RHaveL";

    static constexpr _float DEATH_PAUSE_SEC = 0.7f;
    static constexpr _float DEATH_SHAKE_SEC = 0.7f;

private:
    CBoss_Armadillo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss_Armadillo(const CBoss_Armadillo& Prototype);
    virtual ~CBoss_Armadillo() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

public:
    virtual CAnimator* Get_BodyAnimator() const override;
    virtual CMultiHitBoxPart* Get_HitBoxPart() const override;

public:
    void  Reset_CatchHit() { m_bCatchHit = false; }
    _bool Is_CatchHit() const { return m_bCatchHit; }

    //QTE
    void  Begin_QTE(_float fSeconds);
    void  End_QTE();
    _bool Is_QTEEscaped() const { return m_bQTEEscaped; }
    _bool Tick_QTETimer(_float fDt) { m_fQTETimer -= fDt; return m_fQTETimer <= 0.f; }

    void  Fire_Grab();
    void  Fire_Release(KIRBY_ATTACHMENT_END_REASON eType);

    _bool Sweep_Wall(const _float3& vDir, _float fDist, _float3* pOutNormal) const;
    void  Set_TwinDanceOffset(_bool bOn);

    void Summon_Partner();
    void Fire_PartnerThrow();
    void Enable_PartnerSpinHit(_bool b);
    void Play_PartnerAnim(const _char* szClip, _bool bLoop);

    _bool Has_Partner() const { return m_pPartner != nullptr; }

    void Show_Cage();
    void Hide_Cage();

    void Set_RollFx(_bool bOn);
    void Play_WallImpact();

protected:
    virtual CMonsterBrain* Create_Brain() override;
    virtual void           Play_Intro() override;
    virtual _bool          Is_Intro_Finished() const override;
    virtual void           Play_Death() override;
    virtual _bool          Is_Death_Finished() const override;
    virtual void           On_Enter_Corpse() override;
    virtual _float         Get_CorpseLinger() const override { return 0.f; }

    virtual const vector<_float>& Get_PhaseThresholds() const override { return s_Thresholds; }
    virtual const _tchar* Get_AppearEventTag() const override { return TEXT("Armadillo_Appear"); }

    virtual _float Get_CapsuleRadius() const override { return s_fCCT_Radius; }
    virtual _float Get_CapsuleHeight() const override { return s_fCCT_Height; }
    virtual _float Get_InteractRadius() const override { return 0.f; }
    virtual _bool  Get_HurtBoxDesc(CAPSULE_DESC& Out) const override;
    virtual _bool  Is_Touch_Harmful() const override { return false; }

    virtual HRESULT Ready_AnimEvents() override;
    virtual HRESULT Ready_PartObjects() override;
    virtual const _float4x4* Get_FxParentMatrix(const _wstring& strFx) const override;

private:
    CBoss_Armadillo_Body* m_pBody = { nullptr };
    static const vector<_float> s_Thresholds;

    CProjectile_Partner* m_pPartner = { nullptr };
    CBoss_Armadillo_Cage* m_pCage = { nullptr };

    _float3 m_vBodyOffsetStart = { 0.f, 0.f, 0.f };
    _float3 m_vBodyOffsetTarget = { 0.f, 0.f, 0.f };
    _float  m_fBodyOffsetT = { 1.f };
    static constexpr _float s_fBodyOffsetBlendTime = 0.25f;

    _bool m_bCatchHit = { false };
    _bool  m_bQTEEscaped = { false };
    _float m_fQTETimer = { 0.f };

    enum class EDEATH { POSE_WAIT, PAUSING, PLAYING };
    _bool   m_bDeathSeq = { false };
    EDEATH  m_eDeathStep = { EDEATH::POSE_WAIT };
    _int    m_iDeathPoseDelay = { 0 };
    _float  m_fDeathPauseTimer = { 0.f };

    _bool   m_bRutTrail = { false };
    _float3 m_vRutLastPos = { 0.f, 0.f, 0.f };
    _int    m_iRutToggle = { 0 };

    static constexpr _float s_fRutInterval = 2.f;

    CEffect_Container* m_pRollWind = { nullptr };

#ifdef _DEBUG
    mutable _bool m_bDebugWallHit = { false };
#endif

private:
    void Update_BodyOffset(_float fTimeDelta);
    void Fire_CatchCamera(const _tchar* szTrack);
    void Tick_DeathSequence(_float fTimeDelta);
    void Update_RutTrail(_float fTimeDelta);

public:
    static CBoss_Armadillo* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CBoss_Armadillo* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END