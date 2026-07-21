#pragma once
#include "Projectile.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CGigatzoBullet final : public CProjectile
{
    GENERATED_BODY(CGigatzoBullet)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_GigatzoBullet";
    static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_GigatzoBullet";

private:
    CGigatzoBullet(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CGigatzoBullet(const CGigatzoBullet& Prototype);
    virtual ~CGigatzoBullet() = default;

public:
    virtual void            Update(_float fTimeDelta) override;
    virtual HRESULT         Render() override;
    virtual HRESULT		    Render_Shadow() override;
    virtual void            Launch(const _float3& vPos, const _float3& vDir) override;
    virtual void            Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

    void                    Configure(_float fSpeed, _float fDamage, _float fLifeSec);

protected:
    virtual HRESULT         Ready_Visual() override;
    virtual void            On_Activated() override;

    virtual void            On_Impact() override;

private:
    HRESULT                 Bind_ShaderResources();

private:
    CShader*                m_pShaderCom = { nullptr };
    CModel*                 m_pModelCom = { nullptr };
    CAnimator*              m_pAnimatorCom = { nullptr };
    _bool                   m_bIntro = { false };

public:
    static CGigatzoBullet*  Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*    Clone(void* pArg) override;

protected:
    virtual void            Free() override;
};

NS_END