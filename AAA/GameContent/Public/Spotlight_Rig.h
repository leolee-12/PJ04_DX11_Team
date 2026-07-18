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
    void   Aim_At(_fvector vTargetPos);                  // 현재 위치에서 타겟으로 콘 축
    void   Set_Direction(_fvector vDir);
    void   Set_Color(const _float3& vColor, _float fIntensity);
    void   Set_Cone(_float fInnerDeg, _float fOuterDeg); // 전체 콘각(도)
    void   Set_Range(_float fRange);
    void   Set_Enabled(_bool bOn);                       // off = 색 0
    void   Push();                                       // 변경분 반영 (프레임당 1회)
    _int   Get_Index() const { return m_iIndex; }

    void   Set_Target(_fvector vFocusPos);                  // 비출 지점(레오파드 or 커비)만 지정
    void   Set_FollowSpeed(_float f) { m_fFollowSpeed = f; }
    void   Set_OverheadOffset(_fvector vOffset);            // 설정 시 광원이 포커스 위로 따라감(옵션)
    void   Snap();                                          // 즉시 정렬(스윕 없이)
    void   Update(_float fTimeDelta);

    void Set_GodRayStrength(_float f) { m_fGodRayStrength = f; }
    void Set_GodRayBeamDensity(_float f) { m_fBeamDensity = f; }

public:
    static CSpotlight_Rig* Create(CGameInstance_Proxy* pProxy, const LIGHT_DESC& tInit);
    virtual void Free() override;

private:
    CGameInstance_Proxy* m_pProxy = { nullptr };
    LIGHT_DESC m_tDesc{};
    _int       m_iIndex = { -1 };

    _float3    m_vColor{ 1.f, 1.f, 1.f };
    _float     m_fIntensity = { 1.f };
    _bool      m_bEnabled = { true };

    _float3 m_vFocusTarget{};          
    _float3 m_vFocusCurrent{};         
    _float  m_fFollowSpeed = { 6.f };  
    _bool   m_bFocusInit = { false };

    _float3 m_vOverheadOffset{};
    _bool   m_bOverhead = { false };

    _float  m_fGodRayStrength = { 1.f };
    _float m_fBeamDensity = { 1.2f };
};

NS_END