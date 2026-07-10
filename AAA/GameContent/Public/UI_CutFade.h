#pragma once

#include "GameContent_Defines.h"
#include "UI_GenericContainer.h"

NS_BEGIN(Client)

// 연출용 암전 페이드: CutFade.Out 수신 -> 검게 덮고 OutDone 발행,
// 암전 유지하다 CutFade.In 수신 -> 걷어내고 InDone 발행 후 비활성
class CLIENT_DLL CUI_CutFade final : public CUI_GenericContainer
{
    GENERATED_BODY(CUI_CutFade)

    PROPERTY(_float, m_fOutDur, L"FadeOut Duration", L"CutFade")
    PROPERTY(_float, m_fInDur, L"FadeIn Duration", L"CutFade")

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_CutFade";

private:
    CUI_CutFade(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_CutFade(const CUI_CutFade& Prototype);
    virtual ~CUI_CutFade() = default;

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
    void Begin_FadeOut();
    void Begin_FadeIn();

private:
    enum class EFADE { IDLE, FADEOUT, BLACK, FADEIN };
    EFADE  m_eState = { EFADE::IDLE };
    _float m_fElapsed = { 0.f };

public:
    static CUI_CutFade* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END