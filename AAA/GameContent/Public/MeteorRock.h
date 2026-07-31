#pragma once
#include "Projectile.h"
#include "Effect_Loader.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CMeteorRock abstract : public CProjectile
{
    GENERATED_BODY_ABSTRACT(CMeteorRock)

public:
    enum class METEOR_STATE { FALLING, LANDED };
    static constexpr const _tchar* SND_FALLING          = L"GimmickVolcanoRock_Falling.wav";

protected:
    CMeteorRock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CMeteorRock(const CMeteorRock& Prototype);
    virtual ~CMeteorRock() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual HRESULT Render_Shadow() override;

    virtual void Launch(const _float3& vPos, const _float3& vDir) override; 

    void Configure(_float fSpeed, _float fLifeSec, _bool bBreakOnLand, _float fHitRadius);
    void Set_TargetPos(const _float3& vTarget) { m_vTargetPos = vTarget; }

protected:
    virtual HRESULT Ready_Visual() override;
    virtual void On_Activated() override;
    virtual void On_Land();
    virtual const _tchar* Get_ModelProtoTag() = 0;
    virtual const _tchar* Get_BreakSoundKey() = 0;

    virtual HRESULT Ready_HitBox() override;
    virtual void    Kill() override;

    void Enter_Landed();
    void Spawn_ExplosionFx();
    void Spawn_AuraFx();                      
    void Release_AuraFx();
    HRESULT Bind_ShaderResources();


protected:
    METEOR_STATE m_eState = { METEOR_STATE::FALLING };

    CShader* m_pShaderCom = { nullptr };
    CModel* m_pModelCom = { nullptr };

    FX_HANDLE       m_hAura{ };
    CSound_Handle   m_FallSound;

    static constexpr _float FALL_SOUND_VOLUME = 0.3f;
    static constexpr _float BREAK_SOUND_VOLUME = 0.5f;
    static constexpr _float SOUND_FALLOFF_RANGE = 50.f;

    _float m_fLingerTimer = { 0.f };
    _float m_fVibrationElapsed = { 0.f };
    _float3 m_vTargetPos = {};
    _float3 m_vDir = {};
    _bool m_bBreakOnLand = { false };

    static constexpr _float LINGER_SEC = 0.05f;
    static constexpr _float SPIN_DEG = 60.f;
    static constexpr _float FX_DIRECTION_EPSILON_SQ = 1e-6f;
    static constexpr _float FX_REFERENCE_DOT_LIMIT  = 0.99f;

    static constexpr const _tchar* FX_AURA = L"MeteorAura";

    static constexpr _float VIBRATION_AMPLITUDE = 0.05f;
    static constexpr _float VIBRATION_FREQ_X = 120.f;
    static constexpr _float VIBRATION_FREQ_Z = 120.f;

    static constexpr _float SHAKE_BASE_TRAUMA = 0.5f;
    static constexpr _float SHAKE_FULL_RADIUS = 20.f;
    static constexpr _float SHAKE_ZERO_RADIUS = 50.f;
protected:
    virtual void Free() override;
};

NS_END
