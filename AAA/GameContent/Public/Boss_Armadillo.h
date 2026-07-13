#pragma once
#include "Boss.h"

NS_BEGIN(Client)

class CBoss_Armadillo_Body;
class CProjectile_Partner;

class CBoss_Armadillo final : public CBoss
{
    GENERATED_BODY(CBoss_Armadillo)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Armadillo";

    static constexpr _float s_fCCT_Radius = 3.f;
    static constexpr _float s_fCCT_Height = 6.f;

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

    _bool Sweep_Wall(const _float3& vDir, _float fDist, _float3* pOutNormal) const;

    void Summon_Partner();
    void Fire_PartnerThrow();
    void Enable_PartnerSpinHit(_bool b);
    void Play_PartnerAnim(const _char* szClip, _bool bLoop);

protected:
    virtual CMonsterBrain* Create_Brain() override;
    virtual void           Play_Intro() override;
    virtual _bool          Is_Intro_Finished() const override;
    virtual void           Play_Death() override;
    virtual _bool          Is_Death_Finished() const override;

    virtual const vector<_float>& Get_PhaseThresholds() const override { return s_Thresholds; }
    virtual const _tchar* Get_AppearEventTag() const override { return TEXT("Armadillo_Appear"); }

    virtual _float Get_CapsuleRadius() const override { return s_fCCT_Radius; }
    virtual _float Get_CapsuleHeight() const override { return s_fCCT_Height; }
    virtual _float Get_InteractRadius() const override { return 0.f; }
    virtual _bool  Get_HurtBoxDesc(CAPSULE_DESC& Out) const override;
    virtual _bool  Is_Touch_Harmful() const override { return false; }

    virtual HRESULT Ready_AnimEvents() override;
    virtual HRESULT Ready_PartObjects() override;

private:
    CBoss_Armadillo_Body* m_pBody = { nullptr };
    static const vector<_float> s_Thresholds;

    _bool m_bCatchHit = { false };
    CProjectile_Partner* m_pPartner = { nullptr };

public:
    static CBoss_Armadillo* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CBoss_Armadillo* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END