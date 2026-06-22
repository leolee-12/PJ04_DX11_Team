#pragma once
#include "GameContent_Defines.h"
#include "UIContainerObject.h"

NS_BEGIN(Client)

class CUI_GaugeFill;
class CUI_GaugeBarCom;
class CUI_Text;

class CLIENT_DLL CUI_BossStatus final : public CUIContainerObject
{
    GENERATED_BODY(CUI_BossStatus)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_BossStatus";

private:
    CUI_BossStatus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_BossStatus(const CUI_BossStatus& Prototype);
    virtual ~CUI_BossStatus() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Priority_Update(_float fTimeDelta) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }

protected:
    virtual HRESULT Ready_Events() override;

private:
    CUI_GaugeBarCom* m_pGaugeBar = { nullptr };
    CUI_Text* m_pNameText = { nullptr };
    _wstring  m_strPendingName;

    _float m_fDefaultMaxHP = { 100.f };
    _float m_fDefaultCurrHP = { 100.f };

    _bool  m_bPendingHP = { false };
    _float m_fPendingMax = { 100.f };
    _float m_fPendingCurr = { 100.f };
    _bool  m_bPendingAppear = { false };

    enum class APPEAR_PHASE { NONE, SLIDING, DONE };
    APPEAR_PHASE m_eAppearPhase = { APPEAR_PHASE::NONE };

    _float3 m_vHomePos = {};
    _float m_fSlideTime = { 0.f };
    _float m_fSlideDuration = { 1.f };
    _float m_fSlideOffsetX = { 600.f };

private:
    HRESULT Ready_Components();
    void    Try_BindGauge();
    void    Start_SlideIn();
    void    Update_SlideIn(_float fTimeDelta);
    static _float Ease_OutCubic(_float t) 
    { 
        _float u = 1.f - t;  
        return 1.f - u * u * u;
    }

public:
    static CUI_BossStatus* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END