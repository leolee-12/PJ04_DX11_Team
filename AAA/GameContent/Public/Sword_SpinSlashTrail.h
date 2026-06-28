#pragma once

#include "Effect_Container.h"

NS_BEGIN(Engine)

class CEffect_Part;

class CSword_SpinSlashTrail final : public CEffect_Container
{
    GENERATED_BODY(CSword_SpinSlashTrail)

public:
    struct SWORD_SLASH1_DESC : public CEffect_Container::EFFECT_CONTAINER_DESC
    {

    };

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Sword_SpinSlashTrail";

private:
    CSword_SpinSlashTrail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CSword_SpinSlashTrail(const CSword_SpinSlashTrail& Prototype);
    virtual ~CSword_SpinSlashTrail() = default;

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
    void Start_FadeOut(_float fFadeOutDuration = 0.3f);

    static CSword_SpinSlashTrail* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

private:
    _bool m_bFadeOutRequested{};

private:
    virtual void Free() override;
};

NS_END
