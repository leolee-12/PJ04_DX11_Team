#pragma once
#include "PhysicsProjectile.h"

NS_BEGIN(Engine) class CShader; class CModel; class CAnimator; NS_END
NS_BEGIN(Client)

// 아르마딜로 파트너 인형. 소환 시 보스 Partner1L 소켓에 부착(캐리),
// 트윈롤링 중엔 부착 상태로 자체 스핀 히트박스, 던지면 포물선 투사체.
// 착탄/명중 시 Break 클립 재생 후 풀 반환. 동시 N개 허용(풀).
class CProjectile_Partner final : public CPhysicsProjectile
{
    GENERATED_BODY(CProjectile_Partner)
public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Projectile_Partner";
    static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_ArmadilloPartner";
    static constexpr const _tchar* POOL_KEY = L"ArmadilloPartner";

    static constexpr _float ANIM_SPEED = 1.5f;

    enum class STATE { CARRIED, FLYING, BREAKING };

private:
    CProjectile_Partner(ID3D11Device*, ID3D11DeviceContext*);
    CProjectile_Partner(const CProjectile_Partner&);
    virtual ~CProjectile_Partner() = default;

public:
    virtual void    Update(_float fTimeDelta) override;   // 캐리 중 스핀 히트박스 추적 추가
    virtual HRESULT Render() override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }

public:
    void Play_Anim(const _char* szClip, _bool bLoop, _float fSpeed = ANIM_SPEED);
    void Enable_SpinHitBox(_bool b);                      // 트윈롤링 중 자체 판정
    void Despawn() { Kill(); }                            // 보스 사망 등 외부 정리용

protected:
    virtual HRESULT Ready_Visual() override;
    virtual void    On_Activated() override;              // 부착 = Appear 재생
    virtual void    On_Launched() override;               // 발사 = TwinRolling 회전 재활용
    virtual void    On_Bounce(_int iCount) override {}
    virtual void    On_Impact() override {}
    virtual void    Update_Terminal(_float dt) override;  // Break 끝나면 풀 반환
    virtual void    Tick_Visual(_float dt) override;

private:
    void    Enter_Break();
    HRESULT Bind_ShaderResources();
    void    Tick_LaunchFx(_float fTimeDelta);

private:
    CShader* m_pShaderCom = { nullptr };
    CModel* m_pModelCom = { nullptr };
    CAnimator* m_pAnimatorCom = { nullptr };

    STATE  m_eState = { STATE::CARRIED };

    _float m_fFxTimer = { 0.f };
    static constexpr _float FX_INTERVAL = 1.f;

public:
    static CProjectile_Partner* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void Free() override;
};
NS_END