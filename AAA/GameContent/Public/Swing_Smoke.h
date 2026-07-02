#pragma once

#include "GameContent_Defines.h"
#include "Effect_MeshParticle.h"

NS_BEGIN(Client)

class CSwing_Smoke final : public CEffect_MeshParticle
{
    GENERATED_BODY(CSwing_Smoke)

    // --- 휘두르기 전용: 반원 호 + 순차 스폰 (베이스 미제공) ---
    PROPERTY(_float, m_fRadius, L"Radius_S", L"Swing"); // 중점에서 거리
    PROPERTY(_float, m_fStartAngleDeg, L"Start Angle_S", L"Swing");
    PROPERTY(_float, m_fArcSpanDeg, L"Arc Span_S", L"Swing"); // 180 = 반원
    PROPERTY(_float, m_fSweepWindow, L"Sweep Window_S", L"Swing"); // 호를 쓰는 비율시간
    PROPERTY(_float, m_fFaceOffsetDeg, L"Face Offset_S", L"Swing"); // 메쉬 정렬 오프셋

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Swing_Smoke";
    static constexpr const wchar_t* MODEL_PROTO_TAG = L"Prototype_Component_Model_SmokeSphereOriginal";

private:
    CSwing_Smoke(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CSwing_Smoke(const CSwing_Smoke& Prototype);
    virtual ~CSwing_Smoke() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Priority_Update(_float fTimeDelta) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    virtual void Effect_Start() override;

    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

public:
    static CSwing_Smoke* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
private:
    virtual void Free();
};

NS_END