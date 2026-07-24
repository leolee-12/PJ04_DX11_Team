#pragma once
#include "GameContent_Defines.h"
#include "Camera_Shakeable.h"

NS_BEGIN(Client)

// 커비가 제트코스터를 탈 때 커비 진행방향(look: 요+피치)에 카메라 각도를 맞추는 체이스 카메라.
// 롤(뱅킹)은 따라가지 않고 월드업 기준으로 화면 수평을 유지한다.
class CLIENT_DLL CCamera_Coaster final : public CCamera_Shakeable
{
    GENERATED_BODY(CCamera_Coaster)
public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_CameraCoaster";

    typedef struct tagCoasterCamDesc final : public CCamera::CAMERA_DESC {
    } COASTERCAM_DESC;

private:
    CCamera_Coaster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CCamera_Coaster(const CCamera_Coaster& Prototype);
    virtual ~CCamera_Coaster() = default;

public:
    virtual HRESULT Initialize_Prototype() override { return S_OK; }
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

    void Snap() { m_bInit = false; m_eyeVel = {}; m_atVel = {}; }   // 활성 시 하드 스냅
    void Clear_LevelRefs() { m_pPlayer = nullptr; }

private:
    virtual HRESULT Ready_Events() override;

    CGameObject* m_pPlayer = { nullptr };

    // 커비 기준 배치 튜닝값
    _float m_fBackDist = { 9.f };    // 진행 반대로 물러난 거리
    _float m_fHeight = { 7.f };   // 월드업 방향 높이
    _float m_fAimAhead = { 10.f };    // 시선을 진행방향 앞으로 당기는 양
    _float m_fAimHeight = { 1.2f };   // 시선 높이
    _float m_fSmoothTime = { 0.25f };  // 코스터라 area보다 빠르게 추종
    _float m_fFovDeg = { 60.f };   // 속도감 위해 살짝 넓게

    _float3 m_eyeCur = {}, m_atCur = {}, m_eyeVel = {}, m_atVel = {};
    _bool   m_bInit = { false };

public:
    static CCamera_Coaster* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END