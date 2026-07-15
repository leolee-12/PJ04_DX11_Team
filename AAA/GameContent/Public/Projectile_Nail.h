#pragma once
#include "Projectile.h"

NS_BEGIN(Engine) class CShader; class CModel; NS_END
NS_BEGIN(Client)

// 네일(표창): 커비는 관통(데미지만), 정적 지형에 닿으면 꽂혀 잠시 유지 후 소멸. 풀 관리.
class CProjectile_Nail final : public CProjectile
{
    GENERATED_BODY(CProjectile_Nail)
public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Projectile_Nail";
    static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_LeopardNail";
    static constexpr const _tchar* POOL_KEY = L"LeopardNail";

    enum class STATE { FLYING, STUCK };

private:
    CProjectile_Nail(ID3D11Device*, ID3D11DeviceContext*);
    CProjectile_Nail(const CProjectile_Nail&);
    virtual ~CProjectile_Nail() = default;

public:
    virtual void    Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

public:
    void Launch_At(const _float3& vTargetPos);

protected:
    virtual HRESULT Ready_Visual() override;
    virtual void    On_Impact() override {}   // 커비 히트로는 안 사라짐(관통, 데미지만)

private:
    void    Spin(_float dt);
    void    Enter_Stuck(const _float3& vNormal);
    HRESULT Bind_ShaderResources();

private:
    CShader* m_pShaderCom = { nullptr };
    CModel* m_pModelCom = { nullptr };

    STATE  m_eState = { STATE::FLYING };
    _float m_fStuckTimer = { 0.f };

    static constexpr _float SPIN_SPEED = 30.f;   // rad/s
    static constexpr _float STUCK_LINGER = 3.f;    // 꽂힌 채 유지(초)

public:
    static CProjectile_Nail* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END