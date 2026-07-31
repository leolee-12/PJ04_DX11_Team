#pragma once
#include "Projectile.h"

NS_BEGIN(Engine)
class CEffect_Container;
NS_END

NS_BEGIN(Client)
class IInhalable;

class CSpit_Projectile final : public CProjectile
{
    GENERATED_BODY(CSpit_Projectile)
public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Spit_Projectile";

private:
    CSpit_Projectile(ID3D11Device*, ID3D11DeviceContext*);
    CSpit_Projectile(const CSpit_Projectile&);
    virtual ~CSpit_Projectile() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;

    void            Fire(IInhalable* const* ppPayloads, _uint iCount, const _float3& vPos, const _float3& vDir);

    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override { return S_OK; }
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    virtual HRESULT Ready_HitBox() override;  // 방향 뒤집기: PLAYER_PROJECTILE 등록, MONSTER_HURT 타격
    virtual void    Kill() override;          // 수명종료/명중 공통 경로: 페이로드 정리

private:
    void            Drive_Payload();          // 페이로드 트랜스폼을 발사체 것으로 덮어씀

private:
    struct PAYLOAD
    {
        IInhalable* pInhalable = { nullptr };
        _float3     vScale = { 1.f, 1.f, 1.f };
        _float4x4   matRot = {};   // 삼킨 당시 회전
    };

    static constexpr _uint  s_iMaxPayload = 3;
    PAYLOAD  m_Payloads[s_iMaxPayload];
    _uint    m_iPayloadCount = { 0 };

    float   m_fSpinDeg = { 0.f };
    _float3  m_vSpinAxis = { 0.f, 0.f, 1.f };
    static constexpr _float SPIN_SPEED_DEG = 720.f;

    CEffect_Container*          m_pSpitFx = { nullptr };

public:
    static CSpit_Projectile* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void Free() override;
};
NS_END