#pragma once
#include "GameContent_Defines.h"
#include "GameObject.h"
#include "Damageable.h"        // ATTACK_INFO / IDamageable

NS_BEGIN(Engine) 
class CCollider; 
NS_END

NS_BEGIN(Client)
class CProjectile_Manager;

// 풀링 투사체 베이스. CEffect_Container 의 풀 자기회수 패턴 미러링.
// 휴면 시 m_bAlive=false 로 Update/Late_Update early-out(Object_Manager 가 Active 무시하고 무조건 tick).
class CLIENT_DLL CProjectile abstract : public CGameObject
{
    GENERATED_BODY_ABSTRACT(CProjectile)

protected:
    CProjectile(ID3D11Device*, ID3D11DeviceContext*);
    CProjectile(const CProjectile&);
    virtual ~CProjectile() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Priority_Update(_float) override {}
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override { return S_OK; }   // concrete 가 모델 렌더 override

public:
    virtual void Launch(const _float3& vPos, const _float3& vDir);
    virtual void Launch_Arc(const _float3& vStart, const _float3& vTarget, _float fDur, _float fHeight) {}

    void            Set_Speed(_float fSpeed) { m_fSpeed = fSpeed; }

   // 풀 반납처 (CEffect_Container::Set_Pool 미러)
    void Set_Pool(CProjectile_Manager* p, _uint iLevel, const _wstring& strKey)
    {
         m_pPool = p; m_iPoolLevel = iLevel; m_strPoolKey = strKey;
    }
    
    _bool Is_Alive() const { return m_bAlive; }

    void Attach_To_Socket(const _float4x4* pBone, const _float4x4* pOwnerWorld, _fmatrix matOffset);
    void Detach() { m_bCarried = false; m_pSocketBone = nullptr; m_pSocketOwnerWorld = nullptr; }

protected:
    virtual void            Kill();        
    virtual HRESULT         Ready_HitBox();
    virtual HRESULT         Ready_Movement() { return S_OK; }
    virtual HRESULT         Ready_Visual() { return S_OK; }
    virtual void            On_Activated() {}
    virtual void            On_Impact() { Kill(); }

    void Update_Socket()
    {
        if (!m_pSocketBone || !m_pSocketOwnerWorld) return;
        _matrix mat = XMLoadFloat4x4(&m_SocketOffset)
            * XMLoadFloat4x4(m_pSocketBone)
            * XMLoadFloat4x4(m_pSocketOwnerWorld);
        m_pTransformCom->Set_WorldMatrix(mat);
    }

protected:
    CCollider* m_pHitBox = { nullptr };
    _bool   m_bAlive = { false };
    _float  m_fAccLife = { 0.f };
    _float3 m_vVelocity = {};

    CProjectile_Manager* m_pPool = { nullptr };
    _uint    m_iPoolLevel = {};
    _wstring m_strPoolKey;

    _float m_fSpeed = { 20.f };
    _float m_fLifeTime = { 5.f };
    _float m_fDamage = { 1.f };
    _float m_fKnockback = { 8.f };

    _float m_fHitHeight = { 0.05f };
    _float m_fHitRadius = { 0.5f };
    _float3 m_vCenterOffset = { 0.f, 0.f, 0.f };
    _float3 m_vRadians = { 0.f, 0.f, 0.f };


    _bool m_bCarried = { false };
    const _float4x4* m_pSocketBone = { nullptr };
    const _float4x4* m_pSocketOwnerWorld = { nullptr };
    _float4x4        m_SocketOffset = {};

protected:
    virtual void Free() override;
};
NS_END