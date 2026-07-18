#pragma once
#include "Base.h"
#include "GameContent_Defines.h"

NS_BEGIN(Engine) 
class CGameInstance_Proxy; 
NS_END

NS_BEGIN(Client)

class CSpotlight_Rig final : public CBase
{
private:
    CSpotlight_Rig() = default;
    virtual ~CSpotlight_Rig() = default;

public:
    HRESULT Initialize(CGameInstance_Proxy* pProxy, const LIGHT_DESC& tInit);

    void   Set_Position(_fvector vPos);
    void   Aim_At(_fvector vTargetPos);                  
    void   Set_Direction(_fvector vDir);
    void   Set_Color(const _float3& vColor, _float fIntensity);
    void   Set_Cone(_float fInnerDeg, _float fOuterDeg);
    void   Set_Range(_float fRange);
    void   Set_Enabled(_bool bOn);                       
    void   Push();                                       
    _int   Get_Index() const { return m_iIndex; }

    void   Set_Target(_fvector vFocusPos);                  
    void   Set_FollowSpeed(_float f) { m_fFollowSpeed = f; }
    void   Set_OverheadOffset(_fvector vOffset);            
    void   Snap();            
    void   Snap_Fade(_bool bOn);
    void   Update(_float fTimeDelta);

    void Set_GodRayStrength(_float f) { m_fGodRayStrength = f; }
    void Set_GodRayBeamDensity(_float f) { m_fBeamDensity = f; }

    void Set_FadeDuration(_float f) { m_fFadeDuration = f; }
    void Set_FadeStartCone(_float fInnerDeg, _float fOuterDeg)
    {
        m_fFadeStartInnerDeg = fInnerDeg; m_fFadeStartOuterDeg = fOuterDeg;
    }

private:
    CGameInstance_Proxy* m_pProxy = { nullptr };
    LIGHT_DESC m_tDesc{};
    _int       m_iIndex = { -1 };

    _float3    m_vColor{ 1.f, 1.f, 0.f };
    _float     m_fIntensity = { 1.f };
    _bool      m_bEnabled = { true };

    _float3 m_vFocusTarget{};          
    _float3 m_vFocusCurrent{};         
    _float  m_fFollowSpeed = { 6.f };  
    _bool   m_bFocusInit = { false };

    _float3 m_vOverheadOffset{};
    _bool   m_bOverhead = { false };

    _float  m_fGodRayStrength = { 1.f };
    _float  m_fGodRayNear = { 12.f };
    _float  m_fBeamDensity = { 0.025f };

    _float  m_fInnerDeg = { 20.f }, m_fOuterDeg = { 25.f };
    _float  m_fFadeStartInnerDeg = { 1.f }, m_fFadeStartOuterDeg = { 3.f };

    _float  m_fFadeDuration = { 0.4f };
    _float  m_fFadeT = { 0.f };      
    _int    m_iFadeDir = { 0 };      
    _float  m_fCurFade = { 0.f };    


private:
    void Apply_Fade(_float fTimeDelta);

public:
    static CSpotlight_Rig* Create(CGameInstance_Proxy* pProxy, const LIGHT_DESC& tInit);
    virtual void Free() override;
};

NS_END