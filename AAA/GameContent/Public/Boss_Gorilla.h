#pragma once
#include "Boss.h"

NS_BEGIN(Client)

class CBoss_Gorilla_Body;

class CBoss_Gorilla final : public CBoss
{
    GENERATED_BODY(CBoss_Gorilla)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Gorilla";

    // TODO(파츠 단계에서 튜닝)
    static constexpr _float s_fCCT_Radius = 1.8f;
    static constexpr _float s_fCCT_Height = 1.5f;

private:
    CBoss_Gorilla(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss_Gorilla(const CBoss_Gorilla& Prototype);
    virtual ~CBoss_Gorilla() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    On_Deserialized() override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    // CBossBase 연출 훅 (파츠 전까지 스텁)
    virtual CMonsterBrain* Create_Brain() override;
    virtual void           Play_Intro() override {}                         // TODO: 바디 애님
    virtual _bool          Is_Intro_Finished() const override { return true; }
    virtual void           Play_Death() override {}                         // TODO
    virtual _bool          Is_Death_Finished() const override { return true; }

    // CBoss 페이즈 정의
    virtual const vector<_float>& Get_PhaseThresholds() const override { return s_Thresholds; }
    virtual void           Play_PhaseTransition(_int iNewPhase) override {} // TODO: 전환 연출
    virtual _bool          Is_PhaseTransition_Finished() const override { return true; }
    virtual void           On_PhaseChanged(_int iOldPhase, _int iNewPhase) override;

    // CMonster 충돌 스펙
    virtual _float Get_CapsuleRadius() const override { return s_fCCT_Radius; }
    virtual _float Get_CapsuleHeight() const override { return s_fCCT_Height; }
    virtual _float Get_InteractRadius() const override { return 0.f; }
    virtual _bool  Get_HurtBoxDesc(CAPSULE_DESC& Out) const override;

    virtual CAnimator* Get_BodyAnimator() const override;

    virtual HRESULT Ready_PartObjects() override;
    //virtual HRESULT Ready_AnimEvents() override;

private:
    CBoss_Gorilla_Body* m_pBody = { nullptr };
    static const vector<_float> s_Thresholds;   // HP비율 내림차순 → 3페이즈

public:
    static CBoss_Gorilla* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CBoss_Gorilla* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END