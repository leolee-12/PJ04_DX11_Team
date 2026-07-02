#pragma once
#include "Effect_Quad.h"

NS_BEGIN(Client)

class CBubble final : public CEffect_Quad
{
    GENERATED_BODY(CBubble)

public:
    struct BUBBLE_QUAD_DESC : public CEffect_Quad::EFFECT_QUAD_DESC
    {

    };

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Bubble";

private:
    CBubble(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBubble(const CBubble& Prototype);
    virtual ~CBubble() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

public:
    static CBubble* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
private:
    virtual void Free() override;
};

NS_END
