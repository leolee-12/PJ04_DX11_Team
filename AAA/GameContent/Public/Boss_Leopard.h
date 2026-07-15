#pragma once
#include "Boss.h"

NS_BEGIN(Client)

class CBoss_Leopard_Body;
class CProjectile_Nail;

class CBoss_Leopard final : public CBoss
{
    GENERATED_BODY(CBoss_Leopard)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Leopard";

    static constexpr _float s_fCCT_Radius = 3.f;   // TODO: 레오파드 크기
    static constexpr _float s_fCCT_Height = 6.f;

    static constexpr _float DEATH_PAUSE_SEC = 0.7f;
    static constexpr _float DEATH_SHAKE_SEC = 0.7f;

    static constexpr _int NAIL_COUNT = 5;

    enum PILLAR : _int { PILLAR_FL, PILLAR_FR, PILLAR_BR, PILLAR_BL, PILLAR_COUNT };

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

public:
    const _float3& Get_CurPillarPos() const { return s_vPillarPos[m_iCurPillar]; }
    void  Set_CurPillar(PILLAR e) { m_iCurPillar = e; }
    _int  Advance_ToAdjacentPillar();

    void Spawn_HandNails();
    void Launch_NextHandNail();

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

private:
    void Tick_DeathSequence(_float fTimeDelta);

public:
    static CBoss_Leopard* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CBoss_Leopard* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END