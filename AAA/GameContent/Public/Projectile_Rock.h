#pragma once
#include "Projectile.h"

NS_BEGIN(Engine) 
class CShader; 
class CModel; 
class CAnimator; 
NS_END


NS_BEGIN(Client)

class CProjectile_Rock final : public CProjectile
{
    GENERATED_BODY(CProjectile_Rock)
public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Projectile_Rock";
    static constexpr const _tchar* POOL_KEY = L"MetaRock";
    static constexpr const wchar_t* MODEL_PROTO_TAG_A = L"Prototype_Component_Model_RockA";
    static constexpr const wchar_t* MODEL_PROTO_TAG_B = L"Prototype_Component_Model_RockB";

    static constexpr _float FALL_SPEED = 45.f;
    static constexpr _float HIT_WINDOW = 0.15f;                     
    static constexpr const _char* ANIM_BREAK = "Disappearance";  

    enum class STATE { FALLING, BREAKING };

private:
    CProjectile_Rock(ID3D11Device*, ID3D11DeviceContext*);
    CProjectile_Rock(const CProjectile_Rock&);
    virtual ~CProjectile_Rock() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

    void Drop(const _float3& vTile, _float fSpawnHeight);  

protected:
    virtual HRESULT Ready_Visual() override;
    virtual HRESULT Ready_HitBox() override;
    virtual void    On_Activated() override;                
    virtual void    On_Impact() override {}                 

private:
    HRESULT Bind_ShaderResources();

private:
    CShader* m_pShaderCom = { nullptr };
    CModel* m_pModelCom = { nullptr };
    CAnimator* m_pAnimatorCom = { nullptr };

    STATE  m_eState = { STATE::FALLING };
    _float m_fTargetY = { 0.f };
    _float m_fHitTimer = { 0.f };

public:
    static CProjectile_Rock* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END