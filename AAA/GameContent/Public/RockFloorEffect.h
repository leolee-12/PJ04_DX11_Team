#pragma once

#include "GameContent_Defines.h"

#include "Effect_MeshParticle.h"

NS_BEGIN(Client)

class CRockFloorEffect final : public CEffect_MeshParticle
{
    GENERATED_BODY(CRockFloorEffect)

    PROPERTY(_float, m_fRiseHeight,     L"Rise Height_R",       L"RockFloor"); // 목표 솟구침 높이
    PROPERTY(_float, m_fRiseHeightRand, L"Rise Height Rand_R",  L"RockFloor"); // 파티클별 랜덤 가감(0=균일)
    PROPERTY(_float, m_fRiseTime,       L"Rise Time_R",         L"RockFloor"); // 목표 높이 도달 시간(초)
    PROPERTY(_bool,  m_bRiseEase,       L"Rise Ease_R",         L"RockFloor"); // smoothstep 이징
    PROPERTY(_float, m_fFadeOutTime,    L"FadeOut Time_R",      L"RockFloor"); // 끝나기 N초 전부터 페이드

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_RockFloorEffect";
    static constexpr const wchar_t* MODEL_PROTO_TAG = L"Proto_Model_RockFloorEffect";

private:
    CRockFloorEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CRockFloorEffect(const CRockFloorEffect& Prototype);
    virtual ~CRockFloorEffect() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Priority_Update(_float fTimeDelta) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

public:
    virtual void Effect_Start() override;

protected:
    virtual void Update_Core(const _float fTimeDelta, const _float fRatio) override;

private:
    vector<_float> m_RiseHeights;

public:
    static CRockFloorEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
private:
    virtual void Free();
};

NS_END