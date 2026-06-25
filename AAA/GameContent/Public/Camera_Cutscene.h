#pragma once
#include "GameContent_Defines.h"
#include "Camera.h"

NS_BEGIN(Engine) class CAnimator; NS_END

NS_BEGIN(Client)

// 추출한 원작 카메라 트랙 (프레임별 eye/aim)
struct CAM_TRACK
{
    _int    frameCount = 0;
    _float  fovY = 0.66f;          // radians
    _bool   aim = true;
    vector<_float3> eye;
    vector<_float3> at;

    HRESULT Load(const wstring& path);
    void    Sample(_float progress, _float3& outEye, _float3& outAt) const;
};

class CLIENT_DLL CCamera_Cutscene final : public CCamera
{
    GENERATED_BODY(CCamera_Cutscene)
public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_CameraCutscene";
    struct CUTSCENECAM_DESC : public CCamera::CAMERA_DESC {};

private:
    CCamera_Cutscene(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CCamera_Cutscene(const CCamera_Cutscene& Prototype);
    virtual ~CCamera_Cutscene() = default;

public:
    virtual HRESULT Initialize_Prototype() override { return S_OK; }
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

    // 트랙 인계 (Cutscene_CameraChange 핸들러가 호출)
    _bool Play_Track(const _tchar* szTrack, CAnimator* pProgress, const _float4x4* pAnchorWorld);
    void  Add_Shake(_float fTrauma) { m_fTrauma = min(1.f, m_fTrauma + fTrauma); }
    void Set_Rumble(_float fLevel) { m_fRumble = max(0.f, min(1.f, fLevel)); }
    void Stop_Rumble() { Add_Shake(m_fRumble); m_fRumble = 0.f; }

private:
    unordered_map<wstring, CAM_TRACK> m_Tracks;     // 이름 -> 트랙 캐시
    CAM_TRACK* m_pCur = { nullptr };
    CAnimator* m_pProgress = { nullptr };
    const _float4x4* m_pAnchor = { nullptr };
    wstring          m_strDir = L"../../Resources/YSH/CameraData/CamAnim/";

    _float m_fTrauma = { 0.f }, m_fShakeTime = { 0.f }, m_fRumble = { 0.f };
    // 튜닝
    _float m_fTraumaDecay = { 1.6f };                       // 초당 감쇠
    _float m_fShakeFreq = { 22.f };
    _float m_fShakeYaw = { XMConvertToRadians(2.2f) };
    _float m_fShakePitch = { XMConvertToRadians(1.8f) };
    _float m_fShakeRoll = { XMConvertToRadians(1.2f) };
    _float m_fShakePos = { 0.12f };

private:
    virtual HRESULT Ready_Events() override;   
    void Apply_Shake(_matrix& CamWorld);
    void Apply_Pose();

public:
    static CCamera_Cutscene* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END