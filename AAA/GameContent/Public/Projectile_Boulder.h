#pragma once
#include "PhysicsProjectile.h"

NS_BEGIN(Engine) class CShader; class CModel; class CAnimator; NS_END
NS_BEGIN(Client)

// 고릴라 투척 돌. ANIM 모델(파괴 클립 + 파편메쉬 내장).
// 물리(CCT 바운스/이동/수명)는 부모. 여기선 Wait 루프 + 비행 중 LowM 본 회전 + 2바운스 파괴.
class CLIENT_DLL CProjectile_Boulder final : public CPhysicsProjectile
{
    GENERATED_BODY(CProjectile_Boulder)
public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Projectile_Boulder";
    static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_Boulder";

    enum class STATE { FLYING, BREAKING };

private:
    CProjectile_Boulder(ID3D11Device*, ID3D11DeviceContext*);
    CProjectile_Boulder(const CProjectile_Boulder&);
    virtual ~CProjectile_Boulder() = default;

public:
    virtual HRESULT Render() override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    virtual HRESULT Ready_Visual() override;             
    virtual void    On_Launched() override;              
    virtual void    On_Bounce(_int iCount) override;     
    virtual void    Update_Terminal(_float dt) override; 
    virtual void    Tick_Visual(_float dt) override;     
    virtual void    On_Activated() override;
    virtual void    On_Impact() override {}

private:
    void    Enter_Break();
    HRESULT Bind_ShaderResources();

private:
    CShader* m_pShaderCom = { nullptr };
    CModel* m_pModelCom = { nullptr };
    CAnimator* m_pAnimatorCom = { nullptr };

    STATE   m_eState = { STATE::FLYING };
    _float m_fSpinAngle = { 0.f };
    _float3 m_vSpinAxis = { 1.f, 0.f, 0.f };

    static constexpr _float RESTITUTION = 0.3f;         // 바운스 반발
    static constexpr _float HORIZ_DAMP = 0.7f;          // 바운스 수평 마찰
    static constexpr _float SPIN_SPEED_DEG = 200.f;     // 초당 바퀴수

public:
    static CProjectile_Boulder* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void Free() override;
};
NS_END