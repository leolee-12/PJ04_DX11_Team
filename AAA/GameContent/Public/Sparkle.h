#pragma once
#include "Effect_Quad.h"

NS_BEGIN(Client)

class CSparkle final : public CEffect_Quad
{
    GENERATED_BODY(CSparkle)

public:
    struct SPARKLE_QUAD_DESC : public CEffect_Quad::EFFECT_QUAD_DESC
    {

    };

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Sparkle";

private:
    CSparkle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CSparkle(const CSparkle& Prototype);
    virtual ~CSparkle() = default;

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
    static CSparkle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
private:
    virtual void Free() override;
};

NS_END
