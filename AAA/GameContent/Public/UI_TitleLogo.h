#pragma once

#include "GameContent_Defines.h"
#include "UI_GenericContainer.h"

NS_BEGIN(Client)

class CLIENT_DLL CUI_TitleLogo final : public CUI_GenericContainer
{
    GENERATED_BODY(CUI_TitleLogo)

    PROPERTY(_float, m_fFadeInDur, L"FadeIn Duration", L"TitleLogo")
    PROPERTY(_float, m_fHoldDur, L"Hold Duration", L"TitleLogo")
    PROPERTY(_float, m_fFadeOutDur, L"FadeOut Duration", L"TitleLogo")

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_TitleLogo";

private:
    CUI_TitleLogo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_TitleLogo(const CUI_TitleLogo& Prototype);
    virtual ~CUI_TitleLogo() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }

public:
    virtual void On_Deserialized() override;

protected:
    virtual HRESULT Ready_Events() override;

private:
    void Show();

private:
    enum class ELOGO { IDLE, FADE_IN, HOLD, FADE_OUT };
    ELOGO  m_eState = { ELOGO::IDLE };
    _float m_fElapsed = { 0.f };

public:
    static CUI_TitleLogo* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END