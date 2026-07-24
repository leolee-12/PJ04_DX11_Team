#pragma once
#include "UI_CurtainTexture.h"

NS_BEGIN(Client)

// 비균일 스케일 유지 + 시퀀스 시작 후 FadeDelay초 뒤 알파 1->0 페이드아웃.
class CLIENT_DLL CUI_CurtainFadeOut final : public CUI_CurtainTexture
{
    GENERATED_BODY(CUI_CurtainFadeOut)

    PROPERTY(_float, m_fFadeDelay, L"FadeDelay", L"Anim")   // 시퀀스 시작 후 이 초 뒤 페이드 시작
        PROPERTY(_float, m_fFadeDuration, L"FadeDuration", L"Anim")   // 페이드 지속(초)

public:
    typedef struct tagUICurtainFadeOutDesc : public CUI_CurtainTexture::UI_CURTAINTEXTURE_DESC
    {
        _float fFadeDelay = { 1.f };
        _float fFadeDuration = { 0.5f };
    } UI_CURTAINFADEOUT_DESC;

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_UI_CurtainFadeOut";

protected:
    CUI_CurtainFadeOut(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_CurtainFadeOut(const CUI_CurtainFadeOut& Prototype);
    virtual ~CUI_CurtainFadeOut() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }

    virtual _bool   Is_Finished() const override { return m_bFinished; }
    virtual void    Begin_Delayed() override { __super::Begin_Delayed(); m_fTimer = 0.f; m_fAlpha = 1.f; }
    virtual void    Reset() override;

private:
    _float m_fTimer = { 0.f };

public:
    static CUI_CurtainFadeOut* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void    Free() override;
};
NS_END