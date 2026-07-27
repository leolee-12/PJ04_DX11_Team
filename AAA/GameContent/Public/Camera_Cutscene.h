#pragma once
#include "GameContent_Defines.h"
#include "Camera_Shakeable.h"

NS_BEGIN(Engine) 
class CAnimator;
NS_END

NS_BEGIN(Client)

// 추출한 원작 카메라 트랙 (프레임별 eye/aim)
struct CAM_TRACK
{
    _int    frameCount = 0;
    _float  fovY = 0.66f;          // radians
    _bool   aim = true;
    vector<_float3> eye;
    vector<_float3> at;
    vector<_float>  fov;

    HRESULT Load(const wstring& path);
    void    Sample(_float progress, _float3& outEye, _float3& outAt, _float& outFov) const;
};

class CLIENT_DLL CCamera_Cutscene final : public CCamera_Shakeable
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
    _bool Play_Track(const _tchar* szTrack, CAnimator* pProgress, const _float4x4* pAnchorWorld, _float fSpeed = 1.f);

    void   Set_TrackSpeed(_float fSpeed) { m_fSpeed = fSpeed; }
    _float Get_TrackSpeed() const { return m_fSpeed; }

private:
    unordered_map<wstring, CAM_TRACK> m_Tracks;     // 이름 -> 트랙 캐시
    CAM_TRACK* m_pCur = { nullptr };
    CAnimator* m_pProgress = { nullptr };
    const _float4x4* m_pAnchor = { nullptr };
    wstring          m_strDir = L"../../Resources/YSH/CameraData/CamAnim/";

    _float m_fLocalTime = { 0.f };
    _float m_fSpeed = { 1.f };

private:
    virtual HRESULT Ready_Events() override;   
    void Apply_Pose();

public:
    static CCamera_Cutscene* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END