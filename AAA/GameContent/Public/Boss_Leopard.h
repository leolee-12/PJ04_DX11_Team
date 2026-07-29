#pragma once
#include "Boss.h"

NS_BEGIN(Client)

class CBoss_Leopard_Body;
class CProjectile_Nail;
class CSpotlight_Rig;

class CBoss_Leopard final : public CBoss
{
    GENERATED_BODY(CBoss_Leopard)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Leopard";

    static constexpr _float s_fCCT_Radius = 1.2f;
    static constexpr _float s_fCCT_Height = 0.8f;

    static constexpr _float DEATH_PAUSE_SEC = 0.7f;
    static constexpr _float DEATH_SHAKE_SEC = 0.7f;

    static constexpr _int NAIL_COUNT = 5;

    static constexpr _float BASE_ANIM_SPEED = 1.5f;

    enum PILLAR : _int { PILLAR_FL, PILLAR_FR, PILLAR_BR, PILLAR_BL, PILLAR_COUNT };

#pragma region 캐롤라인 보스씬 사운드
    static constexpr const _tchar* SND_LIGHT_ON             = L"CharaBossLeopard_LightOn.wav";
    static constexpr const _tchar* SND_LIGHT_OFF            = L"CharaBossLeopard_LightOff.wav";
    static constexpr const _tchar* SND_JUMPATTACK_LANDING   = L"CharaBossLeopard_ShockWave.wav";

#pragma endregion

private:
    CBoss_Leopard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss_Leopard(const CBoss_Leopard& Prototype);
    virtual ~CBoss_Leopard() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

    virtual CAnimator* Get_BodyAnimator() const override;
    virtual CMultiHitBoxPart* Get_HitBoxPart() const override;

public:
    const _float3& Get_CurPillarPos() const { return s_vPillarPos[m_iCurPillar]; }
    void  Set_CurPillar(PILLAR e) { m_iCurPillar = e; }
    _int  Advance_ToAdjacentPillar();

    void  Spawn_HandNails();
    void  Launch_NextHandNail();

    void  Enter_PillarMode();
    void  Exit_PillarMode(); 
    _bool Is_PillarMode() const { return m_bPillarMode; }

    void  Set_AfterimageFx(_bool bOn, const _wstring& strEffectId = L"");
    void  Spawn_FloorFx();
    void  Spawn_JumpSmoke(const _float3& vPos);

    void  Spotlight_Off();
    void  Spotlight_On_Snap();
    void  Spotlight_TrackKirby();
    void  Spotlight_LockTarget(_fvector vWorldPos);

    void  Fire_CamShake();

protected:
    virtual CMonsterBrain* Create_Brain() override;
    virtual void           Play_Intro() override;
    virtual _bool          Is_Intro_Finished() const override;
    virtual void           Play_Death() override;
    virtual _bool          Is_Death_Finished() const override;
    virtual void           On_Enter_Corpse() override;
    virtual _float         Get_CorpseLinger() const override { return 0.f; }

    virtual const vector<_float>& Get_PhaseThresholds() const override { return s_Thresholds; }
    virtual const _tchar* Get_AppearEventTag() const override { return TEXT("Leopard_Appear"); }

    virtual _float Get_CapsuleRadius() const override { return s_fCCT_Radius; }
    virtual _float Get_CapsuleHeight() const override { return s_fCCT_Height; }
    virtual _float Get_InteractRadius() const override { return 0.f; }
    virtual _bool  Get_HurtBoxDesc(CAPSULE_DESC& Out) const override;
    virtual _bool  Is_Touch_Harmful() const override { return false; }

    virtual HRESULT Ready_PartObjects() override;
    virtual HRESULT Ready_AnimEvents() override;

    virtual void    On_Deserialized() override;
    virtual const   _float4x4* Get_FxParentMatrix(const _wstring& strFx) const
    {
        return m_pTransformCom->Get_WorldMatrixPtr();
    }

private:
    CBoss_Leopard_Body* m_pBody = { nullptr };
    static const vector<_float> s_Thresholds;

    enum class EDEATH { POSE_WAIT, PAUSING, PLAYING };
    _bool   m_bDeathSeq = { false };
    EDEATH  m_eDeathStep = { EDEATH::POSE_WAIT };
    _int    m_iDeathPoseDelay = { 0 };
    _float  m_fDeathPauseTimer = { 0.f };

    static const _float3 s_vPillarPos[PILLAR_COUNT];
    _int m_iCurPillar = { PILLAR_FL };

    CProjectile_Nail* m_pNails[NAIL_COUNT] = {};

    _bool m_bPillarMode = { false };

    static constexpr _float s_fAfterimageInterval = 0.02f;
    _bool  m_bAfterimageFx = { false };
    _float m_fAfterimageAccum = { 0.f };
    _wstring m_strAfterimageId;

    CSpotlight_Rig* m_pSpotRig = { nullptr };

    enum SPOT_TARGET { SPOT_LEOPARD, SPOT_KIRBY, SPOT_FIXED };
    SPOT_TARGET m_eSpotTarget = { SPOT_LEOPARD };

private:
    void Tick_DeathSequence(_float fTimeDelta);
    void  Update_Afterimage(_float fTimeDelta);
    void  Spawn_Afterimage();
    HRESULT Ready_TestSpotlight();
#ifdef _DEBUG
    void Debug_KeyInput();
#endif

public:
    static CBoss_Leopard* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CBoss_Leopard* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END