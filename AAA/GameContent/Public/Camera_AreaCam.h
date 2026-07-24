#pragma once
#include "GameContent_Defines.h"
#include "Camera_Shakeable.h"
#include "Camera_AreaData.h"
NS_BEGIN(Client)

class CLIENT_DLL CCamera_AreaCam final : public CCamera_Shakeable
{
    GENERATED_BODY(CCamera_AreaCam)
public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_CameraAreaCam";
    static constexpr const _float4  s_vSpotlightDarken = { 0.8f, 0.f, 0.f, 0.f };
    typedef struct tagAreaCamDesc final : public CCamera::CAMERA_DESC {
        wstring strDataPath = L"../../Resources/YSH/CameraData/Level0_Stage1_Step01_cam.json";
    } AREACAM_DESC;
private:
    CCamera_AreaCam(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CCamera_AreaCam(const CCamera_AreaCam& Prototype);
    virtual ~CCamera_AreaCam() = default;
public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

public:
    void    Begin_TransformZoom() { m_bTransZoom = true; }
    void    End_TransformZoom() { m_bTransZoom = false; }
    _bool   Is_TransformZoom() const { return m_bTransZoom; }
    void Begin_Handoff(_fvector vEye, _fvector vAt, _float fFovyRad = -1.f)
    {
        XMStoreFloat3(&m_eyeCur, vEye);
        XMStoreFloat3(&m_atCur, vAt);
        m_eyeVel = {};
        m_atVel = {};
        m_bInit = true;
        m_blendTimer = m_blendDur;            

        if (fFovyRad > 0.f)
            m_fFovCurDeg = XMConvertToDegrees(fFovyRad);  
    }
    void Snap()
    {
        m_bInit = false;
        m_lastArea = -1;
        m_blendTimer = 0.f;
        m_eyeVel = {};
        m_atVel = {};
        m_fFovCurDeg = -1.f;
        m_transWeight = 0.f;
        m_solver.Snap();
    }

    void Rearrange(const wstring& strDataPath);

private:
    virtual HRESULT Ready_Events() override;
    _float Resolve_TimeDelta(_float fTimeDelta);
private:
    CAreaCameraSolver m_solver;
    CGameObject* m_pTarget = nullptr;
    wstring      m_strDataPath;
    _float3 m_eyeCur = {}, m_atCur = {}, m_eyeVel = {}, m_atVel = {};
    _float m_fFovCurDeg = { -1.f };

    _float m_smoothTrans = { 0.90f };
    _float m_blendDur = { 0.60f };
    _float m_blendTimer = { 0.f };
    _int m_lastArea = { -1 };
    _bool   m_bInit = { false };

    // ==== 변신 줌인 상태 ====
    _bool   m_bTransZoom = { false };  // 변신 중 여부(외부 토글)
    _float  m_transWeight = { 0.f };   // 0=평상, 1=완전 줌인
    _float  m_transInDur = { 0.5f };   // 줌인 걸리는 시간(초)
    _float  m_transOutDur = { 0.6f };  // 원위치 복귀 시간(초)
    _float  m_transFov = { 40.f };     // 줌인 목표 fov(도), 작을수록 확대
    _float  m_transAimY = { 0.6f };    // 커비 발밑 위로 시선 올릴 오프셋

public:
    static CCamera_AreaCam* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END