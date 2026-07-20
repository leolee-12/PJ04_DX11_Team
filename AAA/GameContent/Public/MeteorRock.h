#pragma once
#include "Projectile.h"

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

    void Configure(_float fSpeed, _float fLifeSec, _bool bBreakOnLand);

protected:
    virtual HRESULT Ready_Visual() override;
    virtual void On_Activated() override;
    virtual void On_Land();
    virtual const _tchar* Get_ModelProtoTag() = 0;

    void Enter_Landed();
    HRESULT Bind_ShaderResources();


protected:
    METEOR_STATE m_eState = { METEOR_STATE::FALLING };

    _float m_fLingerTimer = { 0.f };
    _bool m_bBreakOnLand = { false };

    CShader* m_pShaderCom = { nullptr };
    CModel* m_pModelCom = { nullptr };

    static constexpr _float LINGER_SEC = 1.5f;
    static constexpr _float SPIN_DEG = 60.f;
    static constexpr _float STUCK_PULLBACK = 0.1f;

protected:
    virtual void Free() override;
};

NS_END
