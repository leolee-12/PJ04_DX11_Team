#pragma once
#include "GameContent_Defines.h"
#include "Camera.h"

NS_BEGIN(Client)

class CLIENT_DLL CCamera_Boss final : public CCamera
{
    GENERATED_BODY(CCamera_Boss)
public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_CameraBoss";

    typedef struct tagBossCamDesc final : public CCamera::CAMERA_DESC {
    } BOSSCAM_DESC;

private:
    CCamera_Boss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CCamera_Boss(const CCamera_Boss& Prototype);
    virtual ~CCamera_Boss() = default;

public:
    virtual HRESULT Initialize_Prototype() override { return S_OK; }
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

    void Snap() { m_bInit = false; m_eyeVel = {}; m_atVel = {}; }   // 활성 시 하드컷

    void Add_Shake(_float fTrauma, _float fDuration = 0.f)
    {
        m_fTrauma = min(1.f, m_fTrauma + fTrauma);
        m_fTraumaDecay = (fDuration > 0.f) ? (m_fTrauma / fDuration) : DEFAULT_TRAUMA_DECAY;
    }
    void Set_Rumble(_float fLevel) { m_fRumble = max(0.f, min(1.f, fLevel)); }
    void Stop_Rumble() { Add_Shake(m_fRumble); m_fRumble = 0.f; }

private:
    virtual HRESULT Ready_Events() override;
    void            Apply_Shake(_matrix& CamWorld);

    CGameObject* m_pPlayer = { nullptr };
    CGameObject* m_pBoss = { nullptr };
    wstring m_strPlayerLayer, m_strPlayerObj, m_strBossLayer, m_strBossObj;

    _float m_fBackDist = { 14.0f };    // 커비 뒤로
    _float m_fHeight = { 4.f };    // 카메라 높이
    _float m_fShoulderOffset = { -1.8f };   // 음수=왼쪽 어깨
    _float m_fAimBias = { 0.65f };   // 0=커비 / 1=보스 쪽을 봄
    _float m_fAimHeight = { 6.f };    // 시선 높이(보스가 크니 위로)
    _float m_fSmoothTime = { 0.18f };
    _float m_fFovDeg = { 50.f };

    _float3 m_eyeCur = {}, m_atCur = {}, m_eyeVel = {}, m_atVel = {};
    _bool   m_bInit = { false };

    _float m_fTrauma = { 0.f }, m_fShakeTime = { 0.f }, m_fRumble = { 0.f };
    _float m_fTraumaDecay = { 1.6f };
    _float m_fShakeFreq = { 22.f };
    _float m_fShakeYaw = { XMConvertToRadians(2.2f) };
    _float m_fShakePitch = { XMConvertToRadians(1.8f) };
    _float m_fShakeRoll = { XMConvertToRadians(1.2f) };
    _float m_fShakePos = { 0.12f };

    static constexpr _float DEFAULT_TRAUMA_DECAY = 1.6f;

public:
    static CCamera_Boss* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END