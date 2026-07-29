#pragma once
#include "GameContent_Defines.h"
#include "Component.h"

NS_BEGIN(Client)
class CUI_GaugeFill;

class CLIENT_DLL CUI_GaugeBarCom final : public CComponent
{
public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Com_GaugeBar";

    typedef struct tagGaugeBarDesc
    {
        _float fMaxValue = { 100.f };
        _float fCurrent = { 100.f };
    } GAUGEBAR_DESC;

private:
    CUI_GaugeBarCom(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_GaugeBarCom(const CUI_GaugeBarCom& Prototype);
    virtual ~CUI_GaugeBarCom() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    void   Bind_Gauge(CUI_GaugeFill* pGauge) { m_pGauge = pGauge; Push(); }
    _bool  Is_Bound() const { return m_pGauge != nullptr; }

    void   Set_Value(_float fCurrent, _float fMax) { m_fMax = max(1.f, fMax); Set_Current(fCurrent); }
    void   Set_Current(_float fCurrent) { m_fCur = clamp(fCurrent, 0.f, m_fMax); Push(); }
    void   Add_Value(_float fDelta) { Set_Current(m_fCur + fDelta); }

    _float Get_Current() const { return m_fCur; }
    _float Get_Max()     const { return m_fMax; }

    void  Appear(_float fCurrent, _float fMax);
    _bool Is_Appearing() const;
    _bool Is_Filling() const;
    void  Reset_Empty();
    void  Snap_Value(_float fCurrent, _float fMax);

private:
    void   Push();

private:
    CUI_GaugeFill* m_pGauge = { nullptr };
    _float m_fMax = { 100.f };
    _float m_fCur = { 100.f };

public:
    static CUI_GaugeBarCom* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CUI_GaugeBarCom* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END