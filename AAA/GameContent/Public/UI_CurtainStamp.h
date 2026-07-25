#pragma once
#include "UI_CurtainTexture.h"

NS_BEGIN(Client)

// 스탬프 인트로: StartDelay 후, 에디터 비균일 스케일을 기준으로 팩터(PunchScale->1)를 눌러 착지.
// 착지 후 EndDelay초 홀드하고 완료. 종횡비 보존.
class CLIENT_DLL CUI_CurtainStamp final : public CUI_CurtainTexture
{
    GENERATED_BODY(CUI_CurtainStamp)

    PROPERTY(_float, m_fPunchScale, L"PunchScale", L"Anim")
    PROPERTY(_float, m_fEndDelay, L"EndDelay", L"Anim")
    PROPERTY(_float, m_fBounce, L"Bounce", L"Anim")

public:
    typedef struct tagUICurtainStampDesc : public CUI_CurtainTexture::UI_CURTAINTEXTURE_DESC
    {
        _float fPunchScale = { 1.4f };
        _float fEndDelay = { 1.f };
        _float fBounce = { 2.5f };
    } UI_CURTAINSTAMP_DESC;

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_UI_CurtainStamp";

protected:
    CUI_CurtainStamp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_CurtainStamp(const CUI_CurtainStamp& Prototype);
    virtual ~CUI_CurtainStamp() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;   // 비균일 팩터 펀치(베이스 애님 미사용)
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }

    virtual _bool   Is_Finished() const override { return m_bFinished; }
    virtual void    Begin_Delayed() override;
    virtual void    Reset() override;

private:
    _float2 m_vBaseScale = { 0.f, 0.f };   // 스탬프 시작 시 캡처한 에디터 비균일 스케일
    _bool   m_bPunchDone = { false };
    _float  m_fHoldAcc = { 0.f };

public:
    static CUI_CurtainStamp* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void    Free() override;
};
NS_END