#pragma once
#include "GameContent_Defines.h"
#include "Component.h"

NS_BEGIN(Engine)
class CTransform;
class CController;
NS_END

NS_BEGIN(Client)

class CBubbleMovement final : public CComponent
{
    GENERATED_BODY(CBubbleMovement)
private:
    CBubbleMovement(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBubbleMovement(const CBubbleMovement& Prototype);
    virtual ~CBubbleMovement() = default;

public:
    virtual HRESULT         Initialize_Prototype() override;
    virtual HRESULT         Initialize(void* pArg) override;

    void                    Set_Refs(CTransform* pTransform, CController* pController);
    void                    Set_Physics(_float fGravity, _float fRestitution, _float fHorizDamp);

    void                    Launch(_fvector vVelocity);

    // 이동 + 바운스 1틱 
    _bool                   Tick(_float fTimeDelta);
    _bool                   Is_Grounded() const { return m_bGrounded; }
    _float3                 Get_Velocity() const { return m_vVelocity; }

    void                    Stop();

private:
    CTransform*             m_pTransform = { nullptr };
    CController*            m_pController = { nullptr };

    _float3                 m_vVelocity = { 0.f, 0.f, 0.f };
    _bool                   m_bGrounded = { false };

    _float                  m_fGravity = { -12.f };
    _float                  m_fRestitution = { 0.8f };
    _float                  m_fHorizDamp = { 0.85f };

public:
    static CBubbleMovement* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent*     Clone(void* pArg) override;
    virtual void            Free() override;

};

NS_END