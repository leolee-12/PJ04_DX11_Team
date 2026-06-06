#pragma once

#include "Effect_Container.h"

NS_BEGIN(Engine)

class CEffect_Part;

class CSwordSlash final : public CEffect_Container
{
    GENERATED_BODY(CSwordSlash)

public:
    struct SWORD_SLASH_DESC : public CEffect_Container::EFFECT_CONTAINER_DESC
    {

    };

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_SwordSlash";

private:
    CSwordSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CSwordSlash(const CSwordSlash& Prototype);
    virtual ~CSwordSlash() = default;

protected:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_EffectPartObjects();

public:
    static CSwordSlash* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
private:
    virtual void Free() override;
};

NS_END